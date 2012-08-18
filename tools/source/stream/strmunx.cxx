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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <vector>

#include <osl/mutex.hxx>
#include <osl/thread.h> // osl_getThreadTextEncoding

// class FileBase
#include <osl/file.hxx>
#include <osl/detail/file.h>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

using namespace osl;

// InternalLock ----------------------------------------------------------------

namespace { struct LockMutex : public rtl::Static< osl::Mutex, LockMutex > {}; }

class InternalStreamLock
{
    sal_Size           m_nStartPos;
    sal_Size           m_nEndPos;
    SvFileStream*      m_pStream;
    osl::DirectoryItem m_aItem;

    InternalStreamLock( sal_Size, sal_Size, SvFileStream* );
    ~InternalStreamLock();
public:
    static sal_Bool LockFile( sal_Size nStart, sal_Size nEnd, SvFileStream* );
    static void UnlockFile( sal_Size nStart, sal_Size nEnd, SvFileStream* );
};

typedef ::std::vector< InternalStreamLock* > InternalStreamLockList;

namespace { struct LockList : public rtl::Static< InternalStreamLockList, LockList > {}; }

InternalStreamLock::InternalStreamLock(
    sal_Size nStart,
    sal_Size nEnd,
    SvFileStream* pStream ) :
        m_nStartPos( nStart ),
        m_nEndPos( nEnd ),
        m_pStream( pStream )
{
    osl::DirectoryItem::get( m_pStream->GetFileName(), m_aItem );
    LockList::get().push_back( this );
#if OSL_DEBUG_LEVEL > 1
    rtl::OString aFileName(rtl::OUStringToOString(m_pStream->GetFileName(),
                                                  osl_getThreadTextEncoding()));
    fprintf( stderr, "locked %s", aFileName.getStr() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %ld ... %ld ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

InternalStreamLock::~InternalStreamLock()
{
    for ( InternalStreamLockList::iterator it = LockList::get().begin();
          it != LockList::get().end();
          ++it
    ) {
        if ( this == *it ) {
            LockList::get().erase( it );
            break;
        }
    }
#if OSL_DEBUG_LEVEL > 1
    rtl::OString aFileName(rtl::OUStringToOString(m_pStream->GetFileName(),
                                                  osl_getThreadTextEncoding()));
    fprintf( stderr, "unlocked %s", aFileName.getStr() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %ld ... %ld ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

sal_Bool InternalStreamLock::LockFile( sal_Size nStart, sal_Size nEnd, SvFileStream* pStream )
{
    osl::MutexGuard aGuard( LockMutex::get() );
    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get( pStream->GetFileName(), aItem) != osl::FileBase::E_None )
    {
        SAL_INFO("tools", "Failed to lookup stream for locking");
        return sal_True;
    }

    osl::FileStatus aStatus( osl_FileStatus_Mask_Type );
    if ( aItem.getFileStatus( aStatus ) != osl::FileBase::E_None )
    {
        SAL_INFO("tools", "Failed to stat stream for locking");
        return sal_True;
    }
    if( aStatus.getFileType() == osl::FileStatus::Directory )
        return sal_True;

    InternalStreamLock* pLock = NULL;
    InternalStreamLockList &rLockList = LockList::get();
    for( size_t i = 0; i < rLockList.size(); ++i )
    {
        pLock = rLockList[ i ];
        if( aItem.isIdenticalTo( pLock->m_aItem ) )
        {
            sal_Bool bDenyByOptions = sal_False;
            StreamMode nLockMode = pLock->m_pStream->GetStreamMode();
            StreamMode nNewMode = pStream->GetStreamMode();

            if( nLockMode & STREAM_SHARE_DENYALL )
                bDenyByOptions = sal_True;
            else if( ( nLockMode & STREAM_SHARE_DENYWRITE ) &&
                     ( nNewMode & STREAM_WRITE ) )
                bDenyByOptions = sal_True;
            else if( ( nLockMode & STREAM_SHARE_DENYREAD ) &&
                     ( nNewMode & STREAM_READ ) )
                bDenyByOptions = sal_True;

            if( bDenyByOptions )
            {
                if( pLock->m_nStartPos == 0 && pLock->m_nEndPos == 0 ) // whole file is already locked
                    return sal_False;
                if( nStart == 0 && nEnd == 0) // cannot lock whole file
                    return sal_False;

                if( ( nStart < pLock->m_nStartPos && nEnd > pLock->m_nStartPos ) ||
                    ( nStart < pLock->m_nEndPos && nEnd > pLock->m_nEndPos ) )
                    return sal_False;
            }
        }
    }
    // hint: new InternalStreamLock() adds the entry to the global list
    pLock  = new InternalStreamLock( nStart, nEnd, pStream );
    return sal_True;
}

void InternalStreamLock::UnlockFile( sal_Size nStart, sal_Size nEnd, SvFileStream* pStream )
{
    osl::MutexGuard aGuard( LockMutex::get() );
    InternalStreamLock* pLock = NULL;
    InternalStreamLockList &rLockList = LockList::get();
    if( nStart == 0 && nEnd == 0 )
    {
        // nStart & nEnd = 0, so delete all locks
        for( size_t i = 0; i < rLockList.size(); ++i )
        {
            if( ( pLock = rLockList[ i ] )->m_pStream == pStream )
            {
                // hint: delete will remove pLock from the global list
                delete pLock;
                i--;
            }
        }
        return;
    }
    for( size_t i = 0; i < rLockList.size(); ++i )
    {
        if (  ( pLock = rLockList[ i ] )->m_pStream == pStream
           && nStart == pLock->m_nStartPos
           && nEnd == pLock->m_nEndPos
        ) {
            // hint: delete will remove pLock from the global list
            delete pLock;
            return;
        }
    }
}

// StreamData ------------------------------------------------------------------

class StreamData
{
public:
    oslFileHandle rHandle;

    StreamData() : rHandle( 0 ) { }
};

static sal_uInt32 GetSvError( int nErrno )
{
    static struct { int nErr; sal_uInt32 sv; } errArr[] =
    {
        { 0,            SVSTREAM_OK },
        { EACCES,       SVSTREAM_ACCESS_DENIED },
        { EBADF,        SVSTREAM_INVALID_HANDLE },
#if defined(RS6000) || defined(NETBSD) || \
    defined(FREEBSD) || defined(MACOSX) || defined(OPENBSD) || \
    defined(__FreeBSD_kernel__) || defined (AIX) || defined(DRAGONFLY) || \
    defined(IOS)
        { EDEADLK,      SVSTREAM_LOCKING_VIOLATION },
#else
        { EDEADLOCK,    SVSTREAM_LOCKING_VIOLATION },
#endif
        { EINVAL,       SVSTREAM_INVALID_PARAMETER },
        { EMFILE,       SVSTREAM_TOO_MANY_OPEN_FILES },
        { ENFILE,       SVSTREAM_TOO_MANY_OPEN_FILES },
        { ENOENT,       SVSTREAM_FILE_NOT_FOUND },
        { EPERM,        SVSTREAM_ACCESS_DENIED },
        { EROFS,        SVSTREAM_ACCESS_DENIED },
        { EAGAIN,       SVSTREAM_LOCKING_VIOLATION },
        { EISDIR,       SVSTREAM_PATH_NOT_FOUND },
        { ELOOP,        SVSTREAM_PATH_NOT_FOUND },
#if !defined(RS6000) && !defined(NETBSD) && !defined (FREEBSD) && \
    !defined(MACOSX) && !defined(OPENBSD) && !defined(__FreeBSD_kernel__) && \
    !defined(DRAGONFLY)
        { EMULTIHOP,    SVSTREAM_PATH_NOT_FOUND },
        { ENOLINK,      SVSTREAM_PATH_NOT_FOUND },
#endif
        { ENOTDIR,      SVSTREAM_PATH_NOT_FOUND },
        { ETXTBSY,      SVSTREAM_ACCESS_DENIED  },
        { EEXIST,       SVSTREAM_CANNOT_MAKE    },
        { ENOSPC,       SVSTREAM_DISK_FULL      },
        { (int)0xFFFF,  SVSTREAM_GENERALERROR }
    };

    sal_uInt32 nRetVal = SVSTREAM_GENERALERROR;    // Standardfehler
    int i=0;
    do
    {
        if ( errArr[i].nErr == nErrno )
        {
            nRetVal = errArr[i].sv;
            break;
        }
        ++i;
    }
    while( errArr[i].nErr != 0xFFFF );
    return nRetVal;
}

static sal_uInt32 GetSvError( oslFileError nErrno )
{
    static struct { oslFileError nErr; sal_uInt32 sv; } errArr[] =
    {
        { osl_File_E_None,        SVSTREAM_OK },
        { osl_File_E_ACCES,       SVSTREAM_ACCESS_DENIED },
        { osl_File_E_BADF,        SVSTREAM_INVALID_HANDLE },
        { osl_File_E_DEADLK,      SVSTREAM_LOCKING_VIOLATION },
        { osl_File_E_INVAL,       SVSTREAM_INVALID_PARAMETER },
        { osl_File_E_MFILE,       SVSTREAM_TOO_MANY_OPEN_FILES },
        { osl_File_E_NFILE,       SVSTREAM_TOO_MANY_OPEN_FILES },
        { osl_File_E_NOENT,       SVSTREAM_FILE_NOT_FOUND },
        { osl_File_E_PERM,        SVSTREAM_ACCESS_DENIED },
        { osl_File_E_ROFS,        SVSTREAM_ACCESS_DENIED },
        { osl_File_E_AGAIN,       SVSTREAM_LOCKING_VIOLATION },
        { osl_File_E_ISDIR,       SVSTREAM_PATH_NOT_FOUND },
        { osl_File_E_LOOP,        SVSTREAM_PATH_NOT_FOUND },
        { osl_File_E_MULTIHOP,    SVSTREAM_PATH_NOT_FOUND },
        { osl_File_E_NOLINK,      SVSTREAM_PATH_NOT_FOUND },
        { osl_File_E_NOTDIR,      SVSTREAM_PATH_NOT_FOUND },
        { osl_File_E_EXIST,       SVSTREAM_CANNOT_MAKE    },
        { osl_File_E_NOSPC,       SVSTREAM_DISK_FULL      },
        { (oslFileError)0xFFFF,   SVSTREAM_GENERALERROR }
    };

    sal_uInt32 nRetVal = SVSTREAM_GENERALERROR;    // Standardfehler
    int i=0;
    do
    {
        if ( errArr[i].nErr == nErrno )
        {
            nRetVal = errArr[i].sv;
            break;
        }
        ++i;
    }
    while( errArr[i].nErr != (oslFileError)0xFFFF );
    return nRetVal;
}

SvFileStream::SvFileStream( const String& rFileName, StreamMode nOpenMode )
{
    bIsOpen             = sal_False;
    nLockCounter        = 0;
    bIsWritable         = sal_False;
    pInstanceData       = new StreamData;

    SetBufferSize( 1024 );
    // convert URL to SystemPath, if necessary
    ::rtl::OUString aSystemFileName;
    if( FileBase::getSystemPathFromFileURL( rFileName , aSystemFileName )
        != FileBase::E_None )
    {
        aSystemFileName = rFileName;
    }
    Open( aSystemFileName, nOpenMode );
}

SvFileStream::SvFileStream()
{
    bIsOpen             = sal_False;
    nLockCounter        = 0;
    bIsWritable         = sal_False;
    pInstanceData       = new StreamData;
    SetBufferSize( 1024 );
}

SvFileStream::~SvFileStream()
{
    Close();

    InternalStreamLock::UnlockFile( 0, 0, this );

    if (pInstanceData)
        delete pInstanceData;
}

sal_uInt32 SvFileStream::GetFileHandle() const
{
    sal_IntPtr handle;
    if (osl_getFileOSHandle(pInstanceData->rHandle, &handle) == osl_File_E_None)
        return (sal_uInt32) handle;
    else
        return (sal_uInt32) -1;
}

sal_uInt16 SvFileStream::IsA() const
{
    return ID_FILESTREAM;
}

sal_Size SvFileStream::GetData( void* pData, sal_Size nSize )
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("SvFileStream::GetData(): "));
    aTraceStr.append(static_cast<sal_Int64>(nSize));
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" Bytes from "));
    aTraceStr.append(rtl::OUStringToOString(aFilename,
        osl_getThreadTextEncoding()));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    sal_uInt64 nRead = 0;
    if ( IsOpen() )
    {
        oslFileError rc = osl_readFile(pInstanceData->rHandle,pData,(sal_uInt64)nSize,&nRead);
        if ( rc != osl_File_E_None )
        {
            SetError( ::GetSvError( rc ));
            return -1;
        }
    }
    return (sal_Size)nRead;
}

