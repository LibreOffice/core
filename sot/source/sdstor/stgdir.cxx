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


#include <string.h>

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"


















StgDirEntry::StgDirEntry( const void* pBuffer, sal_uInt32 nBufferLen, bool * pbOk ) : StgAvlNode()
{
    *pbOk = aEntry.Load( pBuffer, nBufferLen );

    InitMembers();
}

StgDirEntry::StgDirEntry( const StgEntry& r ) : StgAvlNode(), aEntry( r )
{
    InitMembers();
}



void StgDirEntry::InitMembers()
{
    aSave       = aEntry;
    pUp         =
    pDown       = NULL;
    ppRoot      = NULL;
    pStgStrm    = NULL;
    pCurStrm    =
    pTmpStrm    = NULL;
    nPos        =
    nEntry      =
    nRefCnt     = 0;
    nMode       = STREAM_READ;
    bDirect     = true;
    bInvalid    =
    bCreated    =
    bRenamed    =
    bRemoved    =
    bTemp       =
    bDirty      =
    bZombie     = false;
}

StgDirEntry::~StgDirEntry()
{
    Close();
    delete pCurStrm;
    delete pStgStrm;
    delete pDown;
}



short StgDirEntry::Compare( const StgAvlNode* p ) const
{
    short nResult = -1;
    if ( p )
    {
        const StgDirEntry* pEntry = (const StgDirEntry*) p;
        nResult = aEntry.Compare( pEntry->aEntry );
    }
    return nResult;
}







void StgDirEntry::Enum( sal_Int32& n )
{
    sal_Int32 nLeft = STG_FREE, nRight = STG_FREE, nDown = STG_FREE;
    nEntry = n++;
    if( pLeft )
    {
        ((StgDirEntry*) pLeft)->Enum( n ); nLeft = ((StgDirEntry*) pLeft)->nEntry;
    }
    if( pRight )
    {
        ((StgDirEntry*) pRight)->Enum( n ); nRight = ((StgDirEntry*) pRight)->nEntry;
    }
    if( pDown )
    {
        pDown->Enum( n ); nDown = pDown->nEntry;
    }
    aSave.SetLeaf( STG_LEFT, nLeft );
    aSave.SetLeaf( STG_RIGHT, nRight );
    aSave.SetLeaf( STG_CHILD, nDown );
}




void StgDirEntry::DelTemp( bool bForce )
{
    if( pLeft )
        ((StgDirEntry*) pLeft)->DelTemp( false );
    if( pRight )
        ((StgDirEntry*) pRight)->DelTemp( false );
    if( pDown )
    {
        
        if( bInvalid && aEntry.GetType() == STG_STORAGE )
            bForce = true;
        pDown->DelTemp( bForce );
    }
    if( ( bForce || bInvalid )
     && ( aEntry.GetType() != STG_ROOT ) /* && ( nRefCnt <= 1 ) */ )
    {
        Close();
        if( pUp )
        {
            
            bool bDel = nRefCnt == 0;
            StgAvlNode::Remove( (StgAvlNode**) &pUp->pDown, this, bDel );
            if( !bDel )
            {
                pLeft = pRight = pDown = 0;
                bInvalid = bZombie = true;
            }
        }
    }
}



bool StgDirEntry::Store( StgDirStrm& rStrm )
{
    void* pEntry = rStrm.GetEntry( nEntry, true );
    if( !pEntry )
        return false;
    
    aSave.Store( pEntry );
    if( pLeft )
        if( !((StgDirEntry*) pLeft)->Store( rStrm ) )
            return false;
    if( pRight )
        if( !((StgDirEntry*) pRight)->Store( rStrm ) )
            return false;
    if( pDown )
        if( !pDown->Store( rStrm ) )
            return false;
    return true;
}

bool StgDirEntry::StoreStream( StgIo& rIo )
{
    if( aEntry.GetType() == STG_STREAM || aEntry.GetType() == STG_ROOT )
    {
        if( bInvalid )
        {
            
            if( !pStgStrm )
            {
                OpenStream( rIo );
                delete pStgStrm, pStgStrm = NULL;
            }
            else
                pStgStrm->SetSize( 0 );
        }
        
        else if( !Tmp2Strm() )
            return false;
    }
    return true;
}



