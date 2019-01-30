/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <algorithm>

#include <sfx2/objsh.hxx>

#include <adiasync.hxx>
#include <brdcst.hxx>
#include <global.hxx>
#include <document.hxx>
#include <sc.hrc>
#include <osl/diagnose.h>
#include <osl/thread.h>

ScAddInAsyncs theAddInAsyncTbl;

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData )
{
    ScAddInAsync::CallBack( sal_uLong( nHandle ), pData );
}
}

ScAddInAsync::ScAddInAsync(sal_uLong nHandleP, LegacyFuncData* pFuncData, ScDocument* pDoc) :
    SvtBroadcaster(),
    pStr( nullptr ),
    mpFuncData(pFuncData),
    nHandle( nHandleP ),
    meType(pFuncData->GetAsyncType()),
    bValid( false )
{
    pDocs.reset(new ScAddInDocs);
    pDocs->insert( pDoc );
    theAddInAsyncTbl.emplace( this );
}

ScAddInAsync::~ScAddInAsync()
{
    // in dTor because of theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
    mpFuncData->Unadvice( static_cast<double>(nHandle) );
    if ( meType == ParamType::PTR_STRING && pStr )      // include type comparison because of union
        delete pStr;
    pDocs.reset();
}

ScAddInAsync* ScAddInAsync::Get( sal_uLong nHandleP )
{
    ScAddInAsync* pRet = nullptr;
    auto it = std::find_if(
        theAddInAsyncTbl.begin(), theAddInAsyncTbl.end(),
        [nHandleP](std::unique_ptr<ScAddInAsync> const & el)
            { return el->nHandle == nHandleP; });
    if ( it != theAddInAsyncTbl.end() )
        pRet = it->get();
    return pRet;
}

void ScAddInAsync::CallBack( sal_uLong nHandleP, void* pData )
{
    auto asyncIt = std::find_if(
        theAddInAsyncTbl.begin(), theAddInAsyncTbl.end(),
        [nHandleP](std::unique_ptr<ScAddInAsync> const & el)
            { return el->nHandle == nHandleP; });
    if ( asyncIt == theAddInAsyncTbl.end() )
        return;
    ScAddInAsync* p = asyncIt->get();

    if ( !p->HasListeners() )
    {
        // not in dTor because of theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        theAddInAsyncTbl.erase( asyncIt );
        return ;
    }
    switch ( p->meType )
    {
        case ParamType::PTR_DOUBLE :
            p->nVal = *static_cast<double*>(pData);
            break;
        case ParamType::PTR_STRING :
        {
            sal_Char* pChar = static_cast<sal_Char*>(pData);
            if ( p->pStr )
                *p->pStr = OUString( pChar, strlen(pChar),osl_getThreadTextEncoding() );
            else
                p->pStr = new OUString( pChar, strlen(pChar), osl_getThreadTextEncoding() );
            break;
        }
        default :
            OSL_FAIL( "unknown AsyncType" );
            return;
    }
    p->bValid = true;
    p->Broadcast( ScHint(SfxHintId::ScDataChanged, ScAddress()) );

    for ( ScDocument* pDoc : *p->pDocs )
    {
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxHint( SfxHintId::ScDataChanged ) );
    }
}

void ScAddInAsync::RemoveDocument( ScDocument* pDocumentP )
{
    for( ScAddInAsyncs::reverse_iterator iter1 = theAddInAsyncTbl.rbegin(); iter1 != theAddInAsyncTbl.rend(); ++iter1 )
    {   // backwards because of pointer-movement in array
        ScAddInAsync* pAsync = iter1->get();
        ScAddInDocs* p = pAsync->pDocs.get();
        ScAddInDocs::iterator iter2 = p->find( pDocumentP );
        if( iter2 != p->end() )
        {
            p->erase( iter2 );
            if ( p->empty() )
            {   // this AddIn is not used anymore
                theAddInAsyncTbl.erase( --(iter1.base()) );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
