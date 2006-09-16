 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accfrmobjmap.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 20:35:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

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