sal_Size SvFileStream::PutData( const void* pData, sal_Size nSize )
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("SvFileStream::PutData(): "));
    aTraceStr.append(static_cast<sal_Int64>(nSize));
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" Bytes to "));
    aTraceStr.append(rtl::OUStringToOString(aFilename,
        osl_getThreadTextEncoding()));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    sal_uInt64 nWrite = 0;
    if ( IsOpen() )
    {
        oslFileError rc = osl_writeFile(pInstanceData->rHandle,pData,(sal_uInt64)nSize,&nWrite);
        if ( rc != osl_File_E_None )
        {
            SetError( ::GetSvError( rc ) );
            return -1;
        }
        else if( !nWrite )
            SetError( SVSTREAM_DISK_FULL );
    }
    return (sal_Size)nWrite;
}

sal_Size SvFileStream::SeekPos( sal_Size nPos )
{
    if ( IsOpen() )
    {
        oslFileError rc;
        sal_uInt64 nNewPos;
        if ( nPos != STREAM_SEEK_TO_END )
            rc = osl_setFilePos( pInstanceData->rHandle, osl_Pos_Absolut, nPos );
        else
            rc = osl_setFilePos( pInstanceData->rHandle, osl_Pos_End, 0 );

        if ( rc != osl_File_E_None )
        {
            SetError( SVSTREAM_SEEK_ERROR );
            return 0L;
        }
        rc = osl_getFilePos( pInstanceData->rHandle, &nNewPos );
        return (sal_Size) nNewPos;
    }
    SetError( SVSTREAM_GENERALERROR );
    return 0L;
}