bool StgDirEntry::StoreStreams( StgIo& rIo )
{
    if( !StoreStream( rIo ) )
        return false;
    if( pLeft )
        if( !((StgDirEntry*) pLeft)->StoreStreams( rIo ) )
            return false;
    if( pRight )
        if( !((StgDirEntry*) pRight)->StoreStreams( rIo ) )
            return false;
    if( pDown )
        if( !pDown->StoreStreams( rIo ) )
            return false;
    return true;
}



void StgDirEntry::RevertAll()
{
    aEntry = aSave;
    if( pLeft )
        ((StgDirEntry*) pLeft)->RevertAll();
    if( pRight )
        ((StgDirEntry*) pRight)->RevertAll();
    if( pDown )
        pDown->RevertAll();
}



bool StgDirEntry::IsDirty()
{
    if( bDirty || bInvalid )
        return true;
    if( pLeft && ((StgDirEntry*) pLeft)->IsDirty() )
        return true;
    if( pRight && ((StgDirEntry*) pRight)->IsDirty() )
        return true;
    if( pDown && pDown->IsDirty() )
        return true;
    return false;
}



void StgDirEntry::OpenStream( StgIo& rIo, bool bForceBig )
{
    sal_Int32 nThreshold = (sal_uInt16) rIo.aHdr.GetThreshold();
    delete pStgStrm;
    if( !bForceBig && aEntry.GetSize() < nThreshold )
        pStgStrm = new StgSmallStrm( rIo, *this );
    else
        pStgStrm = new StgDataStrm( rIo, *this );
    if( bInvalid && aEntry.GetSize() )
    {
        
        SetSize( 0L );

    }
    nPos = 0;
}






void StgDirEntry::Close()
{
    delete pTmpStrm;
    pTmpStrm = NULL;

    bInvalid = bTemp;
}



sal_Int32 StgDirEntry::GetSize()
{
    sal_Int32 n;
    if( pTmpStrm )
        n = pTmpStrm->GetSize();
    else if( pCurStrm )
        n = pCurStrm->GetSize();
    else n = aEntry.GetSize();
    return n;
}



bool StgDirEntry::SetSize( sal_Int32 nNewSize )
{
    if (
         !( nMode & STREAM_WRITE ) ||
         (!bDirect && !pTmpStrm && !Strm2Tmp())
       )
    {
        return false;
    }

    if( nNewSize < nPos )
        nPos = nNewSize;
    if( pTmpStrm )
    {
        pTmpStrm->SetSize( nNewSize );
        pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
        return pTmpStrm->GetError() == SVSTREAM_OK;
    }
    else
    {
        OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
        if ( !pStgStrm )
            return false;

        bool bRes = false;
        StgIo& rIo = pStgStrm->GetIo();
        sal_Int32 nThreshold = rIo.aHdr.GetThreshold();
        
        StgStrm* pOld = NULL;
        sal_uInt16 nOldSize = 0;
        if( nNewSize >= nThreshold && pStgStrm->IsSmallStrm() )
        {
            pOld = pStgStrm;
            nOldSize = (sal_uInt16) pOld->GetSize();
            pStgStrm = new StgDataStrm( rIo, STG_EOF, 0 );
        }
        else if( nNewSize < nThreshold && !pStgStrm->IsSmallStrm() )
        {
            pOld = pStgStrm;
            nOldSize = (sal_uInt16) nNewSize;
            pStgStrm = new StgSmallStrm( rIo, STG_EOF, 0 );
        }
        
        if( pStgStrm->SetSize( nNewSize ) )
        {
            
            if( pOld )
            {
                
                if( nOldSize )
                {
                    void* pBuf = new sal_uInt8[ nOldSize ];
                    pOld->Pos2Page( 0L );
                    pStgStrm->Pos2Page( 0L );
                    if( pOld->Read( pBuf, nOldSize )
                     && pStgStrm->Write( pBuf, nOldSize ) )
                        bRes = true;
                    delete[] static_cast<sal_uInt8*>(pBuf);
                }
                else
                    bRes = true;
                if( bRes )
                {
                    pOld->SetSize( 0 );
                    delete pOld;
                    pStgStrm->Pos2Page( nPos );
                    pStgStrm->SetEntry( *this );
                }
                else
                {
                    pStgStrm->SetSize( 0 );
                    delete pStgStrm;
                    pStgStrm = pOld;
                }
            }
            else
            {
                pStgStrm->Pos2Page( nPos );
                bRes = true;
            }
        }
        return bRes;
    }
}



