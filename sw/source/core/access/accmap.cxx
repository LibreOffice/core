 /*************************************************************************
 *
 *  $RCSfile: accmap.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2002-02-04 14:07:14 $
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

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef __SGI_STL_MAP
#include <map>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _ACCCONTEXT_HXX
#include <acccontext.hxx>
#endif
#ifndef _ACCDOC_HXX
#include <accdoc.hxx>
#endif
#ifndef _ACCPARA_HXX
#include <accpara.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

SwAccessibleMap aAccMap;

struct SwFrmFunc
{
    sal_Bool operator()( const SwFrm * p1,
                         const SwFrm * p2) const
    {
        return p1 < p2;
    }
};

typedef ::std::map < const SwFrm *, ::vos::ORef < SwAccessibleContext >, SwFrmFunc > _SwAccessibleMap_Impl;

class SwAccessibleMap_Impl: public _SwAccessibleMap_Impl
{
};

SwAccessibleMap::SwAccessibleMap() :
    pMap( new SwAccessibleMap_Impl ),
    nPara( 1 )
{
}

SwAccessibleMap::~SwAccessibleMap()
{
    delete pMap;
}

Reference< XAccessible > SwAccessibleMap::GetDocumentView(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
            const Rectangle& rVisArea,
            const SwRootFrm *pRootFrm )
{
    SwAccessibleContext *pAcc = 0;

    SwAccessibleMap_Impl::iterator aIter = pMap->find( pRootFrm );
    if( aIter != pMap->end() )
    {
        pAcc = (*aIter).second.getBodyPtr();
        pAcc->SetVisArea( rVisArea );
    }
    else
    {
        pAcc = new SwAccessibleDocument( rxParent, rVisArea, pRootFrm );
        SwAccessibleMap_Impl::value_type aEntry( pRootFrm, pAcc );
        pMap->insert( aEntry );
    }

    Reference< XAccessible > xRet( pAcc );

    return xRet;
}

SwAccessibleContext *SwAccessibleMap::GetContext( const Rectangle& rVisArea,
                                                   const SwFrm *pFrm )
{
    SwAccessibleContext *pAcc = 0;

    SwAccessibleMap_Impl::iterator aIter = pMap->find( pFrm );
    if( aIter != pMap->end() )
    {
        pAcc = (*aIter).second.getBodyPtr();
    }
    else
    {
        if( pFrm->IsTxtFrm() )
            pAcc = new SwAccessibleParagraph( nPara++, rVisArea,
                                              (const SwTxtFrm *)pFrm );

        ASSERT( pAcc, "unknown frame type" );
        if( pAcc )
        {
            SwAccessibleMap_Impl::value_type aEntry( pFrm, pAcc );
            pMap->insert( aEntry );
        }
    }

    return pAcc;
}

void SwAccessibleMap::RemoveContext( SwAccessibleContext *pAcc )
{
    SwAccessibleMap_Impl::iterator aIter = pMap->find( pAcc->GetFrm() );
    if( aIter != pMap->end() )
        pMap->erase( aIter );
}
