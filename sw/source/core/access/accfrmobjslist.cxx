 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accfrmobjslist.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 20:36:02 $
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



#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
#ifndef _ACCFRMOBJSLIST_HXX
#include <accfrmobjslist.hxx>
#endif


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
