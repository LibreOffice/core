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

#ifndef INCLUDED_HWPFILTER_SOURCE_LIST_HXX
#define INCLUDED_HWPFILTER_SOURCE_LIST_HXX

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

    // bug-compatible with original LinkedList.h/cxx: Ignore parameter!
    void operator++( int );   /// advance iterator by one step (ignore n !!!)
    void operator--( int );   /// go one step backwards (ignore n !!!)
    void operator++();        /// advance iterator by one step
    void operator--();        /// go one step backwards

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
