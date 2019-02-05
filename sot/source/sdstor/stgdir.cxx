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


#include <string.h>

#include <sot/stg.hxx>
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"

#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <memory>

//////////////////////////// class StgDirEntry

// This class holds the dir entry data and maintains dirty flags for both
// the entry and the data.

// Transacted mode for streams: On the first write, a temp stream pTmpStrm
// is created and operated on. A commit moves pTmpStrm to pCurStrm, which
// is used for subsequent reads. A new write creates a new copy of pTmpStrm
// based on pCurStrm. Reverting throws away pTmpStrm.
// Transacted mode for storages: A copy of the dir ents is kept in aSave.
// Committing means copying aEntry to aSave. Reverting means to copy aSave
// to aEntry, delete newly created entries and to reactivate removed entries.

// Problem of implementation: No hierarchical commits. Therefore only
// overall transaction-oriented or direct.

StgDirEntry::StgDirEntry( const void* pBuffer, sal_uInt32 nBufferLen, sal_uInt64 nUnderlyingStreamSize, bool * pbOk ) : StgAvlNode()
{
    *pbOk = m_aEntry.Load( pBuffer, nBufferLen, nUnderlyingStreamSize );

    InitMembers();
}

StgDirEntry::StgDirEntry( const StgEntry& r ) : StgAvlNode(), m_aEntry( r )
{
    InitMembers();
}

// Helper for all ctors

void StgDirEntry::InitMembers()
{
    m_aSave       = m_aEntry;
    m_pUp         =
    m_pDown       = nullptr;
    m_pStgStrm    = nullptr;
    m_pCurStrm    =
    m_pTmpStrm    = nullptr;
    m_nPos        =
    m_nEntry      =
    m_nRefCnt     = 0;
    m_nMode       = StreamMode::READ;
    m_bDirect     = true;
    m_bInvalid    =
    m_bRemoved    =
    m_bTemp       =
    m_bDirty      =
    m_bZombie     = false;
}

StgDirEntry::~StgDirEntry()
{
    Close();
    delete m_pCurStrm;
    delete m_pStgStrm;
    delete m_pDown;
}

// Comparison function

sal_Int32 StgDirEntry::Compare( const StgAvlNode* p ) const
{
    sal_Int32 nResult = -1;
    if ( p )
    {
        const StgDirEntry* pEntry = static_cast<const StgDirEntry*>(p);
        nResult = m_aEntry.Compare( pEntry->m_aEntry );
    }
    return nResult;
}

// Enumerate the entry numbers.
// n is incremented to show the total # of entries.
// These number are later used as page numbers when storing
// the TOC tree into the TOC stream. Remember that aSave is
// stored, not aEntry.

void StgDirEntry::Enum( sal_Int32& n )
{
    sal_Int32 nLeft = STG_FREE, nRight = STG_FREE, nDown = STG_FREE;
    m_nEntry = n++;
    if( m_pLeft )
    {
        static_cast<StgDirEntry*>(m_pLeft)->Enum( n );
        nLeft = static_cast<StgDirEntry*>(m_pLeft)->m_nEntry;
    }
    if( m_pRight )
    {
        static_cast<StgDirEntry*>(m_pRight)->Enum( n );
        nRight = static_cast<StgDirEntry*>(m_pRight)->m_nEntry;
    }
    if( m_pDown )
    {
        m_pDown->Enum( n ); nDown = m_pDown->m_nEntry;
    }
    m_aSave.SetLeaf( STG_LEFT, nLeft );
    m_aSave.SetLeaf( STG_RIGHT, nRight );
    m_aSave.SetLeaf( STG_CHILD, nDown );
}

// Delete all temporary entries before writing the TOC stream.
// Until now Deltemp is never called with bForce True

