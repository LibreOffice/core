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

#include <DocumentListItemsManager.hxx>
#include <SwNodeNum.hxx>
#include <ndtxt.hxx>


namespace sw
{

DocumentListItemsManager::DocumentListItemsManager() : mpListItemsList( new tImplSortedNodeNumList() ) // #i83479#
{
}

bool DocumentListItemsManager::lessThanNodeNum::operator()( const SwNodeNum* pNodeNumOne,
                                         const SwNodeNum* pNodeNumTwo ) const
{
    return pNodeNumOne->LessThan( *pNodeNumTwo );
}

void DocumentListItemsManager::addListItem( const SwNodeNum& rNodeNum )
{
    if ( mpListItemsList == nullptr )
    {
        return;
    }

    const bool bAlreadyInserted(
            mpListItemsList->find( &rNodeNum ) != mpListItemsList->end() );
    OSL_ENSURE( !bAlreadyInserted,
            "<DocumentListItemsManager::addListItem(..)> - <SwNodeNum> instance already registered as numbered item!" );
    if ( !bAlreadyInserted )
    {
        mpListItemsList->insert( &rNodeNum );
    }
}

void DocumentListItemsManager::removeListItem( const SwNodeNum& rNodeNum )
{
    if ( mpListItemsList == nullptr )
    {
        return;
    }

    const tImplSortedNodeNumList::size_type nDeleted = mpListItemsList->erase( &rNodeNum );
    if ( nDeleted > 1 )
    {
        OSL_FAIL( "<DocumentListItemsManager::removeListItem(..)> - <SwNodeNum> was registered more than once as numbered item!" );
    }
}

OUString DocumentListItemsManager::getListItemText( const SwNodeNum& rNodeNum,
                               const bool bWithNumber,
                               const bool bWithSpacesForLevel ) const
{
    return rNodeNum.GetTextNode()
           ? rNodeNum.GetTextNode()->GetExpandText( 0, -1, bWithNumber,
                                                  bWithNumber, bWithSpacesForLevel )
           : OUString();
}

void DocumentListItemsManager::getNumItems( tSortedNodeNumList& orNodeNumList ) const
{
    orNodeNumList.clear();
    orNodeNumList.reserve( mpListItemsList->size() );

    tImplSortedNodeNumList::iterator aIter;
    tImplSortedNodeNumList::iterator aEndIter = mpListItemsList->end();
    for ( aIter = mpListItemsList->begin(); aIter != aEndIter; ++aIter )
    {
        const SwNodeNum* pNodeNum = (*aIter);
        if ( pNodeNum->IsCounted() &&
             pNodeNum->GetTextNode() && pNodeNum->GetTextNode()->HasNumber() )
        {
            orNodeNumList.push_back( pNodeNum );
        }
    }
}

DocumentListItemsManager::~DocumentListItemsManager()
{
// #i83479#
delete mpListItemsList;
mpListItemsList = nullptr;
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
