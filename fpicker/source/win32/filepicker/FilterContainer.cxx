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

#include <stdexcept>
#include <osl/diagnose.h>
#include "FilterContainer.hxx"

#include <utility>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//-------------------------------------------------------------------
// namespace directives
//-------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// ctor
//-------------------------------------------------------------------------------------

CFilterContainer::CFilterContainer( sal_Int32 initSize ) :
    m_vFilters( initSize ),
    m_bIterInitialized( sal_False )
{
}

//-----------------------------------------------------------------------------------------
// add a name/filter pair
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilterContainer::addFilter(
    const OUString& aName, const OUString& aFilter, sal_Bool bAllowDuplicates )
{
    // check if the filter is already in the container
    sal_Int32 pos = -1;

    if ( !bAllowDuplicates )
    {
        pos = getFilterTagPos( aName );
        if ( pos < 0 ) // if not there, append
        {
            m_vFilters.push_back( std::make_pair( aName, aFilter ) );
            m_bIterInitialized = sal_False;
        }
    }
    else
    {
        m_vFilters.push_back( std::make_pair( aName, aFilter ) );
        m_bIterInitialized = sal_False;
    }

    return ( pos < 0 ) ? sal_True : sal_False;
}

//-----------------------------------------------------------------------------------------
// delete a filter
// Precondition: the container is not empty
//               there is a filter identified by the given name
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilterContainer::delFilter( const OUString& aName )
{
    OSL_ASSERT( !m_vFilters.empty() );

    sal_Int32 pos = getFilterTagPos( aName );
    if ( pos > -1 )
    {
        m_vFilters.erase( ( m_vFilters.begin() + pos ) );
        m_bIterInitialized = sal_False;
    }

    return ( pos > -1 ) ? sal_True : sal_False;
}

//-----------------------------------------------------------------------------------------
// return the number of filters currently in the container
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilterContainer::numFilter( )
{
    return m_vFilters.size( );
}

//-----------------------------------------------------------------------------------------
// clear all entries
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilterContainer::empty()
{
    m_vFilters.clear( );
}

