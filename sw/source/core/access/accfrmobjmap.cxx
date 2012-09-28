/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* static */ bool SwAccessibleChildMap::IsSortingRequired( const SwFrm& rFrm )
{
    return ( rFrm.IsPageFrm() &&
             static_cast< const SwPageFrm& >( rFrm ).GetSortedObjs() ) ||
           ( rFrm.IsTxtFrm() &&
             rFrm.GetDrawObjs() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
