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
#include "precompiled_vcl.hxx"

#ifndef LAZYDELETE_CXX
#define LAZYDELETE_CXX

#include "vcl/window.hxx"
#include "vcl/menu.hxx"
#include "vcl/lazydelete.hxx"
#include "svdata.hxx"

namespace vcl {

LazyDeletorBase::LazyDeletorBase()
{
}

LazyDeletorBase::~LazyDeletorBase()
{
}

// instantiate instance pointers for LazyDeletor<Window,Menu>
template<> LazyDeletor<Window>* LazyDeletor<Window>::s_pOneInstance = NULL;
template<> LazyDeletor<Menu>* LazyDeletor<Menu>::s_pOneInstance = NULL;

// a list for all LazyeDeletor<T> singletons
static std::vector< LazyDeletorBase* > lcl_aDeletors;

void LazyDelete::addDeletor( LazyDeletorBase* i_pDel )
{
    lcl_aDeletors.push_back( i_pDel );
}

void LazyDelete::flush()
{
    unsigned int nCount = lcl_aDeletors.size();
    for( unsigned int i = 0; i < nCount; i++ )
        delete lcl_aDeletors[i];
    lcl_aDeletors.clear();
}

// specialized is_less function for Window
template<> bool LazyDeletor<Window>::is_less( Window* left, Window* right )
{
    return (left != right && right->IsChild( left, sal_True )) ? true : false;
}

// specialized is_less function for Menu
template<> bool LazyDeletor<Menu>::is_less( Menu* left, Menu* right )
{
    while( left && left != right )
        left = left->ImplGetStartedFrom();
    return left != NULL;
}

DeleteOnDeinitBase::~DeleteOnDeinitBase()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData && pSVData->mpDeinitDeleteList != NULL )
        pSVData->mpDeinitDeleteList->remove( this );
}

void DeleteOnDeinitBase::addDeinitContainer( DeleteOnDeinitBase* i_pContainer )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData )
    {
        ImplInitSVData();
        pSVData = ImplGetSVData();
    }

    DBG_ASSERT( ! pSVData->mbDeInit, "DeleteOnDeinit added after DeiInitVCL !" );
    if( pSVData->mbDeInit )
        return;

    if( pSVData->mpDeinitDeleteList == NULL )
        pSVData->mpDeinitDeleteList = new std::list< DeleteOnDeinitBase* >();
    pSVData->mpDeinitDeleteList->push_back( i_pContainer );
}

void DeleteOnDeinitBase::ImplDeleteOnDeInit()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpDeinitDeleteList )
    {
        for( std::list< vcl::DeleteOnDeinitBase* >::iterator it = pSVData->mpDeinitDeleteList->begin();
             it != pSVData->mpDeinitDeleteList->end(); ++it )
        {
            (*it)->doCleanup();
        }
        delete pSVData->mpDeinitDeleteList;
        pSVData->mpDeinitDeleteList = NULL;
    }
}

} // namespace vcl

#endif

