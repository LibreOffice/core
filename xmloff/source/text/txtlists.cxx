/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: txtlists.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_xmloff.hxx"

#include <txtlists.hxx>

#include <tools/debug.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>

XMLTextListsHelper::XMLTextListsHelper()
    : mpProcessedLists( 0 ),
      msLastProcessedListId(),
      msListStyleOfLastProcessedList(),
      mpContinuingLists( 0 ),
      mpListStack( 0 )
{
}

XMLTextListsHelper::~XMLTextListsHelper()
{
    if ( mpProcessedLists )
    {
        mpProcessedLists->clear();
        delete mpProcessedLists;
    }
    if ( mpContinuingLists )
    {
        mpContinuingLists->clear();
        delete mpContinuingLists;
    }
    if ( mpListStack )
    {
        mpListStack->clear();
        delete mpListStack;
    }
}

void XMLTextListsHelper::KeepListAsProcessed( ::rtl::OUString sListId,
                                              ::rtl::OUString sListStyleName,
                                              ::rtl::OUString sContinueListId )
{
    if ( IsListProcessed( sListId ) )
    {
        DBG_ASSERT( false,
                    "<XMLTextListsHelper::AddList(..)> - list id already added" );
        return;
    }

    if ( mpProcessedLists == 0 )
    {
        mpProcessedLists = new tMapForLists();
    }

    ::std::pair< ::rtl::OUString, ::rtl::OUString >
                                aListData( sListStyleName, sContinueListId );
    (*mpProcessedLists)[ sListId ] = aListData;

    msLastProcessedListId = sListId;
    msListStyleOfLastProcessedList = sListStyleName;
}

sal_Bool XMLTextListsHelper::IsListProcessed( const ::rtl::OUString sListId ) const
{
    if ( mpProcessedLists == 0 )
    {
        return sal_False;
    }

    return mpProcessedLists->find( sListId ) != mpProcessedLists->end();
}

::rtl::OUString XMLTextListsHelper::GetListStyleOfProcessedList(
                                            const ::rtl::OUString sListId ) const
{
    if ( mpProcessedLists != 0 )
    {
        tMapForLists::const_iterator aIter = mpProcessedLists->find( sListId );
        if ( aIter != mpProcessedLists->end() )
        {
            return (*aIter).second.first;
        }
    }

    return ::rtl::OUString();
}

::rtl::OUString XMLTextListsHelper::GetContinueListIdOfProcessedList(
                                            const ::rtl::OUString sListId ) const
{
    if ( mpProcessedLists != 0 )
    {
        tMapForLists::const_iterator aIter = mpProcessedLists->find( sListId );
        if ( aIter != mpProcessedLists->end() )
        {
            return (*aIter).second.second;
        }
    }

    return ::rtl::OUString();
}

const ::rtl::OUString& XMLTextListsHelper::GetLastProcessedListId() const
{
    return msLastProcessedListId;
}

const ::rtl::OUString& XMLTextListsHelper::GetListStyleOfLastProcessedList() const
{
    return msListStyleOfLastProcessedList;
}

::rtl::OUString XMLTextListsHelper::GenerateNewListId() const
{
    // --> OD 2008-08-06 #i92478#
    ::rtl::OUString sTmpStr( ::rtl::OUString::createFromAscii( "list" ) );
    // <--
    sal_Int64 n = Time().GetTime();
    n += Date().GetDate();
    n += rand();
    // --> OD 2008-08-06 #i92478#
    sTmpStr += ::rtl::OUString::valueOf( n );
    // <--

    long nHitCount = 0;
    ::rtl::OUString sNewListId( sTmpStr );
    if ( mpProcessedLists != 0 )
    {
        while ( mpProcessedLists->find( sNewListId ) != mpProcessedLists->end() )
        {
            ++nHitCount;
            sNewListId = sTmpStr;
            sNewListId += ::rtl::OUString::valueOf( nHitCount );
        }
    }

    return sNewListId;
}

void XMLTextListsHelper::StoreLastContinuingList( ::rtl::OUString sListId,
                                                  ::rtl::OUString sContinuingListId )
{
    if ( mpContinuingLists == 0 )
    {
        mpContinuingLists = new tMapForContinuingLists();
    }

    (*mpContinuingLists)[ sListId ] = sContinuingListId;
}

::rtl::OUString XMLTextListsHelper::GetLastContinuingListId(
                                                ::rtl::OUString sListId ) const
{
    if ( mpContinuingLists != 0)
    {
        tMapForContinuingLists::const_iterator aIter =
                                                mpContinuingLists->find( sListId );
        if ( aIter != mpContinuingLists->end() )
        {
            return (*aIter).second;
        }
    }

    return sListId;
}

void XMLTextListsHelper::PushListOnStack( ::rtl::OUString sListId,
                                          ::rtl::OUString sListStyleName )
{
    if ( mpListStack == 0 )
    {
        mpListStack = new tStackForLists();
    }
    ::std::pair< ::rtl::OUString, ::rtl::OUString >
                                aListData( sListId, sListStyleName );
    mpListStack->push_back( aListData );
}
void XMLTextListsHelper::PopListFromStack()
{
    if ( mpListStack != 0 &&
         mpListStack->size() > 0 )
    {
        mpListStack->pop_back();
    }
}

sal_Bool XMLTextListsHelper::EqualsToTopListStyleOnStack( const ::rtl::OUString sListId ) const
{
    return mpListStack != 0
           ? sListId == mpListStack->back().second
           : sal_False;
}
