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

// forward declared because it's default tempate arg for sorted_vector
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
    : private std::vector<Value>
{
private:
    typedef Find<Value, Compare> Find_t;
    typedef typename std::vector<Value> base_t;
    typedef typename std::vector<Value>::iterator  iterator;
public:
    typedef typename std::vector<Value>::const_iterator const_iterator;
    typedef typename std::vector<Value>::size_type size_type;

    using base_t::clear;
    using base_t::empty;
    using base_t::size;

    // MODIFIERS

    std::pair<const_iterator,bool> insert( const Value& x )
    {
        std::pair<const_iterator, bool> const ret(Find_t()(begin(), end(), x));
        if (!ret.second)
        {
            const_iterator const it = base_t::insert(
                            begin_nonconst() + (ret.first - begin()), x);
            return std::make_pair(it, true);
        }
        return std::make_pair(ret.first, false);
    }

    size_type erase( const Value& x )
    {
        std::pair<const_iterator, bool> const ret(Find_t()(begin(), end(), x));
        if (ret.second)
        {
            base_t::erase(begin_nonconst() + (ret.first - begin()));
            return 1;
        }
        return 0;
    }

    void erase( size_t index )
    {
        base_t::erase( begin_nonconst() + index );
    }

    // like C++ 2011: erase with const_iterator (doesn't change sort order)
    void erase(const_iterator const& position)
    {   // C++98 has vector::erase(iterator), so call that
        base_t::erase(begin_nonconst() + (position - begin()));
    }

    void erase(const_iterator const& first, const_iterator const& last)
    {
        base_t::erase(begin_nonconst() + (first - begin()),
                      begin_nonconst() + (last  - begin()));
    }

    // ACCESSORS

    // Only return a const iterator, so that the vector cannot be directly updated.
    const_iterator begin() const
    {
        return base_t::begin();
    }

    // Only return a const iterator, so that the vector cannot be directly updated.
    const_iterator end() const
    {
        return base_t::end();
    }

    const Value& front() const
    {
        return base_t::front();
    }

    const Value& back() const
    {
        return base_t::back();
    }

    const Value& operator[]( size_t index ) const
    {
        return base_t::operator[]( index );
    }

    // OPERATIONS

    const_iterator lower_bound( const Value& x ) const
    {
        return std::lower_bound( base_t::begin(), base_t::end(), x, Compare() );
    }

    /* Searches the container for an element with a value of x
     * and returns an iterator to it if found, otherwise it returns an
     * iterator to sorted_vector::end (the element past the end of the container).
     *
     * Only return a const iterator, so that the vector cannot be directly updated.
     */
    const_iterator find( const Value& x ) const
    {
        std::pair<const_iterator, bool> const ret(Find_t()(begin(), end(), x));
        return (ret.second) ? ret.first : end();
    }

    void insert(sorted_vector<Value,Compare,Find> const& rOther)
    {
       // optimisation for the rather common case that we are overwriting this with the contents
       // of another sorted vector
       if ( empty() )
       {
           base_t::insert(begin_nonconst(), rOther.begin(), rOther.end());
       }
       else
           for( const_iterator it = rOther.begin(); it != rOther.end(); ++it )
               insert( *it );
    }

    /* Clear() elements in the vector, and free them one by one. */
    void DeleteAndDestroyAll()
    {
        for( const_iterator it = begin(); it != end(); ++it )
            delete *it;
        clear();
    }

private:

    typename base_t::iterator begin_nonconst() { return base_t::begin(); }
    typename base_t::iterator end_nonconst()   { return base_t::end(); }

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

/** the elments are partially ordered by Compare,
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
