#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
const size_t DEFAULT_CAPACITY_FOR_EACH_BLOCK = 40;
namespace sjtu {
int countOfSplit = 0;
int countOfMerge = 0;
template <typename T>
inline T max(T a, T b) { return a > b ? a : b; }

template <class T>
class deque {
public:
    class iterator;
    class const_iterator;
    friend iterator;
    friend const_iterator;

private:
    class Node {
    public:
        T _data;
        Node* prevNode;
        Node* nextNode;
        Node()
            : prevNode(nullptr)
            , nextNode(nullptr)
        {
        }
        Node(const T& data0, Node* prev = nullptr, Node* next = nullptr)
            : prevNode(prev)
            , nextNode(next)
            , _data(data0)
        {
        }
        Node(const Node& other)
            : prevNode(nullptr)
            , nextNode(nullptr)
            , _data(other._data)
        {
        }
        ~Node()
        {
        }
    };
    class Block {
    public:
        size_t _sizeOfBlock = 0;
        Node* headNode;
        Node* tailNode;
        Block* prevBlock;
        Block* nextBlock;
        Block()
            : prevBlock(nullptr)
            , nextBlock(nullptr)
        {
            headNode = (Node*)malloc(sizeof(Node));
            tailNode = (Node*)malloc(sizeof(Node));
            headNode->nextNode = tailNode;
            tailNode->prevNode = headNode;
        }

        // copy constructor
        // deep copy : create a new Block the same as the other
        Block(const Block& other)
            : prevBlock(nullptr)
            , nextBlock(nullptr)
        {
            headNode = (Node*)malloc(sizeof(Node));
            tailNode = (Node*)malloc(sizeof(Node));
            Node* tmpCopy = other.headNode->nextNode;
            Node* tmp;
            Node* tmpPrev = headNode;
            while (tmpCopy != other.tailNode) {
                tmp = new Node(*tmpCopy);
                tmpPrev->nextNode = tmp;
                tmp->prevNode = tmpPrev;
                tmpPrev = tmp;
                tmpCopy = tmpCopy->nextNode;
            }
            _sizeOfBlock = other._sizeOfBlock;
            tailNode->prevNode = tmpPrev;
            tmpPrev->nextNode = tailNode;
        }
        ~Block()
        {
            Node* tmp = headNode->nextNode;
            while (tmp != tailNode) {
                Node* tmpNode = tmp;
                tmp = tmp->nextNode;
                delete tmpNode;
            }
            free(headNode);
            free(tailNode);
        }
    };
    // the head and tail Block
    Block* headBlock;
    Block* tailBlock;
    // store the number of elements
    // update when add and remove
    size_t _size = 0;

