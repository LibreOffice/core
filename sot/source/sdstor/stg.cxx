/*************************************************************************
 *
 *  $RCSfile: stg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mba $ $Date: 2001-02-26 16:16:55 $
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

#include <storinfo.hxx>
#include <osl/file.hxx>
#include <tools/tempfile.hxx>
#ifndef _TOOLS_OWNLIST_HXX
#include <tools/ownlist.hxx>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif
#include <tools/pstm.hxx>
#include <tools/debug.hxx>

#include "stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#include "stgole.hxx"
#pragma hdrstop

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
    : bAutoCommit( FALSE )
{
    nMode  = STREAM_READ;
    nError = SVSTREAM_OK;
}

StorageBase::~StorageBase()
{
}

// The following three methods are declared as const, since they
// may be called from within a const method.

ULONG StorageBase::GetError() const
{
    ULONG n = nError;
    ((StorageBase*) this)->nError = SVSTREAM_OK;
    return n;
}

void StorageBase::SetError( ULONG n ) const
{
    if( !nError )
        ((StorageBase*) this)->nError = n;
}

void StorageBase::ResetError() const
{
    ((StorageBase*) this)->nError = SVSTREAM_OK;
}

// Retrieve the underlying SvStream for info purposes

const SvStream* OLEStorageBase::GetSvStream_Impl() const
{
    return pIo ? pIo->GetStrm() : NULL;
}

OLEStorageBase::OLEStorageBase( StgIo* p, StgDirEntry* pe, StreamMode& nMode )
    : pIo( p ), pEntry( pe ), nStreamMode( nMode )
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

BOOL OLEStorageBase::Validate_Impl( BOOL bWrite ) const
{
    if( pEntry
        && !pEntry->bInvalid
        &&  ( !bWrite || !pEntry->bDirect || ( nStreamMode & STREAM_WRITE ) ) )
        return TRUE;
    return FALSE;
}

BOOL OLEStorageBase::ValidateMode_Impl( StreamMode m, StgDirEntry* p ) const
{
    if( m == INTERNAL_MODE )
        return TRUE;
    USHORT nCurMode = ( p && p->nRefCnt ) ? p->nMode : 0xFFFF;
    if( ( m & 3 ) == STREAM_READ )
    {
        // only SHARE_DENYWRITE or SHARE_DENYALL allowed
        if( ( ( m & STREAM_SHARE_DENYWRITE )
           && ( nCurMode & STREAM_SHARE_DENYWRITE ) )
         || ( ( m & STREAM_SHARE_DENYALL )
           && ( nCurMode & STREAM_SHARE_DENYALL ) ) )
            return TRUE;
    }
    else
    {
        // only SHARE_DENYALL allowed
        // storages open in r/o mode are OK, since only
        // the commit may fail
        if( ( m & STREAM_SHARE_DENYALL )
         && ( nCurMode & STREAM_SHARE_DENYALL ) )
            return TRUE;
    }
    return FALSE;
}


//////////////////////// class StorageStream /////////////////////////////

TYPEINIT1( StorageStream, BaseStorageStream );

StorageStream::StorageStream( StgIo* p, StgDirEntry* q, StreamMode m )
             : OLEStorageBase( p, q, nMode ), nPos( 0L )
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
    nMode = m;
}

StorageStream::~StorageStream()
{
    // Do an auto-commit if the entry is open in direct mode
    if( bAutoCommit )
        Commit();
    if( pEntry && pEntry->nRefCnt && pEntry->bDirect && (nMode & STREAM_WRITE) )
        pEntry->Commit();
}

BOOL StorageStream::Equals( const BaseStorageStream& rStream ) const
{
    const StorageStream* pOther = PTR_CAST( StorageStream, &rStream );
    return pOther && ( pOther->pEntry == pEntry );
}

ULONG StorageStream::Read( void* pData, ULONG nSize )
{
    if( Validate() )
    {
        pEntry->Seek( nPos );
        nSize = pEntry->Read( pData, (INT32) nSize );
        pIo->MoveError( *this );
        nPos += nSize;
    }
    else
        nSize = 0L;
    return nSize;
}

ULONG StorageStream::Write( const void* pData, ULONG nSize )
{
    if( Validate( TRUE ) )
    {
        pEntry->Seek( nPos );
        nSize = pEntry->Write( pData, (INT32) nSize );
        pIo->MoveError( *this );
        nPos += nSize;
    }
    else
        nSize = 0L;
    return nSize;
}

ULONG StorageStream::Seek( ULONG n )
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

BOOL StorageStream::SetSize( ULONG nNewSize )
{
    if( Validate( TRUE ) )
    {
        BOOL b = pEntry->SetSize( (INT32) nNewSize );
        pIo->MoveError( *this );
        return b;
    }
    else
        return FALSE;
}

BOOL StorageStream::Commit()
{
    if( !Validate() )
        return FALSE;
    if( !( nMode & STREAM_WRITE ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return FALSE;
    }
    else
    {
        pEntry->Commit();
        pIo->MoveError( *this );
        return Good();
    }
}

BOOL StorageStream::Revert()
{
    pEntry->Revert();
    pIo->MoveError( *this );
    return Good();
}

BOOL StorageStream::CopyTo( BaseStorageStream* pDest )
{
    if( !Validate() || !pDest->Validate( TRUE ) || Equals( *pDest ) )
        return FALSE;
    pEntry->Copy( *pDest );
    pDest->Commit();
    pIo->MoveError( *this );
    SetError( pDest->GetError() );
    return BOOL( Good() && pDest->Good() );
}

const SvStream* StorageStream::GetSvStream() const
{
    return GetSvStream_Impl();
}

BOOL StorageStream::Validate( BOOL bValidate ) const
{
    BOOL bRet = Validate_Impl( bValidate );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

BOOL StorageStream::ValidateMode( StreamMode nMode ) const
{
    BOOL bRet = ValidateMode_Impl( nMode, NULL );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

BOOL StorageStream::ValidateMode( StreamMode nMode, StgDirEntry* p ) const
{
    BOOL bRet = ValidateMode_Impl( nMode, p );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

///////////////////////// class SvStorageInfo //////////////////////////////

SvStorageInfo::SvStorageInfo( const StgDirEntry& rE )
{
    rE.aEntry.GetName( aName );
    bStorage = BOOL( rE.aEntry.GetType() == STG_STORAGE );
    bStream  = BOOL( rE.aEntry.GetType() == STG_STREAM );
    nSize    = bStorage ? 0 : rE.aEntry.GetSize();
}

/////////////////////////// class Storage ////////////////////////////////

BOOL Storage::IsStorageFile( const String & rFileName )
{
    StgIo aIo;
    if( aIo.Open( rFileName, STREAM_STD_READ ) )
        return aIo.Load();
    return FALSE;
}

BOOL Storage::IsStorageFile( SvStream* pStream )
{
    StgHeader aHdr;
    ULONG nPos = pStream->Tell();
    BOOL bRet = ( aHdr.Load( *pStream ) && aHdr.Check() );
    pStream->Seek( nPos );
    return bRet;
}

// Open the storage file. If writing is permitted and the file is not
// a storage file, initialize it.

TYPEINIT1( Storage, BaseStorage );

Storage::Storage( const String& rFile, StreamMode m, BOOL bDirect )
       : OLEStorageBase( new StgIo, NULL, nMode ), aName( rFile ), bIsRoot( FALSE )
{
    BOOL bTemp = FALSE;
    if( !aName.Len() )
    {
        // no name = temporary name!
        aName = TempFile::CreateTempName();
        bTemp = TRUE;
    }
    // the root storage creates the I/O system
    nMode = m;
    if( pIo->Open( aName, m ) )
    {
        Init( BOOL( ( m & ( STREAM_TRUNC | STREAM_NOCREATE ) ) == STREAM_TRUNC ) );
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

Storage::Storage( SvStream& r, BOOL bDirect )
       : OLEStorageBase( new StgIo, NULL, nMode ), bIsRoot( FALSE )
{
    nMode = STREAM_READ;
    if( r.IsWritable() )
        nMode = STREAM_READ | STREAM_WRITE;
    if( r.GetError() == SVSTREAM_OK )
    {
        pIo->SetStrm( &r, FALSE );
        ULONG nSize = r.Seek( STREAM_SEEK_TO_END );
        r.Seek( 0L );
        // Initializing is OK if the stream is empty
        Init( BOOL( nSize == 0 ) );
        if( pEntry )
        {
            pEntry->bDirect = bDirect;
            pEntry->nMode = nMode;
        }
        pIo->MoveError( *this );
    }
    else
    {
        SetError( r.GetError() );
        pEntry = NULL;
    }
}

// Perform common code for both ctors above.

void Storage::Init( BOOL bCreate )
{
    pEntry = NULL;
    BOOL bHdrLoaded = FALSE;
    bIsRoot = TRUE;
    if( pIo->Good() )
    {
        ULONG nSize = pIo->GetStrm()->Seek( STREAM_SEEK_TO_END );
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
       : OLEStorageBase( p, q, nMode ), bIsRoot( FALSE )
{
    if( q )
        q->aEntry.GetName( aName );
    else
        m &= ~STREAM_READWRITE;
    nMode   = m;
    if( q && q->nRefCnt == 1 )
        q->nMode = m;
}

Storage::~Storage()
{
    // Invalidate all open substorages
    if( bAutoCommit )
        Commit();
    if( pEntry )
    {
        // Do an auto-commit if the entry is open in direct mode
        if( pEntry->nRefCnt && pEntry->bDirect && (nMode & STREAM_WRITE) )
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

BaseStorage* Storage::OpenUCBStorage( const String& rName, StreamMode m, BOOL bDirect )
{
    DBG_ERROR("Not supported!");
/*
    BaseStorage* pStorage = new Storage( pIo, NULL, m );
    SetError( ERRCODE_IO_NOTSUPPORTED );
    return pStorage;
 */
    return OpenStorage( rName, m, bDirect );
}

