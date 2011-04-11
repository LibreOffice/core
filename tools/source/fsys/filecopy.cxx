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
#include "precompiled_tools.hxx"

#if defined WNT
#include <windows.h>
#include <io.h>

#elif defined(OS2)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>

#elif defined UNX
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#endif

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include "comdep.hxx"
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <osl/file.hxx>

using namespace ::osl;

/*************************************************************************
|*
|*    FileCopier::FileCopier()
|*
*************************************************************************/

FileCopier::FileCopier() :

    nBytesTotal ( 0 ),
    nBytesCopied( 0 ),
    nBlockSize  ( 4096 ),
    pImp        ( new FileCopier_Impl )

{
}

// -----------------------------------------------------------------------

FileCopier::FileCopier( const DirEntry& rSource, const DirEntry& rTarget ) :

    aSource     ( rSource ),
    aTarget     ( rTarget ),
    nBytesTotal ( 0 ),
    nBytesCopied( 0 ),
    nBlockSize  ( 4096 ),
    pImp        ( new FileCopier_Impl )

{
}

// -----------------------------------------------------------------------

FileCopier::FileCopier( const FileCopier& rCopier ) :

    aSource         ( rCopier.aSource ),
    aTarget         ( rCopier.aTarget ),
    nBytesTotal     ( 0 ),
    nBytesCopied    ( 0 ),
    aProgressLink   ( rCopier.aProgressLink ),
    nBlockSize      ( 4096 ),
    pImp            ( new FileCopier_Impl )

{
}

/*************************************************************************
|*
|*    FileCopier::~FileCopier()
|*
*************************************************************************/

FileCopier::~FileCopier()
{
    delete pImp;
}

/*************************************************************************
|*
|*    FileCopier::operator =()
|*
*************************************************************************/

FileCopier& FileCopier::operator = ( const FileCopier &rCopier )
{
    aSource = rCopier.aSource;
    aTarget = rCopier.aTarget;
    nBytesTotal = rCopier.nBytesTotal;
    nBytesCopied = rCopier.nBytesCopied;
    nBytesCopied = rCopier.nBytesCopied;
    nBlockSize = rCopier.nBlockSize;
    aProgressLink = rCopier.aProgressLink;
    *pImp = *(rCopier.pImp);
    return *this;
}

/*************************************************************************
|*
|*    FileCopier::Progress()
|*
*************************************************************************/

sal_Bool FileCopier::Progress()
{
    if ( !aProgressLink )
        return sal_True;
    else
    {
        if ( aProgressLink.Call( this ) )
            return sal_True;
        return ( 0 == Error( ERRCODE_ABORT, 0, 0 ) );
    }
}

//---------------------------------------------------------------------------

ErrCode FileCopier::Error( ErrCode eErr, const DirEntry* pSource, const DirEntry* pTarget )
{
    // kein Fehler oder kein ErrorHandler?
    if ( !eErr || !pImp->aErrorLink )
        // => Error beibehalten
        return eErr;

    // sonst gesetzten ErrorHandler fragen
    pImp->pErrSource = pSource;
    pImp->pErrTarget = pTarget;
    pImp->eErr = eErr;
    ErrCode eRet = (ErrCode) pImp->aErrorLink.Call( this );
    pImp->pErrSource = 0;
    pImp->pErrTarget = 0;
    return eRet;
}

//---------------------------------------------------------------------------

const DirEntry* FileCopier::GetErrorSource() const
{
    return pImp->pErrSource;
}

//---------------------------------------------------------------------------

const DirEntry* FileCopier::GetErrorTarget() const
{
    return pImp->pErrTarget;
}

//---------------------------------------------------------------------------

ErrCode FileCopier::GetError() const
{
    return pImp->eErr;
}

//---------------------------------------------------------------------------

void FileCopier::SetErrorHdl( const Link &rLink )
{
    pImp->aErrorLink = rLink;
}

//---------------------------------------------------------------------------

const Link& FileCopier::GetErrorHdl() const
{
    return pImp->aErrorLink ;
}

/*************************************************************************
|*
|*    FileCopier::Execute()
|*
*************************************************************************/

