/*************************************************************************
 *
 *  $RCSfile: stgdir.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-20 12:56:37 $
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

#include <string.h>     // memcpy()

#include "stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#pragma hdrstop

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

StgDirEntry::StgDirEntry( const void* pFrom, BOOL * pbOk ) : StgAvlNode()
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
    bDirect     = TRUE;
    bInvalid    =
    bCreated    =
    bRenamed    =
    bRemoved    =
    bTemp       =
    bDirty      =
    bZombie     = FALSE;
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

void StgDirEntry::Enum( INT32& n )
{
    INT32 nLeft = STG_FREE, nRight = STG_FREE, nDown = STG_FREE;
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

void StgDirEntry::DelTemp( BOOL bForce )
{
    if( pLeft )
        ((StgDirEntry*) pLeft)->DelTemp( FALSE );
    if( pRight )
        ((StgDirEntry*) pRight)->DelTemp( FALSE );
    if( pDown )
    {
        // If the storage is dead, of course all elements are dead, too
        if( bInvalid && aEntry.GetType() == STG_STORAGE )
            bForce = TRUE;
        pDown->DelTemp( bForce );
    }
    if( ( bForce || bInvalid )
     && ( aEntry.GetType() != STG_ROOT ) /* && ( nRefCnt <= 1 ) */ )
    {
        Close();
        if( pUp )
        {
            // this deletes the element if refcnt == 0!
            BOOL bDel = nRefCnt == 0;
            StgAvlNode::Remove( (StgAvlNode**) &pUp->pDown, this, bDel );
            if( !bDel )
            {
                pLeft = pRight = pDown = 0;
                bInvalid = bZombie = TRUE;
            }
        }
    }
}

// Save the tree into the given dir stream

BOOL StgDirEntry::Store( StgDirStrm& rStrm )
{
    void* pEntry = rStrm.GetEntry( nEntry, TRUE );
    if( !pEntry )
        return FALSE;
    // Do not store the current (maybe not commited) entry
    aSave.Store( pEntry );
    if( pLeft )
        if( !((StgDirEntry*) pLeft)->Store( rStrm ) )
            return FALSE;
    if( pRight )
        if( !((StgDirEntry*) pRight)->Store( rStrm ) )
            return FALSE;
    if( pDown )
        if( !pDown->Store( rStrm ) )
            return FALSE;
    return TRUE;
}

BOOL StgDirEntry::StoreStream( StgIo& rIo )
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
            return FALSE;
    }
    return TRUE;
}

// Save all dirty streams

