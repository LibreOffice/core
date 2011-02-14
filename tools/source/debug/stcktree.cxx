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
#include "precompiled_tools.hxx"

#include <string.h>

#include <tools/debug.hxx>

// -----------------------------------------------------------------------

#if defined( DBG_UTIL ) && defined( WNT ) && defined( INTEL )

struct ImpDbgStackTree
{
    ImpDbgStackTree*    pLeft_;
    ImpDbgStackTree*    pRight_;
    ImpDbgStackTree*    pCaller_;
    ImpDbgStackTree*    pSub_;
    sal_uIntPtr             nIP_;
    sal_uIntPtr             nBytesLeak_;
    sal_uIntPtr             nBytesPeak_;
    sal_uIntPtr             nBytes_;
    sal_uIntPtr             nCountLeak_;
    sal_uIntPtr             nCountPeak_;
    sal_uIntPtr             nCount_;
    sal_uIntPtr             nMax_;
    sal_uIntPtr             nMin_;

                        ImpDbgStackTree( ImpDbgStackTree* pSub, sal_uIntPtr nIP );
                        ~ImpDbgStackTree();

    ImpDbgStackTree*    Add( sal_uIntPtr nAlloc, sal_uIntPtr* pBP, sal_uIntPtr nIP );
    void                Print( int nLevel, sal_uIntPtr nCount, sal_uIntPtr nCountLeak );
    void                Print( int nLevel );
};

static ImpDbgStackTree* pImpDbgStackTreeRoot     = NULL;
static sal_uIntPtr*         pImpDbgStackTreeBP       = NULL;
static sal_uIntPtr          nImpDbgStackTreeMain     = 0;
static int              nImpDbgStackTreeSem      = 0;

// -----------------------------------------------------------------------

ImpDbgStackTree::ImpDbgStackTree( ImpDbgStackTree* pSub, sal_uIntPtr nIP )
{
    pSub_ = pSub;
    nIP_ = nIP;
    pLeft_ = pRight_ = pCaller_ = NULL;
    nBytesLeak_ = nBytesPeak_ = nBytes_ = 0;
    nCountLeak_ = nCountPeak_ = nCount_ = 0;
}

// -----------------------------------------------------------------------

ImpDbgStackTree::~ImpDbgStackTree()
{
    if ( pLeft_ )
        delete pLeft_;
    if ( pRight_ )
        delete pRight_;
    if ( pCaller_ )
        delete pCaller_;
}

// -----------------------------------------------------------------------

void ImpDbgStackTree::Print( int nLevel, sal_uIntPtr nCount, sal_uIntPtr nCountLeak )
{
    if ( pLeft_ )
        pLeft_->Print( nLevel, nCount, nCountLeak );

    if ( nCount_ >= nCount && nCountLeak_ >= nCountLeak )
    {
        if ( nMax_ == nMin_ )
        {
            sal_uIntPtr nTemp = nCountLeak_ * nMin_;
            DbgOutf( "%*c%08lx Count=%lu/%lu/%lu Bytes=%lu/%lu/%lu Size=%lu",
                     nLevel, ' ', nIP_,
                     nCount_, nCountPeak_, nCountLeak_,
                     nBytes_, nBytesPeak_, nTemp,
                     nMin_ );
        }
        else
        {
            DbgOutf( "%*c%08lx Count=%lu/%lu/%lu Bytes=%lu/%lu/%lu Size=%lu-%lu",
                     nLevel, ' ', nIP_,
                     nCount_, nCountPeak_, nCountLeak_,
                     nBytes_, nBytesPeak_, nBytesLeak_,
                     nMin_, nMax_ );
        }

        if ( pCaller_ )
            if( nLevel > 3 && nCountLeak )
                pCaller_->Print( nLevel + 1, nCount, 1 );
            else
                pCaller_->Print( nLevel + 1, nCount, nCountLeak );
    }

    if ( pRight_ )
        pRight_->Print( nLevel, nCount, nCountLeak );
}

// -----------------------------------------------------------------------

void ImpDbgStackTree::Print( int nLevel )
{
    if ( pSub_ )
        pSub_->Print( nLevel + 1 );
    DbgOutf( "%*c%08lx", nLevel, ' ',nIP_ );
}

// -----------------------------------------------------------------------

ImpDbgStackTree* ImpDbgStackTree::Add( sal_uIntPtr nAlloc, sal_uIntPtr *pBP, sal_uIntPtr nIP )
{
    if ( nIP < nIP_ )
    {
        if ( !pLeft_ )
            pLeft_ = new ImpDbgStackTree( pSub_, nIP );
        return pLeft_->Add( nAlloc, pBP, nIP );
    }
    if ( nIP > nIP_ )
    {
        if ( !pRight_ )
            pRight_ = new ImpDbgStackTree( pSub_, nIP );
        return pRight_->Add( nAlloc, pBP, nIP );
    }

    nCount_++;
    nCountLeak_++;
    if ( nCountLeak_ > nCountPeak_ )
        nCountPeak_ = nCountLeak_;
    nBytes_     += nAlloc;
    nBytesLeak_ += nAlloc;
    if ( nBytesLeak_ > nBytesPeak_ )
        nBytesPeak_ = nBytesLeak_;
    if ( nCount_ == 1 )
        nMax_ = nMin_ = nAlloc;
    else if ( nMax_ < nAlloc )
        nMax_ = nAlloc;
    else if ( nMin_ > nAlloc )
        nMin_ = nAlloc;

    if ( !(pBP[0] & 3) && (sal_uIntPtr)pBP < pBP[0] && pBP[0] < (sal_uIntPtr)pImpDbgStackTreeBP )
    {
        pBP = (sal_uIntPtr*)pBP[0];
        nIP = pBP[1];
        if ( 0x01100000 <= nIP && nIP < 0x20000000 && nIP != nImpDbgStackTreeMain )
        {
            if ( !pCaller_ )
                pCaller_ = new ImpDbgStackTree( this, nIP );
            return pCaller_->Add( nAlloc, pBP, nIP );
        }
        else
            return this;
    }

    return this;
}

