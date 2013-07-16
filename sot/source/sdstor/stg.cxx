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


#include <sot/storinfo.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#include "stgole.hxx"

static long nTmpCount = 0;

// The internal open mode is STREAM_READ | STREAM_TRUNC, which is silly
// by itself. It inhibits the checking of sharing modes and is used
// during CopyTo() and MoveTo() for opening a stream in read mode
// although it may be open in DENYALL mode

#define INTERNAL_MODE ( STREAM_READ | STREAM_TRUNC )

///////////////////////// class StorageBase //////////////////////////////

TYPEINIT0( StorageBase );
TYPEINIT1( BaseStorageStream, StorageBase );
TYPEINIT1( BaseStorage, StorageBase );

StorageBase::StorageBase()
    : m_bAutoCommit( false )
{
    m_nMode  = STREAM_READ;
    m_nError = SVSTREAM_OK;
}

StorageBase::~StorageBase()
{
}

// The following three methods are declared as const, since they
// may be called from within a const method.

sal_uLong StorageBase::GetError() const
{
    sal_uLong n = m_nError;
    ((StorageBase*) this)->m_nError = SVSTREAM_OK;
    return n;
}

void StorageBase::SetError( sal_uLong n ) const
{
    if( !m_nError )
        ((StorageBase*) this)->m_nError = n;
}

void StorageBase::ResetError() const
{
    ((StorageBase*) this)->m_nError = SVSTREAM_OK;
}

// Retrieve the underlying SvStream for info purposes

const SvStream* OLEStorageBase::GetSvStream_Impl() const
{
    return pIo ? pIo->GetStrm() : NULL;
}

OLEStorageBase::OLEStorageBase( StgIo* p, StgDirEntry* pe, StreamMode& nMode )
    : nStreamMode( nMode ), pIo( p ), pEntry( pe )
{
    if ( p )
        p->IncRef();
    if( pe )
        pe->nRefCnt++;
}

OLEStorageBase::~OLEStorageBase()
{
    if( pEntry )
    {
        DBG_ASSERT( pEntry->nRefCnt, "RefCount unter 0" );
        if( !--pEntry->nRefCnt )
        {
            if( pEntry->bZombie )
                delete pEntry;
            else
                pEntry->Close();
        }

        pEntry = NULL;
    }


    if( pIo && !pIo->DecRef() )
    {
        delete pIo;
        pIo = NULL;
    }
}

// Validate the instance for I/O

bool OLEStorageBase::Validate_Impl( bool bWrite ) const
{
    if( pIo
        && pIo->pTOC
        && pEntry
        && !pEntry->bInvalid
        &&  ( !bWrite || !pEntry->bDirect || ( nStreamMode & STREAM_WRITE ) ) )
            return true;
    return false;
}

bool OLEStorageBase::ValidateMode_Impl( StreamMode m, StgDirEntry* p ) const
{
    if( m == INTERNAL_MODE )
        return true;
    sal_uInt16 nCurMode = ( p && p->nRefCnt ) ? p->nMode : 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return true;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return true;
    }
    return false;
}


//////////////////////// class StorageStream /////////////////////////////

TYPEINIT1( StorageStream, BaseStorageStream );

StorageStream::StorageStream( StgIo* p, StgDirEntry* q, StreamMode m )
             : OLEStorageBase( p, q, m_nMode ), nPos( 0L )
{
    // The dir entry may be 0; this means that the stream is invalid.
    if( q && p )
    {
        if( q->nRefCnt == 1 )
        {
            q->nMode = m;
            q->OpenStream( *p );
        }
    }
    else
        m &= ~STREAM_READWRITE;
    m_nMode = m;
}

StorageStream::~StorageStream()
{
    // Do an auto-commit if the entry is open in direct mode
    if( m_bAutoCommit )
        Commit();
    if( pEntry && pEntry->nRefCnt && pEntry->bDirect && (m_nMode & STREAM_WRITE) )
        pEntry->Commit();
}