BOOL StgDirEntry::StoreStreams( StgIo& rIo )
{
    if( !StoreStream( rIo ) )
        return FALSE;
    if( pLeft )
        if( !((StgDirEntry*) pLeft)->StoreStreams( rIo ) )
            return FALSE;
    if( pRight )
        if( !((StgDirEntry*) pRight)->StoreStreams( rIo ) )
            return FALSE;
    if( pDown )
        if( !pDown->StoreStreams( rIo ) )
            return FALSE;
    return TRUE;
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

BOOL StgDirEntry::IsDirty()
{
    if( bDirty || bInvalid )
        return TRUE;
    if( pLeft && ((StgDirEntry*) pLeft)->IsDirty() )
        return TRUE;
    if( pRight && ((StgDirEntry*) pRight)->IsDirty() )
        return TRUE;
    if( pDown && pDown->IsDirty() )
        return TRUE;
    return FALSE;
}

// Set up a stream.

void StgDirEntry::OpenStream( StgIo& rIo, BOOL bForceBig )
{
    short nThreshold = (USHORT) rIo.aHdr.GetThreshold();
    delete pStgStrm;
    if( !bForceBig && aEntry.GetSize() < (INT32) nThreshold )
        pStgStrm = new StgSmallStrm( rIo, this );
    else
        pStgStrm = new StgDataStrm( rIo, this );
    if( bInvalid && aEntry.GetSize() )
    {
        // This entry has invalid data, so delete that data
        SetSize( 0L );
//      bRemoved = bInvalid = FALSE;
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

INT32 StgDirEntry::GetSize()
{
    INT32 n;
    if( pTmpStrm )
        n = pTmpStrm->GetSize();
    else if( pCurStrm )
        n = pCurStrm->GetSize();
    else n = aEntry.GetSize();
    return n;
}

// Set the stream size. This means also creating a temp stream.

BOOL StgDirEntry::SetSize( INT32 nNewSize )
{
    if( !bDirect && !pTmpStrm && !Strm2Tmp() )
        return FALSE;
    if( nNewSize < nPos )
        nPos = nNewSize;
    if( pTmpStrm )
    {
        pTmpStrm->SetSize( nNewSize );
        pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
        return BOOL( pTmpStrm->GetError() == SVSTREAM_OK );
    }
    else
    {
        BOOL bRes = FALSE;
        StgIo& rIo = pStgStrm->GetIo();
        short nThreshold = (USHORT) rIo.aHdr.GetThreshold();
        // ensure the correct storage stream!
        StgStrm* pOld = NULL;
        USHORT nOldSize = 0;
        if( nNewSize > nThreshold && pStgStrm->IsSmallStrm() )
        {
            pOld = pStgStrm;
            nOldSize = (USHORT) pOld->GetSize();
            pStgStrm = new StgDataStrm( rIo, STG_EOF, 0 );
        }
        else if( nNewSize < nThreshold && !pStgStrm->IsSmallStrm() )
        {
            pOld = pStgStrm;
            nOldSize = (USHORT) nNewSize;
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
                    void* pBuf = new BYTE[ nOldSize ];
                    pOld->Pos2Page( 0L );
                    pStgStrm->Pos2Page( 0L );
                    if( pOld->Read( pBuf, nOldSize )
                     && pStgStrm->Write( pBuf, nOldSize ) )
                        bRes = TRUE;
                    delete pBuf;
                }
                else
                    bRes = TRUE;
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
                bRes = TRUE;
            }
        }
        return bRes;
    }
}

// Seek. On negative values, seek to EOF.

INT32 StgDirEntry::Seek( INT32 nNew )
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
        INT32 nSize = aEntry.GetSize();
        if( nNew < 0 )
            nNew = nSize;
        // enlarge?
        if( nNew > nSize )
        {
            if( !SetSize( nNew ) )
                return nPos;
            else
                return Seek( nNew );
        }
        pStgStrm->Pos2Page( nNew );
        nNew = pStgStrm->GetPos();
    }
    return nPos = nNew;
}

// Read

INT32 StgDirEntry::Read( void* p, INT32 nLen )
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

INT32 StgDirEntry::Write( const void* p, INT32 nLen )
{
    if( nLen <= 0 )
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
        INT32 nNew = nPos + nLen;
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
    INT32 n = GetSize();
    if( rDest.SetSize( n ) && n )
    {
        void* p = new BYTE[ 4096 ];
        Seek( 0L );
        rDest.Seek( 0L );
        while( n )
        {
            INT32 nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( Read( p, nn ) != nn )
                break;
            if( rDest.Write( p, nn ) != nn )
                break;
            n -= nn;
        }
        delete p;
    }
}

void StgDirEntry::Copy( BaseStorageStream& rDest )
{
    INT32 n = GetSize();
    if( rDest.SetSize( n ) && n )
    {
        ULONG nPos = rDest.Tell();
        void* p = new BYTE[ 4096 ];
        Seek( 0L );
        rDest.Seek( 0L );
        while( n )
        {
            INT32 nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( Read( p, nn ) != nn )
                break;
            if( rDest.Write( p, nn ) != nn )
                break;
            n -= nn;
        }
        delete p;
        rDest.Seek( nPos );             // ?! Seems to be undocumented !
    }
}

// Commit this entry

BOOL StgDirEntry::Commit()
{
    aSave = aEntry;
    BOOL bRes = TRUE;
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

BOOL StgDirEntry::Revert()
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
            BOOL bSomeRenamed = FALSE;
            StgIterator aIter( *this );
            StgDirEntry* p = aIter.First();
            while( p )
            {
                p->aEntry = p->aSave;
                p->bDirty = FALSE;
                bSomeRenamed = BOOL( bSomeRenamed | p->bRenamed );
                // Remove any new entries
                if( p->bCreated )
                {
                    p->bCreated = FALSE;
                    p->Close();
                    p->bInvalid = TRUE;
                }
                // Reactivate any removed entries
                else if( p->bRemoved )
                    p->bRemoved = p->bInvalid = p->bTemp = FALSE;
                p = aIter.Next();
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
                        p->bRenamed = FALSE;
                    }
                    p = aIter.Next();
                }
            }
            DelTemp( FALSE );
            break;
        }
    }
    return TRUE;
}

