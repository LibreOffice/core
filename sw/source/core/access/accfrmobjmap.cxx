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
#include <frmfmt.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <node.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

#include <svx/svdobj.hxx>

using namespace sw::access;

SwAccessibleChildMap::SwAccessibleChildMap( const SwRect& rVisArea,
                                            const SwFrm& rFrm,
                                            SwAccessibleMap& rAccMap )
    : nHellId( rAccMap.GetShell()->GetDoc()->GetHellId() )
    , nControlsId( rAccMap.GetShell()->GetDoc()->GetControlsId() )
{
    const bool bVisibleChildrenOnly = SwAccessibleChild( &rFrm ).IsVisibleChildrenOnly();

    sal_uInt32 nPos = 0;
    SwAccessibleChild aLower( rFrm.GetLower() );
    while( aLower.GetSwFrm() )
    {
        if ( !bVisibleChildrenOnly ||
             aLower.AlwaysIncludeAsChild() ||
             aLower.GetBox( rAccMap ).IsOver( rVisArea ) )
        {
            insert( nPos++, SwAccessibleChildMapKey::TEXT, aLower );
        }

        aLower = aLower.GetSwFrm()->GetNext();
    }

    if ( rFrm.IsPageFrm() )
    {
        OSL_ENSURE( bVisibleChildrenOnly, "page frame within tab frame???" );
        const SwPageFrm *pPgFrm =
            static_cast< const SwPageFrm * >( &rFrm );
        const SwSortedObjs *pObjs = pPgFrm->GetSortedObjs();
        if ( pObjs )
        {
            for( sal_uInt16 i=0; i<pObjs->Count(); i++ )
            {
                aLower = (*pObjs)[i]->GetDrawObj();
                if ( aLower.GetBox( rAccMap ).IsOver( rVisArea ) )
                {
                    insert( aLower.GetDrawObject(), aLower );
                }
            }
        }
    }
    else if( rFrm.IsTxtFrm() )
    {
        const SwSortedObjs *pObjs = rFrm.GetDrawObjs();
        if ( pObjs )
        {
            for( sal_uInt16 i=0; i<pObjs->Count(); i++ )
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
                                rAccMap.GetContextImpl( &rFrm, sal_False );
            if( xAccImpl.is() )
            {
                SwAccessibleContext* pAccImpl = xAccImpl.get();
                if ( pAccImpl &&
                     pAccImpl->HasAdditionalAccessibleChildren() )
                {
                    std::vector< Window* >* pAdditionalChildren =
                                                new std::vector< Window* >();
                    pAccImpl->GetAdditionalAccessibleChildren( pAdditionalChildren );

                    sal_Int32 nCounter( 0 );
                    for ( std::vector< Window* >::iterator aIter = pAdditionalChildren->begin();
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
    return _SwAccessibleChildMap::insert( aEntry );
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
    return _SwAccessibleChildMap::insert( aEntry );
}

bool SwAccessibleChildMap::IsSortingRequired( const SwFrm& rFrm )
{
    return ( rFrm.IsPageFrm() &&
             static_cast< const SwPageFrm& >( rFrm ).GetSortedObjs() ) ||
           ( rFrm.IsTxtFrm() &&
             rFrm.GetDrawObjs() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