FSysError FileCopier::DoCopy_Impl(
    const DirEntry &rSource, const DirEntry &rTarget )
{
    FSysError eRet = FSYS_ERR_OK;
    ErrCode eWarn = FSYS_ERR_OK;

    // HPFS->FAT?
    FSysPathStyle eSourceStyle = DirEntry::GetPathStyle( rSource.ImpGetTopPtr()->GetName() );
    FSysPathStyle eTargetStyle = DirEntry::GetPathStyle( rTarget.ImpGetTopPtr()->GetName() );
    sal_Bool bMakeShortNames = ( eSourceStyle == FSYS_STYLE_HPFS && eTargetStyle == FSYS_STYLE_FAT );

    // Zieldateiname ggf. kuerzen
    DirEntry aTgt;
    if ( bMakeShortNames )
    {
        aTgt = rTarget.GetPath();
        aTgt.MakeShortName( rTarget.GetName() );
    }
    else
        aTgt = rTarget;

    // kein Move wenn Namen gekuerzt werden muessten
    if ( bMakeShortNames && FSYS_ACTION_MOVE == ( pImp->nActions & FSYS_ACTION_MOVE ) && aTgt != rTarget )
        return ERRCODE_IO_NAMETOOLONG;

    // source is directory?
    FileStat aSourceFileStat( rSource );
    if ( aSourceFileStat.IsKind( FSYS_KIND_DIR ) )
    {
#ifdef OS2
        CHAR szSource[CCHMAXPATHCOMP];
        HOBJECT hSourceObject;

        strcpy(szSource, ByteString(rSource.GetFull(), osl_getThreadTextEncoding()).GetBuffer());
        hSourceObject = WinQueryObject(szSource);

        if ( hSourceObject )
        {
            PSZ  pszSourceName;
            PSZ  pszTargetName;
            CHAR szTarget[CCHMAXPATHCOMP];
            HOBJECT hTargetObject;
            HOBJECT hReturn = NULLHANDLE;

            strcpy(szTarget, ByteString(rTarget.GetFull(), osl_getThreadTextEncoding()).GetBuffer());
            pszTargetName = strrchr(szTarget, '\\');
            pszSourceName = strrchr(szSource, '\\');

            hTargetObject = WinQueryObject(szTarget);

            if ( hTargetObject )
                WinDestroyObject(hTargetObject);

            if ( pszTargetName && pszSourceName )
            {
                *pszTargetName = '\0';
                pszSourceName++;
                pszTargetName++;

                if(strcmp(pszSourceName, pszTargetName) == 0)
                {
                    hTargetObject = WinQueryObject(szTarget);

                    if(pImp->nActions & FSYS_ACTION_MOVE)
                    {
                        hReturn = WinMoveObject(hSourceObject, hTargetObject, 0);
                    }
                    else
                    {
                        hReturn = WinCopyObject(hSourceObject, hTargetObject, 0);
                    }
                    if ( bMakeShortNames && aTarget.Exists() )
                        aTarget.Kill();
                    return hReturn ? FSYS_ERR_OK : FSYS_ERR_UNKNOWN;
                }
            }
        }
#endif
        // recursive copy
        eRet = Error( aTgt.MakeDir() ? FSYS_ERR_OK : FSYS_ERR_UNKNOWN, 0, &aTgt );
        Dir aSourceDir( rSource, FSYS_KIND_DIR|FSYS_KIND_FILE );
        for ( sal_uInt16 n = 0; ERRCODE_TOERROR(eRet) == FSYS_ERR_OK && n < aSourceDir.Count(); ++n )
        {
            const DirEntry &rSubSource = aSourceDir[n];
            DirEntryFlag eFlag = rSubSource.GetFlag();
            if ( eFlag != FSYS_FLAG_CURRENT && eFlag != FSYS_FLAG_PARENT )
            {
                DirEntry aSubTarget( aTgt );
                aSubTarget += rSubSource.GetName();
                eRet = DoCopy_Impl( rSubSource, aSubTarget );
                if ( eRet && !eWarn )
                eWarn = eRet;
            }
        }
    }
    else if ( aSourceFileStat.IsKind(FSYS_KIND_FILE) )
    {
        if ( ( FSYS_ACTION_KEEP_EXISTING == ( pImp->nActions & FSYS_ACTION_KEEP_EXISTING ) ) &&
             aTgt.Exists() )
        {
            // Do not overwrite existing file in target folder.
            return ERRCODE_NONE;
        }

        // copy file
        nBytesCopied = 0;
        nBytesTotal = FileStat( rSource ).GetSize();

        ::rtl::OUString aFileName;
        FileBase::getFileURLFromSystemPath( ::rtl::OUString(rSource.GetFull()), aFileName );
        SvFileStream aSrc( aFileName, STREAM_READ|STREAM_NOCREATE|STREAM_SHARE_DENYNONE );

        if ( !aSrc.GetError() )
        {
#ifdef UNX
            struct stat buf;
            if ( fstat( aSrc.GetFileHandle(), &buf ) == -1 )
                eRet = Error( FSYS_ERR_ACCESSDENIED, 0, &aTgt );
#endif
            ::rtl::OUString aTargetFileName;
            FileBase::getFileURLFromSystemPath( ::rtl::OUString(aTgt.GetFull()), aTargetFileName );

            SvFileStream aTargetStream( aTargetFileName, STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYWRITE );
            if ( !aTargetStream.GetError() )
            {
#ifdef UNX
                if ( fchmod( aTargetStream.GetFileHandle(), buf.st_mode ) == -1 )
                    eRet = Error( FSYS_ERR_ACCESSDENIED, 0, &aTgt );
#endif
                size_t nAllocSize = 0, nSize = 0;
                char *pBuf = 0;
                while ( Progress() && nSize == nAllocSize && eRet == FSYS_ERR_OK )
                {
                    // adjust the block-size
                    if ( nBlockSize > nAllocSize )
                    {
                        delete[] pBuf;
                        nAllocSize = nBlockSize;
                        pBuf = new char[nAllocSize];
                    }

                    // copy one block
                    nSize = aSrc.Read( pBuf, nBlockSize );
                    aTargetStream.Write( pBuf, nSize );
                    if ( aTargetStream.GetError() )
                        eRet = Error( aTargetStream.GetError(), 0, &aTgt );

                    // adjust counters
                    nBytesCopied += nSize;
                    if ( nBytesCopied > nBytesTotal )
                        nBytesTotal = nBytesCopied;
                }
                delete[] pBuf;
            }
            else
                eRet = Error( aTargetStream.GetError(), 0, &aTgt );

            // unvollstaendiges File wieder loeschen
            aTargetStream.Close();

            if ( nBytesCopied != nBytesTotal )
            {
                aTgt.Kill();
            }
        }
        else
            eRet = Error( aSrc.GetError(), &rSource, 0 );
    }
    else if ( aSourceFileStat.IsKind(FSYS_KIND_NONE) )
        eRet = Error( ERRCODE_IO_NOTEXISTS, &rSource, 0 );
    else
        eRet = Error( ERRCODE_IO_NOTSUPPORTED, &rSource, 0 );

#ifdef WNT
    // Set LastWriteTime and Attributes of the target identical with the source

    if ( FSYS_ERR_OK == ERRCODE_TOERROR(eRet) )
    {
        WIN32_FIND_DATA fdSource;
        ByteString aFullSource(aSource.GetFull(), osl_getThreadTextEncoding());
        ByteString aFullTarget(aTgt.GetFull(), osl_getThreadTextEncoding());
        HANDLE  hFind = FindFirstFile( aFullSource.GetBuffer() , &fdSource );
        if ( hFind != INVALID_HANDLE_VALUE )
        {
            FindClose( hFind );

            HANDLE hFile = CreateFile( aFullTarget.GetBuffer(), GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if ( hFile != INVALID_HANDLE_VALUE )
            {
                SetFileTime( hFile, NULL, NULL, &fdSource.ftLastWriteTime );
                CloseHandle( hFile );
            }

            SetFileAttributes( aFullTarget.GetBuffer(), fdSource.dwFileAttributes );
        }
    }
#endif
    // bei Move ggf. das File/Dir loeschen
    if ( FSYS_ERR_OK == ERRCODE_TOERROR(eRet) && ( pImp->nActions  & FSYS_ACTION_MOVE ) )
    {
        ErrCode eKillErr = Error( rSource.Kill() | ERRCODE_WARNING_MASK, &rSource, 0 );
        if ( eKillErr != ERRCODE_WARNING_MASK )
        {
            if ( rSource.Exists() )
                // loeschen ging nicht => dann die Kopie wieder loeschen
                aTgt.Kill( pImp->nActions );
            if ( !eWarn )
                eWarn = eKillErr;
        }
    }

    return !eRet ? eWarn : eRet;
}

// -----------------------------------------------------------------------

FSysError FileCopier::Execute( FSysAction nActions )
{
    return ExecuteExact( nActions );
}

// -----------------------------------------------------------------------

FSysError FileCopier::ExecuteExact( FSysAction nActions, FSysExact eExact )
{
    DirEntry aAbsSource = DirEntry( aSource);
    DirEntry aAbsTarget = DirEntry( aTarget );
    pImp->nActions = nActions;

    // check if both pathes are accessible and source and target are different
    if ( !aAbsTarget.ToAbs() || !aAbsSource.ToAbs() || aAbsTarget == aAbsSource )
        return FSYS_ERR_ACCESSDENIED;

    // check if copy would be endless recursive into itself
    if ( FSYS_ACTION_RECURSIVE == ( nActions & FSYS_ACTION_RECURSIVE ) &&
         aAbsSource.Contains( aAbsTarget ) )
        return ERRCODE_IO_RECURSIVE;

    // target is directory?
    if ( eExact == FSYS_NOTEXACT &&
         FileStat( aAbsTarget ).IsKind(FSYS_KIND_DIR) && FileStat( aAbsSource ).IsKind(FSYS_KIND_FILE) )
        // append name of source
        aAbsTarget += aSource.GetName();

    // recursive copy
    return DoCopy_Impl( aAbsSource, aAbsTarget );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
