/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_SORTED_VECTOR_HXX
#define INCLUDED_O3TL_SORTED_VECTOR_HXX

#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

namespace o3tl
{

// forward declared because it's default template arg for sorted_vector
template<class Value, class Compare>
struct find_unique;

/** Represents a sorted vector of values.

    @tpl Value class of item to be stored in container
    @tpl Compare comparison method
    @tpl Find   look up index of a Value in the array
*/
template<
     typename Value,
     typename Compare = std::less<Value>,
     template<typename, typename> class Find = find_unique,
     bool = std::is_copy_constructible<Value>::value >
class sorted_vector
{
private:
    typedef Find<Value, Compare> Find_t;
    typedef typename std::vector<Value> vector_t;
    typedef typename std::vector<Value>::iterator  iterator;
public:
    typedef typename std::vector<Value>::const_iterator const_iterator;
    typedef typename std::vector<Value>::const_reverse_iterator const_reverse_iterator;
    typedef typename std::vector<Value>::difference_type difference_type;
    typedef typename std::vector<Value>::size_type size_type;
    typedef Value value_type;

    constexpr sorted_vector( std::initializer_list<Value> init )
        : m_vector(init)
    {
        std::sort(m_vector.begin(), m_vector.end(), Compare());
    }
    sorted_vector() = default;
    sorted_vector(sorted_vector const&) = default;
    sorted_vector(sorted_vector&&) = default;

    sorted_vector& operator=(sorted_vector const&) = default;
    sorted_vector& operator=(sorted_vector&&) = default;

    // MODIFIERS

    std::pair<const_iterator,bool> insert( Value&& x )
    {
        std::pair<const_iterator, bool> const ret(Find_t()(m_vector.begin(), m_vector.end(), x));
        if (!ret.second)
        {
            const_iterator const it = m_vector.insert(m_vector.begin() + (ret.first - m_vector.begin()), std::move(x));
            return std::make_pair(it, true);
        }
        return std::make_pair(ret.first, false);
    }

    std::pair<const_iterator,bool> insert( const Value& x )
    {
        std::pair<const_iterator, bool> const ret(Find_t()(m_vector.begin(), m_vector.end(), x));
        if (!ret.second)
        {
            const_iterator const it = m_vector.insert(m_vector.begin() + (ret.first - m_vector.begin()), x);
            return std::make_pair(it, true);
        }
        return std::make_pair(ret.first, false);
    }

    size_type erase( const Value& x )
    {
        std::pair<const_iterator, bool> const ret(Find_t()(m_vector.begin(), m_vector.end(), x));
        if (ret.second)
        {
            m_vector.erase(m_vector.begin() + (ret.first - m_vector.begin()));
            return 1;
        }
        return 0;
    }

    void erase_at(size_t index)
    {
        m_vector.erase(m_vector.begin() + index);
    }

    // like C++ 2011: erase with const_iterator (doesn't change sort order)
    const_iterator erase(const_iterator const& position)
    {   // C++98 has vector::erase(iterator), so call that
        return m_vector.erase(m_vector.begin() + (position - m_vector.begin()));
    }

    void erase(const_iterator const& first, const_iterator const& last)
    {
        m_vector.erase(m_vector.begin() + (first - m_vector.begin()),
                       m_vector.begin() + (last - m_vector.begin()));
    }

    /**
     * make erase return the removed element, otherwise there is no useful way of extracting a std::unique_ptr
     * from this.
     */
    Value erase_extract( size_t index )
    {
        Value val = std::move(m_vector[index]);
        m_vector.erase(m_vector.begin() + index);
        return val;
    }

    void clear()
    {
        m_vector.clear();
    }

    void swap(sorted_vector & other)
    {
        m_vector.swap(other.m_vector);
    }

    void reserve(size_type amount)
    {
        m_vector.reserve(amount);
    }

    // ACCESSORS

    size_type size() const
    {
        return m_vector.size();
    }

    bool empty() const
    {
        return m_vector.empty();
    }

    // Only return a const iterator, so that the vector cannot be directly updated.
    const_iterator begin() const
    {
        return m_vector.begin();
    }

    // Only return a const iterator, so that the vector cannot be directly updated.
    const_iterator end() const
    {
        return m_vector.end();
    }

    // Only return a const iterator, so that the vector cannot be directly updated.
    const_reverse_iterator rbegin() const
    {
        return m_vector.rbegin();
    }

    // Only return a const iterator, so that the vector cannot be directly updated.
    const_reverse_iterator rend() const
    {
        return m_vector.rend();
    }

    const Value& front() const
    {
        return m_vector.front();
    }

    const Value& back() const
    {
        return m_vector.back();
    }

    const Value& operator[]( size_t index ) const
    {
        return m_vector.operator[]( index );
    }

    // OPERATIONS

    const_iterator lower_bound( const Value& x ) const
    {
        return std::lower_bound( m_vector.begin(), m_vector.end(), x, Compare() );
    }

    const_iterator upper_bound( const Value& x ) const
    {
        return std::upper_bound( m_vector.begin(), m_vector.end(), x, Compare() );
    }

    /* Searches the container for an element with a value of x
     * and returns an iterator to it if found, otherwise it returns an
     * iterator to sorted_vector::end (the element past the end of the container).
     *
     * Only return a const iterator, so that the vector cannot be directly updated.
     */
    const_iterator find( const Value& x ) const
    {
        std::pair<const_iterator, bool> const ret(Find_t()(m_vector.begin(), m_vector.end(), x));
        return (ret.second) ? ret.first : m_vector.end();
    }