void SvFileStream::FlushData()
{
// lokal gibt es nicht
}

sal_Bool SvFileStream::LockRange( sal_Size nByteOffset, sal_Size nBytes )
{
    int nLockMode = 0;

    if ( ! IsOpen() )
        return sal_False;

    if ( eStreamMode & STREAM_SHARE_DENYALL )
        {
        if (bIsWritable)
            nLockMode = F_WRLCK;
        else
            nLockMode = F_RDLCK;
        }

    if ( eStreamMode & STREAM_SHARE_DENYREAD )
        {
        if (bIsWritable)
            nLockMode = F_WRLCK;
        else
        {
            SetError(SVSTREAM_LOCKING_VIOLATION);
            return sal_False;
        }
        }

    if ( eStreamMode & STREAM_SHARE_DENYWRITE )
        {
        if (bIsWritable)
            nLockMode = F_WRLCK;
        else
            nLockMode = F_RDLCK;
        }

    if (!nLockMode)
        return sal_True;

    if( ! InternalStreamLock::LockFile( nByteOffset, nByteOffset+nBytes, this ) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "InternalLock on %s [ %ld ... %ld ] failed\n",
                 rtl::OUStringToOString(aFilename, osl_getThreadTextEncoding()).getStr(), nByteOffset, nByteOffset+nBytes );
#endif
        return sal_False;
    }

    return sal_True;
}

