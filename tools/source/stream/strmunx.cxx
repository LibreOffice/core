/*************************************************************************
 *
 *  $RCSfile: strmunx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:09 $
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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h> // fuer getenv()

#include <debug.hxx>
#include <fsys.hxx>
#include <stream.hxx>

#include <vos/mutex.hxx>
#include <osl/thread.h> // osl_getThreadTextEncoding

// class FileBase
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
using namespace osl;

// -----------------------------------------------------------------------

// ----------------
// - InternalLock -
// ----------------

class InternalStreamLock;
DECLARE_LIST( InternalStreamLockList, InternalStreamLock* );

class InternalStreamLock
{
    ULONG           m_nStartPos;
    ULONG           m_nEndPos;
    SvFileStream*   m_pStream;
    struct stat     m_aStat;

    static InternalStreamLockList LockList;
#ifndef BOOTSTRAP
    static NAMESPACE_VOS(OMutex) LockMutex;
#endif

    InternalStreamLock( ULONG, ULONG, SvFileStream* );
    ~InternalStreamLock();
public:
    static BOOL LockFile( ULONG nStart, ULONG nEnd, SvFileStream* );
    static void UnlockFile( ULONG nStart, ULONG nEnd, SvFileStream* );
};

InternalStreamLockList InternalStreamLock::LockList;
#ifndef BOOTSTRAP
NAMESPACE_VOS(OMutex) InternalStreamLock::LockMutex;
#endif

InternalStreamLock::InternalStreamLock(
    ULONG nStart,
    ULONG nEnd,
    SvFileStream* pStream ) :
        m_nStartPos( nStart ),
        m_nEndPos( nEnd ),
        m_pStream( pStream )
{
    ByteString aFileName(m_pStream->GetFileName(), osl_getThreadTextEncoding());
    stat( aFileName.GetBuffer(), &m_aStat );
    LockList.Insert( this, LIST_APPEND );
#ifdef DEBUG
    fprintf( stderr, "locked %s", aFileName.GetBuffer() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %d ... %d ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

InternalStreamLock::~InternalStreamLock()
{
    LockList.Remove( this );
#ifdef DEBUG
    ByteString aFileName(m_pStream->GetFileName(), osl_getThreadTextEncoding());
    fprintf( stderr, "unlocked %s", aFileName.GetBuffer() );
    if( m_nStartPos || m_nEndPos )
        fprintf(stderr, " [ %d ... %d ]", m_nStartPos, m_nEndPos );
    fprintf( stderr, "\n" );
#endif
}

BOOL InternalStreamLock::LockFile( ULONG nStart, ULONG nEnd, SvFileStream* pStream )
{
#ifndef BOOTSTRAP
    NAMESPACE_VOS( OGuard ) aGuard( LockMutex );
#endif
    ByteString aFileName(pStream->GetFileName(), osl_getThreadTextEncoding());
    struct stat aStat;
    if( stat( aFileName.GetBuffer(), &aStat ) )
        return FALSE;

    if( S_ISDIR( aStat.st_mode ) )
        return TRUE;

    InternalStreamLock* pLock = NULL;
    for( int i = 0; i < LockList.Count(); i++ )
    {
        pLock = LockList.GetObject( i );
        if( aStat.st_ino == pLock->m_aStat.st_ino )
        {
            BOOL bDenyByOptions = FALSE;
            StreamMode nLockMode = pLock->m_pStream->GetStreamMode();
            StreamMode nNewMode = pStream->GetStreamMode();

            if( nLockMode & STREAM_SHARE_DENYALL )
                bDenyByOptions = TRUE;
            else if( ( nLockMode & STREAM_SHARE_DENYWRITE ) &&
                     ( nNewMode & STREAM_WRITE ) )
                bDenyByOptions = TRUE;
            else if( ( nLockMode & STREAM_SHARE_DENYREAD ) &&
                     ( nNewMode & STREAM_READ ) )
                bDenyByOptions = TRUE;

            if( bDenyByOptions )
            {
                if( pLock->m_nStartPos == 0 && pLock->m_nEndPos == 0 ) // whole file is already locked
                    return FALSE;
                if( nStart == 0 && nEnd == 0) // cannot lock whole file
                    return FALSE;

                if( ( nStart < pLock->m_nStartPos && nEnd > pLock->m_nStartPos ) ||
                    ( nStart < pLock->m_nEndPos && nEnd > pLock->m_nEndPos ) )
                    return FALSE;
            }
        }
    }
    pLock  = new InternalStreamLock( nStart, nEnd, pStream );
    return TRUE;
}

void InternalStreamLock::UnlockFile( ULONG nStart, ULONG nEnd, SvFileStream* pStream )
{
#ifndef BOOTSTRAP
    NAMESPACE_VOS( OGuard ) aGuard( LockMutex );
#endif
    InternalStreamLock* pLock = NULL;
    if( nStart == 0 && nEnd == 0 )
    {
        for( int i = 0; i < LockList.Count(); i++ )
        {
            if( ( pLock = LockList.GetObject( i ) )->m_pStream == pStream )
            {
                delete pLock;
                i--;
            }
        }
        return;
    }
    for( int i = 0; i < LockList.Count(); i++ )
    {
        if( ( pLock = LockList.GetObject( i ) )->m_pStream == pStream &&
            nStart == pLock->m_nStartPos && nEnd == pLock->m_nEndPos )
        {
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

static ULONG GetSvError( int nErrno )
{
    static struct { int nErr; ULONG sv; } errArr[] =
    {
        { 0,            SVSTREAM_OK },
        { EACCES,       SVSTREAM_ACCESS_DENIED },
        { EBADF,        SVSTREAM_INVALID_HANDLE },
#if defined( RS6000 ) || defined( ALPHA ) || defined( HP9000 ) || defined( NETBSD ) || defined( S390 ) || defined(FREEBSD) || defined(MACOSX)
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
#if ! defined( RS6000 ) && ! defined( ALPHA ) && ! defined( NETBSD ) && ! defined (FREEBSD) && ! defined (MACOSX)
        { EMULTIHOP,    SVSTREAM_PATH_NOT_FOUND },
        { ENOLINK,      SVSTREAM_PATH_NOT_FOUND },
#endif
        { ENOTDIR,      SVSTREAM_PATH_NOT_FOUND },
        { ETXTBSY,      SVSTREAM_ACCESS_DENIED  },
        { EEXIST,       SVSTREAM_CANNOT_MAKE    },
        { ENOSPC,       SVSTREAM_DISK_FULL      },
        { (int)0xFFFF,  SVSTREAM_GENERALERROR }
    };

    ULONG nRetVal = SVSTREAM_GENERALERROR;    // Standardfehler
    int i=0;
    do
    {
        if ( errArr[i].nErr == nErrno )
        {
            nRetVal = errArr[i].sv;
            break;
        }
        i++;
    }
    while( errArr[i].nErr != 0xFFFF );
    return nRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvFileStream::SvFileStream( const String& rFileName, StreamMode nOpenMode )
{
    bIsOpen             = FALSE;
    nLockCounter        = 0;
    bIsWritable         = FALSE;
    pInstanceData       = new StreamData;

    SetBufferSize( 1024 );
    // convert URL to SystemPath, if necessary
    ::rtl::OUString aFileName, aNormPath;
    if ( FileBase::getNormalizedPathFromFileURL( rFileName, aNormPath ) == FileBase::E_None )
        FileBase::getSystemPathFromNormalizedPath( aNormPath, aFileName );
    else
        aFileName = rFileName;
    Open( aFileName, nOpenMode );
}

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 22.11.94
|*    Letzte Aenderung  OV 22.11.94
|*
*************************************************************************/

