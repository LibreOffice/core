/*************************************************************************
 *
 *  $RCSfile: filecopy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:06 $
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

#if defined(WIN) || defined(WNT)
#ifndef _SVWIN_H
#include <io.h>
#include <svwin.h>
#endif

#elif defined(PM2) || defined(DOS)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <share.h>
#include <io.h>

#elif defined UNX
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#elif defined MAC
#include <mac_start.h>
#include <fcntl.h>
#include <mac_end.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef PM2
#include <stdio.h>
#endif

#ifndef _COMDEP_HXX
#include "comdep.hxx"
#endif

#ifndef _FSYS_HXX
#include "fsys.hxx"
#endif

#ifndef _STREAM_HXX
#ifdef UNX
#define private public
#endif
#include "stream.hxx"
#endif

EA_Copier* EA_Copier::_pCopier = NULL;

//========================================================================

EA_Copier::~EA_Copier()
{
}

//------------------------------------------------------------------------
void EA_Copier::Register( EA_Copier* pCopier )
{
    _pCopier = pCopier;
}

//------------------------------------------------------------------------

EA_Copier* EA_Copier::Get()
{
    return _pCopier;
}

/*************************************************************************
|*
|*    FileCopier::FileCopier()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 13.04.94
|*    Letzte Aenderung  MI 13.04.94
|*
*************************************************************************/

FileCopier::FileCopier() :

    nBlockSize  ( 4096 ),
    nBytesTotal ( 0 ),
    nBytesCopied( 0 ),
    pImp        ( new FileCopier_Impl )

{
}

// -----------------------------------------------------------------------

FileCopier::FileCopier( const DirEntry& rSource, const DirEntry& rTarget ) :

    aSource     ( rSource ),
    aTarget     ( rTarget ),
    nBlockSize  ( 4096 ),
    nBytesTotal ( 0 ),
    nBytesCopied( 0 ),
    pImp        ( new FileCopier_Impl )

{
}

// -----------------------------------------------------------------------

FileCopier::FileCopier( const FileCopier& rCopier ) :

    aSource         ( rCopier.aSource ),
    aTarget         ( rCopier.aTarget ),
    nBlockSize      ( 4096 ),
    nBytesTotal     ( 0 ),
    nBytesCopied    ( 0 ),
    aProgressLink   ( rCopier.aProgressLink ),
    pImp            ( new FileCopier_Impl )

{
}

/*************************************************************************
|*
|*    FileCopier::~FileCopier()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 13.04.94
|*    Letzte Aenderung  MI 13.04.94
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 13.04.94
|*    Letzte Aenderung  MI 13.04.94
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 13.04.94
|*    Letzte Aenderung  MI 13.04.94
|*
*************************************************************************/