sal_Bool SvFileStream::UnlockRange( sal_Size nByteOffset, sal_Size nBytes )
{
    if ( ! IsOpen() )
        return sal_False;

    InternalStreamLock::UnlockFile( nByteOffset, nByteOffset+nBytes, this );

    return sal_True;
}

sal_Bool SvFileStream::LockFile()
{
  return LockRange( 0UL, 0UL );
}

sal_Bool SvFileStream::UnlockFile()
{
    return UnlockRange( 0UL, 0UL );
}

void SvFileStream::Open( const String& rFilename, StreamMode nOpenMode )
{
    sal_uInt32 uFlags;
    oslFileHandle nHandleTmp;

    Close();
    errno = 0;
    eStreamMode = nOpenMode;
    eStreamMode &= ~STREAM_TRUNC; // beim ReOpen nicht cutten

//    !!! NoOp: Ansonsten ToAbs() verwendern
//    !!! DirEntry aDirEntry( rFilename );
//    !!! aFilename = aDirEntry.GetFull();
    aFilename = rFilename;
    rtl::OString aLocalFilename(rtl::OUStringToOString(aFilename, osl_getThreadTextEncoding()));

#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(RTL_CONSTASCII_STRINGPARAM(
        "SvFileStream::Open(): "));
    aTraceStr.append(aLocalFilename);
    OSL_TRACE( "%s", aTraceStr.getStr() );
