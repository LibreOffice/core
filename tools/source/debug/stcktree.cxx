/*************************************************************************
 *
 *  $RCSfile: stcktree.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:06 $
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

#include <string.h>

#include <debug.hxx>

// -----------------------------------------------------------------------

#if defined( DBG_UTIL ) && defined( WNT ) && defined( INTEL )

struct ImpDbgStackTree
{
    ImpDbgStackTree*    pLeft_;
    ImpDbgStackTree*    pRight_;
    ImpDbgStackTree*    pCaller_;
    ImpDbgStackTree*    pSub_;
    ULONG               nIP_;
    ULONG               nBytesLeak_;
    ULONG               nBytesPeak_;
    ULONG               nBytes_;
    ULONG               nCountLeak_;
    ULONG               nCountPeak_;
    ULONG               nCount_;
    ULONG               nMax_;
    ULONG               nMin_;

                        ImpDbgStackTree( ImpDbgStackTree* pSub, ULONG nIP );
                        ~ImpDbgStackTree();

    ImpDbgStackTree*    Add( ULONG nAlloc, ULONG* pBP, ULONG nIP );
    void                Print( int nLevel, ULONG nCount, ULONG nCountLeak );
    void                Print( int nLevel );
};

static ImpDbgStackTree* pImpDbgStackTreeRoot     = NULL;
static ULONG*           pImpDbgStackTreeBP       = NULL;
static ULONG            nImpDbgStackTreeMain     = 0;
static int              nImpDbgStackTreeSem      = 0;

// -----------------------------------------------------------------------

ImpDbgStackTree::ImpDbgStackTree( ImpDbgStackTree* pSub, ULONG nIP )
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

void ImpDbgStackTree::Print( int nLevel, ULONG nCount, ULONG nCountLeak )
{
    if ( pLeft_ )
        pLeft_->Print( nLevel, nCount, nCountLeak );

    if ( nCount_ >= nCount && nCountLeak_ >= nCountLeak )
    {
        if ( nMax_ == nMin_ )
        {
            ULONG nTemp = nCountLeak_ * nMin_;
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

ImpDbgStackTree* ImpDbgStackTree::Add( ULONG nAlloc, ULONG *pBP, ULONG nIP )
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

    if ( !(pBP[0] & 3) && (ULONG)pBP < pBP[0] && pBP[0] < (ULONG)pImpDbgStackTreeBP )
    {
        pBP = (ULONG*)pBP[0];
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
        ULONG* pBP;
        __asm mov pBP, ebp;

        pImpDbgStackTreeBP   = (ULONG*)pBP[0];
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
            ULONG nOldOut = pData->nTraceOut;
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

void* DbgGetStackTree( ULONG nAlloc )
{
    ImpDbgStackTree* pReturn = NULL;

    if ( nImpDbgStackTreeMain && !nImpDbgStackTreeSem )
    {
        nImpDbgStackTreeSem++;

        ULONG* pBP;
        __asm mov pBP, ebp;

        ULONG  nIP = pBP[1];
        if ( !pImpDbgStackTreeRoot )
            pImpDbgStackTreeRoot = new ImpDbgStackTree( NULL, nIP );
        pReturn = pImpDbgStackTreeRoot->Add( nAlloc, pBP, nIP );
        nImpDbgStackTreeSem--;
    }

    return pReturn;
}

// -----------------------------------------------------------------------

void DbgFreeStackTree( void* pVoid, ULONG nAlloc )
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
        ULONG nOldOut = pData->nTraceOut;
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
void* DbgGetStackTree( ULONG ) { return NULL; }
void DbgFreeStackTree( void*, ULONG nAlloc ) {}
void DbgPrintStackTree( void* ) {}

#endif