sal_Int32 StgDirEntry::Seek( sal_Int32 nNew )
{
    if( pTmpStrm )
    {
        if( nNew < 0 )
            nNew = pTmpStrm->GetSize();
        nNew = pTmpStrm->Seek( nNew );
    }
    else if( pCurStrm )
    {
        if( nNew < 0 )
            nNew = pCurStrm->GetSize();
        nNew = pCurStrm->Seek( nNew );
    }
    else
    {
        OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
        if ( !pStgStrm )
            return nPos;

        sal_Int32 nSize = aEntry.GetSize();

        if( nNew < 0 )
            nNew = nSize;

        
        if( nNew > nSize )
        {
            if ( !( nMode & STREAM_WRITE ) || !SetSize( nNew ) )
            {
                OSL_ENSURE( nMode & STREAM_WRITE, "Trying to resize readonly stream by seeking, could be a wrong offset!" );
                return nPos;
            }
            else
                return Seek( nNew );
        }
        pStgStrm->Pos2Page( nNew );
        nNew = pStgStrm->GetPos();
    }

    return nPos = nNew;
}



sal_Int32 StgDirEntry::Read( void* p, sal_Int32 nLen )
{
    if( nLen <= 0 )
        return 0;
    if( pTmpStrm )
        nLen = pTmpStrm->Read( p, nLen );
    else if( pCurStrm )
        nLen = pCurStrm->Read( p, nLen );
    else
    {
        OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
        if ( !pStgStrm )
            return 0;

        nLen = pStgStrm->Read( p, nLen );
    }

    nPos += nLen;
    return nLen;
}



sal_Int32 StgDirEntry::Write( const void* p, sal_Int32 nLen )
{
    if( nLen <= 0 || !( nMode & STREAM_WRITE ) )
        return 0;

    
    if( bDirect && ( pCurStrm || pTmpStrm ) && !Tmp2Strm() )
        return 0;
    
    if( !bDirect && !pTmpStrm && !Strm2Tmp() )
        return 0;

    OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
    if ( !pStgStrm )
        return 0;

    if( pTmpStrm )
    {
        nLen = pTmpStrm->Write( p, nLen );
        pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
    }
    else
    {
        sal_Int32 nNew = nPos + nLen;
        if( nNew > pStgStrm->GetSize() )
        {
            if( !SetSize( nNew ) )
                return 0L;
            pStgStrm->Pos2Page( nPos );
        }
        nLen = pStgStrm->Write( p, nLen );
    }
    nPos += nLen;
    return nLen;
}

void StgDirEntry::Copy( BaseStorageStream& rDest )
{
    sal_Int32 n = GetSize();
    if( rDest.SetSize( n ) && n )
    {
        sal_uLong Pos = rDest.Tell();
        sal_uInt8 aTempBytes[ 4096 ];
        void* p = static_cast<void*>( aTempBytes );
        Seek( 0L );
        rDest.Seek( 0L );
        while( n )
        {
            sal_Int32 nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( Read( p, nn ) != nn )
                break;
            if( sal::static_int_cast<sal_Int32>(rDest.Write( p, nn )) != nn )
                break;
            n -= nn;
        }
        rDest.Seek( Pos );             
    }
}



bool StgDirEntry::Commit()
{
    

    aSave = aEntry;
    bool bRes = true;
    if( aEntry.GetType() == STG_STREAM )
    {
        if( pTmpStrm )
            delete pCurStrm, pCurStrm = pTmpStrm, pTmpStrm = NULL;
        if( bRemoved )
            
            if( pStgStrm )
                pStgStrm->SetSize( 0 );
    }
    else if( aEntry.GetType() == STG_STORAGE && bDirect && bRes )
    {
        StgIterator aIter( *this );
        for( StgDirEntry* p = aIter.First(); p && bRes; p = aIter.Next() )
            bRes = p->Commit();
    }
    return bRes;
}