// -----------------------------------------------------------------------

void DbgStartStackTree()
{
    if ( !nImpDbgStackTreeMain )
    {
        sal_uIntPtr* pBP;
        __asm mov pBP, ebp;

        pImpDbgStackTreeBP   = (sal_uIntPtr*)pBP[0];
        nImpDbgStackTreeMain = pImpDbgStackTreeBP[1];
    }
}

// -----------------------------------------------------------------------

void DbgEndStackTree()
{
    if ( nImpDbgStackTreeMain )
    {
        nImpDbgStackTreeMain = 0;
        if ( pImpDbgStackTreeRoot )
        {
            // Ausgaben ins File umleiten
            DbgData* pData = DbgGetData();
            sal_uIntPtr nOldOut = pData->nTraceOut;
            pData->nTraceOut = DBG_OUT_FILE;

            DbgOutf( "Leak-Report" );
            DbgOutf( "===========" );
            DbgOutf( "Mem-StackTree:" );
            DbgOutf( "{" );
            pImpDbgStackTreeRoot->Print( 1, 1, 2 );
            DbgOutf( "}" );

            DbgOutf( "Alloc-Report" );
            DbgOutf( "===========" );
            DbgOutf( "Mem-StackTree:" );
            DbgOutf( "{" );
            pImpDbgStackTreeRoot->Print( 1, 1000, 0 ); // ???
            DbgOutf( "}" );

            pData->nTraceOut = nOldOut;

            nImpDbgStackTreeSem++;
            delete pImpDbgStackTreeRoot;
            pImpDbgStackTreeRoot = NULL;
            nImpDbgStackTreeSem--;
        }
    }
}

// -----------------------------------------------------------------------

void* DbgGetStackTree( sal_uIntPtr nAlloc )
{
    ImpDbgStackTree* pReturn = NULL;

    if ( nImpDbgStackTreeMain && !nImpDbgStackTreeSem )
    {
        nImpDbgStackTreeSem++;

        sal_uIntPtr* pBP;
        __asm mov pBP, ebp;

        sal_uIntPtr  nIP = pBP[1];
        if ( !pImpDbgStackTreeRoot )
            pImpDbgStackTreeRoot = new ImpDbgStackTree( NULL, nIP );
        pReturn = pImpDbgStackTreeRoot->Add( nAlloc, pBP, nIP );
        nImpDbgStackTreeSem--;
    }

    return pReturn;
}

// -----------------------------------------------------------------------

void DbgFreeStackTree( void* pVoid, sal_uIntPtr nAlloc )
{
    ImpDbgStackTree* p = (ImpDbgStackTree*)pVoid;

    if ( p && nImpDbgStackTreeMain && !nImpDbgStackTreeSem )
    {
        if ( nAlloc < p->nMin_ )
            nAlloc = p->nMin_;

        p->nCountLeak_--;
        p->nBytesLeak_ -= nAlloc;

        if ( p->nMax_ && 0xFFFFFFFF / p->nMax_ > p->nCountLeak_ )
        {
            if ( p->nBytesLeak_ > p->nMax_ * p->nCountLeak_ )
            {
                nAlloc         += p->nBytesLeak_ - p->nMax_ * p->nCountLeak_;
                p->nBytesLeak_  = p->nMax_ * p->nCountLeak_;
            }
        }

        if ( p->pSub_ )
            DbgFreeStackTree( (void*)(p->pSub_), nAlloc );
    }
}

// -----------------------------------------------------------------------

void DbgPrintStackTree( void* pVoid )
{
    ImpDbgStackTree* p = (ImpDbgStackTree*)pVoid;

    if ( p && nImpDbgStackTreeMain && !nImpDbgStackTreeSem )
    {
        // Ausgaben ins File umleiten
        DbgData* pData = DbgGetData();
        sal_uIntPtr nOldOut = pData->nTraceOut;
        pData->nTraceOut = DBG_OUT_FILE;

        DbgOutf( "Mem-StackTree:" );
        DbgOutf( "{" );
        p->Print( 1 );
        DbgOutf( "}" );

        pData->nTraceOut = nOldOut;
    }
}

#else

void DbgStartStackTree() {}
void DbgEndStackTree() {}
void* DbgGetStackTree( sal_uIntPtr ) { return NULL; }
void DbgFreeStackTree( void*, sal_uIntPtr ) {}
void DbgPrintStackTree( void* ) {}

#endif
