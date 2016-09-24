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
#include <functional>
#include <algorithm>

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
template<typename Value, typename Compare = std::less<Value>,
     template<typename, typename> class Find = find_unique >
class sorted_vector
{
private:
    typedef Find<Value, Compare> Find_t;
    typedef typename std::vector<Value> vector_t;
    typedef typename std::vector<Value>::iterator  iterator;
public:
    typedef typename std::vector<Value>::const_iterator const_iterator;
    typedef typename std::vector<Value>::size_type size_type;

    // MODIFIERS

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

    void erase( size_t index )
    {
        m_vector.erase(m_vector.begin() + index);
    }

    // like C++ 2011: erase with const_iterator (doesn't change sort order)
    void erase(const_iterator const& position)
    {   // C++98 has vector::erase(iterator), so call that
        m_vector.erase(m_vector.begin() + (position - m_vector.begin()));
    }

    void erase(const_iterator const& first, const_iterator const& last)
    {
        m_vector.erase(m_vector.begin() + (first - m_vector.begin()),
                       m_vector.begin() + (last - m_vector.begin()));
    }

    void clear()
    {
        m_vector.clear();
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


/** Implements an ordering function over a pointer, where the comparison uses the < operator on the pointed-to types.
    Very useful for the cases where we put pointers to objects inside a sorted_vector.
*/
template <class T> struct less_ptr_to : public std::binary_function <T*,T*,bool>
{
    bool operator() ( T* const& lhs, T* const& rhs ) const
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