void StgDirEntry::DelTemp( bool bForce )
{
    if( m_pLeft )
        static_cast<StgDirEntry*>(m_pLeft)->DelTemp( false );
    if( m_pRight )
        static_cast<StgDirEntry*>(m_pRight)->DelTemp( false );
    if( m_pDown )
    {
        // If the storage is dead, of course all elements are dead, too
        if( m_bInvalid && m_aEntry.GetType() == STG_STORAGE )
            bForce = true;
        m_pDown->DelTemp( bForce );
    }
    if( !( bForce || m_bInvalid ) || ( m_aEntry.GetType() == STG_ROOT ) )
        return;

    Close();
    if( m_pUp )
    {
        // this deletes the element if refcnt == 0!
        bool bDel = m_nRefCnt == 0;
        StgAvlNode::Remove( reinterpret_cast<StgAvlNode**>(&m_pUp->m_pDown), this, bDel );
        if( !bDel )
        {
            m_pLeft = m_pRight = m_pDown = nullptr;
            m_bInvalid = m_bZombie = true;
        }
    }
}

// Save the tree into the given dir stream

bool StgDirEntry::Store( StgDirStrm& rStrm )
{
    void* pEntry = rStrm.GetEntry( m_nEntry, true );
    if( !pEntry )
        return false;
    // Do not store the current (maybe not committed) entry
    m_aSave.Store( pEntry );
    if( m_pLeft )
        if( !static_cast<StgDirEntry*>(m_pLeft)->Store( rStrm ) )
            return false;
    if( m_pRight )
        if( !static_cast<StgDirEntry*>(m_pRight)->Store( rStrm ) )
            return false;
    if( m_pDown && !m_pDown->Store( rStrm ) )
        return false;
    return true;
}

bool StgDirEntry::StoreStream( StgIo& rIo )
{
    if( m_aEntry.GetType() == STG_STREAM || m_aEntry.GetType() == STG_ROOT )
    {
        if( m_bInvalid )
        {
            // Delete the stream if needed
            if( !m_pStgStrm )
            {
                OpenStream( rIo );
                delete m_pStgStrm;
                m_pStgStrm = nullptr;
            }
            else
                m_pStgStrm->SetSize( 0 );
        }
        // or write the data stream
        else if( !Tmp2Strm() )
            return false;
    }
    return true;
}

// Save all dirty streams

bool StgDirEntry::StoreStreams( StgIo& rIo )
{
    if( !StoreStream( rIo ) )
        return false;
    if( m_pLeft )
        if( !static_cast<StgDirEntry*>(m_pLeft)->StoreStreams( rIo ) )
            return false;
    if( m_pRight )
        if( !static_cast<StgDirEntry*>(m_pRight)->StoreStreams( rIo ) )
            return false;
    if( m_pDown )
        if( !m_pDown->StoreStreams( rIo ) )
            return false;
    return true;
}

// Revert all directory entries after failure to write the TOC stream

void StgDirEntry::RevertAll()
{
    m_aEntry = m_aSave;
    if( m_pLeft )
        static_cast<StgDirEntry*>(m_pLeft)->RevertAll();
    if( m_pRight )
        static_cast<StgDirEntry*>(m_pRight)->RevertAll();
    if( m_pDown )
        m_pDown->RevertAll();
}

// Look if any element of the tree is dirty

bool StgDirEntry::IsDirty()
{
    if( m_bDirty || m_bInvalid )
        return true;
    if( m_pLeft && static_cast<StgDirEntry*>(m_pLeft)->IsDirty() )
        return true;
    if( m_pRight && static_cast<StgDirEntry*>(m_pRight)->IsDirty() )
        return true;
    if( m_pDown && m_pDown->IsDirty() )
        return true;
    return false;
}

// Set up a stream.

void StgDirEntry::OpenStream( StgIo& rIo )
{
    sal_Int32 nThreshold = static_cast<sal_uInt16>(rIo.m_aHdr.GetThreshold());
    delete m_pStgStrm;
    if( m_aEntry.GetSize() < nThreshold )
        m_pStgStrm = new StgSmallStrm( rIo, *this );
    else
        m_pStgStrm = new StgDataStrm( rIo, *this );
    if( m_bInvalid && m_aEntry.GetSize() )
    {
        // This entry has invalid data, so delete that data
        SetSize( 0 );
//      bRemoved = bInvalid = false;
    }
    m_nPos = 0;
}

