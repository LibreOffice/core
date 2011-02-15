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
#include "precompiled_sot.hxx"

#include <string.h>     // memcpy()

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"


//////////////////////////// class StgDirEntry /////////////////////////////

// This class holds the dir entry data and maintains dirty flags for both
// the entry and the data.

// Transacted mode for streams: On the first write, a temp stream pTmpStrm
// is created and operated on. A commit moves pTmpStrm to pCurStrm, which
// is used for subsequent reads. A new write creates a new copy of pTmpStrm
// based on pCurStrm. Reverting throws away pTmpStrm.
// Transacted mode for storages: A copy of the dir ents is kept in aSave.
// Committing means copying aEntry to aSave. Reverting means to copy aSave
// to aEntry, delete newly created entries and to reactivate removed entries.

// Problem der Implementation: Keine Hierarchischen commits. Daher nur
// insgesamt transaktionsorientert oder direkt.

StgDirEntry::StgDirEntry( const void* pFrom, sal_Bool * pbOk ) : StgAvlNode()
{
    *pbOk = aEntry.Load( pFrom );

    InitMembers();
}

StgDirEntry::StgDirEntry( const StgEntry& r ) : StgAvlNode(), aEntry( r )
{
    InitMembers();
}

// Helper for all ctors

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
    bDirect     = sal_True;
    bInvalid    =
    bCreated    =
    bRenamed    =
    bRemoved    =
    bTemp       =
    bDirty      =
    bZombie     = sal_False;
}

StgDirEntry::~StgDirEntry()
{
    Close();
    delete pCurStrm;
    delete pStgStrm;
    delete pDown;
}

// Comparison function

short StgDirEntry::Compare( const StgAvlNode* p ) const
{
    const StgDirEntry* pEntry = (const StgDirEntry*) p;
    return aEntry.Compare( pEntry->aEntry );
}

// Enumerate the entry numbers.
// n is incremented to show the total # of entries.
// These number are later used as page numbers when storing
// the TOC tree into the TOC stream. Remember that aSave is
// stored, not aEntry.

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

// Delete all temporary entries before writing the TOC stream.
// Until now Deltem is never called with bForce True

void StgDirEntry::DelTemp( sal_Bool bForce )
{
    if( pLeft )
        ((StgDirEntry*) pLeft)->DelTemp( sal_False );
    if( pRight )
        ((StgDirEntry*) pRight)->DelTemp( sal_False );
    if( pDown )
    {
        // If the storage is dead, of course all elements are dead, too
        if( bInvalid && aEntry.GetType() == STG_STORAGE )
            bForce = sal_True;
        pDown->DelTemp( bForce );
    }
    if( ( bForce || bInvalid )
     && ( aEntry.GetType() != STG_ROOT ) /* && ( nRefCnt <= 1 ) */ )
    {
        Close();
        if( pUp )
        {
            // this deletes the element if refcnt == 0!
            sal_Bool bDel = nRefCnt == 0;
            StgAvlNode::Remove( (StgAvlNode**) &pUp->pDown, this, bDel );
            if( !bDel )
            {
                pLeft = pRight = pDown = 0;
                bInvalid = bZombie = sal_True;
            }
        }
    }
}

// Save the tree into the given dir stream

sal_Bool StgDirEntry::Store( StgDirStrm& rStrm )
{
    void* pEntry = rStrm.GetEntry( nEntry, sal_True );
    if( !pEntry )
        return sal_False;
    // Do not store the current (maybe not commited) entry
    aSave.Store( pEntry );
    if( pLeft )
        if( !((StgDirEntry*) pLeft)->Store( rStrm ) )
            return sal_False;
    if( pRight )
        if( !((StgDirEntry*) pRight)->Store( rStrm ) )
            return sal_False;
    if( pDown )
        if( !pDown->Store( rStrm ) )
            return sal_False;
    return sal_True;
}

