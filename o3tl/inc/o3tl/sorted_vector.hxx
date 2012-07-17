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

/** Helper template */
template <class Value, class Compare>
class sorted_vector_compare : public Compare
{
public:
    bool operator()(const Value& lhs, const Value& rhs) const
    {
        return Compare::operator()(lhs, rhs);
    }
};

/** Represents a sorted vector of values.

    @tpl Value class of item to be stored in container
    @tpl Compare comparison method
*/
template <class Value, class Compare = std::less<Value> >
class sorted_vector
    : private std::vector<Value>
    , private sorted_vector_compare<Value, Compare>
{
private:
    typedef typename std::vector<Value> base_t;
    typedef typename std::vector<Value>::iterator  iterator;
public:
    typedef typename std::vector<Value>::const_iterator const_iterator;
    typedef typename std::vector<Value>::size_type size_type;
    typedef sorted_vector_compare<Value, Compare> MyCompare;

    using base_t::clear;
    using base_t::erase;
    using base_t::empty;
    using base_t::size;

    // MODIFIERS

    std::pair<const_iterator,bool> insert( const Value& x )
    {
        iterator it = lower_bound_nonconst( x );
        if (it == base_t::end() || less_than(x, *it))
        {
            it = base_t::insert( it, x );
            return std::make_pair( it, true );
        }
        return std::make_pair( it, false );
    }

    size_type erase( const Value& x )
    {
        iterator it = lower_bound_nonconst( x );
        if (it != base_t::end() && !less_than(x, *it))
        {
            erase( it );
            return 1;
        }
        return 0;
    }

    void erase( size_t index )
    {
        base_t::erase( begin_nonconst() + index );
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
        const MyCompare& me = *this;
        return std::lower_bound( base_t::begin(), base_t::end(), x, me );
    }

    /* Searches the container for an element with a value of x
     * and returns an iterator to it if found, otherwise it returns an
     * iterator to sorted_vector::end (the element past the end of the container).
     *
     * Only return a const iterator, so that the vector cannot be directly updated.
     */
    const_iterator find( const Value& x ) const
    {
        const_iterator it = lower_bound( x );
        if (it == base_t::end() || less_than(x, *it))
        {
            return base_t::end();
        }
        return it;
    }

    void insert( sorted_vector<Value,Compare> &rOther )
    {
       // optimisation for the rather common case that we are overwriting this with the contents
       // of another sorted vector
       if ( empty() )
       {
           base_t::insert( begin_nonconst(),
                   rOther.begin_nonconst(), rOther.end_nonconst() );
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
    /** just makes the code easier to read */
    bool less_than(const Value& lhs, const Value& rhs) const
    {
        const MyCompare& me = *this;
        return me.operator()(lhs, rhs);
    }

    iterator lower_bound_nonconst( const Value& x )
    {
        const MyCompare& me = *this;
        return std::lower_bound( base_t::begin(), base_t::end(), x, me );
    }

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


}   // namespace o3tl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
