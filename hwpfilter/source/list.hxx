/*************************************************************************
 *
 *  $RCSfile: list.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:46:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef list_hxx
#define list_hxx

/**
 * Re-implement a simple container: LinkedList + LinkedListIterator
 *
 * DO NOT USE EXCEPT FOR REPLACING THE ORIGINAL LinkedList/LinkedListIterator!
 * USE STL CONTAINERS FOR NEW CODE!
 *
 * The classes LinkedList and LinkedListIterator were originally
 * implemented in two files LinkedList.cpp/.h, whose license would not
 * allow re-distribution through OpenOffice.org. This file
 * re-implements the same functionality, based on the STL.
 */

#include <vector>

template<class T>
class LinkedList
{
    typedef std::vector<T*> list_t;
    list_t maList;

public:
    /// construct list with one element (pItem) or no element (pItem == NULL)
    LinkedList( T* pItem = NULL );
    ~LinkedList();

    T* find( const int n );   /// return nth element in list
    T* first();         /// return first element in list
    T* last();          /// return last element in list

    const int count() const;  /// return number of elements in list
    const int empty() const;  /// determine whether list contains any elements

    /// insert pItem into list at position n; at end if n == -1; return count()
    const int insert( T* pItem, int n = -1 );

    /// remove nth element from list
    T* remove( const int n );

    /// remove given element from list
    const int remove( T* pItem );
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
    LinkedListIterator( LinkedList<T>* pList = NULL );
    ~LinkedListIterator();

    T* current();               /// return current element, or NULL if invalid
    const int set( const int n ); /// set iterator to position n

    void reset( );              /// set iterator to first element
    void resetLast();           /// set iterator to last element

    // bug-compatible with original LinkedList.h/cpp: Ignore parameter!
    void operator++( int );   /// advance iterator by one step (ignore n !!!)
    void operator--( int );   /// go one step backwards (ignore n !!!)

private:
    bool valid();
};



//
// IMPLEMENTATION
//
// (the implementation of template classes must be accessible to using
// code, hence this implementation is in the header.)
//

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
const int LinkedList<T>::count() const
{
    return static_cast<int>( maList.size() );
}

template<class T>
const int LinkedList<T>::empty() const
{
    return count() == 0;
}

template<class T>
const int LinkedList<T>::insert( T* pItem, int n )
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
const int LinkedList<T>::remove( T* pItem )
{
    ASSERT( pItem != NULL );

    int i = 0;
    typename list_t::iterator aIter = maList.begin();
    typename list_t::iterator aEnd = maList.end();
    while( aIter != aEnd  && *aIter != pItem )
    {
        i++;
        aIter++;
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
const int LinkedListIterator<T>::set( const int nIndex )
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
void LinkedListIterator<T>::resetLast()
{
    ASSERT( mpList != NULL );
    mnPosition = mpList.size() - 1;
    ASSERT( valid() );
}

template<class T>
void LinkedListIterator<T>::operator++( int )
{
    ASSERT( mpList != NULL );

    // bug-compatible with LinkedList.cpp: ignore parameter!
    mnPosition ++;
}

template<class T>
void LinkedListIterator<T>::operator--( int )
{
    ASSERT( mpList != NULL );

    // bug-compatible with LinkedList.cpp: ignore parameter!
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