    size_t getSqrtN()
    {
        return max(size_t(sqrt(_size)), DEFAULT_CAPACITY_FOR_EACH_BLOCK);
    }
    // split the block to two blocks with half the size
    // pos is an optional arg to update an iterator which may be re-used by other function
    void Split(Block* BlockToSplit, iterator& pos)
    {
        size_t tmpSqrtN = getSqrtN();
        if (BlockToSplit->_sizeOfBlock < 2 * tmpSqrtN) { // no need to split
            return;
        } else {
            int newSize = BlockToSplit->_sizeOfBlock >> 1;
            Block* newBlock = new Block;
            newBlock->nextBlock = BlockToSplit->nextBlock;
            newBlock->prevBlock = BlockToSplit;
            BlockToSplit->nextBlock->prevBlock = newBlock;
            BlockToSplit->nextBlock = newBlock;
            Node* tmp = BlockToSplit->headNode;
            bool flag = false; // pos in the former half or the latter
            for (int i = 0; i < newSize; i++) {
                tmp = tmp->nextNode;
                if (pos.getDeque() != nullptr && pos.getNode() == tmp) {
                    flag = true;
                }
            }
            newBlock->headNode->nextNode = tmp->nextNode;
            newBlock->tailNode->prevNode = BlockToSplit->tailNode->prevNode;
            newBlock->headNode->nextNode->prevNode = newBlock->headNode;
            newBlock->tailNode->prevNode->nextNode = newBlock->tailNode;
            tmp->nextNode = BlockToSplit->tailNode;
            BlockToSplit->tailNode->prevNode = tmp;
            // update size
            newBlock->_sizeOfBlock = BlockToSplit->_sizeOfBlock - newSize;
            BlockToSplit->_sizeOfBlock = newSize;
            if (pos.getDeque() != nullptr && !flag) {
                pos.getBlock() = newBlock;
            }
            countOfSplit++;
        }
    }
    // merge the block and the next one to a larger one
    // update pos
    void Merge(Block* Block1, Block* Block2, iterator& pos)
    {
        size_t tmpSqrtN = getSqrtN();
        if (Block1->_sizeOfBlock + Block2->_sizeOfBlock > 2 * tmpSqrtN) {
            return;
        } else {
            //block connect
            Block1->nextBlock = Block2->nextBlock;
            Block2->nextBlock->prevBlock = Block1;
            // node connect
            Node* tmp = Block1->tailNode->prevNode; // get the last node of Block1

            tmp->nextNode = Block2->headNode->nextNode;
            Block1->tailNode->prevNode = Block2->tailNode->prevNode;

            Block2->headNode->nextNode->prevNode = tmp;
            Block2->tailNode->prevNode->nextNode = Block1->tailNode;
            Block2->headNode->nextNode = Block2->tailNode;
            Block2->tailNode->prevNode = Block2->headNode;
            // update size
            Block1->_sizeOfBlock = Block1->_sizeOfBlock + Block2->_sizeOfBlock;
            delete Block2;
            if (pos.getDeque() != nullptr) {
                pos.getBlock() = Block1;
            }
            countOfMerge++;
        }
    }

public:
    class iterator {
    private:
        /**
             * TODO add _storage members
             *   just add whatever you want.
             */
        deque<T>* _deque;
        Block* _block;
        Node* _node;
        size_t getIndex() const
        {
            size_t count = 0;
            Block* tmp = _deque->headBlock->nextBlock;
            while (tmp != _block) {
                count += tmp->_sizeOfBlock;
                tmp = tmp->nextBlock;
            }
            Node* tmpNode = _block->headNode->nextNode;
            while (tmpNode != _node) {
                count++;
                tmpNode = tmpNode->nextNode;
            }
            return count;
        }