// Close the open stream without committing. If the entry is marked as
// temporary, delete it.
// Do not delete pCurStrm here!
// (TLX:??? At least pStgStrm must be deleted.)

void StgDirEntry::Close()
{
    delete m_pTmpStrm;
    m_pTmpStrm = nullptr;
//  nRefCnt  = 0;
    m_bInvalid = m_bTemp;
}

// Get the current stream size

sal_Int32 StgDirEntry::GetSize() const
{
    sal_Int32 n;
    if( m_pTmpStrm )
        n = m_pTmpStrm->GetSize();
    else if( m_pCurStrm )
        n = m_pCurStrm->GetSize();
    else n = m_aEntry.GetSize();
    return n;
}

// Set the stream size. This means also creating a temp stream.

bool StgDirEntry::SetSize( sal_Int32 nNewSize )
{
    if (
         !( m_nMode & StreamMode::WRITE ) ||
         (!m_bDirect && !m_pTmpStrm && !Strm2Tmp())
       )
    {
        return false;
    }

    if( nNewSize < m_nPos )
        m_nPos = nNewSize;
    if( m_pTmpStrm )
    {
        m_pTmpStrm->SetSize( nNewSize );
        m_pStgStrm->GetIo().SetError( m_pTmpStrm->GetError() );
        return m_pTmpStrm->GetError() == ERRCODE_NONE;
    }
    else
    {
        OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
        if ( !m_pStgStrm )
            return false;

        bool bRes = false;
        StgIo& rIo = m_pStgStrm->GetIo();
        sal_Int32 nThreshold = rIo.m_aHdr.GetThreshold();
        // ensure the correct storage stream!
        StgStrm* pOld = nullptr;
        sal_uInt16 nOldSize = 0;
        if( nNewSize >= nThreshold && m_pStgStrm->IsSmallStrm() )
        {
            pOld = m_pStgStrm;
            nOldSize = static_cast<sal_uInt16>(pOld->GetSize());
            m_pStgStrm = new StgDataStrm( rIo, STG_EOF, 0 );
        }
        else if( nNewSize < nThreshold && !m_pStgStrm->IsSmallStrm() )
        {
            pOld = m_pStgStrm;
            nOldSize = static_cast<sal_uInt16>(nNewSize);
            m_pStgStrm = new StgSmallStrm( rIo, STG_EOF );
        }
        // now set the new size
        if( m_pStgStrm->SetSize( nNewSize ) )
        {
            // did we create a new stream?
            if( pOld )
            {
                // if so, we probably need to copy the old data
                if( nOldSize )
                {
                    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ nOldSize ]);
                    pOld->Pos2Page( 0 );
                    m_pStgStrm->Pos2Page( 0 );
                    if( pOld->Read( pBuf.get(), nOldSize )
                        && m_pStgStrm->Write( pBuf.get(), nOldSize ) )
                        bRes = true;
                }
                else
                    bRes = true;
                if( bRes )
                {
                    pOld->SetSize( 0 );
                    delete pOld;
                    m_pStgStrm->Pos2Page( m_nPos );
                    m_pStgStrm->SetEntry( *this );
                }
                else
                {
                    m_pStgStrm->SetSize( 0 );
                    delete m_pStgStrm;
                    m_pStgStrm = pOld;
                }
            }
            else
            {
                m_pStgStrm->Pos2Page( m_nPos );
                bRes = true;
            }
        }
        return bRes;
    }
}

// Seek. On negative values, seek to EOF.