bool StgDirEntry::Revert()
{
    aEntry = aSave;
    switch( aEntry.GetType() )
    {
        case STG_STREAM:
            if( pCurStrm )
                delete pTmpStrm, pTmpStrm = pCurStrm, pCurStrm = NULL;
            break;
        case STG_STORAGE:
        {
            bool bSomeRenamed = false;
            StgIterator aOIter( *this );
            StgDirEntry* op = aOIter.First();
            while( op )
            {
                op->aEntry = op->aSave;
                op->bDirty = false;
                bSomeRenamed = ( bSomeRenamed || op->bRenamed );
                
                if( op->bCreated )
                {
                    op->bCreated = false;
                    op->Close();
                    op->bInvalid = true;
                }
                
                else if( op->bRemoved )
                    op->bRemoved = op->bInvalid = op->bTemp = false;
                op = aOIter.Next();
            }
            
            if( bSomeRenamed )
            {
                StgIterator aIter( *this );
                StgDirEntry* p = aIter.First();
                while( p )
                {
                    if( p->bRenamed )
                    {
                        StgAvlNode::Move
                            ( (StgAvlNode**) &p->pUp->pDown,
                              (StgAvlNode**) &p->pUp->pDown, p );
                        p->bRenamed = false;
                    }
                    p = aIter.Next();
                }
            }
            DelTemp( false );
            break;
        }
        case STG_EMPTY:
        case STG_LOCKBYTES:
        case STG_PROPERTY:
        case STG_ROOT:
         break;
    }
    return true;
}



bool StgDirEntry::Strm2Tmp()
{
    if( !pTmpStrm )
    {
        sal_uLong n = 0;
        if( pCurStrm )
        {
            
            pTmpStrm = new StgTmpStrm;
            if( pTmpStrm->GetError() == SVSTREAM_OK && pTmpStrm->Copy( *pCurStrm ) )
                return true;
            n = 1;  
        }
        else
        {
            n = aEntry.GetSize();
            pTmpStrm = new StgTmpStrm( n );
            if( pTmpStrm->GetError() == SVSTREAM_OK )
            {
                if( n )
                {
                    OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
                    if ( !pStgStrm )
                        return false;

                    sal_uInt8 aTempBytes[ 4096 ];
                    void* p = static_cast<void*>( aTempBytes );
                    pStgStrm->Pos2Page( 0L );
                    while( n )
                    {
                        sal_uLong nn = n;
                        if( nn > 4096 )
                            nn = 4096;
                        if( (sal_uLong) pStgStrm->Read( p, nn ) != nn )
                            break;
                        if( pTmpStrm->Write( p, nn ) != nn )
                            break;
                        n -= nn;
                    }
                    pStgStrm->Pos2Page( nPos );
                    pTmpStrm->Seek( nPos );
                }
            }
            else
                n = 1;
        }

        if( n )
        {
            OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
            if ( pStgStrm )
                pStgStrm->GetIo().SetError( pTmpStrm->GetError() );

            delete pTmpStrm;
            pTmpStrm = NULL;
            return false;
        }
    }
    return true;
}



bool StgDirEntry::Tmp2Strm()
{
    
    if( !pTmpStrm )
        pTmpStrm = pCurStrm, pCurStrm = NULL;
    if( pTmpStrm )
    {
        OSL_ENSURE( pStgStrm, "The pointer may not be NULL!" );
        if ( !pStgStrm )
            return false;
        sal_uLong n = pTmpStrm->GetSize();
        StgStrm* pNewStrm;
        StgIo& rIo = pStgStrm->GetIo();
        sal_uLong nThreshold = (sal_uLong) rIo.aHdr.GetThreshold();
        if( n < nThreshold )
            pNewStrm = new StgSmallStrm( rIo, STG_EOF, 0 );
        else
            pNewStrm = new StgDataStrm( rIo, STG_EOF, 0 );
        if( pNewStrm->SetSize( n ) )
        {
            sal_uInt8 p[ 4096 ];
            pTmpStrm->Seek( 0L );
            while( n )
            {
                sal_uLong nn = n;
                if( nn > 4096 )
                    nn = 4096;
                if( pTmpStrm->Read( p, nn ) != nn )
                    break;
                if( (sal_uLong) pNewStrm->Write( p, nn ) != nn )
                    break;
                n -= nn;
            }
            if( n )
            {
                pTmpStrm->Seek( nPos );
                pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
                delete pNewStrm;
                return false;
            }
            else
            {
                pStgStrm->SetSize( 0L );
                delete pStgStrm;
                pStgStrm = pNewStrm;
                pNewStrm->SetEntry( *this );
                pNewStrm->Pos2Page( nPos );
                delete pTmpStrm;
                delete pCurStrm;
                pTmpStrm = pCurStrm = NULL;
                aSave = aEntry;
            }
        }
    }
    return true;
}



bool StgDirEntry::IsContained( StgDirEntry* pStg )
{
    if( aEntry.GetType() == STG_STORAGE )
    {
        StgIterator aIter( *this );
        StgDirEntry* p = aIter.First();
        while( p )
        {
            if( !p->aEntry.Compare( pStg->aEntry ) )
                return false;
            if( p->aEntry.GetType() == STG_STORAGE )
                if( !p->IsContained( pStg ) )
                    return false;
            p = aIter.Next();
        }
    }
    return true;
}