    public:
        /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, the behaviour is **undefined**.
             * as well as operator-
             */
        iterator(deque<T>* d = nullptr, Block* b = nullptr, Node* n = nullptr)
            : _deque(d)
            , _block(b)
            , _node(n)
        {
        }
        iterator(const iterator& other)
            : _deque(other._deque)
            , _block(other._block)
            , _node(other._node)
        {
        }
        iterator(const const_iterator& other)
            : _deque(other._deque)
            , _block(other._block)
            , _node(other._node)
        {
        }
        iterator operator+(const int& n) const
        {
            //TODO
            if (n < 0) {
                return (*this) - (-n);
            } else {
                iterator tmp(*this);
                tmp += n;
                return tmp;
            }
        }
        iterator operator-(const int& n) const
        {
            //TODO
            if (n < 0) {
                return (*this) + (-n);
            } else {
                iterator tmp(*this);
                tmp -= n;
                return tmp;
            }
        }
        // return th distance between two iterator,
        // if these two iterators points to different vectors, throw invaild_iterator.
        int operator-(const iterator& rhs) const
        {
            //TODO
            if (_deque != rhs._deque) {
                throw invalid_iterator();
            } else {
                size_t index1 = getIndex();
                size_t index2 = rhs.getIndex();
                return index1 - index2;
            }
        }
        iterator& operator+=(const int& n)
        {
            if (n < 0) {
                (*this) -= (-n);
                return *this;
            } else if (n == 0) {
                return *this;
            } else {
                int count = n;
                if (_node == _block->tailNode && _block == _deque->tailBlock->prevBlock) // end() + n
                    throw invalid_iterator();
                // in the same block
                while (_node->nextNode != _block->tailNode) {
                    _node = _node->nextNode;
                    count--;
                    if (count == 0)
                        return (*this);
                }
                // not in the same block
                while (_block->nextBlock != _deque->tailBlock) {
                    _block = _block->nextBlock;
                    if (_block->_sizeOfBlock >= count) { // found the block
                        _node = _block->headNode;
                        for (int i = 0; i < count; i++) {
                            _node = _node->nextNode;
                        }
                        return (*this);
                    } else {
                        count -= _block->_sizeOfBlock;
                    }
                }
                if (count == 1) {
                    *this = _deque->end();
                    return *this;
                } else
                    throw invalid_iterator();
            }
        }
        iterator& operator-=(const int& n)
        {
            //TODO
            if (n < 0) {
                (*this) += (-n);
                return *this;
            } else if (n == 0) {
                return *this;
            } else {
                int count = n;
                if (_block->prevBlock == _deque->headBlock && _node->prevNode == _block->headNode)
                    throw invalid_iterator();
                // in the same block
                while (_node->prevNode != _block->headNode) {
                    _node = _node->prevNode;
                    count--;
                    if (count == 0)
                        return (*this);
                }
                // not in the same block
                while (_block->prevBlock != _deque->headBlock) {
                    _block = _block->prevBlock;
                    if (_block->_sizeOfBlock >= count) {
                        _node = _block->tailNode;
                        for (int i = 0; i < count; i++) {
                            _node = _node->prevNode;
                        }
                        return (*this);
                    } else {
                        count -= _block->_sizeOfBlock;
                    }
                }
                throw invalid_iterator();
            }
        }
        /**
             * TODO iter++
             */
        iterator operator++(int)
        {
            iterator tmp(*this);
            if (_node == _block->tailNode && _block == _deque->tailBlock->prevBlock) // end()++
                throw invalid_iterator();
            if (_node->nextNode != _block->tailNode) {
                _node = _node->nextNode;
            } else if (_block->nextBlock != _deque->tailBlock) { // reach the end of the block
                _block = _block->nextBlock;
                _node = _block->headNode->nextNode;
            } else { // reach end()
                _node = _node->nextNode;
            }
            return tmp;
        }
        /**
             * TODO ++iter
             */
        iterator& operator++()
        {
            if (_node == _block->tailNode && _block == _deque->tailBlock->prevBlock) // end()++
                throw invalid_iterator();
            if (_node->nextNode != _block->tailNode) {
                _node = _node->nextNode;
            } else if (_block->nextBlock != _deque->tailBlock) {
                _block = _block->nextBlock;
                _node = _block->headNode->nextNode;
            } else {
                _node = _node->nextNode;
            }
            return *this;
        }
        /**
             * TODO iter--
             */
        iterator operator--(int)
        {
            iterator tmp(*this);
            if (_node->prevNode != _block->headNode) {
                _node = _node->prevNode;
            } else if (_block->prevBlock != _deque->headBlock) { // reach the start of the block
                _block = _block->prevBlock;
                _node = _block->tailNode->prevNode;
            } else {
                throw invalid_iterator();
            }
            return tmp;
        }
        /**
             * TODO --iter
             */
        iterator& operator--()
        {
            if (_node->prevNode != _block->headNode) {
                _node = _node->prevNode;
            } else if (_block->prevBlock != _deque->headBlock) {
                _block = _block->prevBlock;
                _node = _block->tailNode->prevNode;
            } else {
                throw invalid_iterator();
            }
            return (*this);
        }
        /**
             * TODO *it
             */
        T& operator*() const
        {
            if (_node == _block->tailNode || _block == _deque->headBlock)
                throw invalid_iterator();
            return _node->_data;
        }
        /**
             * TODO it->field
             */
        T* operator->() const noexcept
        {
            if (_node == _block->tailNode || _block == _deque->headBlock)
                throw invalid_iterator();
            return &(_node->_data);
        }
        /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
        bool operator==(const iterator& rhs) const
        {
            if (_node == rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        bool operator==(const const_iterator& rhs) const
        {
            if (_node == rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        /**
             * some other operator for iterator.
             */
        bool operator!=(const iterator& rhs) const
        {
            if (_node != rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        bool operator!=(const const_iterator& rhs) const
        {
            if (_node != rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        deque<T>*& getDeque()
        {
            return _deque;
        }
        Block*& getBlock()
        {
            return _block;
        }
        Node*& getNode()
        {
            return _node;
        }
    };
    class const_iterator {
        // it should has similar member method as iterator.
        //  and it should be able to construct from an iterator.
    private:
        // _storage members.
        const deque<T>* _deque;
        Block* _block;
        Node* _node;
        size_t getIndex() const
        {
            size_t count = 0;
            Block* tmp = _deque->headBlock->nextBlock;
            while (tmp != _block) {
                count += tmp->_sizeOfBlock;
                tmp = tmp->nextBlock;
            }
            Node* tmpNode = _block->headNode->nextNode;
            while (tmpNode != _node) {
                count++;
                tmpNode = tmpNode->nextNode;
            }
            return count;
        }

    public:
        const_iterator(const deque<T>* d = nullptr, Block* b = nullptr, Node* n = nullptr)
            : _deque(d)
            , _block(b)
            , _node(n)
        {
            // TODO
        }
        const_iterator(const const_iterator& other)
            : _deque(other._deque)
            , _block(other._block)
            , _node(other._node)
        {
            // TODO
        }
        const_iterator(const iterator& other)
            : _deque(other._deque)
            , _block(other._block)
            , _node(other._node)
        {
            // TODO
        }
        // And other methods in iterator.
        // And other methods in iterator.
        // And other methods in iterator.
        const_iterator operator+(const int& n) const
        {
            //TODO
            const_iterator tmp(*this);
            tmp += n;
            return tmp;
        }
        const_iterator operator-(const int& n) const
        {
            //TODO
            const_iterator tmp(*this);
            tmp -= n;
            return tmp;
        }
        // return th distance between two iterator,
        // if these two iterators points to different vectors, throw invaild_iterator.
        int operator-(const const_iterator& rhs) const
        {
            //TODO
            if (_deque != rhs._deque) {
                throw invalid_iterator();
            } else {

                size_t index1 = getIndex();
                size_t index2 = rhs.getIndex();
                return index1 - index2;
            }
        }
        const_iterator& operator+=(const int& n)
        {
            if (n < 0) {
                (*this) -= (-n);
                return *this;
            } else if (n == 0) {
                return *this;
            } else {
                int count = n;
                if (_node == _block->tailNode && _block == _deque->tailBlock->prevBlock) // end() + n
                    throw invalid_iterator();
                // in the same block
                while (_node->nextNode != _block->tailNode) {
                    _node = _node->nextNode;
                    count--;
                    if (count == 0)
                        return (*this);
                }
                // not in the same block
                while (_block->nextBlock != _deque->tailBlock) {
                    _block = _block->nextBlock;
                    if (_block->_sizeOfBlock >= count) { // found the block
                        _node = _block->headNode;
                        for (int i = 0; i < count; i++) {
                            _node = _node->nextNode;
                        }
                        return (*this);
                    } else {
                        count -= _block->_sizeOfBlock;
                    }
                }
                if (count == 1) {
                    *this = _deque->cend();
                    return *this;
                } else
                    throw invalid_iterator();
            }
        }
        const_iterator& operator-=(const int& n)
        {
            //TODO
            if (n < 0) {
                (*this) += (-n);
                return *this;
            } else if (n == 0) {
                return *this;
            } else {
                int count = n;
                if (_block->prevBlock == _deque->headBlock && _node->prevNode == _block->headNode)
                    throw invalid_iterator();
                // in the same block
                while (_node->prevNode != _block->headNode) {
                    _node = _node->prevNode;
                    count--;
                    if (count == 0)
                        return (*this);
                }
                // not in the same block
                while (_block->prevBlock != _deque->headBlock) {
                    _block = _block->prevBlock;
                    if (_block->_sizeOfBlock >= count) {
                        _node = _block->tailNode;
                        for (int i = 0; i < count; i++) {
                            _node = _node->prevNode;
                        }
                        return (*this);
                    } else {
                        count -= _block->_sizeOfBlock;
                    }
                }
                throw invalid_iterator();
            }
        }
        /**
             * TODO iter++
             */
        const_iterator operator++(int)
        {
            const_iterator tmp(*this);
            if (_node == _block->tailNode && _block == _deque->tailBlock->prevBlock) // end()++
                throw invalid_iterator();
            if (_node->nextNode != _block->tailNode) {
                _node = _node->nextNode;
            } else if (_block->nextBlock != _deque->tailBlock) { // reach the end of the block
                _block = _block->nextBlock;
                _node = _block->headNode->nextNode;
            } else { // reach end()
                _node = _node->nextNode;
            }
            return tmp;
        }
        /**
             * TODO ++iter
             */
        const_iterator& operator++()
        {
            if (_node == _block->tailNode && _block == _deque->tailBlock->prevBlock) // end()++
                throw invalid_iterator();
            if (_node->nextNode != _block->tailNode) {
                _node = _node->nextNode;
            } else if (_block->nextBlock != _deque->tailBlock) { // reach the end of the block
                _block = _block->nextBlock;
                _node = _block->headNode->nextNode;
            } else { // reach end()
                _node = _node->nextNode;
            }
            return *this;
        }
        /**
             * TODO iter--
             */
        const_iterator operator--(int)
        {
            const_iterator tmp(*this);
            if (_node->prevNode != _block->headNode) {
                _node = _node->prevNode;
            } else if (_block->prevBlock != _deque->headBlock) { // reach the start of the block
                _block = _block->prevBlock;
                _node = _block->tailNode->prevNode;
            } else {
                throw invalid_iterator();
            }
            return tmp;
        }
        /**
             * TODO --iter
             */
        const_iterator& operator--()
        {
            if (_node->prevNode != _block->headNode) {
                _node = _node->prevNode;
            } else if (_block->prevBlock != _deque->headBlock) {
                _block = _block->prevBlock;
                _node = _block->tailNode->prevNode;
            } else {
                throw invalid_iterator();
            }
            return (*this);
        }
        /**
             * TODO *it
             */
        const T& operator*() const
        {
            if (_node == _block->tailNode || _node == _block->headNode)
                throw invalid_iterator();
            return _node->_data;
        }
        /**
             * TODO it->field
             */
        const T* operator->() const noexcept
        {
            return &(_node->_data);
        }
        /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
        bool operator==(const iterator& rhs) const
        {
            if (_node == rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        bool operator==(const const_iterator& rhs) const
        {
            if (_node == rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        /**
             * some other operator for iterator.
             */
        bool operator!=(const iterator& rhs) const
        {
            if (_node != rhs._node) {
                return true;
            } else {
                return false;
            }
        }
        bool operator!=(const const_iterator& rhs) const
        {
            if (_node != rhs._node) {
                return true;
            } else {
                return false;
            }
        }
    };
    /**
         * TODO Constructors
         */
    deque()
    {
        headBlock = new Block;
        tailBlock = new Block;
        Block* tmp = new Block;
        tmp->nextBlock = tailBlock;
        tmp->prevBlock = headBlock;
        headBlock->nextBlock = tmp;
        tailBlock->prevBlock = tmp;
        _size = 0;
    }
    deque(const deque& other)
    {
        headBlock = new Block;
        tailBlock = new Block;
        Block* tmpCopy = other.headBlock->nextBlock;
        Block* tmp;
        Block* tmpPrev = headBlock;
        while (tmpCopy != other.tailBlock) {
            tmp = new Block(*tmpCopy);
            tmp->prevBlock = tmpPrev;
            tmpPrev->nextBlock = tmp;
            tmpPrev = tmp;
            tmpCopy = tmpCopy->nextBlock;
        }
        tailBlock->prevBlock = tmpPrev;
        tmpPrev->nextBlock = tailBlock;
        _size = other._size;
    }
    /**
         * TODO Deconstructor
         */
    ~deque()
    {
        Block* tmp = headBlock->nextBlock;
        while (tmp != tailBlock) {
            Block* tmpBlock = tmp;
            tmp = tmp->nextBlock;
            delete tmpBlock;
        }
        delete headBlock;
        delete tailBlock;
    }
    /**
         * TODO assignment operator
         */
    deque& operator=(const deque& other)
    {
        // free
        if (&other == this)
            return *this;
        Block* tmp = headBlock->nextBlock;
        while (tmp != tailBlock) {
            Block* tmpBlock = tmp;
            tmp = tmp->nextBlock;
            delete tmpBlock;
        }
        Block* tmpCopy = other.headBlock->nextBlock;
        Block* tmpPrev = headBlock;
        while (tmpCopy != other.tailBlock) {
            tmp = new Block(*tmpCopy);
            tmp->prevBlock = tmpPrev;
            tmpPrev->nextBlock = tmp;
            tmpPrev = tmp;
            tmpCopy = tmpCopy->nextBlock;
        }
        tailBlock->prevBlock = tmpPrev;
        tmpPrev->nextBlock = tailBlock;
        _size = other._size;
        return *this;
    }
    /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
    T& at(const size_t& pos)
    {
        // pos is 0-base
        if (pos < 0 || pos >= _size) {
            throw index_out_of_bound();
        }
        Block* tmpBlock = headBlock->nextBlock;
        size_t count = 0;
        while (tmpBlock != tailBlock) {
            if (count + tmpBlock->_sizeOfBlock >= pos + 1) { // if reach the block
                Node* tmpNode = tmpBlock->headNode->nextNode;
                while (tmpNode != tmpBlock->tailNode) {
                    count++;
                    if (count == pos + 1) {
                        return tmpNode->_data;
                    }
                    tmpNode = tmpNode->nextNode;
                }
            } else {
                count += tmpBlock->_sizeOfBlock;
            }
            tmpBlock = tmpBlock->nextBlock;
        }
        throw index_out_of_bound();
    }
    const T& at(const size_t& pos) const
    {
        // pos is 0-base
        if (pos < 0 || pos >= _size) {
            throw index_out_of_bound();
        }
        Block* tmpBlock = headBlock->nextBlock;
        size_t count = 0;
        while (tmpBlock != tailBlock) {
            if (count + tmpBlock->_sizeOfBlock >= pos + 1) { // if reach the block
                Node* tmpNode = tmpBlock->headNode->nextNode;
                while (tmpNode != tmpBlock->tailNode) {
                    count++;
                    if (count == pos + 1) {
                        return tmpNode->_data;
                    }
                    tmpNode = tmpNode->nextNode;
                }
            } else {
                count += tmpBlock->_sizeOfBlock;
            }
            tmpBlock = tmpBlock->nextBlock;
        }
        throw index_out_of_bound();
    }
    T& operator[](const size_t& pos)
    {
        // pos is 0-base
        if (pos < 0 || pos >= _size) {
            throw index_out_of_bound();
        }
        Block* tmpBlock = headBlock->nextBlock;
        size_t count = 0;
        while (tmpBlock != tailBlock) {
            if (count + tmpBlock->_sizeOfBlock >= pos + 1) { // if reach the block
                Node* tmpNode = tmpBlock->headNode->nextNode;
                while (tmpNode != tmpBlock->tailNode) {
                    count++;
                    if (count == pos + 1) {
                        return tmpNode->_data;
                    }
                    tmpNode = tmpNode->nextNode;
                }
            } else {
                count += tmpBlock->_sizeOfBlock;
            }
            tmpBlock = tmpBlock->nextBlock;
        }
        throw index_out_of_bound();
    }
    const T& operator[](const size_t& pos) const
    {
        // pos is 0-base
        if (pos < 0 || pos >= _size) {
            throw index_out_of_bound();
        }
        Block* tmpBlock = headBlock->nextBlock;
        size_t count = 0;
        while (tmpBlock != tailBlock) {
            if (count + tmpBlock->_sizeOfBlock >= pos + 1) { // if reach the block
                Node* tmpNode = tmpBlock->headNode->nextNode;
                while (tmpNode != tmpBlock->tailNode) {
                    count++;
                    if (count == pos + 1) {
                        return tmpNode->_data;
                    }
                    tmpNode = tmpNode->nextNode;
                }
            } else {
                count += tmpBlock->_sizeOfBlock;
            }
            tmpBlock = tmpBlock->nextBlock;
        }
        throw index_out_of_bound();
    }
    /**
         * access the first element
         * throw container_is_empty when the container is empty.
         */
    const T& front() const
    {
        if (_size == 0)
            throw container_is_empty();
        return headBlock->nextBlock->headNode->nextNode->_data;
    }
    /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
    const T& back() const
    {
        if (_size == 0)
            throw container_is_empty();
        return tailBlock->prevBlock->tailNode->prevNode->_data;
    }
    /**
         * returns an iterator to the beginning.
         */
    iterator begin()
    {
        return iterator(this, headBlock->nextBlock, headBlock->nextBlock->headNode->nextNode);
    }
    const_iterator cbegin() const
    {
        return const_iterator(this, headBlock->nextBlock, headBlock->nextBlock->headNode->nextNode);
    }
    /**
         * returns an iterator to the end.
         */
    iterator end()
    {
        return iterator(this, tailBlock->prevBlock, tailBlock->prevBlock->tailNode);
    }
    const_iterator cend() const
    {
        return const_iterator(this, tailBlock->prevBlock, tailBlock->prevBlock->tailNode);
    }
    /**
         * checks whether the container is empty.
         */
    bool empty() const
    {
        if (_size == 0)
            return true;
        else
            return false;
    }
    /**
         * returns the number of elements
         */
    size_t size() const
    {
        return _size;
    }
    /**
         * clears the contents
         */
    void clear()
    {
        Block* tmp = headBlock->nextBlock;
        while (tmp != tailBlock) {
            Block* tmpBlock = tmp;
            tmp = tmp->nextBlock;
            delete tmpBlock;
        }
        tmp = new Block;
        headBlock->nextBlock = tmp;
        tailBlock->prevBlock = tmp;
        tmp->prevBlock = headBlock;
        tmp->nextBlock = tailBlock;
        _size = 0;
    }
    /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
    iterator insert(iterator pos, const T& value)
    {
        if (pos.getDeque() != this) {
            throw invalid_iterator();
        }
        Node* tmpNode = pos.getNode();
        Node* tmp = new Node(value, tmpNode->prevNode, tmpNode);
        tmpNode->prevNode->nextNode = tmp;
        tmpNode->prevNode = tmp;
        pos.getBlock()->_sizeOfBlock++;
        _size++;
        pos--; // move to the one inserted
        Split(pos.getBlock(), pos);
        return pos;
    }
    /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
    iterator erase(iterator pos)
    {
        if (pos.getDeque() != this) {
            throw invalid_iterator();
        }
        Node* tmpNode = pos.getNode();
        Block* tmpBlock = pos.getBlock();
        pos++; /// move to the next node
        tmpNode->prevNode->nextNode = tmpNode->nextNode;
        tmpNode->nextNode->prevNode = tmpNode->prevNode;
        delete tmpNode;
        _size--;
        tmpBlock->_sizeOfBlock--;
        if (tmpBlock->nextBlock != tailBlock) {
            Merge(tmpBlock, tmpBlock->nextBlock, pos);
            return pos;
        } else if (tmpBlock->_sizeOfBlock == 0 && _size != 0) { /// the last Block
            pos = iterator(this, tmpBlock->prevBlock, tmpBlock->prevBlock->tailNode);
            tmpBlock->prevBlock->nextBlock = tmpBlock->nextBlock;
            tmpBlock->nextBlock->prevBlock = tmpBlock->prevBlock;
            return pos;
        } else { /// become an empty deque
            return pos;
        }
    }
    /**
         * adds an element to the end
         */
    void push_back(const T& value)
    {
        Block* tmpBlock = tailBlock->prevBlock;
        Node* tmp = new Node(value, tmpBlock->tailNode->prevNode, tmpBlock->tailNode);
        tmp->prevNode->nextNode = tmp;
        tmp->nextNode->prevNode = tmp;
        tmpBlock->_sizeOfBlock++;
        _size++;
        iterator tmpIter = iterator();
        Split(tmpBlock, tmpIter);
    }
    /**
         * removes the last element
         *     throw when the container is empty.
         */
    void pop_back()
    {
        if (_size == 0) {
            throw container_is_empty();
        }
        Block* tmpBlock = tailBlock->prevBlock;
        Node* tmp = tmpBlock->tailNode->prevNode;
        tmp->prevNode->nextNode = tmp->nextNode;
        tmp->nextNode->prevNode = tmp->prevNode;
        delete tmp;
        tmpBlock->_sizeOfBlock--;
        _size--;
        if (tmpBlock->_sizeOfBlock == 0 && _size != 0) { // block becomes empty after pop_back // _size != 0 means leaving one empty block
            tmpBlock->prevBlock->nextBlock = tmpBlock->nextBlock;
            tmpBlock->nextBlock->prevBlock = tmpBlock->prevBlock;
            delete tmpBlock;
        } else if (_size != 0) {
            if (tmpBlock->prevBlock != headBlock) {
                iterator tmpIter = iterator();
                Split(tmpBlock, tmpIter);
                Merge(tmpBlock->prevBlock, tmpBlock, tmpIter);
            }
        }
    }
    /**
         * inserts an element to the beginning.
         */
    void push_front(const T& value)
    {
        Block* tmpBlock = headBlock->nextBlock;
        Node* tmp = new Node(value, tmpBlock->headNode, tmpBlock->headNode->nextNode);
        tmp->prevNode->nextNode = tmp;
        tmp->nextNode->prevNode = tmp;
        tmpBlock->_sizeOfBlock++;
        _size++;
        iterator tmpIter = iterator();
        Split(tmpBlock, tmpIter);
    }
    /**
         * removes the first element.
         *     throw when the container is empty.
         */
    void pop_front()
    {
        if (_size == 0) {
            throw container_is_empty();
        }
        Block* tmpBlock = headBlock->nextBlock;
        Node* tmp = tmpBlock->headNode->nextNode;
        tmp->prevNode->nextNode = tmp->nextNode;
        tmp->nextNode->prevNode = tmp->prevNode;
        delete tmp;
        tmpBlock->_sizeOfBlock--;
        _size--;
        if (tmpBlock->_sizeOfBlock == 0 && _size != 0) {
            tmpBlock->nextBlock->prevBlock = tmpBlock->prevBlock;
            tmpBlock->prevBlock->nextBlock = tmpBlock->nextBlock;
            delete tmpBlock;
        } else if (_size != 0) {
            if (tmpBlock->nextBlock != tailBlock) {
                iterator tmpIter = iterator();
                Merge(tmpBlock, tmpBlock->nextBlock, tmpIter);
                Split(tmpBlock, tmpIter);
            }
        }
    }

    void debugComplexity()
    {
        std::cout << ':' << _size << '\n';
        Block* tmpBlock = headBlock->nextBlock;
        while (tmpBlock != tailBlock) {
            std::cout << tmpBlock->_sizeOfBlock << '\n';
            tmpBlock = tmpBlock->nextBlock;
        }
    }
};
}

#endif