sal_Bool StgDirEntry::StoreStream( StgIo& rIo )
{
    if( aEntry.GetType() == STG_STREAM || aEntry.GetType() == STG_ROOT )
    {
        if( bInvalid )
        {
            // Delete the stream if needed
            if( !pStgStrm )
            {
                OpenStream( rIo );
                delete pStgStrm, pStgStrm = NULL;
            }
            else
                pStgStrm->SetSize( 0 );
        }
        // or write the data stream
        else if( !Tmp2Strm() )
            return sal_False;
    }
    return sal_True;
}

// Save all dirty streams

sal_Bool StgDirEntry::StoreStreams( StgIo& rIo )
{
    if( !StoreStream( rIo ) )
        return sal_False;
    if( pLeft )
        if( !((StgDirEntry*) pLeft)->StoreStreams( rIo ) )
            return sal_False;
    if( pRight )
        if( !((StgDirEntry*) pRight)->StoreStreams( rIo ) )
            return sal_False;
    if( pDown )
        if( !pDown->StoreStreams( rIo ) )
            return sal_False;
    return sal_True;
}

// Revert all directory entries after failure to write the TOC stream

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

// Look if any element of the tree is dirty

sal_Bool StgDirEntry::IsDirty()
{
    if( bDirty || bInvalid )
        return sal_True;
    if( pLeft && ((StgDirEntry*) pLeft)->IsDirty() )
        return sal_True;
    if( pRight && ((StgDirEntry*) pRight)->IsDirty() )
        return sal_True;
    if( pDown && pDown->IsDirty() )
        return sal_True;
    return sal_False;
}

// Set up a stream.

void StgDirEntry::OpenStream( StgIo& rIo, sal_Bool bForceBig )
{
    sal_Int32 nThreshold = (sal_uInt16) rIo.aHdr.GetThreshold();
    delete pStgStrm;
    if( !bForceBig && aEntry.GetSize() < nThreshold )
        pStgStrm = new StgSmallStrm( rIo, this );
    else
        pStgStrm = new StgDataStrm( rIo, this );
    if( bInvalid && aEntry.GetSize() )
    {
        // This entry has invalid data, so delete that data
        SetSize( 0L );
//      bRemoved = bInvalid = sal_False;
    }
    nPos = 0;
}

// Close the open stream without committing. If the entry is marked as
// temporary, delete it.
// Do not delete pCurStrm here!
// (TLX:??? Zumindest pStgStrm muss deleted werden.)

void StgDirEntry::Close()
{
    delete pTmpStrm;
    pTmpStrm = NULL;
//  nRefCnt  = 0;
    bInvalid = bTemp;
}

// Get the current stream size

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

// Set the stream size. This means also creating a temp stream.

sal_Bool StgDirEntry::SetSize( sal_Int32 nNewSize )
{
    if (
         !( nMode & STREAM_WRITE ) ||
         (!bDirect && !pTmpStrm && !Strm2Tmp())
       )
    {
        return sal_False;
    }

    if( nNewSize < nPos )
        nPos = nNewSize;
    if( pTmpStrm )
    {
        pTmpStrm->SetSize( nNewSize );
        pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
        return sal_Bool( pTmpStrm->GetError() == SVSTREAM_OK );
    }
    else
    {
        sal_Bool bRes = sal_False;
        StgIo& rIo = pStgStrm->GetIo();
        sal_Int32 nThreshold = rIo.aHdr.GetThreshold();
        // ensure the correct storage stream!
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
        // now set the new size
        if( pStgStrm->SetSize( nNewSize ) )
        {
            // did we create a new stream?
            if( pOld )
            {
                // if so, we probably need to copy the old data
                if( nOldSize )
                {
                    void* pBuf = new sal_uInt8[ nOldSize ];
                    pOld->Pos2Page( 0L );
                    pStgStrm->Pos2Page( 0L );
                    if( pOld->Read( pBuf, nOldSize )
                     && pStgStrm->Write( pBuf, nOldSize ) )
                        bRes = sal_True;
                    delete[] static_cast<sal_uInt8*>(pBuf);
                }
                else
                    bRes = sal_True;
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
                bRes = sal_True;
            }
        }
        return bRes;
    }
}