BOOL FileCopier::Progress()
{
    if ( !aProgressLink )
        return TRUE;
    else
    {
        if ( aProgressLink.Call( this ) )
            return TRUE;
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 13.04.94
|*    Letzte Aenderung  PB 16.06.00
|*
*************************************************************************/

#ifndef MAC

#ifdef OS2
BOOL createLongNameEA   ( const PCSZ pszPath, ULONG ulAttributes, const String& aLongName );
#endif

FSysError FileCopier::DoCopy_Impl( const DirEntry &rSource, const DirEntry &rTarget,
                                                                   BOOL bTop )
{
    FSysError eRet = FSYS_ERR_OK;
    ErrCode eWarn = FSYS_ERR_OK;

    // HPFS->FAT?
    FSysPathStyle eSourceStyle = DirEntry::GetPathStyle( rSource.ImpGetTopPtr()->GetName() );
    FSysPathStyle eTargetStyle = DirEntry::GetPathStyle( rTarget.ImpGetTopPtr()->GetName() );
    BOOL bMakeShortNames = ( eSourceStyle == FSYS_STYLE_HPFS && eTargetStyle == FSYS_STYLE_FAT );

    // Zieldateiname ggf. kuerzen
    DirEntry aTarget;
    if ( bMakeShortNames )
    {
        aTarget = rTarget.GetPath();
        aTarget.MakeShortName( rTarget.GetName() );
    }
    else
        aTarget = rTarget;

    // kein Move wenn Namen gekuerzt werden muessten
    if ( bMakeShortNames && FSYS_ACTION_MOVE == ( pImp->nActions & FSYS_ACTION_MOVE ) && aTarget != rTarget )
        return ERRCODE_IO_NAMETOOLONG;

    // source is directory?
    FileStat aSourceFileStat( rSource );
    if ( aSourceFileStat.IsKind( FSYS_KIND_DIR ) )
    {
#ifdef OS2
        CHAR szSource[CCHMAXPATHCOMP];
        HOBJECT hSourceObject;

        strcpy(szSource, rSource.GetFull().GetStr());
        hSourceObject = WinQueryObject(szSource);

        if ( hSourceObject )
        {
            PSZ  pszSourceName;
            PSZ  pszTargetName;
            CHAR szTarget[CCHMAXPATHCOMP];
            HOBJECT hTargetObject;
            HOBJECT hReturn = NULLHANDLE;

            strcpy(szTarget, rTarget.GetFull().GetStr());
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
        eRet = Error( aTarget.MakeDir() ? FSYS_ERR_OK : FSYS_ERR_UNKNOWN, 0, &aTarget );
        Dir aSourceDir( rSource, FSYS_KIND_DIR|FSYS_KIND_FILE );
        for ( USHORT n = 0; ERRCODE_TOERROR(eRet) == FSYS_ERR_OK && n < aSourceDir.Count(); ++n )
        {
            const DirEntry &rSubSource = aSourceDir[n];
            DirEntryFlag eFlag = rSubSource.GetFlag();
            if ( eFlag != FSYS_FLAG_CURRENT && eFlag != FSYS_FLAG_PARENT )
            {
                DirEntry aSubTarget( aTarget );
                aSubTarget += rSubSource.GetName();
                eRet = DoCopy_Impl( rSubSource, aSubTarget, FALSE );
                if ( eRet && !eWarn )
                eWarn = eRet;
            }
        }
    }
    else if ( aSourceFileStat.IsKind(FSYS_KIND_FILE) )
    {
        if ( ( FSYS_ACTION_KEEP_EXISTING == ( pImp->nActions & FSYS_ACTION_KEEP_EXISTING ) ) &&
             aTarget.Exists() )
        {
            // Do not overwrite existing file in target folder.
            return ERRCODE_NONE;
        }

        // copy file
        nBytesCopied = 0;
        nBytesTotal = FileStat( rSource ).GetSize();

        String aFileName( GUI2FSYS( rSource.GetFull() ).GetBuffer() );
        SvFileStream aSource( aFileName, STREAM_READ|STREAM_NOCREATE|STREAM_SHARE_DENYNONE );

        if ( !aSource.GetError() )
        {
#ifdef UNX
            struct stat buf;
            if ( fstat( aSource.GetFileHandle(), &buf ) == -1 )
                eRet = Error( FSYS_ERR_ACCESSDENIED, 0, &aTarget );
#endif
            SvFileStream aTargetStream( GUI2FSYS( aTarget.GetFull() ),
                                        STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYWRITE );
            if ( !aTargetStream.GetError() )
            {
#ifdef UNX
                if ( fchmod( aTargetStream.GetFileHandle(), buf.st_mode ) == -1 )
                    eRet = Error( FSYS_ERR_ACCESSDENIED, 0, &aTarget );
#endif
                size_t nAllocSize = 0, nSize = 0;
                char *pBuf = 0;
                while ( Progress() && nSize == nAllocSize && eRet == FSYS_ERR_OK )
                {
                    // adjust the block-size
                    if ( nBlockSize > nAllocSize )
                    {
                        delete pBuf;
                        nAllocSize = nBlockSize;
                        pBuf = new char[nAllocSize];
                    }

                    // copy one block
                    nSize = aSource.Read( pBuf, nBlockSize );
                    aTargetStream.Write( pBuf, nSize );
                    if ( aTargetStream.GetError() )
                        eRet = Error( aTargetStream.GetError(), 0, &aTarget );

                    // adjust counters
                    nBytesCopied += nSize;
                    if ( nBytesCopied > nBytesTotal )
                        nBytesTotal = nBytesCopied;
                }
                delete pBuf;
            }
            else
                eRet = Error( aTargetStream.GetError(), 0, &aTarget );

            // ggf. EAs kopieren
            if ( !eRet && EA_Copier::Get() && !EA_Copier::Get()->Copy( aSource, aTargetStream ) )
                eRet = FSYS_ERR_UNKNOWN | ERRCODE_WARNING_MASK;

            // unvollstaendiges File wieder loeschen
            aTargetStream.Close();

            if ( nBytesCopied != nBytesTotal )
            {
                aTarget.Kill();
            }
#ifdef OS2
            else
            {
                // falls die Source-Target-Filenamen nicht gleich sind und der Target-Filename nicht
                // 8.3 aber auf FAT ist, dann mu? das EA .longname korrekt gesetzt werden (TPF)
                // (rSource.GetName()!=rTarget.GetName())

                if ( rTarget.IsLongNameOnFAT() && Folder::IsAvailable() )
                {
                    createLongNameEA( (const char*)aTargetStream.GetFileName(),
                                      FILE_NORMAL, rTarget.GetName() );
                }
            }
#endif
        }
        else
            eRet = Error( aSource.GetError(), &rSource, 0 );
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
        ByteString aFullTarget(aTarget.GetFull(), osl_getThreadTextEncoding());
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
                aTarget.Kill( pImp->nActions );
            if ( !eWarn )
                eWarn = eKillErr;
        }
    }

    return !eRet ? eWarn : eRet;
}

#endif

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
    return DoCopy_Impl( aAbsSource, aAbsTarget, TRUE );
}


