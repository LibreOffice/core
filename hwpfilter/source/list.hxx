/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef list_hxx
#define list_hxx

/**
 * Re-implement a simple container: LinkedList + LinkedListIterator
 *
 * DO NOT USE EXCEPT FOR REPLACING THE ORIGINAL LinkedList/LinkedListIterator!
 * USE STL CONTAINERS FOR NEW CODE!
 *
 * The classes LinkedList and LinkedListIterator were originally
 * implemented in two files LinkedList.cxx/.h, whose license would not
 * allow re-distribution through OpenOffice.org. This file
 * re-implements the same functionality, based on the STL.
 */

#include <cstddef>
#include <vector>

template<class T>
class LinkedList
{
    typedef std::vector<T*> list_t;
    list_t maList;

public:
    /// construct list with one element (pItem) or no element (pItem == NULL)
    LinkedList( T* pItem = 0 );
    ~LinkedList();

    T* find( const int n );   /// return nth element in list
    T* first();         /// return first element in list
    T* last();          /// return last element in list

    int count() const;  /// return number of elements in list
    int empty() const;  /// determine whether list contains any elements

    /// insert pItem into list at position n; at end if n == -1; return count()
    int insert( T* pItem, int n = -1 );

    /// remove nth element from list
    T* remove( const int n );

    /// remove given element from list
    int remove( T* pItem );
};

/** iterator class for LinkedList<T>. Iterator may travel outside of
 * list using operator++/--, in which case current() must return
 * NULL. */
template<class T>
class LinkedListIterator
{
    // iterator state: reference list + position
    LinkedList<T>* mpList;
    int mnPosition;

public:
    /// construct list with single element
    LinkedListIterator( LinkedList<T>* pList = 0 );
    ~LinkedListIterator();

    T* current();               /// return current element, or NULL if invalid
    void set( const int n );    /// set iterator to position n

    void reset( );              /// set iterator to first element

    // bug-compatible with original LinkedList.h/cxx: Ignore parameter!
    void operator++( int );   /// advance iterator by one step (ignore n !!!)
    void operator--( int );   /// go one step backwards (ignore n !!!)
    void operator++();        /// advance iterator by one step
    void operator--();        /// go one step backwards

private:
    bool valid();
};




// IMPLEMENTATION

// (the implementation of template classes must be accessible to using
// code, hence this implementation is in the header.)


#include <algorithm>

// define assert based on SAL, so we do not introduce a tools dependency
#include <osl/diagnose.h>
#define ASSERT(x) OSL_ENSURE((x), " HWP FILTER: " #x)


template<class T>
LinkedList<T>::LinkedList( T* pItem )
{
    if( pItem != NULL )
        maList.push_back( pItem );
}

template<class T>
LinkedList<T>::~LinkedList()
{
}

template<class T>
T* LinkedList<T>::find( const int n )
{
    ASSERT( n >= 0  &&  n < static_cast<int>( maList.size() ) );
    return maList[n];
}

template<class T>
T* LinkedList<T>::first()
{
    return find( 0 );
}

template<class T>
T* LinkedList<T>::last()
{
    return find( count() - 1 );
}

template<class T>
int LinkedList<T>::count() const
{
    return static_cast<int>( maList.size() );
}

template<class T>
int LinkedList<T>::empty() const
{
    return count() == 0;
}

template<class T>
int LinkedList<T>::insert( T* pItem, int n )
{
    ASSERT( pItem != NULL );
    ASSERT( n >= -1  &&  n <= static_cast<int>( maList.size() ));

    if( n == -1 )
    {
        maList.push_back( pItem );
    }
    else
    {
        maList.insert( maList.begin() + n, pItem );
    }

    return static_cast<int>( maList.size() );
}

template<class T>
T* LinkedList<T>::remove( const int n )
{
    ASSERT( n >= -1  &&  n <= static_cast<int>( maList.size() ) );

    T* pItem = maList[ n ];
    ASSERT( pItem != NULL );

    maList.erase( maList.begin() + n );
    return pItem;
}

template<class T>
int LinkedList<T>::remove( T* pItem )
{
    ASSERT( pItem != NULL );

    int i = 0;
    typename list_t::iterator aIter = maList.begin();
    typename list_t::iterator aEnd = maList.end();
    while( aIter != aEnd  && *aIter != pItem )
    {
        ++i;
        ++aIter;
    }

    if( aIter != aEnd )
    {
        // found!
        ASSERT( *aIter == pItem );
        maList.erase( aIter );
    }
    else
    {
        // else: not found
        i = -1;
    }

    return i;
}



template<class T>
LinkedListIterator<T>::LinkedListIterator( LinkedList<T>* pList ) :
    mpList( pList ),
    mnPosition( 0 )
{
    ASSERT( pList != NULL );
}

template<class T>
LinkedListIterator<T>::~LinkedListIterator()
{
}

template<class T>
T* LinkedListIterator<T>::current()
{
    return valid() ? mpList->find( mnPosition ) : NULL;
}

template<class T>
void LinkedListIterator<T>::set( const int nIndex )
{
    ASSERT( mpList != NULL );
    mnPosition = nIndex;
    ASSERT( valid() );
}


template<class T>
void LinkedListIterator<T>::reset()
{
    ASSERT( mpList != NULL );
    mnPosition = 0;
    ASSERT( valid() );
}

template<class T>
void LinkedListIterator<T>::operator++( int )
{
    ASSERT( mpList != NULL );

    // bug-compatible with LinkedList.cxx: ignore parameter!
    mnPosition ++;
}

template<class T>
void LinkedListIterator<T>::operator--( int )
{
    ASSERT( mpList != NULL );

    // bug-compatible with LinkedList.cxx: ignore parameter!
    mnPosition --;
}

template<class T>
void LinkedListIterator<T>::operator++()
{
    ASSERT( mpList != NULL );
    mnPosition ++;
}

template<class T>
void LinkedListIterator<T>::operator--()
{
    ASSERT( mpList != NULL );
    mnPosition --;
}

template<class T>
bool LinkedListIterator<T>::valid()
{
    return mpList != NULL
        && mnPosition >= 0
        && mnPosition < mpList->count();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