bool StorageStream::Equals( const BaseStorageStream& rStream ) const
{
    const StorageStream* pOther = PTR_CAST( StorageStream, &rStream );
    return pOther && ( pOther->pEntry == pEntry );
}

sal_uLong StorageStream::Read( void* pData, sal_uLong nSize )
{
    if( Validate() )
    {
        pEntry->Seek( nPos );
        nSize = pEntry->Read( pData, (sal_Int32) nSize );
        pIo->MoveError( *this );
        nPos += nSize;
    }
    else
        nSize = 0L;
    return nSize;
}

sal_uLong StorageStream::Write( const void* pData, sal_uLong nSize )
{
    if( Validate( true ) )
    {
        pEntry->Seek( nPos );
        nSize = pEntry->Write( pData, (sal_Int32) nSize );
        pIo->MoveError( *this );
        nPos += nSize;
    }
    else
        nSize = 0L;
    return nSize;
}

sal_uLong StorageStream::Seek( sal_uLong n )
{
    if( Validate() )
        return nPos = pEntry->Seek( n );
    else
        return n;
}

void StorageStream::Flush()
{
    // Flushing means committing, since streams are never transacted
    Commit();
}

bool StorageStream::SetSize( sal_uLong nNewSize )
{
    if( Validate( true ) )
    {
        bool b = pEntry->SetSize( (sal_Int32) nNewSize );
        pIo->MoveError( *this );
        return b;
    }
    else
        return false;
}

sal_uLong StorageStream::GetSize() const
{
    if( Validate() )
        return pEntry->GetSize();
    return 0;
}

