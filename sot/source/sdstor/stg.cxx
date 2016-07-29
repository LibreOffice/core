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
#include <sot/exchange.hxx>
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

// The internal open mode is StreamMode::READ | StreamMode::TRUNC, which is silly
// by itself. It inhibits the checking of sharing modes and is used
// during CopyTo() and MoveTo() for opening a stream in read mode
// although it may be open in DENYALL mode

#define INTERNAL_MODE ( StreamMode::READ | StreamMode::TRUNC )

///////////////////////// class StorageBase


StorageBase::StorageBase()
    : m_bAutoCommit( false )
{
    m_nMode  = StreamMode::READ;
    m_nError = SVSTREAM_OK;
}

StorageBase::~StorageBase()
{
}

// The following three methods are declared as const, since they
// may be called from within a const method.

ErrCode StorageBase::GetError() const
{
    const ErrCode n = m_nError;
    m_nError = SVSTREAM_OK;
    return n;
}

void StorageBase::SetError( ErrCode n ) const
{
    if( !m_nError )
        m_nError = n;
}

void StorageBase::ResetError() const
{
    m_nError = SVSTREAM_OK;
}

OLEStorageBase::OLEStorageBase( StgIo* p, StgDirEntry* pe, StreamMode& nMode )
    : nStreamMode( nMode ), pIo( p ), pEntry( pe )
{
    if ( p )
        p->IncRef();
    if( pe )
        pe->m_nRefCnt++;
}

OLEStorageBase::~OLEStorageBase()
{
    if( pEntry )
    {
        DBG_ASSERT( pEntry->m_nRefCnt, "RefCount unter 0" );
        if( !--pEntry->m_nRefCnt )
        {
            if( pEntry->m_bZombie )
                delete pEntry;
            else
                pEntry->Close();
        }

        pEntry = nullptr;
    }


    if( pIo && !pIo->DecRef() )
    {
        delete pIo;
        pIo = nullptr;
    }
}

// Validate the instance for I/O

bool OLEStorageBase::Validate_Impl( bool bWrite ) const
{
    if( pIo
        && pIo->m_pTOC
        && pEntry
        && !pEntry->m_bInvalid
        &&  ( !bWrite || !pEntry->m_bDirect || ( nStreamMode & StreamMode::WRITE ) ) )
            return true;
    return false;
}

bool OLEStorageBase::ValidateMode_Impl( StreamMode m, StgDirEntry* p )
{
    if( m == INTERNAL_MODE )
        return true;
    StreamMode nCurMode = ( p && p->m_nRefCnt ) ? p->m_nMode : StreamMode::SHARE_DENYALL;
    if( ( m & STREAM_READWRITE ) == StreamMode::READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & StreamMode::SHARE_DENYWRITE )
           && ( nCurMode & StreamMode::SHARE_DENYWRITE ) )
         || ( ( m & StreamMode::SHARE_DENYALL )
           && ( nCurMode & StreamMode::SHARE_DENYALL ) ) )
            return true;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & StreamMode::SHARE_DENYALL )
         && ( nCurMode & StreamMode::SHARE_DENYALL ) )
            return true;
    }
    return false;
}


//////////////////////// class StorageStream


StorageStream::StorageStream( StgIo* p, StgDirEntry* q, StreamMode m )
             : OLEStorageBase( p, q, m_nMode ), nPos( 0L )
{
    // The dir entry may be 0; this means that the stream is invalid.
    if( q && p )
    {
        if( q->m_nRefCnt == 1 )
        {
            q->m_nMode = m;
            q->OpenStream( *p );
        }
    }
    else
        m &= ~StreamMode(STREAM_READWRITE);
    m_nMode = m;
}

StorageStream::~StorageStream()
{
    // Do an auto-commit if the entry is open in direct mode
    if( m_bAutoCommit )
        Commit();
    if( pEntry && pEntry->m_nRefCnt && pEntry->m_bDirect && (m_nMode & StreamMode::WRITE) )
        pEntry->Commit();
}