// Seek. On negative values, seek to EOF.

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
        sal_Int32 nSize = aEntry.GetSize();

        if( nNew < 0 )
            nNew = nSize;

        // try to enlarge, the readonly streams should not allow this
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

// Read

sal_Int32 StgDirEntry::Read( void* p, sal_Int32 nLen )
{
    if( nLen <= 0 )
        return 0;
    if( pTmpStrm )
        nLen = pTmpStrm->Read( p, nLen );
    else if( pCurStrm )
        nLen = pCurStrm->Read( p, nLen );
    else
        nLen = pStgStrm->Read( p, nLen );
    nPos += nLen;
    return nLen;
}

// Write

sal_Int32 StgDirEntry::Write( const void* p, sal_Int32 nLen )
{
    if( nLen <= 0 || !( nMode & STREAM_WRITE ) )
        return 0;

    // Was this stream committed internally and reopened in direct mode?
    if( bDirect && ( pCurStrm || pTmpStrm ) && !Tmp2Strm() )
        return 0;
    // Is this stream opened in transacted mode? Do we have to make a copy?
    if( !bDirect && !pTmpStrm && !Strm2Tmp() )
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

// Copy the data of one entry into another entry.

void StgDirEntry::Copy( StgDirEntry& rDest )
{
    sal_Int32 n = GetSize();
    if( rDest.SetSize( n ) && n )
    {
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
            if( rDest.Write( p, nn ) != nn )
                break;
            n -= nn;
        }
    }
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
        rDest.Seek( Pos );             // ?! Seems to be undocumented !
    }
}

// Commit this entry

sal_Bool StgDirEntry::Commit()
{
    // OSL_ENSURE( nMode & STREAM_WRITE, "Trying to commit readonly stream!" );

    aSave = aEntry;
    sal_Bool bRes = sal_True;
    if( aEntry.GetType() == STG_STREAM )
    {
        if( pTmpStrm )
            delete pCurStrm, pCurStrm = pTmpStrm, pTmpStrm = NULL;
        if( bRemoved )
            // Delete the stream if needed
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

// Revert the entry

sal_Bool StgDirEntry::Revert()
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
            sal_Bool bSomeRenamed = sal_False;
            StgIterator aOIter( *this );
            StgDirEntry* op = aOIter.First();
            while( op )
            {
                op->aEntry = op->aSave;
                op->bDirty = sal_False;
                bSomeRenamed = sal_Bool( bSomeRenamed | op->bRenamed );
                // Remove any new entries
                if( op->bCreated )
                {
                    op->bCreated = sal_False;
                    op->Close();
                    op->bInvalid = sal_True;
                }
                // Reactivate any removed entries
                else if( op->bRemoved )
                    op->bRemoved = op->bInvalid = op->bTemp = sal_False;
                op = aOIter.Next();
            }
            // Resort all renamed entries
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
                        p->bRenamed = sal_False;
                    }
                    p = aIter.Next();
                }
            }
            DelTemp( sal_False );
            break;
        }
        case STG_EMPTY:
        case STG_LOCKBYTES:
        case STG_PROPERTY:
        case STG_ROOT:
         break;
    }
    return sal_True;
}

// Copy the stg stream to the temp stream