void StgDirEntry::Invalidate( bool bDel )
{

    if( bDel )
        bRemoved = bInvalid = true;
    switch( aEntry.GetType() )
    {
        case STG_STORAGE:
        case STG_ROOT:
        {
            StgIterator aIter( *this );
            for( StgDirEntry* p = aIter.First(); p; p = aIter.Next() )
                p->Invalidate( bDel );
            break;
        }
        default:
            break;
    }
}





StgDirStrm::StgDirStrm( StgIo& r )
          : StgDataStrm( r, r.aHdr.GetTOCStart(), -1 )
          , pRoot( NULL )
          , nEntries( 0 )
{
    if( r.GetError() )
        return;
    nEntries = nPageSize / STGENTRY_SIZE;
    if( nStart == STG_EOF )
    {
        StgEntry aRoot;
        aRoot.Init();
        aRoot.SetName( OUString("Root Entry") );
        aRoot.SetType( STG_ROOT );
        pRoot = new StgDirEntry( aRoot );
        pRoot->SetDirty();
    }
    else
    {
        
        
        pEntry = (StgDirEntry*) this; 
        SetupEntry( 0, pRoot );
        pEntry = NULL;
    }
}

StgDirStrm::~StgDirStrm()
{
    delete pRoot;
}



void StgDirStrm::SetupEntry( sal_Int32 n, StgDirEntry* pUpper )
{
    void* p = ( n == STG_FREE ) ? NULL : GetEntry( n );
    if( p )
    {
        bool bOk(false);
        StgDirEntry* pCur = new StgDirEntry( p, STGENTRY_SIZE, &bOk );

        if( !bOk )
        {
            delete pCur;
            rIo.SetError( SVSTREAM_GENERALERROR );
            
            return;
        }

        
        if( !pUpper )
            pCur->aEntry.SetType( STG_ROOT );

        sal_Int32 nLeft = pCur->aEntry.GetLeaf( STG_LEFT );
        sal_Int32 nRight = pCur->aEntry.GetLeaf( STG_RIGHT );
        
        sal_Int32 nLeaf = STG_FREE;
        if( pCur->aEntry.GetType() == STG_STORAGE || pCur->aEntry.GetType() == STG_ROOT )
        {
            nLeaf = pCur->aEntry.GetLeaf( STG_CHILD );
            if (nLeaf != STG_FREE && nLeaf == n)
            {
                delete pCur;
                rIo.SetError( SVSTREAM_GENERALERROR );
                return;
            }
        }

        if( nLeaf != 0 && nLeft != 0 && nRight != 0 )
        {
            
            if (pUpper)
            {
                if (pUpper->aEntry.GetLeaf(STG_CHILD) == nLeaf)
                {
                    OSL_FAIL("Leaf node of upper StgDirEntry is same as current StgDirEntry's leaf node. Circular entry chain, discarding link");
                    delete pCur;
                    return;
                }

                StgDirEntry *pUpperUpper = pUpper->pUp;
                if (pUpperUpper && pUpperUpper->aEntry.GetLeaf(STG_CHILD) == nLeaf)
                {
                    OSL_FAIL("Leaf node of upper-upper StgDirEntry is same as current StgDirEntry's leaf node. Circular entry chain, discarding link");
                    delete pCur;
                    return;
                }
            }

            if( StgAvlNode::Insert
                ( (StgAvlNode**) ( pUpper ? &pUpper->pDown : &pRoot ), pCur ) )
            {
                pCur->pUp    = pUpper;
                pCur->ppRoot = &pRoot;
            }
            else
            {
                
                
                
                
                delete pCur;
                return;
            }
            SetupEntry( nLeft, pUpper );
            SetupEntry( nRight, pUpper );
            SetupEntry( nLeaf, pCur );
        }
        else
        {
            delete pCur;
        }
    }
}



bool StgDirStrm::SetSize( sal_Int32 nBytes )
{
    
    if ( nBytes < 0 )
        nBytes = 0;

    nBytes = ( ( nBytes + nPageSize - 1 ) / nPageSize ) * nPageSize;
    return StgStrm::SetSize( nBytes );
}