sal_Int32 StgDirEntry::Seek( sal_Int32 nNew )
{
    if( m_pTmpStrm )
    {
        if( nNew < 0 )
            nNew = m_pTmpStrm->GetSize();
        nNew = m_pTmpStrm->Seek( nNew );
    }
    else if( m_pCurStrm )
    {
        if( nNew < 0 )
            nNew = m_pCurStrm->GetSize();
        nNew = m_pCurStrm->Seek( nNew );
    }
    else
    {
        OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
        if ( !m_pStgStrm )
            return m_nPos;

        sal_Int32 nSize = m_aEntry.GetSize();

        if( nNew < 0 )
            nNew = nSize;

        // try to enlarge, readonly streams do not allow this
        if( nNew > nSize )
        {
            if ( !( m_nMode & StreamMode::WRITE ) || !SetSize( nNew ) )
            {
                return m_nPos;
            }
            else
                return Seek( nNew );
        }
        m_pStgStrm->Pos2Page( nNew );
        nNew = m_pStgStrm->GetPos();
    }

    return m_nPos = nNew;
}

// Read

sal_Int32 StgDirEntry::Read( void* p, sal_Int32 nLen )
{
    if( nLen <= 0 )
        return 0;
    if( m_pTmpStrm )
        nLen = m_pTmpStrm->ReadBytes( p, nLen );
    else if( m_pCurStrm )
        nLen = m_pCurStrm->ReadBytes( p, nLen );
    else
    {
        OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
        if ( !m_pStgStrm )
            return 0;

        nLen = m_pStgStrm->Read( p, nLen );
    }

    m_nPos += nLen;
    return nLen;
}

// Write

sal_Int32 StgDirEntry::Write( const void* p, sal_Int32 nLen )
{
    if( nLen <= 0 || !( m_nMode & StreamMode::WRITE ) )
        return 0;

    // Was this stream committed internally and reopened in direct mode?
    if( m_bDirect && ( m_pCurStrm || m_pTmpStrm ) && !Tmp2Strm() )
        return 0;
    // Is this stream opened in transacted mode? Do we have to make a copy?
    if( !m_bDirect && !m_pTmpStrm && !Strm2Tmp() )
        return 0;

    OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
    if ( !m_pStgStrm )
        return 0;

    if( m_pTmpStrm )
    {
        nLen = m_pTmpStrm->WriteBytes( p, nLen );
        m_pStgStrm->GetIo().SetError( m_pTmpStrm->GetError() );
    }
    else
    {
        sal_Int32 nNew = m_nPos + nLen;
        if( nNew > m_pStgStrm->GetSize() )
        {
            if( !SetSize( nNew ) )
                return 0L;
            m_pStgStrm->Pos2Page( m_nPos );
        }
        nLen = m_pStgStrm->Write( p, nLen );
    }
    m_nPos += nLen;
    return nLen;
}