sal_Bool StgDirEntry::Strm2Tmp()
{
    if( !pTmpStrm )
    {
        sal_uLong n = 0;
        if( pCurStrm )
        {
            // It was already commited once
            pTmpStrm = new StgTmpStrm;
            if( pTmpStrm->GetError() == SVSTREAM_OK && pTmpStrm->Copy( *pCurStrm ) )
                return sal_True;
            n = 1;  // indicates error
        }
        else
        {
            n = aEntry.GetSize();
            pTmpStrm = new StgTmpStrm( n );
            if( pTmpStrm->GetError() == SVSTREAM_OK )
            {
                if( n )
                {
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
            pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
            delete pTmpStrm;
            pTmpStrm = NULL;
            return sal_False;
        }
    }
    return sal_True;
}

// Copy the temp stream to the stg stream during the final commit

sal_Bool StgDirEntry::Tmp2Strm()
{
    // We did commit once, but have not written since then
    if( !pTmpStrm )
        pTmpStrm = pCurStrm, pCurStrm = NULL;
    if( pTmpStrm )
    {
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
                return sal_False;
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
    return sal_True;
}

// Check if the given entry is contained in this entry

sal_Bool StgDirEntry::IsContained( StgDirEntry* pStg )
{
    if( aEntry.GetType() == STG_STORAGE )
    {
        StgIterator aIter( *this );
        StgDirEntry* p = aIter.First();
        while( p )
        {
            if( !p->aEntry.Compare( pStg->aEntry ) )
                return sal_False;
            if( p->aEntry.GetType() == STG_STORAGE )
                if( !p->IsContained( pStg ) )
                    return sal_False;
            p = aIter.Next();
        }
    }
    return sal_True;
}

// Invalidate all open entries by setting the RefCount to 0. If the bDel
// flag is set, also set the invalid flag to indicate deletion during the
// next dir stream flush.

void StgDirEntry::Invalidate( sal_Bool bDel )
{
//  nRefCnt = 0;
    if( bDel )
        bRemoved = bInvalid = sal_True;
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

///////////////////////////// class StgDirStrm ////////////////////////////

// This specialized stream is the maintenance stream for the directory tree.

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
        aRoot.SetName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Root Entry" ) ) );
        aRoot.SetType( STG_ROOT );
        pRoot = new StgDirEntry( aRoot );
        pRoot->SetDirty();
    }
    else
    {
        // temporarily use this instance as owner, so
        // the TOC pages can be removed.
        pEntry = (StgDirEntry*) this; // just for a bit pattern
        SetupEntry( 0, pRoot );
        rIo.Revert( pEntry );
        pEntry = NULL;
    }
}

StgDirStrm::~StgDirStrm()
{
    delete pRoot;
}

// Recursively parse the directory tree during reading the TOC stream

void StgDirStrm::SetupEntry( sal_Int32 n, StgDirEntry* pUpper )
{
    void* p = ( n == STG_FREE ) ? NULL : GetEntry( n );
    if( p )
    {
        sal_Bool bOk(sal_False);
        StgDirEntry* pCur = new StgDirEntry( p, &bOk );

        if( !bOk )
        {
            delete pCur;
            rIo.SetError( SVSTREAM_GENERALERROR );
            // an error occured
            return;
        }

        // better it is
        if( !pUpper )
            pCur->aEntry.SetType( STG_ROOT );

        sal_Int32 nLeft = pCur->aEntry.GetLeaf( STG_LEFT );
        sal_Int32 nRight = pCur->aEntry.GetLeaf( STG_RIGHT );
        // substorage?
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
            if( StgAvlNode::Insert
                ( (StgAvlNode**) ( pUpper ? &pUpper->pDown : &pRoot ), pCur ) )
            {
                pCur->pUp    = pUpper;
                pCur->ppRoot = &pRoot;
            }
            else
            {
                rIo.SetError( SVSTREAM_CANNOT_MAKE );
                delete pCur; pCur = NULL;
                return;
            }
            SetupEntry( nLeft, pUpper );
            SetupEntry( nRight, pUpper );
            SetupEntry( nLeaf, pCur );
        }
    }
}

// Extend or shrink the directory stream.

sal_Bool StgDirStrm::SetSize( sal_Int32 nBytes )
{
    // Always allocate full pages
    nBytes = ( ( nBytes + nPageSize - 1 ) / nPageSize ) * nPageSize;
    return StgStrm::SetSize( nBytes );
}

// Save the TOC stream into a new substream after saving all data streams

sal_Bool StgDirStrm::Store()
{
    if( !pRoot->IsDirty() )
        return sal_True;
    if( !pRoot->StoreStreams( rIo ) )
        return sal_False;
    // After writing all streams, the data FAT stream has changed,
    // so we have to commit the root again
    pRoot->Commit();
    // We want a completely new stream, so fake an empty stream
    sal_Int32 nOldStart = nStart;       // save for later deletion
    sal_Int32 nOldSize  = nSize;
    nStart = nPage = STG_EOF;
    nSize  = nPos = 0;
    nOffset = 0;
    // Delete all temporary entries
    pRoot->DelTemp( sal_False );
    // set the entry numbers
    sal_Int32 n = 0;
    pRoot->Enum( n );
    if( !SetSize( n * STGENTRY_SIZE ) )
    {
        nStart = nOldStart; nSize = nOldSize;
        pRoot->RevertAll();
        return sal_False;
    }
    // set up the cache elements for the new stream
    if( !Copy( STG_FREE, nSize ) )
    {
        pRoot->RevertAll();
        return sal_False;
    }
    // Write the data to the new stream
    if( !pRoot->Store( *this ) )
    {
        pRoot->RevertAll();
        return sal_False;
    }
    // fill any remaining entries with empty data
    sal_Int32 ne = nSize / STGENTRY_SIZE;
    StgEntry aEmpty;
    aEmpty.Init();
    while( n < ne )
    {
        void* p = GetEntry( n++, sal_True );
        if( !p )
        {
            pRoot->RevertAll();
            return sal_False;
        }
        aEmpty.Store( p );
    }
    // Now we can release the old stream
    pFat->FreePages( nOldStart, sal_True );
    rIo.aHdr.SetTOCStart( nStart );
    return sal_True;
}

// Get a dir entry.

void* StgDirStrm::GetEntry( sal_Int32 n, sal_Bool bDirty )
{
    if( n < 0 )
        return NULL;

    n *= STGENTRY_SIZE;
    if( n < 0 && n >= nSize )
        return NULL;
    return GetPtr( n, sal_True, bDirty );
}

// Find a dir entry.

StgDirEntry* StgDirStrm::Find( StgDirEntry& rStg, const String& rName )
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
        // Look in the directory attached to the entry
        StgDirEntry aTest( aEntry );
        return (StgDirEntry*) rStg.pDown->Find( &aTest );
    }
    else
        return NULL;
}

