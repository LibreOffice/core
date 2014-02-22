/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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


template<> LazyDeletor<Window>* LazyDeletor<Window>::s_pOneInstance = NULL;
template<> LazyDeletor<Menu>* LazyDeletor<Menu>::s_pOneInstance = NULL;


static std::vector< LazyDeletorBase* > lcl_aDeletors;

void LazyDelete::addDeletor( LazyDeletorBase* i_pDel )
{
    lcl_aDeletors.push_back( i_pDel );
}

void LazyDelete::flush()
{
    DBG_TESTSOLARMUTEX(); 

    unsigned int nCount = lcl_aDeletors.size();
    for( unsigned int i = 0; i < nCount; i++ )
        delete lcl_aDeletors[i];
    lcl_aDeletors.clear();
}


template<> bool LazyDeletor<Window>::is_less( Window* left, Window* right )
{
    return (left != right && right->IsChild( left, true )) ? true : false;
}

#ifndef LINUX

template<> bool LazyDeletor<Menu>::is_less( Menu* left, Menu* right )
{
    while( left && left != right )
        left = left->ImplGetStartedFrom();
    return left != NULL;
}
#endif

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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
