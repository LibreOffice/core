 /*************************************************************************
 *
 *  $RCSfile: accfrmobj.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2002-04-05 12:05:04 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ACCFLYMAP_HXX
#include <accflymap.hxx>
#endif
#ifndef _ACCFRMOBJ_HXX
#include <accfrmobj.hxx>
#endif

SwFrmOrObjIter::SwFrmOrObjIter( const SwFrm *pF ) :
    pFrm( pF ),
    aNext( pF->GetLower() ),
    nNextObj( 0 )
{
    if( !aNext.GetSwFrm() )
    {
        // No lowers?  Maybe thare are some fly frames or SdrObjects
        if( pFrm->IsPageFrm() )
        {
            const SwPageFrm *pPgFrm =
                static_cast< const SwPageFrm * >( pFrm );
            const SwSortDrawObjs *pObjs = pPgFrm->GetSortedObjs();
            if( pObjs && pObjs->Count() > 0 )
                aNext = (*pObjs)[0];
        }
    }
}

const SwFrmOrObj& SwFrmOrObjIter::Next()
{
    aCurr = aNext;
    if( !aCurr.GetSdrObject() ) // We are within the lowers
    {
        ASSERT( aCurr.GetSwFrm(), "next called at end of iterator" );
        if( aCurr.GetSwFrm() )
        {
            aNext = aCurr.GetSwFrm()->GetNext();
            if( !aNext.GetSwFrm() ) // This was the last lower, but maybe
            {                       // maybe there are some draw objs
                if( pFrm->IsPageFrm() )
                {
                    const SwPageFrm *pPgFrm =
                        static_cast< const SwPageFrm * >( pFrm );
                    const SwSortDrawObjs *pObjs = pPgFrm->GetSortedObjs();
                    if( pObjs && pObjs->Count() > 0 )
                        aNext = (*pObjs)[0];
                }
            }
        }
    }
    else
    {
        nNextObj++;
        aNext = (const SdrObject *)0; // clear next
        if( pFrm->IsPageFrm() )
        {
            const SwPageFrm *pPgFrm =
                    static_cast< const SwPageFrm * >( pFrm );
            const SwSortDrawObjs *pObjs = pPgFrm->GetSortedObjs();
            if( pObjs && nNextObj < pObjs->Count() )
                aNext = (*pObjs)[nNextObj];
        }
    }

    return aCurr;
}

SwFrmOrObjSortedIter::SwFrmOrObjSortedIter( const SwFrm *pF,
                                             sal_Bool bReverse ) :
    pFrm( pF ),
    aNext( pF->GetLower() ),
    nNextObj( 0 )
{
    if( !aNext.GetSwFrm() )
    {
        // No lowers?  Maybe thare are some fly frames or SdrObjects
        if( pFrm->IsPageFrm() )
        {
            const SwPageFrm *pPgFrm =
                static_cast< const SwPageFrm * >( pFrm );
            const SwSortDrawObjs *pObjs = pPgFrm->GetSortedObjs();
            if( pObjs && pObjs->Count() > 0 )
                aNext = (*pObjs)[0];
        }
    }
}