bool StorageStream::Equals( const BaseStorageStream& rStream ) const
{
    const StorageStream* pOther =  dynamic_cast<const StorageStream*>( &rStream );
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

sal_uInt64 StorageStream::Seek( sal_uInt64 n )
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
    if( !( m_nMode & StreamMode::WRITE ) )
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

void StorageStream::CopyTo( BaseStorageStream* pDest )
{
    if( !Validate() || !pDest || !pDest->Validate( true ) || Equals( *pDest ) )
        return;
    pEntry->Copy( *pDest );
    pDest->Commit();
    pIo->MoveError( *this );
    SetError( pDest->GetError() );
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
    bool bRet = ValidateMode_Impl( nMode );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

///////////////////////// class SvStorageInfo

SvStorageInfo::SvStorageInfo( const StgDirEntry& rE )
{
    rE.m_aEntry.GetName( aName );
    bStorage = rE.m_aEntry.GetType() == STG_STORAGE;
    bStream  = rE.m_aEntry.GetType() == STG_STREAM;
    nSize    = bStorage ? 0 : rE.m_aEntry.GetSize();
}

/////////////////////////// class Storage

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


Storage::Storage( const OUString& rFile, StreamMode m, bool bDirect )
    : OLEStorageBase( new StgIo, nullptr, m_nMode )
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
        Init( ( m & ( StreamMode::TRUNC | StreamMode::NOCREATE ) ) == StreamMode::TRUNC );
        if( pEntry )
        {
            pEntry->m_bDirect = bDirect;
            pEntry->m_nMode = m;
            pEntry->m_bTemp = bTemp;
        }
    }
    else
    {
        pIo->MoveError( *this );
        pEntry = nullptr;
    }
}

// Create a storage on a given stream.

Storage::Storage( SvStream& r, bool bDirect )
    : OLEStorageBase( new StgIo, nullptr, m_nMode )
    , bIsRoot( false )
{
    m_nMode = StreamMode::READ;
    if( r.IsWritable() )
        m_nMode = StreamMode::READ | StreamMode::WRITE;
    if( r.GetError() == SVSTREAM_OK )
    {
        pIo->SetStrm( &r, false );
        sal_uLong nSize = r.Seek( STREAM_SEEK_TO_END );
        r.Seek( 0L );
        // Initializing is OK if the stream is empty
        Init( nSize == 0 );
        if( pEntry )
        {
            pEntry->m_bDirect = bDirect;
            pEntry->m_nMode = m_nMode;
        }
        pIo->MoveError( *this );
    }
    else
    {
        SetError( r.GetError() );
        pEntry = nullptr;
    }
}


Storage::Storage( UCBStorageStream& rStrm, bool bDirect )
       : OLEStorageBase( new StgIo, nullptr, m_nMode ), bIsRoot( false )
{
    m_nMode = StreamMode::READ;

    if ( rStrm.GetError() != SVSTREAM_OK )
    {
        SetError( rStrm.GetError() );
        pEntry = nullptr;
        return;
    }

    SvStream* pStream = rStrm.GetModifySvStream();
    if ( !pStream )
    {
        OSL_FAIL( "UCBStorageStream can not provide SvStream implementation!\n" );
        SetError( SVSTREAM_GENERALERROR );
        pEntry = nullptr;
        return;
    }

    if( pStream->IsWritable() )
        m_nMode = StreamMode::READ | StreamMode::WRITE;

    pIo->SetStrm( &rStrm );

    sal_uLong nSize = pStream->Seek( STREAM_SEEK_TO_END );
    pStream->Seek( 0L );
    // Initializing is OK if the stream is empty
    Init( nSize == 0 );
    if( pEntry )
    {
        pEntry->m_bDirect = bDirect;
        pEntry->m_nMode = m_nMode;
    }

    pIo->MoveError( *this );
}


// Perform common code for both ctors above.

void Storage::Init( bool bCreate )
{
    pEntry = nullptr;
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
    if( pIo->Good() && pIo->m_pTOC )
    {
        pEntry = pIo->m_pTOC->GetRoot();
        pEntry->m_nRefCnt++;
    }
}

// Internal ctor