BaseStorage* Storage::OpenOLEStorage( const String& rName, StreamMode m, BOOL bDirect )
{
    return OpenStorage( rName, m, bDirect );
}

BaseStorage* Storage::OpenStorage( const String& rName, StreamMode m, BOOL bDirect )
{
    if( !Validate() || !ValidateMode( m ) )
        return new Storage( pIo, NULL, m );
    BOOL bSetAutoCommit = FALSE;
    if( bDirect && !pEntry->bDirect )
    {
        bSetAutoCommit = TRUE;
        bDirect = FALSE;
    }

    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    if( !p )
    {
        if( !( m & STREAM_NOCREATE ) )
        {
            BOOL bTemp = FALSE;
            // create a new storage
            String aNewName = rName;
            if( !aNewName.Len() )
            {
                aNewName.AssignAscii( "Temp Stg " );
                aNewName.Append( String::CreateFromInt32( ++nTmpCount ) );
                bTemp = TRUE;
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
    // Dont check direct conflict if opening readonly
    if( p && (m & STREAM_WRITE ))
    {
        // Either direct or transacted mode is supported
        if( pEntry->nRefCnt == 1 )
            p->bDirect = bDirect;
        if( p->bDirect != bDirect )
            SetError( SVSTREAM_ACCESS_DENIED );
    }
    Storage* pStg = new Storage( pIo, p, m );
    pIo->MoveError( *pStg );
    if( m & STREAM_WRITE ) pStg->bAutoCommit = TRUE;
    return pStg;
}

// Open a stream

BaseStorageStream* Storage::OpenStream( const String& rName, StreamMode m, BOOL )
{
    if( !Validate() || !ValidateMode( m ) )
        return new StorageStream( pIo, NULL, m );
    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    BOOL bTemp = FALSE;
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
                bTemp = TRUE;
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
        pStm->SetAutoCommit( TRUE );
    pIo->MoveError( *pStm );
    return pStm;
}

// Delete a stream or substorage by setting the temp bit.

BOOL Storage::Remove( const String& rName )
{
    if( !Validate( TRUE ) )
        return FALSE;
    StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
    if( p )
    {
        p->Invalidate( TRUE );
        return TRUE;
    }
    else
    {
        SetError( SVSTREAM_FILE_NOT_FOUND );
        return FALSE;
    }
}

// Rename a storage element

BOOL Storage::Rename( const String& rOld, const String& rNew )
{
    if( Validate( TRUE ) )
    {
        BOOL b = pIo->pTOC->Rename( *pEntry, rOld, rNew );
        pIo->MoveError( *this );
        return b;
    }
    else
        return FALSE;
}

// Copy one element

BOOL Storage::CopyTo( const String& rElem, BaseStorage* pDest, const String& rNew )
{
    if( !Validate() || !pDest || !pDest->Validate( TRUE ) )
        return FALSE;
    StgDirEntry* pElem = pIo->pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        /*
        this lines are misterious !!! MM
        if( !pElem->IsContained( pDest->pEntry ) )
        {
            SetError( SVSTREAM_ACCESS_DENIED );
            return FALSE;
        }
        */
        if( pElem->aEntry.GetType() == STG_STORAGE )
        {
            // copy the entire storage
            BaseStorage* p1 = OpenStorage( rElem, INTERNAL_MODE );
            BaseStorage* p2 = pDest->OpenOLEStorage( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pEntry->bDirect );
            p2->SetClassId( p1->GetClassId() );
            p1->CopyTo( p2 );
            SetError( p1->GetError() );
            if( p2->GetError() )
                pDest->SetError( p2->GetError() );
            else
                p2->Commit();
            delete p1;
            delete p2;
            return BOOL( Good() && pDest->Good() );
        }
        else
        {
            // stream copy
            BaseStorageStream* p1 = OpenStream( rElem, INTERNAL_MODE );
            BaseStorageStream* p2 = pDest->OpenStream( rNew, STREAM_WRITE | STREAM_SHARE_DENYALL, pEntry->bDirect );
            p1->CopyTo( p2 );
            SetError( p1->GetError() );
            if( p2->GetError() )
                pDest->SetError( p2->GetError() );
            else
                p2->Commit();
            delete p1;
            delete p2;
            return BOOL( Good() && pDest->Good() );
        }
    }
    SetError( SVSTREAM_FILE_NOT_FOUND );
    return FALSE;
}

BOOL Storage::CopyTo( BaseStorage* pDest ) const
{
    if( !Validate() || !pDest || !pDest->Validate( TRUE ) || Equals( *pDest ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return FALSE;
    }
    Storage* pThis = (Storage*) this;
    /*
    if( !pThis->pEntry->IsContained( pDest->pEntry ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return FALSE;
    }
    */
    pDest->SetClassId( GetClassId() );
    pDest->SetDirty();
    SvStorageInfoList aList;
    FillInfoList( &aList );
    BOOL bRes = TRUE;
    for( USHORT i = 0; i < aList.Count() && bRes; i++ )
    {
        SvStorageInfo& rInfo = aList.GetObject( i );
        bRes = pThis->CopyTo( rInfo.GetName(), pDest, rInfo.GetName() );
    }
    if( !bRes )
        SetError( pDest->GetError() );
    return BOOL( Good() && pDest->Good() );
}

// Move one element

BOOL Storage::MoveTo( const String& rElem, BaseStorage* pDest, const String& rNew )
{
    if( !Validate() || !pDest || !pDest->Validate( TRUE ) || Equals( *pDest ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return FALSE;
    }

    StgDirEntry* pElem = pIo->pTOC->Find( *pEntry, rElem );
    if( pElem )
    {
        // Simplest case: both storages share the same file
        BOOL bRes;
        Storage *pOther = PTR_CAST( Storage, pDest );
        if( pOther && pIo == pOther->pIo && rElem == rNew )
        {
            Storage *p = (Storage*) pDest;
            Storage *pDest = p;
            // both storages are conventional storages, use implementation dependent code
            if( !pElem->IsContained( pDest->pEntry ) )
            {
                // cyclic move
                SetError( SVSTREAM_ACCESS_DENIED );
                return FALSE;
            }
            bRes = pIo->pTOC->Move( *pEntry, *pDest->pEntry, rNew );
            if( !bRes )
            {
                pIo->MoveError( *this );
                pDest->pIo->MoveError( *pDest );
                ULONG nErr = GetError();
                if( !nErr )
                    nErr = pDest->GetError();
                SetError( nErr );
                pDest->SetError( nErr );
            }
        }
        else
        {
            bRes = CopyTo( rElem, pDest, rNew );
            if( bRes )
                bRes = Remove( rElem );
        }
        if( !bRes )
            SetError( pIo->GetError() );
        return bRes;
    }
    SetError( SVSTREAM_FILE_NOT_FOUND );
    return FALSE;
}

BOOL Storage::IsStorage( const String& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
        if( p )
            return BOOL( p->aEntry.GetType() == STG_STORAGE );
    }
    return FALSE;
}

BOOL Storage::IsStream( const String& rName ) const
{
    if( Validate() )
    {
        StgDirEntry* p = pIo->pTOC->Find( *pEntry, rName );
        if( p )
            return BOOL( p->aEntry.GetType() == STG_STREAM );
    }
    return FALSE;
}

BOOL Storage::IsContained( const String& rName ) const
{
    if( Validate() )
        return BOOL( pIo->pTOC->Find( *pEntry, rName ) != NULL );
    else
        return FALSE;
}

// Commit all sub-elements within this storage. If this is
// the root, commit the FAT, the TOC and the header as well.

BOOL Storage::Commit()
{
    BOOL bRes = TRUE;
    if( !Validate() )
        return FALSE;
    if( !( nMode & STREAM_WRITE ) )
    {
        SetError( SVSTREAM_ACCESS_DENIED );
        return FALSE;
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

BOOL Storage::Revert()
{
    return TRUE;
}

///////////////////////////// OLE Support ////////////////////////////////

// Set the storage type

void Storage::SetClass( const SvGlobalName & rClass,
                                ULONG nOriginalClipFormat,
                                const String & rUserTypeName )
{
    if( Validate( TRUE ) )
    {
        // set the class name in the root entry
        pEntry->aEntry.SetClassId( (const ClsId&) rClass.GetCLSID() );
        pEntry->SetDirty();
        // then create the streams
        StgCompObjStream aCompObj( *this, TRUE );
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
                                       ULONG nOriginalClipFormat,
                                       const String & rUserTypeName )
{
    if( Validate( TRUE ) )
    {
        SetClass( rConvertClass, nOriginalClipFormat, rUserTypeName );
        // plus the convert flag:
        StgOleStream aOle( *this, TRUE );
        aOle.GetFlags() |= 4;
        if( !aOle.Store() )
            SetError( aOle.GetError() );
    }
}

SvGlobalName Storage::GetClassName()
{
    StgCompObjStream aCompObj( *this, FALSE );
    if( aCompObj.Load() )
        return SvGlobalName( (const CLSID&) aCompObj.GetClsId() );
    pIo->ResetError();
    return SvGlobalName();
}

ULONG Storage::GetFormat()
{
    StgCompObjStream aCompObj( *this, FALSE );
    if( aCompObj.Load() )
        return aCompObj.GetCbFormat();
    pIo->ResetError();
    return 0;
}

String Storage::GetUserName()
{
    StgCompObjStream aCompObj( *this, FALSE );
    if( aCompObj.Load() )
        return aCompObj.GetUserName();
    pIo->ResetError();
    return String();
}

BOOL Storage::ShouldConvert()
{
    StgOleStream aOle( *this, FALSE );
    if( aOle.Load() )
        return BOOL( ( aOle.GetFlags() & 4 ) != 0 );
    else
    {
        pIo->ResetError();
        return FALSE;
    }
}

BOOL Storage::ValidateFAT()
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

BOOL Storage::Validate( BOOL bValidate ) const
{
    BOOL bRet = Validate_Impl( bValidate );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

BOOL Storage::ValidateMode( StreamMode nMode ) const
{
    BOOL bRet = ValidateMode_Impl( nMode );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

BOOL Storage::ValidateMode( StreamMode nMode, StgDirEntry* p ) const
{
    BOOL bRet = ValidateMode_Impl( nMode, p );
    if ( !bRet )
        SetError( SVSTREAM_ACCESS_DENIED );
    return bRet;
}

BOOL Storage::Equals( const BaseStorage& rStorage ) const
{
    const Storage* pOther = PTR_CAST( Storage, &rStorage );
    return pOther && ( pOther->pEntry == pEntry );
}


