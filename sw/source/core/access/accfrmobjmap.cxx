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

#include <accfrmobjmap.hxx>
#include <accframe.hxx>
#include <accmap.hxx>
#include <acccontext.hxx>

#include <viewsh.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <frmfmt.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <node.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

#include <svx/svdobj.hxx>

using namespace sw::access;

SwAccessibleChildMap::SwAccessibleChildMap( const SwRect& rVisArea,
                                            const SwFrame& rFrame,
                                            SwAccessibleMap& rAccMap )
    : nHellId( rAccMap.GetShell()->GetDoc()->getIDocumentDrawModelAccess().GetHellId() )
    , nControlsId( rAccMap.GetShell()->GetDoc()->getIDocumentDrawModelAccess().GetControlsId() )
{
    const bool bVisibleChildrenOnly = SwAccessibleChild( &rFrame ).IsVisibleChildrenOnly();

    sal_uInt32 nPos = 0;
    SwAccessibleChild aLower( rFrame.GetLower() );
    while( aLower.GetSwFrame() )
    {
        if ( !bVisibleChildrenOnly ||
             aLower.AlwaysIncludeAsChild() ||
             aLower.GetBox( rAccMap ).IsOver( rVisArea ) )
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
            for( size_t i=0; i<pObjs->size(); ++i )
            {
                aLower = (*pObjs)[i]->GetDrawObj();
                if ( aLower.GetBox( rAccMap ).IsOver( rVisArea ) )
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
            for( size_t i=0; i<pObjs->size(); ++i )
            {
                aLower = (*pObjs)[i]->GetDrawObj();
                if ( aLower.IsBoundAsChar() &&
                     ( !bVisibleChildrenOnly ||
                       aLower.AlwaysIncludeAsChild() ||
                       aLower.GetBox( rAccMap ).IsOver( rVisArea ) ) )
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
                    std::vector< vcl::Window* >* pAdditionalChildren =
                                                new std::vector< vcl::Window* >();
                    pAccImpl->GetAdditionalAccessibleChildren( pAdditionalChildren );

                    sal_Int32 nCounter( 0 );
                    for ( std::vector< vcl::Window* >::iterator aIter = pAdditionalChildren->begin();
                          aIter != pAdditionalChildren->end();
                          ++aIter )
                    {
                        aLower = (*aIter);
                        insert( ++nCounter, SwAccessibleChildMapKey::XWINDOW, aLower );
                    }

                    delete pAdditionalChildren;
                }
            }
        }
    }
}

::std::pair< SwAccessibleChildMap::iterator, bool > SwAccessibleChildMap::insert(
                                                const sal_uInt32 nPos,
                                                const SwAccessibleChildMapKey::LayerId eLayerId,
                                                const SwAccessibleChild& rLower )
{
    SwAccessibleChildMapKey aKey( eLayerId, nPos );
    value_type aEntry( aKey, rLower );
    return insert( aEntry );
}

::std::pair< SwAccessibleChildMap::iterator, bool > SwAccessibleChildMap::insert(
                                                const SdrObject *pObj,
                                                const SwAccessibleChild& rLower )
{
    const SdrLayerID nLayer = pObj->GetLayer();
    SwAccessibleChildMapKey::LayerId eLayerId =
                    (nHellId == nLayer)
                    ? SwAccessibleChildMapKey::HELL
                    : ( (nControlsId == nLayer)
                        ? SwAccessibleChildMapKey::CONTROLS
                        : SwAccessibleChildMapKey::HEAVEN );
    SwAccessibleChildMapKey aKey( eLayerId, pObj->GetOrdNum() );
    value_type aEntry( aKey, rLower );
    return insert( aEntry );
}

bool SwAccessibleChildMap::IsSortingRequired( const SwFrame& rFrame )
{
    return ( rFrame.IsPageFrame() &&
             static_cast< const SwPageFrame& >( rFrame ).GetSortedObjs() ) ||
           ( rFrame.IsTextFrame() &&
             rFrame.GetDrawObjs() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