//-----------------------------------------------------------------------------------------
// get a filter by name
// Precondition: the container is not empty
//               there is a filter identified by the name
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilterContainer::getFilter( const OUString& aName, OUString& theFilter ) const
{
    OSL_PRECOND( !m_vFilters.empty() , "Empty filter container" );

    sal_Int32 pos = getFilterTagPos( aName );

    try
    {
        if ( pos > -1 )
            theFilter = m_vFilters.at( pos ).second;
    }
    catch( std::out_of_range& )
    {
        OSL_FAIL( "Filter not in filter container" );
        pos = -1;
    }

    return (pos > -1 ) ? sal_True : sal_False;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilterContainer::getFilter( sal_Int32 aIndex, OUString& theFilter ) const
{
    sal_Bool bRet = sal_True;

    try
    {
        theFilter = m_vFilters.at( aIndex ).first;
    }
    catch( std::out_of_range& )
    {
        OSL_FAIL( "Filter index out of range" );
        bRet = sal_False;
    }

    return bRet;
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilterContainer::getFilterPos( const OUString& aName ) const
{
    return getFilterTagPos( aName );
}

//-----------------------------------------------------------------------------------------
// returns the index of the filter identified by name
//-----------------------------------------------------------------------------------------

sal_Int32 SAL_CALL CFilterContainer::getFilterTagPos( const OUString& aName ) const
{
    if ( !m_vFilters.empty() )
    {
        sal_Int32 i = 0;
        FILTER_VECTOR_T::const_iterator iter;
        FILTER_VECTOR_T::const_iterator iter_end = m_vFilters.end( );

        for ( iter = m_vFilters.begin( ); iter != iter_end; ++iter, ++i )
            if ( ( *iter ).first.equalsIgnoreAsciiCase( aName ) )
                return i;
    }

    return -1;
}

//-----------------------------------------------------------------------------------------
// starts enumerating the filter in the container
//-----------------------------------------------------------------------------------------

void SAL_CALL CFilterContainer::beginEnumFilter( )
{
    m_iter = m_vFilters.begin( );
    m_bIterInitialized = sal_True;
}

//-----------------------------------------------------------------------------------------
// returns true if another filter has been retrieved
//-----------------------------------------------------------------------------------------

sal_Bool SAL_CALL CFilterContainer::getNextFilter( FILTER_ENTRY_T& nextFilterEntry )
{
    OSL_ASSERT( m_bIterInitialized );

    sal_Bool bRet = ( m_iter != m_vFilters.end( ) );

    if ( bRet )
        nextFilterEntry = *m_iter++;
    else
        m_bIterInitialized = sal_False;

    return bRet;
}

//-----------------------------------------------------------------------------------------
void SAL_CALL CFilterContainer::setCurrentFilter( const OUString& aName )
{
    m_sCurrentFilter = aName;
}

//-----------------------------------------------------------------------------------------
OUString SAL_CALL CFilterContainer::getCurrentFilter() const
{
    return m_sCurrentFilter;
}

//###################################################################


//-------------------------------------------------------------------
// calculates the length of a '\0' separated filter, that means
// length of the name + '\0' + length of the filter string +
// a trailing '\0'
//-------------------------------------------------------------------

static sal_uInt32 _getLengthFilter( CFilterContainer::FILTER_ENTRY_T aFilterEntry )
{
    return (
        aFilterEntry.first.getLength( )  + 1 +
        aFilterEntry.second.getLength( ) + 1 );
}

//-------------------------------------------------------------------
// calculates the length of all filters currently in the container
//-------------------------------------------------------------------

static sal_uInt32 _getTotalFilterLength( CFilterContainer& aFilterContainer )
{
    CFilterContainer::FILTER_ENTRY_T nextFilter;

    aFilterContainer.beginEnumFilter( );

    sal_uInt32 totalLength = 0;
    while( aFilterContainer.getNextFilter( nextFilter ) )
        totalLength += _getLengthFilter( nextFilter );

    return ( totalLength > 0 ) ? totalLength + 1 : totalLength;
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

inline
void _wcsmemcpy( sal_Unicode* pDest, const sal_Unicode* pSrc, sal_uInt32 nLength )
{
    memcpy( pDest, pSrc, nLength * sizeof( sal_Unicode ) );
}

//-------------------------------------------------------------------
// a helper trivial helper function to create a filter buffer in the
// format the Win32 API requires,
// e.g. "Text\0*.txt\0Doc\0*.doc;*xls\0\0"
//-------------------------------------------------------------------

OUString SAL_CALL makeWinFilterBuffer( CFilterContainer& aFilterContainer )
{
    // calculate the required buffer size
    sal_uInt32 reqBuffSize = _getTotalFilterLength( aFilterContainer );

    // return if there are no filters
    if ( !reqBuffSize )
        return OUString( );

    sal_Unicode* pBuff = new sal_Unicode[reqBuffSize];

    // initialize the buffer with 0
    ZeroMemory( pBuff, sizeof( sal_Unicode ) * reqBuffSize );

    OUString winFilterBuff;
    CFilterContainer::FILTER_ENTRY_T nextFilter;
    sal_uInt32 memPos = 0;

    aFilterContainer.beginEnumFilter( );

    while( aFilterContainer.getNextFilter( nextFilter ) )
    {
        _wcsmemcpy(
            pBuff + memPos,
            nextFilter.first.getStr( ),
            nextFilter.first.getLength( ) );

        memPos += nextFilter.first.getLength( ) + 1;

        _wcsmemcpy(
            pBuff + memPos,
            nextFilter.second.getStr( ),
            nextFilter.second.getLength( ) );

        memPos += nextFilter.second.getLength( ) + 1 ;
    }

    winFilterBuff = OUString( pBuff, reqBuffSize );

    // remove the allocated buffer
    delete [] pBuff;

    return winFilterBuff;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