// Copy the stg stream to the temp stream

BOOL StgDirEntry::Strm2Tmp()
{
    if( !pTmpStrm )
    {
        ULONG n = 0;
        if( pCurStrm )
        {
            // It was already commited once
            pTmpStrm = new StgTmpStrm;
            if( pTmpStrm->GetError() == SVSTREAM_OK && pTmpStrm->Copy( *pCurStrm ) )
                return TRUE;
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
                    void* p = new BYTE[ 4096 ];
                    pStgStrm->Pos2Page( 0L );
                    while( n )
                    {
                        ULONG nn = n;
                        if( nn > 4096 )
                            nn = 4096;
                        if( (ULONG) pStgStrm->Read( p, nn ) != nn )
                            break;
                        if( pTmpStrm->Write( p, nn ) != nn )
                            break;
                        n -= nn;
                    }
                    delete p;
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
            return FALSE;
        }
    }
    return TRUE;
}

// Copy the temp stream to the stg stream during the final commit

BOOL StgDirEntry::Tmp2Strm()
{
    // We did commit once, but have not written since then
    if( !pTmpStrm )
        pTmpStrm = pCurStrm, pCurStrm = NULL;
    if( pTmpStrm )
    {
        ULONG n = pTmpStrm->GetSize();
        StgStrm* pNewStrm;
        StgIo& rIo = pStgStrm->GetIo();
        ULONG nThreshold = (ULONG) rIo.aHdr.GetThreshold();
        if( n < nThreshold )
            pNewStrm = new StgSmallStrm( rIo, STG_EOF, 0 );
        else
            pNewStrm = new StgDataStrm( rIo, STG_EOF, 0 );
        if( pNewStrm->SetSize( n ) )
        {
            void* p = new BYTE[ 4096 ];
            pTmpStrm->Seek( 0L );
            while( n )
            {
                ULONG nn = n;
                if( nn > 4096 )
                    nn = 4096;
                if( pTmpStrm->Read( p, nn ) != nn )
                    break;
                if( (ULONG) pNewStrm->Write( p, nn ) != nn )
                    break;
                n -= nn;
            }
            delete p;
            if( n )
            {
                pTmpStrm->Seek( nPos );
                pStgStrm->GetIo().SetError( pTmpStrm->GetError() );
                delete pNewStrm;
                return FALSE;
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
    return TRUE;
}

// Check if the given entry is contained in this entry

BOOL StgDirEntry::IsContained( StgDirEntry* pStg )
{
    if( aEntry.GetType() == STG_STORAGE )
    {
        StgIterator aIter( *this );
        StgDirEntry* p = aIter.First();
        while( p )
        {
            if( !p->aEntry.Compare( pStg->aEntry ) )
                return FALSE;
            if( p->aEntry.GetType() == STG_STORAGE )
                if( !p->IsContained( pStg ) )
                    return FALSE;
            p = aIter.Next();
        }
    }
    return TRUE;
}

// Invalidate all open entries by setting the RefCount to 0. If the bDel
// flag is set, also set the invalid flag to indicate deletion during the
// next dir stream flush.

void StgDirEntry::Invalidate( BOOL bDel )
{
//  nRefCnt = 0;
    if( bDel )
        bRemoved = bInvalid = TRUE;
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

void StgDirStrm::SetupEntry( INT32 n, StgDirEntry* pUpper )
{
    void* p = ( n == STG_FREE ) ? NULL : GetEntry( n );
    if( p )
    {
        BOOL bOk;
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

        INT32 nLeft = pCur->aEntry.GetLeaf( STG_LEFT );
        INT32 nRight = pCur->aEntry.GetLeaf( STG_RIGHT );
        // substorage?
        INT32 nLeaf = STG_FREE;
        if( pCur->aEntry.GetType() == STG_STORAGE
         || pCur->aEntry.GetType() == STG_ROOT )
            nLeaf = pCur->aEntry.GetLeaf( STG_CHILD );
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

BOOL StgDirStrm::SetSize( INT32 nBytes )
{
    // Always allocate full pages
    nBytes = ( ( nBytes + nPageSize - 1 ) / nPageSize ) * nPageSize;
    return StgStrm::SetSize( nBytes );
}

// Save the TOC stream into a new substream after saving all data streams

BOOL StgDirStrm::Store()
{
    if( !pRoot->IsDirty() )
        return TRUE;
    if( !pRoot->StoreStreams( rIo ) )
        return FALSE;
    // After writing all streams, the data FAT stream has changed,
    // so we have to commit the root again
    pRoot->Commit();
    // We want a completely new stream, so fake an empty stream
    INT32 nOldStart = nStart;       // save for later deletion
    INT32 nOldSize  = nSize;
    nStart = nPage = STG_EOF;
    nSize  = nPos = 0;
    nOffset = 0;
    // Delete all temporary entries
    pRoot->DelTemp( FALSE );
    // set the entry numbers
    INT32 n = 0;
    pRoot->Enum( n );
    if( !SetSize( n * STGENTRY_SIZE ) )
    {
        nStart = nOldStart; nSize = nOldSize;
        pRoot->RevertAll();
        return FALSE;
    }
    // set up the cache elements for the new stream
    if( !Copy( STG_FREE, nSize ) )
    {
        pRoot->RevertAll();
        return FALSE;
    }
    // Write the data to the new stream
    if( !pRoot->Store( *this ) )
    {
        pRoot->RevertAll();
        return FALSE;
    }
    // fill any remaining entries with empty data
    INT32 ne = nSize / STGENTRY_SIZE;
    StgEntry aEmpty;
    aEmpty.Init();
    while( n < ne )
    {
        void* p = GetEntry( n++, TRUE );
        if( !p )
        {
            pRoot->RevertAll();
            return FALSE;
        }
        aEmpty.Store( p );
    }
    // Now we can release the old stream
    pFat->FreePages( nOldStart, TRUE );
    rIo.aHdr.SetTOCStart( nStart );
    return TRUE;
}

// Get a dir entry.

void* StgDirStrm::GetEntry( INT32 n, BOOL bDirty )
{
    n *= STGENTRY_SIZE;
    if( n >= nSize )
        return NULL;
    return GetPtr( n, TRUE, bDirty );
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
        pRes->bTemp    = FALSE;
        pRes->bCreated =
        pRes->bDirty   = TRUE;
    }
    else
    {
        pRes = new StgDirEntry( aEntry );
        if( StgAvlNode::Insert( (StgAvlNode**) &rStg.pDown, pRes ) )
        {
            pRes->pUp    = &rStg;
            pRes->ppRoot = &pRoot;
            pRes->bCreated =
            pRes->bDirty = TRUE;
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

BOOL StgDirStrm::Rename( StgDirEntry& rStg, const String& rOld, const String& rNew )
{
    StgDirEntry* p = Find( rStg, rOld );
    if( p )
    {

        if( !StgAvlNode::Remove( (StgAvlNode**) &rStg.pDown, p, FALSE ) )
            return FALSE;
        p->aEntry.SetName( rNew );
        if( !StgAvlNode::Insert( (StgAvlNode**) &rStg.pDown, p ) )
            return FALSE;
        p->bRenamed = p->bDirty   = TRUE;
        return TRUE;
    }
    else
    {
        rIo.SetError( SVSTREAM_FILE_NOT_FOUND );
        return FALSE;
    }
}

// Move the given entry to a different storage.

BOOL StgDirStrm::Move( StgDirEntry& rStg1, StgDirEntry& rStg2, const String& rName )
{
    StgDirEntry* p = Find( rStg1, rName );
    if( p )
    {
        if( !StgAvlNode::Move
            ( (StgAvlNode**) &rStg1.pDown, (StgAvlNode**) &rStg2.pDown, p ) )
            return FALSE;
        p->bDirty = TRUE;
        return TRUE;
    }
    else
    {
        rIo.SetError( SVSTREAM_FILE_NOT_FOUND );
        return FALSE;
    }
}