SvFileStream::SvFileStream()
{
    bIsOpen             = FALSE;
    nLockCounter        = 0;
    bIsWritable         = FALSE;
    pInstanceData       = new StreamData;
    SetBufferSize( 1024 );
}

/*************************************************************************
|*
|*    SvFileStream::~SvFileStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 22.11.94
|*    Letzte Aenderung  OV 22.11.94
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
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 22.11.94
|*    Letzte Aenderung  OV 22.11.94
|*
*************************************************************************/

ULONG SvFileStream::GetFileHandle() const
{
    return (ULONG)pInstanceData->nHandle;
}

/*************************************************************************
|*
|*    SvFileStream::IsA()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 14.06.94
|*    Letzte Aenderung  OV 14.06.94
|*
*************************************************************************/

USHORT SvFileStream::IsA() const
{
    return ID_FILESTREAM;
}

/*************************************************************************
|*
|*    SvFileStream::GetData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

ULONG SvFileStream::GetData( void* pData, ULONG nSize )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStream::GetData(): " );
    aTraceStr += nSize;
    aTraceStr += " Bytes from ";
    aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    int nRead = 0;
    if ( IsOpen() )
    {
        nRead= read(pInstanceData->nHandle,pData,(unsigned)nSize);
        if ( nRead == -1 )
            SetError( ::GetSvError( errno ));
    }
    return (ULONG)nRead;
}

/*************************************************************************
|*
|*    SvFileStream::PutData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

ULONG SvFileStream::PutData( const void* pData, ULONG nSize )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStrean::PutData: " );
    aTraceStr += nSize;
    aTraceStr += " Bytes to ";
    aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    int nWrite = 0;
    if ( IsOpen() )
    {
        nWrite= write(pInstanceData->nHandle,pData,(unsigned)nSize);
        if ( nWrite == -1 )
        SetError( ::GetSvError( errno ) );
        else if( !nWrite )
        SetError( SVSTREAM_DISK_FULL );
    }
    return (ULONG)nWrite;
}

/*************************************************************************
|*
|*    SvFileStream::SeekPos()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

ULONG SvFileStream::SeekPos( ULONG nPos )
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
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
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
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::LockRange( ULONG nByteOffset, ULONG nBytes )
{
    struct flock aflock;
    aflock.l_start = nByteOffset;
    aflock.l_whence = SEEK_SET;
    aflock.l_len = nBytes;

    int nLockMode = 0;

    if ( ! IsOpen() )
        return FALSE;

    if ( eStreamMode & STREAM_SHARE_DENYALL )
        if (bIsWritable)
            nLockMode = F_WRLCK;
        else
            nLockMode = F_RDLCK;

    if ( eStreamMode & STREAM_SHARE_DENYREAD )
        if (bIsWritable)
            nLockMode = F_WRLCK;
        else
        {
            SetError(SVSTREAM_LOCKING_VIOLATION);
            return FALSE;
        }

    if ( eStreamMode & STREAM_SHARE_DENYWRITE )
        if (bIsWritable)
            nLockMode = F_WRLCK;
        else
            nLockMode = F_RDLCK;

    if (!nLockMode)
        return TRUE;

    if( ! InternalStreamLock::LockFile( nByteOffset, nByteOffset+nBytes, this ) )
    {
#ifdef DEBUG
        fprintf( stderr, "InternalLock on %s [ %d ... %d ] failed\n",
                 ByteString(aFilename, osl_getThreadTextEncoding()).GetBuffer(), nByteOffset, nByteOffset+nBytes );
#endif
        return FALSE;
    }

    // HACK: File-Locking nur via Environmentvariable einschalten
    // um einen Haenger im Zusammenspiel mit einem Linux
    // NFS-2-Server (kein Lockdaemon) zu verhindern.
    // File-Locking ?ber NFS ist generell ein Performancekiller.
    //                      HR, 22.10.1997 fuer SOLARIS
    //                      CP, 30.11.1997 fuer HPUX
    //                      ER, 18.12.1997 fuer IRIX
    //                      HR, 18.05.1998 Environmentvariable

    if ( pFileLockEnvVar == (char*)1 )
        pFileLockEnvVar = getenv("STAR_ENABLE_FILE_LOCKING");
    if ( ! pFileLockEnvVar )
        return TRUE;

    aflock.l_type = nLockMode;
    if (fcntl(pInstanceData->nHandle, F_GETLK, &aflock) == -1)
    {
    #if ( defined HPUX && defined BAD_UNION )
    #ifdef DBG_UTIL
        fprintf( stderr, "***** FCNTL(lock):errno = %d\n", errno );
    #endif
        if ( errno == EINVAL || errno == ENOSYS )
            return TRUE;
    #endif
    #if defined SINIX
        if (errno == EINVAL)
            return TRUE;
    #endif
    #if defined SOLARIS
        if (errno == ENOSYS)
            return TRUE;
    #endif
        SetError( ::GetSvError( errno ));
        return FALSE;
    }
    if (aflock.l_type != F_UNLCK)
    {
        SetError(SVSTREAM_LOCKING_VIOLATION);
        return FALSE;
    }

    aflock.l_type = nLockMode;
    if (fcntl(pInstanceData->nHandle, F_SETLK, &aflock) == -1)
    {
        SetError( ::GetSvError( errno ));
        return FALSE;
    }
    return TRUE;
}

/*************************************************************************
|*
|*    SvFileStream::UnlockRange()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::UnlockRange( ULONG nByteOffset, ULONG nBytes )
{

    struct flock aflock;
    aflock.l_type = F_UNLCK;
    aflock.l_start = nByteOffset;
    aflock.l_whence = SEEK_SET;
    aflock.l_len = nBytes;

    if ( ! IsOpen() )
        return FALSE;

    InternalStreamLock::UnlockFile( nByteOffset, nByteOffset+nBytes, this );

    if ( ! (eStreamMode &
        (STREAM_SHARE_DENYALL | STREAM_SHARE_DENYREAD | STREAM_SHARE_DENYWRITE)))
        return TRUE;

    // wenn File Locking ausgeschaltet, siehe SvFileStream::LockRange
    if ( ! pFileLockEnvVar )
        return TRUE;

    if (fcntl(pInstanceData->nHandle, F_SETLK, &aflock) != -1)
        return TRUE;

#if ( defined HPUX && defined BAD_UNION )
#ifdef DBG_UTIL
        fprintf( stderr, "***** FCNTL(unlock):errno = %d\n", errno );
#endif
        if ( errno == EINVAL || errno == ENOSYS )
            return TRUE;
#endif
#if ( defined SINIX )
    if (errno == EINVAL)
        return TRUE;
#endif

    SetError( ::GetSvError( errno ));
    return FALSE;
}

/*************************************************************************
|*
|*    SvFileStream::LockFile()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::LockFile()
{
  return LockRange( 0UL, 0UL );
}

/*************************************************************************
|*
|*    SvFileStream::UnlockFile()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::UnlockFile()
{
    return UnlockRange( 0UL, 0UL );
}

/*************************************************************************
|*
|*    SvFileStream::Open()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

void SvFileStream::Open( const String& rFilename, StreamMode nOpenMode )
{
    int nAccess, nAccessRW;
    int nMode;
    int nHandleTmp;
    struct stat buf;
    BOOL bStatValid = FALSE;

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
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    if ( lstat( aLocalFilename.GetBuffer(), &buf ) == 0 )
      {
        bStatValid = TRUE;
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
              delete pBuf;
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
        bIsOpen = TRUE;
        if ( nAccessRW != O_RDONLY )
            bIsWritable = TRUE;

        if ( !LockFile() ) // ganze Datei
        {
            close( nHandleTmp );
            bIsOpen = FALSE;
            bIsWritable = FALSE;
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
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
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
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
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
        DBG_TRACE( aTraceStr.GetBuffer() );
#endif

        Flush();
        close( pInstanceData->nHandle );
        pInstanceData->nHandle = 0;
    }

    bIsOpen     = FALSE;
    bIsWritable = FALSE;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/*************************************************************************
|*
|*    SvFileStream::ResetError()
|*
|*    Beschreibung      STREAM.SDW; Setzt Filepointer auf Dateianfang
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
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
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

void SvFileStream::SetSize (ULONG nSize)
{
  if ( IsOpen() )
    {
    if ( ftruncate ( pInstanceData->nHandle, (off_t) nSize ) == -1 )
      SetError ( ::GetSvError( errno ));
    }
}