bool StorageStream::Commit()
{
    if( !Validate() )
        return false;
    if( !( m_nMode & STREAM_WRITE ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return false;
    }
    else
    {
        pEntry->Commit();
        pIo->MoveError( *this );
        return Good();
    }
}

bool StorageStream::Revert()
{
    bool bResult = false;

    if ( Validate() )
    {
        pEntry->Revert();
        pIo->MoveError( *this );
        bResult = Good();
    }

    return bResult;
}

bool StorageStream::CopyTo( BaseStorageStream* pDest )
{
    if( !Validate() || !pDest || !pDest->Validate( true ) || Equals( *pDest ) )
        return false;
    pEntry->Copy( *pDest );
    pDest->Commit();
    pIo->MoveError( *this );
    SetError( pDest->GetError() );
    return Good() && pDest->Good();
}

const SvStream* StorageStream::GetSvStream() const
{
    return GetSvStream_Impl();
}

bool StorageStream::Validate( bool bValidate ) const
{
    bool bRet = Validate_Impl( bValidate );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

bool StorageStream::ValidateMode( StreamMode nMode ) const
{
    bool bRet = ValidateMode_Impl( nMode, NULL );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

///////////////////////// class SvStorageInfo //////////////////////////////

SvStorageInfo::SvStorageInfo( const StgDirEntry& rE )
{
    rE.aEntry.GetName( aName );
    bStorage = rE.aEntry.GetType() == STG_STORAGE;
    bStream  = rE.aEntry.GetType() == STG_STREAM;
    nSize    = bStorage ? 0 : rE.aEntry.GetSize();
}

/////////////////////////// class Storage ////////////////////////////////

bool Storage::IsStorageFile( const OUString & rFileName )
{
    StgIo aIo;
    if( aIo.Open( rFileName, STREAM_STD_READ ) )
        return aIo.Load();
    return false;
}

bool Storage::IsStorageFile( SvStream* pStream )
{
    bool bRet = false;

    if ( pStream )
    {
        StgHeader aHdr;
        sal_uLong nPos = pStream->Tell();
        bRet = ( aHdr.Load( *pStream ) && aHdr.Check() );

        // It's not a stream error if it is too small for a OLE storage header
        if ( pStream->GetErrorCode() == ERRCODE_IO_CANTSEEK )
            pStream->ResetError();
        pStream->Seek( nPos );
    }

    return bRet;
}

// Open the storage file. If writing is permitted and the file is not
// a storage file, initialize it.

TYPEINIT1( Storage, BaseStorage );

Storage::Storage( const OUString& rFile, StreamMode m, bool bDirect )
    : OLEStorageBase( new StgIo, NULL, m_nMode )
    , aName( rFile ), bIsRoot( false )
{
    bool bTemp = false;
    if( aName.isEmpty() )
    {
        // no name = temporary name!
        aName = utl::TempFile::CreateTempName();
        bTemp = true;
    }
    // the root storage creates the I/O system
    m_nMode = m;
    if( pIo->Open( aName, m ) )
    {
        Init( ( m & ( STREAM_TRUNC | STREAM_NOCREATE ) ) == STREAM_TRUNC );
        if( pEntry )
        {
            pEntry->bDirect = bDirect;
            pEntry->nMode = m;
            pEntry->bTemp = bTemp;
        }
    }
    else
    {
        pIo->MoveError( *this );
        pEntry = NULL;
    }
}

// Create a storage on a given stream.

Storage::Storage( SvStream& r, bool bDirect )
    : OLEStorageBase( new StgIo, NULL, m_nMode )
    , bIsRoot( false )
{
    m_nMode = STREAM_READ;
    if( r.IsWritable() )
        m_nMode = STREAM_READ | STREAM_WRITE;
    if( r.GetError() == SVSTREAM_OK )
    {
        pIo->SetStrm( &r, false );
        sal_uLong nSize = r.Seek( STREAM_SEEK_TO_END );
        r.Seek( 0L );
        // Initializing is OK if the stream is empty
        Init( nSize == 0 );
        if( pEntry )
        {
            pEntry->bDirect = bDirect;
            pEntry->nMode = m_nMode;
        }
        pIo->MoveError( *this );
    }
    else
    {
        SetError( r.GetError() );
        pEntry = NULL;
    }
}


Storage::Storage( UCBStorageStream& rStrm, bool bDirect )
       : OLEStorageBase( new StgIo, NULL, m_nMode ), bIsRoot( false )
{
    m_nMode = STREAM_READ;

    if ( rStrm.GetError() != SVSTREAM_OK )
    {
        SetError( rStrm.GetError() );
        pEntry = NULL;
        return;
    }

    SvStream* pStream = rStrm.GetModifySvStream();
    if ( !pStream )
    {
        OSL_FAIL( "UCBStorageStream can not provide SvStream implementation!\n" );
        SetError( SVSTREAM_GENERALERROR );
        pEntry = NULL;
        return;
    }

    if( pStream->IsWritable() )
        m_nMode = STREAM_READ | STREAM_WRITE;

    pIo->SetStrm( &rStrm );

    sal_uLong nSize = pStream->Seek( STREAM_SEEK_TO_END );
    pStream->Seek( 0L );
    // Initializing is OK if the stream is empty
    Init( nSize == 0 );
    if( pEntry )
    {
        pEntry->bDirect = bDirect;
        pEntry->nMode = m_nMode;
    }

    pIo->MoveError( *this );
}


// Perform common code for both ctors above.

void Storage::Init( bool bCreate )
{
    pEntry = NULL;
    bool bHdrLoaded = false;
    bIsRoot = true;

    OSL_ENSURE( pIo, "The pointer may not be empty at this point!" );
    if( pIo->Good() && pIo->GetStrm() )
    {
        sal_uLong nSize = pIo->GetStrm()->Seek( STREAM_SEEK_TO_END );
        pIo->GetStrm()->Seek( 0L );
        if( nSize )
        {
            bHdrLoaded = pIo->Load();
            if( !bHdrLoaded && !bCreate  )
            {
                // File is not a storage and not empty; do not destroy!
                SetError( SVSTREAM_FILEFORMAT_ERROR );
                return;
            }
        }
    }
    // file is a storage, empty or should be overwritten
    pIo->ResetError();
    // we have to set up the data structures, since
    // the file is empty
    if( !bHdrLoaded )
        pIo->Init();
    if( pIo->Good() && pIo->pTOC )
    {
        pEntry = pIo->pTOC->GetRoot();
        pEntry->nRefCnt++;
    }
}

// Internal ctor

Storage::Storage( StgIo* p, StgDirEntry* q, StreamMode m )
       : OLEStorageBase( p, q, m_nMode ), bIsRoot( false )
{
    if( q )
        q->aEntry.GetName( aName );
    else
        m &= ~STREAM_READWRITE;
    m_nMode   = m;
    if( q && q->nRefCnt == 1 )
        q->nMode = m;
}

Storage::~Storage()
{
    // Invalidate all open substorages
    if( m_bAutoCommit )
        Commit();
    if( pEntry )
    {
        // Do an auto-commit if the entry is open in direct mode
        if( pEntry->nRefCnt && pEntry->bDirect && (m_nMode & STREAM_WRITE) )
            Commit();
        if( pEntry->nRefCnt == 1 )
            pEntry->Invalidate();
    }
    // close the stream is root storage
    if( bIsRoot )
        pIo->Close();
    // remove the file if temporary root storage
    if( bIsRoot && pEntry && pEntry->bTemp )
    {
        osl::File::remove( GetName() );
    }
}

const OUString& Storage::GetName() const
{
    if( !bIsRoot && Validate() )
        pEntry->aEntry.GetName( ((Storage*) this)->aName );
    return aName;
}

// Fill in the info list for this storage

void Storage::FillInfoList( SvStorageInfoList* pList ) const
{
    if( Validate() && pList )
    {
        StgIterator aIter( *pEntry );
        StgDirEntry* p = aIter.First();
        while( p )
        {
            if( !p->bInvalid )
            {
                SvStorageInfo aInfo( *p );
                pList->push_back( aInfo );
            }
            p = aIter.Next();
        }
    }
}

// Open or create a substorage

BaseStorage* Storage::OpenUCBStorage( const OUString& rName, StreamMode m, bool bDirect )
{
    OSL_FAIL("Not supported!");
    return OpenStorage( rName, m, bDirect );
}

BaseStorage* Storage::OpenOLEStorage( const OUString& rName, StreamMode m, bool bDirect )
{
    return OpenStorage( rName, m, bDirect );
}

BaseStorage* Storage::OpenStorage( const OUString& rName, StreamMode m, bool bDirect )
{
    if( !Validate() || !ValidateMode( m ) )
        return new Storage( pIo, NULL, m );
    if( bDirect && !pEntry->bDirect )
        bDirect = false;

    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    if( !p )
    {
        if( !( m & STREAM_NOCREATE ) )
        {
            bool bTemp = false;
            // create a new storage
            OUString aNewName = rName;
            if( aNewName.isEmpty() )
            {
                aNewName = OUString( "Temp Stg " ) + OUString::number( ++nTmpCount );
                bTemp = true;
            }
            p = pIo->pTOC->Create( *pEntry, aNewName, STG_STORAGE );
            if( p )
                p->bTemp = bTemp;
        }
        if( !p )
            pIo->SetError( ( m & STREAM_WRITE )
                             ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
    }
    else if( !ValidateMode( m, p ) )
        p = NULL;
    if( p && p->aEntry.GetType() != STG_STORAGE )
    {
        pIo->SetError( SVSTREAM_FILE_NOT_FOUND );
        p = NULL;
    }

    // Either direct or transacted mode is supported
    if( p && pEntry->nRefCnt == 1 )
        p->bDirect = bDirect;

    // Dont check direct conflict if opening readonly
    if( p && (m & STREAM_WRITE ))
    {
        if( p->bDirect != bDirect )
            SetError( SVSTREAM_ACCESS_DENIED );
    }
    Storage* pStg = new Storage( pIo, p, m );
    pIo->MoveError( *pStg );
    if( m & STREAM_WRITE ) pStg->m_bAutoCommit = true;
    return pStg;
}

// Open a stream

BaseStorageStream* Storage::OpenStream( const OUString& rName, StreamMode m, bool,
                                        const OString*
#ifdef DBG_UTIL
                                        pB
#endif
)
{
    DBG_ASSERT(!pB, "Encryption not supported");

    if( !Validate() || !ValidateMode( m ) )
        return new StorageStream( pIo, NULL, m );
    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    bool bTemp = false;
    if( !p )
    {
        if( !( m & STREAM_NOCREATE ) )
        {
            // create a new stream
            // make a name if the stream is temporary (has no name)
            OUString aNewName( rName );
            if( aNewName.isEmpty() )
            {
                aNewName = OUString( "Temp Strm " ) + OUString::number( ++nTmpCount );
                bTemp = true;
            }
            p = pIo->pTOC->Create( *pEntry, aNewName, STG_STREAM );
        }
        if( !p )
            pIo->SetError( ( m & STREAM_WRITE )
                           ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
    }
    else if( !ValidateMode( m, p ) )
        p = NULL;
    if( p && p->aEntry.GetType() != STG_STREAM )
    {
        pIo->SetError( SVSTREAM_FILE_NOT_FOUND );
        p = NULL;
    }
    if( p )
    {
        p->bTemp = bTemp;
        p->bDirect = pEntry->bDirect;
    }
    StorageStream* pStm = new StorageStream( pIo, p, m );
    if( p && !p->bDirect )
        pStm->SetAutoCommit( true );
    pIo->MoveError( *pStm );
    return pStm;
}

// Delete a stream or substorage by setting the temp bit.

bool Storage::Remove( const OUString& rName )
{
    if( !Validate( true ) )
        return false;
    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    if( p )
    {
        p->Invalidate( true );
        return true;
    }
    else
    {
        SetError( SVSTREAM_FILE_NOT_FOUND );
        return false;
    }
}

// Rename a storage element

bool Storage::Rename( const OUString& rOld, const OUString& rNew )
{
    if( Validate( true ) )
    {
        bool b = pIo->pTOC->Rename( *pEntry, rOld, rNew );
        pIo->MoveError( *this );
        return b;
    }
    else
        return false;
}

// Copy one element

bool Storage::CopyTo( const OUString& rElem, BaseStorage* pDest, const OUString& rNew )
{
    if( !Validate() || !pDest || !pDest->Validate( true ) )
        return false;
    StgDirEntry* pElem = pIo->pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        if( pElem->aEntry.GetType() == STG_STORAGE )
        {
            // copy the entire storage
            BaseStorage* p1 = OpenStorage( rElem, INTERNAL_MODE );
            BaseStorage* p2 = pDest->OpenOLEStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pEntry->bDirect );

            if ( p2 )
            {
                sal_uLong nTmpErr = p2->GetError();
                if( !nTmpErr )
                {
                    p2->SetClassId( p1->GetClassId() );
                    p1->CopyTo( p2 );
                    SetError( p1->GetError() );

                    nTmpErr = p2->GetError();
                    if( !nTmpErr )
                        p2->Commit();
                    else
                        pDest->SetError( nTmpErr );
                }
                else
                    pDest->SetError( nTmpErr );
            }

            delete p1;
            delete p2;
            return Good() && pDest->Good();
        }
        else
        {
            // stream copy
            BaseStorageStream* p1 = OpenStream( rElem, INTERNAL_MODE );
            BaseStorageStream* p2 = pDest->OpenStream( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pEntry->bDirect );

            if ( p2 )
            {
                sal_uLong nTmpErr = p2->GetError();
                if( !nTmpErr )
                {
                    p1->CopyTo( p2 );
                    SetError( p1->GetError() );

                    nTmpErr = p2->GetError();
                    if( !nTmpErr )
                        p2->Commit();
                    else
                        pDest->SetError( nTmpErr );
                }
                else
                    pDest->SetError( nTmpErr );
            }

            delete p1;
            delete p2;
            return Good() && pDest->Good();
        }
    }
    SetError( SVSTREAM_FILE_NOT_FOUND );
    return false;
}

bool Storage::CopyTo( BaseStorage* pDest ) const
{
    if( !Validate() || !pDest || !pDest->Validate( true ) || Equals( *pDest ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return false;
    }
    Storage* pThis = (Storage*) this;
    pDest->SetClassId( GetClassId() );
    pDest->SetDirty();
    SvStorageInfoList aList;
    FillInfoList( &aList );
    bool bRes = true;
    for( size_t i = 0; i < aList.size() && bRes; i++ )
    {
        SvStorageInfo& rInfo = aList[ i ];
        bRes = pThis->CopyTo( rInfo.GetName(), pDest, rInfo.GetName() );
    }
    if( !bRes )
        SetError( pDest->GetError() );
    return Good() && pDest->Good();
}

// Move one element

bool Storage::MoveTo( const OUString& rElem, BaseStorage* pODest, const OUString& rNew )
{
    if( !Validate() || !pODest || !pODest->Validate( true ) || Equals( *pODest ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return false;
    }

    StgDirEntry* pElem = pIo->pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        // Simplest case: both storages share the same file
        bool bRes;
        Storage *pOther = PTR_CAST( Storage, pODest );
        if( pOther && pIo == pOther->pIo && rElem == rNew )
        {
            Storage *p = (Storage*) pODest;
            Storage *pDest = p;
            // both storages are conventional storages, use implementation dependent code
            if( !pElem->IsContained( pDest->pEntry ) )
            {
                // cyclic move
                SetError( SVSTREAM_ACCESS_DENIED );
                return false;
            }
            bRes = pIo->pTOC->Move( *pEntry, *pDest->pEntry, rNew );
            if( !bRes )
            {
                pIo->MoveError( *this );
                pDest->pIo->MoveError( *pDest );
                sal_uLong nErr = GetError();
                if( !nErr )
                    nErr = pDest->GetError();
                SetError( nErr );
                pDest->SetError( nErr );
            }
        }
        else
        {
            bRes = CopyTo( rElem, pODest, rNew );
            if( bRes )
                bRes = Remove( rElem );
        }
        if( !bRes )
            SetError( pIo->GetError() );
        return bRes;
    }
    SetError( SVSTREAM_FILE_NOT_FOUND );
    return false;
}

bool Storage::IsStorage( const OUString& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
        if( p )
            return p->aEntry.GetType() == STG_STORAGE;
    }
    return false;
}

bool Storage::IsStream( const OUString& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
        if( p )
            return p->aEntry.GetType() == STG_STREAM;
    }
    return false;
}

bool Storage::IsContained( const OUString& rName ) const
{
    if( Validate() )
        return pIo->pTOC->Find( *pEntry, rName ) != NULL;
    else
        return false;
}

// Commit all sub-elements within this storage. If this is
// the root, commit the FAT, the TOC and the header as well.

bool Storage::Commit()
{
    bool bRes = true;
    if( !Validate() )
        return false;
    if( !( m_nMode & STREAM_WRITE ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return false;
    }
    else
    {
        // Also commit the sub-streams and Storages
        StgIterator aIter( *pEntry );
        for( StgDirEntry* p = aIter.First(); p && bRes; p = aIter.Next() )
            bRes = p->Commit();
        if( bRes && bIsRoot )
        {
            bRes = pEntry->Commit();
            if( bRes )
                bRes = pIo->CommitAll();
        }
        pIo->MoveError( *this );
    }
    return bRes;
}

bool Storage::Revert()
{
    return true;
}

///////////////////////////// OLE Support ////////////////////////////////

// Set the storage type

void Storage::SetClass( const SvGlobalName & rClass,
                        sal_uLong nOriginalClipFormat,
                        const OUString & rUserTypeName )
{
    if( Validate( true ) )
    {
        // set the class name in the root entry
        pEntry->aEntry.SetClassId( (const ClsId&) rClass.GetCLSID() );
        pEntry->SetDirty();
        // then create the streams
        StgCompObjStream aCompObj( *this, true );
        aCompObj.GetClsId() = (const ClsId&) rClass.GetCLSID();
        aCompObj.GetCbFormat() = nOriginalClipFormat;
        aCompObj.GetUserName() = rUserTypeName;
        if( !aCompObj.Store() )
            SetError( aCompObj.GetError() );
        else
        {
            StgOleStream aOle(*this, true);
            if( !aOle.Store() )
                SetError( aOle.GetError() );
        }
    }
    else
        SetError( SVSTREAM_ACCESS_DENIED );
}

void Storage::SetConvertClass( const SvGlobalName & rConvertClass,
                               sal_uLong nOriginalClipFormat,
                               const OUString & rUserTypeName )
{
    if( Validate( true ) )
    {
        SetClass( rConvertClass, nOriginalClipFormat, rUserTypeName );
        // plus the convert flag:
        StgOleStream aOle( *this, true );
        aOle.GetFlags() |= 4;
        if( !aOle.Store() )
            SetError( aOle.GetError() );
    }
}

SvGlobalName Storage::GetClassName()
{
    StgCompObjStream aCompObj( *this, false );
    if( aCompObj.Load() )
        return SvGlobalName( (const CLSID&) aCompObj.GetClsId() );
    pIo->ResetError();

    if ( pEntry )
        return SvGlobalName( (const CLSID&) pEntry->aEntry.GetClassId() );

    return SvGlobalName();
}

sal_uLong Storage::GetFormat()
{
    StgCompObjStream aCompObj( *this, false );
    if( aCompObj.Load() )
        return aCompObj.GetCbFormat();
    pIo->ResetError();
    return 0;
}

OUString Storage::GetUserName()
{
    StgCompObjStream aCompObj( *this, false );
    if( aCompObj.Load() )
        return aCompObj.GetUserName();
    pIo->ResetError();
    return OUString();
}

bool Storage::ShouldConvert()
{
    StgOleStream aOle( *this, false );
    if( aOle.Load() )
        return ( aOle.GetFlags() & 4 ) != 0;
    else
    {
        pIo->ResetError();
        return false;
    }
}

bool Storage::ValidateFAT()
{
    Link aLink = StgIo::GetErrorLink();
    ErrCode nErr = pIo->ValidateFATs();
    StgIo::SetErrorLink( aLink );
    return nErr == ERRCODE_NONE;
}

void Storage::SetDirty()
{
    if ( pEntry )
        pEntry->SetDirty();
}

void Storage::SetClassId( const ClsId& rId )
{
    if ( pEntry )
        pEntry->aEntry.SetClassId( rId );
}

const ClsId& Storage::GetClassId() const
{
    if ( pEntry )
        return pEntry->aEntry.GetClassId();

    static ClsId aDummyId = {0,0,0,0,0,0,0,0,0,0,0};
    return aDummyId;
}

const SvStream* Storage::GetSvStream() const
{
    return GetSvStream_Impl();
}

bool Storage::Validate( bool bValidate ) const
{
    bool bRet = Validate_Impl( bValidate );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

bool Storage::ValidateMode( StreamMode nMode ) const
{
    bool bRet = ValidateMode_Impl( nMode );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

bool Storage::ValidateMode( StreamMode nMode, StgDirEntry* p ) const
{
    bool bRet = ValidateMode_Impl( nMode, p );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

bool Storage::Equals( const BaseStorage& rStorage ) const
{
    const Storage* pOther = PTR_CAST( Storage, &rStorage );
    return pOther && ( pOther->pEntry == pEntry );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
