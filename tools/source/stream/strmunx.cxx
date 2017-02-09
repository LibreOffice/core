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

#include <tools/stream.hxx>
#include <vector>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>

// class FileBase
#include <osl/file.hxx>
#include <osl/detail/file.h>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

using namespace osl;

// InternalLock ----------------------------------------------------------------

namespace {

struct LockMutex : public rtl::Static< osl::Mutex, LockMutex > {};

struct InternalStreamLock
{
    sal_uInt64         m_nStartPos;
    sal_uInt64         m_nEndPos;
    SvFileStream*      m_pStream;
    osl::DirectoryItem m_aItem;

    InternalStreamLock( sal_uInt64, sal_uInt64, SvFileStream* );
    ~InternalStreamLock();
};

struct LockList : public rtl::Static< std::vector<InternalStreamLock>, LockList > {};

InternalStreamLock::InternalStreamLock(
    sal_uInt64 const nStart,
    sal_uInt64 const nEnd,
    SvFileStream* pStream ) :
        m_nStartPos( nStart ),
        m_nEndPos( nEnd ),
        m_pStream( pStream )
{
    osl::DirectoryItem::get( m_pStream->GetFileName(), m_aItem );
#if OSL_DEBUG_LEVEL > 1
    OString aFileName(OUStringToOString(m_pStream->GetFileName(),
                                                  osl_getThreadTextEncoding()));
    fprintf( stderr, "locked %s", aFileName.getStr() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %ld ... %ld ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

InternalStreamLock::~InternalStreamLock()
{
#if OSL_DEBUG_LEVEL > 1
    OString aFileName(OUStringToOString(m_pStream->GetFileName(),
                                                  osl_getThreadTextEncoding()));
    fprintf( stderr, "unlocked %s", aFileName.getStr() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %ld ... %ld ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

bool lockFile( sal_uInt64 const nStart, sal_uInt64 const nEnd, SvFileStream* pStream )
{
    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get( pStream->GetFileName(), aItem) != osl::FileBase::E_None )
    {
        SAL_INFO("tools.stream", "Failed to lookup stream for locking");
        return true;
    }

    osl::FileStatus aStatus( osl_FileStatus_Mask_Type );
    if ( aItem.getFileStatus( aStatus ) != osl::FileBase::E_None )
    {
        SAL_INFO("tools.stream", "Failed to stat stream for locking");
        return true;
    }
    if( aStatus.getFileType() == osl::FileStatus::Directory )
        return true;

    osl::MutexGuard aGuard( LockMutex::get() );
    std::vector<InternalStreamLock> &rLockList = LockList::get();
    for( std::vector<InternalStreamLock>::const_iterator i = rLockList.begin();
         i != rLockList.end(); )
    {
        if( aItem.isIdenticalTo( i->m_aItem ) )
        {
            bool bDenyByOptions = false;
            StreamMode nLockMode = i->m_pStream->GetStreamMode();
            StreamMode nNewMode = pStream->GetStreamMode();

            if( nLockMode & StreamMode::SHARE_DENYALL )
                bDenyByOptions = true;
            else if( ( nLockMode & StreamMode::SHARE_DENYWRITE ) &&
                     ( nNewMode & StreamMode::WRITE ) )
                bDenyByOptions = true;
            else if( ( nLockMode &StreamMode::SHARE_DENYREAD ) &&
                     ( nNewMode & StreamMode::READ ) )
                bDenyByOptions = true;

            if( bDenyByOptions )
            {
                if( i->m_nStartPos == 0 && i->m_nEndPos == 0 ) // whole file is already locked
                    return false;
                if( nStart == 0 && nEnd == 0) // cannot lock whole file
                    return false;

                if( ( nStart < i->m_nStartPos && nEnd > i->m_nStartPos ) ||
                    ( nStart < i->m_nEndPos && nEnd > i->m_nEndPos ) )
                    return false;
            }
        }
    }
    rLockList.push_back( InternalStreamLock( nStart, nEnd, pStream ) );
    return true;
}

void unlockFile( sal_uInt64 const nStart, sal_uInt64 const nEnd, SvFileStream const * pStream )
{
    osl::MutexGuard aGuard( LockMutex::get() );
    std::vector<InternalStreamLock> &rLockList = LockList::get();
    for( std::vector<InternalStreamLock>::iterator i = rLockList.begin();
         i != rLockList.end(); )
    {
        if ( i->m_pStream == pStream
             && ( ( nStart == 0 && nEnd == 0 )
                  || ( i->m_nStartPos == nStart && i->m_nEndPos == nEnd ) ) )
        {
            i = rLockList.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

}

// StreamData ------------------------------------------------------------------

class StreamData
{
public:
    oslFileHandle rHandle;

    StreamData() : rHandle( nullptr ) { }
};

static ErrCode GetSvError( int nErrno )
{
    static struct { int nErr; ErrCode sv; } errArr[] =
    {
        { 0,            SVSTREAM_OK },
        { EACCES,       SVSTREAM_ACCESS_DENIED },
        { EBADF,        SVSTREAM_INVALID_HANDLE },
#if defined(NETBSD) || \
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
#if !defined(NETBSD) && !defined (FREEBSD) && \
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

    ErrCode nRetVal = SVSTREAM_GENERALERROR; // default error
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

static ErrCode GetSvError( oslFileError nErrno )
{
    static struct { oslFileError nErr; ErrCode sv; } errArr[] =
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

    ErrCode nRetVal = SVSTREAM_GENERALERROR; // default error
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

SvFileStream::SvFileStream( const OUString& rFileName, StreamMode nOpenMode )
{
    bIsOpen             = false;
    nLockCounter        = 0;
    m_isWritable        = false;
    pInstanceData       = new StreamData;

    SetBufferSize( 1024 );
    // convert URL to SystemPath, if necessary
    OUString aSystemFileName;
    if( FileBase::getSystemPathFromFileURL( rFileName , aSystemFileName )
        != FileBase::E_None )
    {
        aSystemFileName = rFileName;
    }
    Open( aSystemFileName, nOpenMode );
}

SvFileStream::SvFileStream()
{
    bIsOpen             = false;
    nLockCounter        = 0;
    m_isWritable        = false;
    pInstanceData       = new StreamData;
    SetBufferSize( 1024 );
}

SvFileStream::~SvFileStream()
{
    Close();

    unlockFile( 0, 0, this );

    delete pInstanceData;
}

std::size_t SvFileStream::GetData( void* pData, std::size_t nSize )
{
    SAL_INFO("tools", OString::number(static_cast<sal_Int64>(nSize)) << " Bytes from " << aFilename);

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
    return (std::size_t)nRead;
}

std::size_t SvFileStream::PutData( const void* pData, std::size_t nSize )
{
    SAL_INFO("tools", OString::number(static_cast<sal_Int64>(nSize)) << " Bytes to " << aFilename);

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
    return (std::size_t)nWrite;
}

sal_uInt64 SvFileStream::SeekPos(sal_uInt64 const nPos)
{
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(nPos != (sal_uInt64)(sal_uInt32)STREAM_SEEK_TO_END);
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
        if ( nPos != STREAM_SEEK_TO_END )
            return nPos;
        rc = osl_getFilePos( pInstanceData->rHandle, &nNewPos );
        return nNewPos;
    }
    SetError( SVSTREAM_GENERALERROR );
    return 0L;
}

void SvFileStream::FlushData()
{
    // does not exist locally
}

bool SvFileStream::LockRange(sal_uInt64 const nByteOffset, std::size_t nBytes)
{
    int nLockMode = 0;

    if ( ! IsOpen() )
        return false;

    if (m_eStreamMode & StreamMode::SHARE_DENYALL)
    {
        if (m_isWritable)
            nLockMode = F_WRLCK;
        else
            nLockMode = F_RDLCK;
    }

    if (m_eStreamMode & StreamMode::SHARE_DENYREAD)
    {
        if (m_isWritable)
            nLockMode = F_WRLCK;
        else
        {
            SetError(SVSTREAM_LOCKING_VIOLATION);
            return false;
        }
    }

    if (m_eStreamMode & StreamMode::SHARE_DENYWRITE)
    {
        if (m_isWritable)
            nLockMode = F_WRLCK;
        else
            nLockMode = F_RDLCK;
    }

    if (!nLockMode)
        return true;

    if( !lockFile( nByteOffset, nByteOffset+nBytes, this ) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "InternalLock on %s [ %ld ... %ld ] failed\n",
                 OUStringToOString(aFilename, osl_getThreadTextEncoding()).getStr(), nByteOffset, nByteOffset+nBytes );
#endif
        return false;
    }

    return true;
}

bool SvFileStream::UnlockRange(sal_uInt64 const nByteOffset, std::size_t nBytes)
{
    if ( ! IsOpen() )
        return false;

    unlockFile( nByteOffset, nByteOffset+nBytes, this );

    return true;
}

bool SvFileStream::LockFile()
{
  return LockRange( 0UL, 0UL );
}

void SvFileStream::UnlockFile()
{
    UnlockRange( 0UL, 0UL );
}

void SvFileStream::Open( const OUString& rFilename, StreamMode nOpenMode )
{
    sal_uInt32 uFlags;
    oslFileHandle nHandleTmp;

    Close();
    errno = 0;
    m_eStreamMode = nOpenMode;
    m_eStreamMode &= ~StreamMode::TRUNC; // don't truncate on reopen

    aFilename = rFilename;

    SAL_INFO("tools", aFilename);

    OUString aFileURL;
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

    if ( !( nOpenMode & StreamMode::WRITE ) )
        uFlags = osl_File_OpenFlag_Read;
    else if ( !( nOpenMode & StreamMode::READ ) )
        uFlags = osl_File_OpenFlag_Write;
    else
        uFlags = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;

    // Fix (MDA, 18.01.95): Don't open with O_CREAT upon RD_ONLY
    // Important for Read-Only-Filesystems (e.g,  CDROM)
    if ( (!( nOpenMode & StreamMode::NOCREATE )) && ( uFlags != osl_File_OpenFlag_Read ) )
        uFlags |= osl_File_OpenFlag_Create;
    if ( nOpenMode & StreamMode::TRUNC )
        uFlags |= osl_File_OpenFlag_Trunc;

    uFlags |= osl_File_OpenFlag_NoExcl | osl_File_OpenFlag_NoLock;

    if ( nOpenMode & StreamMode::WRITE)
    {
        if ( nOpenMode & StreamMode::COPY_ON_SYMLINK )
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
                             OUStringToOString( aStatus.getLinkTargetURL(),
                                                     RTL_TEXTENCODING_UTF8).getStr(),
                             OUStringToOString( aFileURL,
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
            // Change to read-only
            uFlags &= ~osl_File_OpenFlag_Write;
            rc = osl_openFile( aFileURL.pData, &nHandleTmp, uFlags );
        }
    }
    if ( rc == osl_File_E_None )
    {
        pInstanceData->rHandle = nHandleTmp;
        bIsOpen = true;
        if ( uFlags & osl_File_OpenFlag_Write )
            m_isWritable = true;

        if ( !LockFile() ) // whole file
        {
            rc = osl_closeFile( nHandleTmp );
            bIsOpen = false;
            m_isWritable = false;
            pInstanceData->rHandle = nullptr;
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
        SAL_INFO("tools", aFilename);
        Flush();
        osl_closeFile( pInstanceData->rHandle );
        pInstanceData->rHandle = nullptr;
    }

    bIsOpen     = false;
    m_isWritable = false;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/// set filepointer to beginning of file
void SvFileStream::ResetError()
{
    SvStream::ClearError();
}

void SvFileStream::SetSize (sal_uInt64 const nSize)
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
