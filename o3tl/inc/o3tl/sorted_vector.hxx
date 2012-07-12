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
public:
    typedef typename std::vector<Value>::iterator  iterator;
    typedef typename std::vector<Value>::const_iterator  const_iterator;
    typedef typename std::vector<Value>::size_type  size_type;
    typedef sorted_vector_compare<Value, Compare> MyCompare;

    using std::vector<Value>::begin;
    using std::vector<Value>::end;
    using std::vector<Value>::clear;
    using std::vector<Value>::erase;
    using std::vector<Value>::empty;
    using std::vector<Value>::size;
    using std::vector<Value>::operator[];

    // MODIFIERS

    std::pair<iterator,bool> insert( const Value& x )
    {
        const MyCompare& me = *this;
        iterator it = std::lower_bound( begin(), end(), x, me );
        if( it == end() || less_than(x, *it) )
        {
            it = std::vector<Value>::insert( it, x );
            return std::make_pair( it, true );
        }
        return std::make_pair( it, false );
    }

    size_type erase( const Value& x )
    {
        iterator it = find(x);
        if( it != end() )
        {
            erase( it );
            return 1;
        }
        return 0;
    }

    // OPERATIONS

    /* Searches the container for an element with a value of x
     * and returns an iterator to it if found, otherwise it returns an
     * iterator to sorted_vector::end (the element past the end of the container).
     */
    const_iterator find( const Value& x ) const
    {
        const MyCompare& me = *this;
        const_iterator it = std::lower_bound( begin(), end(), x, me );
        if( it == end() || less_than(x, *it) )
        {
            return end();
        }
        return it;
    }
    iterator find( const Value& x )
    {
        const MyCompare& me = *this;
        iterator it = std::lower_bound( begin(), end(), x, me );
        if( it == end() || less_than(x, *it) )
        {
            return end();
        }
        return it;
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
