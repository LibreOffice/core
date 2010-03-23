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


#include <pagefrm.hxx>
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>
#include <accfrmobjslist.hxx>


SwFrmOrObjSList_const_iterator::SwFrmOrObjSList_const_iterator(
    const SwFrmOrObjSList& rLst, sal_Bool ) :
    rList( rLst ), aCurr( rList.pFrm->GetLower() ), nNextObj( 0 )
{
    if( !aCurr.GetSwFrm() )
    {
        if( rList.pFrm->IsPageFrm() )
        {
            const SwPageFrm *pPgFrm =
                static_cast< const SwPageFrm * >( rList.pFrm );
            const SwSortedObjs *pObjs = pPgFrm->GetSortedObjs();
            if( pObjs && pObjs->Count() )
                aCurr = (*pObjs)[nNextObj++]->GetDrawObj();
        }
        else if( rList.pFrm->IsTxtFrm() )
        {
            const SwSortedObjs *pObjs = rList.pFrm->GetDrawObjs();
            if( pObjs && pObjs->Count() )
            {
                aCurr = (*pObjs)[nNextObj++]->GetDrawObj();
                while( aCurr.IsValid() && !aCurr.IsBoundAsChar() )
                    aCurr = (nNextObj < pObjs->Count())
                                ? (*pObjs)[nNextObj++]->GetDrawObj()
                                : static_cast< const SdrObject *>( 0 );

            }
        }
    }
    if( rList.bVisibleOnly )
    {
        // Find the first visible
        while( aCurr.IsValid() &&
               !aCurr.GetBox().IsOver( rList.aVisArea ) )
            next();
    }
}

SwFrmOrObjSList_const_iterator& SwFrmOrObjSList_const_iterator::next()
{
    sal_Bool bGetSdrObject = sal_False;
    if( aCurr.GetSdrObject() )
    {
        bGetSdrObject = sal_True;
    }
    else if( aCurr.GetSwFrm() )
    {
        aCurr = aCurr.GetSwFrm()->GetNext();
        if( !aCurr.GetSwFrm() )
            bGetSdrObject = sal_True;
    }

    if( bGetSdrObject )
    {
        if( rList.pFrm->IsPageFrm() )
        {
            const SwPageFrm *pPgFrm =
                static_cast< const SwPageFrm * >( rList.pFrm );
            const SwSortedObjs *pObjs = pPgFrm->GetSortedObjs();
            aCurr = (pObjs && nNextObj < pObjs->Count())
                            ? (*pObjs)[nNextObj++]->GetDrawObj()
                            : static_cast< const SdrObject *>( 0 );
        }
        else if( rList.pFrm->IsTxtFrm() )
        {
            const SwSortedObjs *pObjs = rList.pFrm->GetDrawObjs();
            aCurr = (pObjs && nNextObj < pObjs->Count())
                            ? (*pObjs)[nNextObj++]->GetDrawObj()
                            : static_cast< const SdrObject *>( 0 );
            while( aCurr.IsValid() && !aCurr.IsBoundAsChar() )
                aCurr = (nNextObj < pObjs->Count())
                            ? (*pObjs)[nNextObj++]->GetDrawObj()
                            : static_cast< const SdrObject *>( 0 );
        }
    }

    return *this;
}

SwFrmOrObjSList_const_iterator& SwFrmOrObjSList_const_iterator::next_visible()
{
    next();
    while( aCurr.IsValid() &&
           !aCurr.GetBox().IsOver( rList.aVisArea ) )
        next();

    return *this;
}
