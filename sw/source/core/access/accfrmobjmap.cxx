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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <doc.hxx>
#include <frmfmt.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <node.hxx>
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>

#ifndef _ACCFFRMOBJMAP_HXX
#include <accfrmobjmap.hxx>
#endif

::std::pair< SwFrmOrObjMap::iterator, bool > SwFrmOrObjMap::insert(
        sal_uInt32 nPos, const SwFrmOrObj& rLower )
{
    SwFrmOrObjMapKey aKey( SwFrmOrObjMapKey::TEXT, nPos );
    value_type aEntry( aKey, rLower );
    return _SwFrmOrObjMap::insert( aEntry );
}

::std::pair< SwFrmOrObjMap::iterator, bool > SwFrmOrObjMap::insert(
        const SdrObject *pObj, const SwFrmOrObj& rLower, const SwDoc *pDoc )
{
    if( !bLayerIdsValid )
    {
        nHellId = pDoc->GetHellId();
        nControlsId = pDoc->GetControlsId();
        bLayerIdsValid = sal_True;
    }

    SdrLayerID nLayer = pObj->GetLayer();
    SwFrmOrObjMapKey::LayerId eLayerId = (nHellId == nLayer)
                    ? SwFrmOrObjMapKey::HELL
                    : ((nControlsId == nLayer) ? SwFrmOrObjMapKey::CONTROLS
                                               : SwFrmOrObjMapKey::HEAVEN);
    SwFrmOrObjMapKey aKey( eLayerId, pObj->GetOrdNum() );
    value_type aEntry( aKey, rLower );
    return _SwFrmOrObjMap::insert( aEntry );
}

SwFrmOrObjMap::SwFrmOrObjMap(
        const SwRect& rVisArea, const SwFrm *pFrm ) :
    bLayerIdsValid( sal_False )
{
    SwFrmOrObj aFrm( pFrm );
    sal_Bool bVisibleOnly = aFrm.IsVisibleChildrenOnly();

    sal_uInt32 nPos = 0;
    SwFrmOrObj aLower( pFrm->GetLower() );
    while( aLower.GetSwFrm() )
    {
        if( !bVisibleOnly || aLower.GetBox().IsOver( rVisArea ) )
            insert( nPos++, aLower );

        aLower = aLower.GetSwFrm()->GetNext();
    }

    if( pFrm->IsPageFrm() )
    {
        ASSERT( bVisibleOnly, "page frame within tab frame???" );
        const SwPageFrm *pPgFrm =
            static_cast< const SwPageFrm * >( pFrm );
        const SwSortedObjs *pObjs = pPgFrm->GetSortedObjs();
        if( pObjs )
        {
            const SwDoc *pDoc = pPgFrm->GetFmt()->GetDoc();
            for( sal_uInt16 i=0; i<pObjs->Count(); i++ )
            {
                aLower = (*pObjs)[i]->GetDrawObj();
                if( aLower.GetBox().IsOver( rVisArea ) )
                    insert( aLower.GetSdrObject(), aLower, pDoc );
            }
        }
    }
    else if( pFrm->IsTxtFrm() )
    {
        const SwDoc *pDoc = static_cast< const SwTxtFrm * >( pFrm )->GetNode()
                                                                   ->GetDoc();
        const SwSortedObjs *pObjs = pFrm->GetDrawObjs();
        if( pObjs )
        {
            for( sal_uInt16 i=0; i<pObjs->Count(); i++ )
            {
                aLower = (*pObjs)[i]->GetDrawObj();
                if( aLower.IsBoundAsChar() &&
                    (!bVisibleOnly || aLower.GetBox().IsOver( rVisArea )) )
                    insert( aLower.GetSdrObject(), aLower, pDoc );
            }
        }
    }
}