// Create a new entry.

StgDirEntry* StgDirStrm::Create
    ( StgDirEntry& rStg, const String& rName, StgEntryType eType )
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
        pRes->bTemp    = sal_False;
        pRes->bCreated =
        pRes->bDirty   = sal_True;
    }
    else
    {
        pRes = new StgDirEntry( aEntry );
        if( StgAvlNode::Insert( (StgAvlNode**) &rStg.pDown, pRes ) )
        {
            pRes->pUp    = &rStg;
            pRes->ppRoot = &pRoot;
            pRes->bCreated =
            pRes->bDirty = sal_True;
        }
        else
        {
            rIo.SetError( SVSTREAM_CANNOT_MAKE );
            delete pRes; pRes = NULL;
        }
    }
    return pRes;
}

// Rename the given entry.

sal_Bool StgDirStrm::Rename( StgDirEntry& rStg, const String& rOld, const String& rNew )
{
    StgDirEntry* p = Find( rStg, rOld );
    if( p )
    {

        if( !StgAvlNode::Remove( (StgAvlNode**) &rStg.pDown, p, sal_False ) )
            return sal_False;
        p->aEntry.SetName( rNew );
        if( !StgAvlNode::Insert( (StgAvlNode**) &rStg.pDown, p ) )
            return sal_False;
        p->bRenamed = p->bDirty   = sal_True;
        return sal_True;
    }
    else
    {
        rIo.SetError( SVSTREAM_FILE_NOT_FOUND );
        return sal_False;
    }
}

// Move the given entry to a different storage.

sal_Bool StgDirStrm::Move( StgDirEntry& rStg1, StgDirEntry& rStg2, const String& rName )
{
    StgDirEntry* p = Find( rStg1, rName );
    if( p )
    {
        if( !StgAvlNode::Move
            ( (StgAvlNode**) &rStg1.pDown, (StgAvlNode**) &rStg2.pDown, p ) )
            return sal_False;
        p->bDirty = sal_True;
        return sal_True;
    }
    else
    {
        rIo.SetError( SVSTREAM_FILE_NOT_FOUND );
        return sal_False;
    }
}