#endif

    rtl::OUString aFileURL;
    osl::DirectoryItem aItem;
    osl::FileStatus aStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_LinkTargetURL );

    // FIXME: we really need to switch to a pure URL model ...
    if ( osl::File::getFileURLFromSystemPath( aFilename, aFileURL ) != osl::FileBase::E_None )
        aFileURL = aFilename;
    bool bStatValid = ( osl::DirectoryItem::get( aFileURL, aItem) == osl::FileBase::E_None &&
                        aItem.getFileStatus( aStatus ) == osl::FileBase::E_None );

    // SvFileStream can't open a directory
    if( bStatValid && aStatus.getFileType() == osl::FileStatus::Directory )
    {
        SetError( ::GetSvError( EISDIR ) );
        return;
    }

    if ( !( nOpenMode & STREAM_WRITE ) )
        uFlags = osl_File_OpenFlag_Read;
    else if ( !( nOpenMode & STREAM_READ ) )
        uFlags = osl_File_OpenFlag_Write;
    else
        uFlags = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;

    // Fix (MDA, 18.01.95): Bei RD_ONLY nicht mit O_CREAT oeffnen
    // Wichtig auf Read-Only-Dateisystemen (wie CDROM)
    if ( (!( nOpenMode & STREAM_NOCREATE )) && ( uFlags != osl_File_OpenFlag_Read ) )
        uFlags |= osl_File_OpenFlag_Create;
    if ( nOpenMode & STREAM_TRUNC )
        uFlags |= osl_File_OpenFlag_Trunc;

    uFlags |= osl_File_OpenFlag_NoExcl | osl_File_OpenFlag_NoLock;

    if ( nOpenMode & STREAM_WRITE)
    {
        if ( nOpenMode & STREAM_COPY_ON_SYMLINK )
        {
            if ( bStatValid && aStatus.getFileType() == osl::FileStatus::Link &&
                 aStatus.getLinkTargetURL().getLength() > 0 )
            {
                // delete the symbolic link, and replace it with the contents of the link
                if (osl::File::remove( aFileURL ) == osl::FileBase::E_None )
                {
                    File::copy( aStatus.getLinkTargetURL(), aFileURL );
#if OSL_DEBUG_LEVEL > 0
                    fprintf( stderr,
                             "Removing link and replacing with file contents (%s) -> (%s).\n",
                             rtl::OUStringToOString( aStatus.getLinkTargetURL(),
                                                     RTL_TEXTENCODING_UTF8).getStr(),
                             rtl::OUStringToOString( aFileURL,
                                                     RTL_TEXTENCODING_UTF8).getStr() );
#endif
                }
            }
        }
    }

    oslFileError rc = osl_openFile( aFileURL.pData, &nHandleTmp, uFlags );
    if ( rc != osl_File_E_None )
    {
        if ( uFlags & osl_File_OpenFlag_Write )
        {
            // auf Lesen runterschalten
            uFlags &= ~osl_File_OpenFlag_Write;
            rc = osl_openFile( aFileURL.pData, &nHandleTmp, uFlags );
        }
    }
    if ( rc == osl_File_E_None )
    {
        pInstanceData->rHandle = nHandleTmp;
        bIsOpen = sal_True;
        if ( uFlags & osl_File_OpenFlag_Write )
            bIsWritable = sal_True;

        if ( !LockFile() ) // ganze Datei
        {
            rc = osl_closeFile( nHandleTmp );
            bIsOpen = sal_False;
            bIsWritable = sal_False;
            pInstanceData->rHandle = 0;
        }
    }
    else
        SetError( ::GetSvError( rc ) );
}

void SvFileStream::Close()
{
    UnlockFile();

    if ( IsOpen() )
    {
#ifdef DBG_UTIL
        rtl::OStringBuffer aTraceStr(
            RTL_CONSTASCII_STRINGPARAM("SvFileStream::Close(): "));
        aTraceStr.append(rtl::OUStringToOString(aFilename,
            osl_getThreadTextEncoding()));
        OSL_TRACE("%s", aTraceStr.getStr());
#endif

        Flush();
        osl_closeFile( pInstanceData->rHandle );
        pInstanceData->rHandle = 0;
    }

    bIsOpen     = sal_False;
    bIsWritable = sal_False;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/// set filepointer to beginning of file
void SvFileStream::ResetError()
{
    SvStream::ClearError();
}

void SvFileStream::SetSize (sal_Size nSize)
{
    if (IsOpen())
    {
        oslFileError rc = osl_setFileSize( pInstanceData->rHandle, nSize );
        if (rc != osl_File_E_None )
        {
            SetError ( ::GetSvError( rc ));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
