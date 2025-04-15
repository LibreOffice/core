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

#include "accfrmobjmap.hxx"
#include <accmap.hxx>
#include "acccontext.hxx"

#include <viewsh.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

#include <svx/svdobj.hxx>

using namespace sw::access;

SwAccessibleChildMap::SwAccessibleChildMap( const SwRect& rVisArea,
                                            const SwFrame& rFrame,
                                            SwAccessibleMap& rAccMap )
    : mnHellId(rAccMap.GetShell().GetDoc()->getIDocumentDrawModelAccess().GetHellId())
    , mnControlsId(rAccMap.GetShell().GetDoc()->getIDocumentDrawModelAccess().GetControlsId())
{
    const bool bVisibleChildrenOnly = SwAccessibleChild( &rFrame ).IsVisibleChildrenOnly();

    sal_uInt32 nPos = 0;
    SwAccessibleChild aLower( rFrame.GetLower() );
    while( aLower.GetSwFrame() )
    {
        if ( !bVisibleChildrenOnly ||
             aLower.AlwaysIncludeAsChild() ||
             aLower.GetBox( rAccMap ).Overlaps( rVisArea ) )
        {
            insert( nPos++, SwAccessibleChildMapKey::TEXT, aLower );
        }

        aLower = aLower.GetSwFrame()->GetNext();
    }

    if ( rFrame.IsPageFrame() )
    {
        OSL_ENSURE( bVisibleChildrenOnly, "page frame within tab frame???" );
        const SwPageFrame *pPgFrame =
            static_cast< const SwPageFrame * >( &rFrame );
        const SwSortedObjs *pObjs = pPgFrame->GetSortedObjs();
        if ( pObjs )
        {
            for(const SwAnchoredObject* pObj : *pObjs)
            {
                aLower = pObj->GetDrawObj();
                if ( aLower.GetBox( rAccMap ).Overlaps( rVisArea ) )
                {
                    insert( aLower.GetDrawObject(), aLower );
                }
            }
        }
    }
    else if( rFrame.IsTextFrame() )
    {
        const SwSortedObjs *pObjs = rFrame.GetDrawObjs();
        if ( pObjs )
        {
            for(const SwAnchoredObject* pObj : *pObjs)
            {
                aLower = pObj->GetDrawObj();
                if ( aLower.IsBoundAsChar() &&
                     ( !bVisibleChildrenOnly ||
                       aLower.AlwaysIncludeAsChild() ||
                       aLower.GetBox( rAccMap ).Overlaps( rVisArea ) ) )
                {
                    insert( aLower.GetDrawObject(), aLower );
                }
            }
        }

        {
            ::rtl::Reference < SwAccessibleContext > xAccImpl =
                                rAccMap.GetContextImpl( &rFrame, false );
            if( xAccImpl.is() )
            {
                SwAccessibleContext* pAccImpl = xAccImpl.get();
                if ( pAccImpl &&
                     pAccImpl->HasAdditionalAccessibleChildren() )
                {
                    std::vector< vcl::Window* > aAdditionalChildren;
                    pAccImpl->GetAdditionalAccessibleChildren( &aAdditionalChildren );

                    sal_Int32 nCounter( 0 );
                    for ( const auto& rpChild : aAdditionalChildren )
                    {
                        aLower = rpChild;
                        insert( ++nCounter, SwAccessibleChildMapKey::XWINDOW, aLower );
                    }
                }
            }
        }
    }
}

std::pair< SwAccessibleChildMap::iterator, bool > SwAccessibleChildMap::insert(
                                                const sal_uInt32 nPos,
                                                const SwAccessibleChildMapKey::LayerId eLayerId,
                                                const SwAccessibleChild& rLower )
{
    SwAccessibleChildMapKey aKey( eLayerId, nPos );
    return emplace( aKey, rLower );
}

std::pair< SwAccessibleChildMap::iterator, bool > SwAccessibleChildMap::insert(
                                                const SdrObject *pObj,
                                                const SwAccessibleChild& rLower )
{
    const SdrLayerID nLayer = pObj->GetLayer();
    SwAccessibleChildMapKey::LayerId eLayerId =
                    (mnHellId == nLayer)
                    ? SwAccessibleChildMapKey::HELL
                    : ( (mnControlsId == nLayer)
                        ? SwAccessibleChildMapKey::CONTROLS
                        : SwAccessibleChildMapKey::HEAVEN );
    SwAccessibleChildMapKey aKey( eLayerId, pObj->GetOrdNum() );
    return emplace( aKey, rLower );
}

bool SwAccessibleChildMap::IsSortingRequired( const SwFrame& rFrame )
{
    return ( rFrame.IsPageFrame() &&
             static_cast< const SwPageFrame& >( rFrame ).GetSortedObjs() ) ||
           ( rFrame.IsTextFrame() &&
             rFrame.GetDrawObjs() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