Storage::Storage( StgIo* p, StgDirEntry* q, StreamMode m )
       : OLEStorageBase( p, q, m_nMode ), bIsRoot( false )
{
    if( q )
        q->m_aEntry.GetName( aName );
    else
        m &= ~StreamMode(STREAM_READWRITE);
    m_nMode   = m;
    if( q && q->m_nRefCnt == 1 )
        q->m_nMode = m;
}

Storage::~Storage()
{
    // Invalidate all open substorages
    if( m_bAutoCommit )
        Commit();
    if( pEntry )
    {
        // Do an auto-commit if the entry is open in direct mode
        if( pEntry->m_nRefCnt && pEntry->m_bDirect && (m_nMode & StreamMode::WRITE) )
            Commit();
        if( pEntry->m_nRefCnt == 1 )
            pEntry->Invalidate(false);
    }
    // close the stream is root storage
    if( bIsRoot )
        pIo->Close();
    // remove the file if temporary root storage
    if( bIsRoot && pEntry && pEntry->m_bTemp )
    {
        osl::File::remove( GetName() );
    }
}

const OUString& Storage::GetName() const
{
    if( !bIsRoot && Validate() )
        pEntry->m_aEntry.GetName( const_cast<Storage*>(this)->aName );
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
            if( !p->m_bInvalid )
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
        return new Storage( pIo, nullptr, m );
    if( bDirect && !pEntry->m_bDirect )
        bDirect = false;

    StgDirEntry* p = pIo->m_pTOC->Find( *pEntry, rName );
    if( !p )
    {
        if( !( m & StreamMode::NOCREATE ) )
        {
            bool bTemp = false;
            // create a new storage
            OUString aNewName = rName;
            if( aNewName.isEmpty() )
            {
                aNewName = "Temp Stg " + OUString::number( ++nTmpCount );
                bTemp = true;
            }
            p = pIo->m_pTOC->Create( *pEntry, aNewName, STG_STORAGE );
            if( p )
                p->m_bTemp = bTemp;
        }
        if( !p )
            pIo->SetError( ( m & StreamMode::WRITE )
                             ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
    }
    else if( !ValidateMode( m, p ) )
        p = nullptr;
    if( p && p->m_aEntry.GetType() != STG_STORAGE )
    {
        pIo->SetError( SVSTREAM_FILE_NOT_FOUND );
        p = nullptr;
    }

    // Either direct or transacted mode is supported
    if( p && pEntry->m_nRefCnt == 1 )
        p->m_bDirect = bDirect;

    // Don't check direct conflict if opening readonly
    if( p && (m & StreamMode::WRITE ))
    {
        if( p->m_bDirect != bDirect )
            SetError( SVSTREAM_ACCESS_DENIED );
    }
    Storage* pStg = new Storage( pIo, p, m );
    pIo->MoveError( *pStg );
    if( m & StreamMode::WRITE ) pStg->m_bAutoCommit = true;
    return pStg;
}

// Open a stream

BaseStorageStream* Storage::OpenStream( const OUString& rName, StreamMode m, bool,
                                        const OString* pB )
{
    DBG_ASSERT(!pB, "Encryption not supported");

    if( !Validate() || !ValidateMode( m ) )
        return new StorageStream( pIo, nullptr, m );
    StgDirEntry* p = pIo->m_pTOC->Find( *pEntry, rName );
    bool bTemp = false;
    if( !p )
    {
        if( !( m & StreamMode::NOCREATE ) )
        {
            // create a new stream
            // make a name if the stream is temporary (has no name)
            OUString aNewName( rName );
            if( aNewName.isEmpty() )
            {
                aNewName = "Temp Strm " + OUString::number( ++nTmpCount );
                bTemp = true;
            }
            p = pIo->m_pTOC->Create( *pEntry, aNewName, STG_STREAM );
        }
        if( !p )
            pIo->SetError( ( m & StreamMode::WRITE )
                           ? SVSTREAM_CANNOT_MAKE : SVSTREAM_FILE_NOT_FOUND );
    }
    else if( !ValidateMode( m, p ) )
        p = nullptr;
    if( p && p->m_aEntry.GetType() != STG_STREAM )
    {
        pIo->SetError( SVSTREAM_FILE_NOT_FOUND );
        p = nullptr;
    }
    if( p )
    {
        p->m_bTemp = bTemp;
        p->m_bDirect = pEntry->m_bDirect;
    }
    StorageStream* pStm = new StorageStream( pIo, p, m );
    if( p && !p->m_bDirect )
        pStm->SetAutoCommit( true );
    pIo->MoveError( *pStm );
    return pStm;
}

// Delete a stream or substorage by setting the temp bit.

void Storage::Remove( const OUString& rName )
{
    if( !Validate( true ) )
        return;
    StgDirEntry* p = pIo->m_pTOC->Find( *pEntry, rName );
    if( p )
    {
        p->Invalidate( true );
    }
    else
    {
        SetError( SVSTREAM_FILE_NOT_FOUND );
    }
}

// Copy one element

bool Storage::CopyTo( const OUString& rElem, BaseStorage* pDest, const OUString& rNew )
{
    if( !Validate() || !pDest || !pDest->Validate( true ) )
        return false;
    StgDirEntry* pElem = pIo->m_pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        if( pElem->m_aEntry.GetType() == STG_STORAGE )
        {
            // copy the entire storage
            BaseStorage* p1 = OpenStorage( rElem, INTERNAL_MODE );
            BaseStorage* p2 = pDest->OpenOLEStorage( rNew, StreamMode::WRITE | StreamMode::SHARE_DENYALL, pEntry->m_bDirect );

            if ( p2 )
            {
                ErrCode nTmpErr = p2->GetError();
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
            BaseStorageStream* p2 = pDest->OpenStream( rNew, StreamMode::WRITE | StreamMode::SHARE_DENYALL, pEntry->m_bDirect );

            if ( p2 )
            {
                ErrCode nTmpErr = p2->GetError();
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
    Storage* pThis = const_cast<Storage*>(this);
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

bool Storage::IsStorage( const OUString& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->m_pTOC->Find( *pEntry, rName );
        if( p )
            return p->m_aEntry.GetType() == STG_STORAGE;
    }
    return false;
}

bool Storage::IsStream( const OUString& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->m_pTOC->Find( *pEntry, rName );
        if( p )
            return p->m_aEntry.GetType() == STG_STREAM;
    }
    return false;
}

bool Storage::IsContained( const OUString& rName ) const
{
    if( Validate() )
        return pIo->m_pTOC->Find( *pEntry, rName ) != nullptr;
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
    if( !( m_nMode & StreamMode::WRITE ) )
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

///////////////////////////// OLE Support

// Set the storage type

void Storage::SetClass( const SvGlobalName & rClass,
                        SotClipboardFormatId nOriginalClipFormat,
                        const OUString & rUserTypeName )
{
    if( Validate( true ) )
    {
        // set the class name in the root entry
        pEntry->m_aEntry.SetClassId( (const ClsId&) rClass.GetCLSID() );
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

SvGlobalName Storage::GetClassName()
{
    StgCompObjStream aCompObj( *this, false );
    if( aCompObj.Load() )
        return SvGlobalName( aCompObj.GetClsId() );
    pIo->ResetError();

    if ( pEntry )
        return SvGlobalName( pEntry->m_aEntry.GetClassId() );

    return SvGlobalName();
}

SotClipboardFormatId Storage::GetFormat()
{
    StgCompObjStream aCompObj( *this, false );
    if( aCompObj.Load() )
        return aCompObj.GetCbFormat();
    pIo->ResetError();
    return SotClipboardFormatId::NONE;
}

OUString Storage::GetUserName()
{
    StgCompObjStream aCompObj( *this, false );
    if( aCompObj.Load() )
        return aCompObj.GetUserName();
    pIo->ResetError();
    return OUString();
}

bool Storage::ValidateFAT()
{
    Link<StgLinkArg&,void> aLink = StgIo::GetErrorLink();
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
        pEntry->m_aEntry.SetClassId( rId );
}

const ClsId& Storage::GetClassId() const
{
    if ( pEntry )
        return pEntry->m_aEntry.GetClassId();

    static ClsId aDummyId = {0,0,0,{0,0,0,0,0,0,0,0}};
    return aDummyId;
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
    const Storage* pOther =  dynamic_cast<const Storage*>( &rStorage );
    return pOther && ( pOther->pEntry == pEntry );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
