 /*************************************************************************
 *
 *  $RCSfile: accfrmobjslist.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 13:59:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

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
