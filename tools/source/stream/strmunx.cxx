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

// don't include "precompiled_tools.hxx" because this file is included in strmsys.cxx

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h> // fuer getenv()

#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <vector>

#include <osl/mutex.hxx>
#include <osl/thread.h> // osl_getThreadTextEncoding

// class FileBase
#include <osl/file.hxx>
#include <rtl/instance.hxx>

using namespace osl;

// -----------------------------------------------------------------------

// ----------------
// - InternalLock -
// ----------------

#ifndef BOOTSTRAP
namespace { struct LockMutex : public rtl::Static< osl::Mutex, LockMutex > {}; }
#endif

class InternalStreamLock
{
    sal_Size            m_nStartPos;
    sal_Size            m_nEndPos;
    SvFileStream*   m_pStream;
    struct stat     m_aStat;

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
    ByteString aFileName(m_pStream->GetFileName(), osl_getThreadTextEncoding());
    stat( aFileName.GetBuffer(), &m_aStat );
    LockList::get().push_back( this );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "locked %s", aFileName.GetBuffer() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %ld ... %ld ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

InternalStreamLock::~InternalStreamLock()
{
    for ( InternalStreamLockList::iterator it = LockList::get().begin();
          it < LockList::get().end();
          ++it
    ) {
        if ( this == *it ) {
            LockList::get().erase( it );
            break;
        }
    }
#if OSL_DEBUG_LEVEL > 1
    ByteString aFileName(m_pStream->GetFileName(), osl_getThreadTextEncoding());
    fprintf( stderr, "unlocked %s", aFileName.GetBuffer() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %ld ... %ld ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

sal_Bool InternalStreamLock::LockFile( sal_Size nStart, sal_Size nEnd, SvFileStream* pStream )
{
#ifndef BOOTSTRAP
    osl::MutexGuard aGuard( LockMutex::get() );
#endif
    ByteString aFileName(pStream->GetFileName(), osl_getThreadTextEncoding());
    struct stat aStat;
    if( stat( aFileName.GetBuffer(), &aStat ) )
        return sal_False;

    if( S_ISDIR( aStat.st_mode ) )
        return sal_True;

    InternalStreamLock* pLock = NULL;
    InternalStreamLockList &rLockList = LockList::get();
    for( size_t i = 0; i < rLockList.size(); ++i )
    {
        pLock = rLockList[ i ];
        if( aStat.st_ino == pLock->m_aStat.st_ino )
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
#ifndef BOOTSTRAP
    osl::MutexGuard aGuard( LockMutex::get() );
#endif
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


// --------------
// - StreamData -
// --------------

class StreamData
{
public:
    int     nHandle;

            StreamData() { nHandle = 0; }
};

// -----------------------------------------------------------------------

static sal_uInt32 GetSvError( int nErrno )
{
    static struct { int nErr; sal_uInt32 sv; } errArr[] =
    {
        { 0,            SVSTREAM_OK },
        { EACCES,       SVSTREAM_ACCESS_DENIED },
        { EBADF,        SVSTREAM_INVALID_HANDLE },
#if defined(RS6000) || defined(ALPHA) || defined(NETBSD) || \
    defined(FREEBSD) || defined(MACOSX) || defined(OPENBSD) || \
    defined(__FreeBSD_kernel__) || defined (AIX) || defined(DRAGONFLY)
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
#if !defined(RS6000) && !defined(ALPHA) && !defined(NETBSD) && !defined (FREEBSD) && \
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

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
*************************************************************************/

SvFileStream::SvFileStream()
{
    bIsOpen             = sal_False;
    nLockCounter        = 0;
    bIsWritable         = sal_False;
    pInstanceData       = new StreamData;
    SetBufferSize( 1024 );
}

/*************************************************************************
|*
|*    SvFileStream::~SvFileStream()
|*
*************************************************************************/

SvFileStream::~SvFileStream()
{
    Close();

    InternalStreamLock::UnlockFile( 0, 0, this );

    if (pInstanceData)
        delete pInstanceData;
}

/*************************************************************************
|*
|*    SvFileStream::GetFileHandle()
|*
*************************************************************************/

sal_uInt32 SvFileStream::GetFileHandle() const
{
    return (sal_uInt32)pInstanceData->nHandle;
}

/*************************************************************************
|*
|*    SvFileStream::IsA()
|*
*************************************************************************/

sal_uInt16 SvFileStream::IsA() const
{
    return ID_FILESTREAM;
}

/*************************************************************************
|*
|*    SvFileStream::GetData()
|*
*************************************************************************/

sal_Size SvFileStream::GetData( void* pData, sal_Size nSize )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStream::GetData(): " );
    aTraceStr += ByteString::CreateFromInt64(nSize);
    aTraceStr += " Bytes from ";
    aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
    OSL_TRACE( aTraceStr.GetBuffer() );
#endif

    int nRead = 0;
    if ( IsOpen() )
    {
        nRead = read(pInstanceData->nHandle,pData,(unsigned)nSize);
        if ( nRead == -1 )
            SetError( ::GetSvError( errno ));
    }
    return (sal_Size)nRead;
}

/*************************************************************************
|*
|*    SvFileStream::PutData()
|*
*************************************************************************/

sal_Size SvFileStream::PutData( const void* pData, sal_Size nSize )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStrean::PutData: " );
    aTraceStr += ByteString::CreateFromInt64(nSize);
    aTraceStr += " Bytes to ";
    aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
    OSL_TRACE( aTraceStr.GetBuffer() );
#endif

    int nWrite = 0;
    if ( IsOpen() )
    {
        nWrite = write(pInstanceData->nHandle,pData,(unsigned)nSize);
        if ( nWrite == -1 )
        SetError( ::GetSvError( errno ) );
        else if( !nWrite )
        SetError( SVSTREAM_DISK_FULL );
    }
    return (sal_Size)nWrite;
}

/*************************************************************************
|*
|*    SvFileStream::SeekPos()
|*
*************************************************************************/

sal_Size SvFileStream::SeekPos( sal_Size nPos )
{
    if ( IsOpen() )
    {
        long nNewPos;
        if ( nPos != STREAM_SEEK_TO_END )
            nNewPos = lseek( pInstanceData->nHandle, (long)nPos, SEEK_SET );
        else
            nNewPos = lseek( pInstanceData->nHandle, 0L, SEEK_END );

        if ( nNewPos == -1 )
        {
            SetError( SVSTREAM_SEEK_ERROR );
            return 0L;
        }
        // langsam aber sicherer als return nNewPos
        return lseek(pInstanceData->nHandle,0L,SEEK_CUR);
        // return nNewPos;
    }
    SetError( SVSTREAM_GENERALERROR );
    return 0L;
}


/*************************************************************************
|*
|*    SvFileStream::FlushData()
|*
*************************************************************************/

void SvFileStream::FlushData()
{
// lokal gibt es nicht
}

static char *pFileLockEnvVar = (char*)1;

/*************************************************************************
|*
|*    SvFileStream::LockRange()
|*
*************************************************************************/

sal_Bool SvFileStream::LockRange( sal_Size nByteOffset, sal_Size nBytes )
{
    struct flock aflock;
    aflock.l_start = nByteOffset;
    aflock.l_whence = SEEK_SET;
    aflock.l_len = nBytes;

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
                 ByteString(aFilename, osl_getThreadTextEncoding()).GetBuffer(), nByteOffset, nByteOffset+nBytes );
#endif
        return sal_False;
    }

    // HACK: File-Locking nur via Environmentvariable einschalten
    // um einen Haenger im Zusammenspiel mit einem Linux
    // NFS-2-Server (kein Lockdaemon) zu verhindern.
    // File-Locking ?ber NFS ist generell ein Performancekiller.
    //                      HR, 22.10.1997 fuer SOLARIS
    //                      HR, 18.05.1998 Environmentvariable

    if ( pFileLockEnvVar == (char*)1 )
        pFileLockEnvVar = getenv("STAR_ENABLE_FILE_LOCKING");
    if ( ! pFileLockEnvVar )
        return sal_True;

    aflock.l_type = nLockMode;
    if (fcntl(pInstanceData->nHandle, F_GETLK, &aflock) == -1)
    {
    #if defined SOLARIS
        if (errno == ENOSYS)
            return sal_True;
    #endif
        SetError( ::GetSvError( errno ));
        return sal_False;
    }
    if (aflock.l_type != F_UNLCK)
    {
        SetError(SVSTREAM_LOCKING_VIOLATION);
        return sal_False;
    }

    aflock.l_type = nLockMode;
    if (fcntl(pInstanceData->nHandle, F_SETLK, &aflock) == -1)
    {
        SetError( ::GetSvError( errno ));
        return sal_False;
    }
    return sal_True;
}

/*************************************************************************
|*
|*    SvFileStream::UnlockRange()
|*
*************************************************************************/

sal_Bool SvFileStream::UnlockRange( sal_Size nByteOffset, sal_Size nBytes )
{

    struct flock aflock;
    aflock.l_type = F_UNLCK;
    aflock.l_start = nByteOffset;
    aflock.l_whence = SEEK_SET;
    aflock.l_len = nBytes;

    if ( ! IsOpen() )
        return sal_False;

    InternalStreamLock::UnlockFile( nByteOffset, nByteOffset+nBytes, this );

    if ( ! (eStreamMode &
        (STREAM_SHARE_DENYALL | STREAM_SHARE_DENYREAD | STREAM_SHARE_DENYWRITE)))
        return sal_True;

    // wenn File Locking ausgeschaltet, siehe SvFileStream::LockRange
    if ( ! pFileLockEnvVar )
        return sal_True;

    if (fcntl(pInstanceData->nHandle, F_SETLK, &aflock) != -1)
        return sal_True;

    SetError( ::GetSvError( errno ));
    return sal_False;
}

/*************************************************************************
|*
|*    SvFileStream::LockFile()
|*
*************************************************************************/

sal_Bool SvFileStream::LockFile()
{
  return LockRange( 0UL, 0UL );
}

/*************************************************************************
|*
|*    SvFileStream::UnlockFile()
|*
*************************************************************************/

sal_Bool SvFileStream::UnlockFile()
{
    return UnlockRange( 0UL, 0UL );
}

/*************************************************************************
|*
|*    SvFileStream::Open()
|*
*************************************************************************/

void SvFileStream::Open( const String& rFilename, StreamMode nOpenMode )
{
    int nAccess, nAccessRW;
    int nMode;
    int nHandleTmp;
    struct stat buf;
    sal_Bool bStatValid = sal_False;

    Close();
    errno = 0;
    eStreamMode = nOpenMode;
    eStreamMode &= ~STREAM_TRUNC; // beim ReOpen nicht cutten

//    !!! NoOp: Ansonsten ToAbs() verwendern
//    !!! DirEntry aDirEntry( rFilename );
//    !!! aFilename = aDirEntry.GetFull();
    aFilename = rFilename;
#ifndef BOOTSTRAP
    FSysRedirector::DoRedirect( aFilename );
#endif
    ByteString aLocalFilename(aFilename, osl_getThreadTextEncoding());

#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStream::Open(): " );
    aTraceStr +=  aLocalFilename;
    OSL_TRACE( aTraceStr.GetBuffer() );
#endif

    if ( lstat( aLocalFilename.GetBuffer(), &buf ) == 0 )
      {
        bStatValid = sal_True;
        // SvFileStream soll kein Directory oeffnen
        if( S_ISDIR( buf.st_mode ) )
          {
            SetError( ::GetSvError( EISDIR ) );
            return;
          }
      }


    if ( !( nOpenMode & STREAM_WRITE ) )
        nAccessRW = O_RDONLY;
    else if ( !( nOpenMode & STREAM_READ ) )
        nAccessRW = O_WRONLY;
    else
        nAccessRW = O_RDWR;

    nAccess = 0;
    // Fix (MDA, 18.01.95): Bei RD_ONLY nicht mit O_CREAT oeffnen
    // Wichtig auf Read-Only-Dateisystemen (wie CDROM)
    if ( (!( nOpenMode & STREAM_NOCREATE )) && ( nAccessRW != O_RDONLY ) )
        nAccess |= O_CREAT;
    if ( nOpenMode & STREAM_TRUNC )
        nAccess |= O_TRUNC;

    nMode = S_IREAD | S_IROTH | S_IRGRP;
    if ( nOpenMode & STREAM_WRITE)
    {
      nMode |= (S_IWRITE | S_IWOTH | S_IWGRP);

      if ( nOpenMode & STREAM_COPY_ON_SYMLINK )
          {
          if ( bStatValid  &&  S_ISLNK( buf.st_mode ) < 0 )
            {
              char *pBuf = new char[ 1024+1 ];
              if ( readlink( aLocalFilename.GetBuffer(), pBuf, 1024 ) > 0 )
                {
                  if (  unlink(aLocalFilename.GetBuffer())  == 0 )
                      {
#ifdef DBG_UTIL
                      fprintf( stderr,
                               "Copying file on symbolic link (%s).\n",
                               aLocalFilename.GetBuffer() );
#endif
                      String aTmpString( pBuf, osl_getThreadTextEncoding() );
                      const DirEntry aSourceEntry( aTmpString );
                      const DirEntry aTargetEntry( aFilename );
                      FileCopier aFileCopier( aSourceEntry, aTargetEntry );
                      aFileCopier.Execute();
                    }
                }
              delete [] pBuf;
            }
        }
    }


    nHandleTmp = open(aLocalFilename.GetBuffer(),nAccessRW|nAccess, nMode );

    if ( nHandleTmp == -1 )
    {
        if ( nAccessRW != O_RDONLY )
        {
            // auf Lesen runterschalten
            nAccessRW = O_RDONLY;
            nAccess = 0;
            nMode = S_IREAD | S_IROTH | S_IRGRP;
            nHandleTmp =open( aLocalFilename.GetBuffer(),
                              nAccessRW|nAccess,
                              nMode );
            }
    }
    if ( nHandleTmp != -1 )
    {
        pInstanceData->nHandle = nHandleTmp;
        bIsOpen = sal_True;
        if ( nAccessRW != O_RDONLY )
            bIsWritable = sal_True;

        if ( !LockFile() ) // ganze Datei
        {
            close( nHandleTmp );
            bIsOpen = sal_False;
            bIsWritable = sal_False;
            pInstanceData->nHandle = 0;
        }
    }
    else
        SetError( ::GetSvError( errno ) );
}

/*************************************************************************
|*
|*    SvFileStream::ReOpen()
|*
*************************************************************************/

void SvFileStream::ReOpen()
{
    if ( !bIsOpen && aFilename.Len() )
        Open( aFilename, eStreamMode );
}

/*************************************************************************
|*
|*    SvFileStream::Close()
|*
*************************************************************************/

void SvFileStream::Close()
{
    InternalStreamLock::UnlockFile( 0, 0, this );

  if ( IsOpen() )
    {
#ifdef DBG_UTIL
        ByteString aTraceStr( "SvFileStream::Close(): " );
        aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
        OSL_TRACE( aTraceStr.GetBuffer() );
#endif

        Flush();
        close( pInstanceData->nHandle );
        pInstanceData->nHandle = 0;
    }

    bIsOpen     = sal_False;
    bIsWritable = sal_False;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/*************************************************************************
|*
|*    SvFileStream::ResetError()
|*
|*    Beschreibung      STREAM.SDW; Setzt Filepointer auf Dateianfang
|*
*************************************************************************/

void SvFileStream::ResetError()
{
    SvStream::ClearError();
}


/*************************************************************************
|*
|*    SvFileStream::SetSize()
|*
|*    Beschreibung      STREAM.SDW;
|*
*************************************************************************/

void SvFileStream::SetSize (sal_Size nSize)
{
    if (IsOpen())
    {
        int fd = pInstanceData->nHandle;
        if (::ftruncate (fd, (off_t)nSize) < 0)
        {
            // Save original error.
            sal_uInt32 nErr = ::GetSvError (errno);

            // Check against current size. Fail upon 'shrink'.
            struct stat aStat;
            if (::fstat (fd, &aStat) < 0)
            {
                SetError (nErr);
                return;
            }
            if ((sal::static_int_cast< sal_sSize >(nSize) <= aStat.st_size))
            {
                // Failure upon 'shrink'. Return original error.
                SetError (nErr);
                return;
            }

            // Save current position.
            sal_Size nCurPos = (sal_Size)::lseek (fd, (off_t)0, SEEK_CUR);
            if (nCurPos == (sal_Size)(-1))
            {
                SetError (nErr);
                return;
            }

            // Try 'expand' via 'lseek()' and 'write()'.
            if (::lseek (fd, (off_t)(nSize - 1), SEEK_SET) < 0)
            {
                SetError (nErr);
                return;
            }
            if (::write (fd, (char*)"", (size_t)1) < 0)
            {
                // Failure. Restore saved position.
                if (::lseek (fd, (off_t)nCurPos, SEEK_SET) < 0)
                {
                    // Double failure.
                }

                SetError (nErr);
                return;
            }

            // Success. Restore saved position.
            if (::lseek (fd, (off_t)nCurPos, SEEK_SET) < 0)
            {
                SetError (nErr);
                return;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
