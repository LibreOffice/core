/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sot/storinfo.hxx>
#include <osl/file.hxx>
#include <tools/tempfile.hxx>
#include <tools/ownlist.hxx>
#include <tools/string.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/pstm.hxx>
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
    : m_bAutoCommit( sal_False )
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
    }


    if( !pIo->DecRef() )
        delete pIo;
}

// Validate the instance for I/O

sal_Bool OLEStorageBase::Validate_Impl( sal_Bool bWrite ) const
{
    if( pEntry
        && !pEntry->bInvalid
        &&  ( !bWrite || !pEntry->bDirect || ( nStreamMode & STREAM_WRITE ) ) )
        return sal_True;
    return sal_False;
}

sal_Bool OLEStorageBase::ValidateMode_Impl( StreamMode m, StgDirEntry* p ) const
{
    if( m == INTERNAL_MODE )
        return sal_True;
    sal_uInt16 nCurMode = ( p && p->nRefCnt ) ? p->nMode : 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return sal_True;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return sal_True;
    }
    return sal_False;
}


//////////////////////// class StorageStream /////////////////////////////

TYPEINIT1( StorageStream, BaseStorageStream );

StorageStream::StorageStream( StgIo* p, StgDirEntry* q, StreamMode m )
             : OLEStorageBase( p, q, m_nMode ), nPos( 0L )
{
    // The dir entry may be 0; this means that the stream is invalid.
    if( q )
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

sal_Bool StorageStream::Equals( const BaseStorageStream& rStream ) const
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
    if( Validate( sal_True ) )
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

sal_Bool StorageStream::SetSize( sal_uLong nNewSize )
{
    if( Validate( sal_True ) )
    {
        sal_Bool b = pEntry->SetSize( (sal_Int32) nNewSize );
        pIo->MoveError( *this );
        return b;
    }
    else
        return sal_False;
}

sal_Bool StorageStream::Commit()
{
    if( !Validate() )
        return sal_False;
    if( !( m_nMode & STREAM_WRITE ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return sal_False;
    }
    else
    {
        pEntry->Commit();
        pIo->MoveError( *this );
        return Good();
    }
}

sal_Bool StorageStream::Revert()
{
    pEntry->Revert();
    pIo->MoveError( *this );
    return Good();
}

sal_Bool StorageStream::CopyTo( BaseStorageStream* pDest )
{
    if( !Validate() || !pDest->Validate( sal_True ) || Equals( *pDest ) )
        return sal_False;
    pEntry->Copy( *pDest );
    pDest->Commit();
    pIo->MoveError( *this );
    SetError( pDest->GetError() );
    return sal_Bool( Good() && pDest->Good() );
}

const SvStream* StorageStream::GetSvStream() const
{
    return GetSvStream_Impl();
}

sal_Bool StorageStream::Validate( sal_Bool bValidate ) const
{
    sal_Bool bRet = Validate_Impl( bValidate );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

sal_Bool StorageStream::ValidateMode( StreamMode nMode ) const
{
    sal_Bool bRet = ValidateMode_Impl( nMode, NULL );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

sal_Bool StorageStream::ValidateMode( StreamMode nMode, StgDirEntry* p ) const
{
    sal_Bool bRet = ValidateMode_Impl( nMode, p );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

///////////////////////// class SvStorageInfo //////////////////////////////

SvStorageInfo::SvStorageInfo( const StgDirEntry& rE )
{
    rE.aEntry.GetName( aName );
    bStorage = sal_Bool( rE.aEntry.GetType() == STG_STORAGE );
    bStream  = sal_Bool( rE.aEntry.GetType() == STG_STREAM );
    nSize    = bStorage ? 0 : rE.aEntry.GetSize();
}

/////////////////////////// class Storage ////////////////////////////////

sal_Bool Storage::IsStorageFile( const String & rFileName )
{
    StgIo aIo;
    if( aIo.Open( rFileName, STREAM_STD_READ ) )
        return aIo.Load();
    return sal_False;
}

sal_Bool Storage::IsStorageFile( SvStream* pStream )
{
    StgHeader aHdr;
    sal_uLong nPos = pStream->Tell();
    sal_Bool bRet = ( aHdr.Load( *pStream ) && aHdr.Check() );

    // It's not a stream error if it is too small for a OLE storage header
    if ( pStream->GetErrorCode() == ERRCODE_IO_CANTSEEK )
        pStream->ResetError();
    pStream->Seek( nPos );
    return bRet;
}

// Open the storage file. If writing is permitted and the file is not
// a storage file, initialize it.

TYPEINIT1( Storage, BaseStorage );

Storage::Storage( const String& rFile, StreamMode m, sal_Bool bDirect )
       : OLEStorageBase( new StgIo, NULL, m_nMode ), aName( rFile ), bIsRoot( sal_False )
{
    sal_Bool bTemp = sal_False;
    if( !aName.Len() )
    {
        // no name = temporary name!
        aName = TempFile::CreateTempName();
        bTemp = sal_True;
    }
    // the root storage creates the I/O system
    m_nMode = m;
    if( pIo->Open( aName, m ) )
    {
        Init( sal_Bool( ( m & ( STREAM_TRUNC | STREAM_NOCREATE ) ) == STREAM_TRUNC ) );
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

Storage::Storage( SvStream& r, sal_Bool bDirect )
       : OLEStorageBase( new StgIo, NULL, m_nMode ), bIsRoot( sal_False )
{
    m_nMode = STREAM_READ;
    if( r.IsWritable() )
        m_nMode = STREAM_READ | STREAM_WRITE;
    if( r.GetError() == SVSTREAM_OK )
    {
        pIo->SetStrm( &r, sal_False );
        sal_uLong nSize = r.Seek( STREAM_SEEK_TO_END );
        r.Seek( 0L );
        // Initializing is OK if the stream is empty
        Init( sal_Bool( nSize == 0 ) );
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


Storage::Storage( UCBStorageStream& rStrm, sal_Bool bDirect )
       : OLEStorageBase( new StgIo, NULL, m_nMode ), bIsRoot( sal_False )
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
    Init( sal_Bool( nSize == 0 ) );
    if( pEntry )
    {
        pEntry->bDirect = bDirect;
        pEntry->nMode = m_nMode;
    }

    pIo->MoveError( *this );
}


// Perform common code for both ctors above.

void Storage::Init( sal_Bool bCreate )
{
    pEntry = NULL;
    sal_Bool bHdrLoaded = sal_False;
    bIsRoot = sal_True;
    if( pIo->Good() )
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
    if( pIo->Good() )
    {
        pEntry = pIo->pTOC->GetRoot();
        pEntry->nRefCnt++;
    }
}

// Internal ctor

Storage::Storage( StgIo* p, StgDirEntry* q, StreamMode m )
       : OLEStorageBase( p, q, m_nMode ), bIsRoot( sal_False )
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

const String& Storage::GetName() const
{
    if( !bIsRoot && Validate() )
        pEntry->aEntry.GetName( ((Storage*) this)->aName );
    return aName;
}

// Fill in the info list for this storage

void Storage::FillInfoList( SvStorageInfoList* pList ) const
{
    if( Validate() )
    {
        StgIterator aIter( *pEntry );
        StgDirEntry* p = aIter.First();
        while( p )
        {
            if( !p->bInvalid )
            {
                SvStorageInfo aInfo( *p );
                pList->Append( aInfo );
            }
            p = aIter.Next();
        }
    }
}

// Open or create a substorage

BaseStorage* Storage::OpenUCBStorage( const String& rName, StreamMode m, sal_Bool bDirect )
{
    OSL_FAIL("Not supported!");
/*
    BaseStorage* pStorage = new Storage( pIo, NULL, m );
    SetError( ERRCODE_IO_NOTSUPPORTED );
    return pStorage;
 */
    return OpenStorage( rName, m, bDirect );
}

BaseStorage* Storage::OpenOLEStorage( const String& rName, StreamMode m, sal_Bool bDirect )
{
    return OpenStorage( rName, m, bDirect );
}

BaseStorage* Storage::OpenStorage( const String& rName, StreamMode m, sal_Bool bDirect )
{
    if( !Validate() || !ValidateMode( m ) )
        return new Storage( pIo, NULL, m );
    if( bDirect && !pEntry->bDirect )
        bDirect = sal_False;

    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    if( !p )
    {
        if( !( m & STREAM_NOCREATE ) )
        {
            sal_Bool bTemp = sal_False;
            // create a new storage
            String aNewName = rName;
            if( !aNewName.Len() )
            {
                aNewName.AssignAscii( "Temp Stg " );
                aNewName.Append( String::CreateFromInt32( ++nTmpCount ) );
                bTemp = sal_True;
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
    if( m & STREAM_WRITE ) pStg->m_bAutoCommit = sal_True;
    return pStg;
}

// Open a stream

BaseStorageStream* Storage::OpenStream( const String& rName, StreamMode m, sal_Bool,
const ByteString*
#ifdef DBG_UTIL
pB
#endif
)
{
    DBG_ASSERT(!pB, "Encryption not supported");

    if( !Validate() || !ValidateMode( m ) )
        return new StorageStream( pIo, NULL, m );
    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    sal_Bool bTemp = sal_False;
    if( !p )
    {
        if( !( m & STREAM_NOCREATE ) )
        {
            // create a new stream
            // make a name if the stream is temporary (has no name)
            String aNewName( rName );
            if( !aNewName.Len() )
            {
                aNewName.AssignAscii( "Temp Strm " );
                aNewName.Append( String::CreateFromInt32( ++nTmpCount ) );
                bTemp = sal_True;
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
        pStm->SetAutoCommit( sal_True );
    pIo->MoveError( *pStm );
    return pStm;
}

// Delete a stream or substorage by setting the temp bit.

sal_Bool Storage::Remove( const String& rName )
{
    if( !Validate( sal_True ) )
        return sal_False;
    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    if( p )
    {
        p->Invalidate( sal_True );
        return sal_True;
    }
    else
    {
        SetError( SVSTREAM_FILE_NOT_FOUND );
        return sal_False;
    }
}

// Rename a storage element

sal_Bool Storage::Rename( const String& rOld, const String& rNew )
{
    if( Validate( sal_True ) )
    {
        sal_Bool b = pIo->pTOC->Rename( *pEntry, rOld, rNew );
        pIo->MoveError( *this );
        return b;
    }
    else
        return sal_False;
}

// Copy one element

sal_Bool Storage::CopyTo( const String& rElem, BaseStorage* pDest, const String& rNew )
{
    if( !Validate() || !pDest || !pDest->Validate( sal_True ) )
        return sal_False;
    StgDirEntry* pElem = pIo->pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        /*
        this lines are misterious !!! MM
        if( !pElem->IsContained( pDest->pEntry ) )
        {
            SetError( SVSTREAM_ACCESS_DENIED );
            return sal_False;
        }
        */
        if( pElem->aEntry.GetType() == STG_STORAGE )
        {
            // copy the entire storage
            BaseStorage* p1 = OpenStorage( rElem, INTERNAL_MODE );
            BaseStorage* p2 = pDest->OpenOLEStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pEntry->bDirect );

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

            delete p1;
            delete p2;
            return sal_Bool( Good() && pDest->Good() );
        }
        else
        {
            // stream copy
            BaseStorageStream* p1 = OpenStream( rElem, INTERNAL_MODE );
            BaseStorageStream* p2 = pDest->OpenStream( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pEntry->bDirect );

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

            delete p1;
            delete p2;
            return sal_Bool( Good() && pDest->Good() );
        }
    }
    SetError( SVSTREAM_FILE_NOT_FOUND );
    return sal_False;
}

sal_Bool Storage::CopyTo( BaseStorage* pDest ) const
{
    if( !Validate() || !pDest || !pDest->Validate( sal_True ) || Equals( *pDest ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return sal_False;
    }
    Storage* pThis = (Storage*) this;
    /*
    if( !pThis->pEntry->IsContained( pDest->pEntry ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return sal_False;
    }
    */
    pDest->SetClassId( GetClassId() );
    pDest->SetDirty();
    SvStorageInfoList aList;
    FillInfoList( &aList );
    sal_Bool bRes = sal_True;
    for( sal_uInt16 i = 0; i < aList.Count() && bRes; i++ )
    {
        SvStorageInfo& rInfo = aList.GetObject( i );
        bRes = pThis->CopyTo( rInfo.GetName(), pDest, rInfo.GetName() );
    }
    if( !bRes )
        SetError( pDest->GetError() );
    return sal_Bool( Good() && pDest->Good() );
}

// Move one element

sal_Bool Storage::MoveTo( const String& rElem, BaseStorage* pODest, const String& rNew )
{
    if( !Validate() || !pODest || !pODest->Validate( sal_True ) || Equals( *pODest ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return sal_False;
    }

    StgDirEntry* pElem = pIo->pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        // Simplest case: both storages share the same file
        sal_Bool bRes;
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
                return sal_False;
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
    return sal_False;
}

sal_Bool Storage::IsStorage( const String& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
        if( p )
            return sal_Bool( p->aEntry.GetType() == STG_STORAGE );
    }
    return sal_False;
}

sal_Bool Storage::IsStream( const String& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
        if( p )
            return sal_Bool( p->aEntry.GetType() == STG_STREAM );
    }
    return sal_False;
}

sal_Bool Storage::IsContained( const String& rName ) const
{
    if( Validate() )
        return sal_Bool( pIo->pTOC->Find( *pEntry, rName ) != NULL );
    else
        return sal_False;
}

// Commit all sub-elements within this storage. If this is
// the root, commit the FAT, the TOC and the header as well.

sal_Bool Storage::Commit()
{
    sal_Bool bRes = sal_True;
    if( !Validate() )
        return sal_False;
    if( !( m_nMode & STREAM_WRITE ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return sal_False;
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

sal_Bool Storage::Revert()
{
    return sal_True;
}

///////////////////////////// OLE Support ////////////////////////////////

// Set the storage type

void Storage::SetClass( const SvGlobalName & rClass,
                                sal_uLong nOriginalClipFormat,
                                const String & rUserTypeName )
{
    if( Validate( sal_True ) )
    {
        // set the class name in the root entry
        pEntry->aEntry.SetClassId( (const ClsId&) rClass.GetCLSID() );
        pEntry->SetDirty();
        // then create the streams
        StgCompObjStream aCompObj( *this, sal_True );
        aCompObj.GetClsId() = (const ClsId&) rClass.GetCLSID();
        aCompObj.GetCbFormat() = nOriginalClipFormat;
        aCompObj.GetUserName() = rUserTypeName;
        if( !aCompObj.Store() )
            SetError( aCompObj.GetError() );
        else
        {
            StgOleStream aOle( *this, STREAM_WRITE );
            if( !aOle.Store() )
                SetError( aOle.GetError() );
        }
    }
    else
        SetError( SVSTREAM_ACCESS_DENIED );
}

void Storage::SetConvertClass( const SvGlobalName & rConvertClass,
                                       sal_uLong nOriginalClipFormat,
                                       const String & rUserTypeName )
{
    if( Validate( sal_True ) )
    {
        SetClass( rConvertClass, nOriginalClipFormat, rUserTypeName );
        // plus the convert flag:
        StgOleStream aOle( *this, sal_True );
        aOle.GetFlags() |= 4;
        if( !aOle.Store() )
            SetError( aOle.GetError() );
    }
}

SvGlobalName Storage::GetClassName()
{
    StgCompObjStream aCompObj( *this, sal_False );
    if( aCompObj.Load() )
        return SvGlobalName( (const CLSID&) aCompObj.GetClsId() );
    pIo->ResetError();

    if ( pEntry )
        return SvGlobalName( (const CLSID&) pEntry->aEntry.GetClassId() );

    return SvGlobalName();
}

sal_uLong Storage::GetFormat()
{
    StgCompObjStream aCompObj( *this, sal_False );
    if( aCompObj.Load() )
        return aCompObj.GetCbFormat();
    pIo->ResetError();
    return 0;
}

String Storage::GetUserName()
{
    StgCompObjStream aCompObj( *this, sal_False );
    if( aCompObj.Load() )
        return aCompObj.GetUserName();
    pIo->ResetError();
    return String();
}

sal_Bool Storage::ShouldConvert()
{
    StgOleStream aOle( *this, sal_False );
    if( aOle.Load() )
        return sal_Bool( ( aOle.GetFlags() & 4 ) != 0 );
    else
    {
        pIo->ResetError();
        return sal_False;
    }
}

sal_Bool Storage::ValidateFAT()
{
    Link aLink = StgIo::GetErrorLink();
    ErrCode nErr = pIo->ValidateFATs();
    StgIo::SetErrorLink( aLink );
    return nErr == ERRCODE_NONE;
}

void Storage::SetDirty()
{
    pEntry->SetDirty();
}

void Storage::SetClassId( const ClsId& rId )
{
    pEntry->aEntry.SetClassId( rId );
}

const ClsId& Storage::GetClassId() const
{
    return pEntry->aEntry.GetClassId();
}

const SvStream* Storage::GetSvStream() const
{
    return GetSvStream_Impl();
}

sal_Bool Storage::Validate( sal_Bool bValidate ) const
{
    sal_Bool bRet = Validate_Impl( bValidate );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

sal_Bool Storage::ValidateMode( StreamMode nMode ) const
{
    sal_Bool bRet = ValidateMode_Impl( nMode );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

sal_Bool Storage::ValidateMode( StreamMode nMode, StgDirEntry* p ) const
{
    sal_Bool bRet = ValidateMode_Impl( nMode, p );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

sal_Bool Storage::Equals( const BaseStorage& rStorage ) const
{
    const Storage* pOther = PTR_CAST( Storage, &rStorage );
    return pOther && ( pOther->pEntry == pEntry );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