    size_type count(const Value& v) const
    {
        return find(v) != end() ? 1 : 0;
    }

    bool operator==(const sorted_vector & other) const
    {
        return m_vector == other.m_vector;
    }

    bool operator!=(const sorted_vector & other) const
    {
        return m_vector != other.m_vector;
    }

    void insert(sorted_vector<Value,Compare,Find> const& rOther)
    {
       // optimization for the rather common case that we are overwriting this with the contents
       // of another sorted vector
       if ( empty() )
       {
           m_vector.insert(m_vector.begin(), rOther.m_vector.begin(), rOther.m_vector.end());
       }
       else
       {
           for (const_iterator it = rOther.m_vector.begin(); it != rOther.m_vector.end(); ++it)
           {
               insert(*it);
           }
       }
    }

    /* Clear() elements in the vector, and free them one by one. */
    void DeleteAndDestroyAll()
    {
        for (const_iterator it = m_vector.begin(); it != m_vector.end(); ++it)
        {
            delete *it;
        }

        clear();
    }

    // fdo#58793: some existing code in Writer (SwpHintsArray)
    // routinely modifies the members of the vector in a way that
    // violates the sort order, and then re-sorts the array.
    // This is a kludge to enable that code to work.
    // If you are calling this function, you are Doing It Wrong!
    void Resort()
    {
        std::stable_sort(m_vector.begin(), m_vector.end(), Compare());
    }

private:

    vector_t m_vector;
};

/* Specialise the template for cases like Value = std::unique_ptr<T>, where
   MSVC2017 needs some help
*/
template<
     typename Value,
     typename Compare,
     template<typename, typename> class Find >
class sorted_vector<Value,Compare,Find,false> : public sorted_vector<Value, Compare, Find, true>
{
public:
    using sorted_vector<Value, Compare, Find, true>::sorted_vector;
    typedef sorted_vector<Value, Compare, Find, true> super_sorted_vector;

    sorted_vector(sorted_vector const&) = delete;
    sorted_vector& operator=(sorted_vector const&) = delete;

    sorted_vector() = default;
    sorted_vector(sorted_vector&&) = default;
    sorted_vector& operator=(sorted_vector&&) = default;

    /**
     * implement find for sorted_vectors containing std::unique_ptr
     */
    typename super_sorted_vector::const_iterator find( typename Value::element_type const * x ) const
    {
        Value tmp(const_cast<typename Value::element_type*>(x));
        auto ret = super_sorted_vector::find(tmp);
        tmp.release();
        return ret;
    }
    /**
     * implement upper_bound for sorted_vectors containing std::unique_ptr
     */
    typename super_sorted_vector::const_iterator upper_bound( typename Value::element_type const * x ) const
    {
        Value tmp(const_cast<typename Value::element_type*>(x));
        auto ret = super_sorted_vector::upper_bound(tmp);
        tmp.release();
        return ret;
    }
    /**
     * implement lower_bound for sorted_vectors containing std::unique_ptr
     */
    typename super_sorted_vector::const_iterator lower_bound( typename Value::element_type const * x ) const
    {
        Value tmp(const_cast<typename Value::element_type*>(x));
        auto ret = super_sorted_vector::lower_bound(tmp);
        tmp.release();
        return ret;
    }
};


/** Implements an ordering function over a pointer, where the comparison uses the < operator on the pointed-to types.
    Very useful for the cases where we put pointers to objects inside a sorted_vector.
*/
template <class T> struct less_ptr_to
{
    bool operator() ( T* const& lhs, T* const& rhs ) const
    {
        return (*lhs) < (*rhs);
    }
};

template <class T> struct less_uniqueptr_to
{
    bool operator() ( std::unique_ptr<T> const& lhs, std::unique_ptr<T> const& rhs ) const
    {
        return (*lhs) < (*rhs);
    }
};

/** the elements are totally ordered by Compare,
    for no 2 elements !Compare(a,b) && !Compare(b,a) is true
  */
template<class Value, class Compare>
struct find_unique
{
    typedef typename sorted_vector<Value, Compare,
            o3tl::find_unique> ::const_iterator const_iterator;
    std::pair<const_iterator, bool> operator()(
            const_iterator first, const_iterator last,
            Value const& v)
    {
        const_iterator const it = std::lower_bound(first, last, v, Compare());
        return std::make_pair(it, (it != last && !Compare()(v, *it)));
    }
};

/** the elements are partially ordered by Compare,
    2 elements are allowed if they are not the same element (pointer equal)
  */
template<class Value, class Compare>
struct find_partialorder_ptrequals
{
    typedef typename sorted_vector<Value, Compare,
            o3tl::find_partialorder_ptrequals>::const_iterator const_iterator;
    std::pair<const_iterator, bool> operator()(
            const_iterator first, const_iterator last,
            Value const& v)
    {
        std::pair<const_iterator, const_iterator> const its =
            std::equal_range(first, last, v, Compare());
        for (const_iterator it = its.first; it != its.second; ++it)
        {
            if (v == *it)
            {
                return std::make_pair(it, true);
            }
        }
        return std::make_pair(its.first, false);
    }
};

}   // namespace o3tl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