bool StgDirStrm::Store()
{
    if( !pRoot || !pRoot->IsDirty() )
        return true;
    if( !pRoot->StoreStreams( rIo ) )
        return false;
    
    
    pRoot->Commit();
    
    sal_Int32 nOldStart = nStart;       
    sal_Int32 nOldSize  = nSize;
    nStart = nPage = STG_EOF;
    nSize  = nPos = 0;
    nOffset = 0;
    
    pRoot->DelTemp( false );
    
    sal_Int32 n = 0;
    pRoot->Enum( n );
    if( !SetSize( n * STGENTRY_SIZE ) )
    {
        nStart = nOldStart; nSize = nOldSize;
        pRoot->RevertAll();
        return false;
    }
    
    if( !Copy( STG_FREE, nSize ) )
    {
        pRoot->RevertAll();
        return false;
    }
    
    if( !pRoot->Store( *this ) )
    {
        pRoot->RevertAll();
        return false;
    }
    
    sal_Int32 ne = nSize / STGENTRY_SIZE;
    StgEntry aEmpty;
    aEmpty.Init();
    while( n < ne )
    {
        void* p = GetEntry( n++, true );
        if( !p )
        {
            pRoot->RevertAll();
            return false;
        }
        aEmpty.Store( p );
    }
    
    pFat->FreePages( nOldStart, true );
    rIo.aHdr.SetTOCStart( nStart );
    return true;
}



void* StgDirStrm::GetEntry( sal_Int32 n, bool bDirty )
{
    if( n < 0 )
        return NULL;

    n *= STGENTRY_SIZE;
    if( n >= nSize )
        return NULL;
    return GetPtr( n, true, bDirty );
}



StgDirEntry* StgDirStrm::Find( StgDirEntry& rStg, const OUString& rName )
{
    if( rStg.pDown )
    {
        StgEntry aEntry;
        aEntry.Init();
        if( !aEntry.SetName( rName ) )
        {
            rIo.SetError( SVSTREAM_GENERALERROR );
            return NULL;
        }
        
        StgDirEntry aTest( aEntry );
        return (StgDirEntry*) rStg.pDown->Find( &aTest );
    }
    else
        return NULL;
}



StgDirEntry* StgDirStrm::Create( StgDirEntry& rStg, const OUString& rName, StgEntryType eType )
{
    StgEntry aEntry;
    aEntry.Init();
    aEntry.SetType( eType );
    if( !aEntry.SetName( rName ) )
    {
        rIo.SetError( SVSTREAM_GENERALERROR );
        return NULL;
    }
    StgDirEntry* pRes = Find( rStg, rName );
    if( pRes )
    {
        if( !pRes->bInvalid )
        {
            rIo.SetError( SVSTREAM_CANNOT_MAKE );
            return NULL;
        }
        pRes->bInvalid =
        pRes->bRemoved =
        pRes->bTemp    = false;
        pRes->bCreated =
        pRes->bDirty   = true;
    }
    else
    {
        pRes = new StgDirEntry( aEntry );
        if( StgAvlNode::Insert( (StgAvlNode**) &rStg.pDown, pRes ) )
        {
            pRes->pUp    = &rStg;
            pRes->ppRoot = &pRoot;
            pRes->bCreated =
            pRes->bDirty = true;
        }
        else
        {
            rIo.SetError( SVSTREAM_CANNOT_MAKE );
            delete pRes; pRes = NULL;
        }
    }
    return pRes;
}



bool StgDirStrm::Rename( StgDirEntry& rStg, const OUString& rOld, const OUString& rNew )
{
    StgDirEntry* p = Find( rStg, rOld );
    if( p )
    {

        if( !StgAvlNode::Remove( (StgAvlNode**) &rStg.pDown, p, false ) )
            return false;
        p->aEntry.SetName( rNew );
        if( !StgAvlNode::Insert( (StgAvlNode**) &rStg.pDown, p ) )
            return false;
        p->bRenamed = p->bDirty   = true;
        return true;
    }
    else
    {
        rIo.SetError( SVSTREAM_FILE_NOT_FOUND );
        return false;
    }
}



bool StgDirStrm::Move( StgDirEntry& rStg1, StgDirEntry& rStg2, const OUString& rName )
{
    StgDirEntry* p = Find( rStg1, rName );
    if( p )
    {
        if( !StgAvlNode::Move
            ( (StgAvlNode**) &rStg1.pDown, (StgAvlNode**) &rStg2.pDown, p ) )
            return false;
        p->bDirty = true;
        return true;
    }
    else
    {
        rIo.SetError( SVSTREAM_FILE_NOT_FOUND );
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