void StgDirEntry::Copy( BaseStorageStream& rDest )
{
    sal_Int32 n = GetSize();
    if( !(rDest.SetSize( n ) && n) )
        return;

    sal_uLong Pos = rDest.Tell();
    sal_uInt8 aTempBytes[ 4096 ];
    void* p = static_cast<void*>( aTempBytes );
    Seek( 0 );
    rDest.Seek( 0 );
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

// Commit this entry

bool StgDirEntry::Commit()
{
    // OSL_ENSURE( nMode & StreamMode::WRITE, "Trying to commit readonly stream!" );

    m_aSave = m_aEntry;
    bool bRes = true;
    if( m_aEntry.GetType() == STG_STREAM )
    {
        if( m_pTmpStrm )
        {
            delete m_pCurStrm;
            m_pCurStrm = m_pTmpStrm;
            m_pTmpStrm = nullptr;
        }
        if( m_bRemoved )
            // Delete the stream if needed
            if( m_pStgStrm )
                m_pStgStrm->SetSize( 0 );
    }
    else if( m_aEntry.GetType() == STG_STORAGE && m_bDirect && bRes )
    {
        StgIterator aIter( *this );
        for( StgDirEntry* p = aIter.First(); p && bRes; p = aIter.Next() )
            bRes = p->Commit();
    }
    return bRes;
}

// Copy the stg stream to the temp stream

bool StgDirEntry::Strm2Tmp()
{
    if( !m_pTmpStrm )
    {
        sal_uLong n = 0;
        if( m_pCurStrm )
        {
            // It was already committed once
            m_pTmpStrm = new StgTmpStrm;
            if( m_pTmpStrm->GetError() == ERRCODE_NONE && m_pTmpStrm->Copy( *m_pCurStrm ) )
                return true;
            n = 1;  // indicates error
        }
        else
        {
            n = m_aEntry.GetSize();
            m_pTmpStrm = new StgTmpStrm( n );
            if( m_pTmpStrm->GetError() == ERRCODE_NONE )
            {
                if( n )
                {
                    OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
                    if ( !m_pStgStrm )
                        return false;

                    sal_uInt8 aTempBytes[ 4096 ];
                    void* p = static_cast<void*>( aTempBytes );
                    m_pStgStrm->Pos2Page( 0 );
                    while( n )
                    {
                        sal_uLong nn = n;
                        if( nn > 4096 )
                            nn = 4096;
                        if( static_cast<sal_uLong>(m_pStgStrm->Read( p, nn )) != nn )
                            break;
                        if (m_pTmpStrm->WriteBytes( p, nn ) != nn)
                            break;
                        n -= nn;
                    }
                    m_pStgStrm->Pos2Page( m_nPos );
                    m_pTmpStrm->Seek( m_nPos );
                }
            }
            else
                n = 1;
        }

        if( n )
        {
            OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
            if ( m_pStgStrm )
                m_pStgStrm->GetIo().SetError( m_pTmpStrm->GetError() );

            delete m_pTmpStrm;
            m_pTmpStrm = nullptr;
            return false;
        }
    }
    return true;
}

// Copy the temp stream to the stg stream during the final commit

bool StgDirEntry::Tmp2Strm()
{
    // We did commit once, but have not written since then
    if( !m_pTmpStrm )
    {
        m_pTmpStrm = m_pCurStrm;
        m_pCurStrm = nullptr;
    }
    if( m_pTmpStrm )
    {
        OSL_ENSURE( m_pStgStrm, "The pointer may not be NULL!" );
        if ( !m_pStgStrm )
            return false;
        sal_uInt64 n = m_pTmpStrm->GetSize();
        std::unique_ptr<StgStrm> pNewStrm;
        StgIo& rIo = m_pStgStrm->GetIo();
        sal_uLong nThreshold = static_cast<sal_uLong>(rIo.m_aHdr.GetThreshold());
        if( n < nThreshold )
            pNewStrm.reset(new StgSmallStrm( rIo, STG_EOF ));
        else
            pNewStrm.reset(new StgDataStrm( rIo, STG_EOF ));
        if( pNewStrm->SetSize( n ) )
        {
            sal_uInt8 p[ 4096 ];
            m_pTmpStrm->Seek( 0 );
            while( n )
            {
                sal_uInt64 nn = n;
                if( nn > 4096 )
                    nn = 4096;
                if (m_pTmpStrm->ReadBytes( p, nn ) != nn)
                    break;
                if( static_cast<sal_uLong>(pNewStrm->Write( p, nn )) != nn )
                    break;
                n -= nn;
            }
            if( n )
            {
                m_pTmpStrm->Seek( m_nPos );
                m_pStgStrm->GetIo().SetError( m_pTmpStrm->GetError() );
                return false;
            }
            else
            {
                m_pStgStrm->SetSize( 0 );
                delete m_pStgStrm;
                m_pStgStrm = pNewStrm.release();
                m_pStgStrm->SetEntry(*this);
                m_pStgStrm->Pos2Page(m_nPos);
                delete m_pTmpStrm;
                delete m_pCurStrm;
                m_pTmpStrm = m_pCurStrm = nullptr;
                m_aSave = m_aEntry;
            }
        }
    }
    return true;
}

// Invalidate all open entries by setting the RefCount to 0. If the bDel
// flag is set, also set the invalid flag to indicate deletion during the
// next dir stream flush.

void StgDirEntry::Invalidate( bool bDel )
{
//  nRefCnt = 0;
    if( bDel )
        m_bRemoved = m_bInvalid = true;
    switch( m_aEntry.GetType() )
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

///////////////////////////// class StgDirStrm

// This specialized stream is the maintenance stream for the directory tree.

StgDirStrm::StgDirStrm( StgIo& r )
          : StgDataStrm( r, r.m_aHdr.GetTOCStart(), -1 )
          , m_pRoot( nullptr )
{
    if( r.GetError() )
        return;
    if( m_nStart == STG_EOF )
    {
        StgEntry aRoot;
        aRoot.Init();
        aRoot.SetName( "Root Entry" );
        aRoot.SetType( STG_ROOT );
        m_pRoot = new StgDirEntry( aRoot );
        m_pRoot->SetDirty();
    }
    else
    {
        // temporarily use this instance as owner, so
        // the TOC pages can be removed.
        m_pEntry = reinterpret_cast<StgDirEntry*>(this); // just for a bit pattern
        SetupEntry( 0, m_pRoot );
        m_pEntry = nullptr;
    }
}

StgDirStrm::~StgDirStrm()
{
    delete m_pRoot;
}

// Recursively parse the directory tree during reading the TOC stream

void StgDirStrm::SetupEntry( sal_Int32 n, StgDirEntry* pUpper )
{
    void* p = ( n == STG_FREE ) ? nullptr : GetEntry( n, false );
    if( !p )
        return;

    SvStream *pUnderlyingStream = m_rIo.GetStrm();
    sal_uInt64 nUnderlyingStreamSize = pUnderlyingStream->TellEnd();

    bool bOk(false);
    std::unique_ptr<StgDirEntry> pCur(new StgDirEntry( p, STGENTRY_SIZE, nUnderlyingStreamSize, &bOk ));

    if( !bOk )
    {
        m_rIo.SetError( SVSTREAM_GENERALERROR );
        // an error occurred
        return;
    }

    // better it is
    if( !pUpper )
        pCur->m_aEntry.SetType( STG_ROOT );

    sal_Int32 nLeft = pCur->m_aEntry.GetLeaf( STG_LEFT );
    sal_Int32 nRight = pCur->m_aEntry.GetLeaf( STG_RIGHT );
    // substorage?
    sal_Int32 nLeaf = STG_FREE;
    if( pCur->m_aEntry.GetType() == STG_STORAGE || pCur->m_aEntry.GetType() == STG_ROOT )
    {
        nLeaf = pCur->m_aEntry.GetLeaf( STG_CHILD );
        if (nLeaf != STG_FREE && nLeaf == n)
        {
            m_rIo.SetError( SVSTREAM_GENERALERROR );
            return;
        }
    }

    if( !(nLeaf != 0 && nLeft != 0 && nRight != 0) )
        return;

    //fdo#41642
    StgDirEntry *pUp = pUpper;
    while (pUp)
    {
        if (pUp->m_aEntry.GetLeaf(STG_CHILD) == nLeaf)
        {
            SAL_WARN("sot", "Leaf node of upper StgDirEntry is same as current StgDirEntry's leaf node. Circular entry chain, discarding link");
            return;
        }
        pUp = pUp->m_pUp;
    }

    if( StgAvlNode::Insert
        ( reinterpret_cast<StgAvlNode**>( pUpper ? &pUpper->m_pDown : &m_pRoot ), pCur.get() ) )
    {
        pCur->m_pUp    = pUpper;
    }
    else
    {
        // bnc#682484: There are some really broken docs out there
        // that contain duplicate entries in 'Directory' section
        // so don't set the error flag here and just skip those
        // (was: rIo.SetError( SVSTREAM_CANNOT_MAKE );)
        return;
    }
    SetupEntry( nLeft, pUpper );
    SetupEntry( nRight, pUpper );
    SetupEntry( nLeaf, pCur.release() );
}

// Extend or shrink the directory stream.

bool StgDirStrm::SetSize( sal_Int32 nBytes )
{
    // Always allocate full pages
    if ( nBytes < 0 )
        nBytes = 0;

    nBytes = ( ( nBytes + m_nPageSize - 1 ) / m_nPageSize ) * m_nPageSize;
    return StgStrm::SetSize( nBytes );
}

// Save the TOC stream into a new substream after saving all data streams

bool StgDirStrm::Store()
{
    if( !m_pRoot || !m_pRoot->IsDirty() )
        return true;
    if( !m_pRoot->StoreStreams( m_rIo ) )
        return false;
    // After writing all streams, the data FAT stream has changed,
    // so we have to commit the root again
    m_pRoot->Commit();
    // We want a completely new stream, so fake an empty stream
    sal_Int32 nOldStart = m_nStart;       // save for later deletion
    sal_Int32 nOldSize  = m_nSize;
    m_nStart = m_nPage = STG_EOF;
    m_nSize = 0;
    SetPos(0, true);
    m_nOffset = 0;
    // Delete all temporary entries
    m_pRoot->DelTemp( false );
    // set the entry numbers
    sal_Int32 n = 0;
    m_pRoot->Enum( n );
    if( !SetSize( n * STGENTRY_SIZE ) )
    {
        m_nStart = nOldStart; m_nSize = nOldSize;
        m_pRoot->RevertAll();
        return false;
    }
    // set up the cache elements for the new stream
    if( !Copy( STG_FREE, m_nSize ) )
    {
        m_pRoot->RevertAll();
        return false;
    }
    // Write the data to the new stream
    if( !m_pRoot->Store( *this ) )
    {
        m_pRoot->RevertAll();
        return false;
    }
    // fill any remaining entries with empty data
    sal_Int32 ne = m_nSize / STGENTRY_SIZE;
    StgEntry aEmpty;
    aEmpty.Init();
    while( n < ne )
    {
        void* p = GetEntry( n++, true );
        if( !p )
        {
            m_pRoot->RevertAll();
            return false;
        }
        aEmpty.Store( p );
    }
    // Now we can release the old stream
    m_pFat->FreePages( nOldStart, true );
    m_rIo.m_aHdr.SetTOCStart( m_nStart );
    return true;
}

// Get a dir entry.

void* StgDirStrm::GetEntry( sal_Int32 n, bool bDirty )
{
    return n < 0 || n >= m_nSize / STGENTRY_SIZE
        ? nullptr : GetPtr( n * STGENTRY_SIZE, bDirty );
}

// Find a dir entry.

StgDirEntry* StgDirStrm::Find( StgDirEntry& rStg, const OUString& rName )
{
    if( rStg.m_pDown )
    {
        StgEntry aEntry;
        aEntry.Init();
        aEntry.SetName( rName );
        // Look in the directory attached to the entry
        StgDirEntry aTest( aEntry );
        return static_cast<StgDirEntry*>( rStg.m_pDown->Find( &aTest ) );
    }
    else
        return nullptr;
}

// Create a new entry.

StgDirEntry* StgDirStrm::Create( StgDirEntry& rStg, const OUString& rName, StgEntryType eType )
{
    StgEntry aEntry;
    aEntry.Init();
    aEntry.SetType( eType );
    aEntry.SetName( rName );
    StgDirEntry* pRes = Find( rStg, rName );
    if( pRes )
    {
        if( !pRes->m_bInvalid )
        {
            m_rIo.SetError( SVSTREAM_CANNOT_MAKE );
            return nullptr;
        }
        pRes->m_bInvalid =
        pRes->m_bRemoved =
        pRes->m_bTemp    = false;
        pRes->m_bDirty   = true;
        return pRes;
    }
    else
    {
        std::unique_ptr<StgDirEntry> pNewRes(new StgDirEntry( aEntry ));
        if( StgAvlNode::Insert( reinterpret_cast<StgAvlNode**>(&rStg.m_pDown), pNewRes.get() ) )
        {
            pNewRes->m_pUp    = &rStg;
            pNewRes->m_bDirty = true;
        }
        else
        {
            m_rIo.SetError( SVSTREAM_CANNOT_MAKE );
            pNewRes.reset();
        }
        return pNewRes.release();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
