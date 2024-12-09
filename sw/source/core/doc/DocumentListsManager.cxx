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
#include <DocumentListsManager.hxx>
#include <doc.hxx>
#include <list.hxx>
#include <numrule.hxx>

#include <comphelper/random.hxx>
#include <osl/diagnose.h>


namespace sw
{

DocumentListsManager::DocumentListsManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc )
{
}

SwList* DocumentListsManager::createList( const OUString& rListId,
                           const UIName& sDefaultListStyleName )
{
    OUString sListId = rListId;
    if ( sListId.isEmpty() )
    {
        sListId = CreateUniqueListId();
    }

    if ( getListByName( sListId ) )
    {
        OSL_FAIL( "<DocumentListsManager::createList(..)> - provided list id already used. Serious defect." );
        return nullptr;
    }

    SwNumRule* pDefaultNumRuleForNewList = m_rDoc.FindNumRulePtr( sDefaultListStyleName );
    if ( !pDefaultNumRuleForNewList )
    {
        OSL_FAIL( "<DocumentListsManager::createList(..)> - for provided default list style name no list style is found. Serious defect." );
        return nullptr;
    }

    SwList* pNewList = new SwList( sListId, *pDefaultNumRuleForNewList, m_rDoc.GetNodes() );
    maLists[sListId].reset(pNewList);

    return pNewList;
}

SwList* DocumentListsManager::getListByName( const OUString& sListId ) const
{
    SwList* pList = nullptr;

    auto aListIter = maLists.find( sListId );
    if ( aListIter != maLists.end() )
    {
        pList = (*aListIter).second.get();
    }

    return pList;
}

void DocumentListsManager::createListForListStyle( const UIName& sListStyleName )
{
    if ( sListStyleName.isEmpty() )
    {
        OSL_FAIL( "<DocumentListsManager::createListForListStyle(..)> - no list style name provided. Serious defect." );
        return;
    }

    if ( getListForListStyle( sListStyleName ) )
    {
        OSL_FAIL( "<DocumentListsManager::createListForListStyle(..)> - a list for the provided list style name already exists. Serious defect." );
        return;
    }

    SwNumRule* pNumRule = m_rDoc.FindNumRulePtr( sListStyleName );
    if ( !pNumRule )
    {
        OSL_FAIL( "<DocumentListsManager::createListForListStyle(..)> - for provided list style name no list style is found. Serious defect." );
        return;
    }

    OUString sListId( pNumRule->GetDefaultListId() ); // can be empty String
    if ( getListByName( sListId ) )
    {
        sListId.clear();
    }
    SwList* pNewList = createList( sListId, sListStyleName );
    maListStyleLists[sListStyleName] = pNewList;
    pNumRule->SetDefaultListId( pNewList->GetListId() );
}

SwList* DocumentListsManager::getListForListStyle( const UIName& sListStyleName ) const
{
    SwList* pList = nullptr;

    auto aListIter = maListStyleLists.find( sListStyleName );
    if ( aListIter != maListStyleLists.end() )
    {
        pList = (*aListIter).second;
    }

    return pList;
}

void DocumentListsManager::deleteListForListStyle( const UIName& sListStyleName )
{
    OUString sListId;
    {
        SwList* pList = getListForListStyle( sListStyleName );
        OSL_ENSURE( pList,
                "<DocumentListsManager::deleteListForListStyle(..)> - misusage of method: no list found for given list style name" );
        if ( pList )
        {
            sListId = pList->GetListId();
        }
    }
    if ( !sListId.isEmpty() )
    {
        maListStyleLists.erase( sListStyleName );
        maLists.erase( sListId );
    }
}

void DocumentListsManager::deleteListsByDefaultListStyle( const UIName& rListStyleName )
{
    auto aListIter = maLists.begin();
    while ( aListIter != maLists.end() )
    {
        if ( (*aListIter).second->GetDefaultListStyleName() == rListStyleName )
        {
            aListIter = maLists.erase(aListIter);
        }
        else
            ++aListIter;
    }
}

void DocumentListsManager::trackChangeOfListStyleName( const UIName& sListStyleName,
                                        const UIName& sNewListStyleName )
{
    SwList* pList = getListForListStyle( sListStyleName );
    OSL_ENSURE( pList,
            "<DocumentListsManager::changeOfListStyleName(..)> - misusage of method: no list found for given list style name" );

    if ( pList != nullptr )
    {
        maListStyleLists.erase( sListStyleName );
        maListStyleLists[sNewListStyleName] = pList;
    }
    for (auto & it : maLists) // tdf#91131 update these references too
    {
        if (it.second->GetDefaultListStyleName() == sListStyleName)
        {
            it.second->SetDefaultListStyleName(sNewListStyleName);
        }
    }
}


DocumentListsManager::~DocumentListsManager()
{
}


OUString DocumentListsManager::MakeListIdUnique( const OUString& aSuggestedUniqueListId )
{
    tools::Long nHitCount = 0;
    OUString aTmpStr = aSuggestedUniqueListId;
    while ( getListByName( aTmpStr ) )
    {
        ++nHitCount;
        aTmpStr = aSuggestedUniqueListId + OUString::number( nHitCount );
    }

    return aTmpStr;
}

OUString DocumentListsManager::CreateUniqueListId()
{
    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);
    if (bHack)
    {
        static sal_Int64 nIdCounter = SAL_CONST_INT64(7000000000);
        return MakeListIdUnique( OUString( "list" + OUString::number(nIdCounter++) ) );
    }
    else
    {
        // #i92478#
        unsigned int const n(comphelper::rng::uniform_uint_distribution(0,
                                std::numeric_limits<unsigned int>::max()));
        OUString const aNewListId = "list" + OUString::number(n);
        return MakeListIdUnique( aNewListId );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
