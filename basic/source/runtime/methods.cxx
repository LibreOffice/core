/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <tools/date.hxx>
#include <basic/sbxvar.hxx>
#include <vos/process.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/sound.hxx>
#include <tools/wintypes.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <tools/urlobj.hxx>
#include <osl/time.h>
#include <unotools/charclass.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/wldcrd.hxx>
#include <i18npool/lang.h>
#include <vcl/dibtools.hxx>

#include "runtime.hxx"
#include "sbunoobj.hxx"
#ifdef WNT
#include <tools/prewin.h>
#include "winbase.h"
#include <tools/postwin.h>
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#else
#include <osl/file.hxx>
#endif
#include "errobject.hxx"

#ifdef _USE_UNO
#include <comphelper/processfactory.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

using namespace comphelper;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;
using namespace com::sun::star::frame;

#endif /* _USE_UNO */

//#define _ENABLE_CUR_DIR

#include "stdobj.hxx"
#include <basic/sbstdobj.hxx>
#include "rtlproto.hxx"
#include "basrid.hxx"
#include "image.hxx"
#include "sb.hrc"
#include "iosys.hxx"
#include "ddectrl.hxx"
#include <sbintern.hxx>
#include <basic/vbahelper.hxx>

#include <list>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if defined (WNT) || defined (OS2)
#include <direct.h>   // _getdcwd get current work directory, _chdrive
#endif

#ifdef UNX
#include <errno.h>
#include <unistd.h>
#endif

#ifdef WNT
#include <io.h>
#endif

#include <basic/sbobjmod.hxx>

// from source/classes/sbxmod.cxx
Reference< XModel > getDocumentModel( StarBASIC* );

static void FilterWhiteSpace( String& rStr )
{
    rStr.EraseAllChars( ' ' );
    rStr.EraseAllChars( '\t' );
    rStr.EraseAllChars( '\n' );
    rStr.EraseAllChars( '\r' );
}

static long GetDayDiff( const Date& rDate )
{
    Date aRefDate( 1,1,1900 );
    long nDiffDays;
    if ( aRefDate > rDate )
    {
        nDiffDays = (long)(aRefDate - rDate);
        nDiffDays *= -1;
    }
    else
        nDiffDays = (long)(rDate - aRefDate);
    nDiffDays += 2; // Anpassung VisualBasic: 1.Jan.1900 == 2
    return nDiffDays;
}

static CharClass& GetCharClass( void )
{
    static sal_Bool bNeedsInit = sal_True;
    static ::com::sun::star::lang::Locale aLocale;
    if( bNeedsInit )
    {
        bNeedsInit = sal_False;
        aLocale = Application::GetSettings().GetLocale();
    }
    static CharClass aCharClass( aLocale );
    return aCharClass;
}

static inline sal_Bool isFolder( FileStatus::Type aType )
{
    return ( aType == FileStatus::Directory || aType == FileStatus::Volume );
}


//*** UCB file access ***

// Converts possibly relative paths to absolute paths
// according to the setting done by ChDir/ChDrive
String getFullPath( const String& aRelPath )
{
    ::rtl::OUString aFileURL;

    // #80204 Try first if it already is a valid URL
    INetURLObject aURLObj( aRelPath );
    aFileURL = aURLObj.GetMainURL( INetURLObject::NO_DECODE );

    if( aFileURL.isEmpty() )
    {
        File::getFileURLFromSystemPath( aRelPath, aFileURL );
    }

    return aFileURL;
}

// Sets (virtual) current path for UCB file access
void implChDir( const String& aDir )
{
    (void)aDir;
    // TODO
}

// Sets (virtual) current drive for UCB file access
void implChDrive( const String& aDrive )
{
    (void)aDrive;
    // TODO
}

// Returns (virtual) current path for UCB file access
String implGetCurDir( void )
{
    String aRetStr;

    return aRetStr;
}

// TODO: -> SbiGlobals
static com::sun::star::uno::Reference< XSimpleFileAccess3 > getFileAccess( void )
{
    static com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI;
    if( !xSFI.is() )
    {
        com::sun::star::uno::Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            xSFI = com::sun::star::uno::Reference< XSimpleFileAccess3 >( xSMgr->createInstance
                ( ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
        }
    }
    return xSFI;
}



// Properties und Methoden legen beim Get (bPut = sal_False) den Returnwert
// im Element 0 des Argv ab; beim Put (bPut = sal_True) wird der Wert aus
// Element 0 gespeichert.

// CreateObject( class )

RTLFUNC(CreateObject)
{
    (void)bWrite;

    String aClass( rPar.Get( 1 )->GetString() );
    SbxObjectRef p = SbxBase::CreateObject( aClass );
    if( !p )
        StarBASIC::Error( SbERR_CANNOT_LOAD );
    else
    {
        // Convenience: BASIC als Parent eintragen
        p->SetParent( pBasic );
        rPar.Get( 0 )->PutObject( p );
    }
}

// Error( n )

RTLFUNC(Error)
{
    (void)bWrite;

    if( !pBasic )
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
    else
    {
        String aErrorMsg;
        SbError nErr = 0L;
        sal_Int32 nCode = 0;
        if( rPar.Count() == 1 )
        {
            nErr = StarBASIC::GetErrBasic();
            aErrorMsg = StarBASIC::GetErrorMsg();
        }
        else
        {
            nCode = rPar.Get( 1 )->GetLong();
            if( nCode > 65535L )
                StarBASIC::Error( SbERR_CONVERSION );
            else
                nErr = StarBASIC::GetSfxFromVBError( (sal_uInt16)nCode );
        }

        bool bVBA = SbiRuntime::isVBAEnabled();
        String tmpErrMsg;
        if( bVBA && aErrorMsg.Len() > 0 )
        {
            tmpErrMsg = aErrorMsg;
        }
        else
        {
            pBasic->MakeErrorText( nErr, aErrorMsg );
            tmpErrMsg = pBasic->GetErrorText();
        }
        // If this rtlfunc 'Error'  passed a errcode the same as the active Err Objects's
        // current err then  return the description for the error message if it is set
        // ( complicated isn't it ? )
        if ( bVBA && rPar.Count() > 1 )
        {
            com::sun::star::uno::Reference< ooo::vba::XErrObject > xErrObj( SbxErrObject::getUnoErrObject() );
            if ( xErrObj.is() && xErrObj->getNumber() == nCode && !xErrObj->getDescription().isEmpty() )
                tmpErrMsg = xErrObj->getDescription();
        }
        rPar.Get( 0 )->PutString( tmpErrMsg );
    }
}

// Sinus

RTLFUNC(Sin)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        rPar.Get( 0 )->PutDouble( sin( pArg->GetDouble() ) );
    }
}

// Cosinus

RTLFUNC(Cos)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        rPar.Get( 0 )->PutDouble( cos( pArg->GetDouble() ) );
    }
}

// Atn

RTLFUNC(Atn)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        rPar.Get( 0 )->PutDouble( atan( pArg->GetDouble() ) );
    }
}



RTLFUNC(Abs)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        rPar.Get( 0 )->PutDouble( fabs( pArg->GetDouble() ) );
    }
}


RTLFUNC(Asc)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        String aStr( pArg->GetString() );
        if ( aStr.Len() == 0 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            rPar.Get(0)->PutEmpty();
        }
        else
        {
            sal_Unicode aCh = aStr.GetBuffer()[0];
            rPar.Get(0)->PutLong( aCh );
        }
    }
}

void implChr( SbxArray& rPar, bool bChrW )
{
    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );

        String aStr;
        if( !bChrW && SbiRuntime::isVBAEnabled() )
        {
            sal_Char c = (sal_Char)pArg->GetByte();
            ByteString s( c );
            aStr = String( s, gsl_getSystemTextEncoding() );
        }
        else
        {
            sal_Unicode aCh = (sal_Unicode)pArg->GetUShort();
            aStr = String( aCh );
        }
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Chr)
{
    (void)pBasic;
    (void)bWrite;

    bool bChrW = false;
    implChr( rPar, bChrW );
}

RTLFUNC(ChrW)
{
    (void)pBasic;
    (void)bWrite;

    bool bChrW = true;
    implChr( rPar, bChrW );
}


#ifdef UNX
#define _MAX_PATH 260
#define _PATH_INCR 250
#endif

RTLFUNC(CurDir)
{
    (void)pBasic;
    (void)bWrite;

    // #57064 Obwohl diese Funktion nicht mit DirEntry arbeitet, ist sie von
    // der Anpassung an virtuelle URLs nich betroffen, da bei Nutzung der
    // DirEntry-Funktionalitaet keine Moeglichkeit besteht, das aktuelle so
    // zu ermitteln, dass eine virtuelle URL geliefert werden koennte.

//  rPar.Get(0)->PutEmpty();
#if defined (WNT) || defined (OS2)
    int nCurDir = 0;  // Current dir // JSM
    if ( rPar.Count() == 2 )
    {
        String aDrive = rPar.Get(1)->GetString();
        if ( aDrive.Len() != 1 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }
        else
        {
            nCurDir = (int)aDrive.GetBuffer()[0];
            if ( !isalpha( nCurDir ) )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return;
            }
            else
                nCurDir -= ( 'A' - 1 );
        }
    }
    char* pBuffer = new char[ _MAX_PATH ];
#ifdef OS2
    if( !nCurDir )
        nCurDir = _getdrive();
#endif
    if ( _getdcwd( nCurDir, pBuffer, _MAX_PATH ) != 0 )
        rPar.Get(0)->PutString( String::CreateFromAscii( pBuffer ) );
    else
        StarBASIC::Error( SbERR_NO_DEVICE );
    delete [] pBuffer;

#elif defined( UNX )

    int nSize = _PATH_INCR;
    char* pMem;
    while( sal_True )
      {
        pMem = new char[nSize];
        if( !pMem )
          {
            StarBASIC::Error( SbERR_NO_MEMORY );
            return;
          }
        if( getcwd( pMem, nSize-1 ) != NULL )
          {
            rPar.Get(0)->PutString( String::CreateFromAscii(pMem) );
            delete [] pMem;
            return;
          }
        if( errno != ERANGE )
          {
            StarBASIC::Error( SbERR_INTERNAL_ERROR );
            delete [] pMem;
            return;
          }
        delete [] pMem;
        nSize += _PATH_INCR;
      };

#endif
}

RTLFUNC(ChDir) // JSM
{
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
#ifdef _ENABLE_CUR_DIR
        String aPath = rPar.Get(1)->GetString();
        sal_Bool bError = sal_False;
#ifdef WNT
        // #55997 Laut MI hilft es bei File-URLs einen DirEntry zwischenzuschalten
        // #40996 Harmoniert bei Verwendung der WIN32-Funktion nicht mit getdir
        DirEntry aEntry( aPath );
        ByteString aFullPath( aEntry.GetFull(), gsl_getSystemTextEncoding() );
        if( chdir( aFullPath.GetBuffer()) )
            bError = sal_True;
#else
        if (!DirEntry(aPath).SetCWD())
            bError = sal_True;
#endif
        if( bError )
            StarBASIC::Error( SbERR_PATH_NOT_FOUND );
#endif
        // VBA: track current directory per document type (separately for Writer, Calc, Impress, etc.)
        if( SbiRuntime::isVBAEnabled() )
            ::basic::vba::registerCurrentDirectory( getDocumentModel( pBasic ), rPar.Get(1)->GetString() );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(ChDrive) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
#ifdef _ENABLE_CUR_DIR
        // Keine Laufwerke in Unix
#ifndef UNX
        String aPar1 = rPar.Get(1)->GetString();

#if defined (WNT) || defined (OS2)
        if (aPar1.Len() > 0)
        {
            int nCurDrive = (int)aPar1.GetBuffer()[0]; ;
            if ( !isalpha( nCurDrive ) )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return;
            }
            else
                nCurDrive -= ( 'A' - 1 );
            if (_chdrive(nCurDrive))
                StarBASIC::Error( SbERR_NO_DEVICE );
        }
#endif

#endif
        // #ifndef UNX
#endif
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}


// Implementation of StepRENAME with UCB
void implStepRenameUCB( const String& aSource, const String& aDest )
{
    com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
    if( xSFI.is() )
    {
        try
        {
            String aSourceFullPath = getFullPath( aSource );
            if( !xSFI->exists( aSourceFullPath ) )
            {
                StarBASIC::Error( SbERR_FILE_NOT_FOUND );
                return;
            }

            String aDestFullPath = getFullPath( aDest );
            if( xSFI->exists( aDestFullPath ) )
                StarBASIC::Error( SbERR_FILE_EXISTS );
            else
                xSFI->move( aSourceFullPath, aDestFullPath );
        }
        catch( Exception & )
        {
            StarBASIC::Error( SbERR_FILE_NOT_FOUND );
        }
    }
}

// Implementation of StepRENAME with OSL
void implStepRenameOSL( const String& aSource, const String& aDest )
{
    FileBase::RC nRet = File::move( getFullPathUNC( aSource ), getFullPathUNC( aDest ) );
    if( nRet != FileBase::E_None )
    {
        StarBASIC::Error( SbERR_PATH_NOT_FOUND );
    }
}

RTLFUNC(FileCopy) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 3)
    {
        String aSource = rPar.Get(1)->GetString();
        String aDest = rPar.Get(2)->GetString();
        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    xSFI->copy( getFullPath( aSource ), getFullPath( aDest ) );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( SbERR_PATH_NOT_FOUND );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            DirEntry aSourceDirEntry(aSource);
            if (aSourceDirEntry.Exists())
            {
                if (aSourceDirEntry.CopyTo(DirEntry(aDest),FSYS_ACTION_COPYFILE) != FSYS_ERR_OK)
                    StarBASIC::Error( SbERR_PATH_NOT_FOUND );
            }
            else
                    StarBASIC::Error( SbERR_PATH_NOT_FOUND );
#else
            FileBase::RC nRet = File::copy( getFullPathUNC( aSource ), getFullPathUNC( aDest ) );
            if( nRet != FileBase::E_None )
            {
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
            }
#endif
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(Kill) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        String aFileSpec = rPar.Get(1)->GetString();

        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                String aFullPath = getFullPath( aFileSpec );
                if( !xSFI->exists( aFullPath ) || xSFI->isFolder( aFullPath ) )
                {
                    StarBASIC::Error( SbERR_FILE_NOT_FOUND );
                    return;
                }
                try
                {
                    xSFI->kill( aFullPath );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            if(DirEntry(aFileSpec).Kill() != FSYS_ERR_OK)
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
#else
            File::remove( getFullPathUNC( aFileSpec ) );
#endif
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(MkDir) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        String aPath = rPar.Get(1)->GetString();

        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    xSFI->createFolder( getFullPath( aPath ) );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            if (!DirEntry(aPath).MakeDir())
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
#else
            Directory::create( getFullPathUNC( aPath ) );
#endif
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}


#ifndef _OLD_FILE_IMPL

// In OSL only empty directories can be deleted
// so we have to delete all files recursively
void implRemoveDirRecursive( const String& aDirPath )
{
    DirectoryItem aItem;
    FileBase::RC nRet = DirectoryItem::get( aDirPath, aItem );
    sal_Bool bExists = (nRet == FileBase::E_None);

    FileStatus aFileStatus( FileStatusMask_Type );
    nRet = aItem.getFileStatus( aFileStatus );
    FileStatus::Type aType = aFileStatus.getFileType();
    sal_Bool bFolder = isFolder( aType );

    if( !bExists || !bFolder )
    {
        StarBASIC::Error( SbERR_PATH_NOT_FOUND );
        return;
    }

    Directory aDir( aDirPath );
    nRet = aDir.open();
    if( nRet != FileBase::E_None )
    {
        StarBASIC::Error( SbERR_PATH_NOT_FOUND );
        return;
    }

    for( ;; )
    {
        DirectoryItem aItem2;
        nRet = aDir.getNextItem( aItem2 );
        if( nRet != FileBase::E_None )
            break;

        // Handle flags
        FileStatus aFileStatus2( FileStatusMask_Type | FileStatusMask_FileURL );
        nRet = aItem2.getFileStatus( aFileStatus2 );
        ::rtl::OUString aPath = aFileStatus2.getFileURL();

        // Directory?
        FileStatus::Type aType2 = aFileStatus2.getFileType();
        sal_Bool bFolder2 = isFolder( aType2 );
        if( bFolder2 )
        {
            implRemoveDirRecursive( aPath );
        }
        else
        {
            File::remove( aPath );
        }
    }
    nRet = aDir.close();

    nRet = Directory::remove( aDirPath );
}
#endif


RTLFUNC(RmDir) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        String aPath = rPar.Get(1)->GetString();
        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    if( !xSFI->isFolder( aPath ) )
                    {
                        StarBASIC::Error( SbERR_PATH_NOT_FOUND );
                        return;
                    }
                    SbiInstance* pInst = pINST;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    if( bCompatibility )
                    {
                        Sequence< ::rtl::OUString > aContent = xSFI->getFolderContents( aPath, true );
                        sal_Int32 nCount = aContent.getLength();
                        if( nCount > 0 )
                        {
                            StarBASIC::Error( SbERR_ACCESS_ERROR );
                            return;
                        }
                    }

                    xSFI->kill( getFullPath( aPath ) );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            DirEntry aDirEntry(aPath);
            if (aDirEntry.Kill() != FSYS_ERR_OK)
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
#else
            implRemoveDirRecursive( getFullPathUNC( aPath ) );
#endif
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(SendKeys) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
}

RTLFUNC(Exp)
{
    (void)pBasic;
    (void)bWrite;

    if( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double aDouble = rPar.Get( 1 )->GetDouble();
        aDouble = exp( aDouble );
        checkArithmeticOverflow( aDouble );
        rPar.Get( 0 )->PutDouble( aDouble );
    }
}

RTLFUNC(FileLen)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        String aStr( pArg->GetString() );
        sal_Int32 nLen = 0;
        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    nLen = xSFI->getSize( getFullPath( aStr ) );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            FileStat aStat = DirEntry( aStr );
            nLen = aStat.GetSize();
#else
            DirectoryItem aItem;
            FileBase::RC nRet = DirectoryItem::get( getFullPathUNC( aStr ), aItem );
            FileStatus aFileStatus( FileStatusMask_FileSize );
            nRet = aItem.getFileStatus( aFileStatus );
            nLen = (sal_Int32)aFileStatus.getFileSize();
#endif
        }
        rPar.Get(0)->PutLong( (long)nLen );
    }
}


RTLFUNC(Hex)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        char aBuffer[16];
        SbxVariableRef pArg = rPar.Get( 1 );
        if ( pArg->IsInteger() )
            snprintf( aBuffer, sizeof(aBuffer), "%X", pArg->GetInteger() );
        else
            snprintf( aBuffer, sizeof(aBuffer), "%lX", static_cast<long unsigned int>(pArg->GetLong()) );
        rPar.Get(0)->PutString( String::CreateFromAscii( aBuffer ) );
    }
}

// InStr( [start],string,string,[compare] )

RTLFUNC(InStr)
{
    (void)pBasic;
    (void)bWrite;

    sal_uIntPtr nArgCount = rPar.Count()-1;
    if ( nArgCount < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_uInt16 nStartPos = 1;

        sal_uInt16 nFirstStringPos = 1;
        if ( nArgCount >= 3 )
        {
            sal_Int32 lStartPos = rPar.Get(1)->GetLong();
            if( lStartPos <= 0 || lStartPos > 0xffff )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                lStartPos = 1;
            }
            nStartPos = (sal_uInt16)lStartPos;
            nFirstStringPos++;
        }

        SbiInstance* pInst = pINST;
        int bTextMode;
        bool bCompatibility = ( pInst && pInst->IsCompatibility() );
        if( bCompatibility )
        {
            SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
            bTextMode = pRT ? pRT->GetImageFlag( SBIMG_COMPARETEXT ) : sal_False;
        }
        else
        {
            bTextMode = 1;;
        }
        if ( nArgCount == 4 )
            bTextMode = rPar.Get(4)->GetInteger();

        sal_uInt16 nPos;
        const String& rToken = rPar.Get(nFirstStringPos+1)->GetString();

        // #97545 Always find empty string
        if( !rToken.Len() )
        {
            nPos = nStartPos;
        }
        else
        {
            if( !bTextMode )
            {
                const String& rStr1 = rPar.Get(nFirstStringPos)->GetString();

                nPos = rStr1.Search( rToken, nStartPos-1 );
                if ( nPos == STRING_NOTFOUND )
                    nPos = 0;
                else
                    nPos++;
            }
            else
            {
                String aStr1 = rPar.Get(nFirstStringPos)->GetString();
                String aToken = rToken;

                aStr1.ToUpperAscii();
                aToken.ToUpperAscii();

                nPos = aStr1.Search( aToken, nStartPos-1 );
                if ( nPos == STRING_NOTFOUND )
                    nPos = 0;
                else
                    nPos++;
            }
        }
        rPar.Get(0)->PutLong( nPos );
    }
}


// InstrRev(string1, string2[, start[, compare]])

RTLFUNC(InStrRev)
{
    (void)pBasic;
    (void)bWrite;

    sal_uIntPtr nArgCount = rPar.Count()-1;
    if ( nArgCount < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr1 = rPar.Get(1)->GetString();
        String aToken = rPar.Get(2)->GetString();

        sal_Int32 lStartPos = -1;
        if ( nArgCount >= 3 )
        {
            lStartPos = rPar.Get(3)->GetLong();
            if( (lStartPos <= 0 && lStartPos != -1) || lStartPos > 0xffff )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                lStartPos = -1;
            }
        }

        SbiInstance* pInst = pINST;
        int bTextMode;
        bool bCompatibility = ( pInst && pInst->IsCompatibility() );
        if( bCompatibility )
        {
            SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
            bTextMode = pRT ? pRT->GetImageFlag( SBIMG_COMPARETEXT ) : sal_False;
        }
        else
        {
            bTextMode = 1;;
        }
        if ( nArgCount == 4 )
            bTextMode = rPar.Get(4)->GetInteger();

        sal_uInt16 nStrLen = aStr1.Len();
        sal_uInt16 nStartPos = lStartPos == -1 ? nStrLen : (sal_uInt16)lStartPos;

        sal_uInt16 nPos = 0;
        if( nStartPos <= nStrLen )
        {
            sal_uInt16 nTokenLen = aToken.Len();
            if( !nTokenLen )
            {
                // Always find empty string
                nPos = nStartPos;
            }
            else if( nStrLen > 0 )
            {
                if( !bTextMode )
                {
                    ::rtl::OUString aOUStr1 ( aStr1 );
                    ::rtl::OUString aOUToken( aToken );
                    sal_Int32 nRet = aOUStr1.lastIndexOf( aOUToken, nStartPos );
                    if( nRet == -1 )
                        nPos = 0;
                    else
                        nPos = (sal_uInt16)nRet + 1;
                }
                else
                {
                    aStr1.ToUpperAscii();
                    aToken.ToUpperAscii();

                    ::rtl::OUString aOUStr1 ( aStr1 );
                    ::rtl::OUString aOUToken( aToken );
                    sal_Int32 nRet = aOUStr1.lastIndexOf( aOUToken, nStartPos );

                    if( nRet == -1 )
                        nPos = 0;
                    else
                        nPos = (sal_uInt16)nRet + 1;
                }
            }
        }
        rPar.Get(0)->PutLong( nPos );
    }
}


/*
    Int( 2.8 )  =  2.0
    Int( -2.8 ) = -3.0
    Fix( 2.8 )  =  2.0
    Fix( -2.8 ) = -2.0    <- !!
*/

RTLFUNC(Int)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        double aDouble= pArg->GetDouble();
        /*
            floor( 2.8 ) =  2.0
            floor( -2.8 ) = -3.0
        */
        aDouble = floor( aDouble );
        rPar.Get(0)->PutDouble( aDouble );
    }
}



RTLFUNC(Fix)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        double aDouble = pArg->GetDouble();
        if ( aDouble >= 0.0 )
            aDouble = floor( aDouble );
        else
            aDouble = ceil( aDouble );
        rPar.Get(0)->PutDouble( aDouble );
    }
}


RTLFUNC(LCase)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        CharClass& rCharClass = GetCharClass();
        String aStr( rPar.Get(1)->GetString() );
        rCharClass.toLower( aStr );
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Left)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr( rPar.Get(1)->GetString() );
        sal_Int32 lResultLen = rPar.Get(2)->GetLong();
        if( lResultLen > 0xffff )
        {
            lResultLen = 0xffff;
        }
        else if( lResultLen < 0 )
        {
            lResultLen = 0;
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
        }
        aStr.Erase( (sal_uInt16)lResultLen );
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Log)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double aArg = rPar.Get(1)->GetDouble();
        if ( aArg > 0 )
        {
            double d = log( aArg );
            checkArithmeticOverflow( d );
            rPar.Get( 0 )->PutDouble( d );
        }
        else
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}

RTLFUNC(LTrim)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr( rPar.Get(1)->GetString() );
        aStr.EraseLeadingChars();
        rPar.Get(0)->PutString( aStr );
    }
}


// Mid( String, nStart, nLength )

RTLFUNC(Mid)
{
    (void)pBasic;
    (void)bWrite;

    sal_uIntPtr nArgCount = rPar.Count()-1;
    if ( nArgCount < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // #23178: Funktionalitaet von Mid$ als Anweisung nachbilden, indem
        // als weiterer (4.) Parameter ein Ersetzungsstring aufgenommen wird.
        // Anders als im Original kann in dieser Variante der 3. Parameter
        // nLength nicht weggelassen werden. Ist ueber bWrite schon vorgesehen.
        if( nArgCount == 4 )
            bWrite = sal_True;

        String aArgStr = rPar.Get(1)->GetString();
        sal_uInt16 nStartPos = (sal_uInt16)(rPar.Get(2)->GetLong() );
        if ( nStartPos == 0 )
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
        else
        {
            nStartPos--;
            sal_uInt16 nLen = 0xffff;
            bool bWriteNoLenParam = false;
            if ( nArgCount == 3 || bWrite )
            {
                sal_Int32 n = rPar.Get(3)->GetLong();
                if( bWrite && n == -1 )
                    bWriteNoLenParam = true;
                nLen = (sal_uInt16)n;
            }
            String aResultStr;
            if ( bWrite )
            {
                SbiInstance* pInst = pINST;
                bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                if( bCompatibility )
                {
                    sal_uInt16 nArgLen = aArgStr.Len();
                    if( nStartPos + 1 > nArgLen )
                    {
                        StarBASIC::Error( SbERR_BAD_ARGUMENT );
                        return;
                    }

                    String aReplaceStr = rPar.Get(4)->GetString();
                    sal_uInt16 nReplaceStrLen = aReplaceStr.Len();
                    sal_uInt16 nReplaceLen;
                    if( bWriteNoLenParam )
                    {
                        nReplaceLen = nReplaceStrLen;
                    }
                    else
                    {
                        nReplaceLen = nLen;
                        if( nReplaceLen > nReplaceStrLen )
                            nReplaceLen = nReplaceStrLen;
                    }

                    sal_uInt16 nReplaceEndPos = nStartPos + nReplaceLen;
                    if( nReplaceEndPos > nArgLen )
                        nReplaceLen -= (nReplaceEndPos - nArgLen);

                    aResultStr = aArgStr;
                    sal_uInt16 nErase = nReplaceLen;
                    aResultStr.Erase( nStartPos, nErase );
                    aResultStr.Insert( aReplaceStr, 0, nReplaceLen, nStartPos );
                }
                else
                {
                    aResultStr = aArgStr;
                    aResultStr.Erase( nStartPos, nLen );
                    aResultStr.Insert(rPar.Get(4)->GetString(),0,nLen,nStartPos);
                }

                rPar.Get(1)->PutString( aResultStr );
            }
            else
            {
                aResultStr = aArgStr.Copy( nStartPos, nLen );
                rPar.Get(0)->PutString( aResultStr );
            }
        }
    }
}

RTLFUNC(Oct)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        char aBuffer[16];
        SbxVariableRef pArg = rPar.Get( 1 );
        if ( pArg->IsInteger() )
            snprintf( aBuffer, sizeof(aBuffer), "%o", pArg->GetInteger() );
        else
            snprintf( aBuffer, sizeof(aBuffer), "%lo", static_cast<long unsigned int>(pArg->GetLong()) );
        rPar.Get(0)->PutString( String::CreateFromAscii( aBuffer ) );
    }
}

// Replace(expression, find, replace[, start[, count[, compare]]])

RTLFUNC(Replace)
{
    (void)pBasic;
    (void)bWrite;

    sal_uIntPtr nArgCount = rPar.Count()-1;
    if ( nArgCount < 3 || nArgCount > 6 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aExpStr = rPar.Get(1)->GetString();
        String aFindStr = rPar.Get(2)->GetString();
        String aReplaceStr = rPar.Get(3)->GetString();

        sal_Int32 lStartPos = 1;
        if ( nArgCount >= 4 )
        {
            if( rPar.Get(4)->GetType() != SbxEMPTY )
                lStartPos = rPar.Get(4)->GetLong();
            if( lStartPos < 1  || lStartPos > 0xffff )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                lStartPos = 1;
            }
        }

        sal_Int32 lCount = -1;
        if( nArgCount >=5 )
        {
            if( rPar.Get(5)->GetType() != SbxEMPTY )
                lCount = rPar.Get(5)->GetLong();
            if( lCount < -1 || lCount > 0xffff )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                lCount = -1;
            }
        }

        SbiInstance* pInst = pINST;
        int bTextMode;
        bool bCompatibility = ( pInst && pInst->IsCompatibility() );
        if( bCompatibility )
        {
            SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
            bTextMode = pRT ? pRT->GetImageFlag( SBIMG_COMPARETEXT ) : sal_False;
        }
        else
        {
            bTextMode = 1;
        }
        if ( nArgCount == 6 )
            bTextMode = rPar.Get(6)->GetInteger();

        sal_uInt16 nExpStrLen = aExpStr.Len();
        sal_uInt16 nFindStrLen = aFindStr.Len();
        sal_uInt16 nReplaceStrLen = aReplaceStr.Len();

        if( lStartPos <= nExpStrLen )
        {
            sal_uInt16 nPos = static_cast<sal_uInt16>( lStartPos - 1 );
            sal_uInt16 nCounts = 0;
            while( lCount == -1 || lCount > nCounts )
            {
                String aSrcStr( aExpStr );
                if( bTextMode )
                {
                    aSrcStr.ToUpperAscii();
                    aFindStr.ToUpperAscii();
                }
                nPos = aSrcStr.Search( aFindStr, nPos );
                if( nPos != STRING_NOTFOUND )
                {
                    aExpStr.Replace( nPos, nFindStrLen, aReplaceStr );
                    nPos = nPos + nReplaceStrLen;
                    nCounts++;
                }
                else
                {
                    break;
                }
            }
        }
        rPar.Get(0)->PutString( aExpStr.Copy( static_cast<sal_uInt16>(lStartPos - 1) )  );
    }
}

RTLFUNC(Right)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        const String& rStr = rPar.Get(1)->GetString();
        sal_Int32 lResultLen = rPar.Get(2)->GetLong();
        if( lResultLen > 0xffff )
        {
            lResultLen = 0xffff;
        }
        else if( lResultLen < 0 )
        {
            lResultLen = 0;
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
        }
        sal_uInt16 nResultLen = (sal_uInt16)lResultLen;
        sal_uInt16 nStrLen = rStr.Len();
        if ( nResultLen > nStrLen )
            nResultLen = nStrLen;
        String aResultStr = rStr.Copy( nStrLen-nResultLen );
        rPar.Get(0)->PutString( aResultStr );
    }
}

RTLFUNC(RTL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get( 0 )->PutObject( pBasic->getRTL() );
}

RTLFUNC(RTrim)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr( rPar.Get(1)->GetString() );
        aStr.EraseTrailingChars();
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Sgn)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double aDouble = rPar.Get(1)->GetDouble();
        sal_Int16 nResult = 0;
        if ( aDouble > 0 )
            nResult = 1;
        else if ( aDouble < 0 )
            nResult = -1;
        rPar.Get(0)->PutInteger( nResult );
    }
}

RTLFUNC(Space)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr;
        aStr.Fill( (sal_uInt16)(rPar.Get(1)->GetLong() ));
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Spc)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr;
        aStr.Fill( (sal_uInt16)(rPar.Get(1)->GetLong() ));
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Sqr)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double aDouble = rPar.Get(1)->GetDouble();
        if ( aDouble >= 0 )
            rPar.Get(0)->PutDouble( sqrt( aDouble ));
        else
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}

RTLFUNC(Str)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr;
        SbxVariableRef pArg = rPar.Get( 1 );
        pArg->Format( aStr );

        // Numbers start with a space
        if( pArg->IsNumericRTL() )
        {
            // Kommas durch Punkte ersetzen, damit es symmetrisch zu Val ist!
            aStr.SearchAndReplace( ',', '.' );

            SbiInstance* pInst = pINST;
            bool bCompatibility = ( pInst && pInst->IsCompatibility() );
            if( bCompatibility )
            {
                xub_StrLen nLen = aStr.Len();

                const sal_Unicode* pBuf = aStr.GetBuffer();

                bool bNeg = ( pBuf[0] == '-' );
                sal_uInt16 iZeroSearch = 0;
                if( bNeg )
                    iZeroSearch++;

                sal_uInt16 iNext = iZeroSearch + 1;
                if( pBuf[iZeroSearch] == '0' && nLen > iNext && pBuf[iNext] == '.' )
                {
                    aStr.Erase( iZeroSearch, 1 );
                    pBuf = aStr.GetBuffer();
                }
                if( !bNeg )
                    aStr.Insert( ' ', 0 );
            }
            else
                aStr.Insert( ' ', 0 );
        }
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(StrComp)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        rPar.Get(0)->PutEmpty();
        return;
    }
    const String& rStr1 = rPar.Get(1)->GetString();
    const String& rStr2 = rPar.Get(2)->GetString();

    SbiInstance* pInst = pINST;
    sal_Int16 nTextCompare;
    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
    if( bCompatibility )
    {
        SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
        nTextCompare = pRT ? pRT->GetImageFlag( SBIMG_COMPARETEXT ) : sal_False;
    }
    else
    {
        nTextCompare = sal_True;
    }
    if ( rPar.Count() == 4 )
        nTextCompare = rPar.Get(3)->GetInteger();

    if( !bCompatibility )
        nTextCompare = !nTextCompare;

    StringCompare aResult;
    sal_Int32 nRetValue = 0;
    if( nTextCompare )
    {
        ::utl::TransliterationWrapper* pTransliterationWrapper = GetSbData()->pTransliterationWrapper;
        if( !pTransliterationWrapper )
        {
            com::sun::star::uno::Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
            pTransliterationWrapper = GetSbData()->pTransliterationWrapper =
                new ::utl::TransliterationWrapper( xSMgr,
                    ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE |
                    ::com::sun::star::i18n::TransliterationModules_IGNORE_KANA |
                    ::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH );
        }

        LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
        pTransliterationWrapper->loadModuleIfNeeded( eLangType );
        nRetValue = pTransliterationWrapper->compareString( rStr1, rStr2 );
    }
    else
    {
        aResult = rStr1.CompareTo( rStr2 );
        if ( aResult == COMPARE_LESS )
            nRetValue = -1;
        else if ( aResult == COMPARE_GREATER )
            nRetValue = 1;
    }

    rPar.Get(0)->PutInteger( sal::static_int_cast< sal_Int16 >( nRetValue ) );
}

RTLFUNC(String)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr;
        sal_Unicode aFiller;
        sal_Int32 lCount = rPar.Get(1)->GetLong();
        if( lCount < 0 || lCount > 0xffff )
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
        sal_uInt16 nCount = (sal_uInt16)lCount;
        if( rPar.Get(2)->GetType() == SbxINTEGER )
            aFiller = (sal_Unicode)rPar.Get(2)->GetInteger();
        else
        {
            const String& rStr = rPar.Get(2)->GetString();
            aFiller = rStr.GetBuffer()[0];
        }
        aStr.Fill( nCount, aFiller );
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Tan)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        rPar.Get( 0 )->PutDouble( tan( pArg->GetDouble() ) );
    }
}

RTLFUNC(UCase)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        CharClass& rCharClass = GetCharClass();
        String aStr( rPar.Get(1)->GetString() );
        rCharClass.toUpper( aStr );
        rPar.Get(0)->PutString( aStr );
    }
}


RTLFUNC(Val)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double nResult = 0.0;
        char* pEndPtr;

        String aStr( rPar.Get(1)->GetString() );
// lt. Mikkysoft bei Kommas abbrechen!
//      for( sal_uInt16 n=0; n < aStr.Len(); n++ )
//          if( aStr[n] == ',' ) aStr[n] = '.';

        FilterWhiteSpace( aStr );
        if ( aStr.GetBuffer()[0] == '&' && aStr.Len() > 1 )
        {
            int nRadix = 10;
            char aChar = (char)aStr.GetBuffer()[1];
            if ( aChar == 'h' || aChar == 'H' )
                nRadix = 16;
            else if ( aChar == 'o' || aChar == 'O' )
                nRadix = 8;
            if ( nRadix != 10 )
            {
                ByteString aByteStr( aStr, gsl_getSystemTextEncoding() );
                sal_Int16 nlResult = (sal_Int16)strtol( aByteStr.GetBuffer()+2, &pEndPtr, nRadix);
                nResult = (double)nlResult;
            }
        }
        else
        {
            // #57844 Lokalisierte Funktion benutzen
            nResult = ::rtl::math::stringToDouble( aStr, '.', ',', NULL, NULL );
            checkArithmeticOverflow( nResult );
            // ATL: nResult = strtod( aStr.GetStr(), &pEndPtr );
        }

        rPar.Get(0)->PutDouble( nResult );
    }
}


// Helper functions for date conversion
sal_Int16 implGetDateDay( double aDate )
{
    aDate -= 2.0; // normieren: 1.1.1900 => 0.0
    aDate = floor( aDate );
    Date aRefDate( 1, 1, 1900 );
    aRefDate += (sal_uIntPtr)aDate;

    sal_Int16 nRet = (sal_Int16)( aRefDate.GetDay() );
    return nRet;
}

sal_Int16 implGetDateMonth( double aDate )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long)aDate;
    nDays -= 2; // normieren: 1.1.1900 => 0.0
    aRefDate += nDays;
    sal_Int16 nRet = (sal_Int16)( aRefDate.GetMonth() );
    return nRet;
}

sal_Int16 implGetDateYear( double aDate )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long) aDate;
    nDays -= 2; // normieren: 1.1.1900 => 0.0
    aRefDate += nDays;
    sal_Int16 nRet = (sal_Int16)( aRefDate.GetYear() );
    return nRet;
}

sal_Bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet )
{
    if ( nYear < 30 && SbiRuntime::isVBAEnabled() )
        nYear += 2000;
    else if ( nYear < 100 )
        nYear += 1900;
    Date aCurDate( nDay, nMonth, nYear );
    if ((nYear < 100 || nYear > 9999) )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return sal_False;
    }
    if ( !SbiRuntime::isVBAEnabled() )
    {
        if ( (nMonth < 1 || nMonth > 12 )||
        (nDay < 1 || nDay > 31 ) )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return sal_False;
        }
    }
    else
    {
        // grab the year & month
        aCurDate = Date( 1, (( nMonth % 12 ) > 0 ) ? ( nMonth % 12 ) : 12 + ( nMonth % 12 ), nYear );

        // adjust year based on month value
        // e.g. 2000, 0, xx = 1999, 12, xx ( or December of the previous year )
        //      2000, 13, xx = 2001, 1, xx ( or January of the following year )
        if( ( nMonth < 1 ) || ( nMonth > 12 ) )
        {
            // inacurrate around leap year, don't use days to calculate,
            // just modify the months directory
            sal_Int16 nYearAdj = ( nMonth /12 ); // default to positive months inputed
            if ( nMonth <=0 )
                nYearAdj = ( ( nMonth -12 ) / 12 );
            aCurDate.SetYear( aCurDate.GetYear() + nYearAdj );
        }

        // adjust day value,
        // e.g. 2000, 2, 0 = 2000, 1, 31 or the last day of the previous month
        //      2000, 1, 32 = 2000, 2, 1 or the first day of the following month
        if( ( nDay < 1 ) || ( nDay > aCurDate.GetDaysInMonth() ) )
            aCurDate += nDay - 1;
        else
            aCurDate.SetDay( nDay );
    }

    long nDiffDays = GetDayDiff( aCurDate );
    rdRet = (double)nDiffDays;
    return sal_True;
}

// Function to convert date to ISO 8601 date format
RTLFUNC(CDateToIso)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        double aDate = rPar.Get(1)->GetDate();

        char Buffer[9];
        snprintf( Buffer, sizeof( Buffer ), "%04d%02d%02d",
            implGetDateYear( aDate ),
            implGetDateMonth( aDate ),
            implGetDateDay( aDate ) );
        String aRetStr = String::CreateFromAscii( Buffer );
        rPar.Get(0)->PutString( aRetStr );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

// Function to convert date from ISO 8601 date format
RTLFUNC(CDateFromIso)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        String aStr = rPar.Get(1)->GetString();
        sal_Int16 iMonthStart = aStr.Len() - 4;
        String aYearStr  = aStr.Copy( 0, iMonthStart );
        String aMonthStr = aStr.Copy( iMonthStart, 2 );
        String aDayStr   = aStr.Copy( iMonthStart+2, 2 );

        double dDate;
        if( implDateSerial( (sal_Int16)aYearStr.ToInt32(),
            (sal_Int16)aMonthStr.ToInt32(), (sal_Int16)aDayStr.ToInt32(), dDate ) )
        {
            rPar.Get(0)->PutDate( dDate );
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(DateSerial)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nYear = rPar.Get(1)->GetInteger();
    sal_Int16 nMonth = rPar.Get(2)->GetInteger();
    sal_Int16 nDay = rPar.Get(3)->GetInteger();

    double dDate;
    if( implDateSerial( nYear, nMonth, nDay, dDate ) )
        rPar.Get(0)->PutDate( dDate );
}

RTLFUNC(TimeSerial)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nHour = rPar.Get(1)->GetInteger();
    if ( nHour == 24 )
        nHour = 0;                      // Wegen UNO DateTimes, die bis 24 Uhr gehen
    sal_Int16 nMinute = rPar.Get(2)->GetInteger();
    sal_Int16 nSecond = rPar.Get(3)->GetInteger();
    if ((nHour < 0 || nHour > 23)   ||
        (nMinute < 0 || nMinute > 59 )  ||
        (nSecond < 0 || nSecond > 59 ))
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_Int32 nSeconds = nHour;
    nSeconds *= 3600;
    nSeconds += nMinute * 60;
    nSeconds += nSecond;
    double nDays = ((double)nSeconds) / (double)(86400.0);
    rPar.Get(0)->PutDate( nDays ); // JSM
}

RTLFUNC(DateValue)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // #39629 pINST pruefen, kann aus URL-Zeile gerufen werden
        SvNumberFormatter* pFormatter = NULL;
        if( pINST )
            pFormatter = pINST->GetNumberFormatter();
        else
        {
            sal_uInt32 n;   // Dummy
            SbiInstance::PrepareNumberFormatter( pFormatter, n, n, n );
        }

        sal_uInt32 nIndex;
        double fResult;
        String aStr( rPar.Get(1)->GetString() );
        sal_Bool bSuccess = pFormatter->IsNumberFormat( aStr, nIndex, fResult );
        short nType = pFormatter->GetType( nIndex );

        // DateValue("February 12, 1969") raises error if the system locale is not en_US
        // by using SbiInstance::GetNumberFormatter.
        // It seems that both locale number formatter and English number formatter
        // are supported in Visual Basic.
        LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
        if( !bSuccess && ( eLangType != LANGUAGE_ENGLISH_US ) )
        {
            // Create a new SvNumberFormatter by using LANGUAGE_ENGLISH to get the date value;
            com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                xFactory = comphelper::getProcessServiceFactory();
            SvNumberFormatter aFormatter( xFactory, LANGUAGE_ENGLISH_US );
            bSuccess = aFormatter.IsNumberFormat( aStr, nIndex, fResult );
            nType = aFormatter.GetType( nIndex );
        }

        if(bSuccess && (nType==NUMBERFORMAT_DATE || nType==NUMBERFORMAT_DATETIME))
        {
            if ( nType == NUMBERFORMAT_DATETIME )
            {
                // Zeit abschneiden
                if ( fResult  > 0.0 )
                    fResult = floor( fResult );
                else
                    fResult = ceil( fResult );
            }
            // fResult += 2.0; // Anpassung  StarCalcFormatter
            rPar.Get(0)->PutDate( fResult ); // JSM
        }
        else
            StarBASIC::Error( SbERR_CONVERSION );

        // #39629 pFormatter kann selbst angefordert sein
        if( !pINST )
            delete pFormatter;
    }
}

RTLFUNC(TimeValue)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // #39629 pINST pruefen, kann aus URL-Zeile gerufen werden
        SvNumberFormatter* pFormatter = NULL;
        if( pINST )
            pFormatter = pINST->GetNumberFormatter();
        else
        {
            sal_uInt32 n;   // Dummy
            SbiInstance::PrepareNumberFormatter( pFormatter, n, n, n );
        }

        sal_uInt32 nIndex;
        double fResult;
        sal_Bool bSuccess = pFormatter->IsNumberFormat( rPar.Get(1)->GetString(),
                                                   nIndex, fResult );
        short nType = pFormatter->GetType(nIndex);
        if(bSuccess && (nType==NUMBERFORMAT_TIME||nType==NUMBERFORMAT_DATETIME))
        {
            if ( nType == NUMBERFORMAT_DATETIME )
                // Tage abschneiden
                fResult = fmod( fResult, 1 );
            rPar.Get(0)->PutDate( fResult ); // JSM
        }
        else
            StarBASIC::Error( SbERR_CONVERSION );

        // #39629 pFormatter kann selbst angefordert sein
        if( !pINST )
            delete pFormatter;
    }
}

RTLFUNC(Day)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        double aDate = pArg->GetDate();

        sal_Int16 nDay = implGetDateDay( aDate );
        rPar.Get(0)->PutInteger( nDay );
    }
}

RTLFUNC(Year)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nYear = implGetDateYear( rPar.Get(1)->GetDate() );
        rPar.Get(0)->PutInteger( nYear );
    }
}

sal_Int16 implGetHour( double dDate )
{
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = (sal_Int32)(nFrac + 0.5);
    sal_Int16 nHour = (sal_Int16)(nSeconds / 3600);
    return nHour;
}

RTLFUNC(Hour)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nHour = implGetHour( nArg );
        rPar.Get(0)->PutInteger( nHour );
    }
}

sal_Int16 implGetMinute( double dDate )
{
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = (sal_Int32)(nFrac + 0.5);
    sal_Int16 nTemp = (sal_Int16)(nSeconds % 3600);
    sal_Int16 nMin = nTemp / 60;
    return nMin;
}

RTLFUNC(Minute)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nMin = implGetMinute( nArg );
        rPar.Get(0)->PutInteger( nMin );
    }
}

RTLFUNC(Month)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nMonth = implGetDateMonth( rPar.Get(1)->GetDate() );
        rPar.Get(0)->PutInteger( nMonth );
    }
}

sal_Int16 implGetSecond( double dDate )
{
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = (sal_Int32)(nFrac + 0.5);
    sal_Int16 nTemp = (sal_Int16)(nSeconds / 3600);
    nSeconds -= nTemp * 3600;
    nTemp = (sal_Int16)(nSeconds / 60);
    nSeconds -= nTemp * 60;

    sal_Int16 nRet = (sal_Int16)nSeconds;
    return nRet;
}

RTLFUNC(Second)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nSecond = implGetSecond( nArg );
        rPar.Get(0)->PutInteger( nSecond );
    }
}

double Now_Impl()
{
    Date aDate;
    Time aTime;
    double aSerial = (double)GetDayDiff( aDate );
    long nSeconds = aTime.GetHour();
    nSeconds *= 3600;
    nSeconds += aTime.GetMin() * 60;
    nSeconds += aTime.GetSec();
    double nDays = ((double)nSeconds) / (double)(24.0*3600.0);
    aSerial += nDays;
    return aSerial;
}

// Date Now(void)

RTLFUNC(Now)
{
        (void)pBasic;
        (void)bWrite;
    rPar.Get(0)->PutDate( Now_Impl() );
}

// Date Time(void)

RTLFUNC(Time)
{
    (void)pBasic;

    if ( !bWrite )
    {
        Time aTime;
        SbxVariable* pMeth = rPar.Get( 0 );
        String aRes;
        if( pMeth->IsFixed() )
        {
            // Time$: hh:mm:ss
            char buf[ 20 ];
            snprintf( buf, sizeof(buf), "%02d:%02d:%02d",
                aTime.GetHour(), aTime.GetMin(), aTime.GetSec() );
            aRes = String::CreateFromAscii( buf );
        }
        else
        {
            // Time: system dependent
            long nSeconds=aTime.GetHour();
            nSeconds *= 3600;
            nSeconds += aTime.GetMin() * 60;
            nSeconds += aTime.GetSec();
            double nDays = (double)nSeconds * ( 1.0 / (24.0*3600.0) );
            Color* pCol;

            // #39629 pINST pruefen, kann aus URL-Zeile gerufen werden
            SvNumberFormatter* pFormatter = NULL;
            sal_uInt32 nIndex;
            if( pINST )
            {
                pFormatter = pINST->GetNumberFormatter();
                nIndex = pINST->GetStdTimeIdx();
            }
            else
            {
                sal_uInt32 n;   // Dummy
                SbiInstance::PrepareNumberFormatter( pFormatter, n, nIndex, n );
            }

            pFormatter->GetOutputString( nDays, nIndex, aRes, &pCol );

            // #39629 pFormatter kann selbst angefordert sein
            if( !pINST )
                delete pFormatter;
        }
        pMeth->PutString( aRes );
    }
    else
    {
        StarBASIC::Error( SbERR_NOT_IMPLEMENTED );
    }
}

RTLFUNC(Timer)
{
    (void)pBasic;
    (void)bWrite;

    Time aTime;
    long nSeconds = aTime.GetHour();
    nSeconds *= 3600;
    nSeconds += aTime.GetMin() * 60;
    nSeconds += aTime.GetSec();
    rPar.Get(0)->PutDate( (double)nSeconds );
}


RTLFUNC(Date)
{
    (void)pBasic;
    (void)bWrite;

    if ( !bWrite )
    {
        Date aToday;
        double nDays = (double)GetDayDiff( aToday );
        SbxVariable* pMeth = rPar.Get( 0 );
        if( pMeth->IsString() )
        {
            String aRes;
            Color* pCol;

            // #39629 pINST pruefen, kann aus URL-Zeile gerufen werden
            SvNumberFormatter* pFormatter = NULL;
            sal_uInt32 nIndex;
            if( pINST )
            {
                pFormatter = pINST->GetNumberFormatter();
                nIndex = pINST->GetStdDateIdx();
            }
            else
            {
                sal_uInt32 n;   // Dummy
                SbiInstance::PrepareNumberFormatter( pFormatter, nIndex, n, n );
            }

            pFormatter->GetOutputString( nDays, nIndex, aRes, &pCol );
            pMeth->PutString( aRes );

            // #39629 pFormatter kann selbst angefordert sein
            if( !pINST )
                delete pFormatter;
        }
        else
            pMeth->PutDate( nDays );
    }
    else
    {
        StarBASIC::Error( SbERR_NOT_IMPLEMENTED );
    }
}

RTLFUNC(IsArray)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        rPar.Get(0)->PutBool((rPar.Get(1)->GetType() & SbxARRAY) ? sal_True : sal_False );
}

RTLFUNC(IsObject)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariable* pVar = rPar.Get(1);
        SbxBase* pObj = (SbxBase*)pVar->GetObject();

        // #100385: GetObject can result in an error, so reset it
        SbxBase::ResetError();

        SbUnoClass* pUnoClass;
        sal_Bool bObject;
        if( pObj &&  NULL != ( pUnoClass=PTR_CAST(SbUnoClass,pObj) ) )
        {
            bObject = pUnoClass->getUnoClass().is();
        }
        else
        {
            bObject = pVar->IsObject();
        }
        rPar.Get( 0 )->PutBool( bObject );
    }
}

RTLFUNC(IsDate)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // #46134 Nur String wird konvertiert, andere Typen ergeben sal_False
        SbxVariableRef xArg = rPar.Get( 1 );
        SbxDataType eType = xArg->GetType();
        sal_Bool bDate = sal_False;

        if( eType == SbxDATE )
        {
            bDate = sal_True;
        }
        else if( eType == SbxSTRING )
        {
            // Error loeschen
            SbxError nPrevError = SbxBase::GetError();
            SbxBase::ResetError();

            // Konvertierung des Parameters nach SbxDATE erzwingen
            xArg->SbxValue::GetDate();

            // Bei Fehler ist es kein Date
            bDate = !SbxBase::IsError();

            // Error-Situation wiederherstellen
            SbxBase::ResetError();
            SbxBase::SetError( nPrevError );
        }
        rPar.Get( 0 )->PutBool( bDate );
    }
}

RTLFUNC(IsEmpty)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        rPar.Get( 0 )->PutBool( rPar.Get(1)->IsEmpty() );
}

RTLFUNC(IsError)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        rPar.Get( 0 )->PutBool( rPar.Get(1)->IsErr() );
}

RTLFUNC(IsNull)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // #51475 Wegen Uno-Objekten auch true liefern,
        // wenn der pObj-Wert NULL ist
        SbxVariableRef pArg = rPar.Get( 1 );
        sal_Bool bNull = rPar.Get(1)->IsNull();
        if( !bNull && pArg->GetType() == SbxOBJECT )
        {
            SbxBase* pObj = pArg->GetObject();
            if( !pObj )
                bNull = sal_True;
        }
        rPar.Get( 0 )->PutBool( bNull );
    }
}

RTLFUNC(IsNumeric)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        rPar.Get( 0 )->PutBool( rPar.Get( 1 )->IsNumericRTL() );
}

// Das machen wir auf die billige Tour

RTLFUNC(IsMissing)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        // #57915 Missing wird durch Error angezeigt
        rPar.Get( 0 )->PutBool( rPar.Get(1)->IsErr() );
}

// Dir( [Maske] [,Attrs] )
// ToDo: Library-globaler Datenbereich fuer Dir-Objekt und Flags


String getDirectoryPath( String aPathStr )
{
    String aRetStr;

    DirectoryItem aItem;
    FileBase::RC nRet = DirectoryItem::get( aPathStr, aItem );
    if( nRet == FileBase::E_None )
    {
        FileStatus aFileStatus( FileStatusMask_Type );
        nRet = aItem.getFileStatus( aFileStatus );
        if( nRet == FileBase::E_None )
        {
            FileStatus::Type aType = aFileStatus.getFileType();
            if( isFolder( aType ) )
            {
                aRetStr = aPathStr;
            }
            else if( aType == FileStatus::Link )
            {
                FileStatus aFileStatus2( FileStatusMask_LinkTargetURL );
                nRet = aItem.getFileStatus( aFileStatus2 );
                if( nRet == FileBase::E_None )
                    aRetStr = getDirectoryPath( aFileStatus2.getLinkTargetURL() );
            }
        }
    }
    return aRetStr;
}

// Function looks for wildcards, removes them and always returns the pure path
String implSetupWildcard( const String& rFileParam, SbiRTLData* pRTLData )
{
    static String aAsterisk = String::CreateFromAscii( "*" );
    static sal_Char cDelim1 = (sal_Char)'/';
    static sal_Char cDelim2 = (sal_Char)'\\';
    static sal_Char cWild1 = '*';
    static sal_Char cWild2 = '?';

    delete pRTLData->pWildCard;
    pRTLData->pWildCard = NULL;
    pRTLData->sFullNameToBeChecked = String();

    String aFileParam = rFileParam;
    xub_StrLen nLastWild = aFileParam.SearchBackward( cWild1 );
    if( nLastWild == STRING_NOTFOUND )
        nLastWild = aFileParam.SearchBackward( cWild2 );
    sal_Bool bHasWildcards = ( nLastWild != STRING_NOTFOUND );


    xub_StrLen nLastDelim = aFileParam.SearchBackward( cDelim1 );
    if( nLastDelim == STRING_NOTFOUND )
        nLastDelim = aFileParam.SearchBackward( cDelim2 );

    if( bHasWildcards )
    {
        // Wildcards in path?
        if( nLastDelim != STRING_NOTFOUND && nLastDelim > nLastWild )
            return aFileParam;
    }
    else
    {
        String aPathStr = getFullPath( aFileParam );
        if( nLastDelim != aFileParam.Len() - 1 )
            pRTLData->sFullNameToBeChecked = aPathStr;
        return aPathStr;
    }

    String aPureFileName;
    if( nLastDelim == STRING_NOTFOUND )
    {
        aPureFileName = aFileParam;
        aFileParam = String();
    }
    else
    {
        aPureFileName = aFileParam.Copy( nLastDelim + 1 );
        aFileParam = aFileParam.Copy( 0, nLastDelim );
    }

    // Try again to get a valid URL/UNC-path with only the path
    String aPathStr = getFullPath( aFileParam );
    xub_StrLen nPureLen = aPureFileName.Len();

    // Is there a pure file name left? Otherwise the path is
    // invalid anyway because it was not accepted by OSL before
    if( nPureLen && aPureFileName != aAsterisk )
    {
        pRTLData->pWildCard = new WildCard( aPureFileName );
    }
    return aPathStr;
}

inline sal_Bool implCheckWildcard( const String& rName, SbiRTLData* pRTLData )
{
    sal_Bool bMatch = sal_True;

    if( pRTLData->pWildCard )
        bMatch = pRTLData->pWildCard->Matches( rName );
    return bMatch;
}


bool isRootDir( String aDirURLStr )
{
    INetURLObject aDirURLObj( aDirURLStr );
    sal_Bool bRoot = sal_False;

    // Check if it's a root directory
    sal_Int32 nCount = aDirURLObj.getSegmentCount();

    // No segment means Unix root directory "file:///"
    if( nCount == 0 )
    {
        bRoot = sal_True;
    }
    // Exactly one segment needs further checking, because it
    // can be Unix "file:///foo/" -> no root
    // or Windows  "file:///c:/"  -> root
    else if( nCount == 1 )
    {
        ::rtl::OUString aSeg1 = aDirURLObj.getName( 0, sal_True,
            INetURLObject::DECODE_WITH_CHARSET );
        if( aSeg1.getStr()[1] == (sal_Unicode)':' )
        {
            bRoot = sal_True;
        }
    }
    // More than one segments can never be root
    // so bRoot remains sal_False

    return bRoot;
}

RTLFUNC(Dir)
{
    (void)pBasic;
    (void)bWrite;

    String aPath;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount > 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbiRTLData* pRTLData = pINST->GetRTLData();

        // #34645: Kann auch von der URL-Zeile ueber 'macro: Dir' aufgerufen werden
        // dann existiert kein pRTLData und die Methode muss verlassen werden
        if( !pRTLData )
            return;

        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                if ( nParCount >= 2 )
                {
                    String aFileParam = rPar.Get(1)->GetString();

                    String aFileURLStr = implSetupWildcard( aFileParam, pRTLData );
                    if( pRTLData->sFullNameToBeChecked.Len() > 0 )
                    {
                        sal_Bool bExists = sal_False;
                        try { bExists = xSFI->exists( aFileURLStr ); }
                        catch( Exception & ) {}

                        String aNameOnlyStr;
                        if( bExists )
                        {
                            INetURLObject aFileURL( aFileURLStr );
                            aNameOnlyStr = aFileURL.getName( INetURLObject::LAST_SEGMENT,
                                true, INetURLObject::DECODE_WITH_CHARSET );
                        }
                        rPar.Get(0)->PutString( aNameOnlyStr );
                        return;
                    }

                    try
                    {
                        String aDirURLStr;
                        sal_Bool bFolder = xSFI->isFolder( aFileURLStr );

                        if( bFolder )
                        {
                            aDirURLStr = aFileURLStr;
                        }
                        else
                        {
                            String aEmptyStr;
                            rPar.Get(0)->PutString( aEmptyStr );
                        }

                        sal_uInt16 nFlags = 0;
                        if ( nParCount > 2 )
                            pRTLData->nDirFlags = nFlags = rPar.Get(2)->GetInteger();
                        else
                            pRTLData->nDirFlags = 0;

                        // Read directory
                        sal_Bool bIncludeFolders = ((nFlags & Sb_ATTR_DIRECTORY) != 0);
                        pRTLData->aDirSeq = xSFI->getFolderContents( aDirURLStr, bIncludeFolders );
                        pRTLData->nCurDirPos = 0;

                        // #78651 Add "." and ".." directories for VB compatibility
                        if( bIncludeFolders )
                        {
                            sal_Bool bRoot = isRootDir( aDirURLStr );

                            // If it's no root directory we flag the need for
                            // the "." and ".." directories by the value -2
                            // for the actual position. Later for -2 will be
                            // returned "." and for -1 ".."
                            if( !bRoot )
                            {
                                pRTLData->nCurDirPos = -2;
                            }
                        }
                    }
                    catch( Exception & )
                    {
                        //StarBASIC::Error( ERRCODE_IO_GENERAL );
                    }
                }


                if( pRTLData->aDirSeq.getLength() > 0 )
                {
                    sal_Bool bFolderFlag = ((pRTLData->nDirFlags & Sb_ATTR_DIRECTORY) != 0);

                    SbiInstance* pInst = pINST;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    for( ;; )
                    {
                        if( pRTLData->nCurDirPos < 0 )
                        {
                            if( pRTLData->nCurDirPos == -2 )
                            {
                                aPath = ::rtl::OUString::createFromAscii( "." );
                            }
                            else if( pRTLData->nCurDirPos == -1 )
                            {
                                aPath = ::rtl::OUString::createFromAscii( ".." );
                            }
                            pRTLData->nCurDirPos++;
                        }
                        else if( pRTLData->nCurDirPos >= pRTLData->aDirSeq.getLength() )
                        {
                            pRTLData->aDirSeq.realloc( 0 );
                            aPath.Erase();
                            break;
                        }
                        else
                        {
                            ::rtl::OUString aFile = pRTLData->aDirSeq.getConstArray()[pRTLData->nCurDirPos++];

                            if( bCompatibility )
                            {
                                if( !bFolderFlag )
                                {
                                    sal_Bool bFolder = xSFI->isFolder( aFile );
                                    if( bFolder )
                                        continue;
                                }
                            }
                            else
                            {
                                // Only directories
                                if( bFolderFlag )
                                {
                                    sal_Bool bFolder = xSFI->isFolder( aFile );
                                    if( !bFolder )
                                        continue;
                                }
                            }

                            INetURLObject aURL( aFile );
                            aPath = aURL.getName( INetURLObject::LAST_SEGMENT, sal_True,
                                INetURLObject::DECODE_WITH_CHARSET );
                        }

                        sal_Bool bMatch = implCheckWildcard( aPath, pRTLData );
                        if( !bMatch )
                            continue;

                        break;
                    }
                }
                rPar.Get(0)->PutString( aPath );
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            if ( nParCount >= 2 )
            {
                delete pRTLData->pDir;
                pRTLData->pDir = 0; // wg. Sonderbehandlung Sb_ATTR_VOLUME
                DirEntry aEntry( rPar.Get(1)->GetString() );
                FileStat aStat( aEntry );
                if(!aStat.GetError() && (aStat.GetKind() & FSYS_KIND_FILE))
                {
                    // ah ja, ist nur ein dateiname
                    // Pfad abschneiden (wg. VB4)
                    rPar.Get(0)->PutString( aEntry.GetName() );
                    return;
                }
                sal_uInt16 nFlags = 0;
                if ( nParCount > 2 )
                    pRTLData->nDirFlags = nFlags = rPar.Get(2)->GetInteger();
                else
                    pRTLData->nDirFlags = 0;

                // Sb_ATTR_VOLUME wird getrennt gehandelt
                if( pRTLData->nDirFlags & Sb_ATTR_VOLUME )
                    aPath = aEntry.GetVolume();
                else
                {
                    // Die richtige Auswahl treffen
                    sal_uInt16 nMode = FSYS_KIND_FILE;
                    if( nFlags & Sb_ATTR_DIRECTORY )
                        nMode |= FSYS_KIND_DIR;
                    if( nFlags == Sb_ATTR_DIRECTORY )
                        nMode = FSYS_KIND_DIR;
                    pRTLData->pDir = new Dir( aEntry, (DirEntryKind) nMode );
                    pRTLData->nCurDirPos = 0;
                }
            }

            if( pRTLData->pDir )
            {
                for( ;; )
                {
                    if( pRTLData->nCurDirPos >= pRTLData->pDir->Count() )
                    {
                        delete pRTLData->pDir;
                        pRTLData->pDir = 0;
                        aPath.Erase();
                        break;
                    }
                    DirEntry aNextEntry=(*(pRTLData->pDir))[pRTLData->nCurDirPos++];
                    aPath = aNextEntry.GetName(); //Full();
                    break;
                }
            }
            rPar.Get(0)->PutString( aPath );
#else
            // TODO: OSL
            if ( nParCount >= 2 )
            {
                String aFileParam = rPar.Get(1)->GetString();

                String aDirURL = implSetupWildcard( aFileParam, pRTLData );

                sal_uInt16 nFlags = 0;
                if ( nParCount > 2 )
                    pRTLData->nDirFlags = nFlags = rPar.Get(2)->GetInteger();
                else
                    pRTLData->nDirFlags = 0;

                // Read directory
                sal_Bool bIncludeFolders = ((nFlags & Sb_ATTR_DIRECTORY) != 0);
                pRTLData->pDir = new Directory( aDirURL );
                FileBase::RC nRet = pRTLData->pDir->open();
                if( nRet != FileBase::E_None )
                {
                    delete pRTLData->pDir;
                    pRTLData->pDir = NULL;
                    rPar.Get(0)->PutString( String() );
                    return;
                }

                // #86950 Add "." and ".." directories for VB compatibility
                pRTLData->nCurDirPos = 0;
                if( bIncludeFolders )
                {
                    sal_Bool bRoot = isRootDir( aDirURL );

                    // If it's no root directory we flag the need for
                    // the "." and ".." directories by the value -2
                    // for the actual position. Later for -2 will be
                    // returned "." and for -1 ".."
                    if( !bRoot )
                    {
                        pRTLData->nCurDirPos = -2;
                    }
                }

            }

            if( pRTLData->pDir )
            {
                sal_Bool bFolderFlag = ((pRTLData->nDirFlags & Sb_ATTR_DIRECTORY) != 0);
                for( ;; )
                {
                    if( pRTLData->nCurDirPos < 0 )
                    {
                        if( pRTLData->nCurDirPos == -2 )
                        {
                            aPath = ::rtl::OUString::createFromAscii( "." );
                        }
                        else if( pRTLData->nCurDirPos == -1 )
                        {
                            aPath = ::rtl::OUString::createFromAscii( ".." );
                        }
                        pRTLData->nCurDirPos++;
                    }
                    else
                    {
                        DirectoryItem aItem;
                        FileBase::RC nRet = pRTLData->pDir->getNextItem( aItem );
                        if( nRet != FileBase::E_None )
                        {
                            delete pRTLData->pDir;
                            pRTLData->pDir = NULL;
                            aPath.Erase();
                            break;
                        }

                        // Handle flags
                        FileStatus aFileStatus( FileStatusMask_Type | FileStatusMask_FileName );
                        nRet = aItem.getFileStatus( aFileStatus );

                        // Only directories?
                        if( bFolderFlag )
                        {
                            FileStatus::Type aType = aFileStatus.getFileType();
                            sal_Bool bFolder = isFolder( aType );
                            if( !bFolder )
                                continue;
                        }

                        aPath = aFileStatus.getFileName();
                    }

                    sal_Bool bMatch = implCheckWildcard( aPath, pRTLData );
                    if( !bMatch )
                        continue;

                    break;
                }
            }
            rPar.Get(0)->PutString( aPath );
#endif
        }
    }
}


RTLFUNC(GetAttr)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        sal_Int16 nFlags = 0;

        // In Windows, We want to use Windows API to get the file attributes
        // for VBA interoperability.
    #if defined( WNT )
        if( SbiRuntime::isVBAEnabled() )
        {
            DirEntry aEntry( rPar.Get(1)->GetString() );
            aEntry.ToAbs();

            // #57064 Bei virtuellen URLs den Real-Path extrahieren
            ByteString aByteStrFullPath( aEntry.GetFull(), gsl_getSystemTextEncoding() );
            DWORD nRealFlags = GetFileAttributes (aByteStrFullPath.GetBuffer());
            if (nRealFlags != 0xffffffff)
            {
                if (nRealFlags == FILE_ATTRIBUTE_NORMAL)
                    nRealFlags = 0;
                nFlags = (sal_Int16) (nRealFlags);
            }
            else
                StarBASIC::Error( SbERR_FILE_NOT_FOUND );

            rPar.Get(0)->PutInteger( nFlags );

            return;
        }
    #endif

        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    String aPath = getFullPath( rPar.Get(1)->GetString() );
                    sal_Bool bExists = sal_False;
                    try { bExists = xSFI->exists( aPath ); }
                    catch( Exception & ) {}
                    if( !bExists )
                    {
                        StarBASIC::Error( SbERR_FILE_NOT_FOUND );
                        return;
                    }

                    sal_Bool bReadOnly = xSFI->isReadOnly( aPath );
                    sal_Bool bHidden = xSFI->isHidden( aPath );
                    sal_Bool bDirectory = xSFI->isFolder( aPath );
                    if( bReadOnly )
                        nFlags |= 0x0001; // ATTR_READONLY
                    if( bHidden )
                        nFlags |= 0x0002; // ATTR_HIDDEN
                    if( bDirectory )
                        nFlags |= 0x0010; // ATTR_DIRECTORY
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
            DirectoryItem aItem;
            FileBase::RC nRet = DirectoryItem::get( getFullPathUNC( rPar.Get(1)->GetString() ), aItem );
            FileStatus aFileStatus( FileStatusMask_Attributes | FileStatusMask_Type );
            nRet = aItem.getFileStatus( aFileStatus );
            sal_uInt64 nAttributes = aFileStatus.getAttributes();
            sal_Bool bReadOnly = (nAttributes & Attribute_ReadOnly) != 0;

            FileStatus::Type aType = aFileStatus.getFileType();
            sal_Bool bDirectory = isFolder( aType );
            if( bReadOnly )
                nFlags |= 0x0001; // ATTR_READONLY
            if( bDirectory )
                nFlags |= 0x0010; // ATTR_DIRECTORY
        }
        rPar.Get(0)->PutInteger( nFlags );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}


RTLFUNC(FileDateTime)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // <-- UCB
        String aPath = rPar.Get(1)->GetString();
        Time aTime;
        Date aDate;
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    com::sun::star::util::DateTime aUnoDT = xSFI->getDateTimeModified( aPath );
                    aTime = Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds );
                    aDate = Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            DirEntry aEntry( aPath );
            FileStat aStat( aEntry );
            aTime = Time( aStat.TimeModified() );
            aDate = Date( aStat.DateModified() );
#else
            DirectoryItem aItem;
            FileBase::RC nRet = DirectoryItem::get( getFullPathUNC( aPath ), aItem );
            FileStatus aFileStatus( FileStatusMask_ModifyTime );
            nRet = aItem.getFileStatus( aFileStatus );
            TimeValue aTimeVal = aFileStatus.getModifyTime();
            oslDateTime aDT;
            osl_getDateTimeFromTimeValue( &aTimeVal, &aDT );

            aTime = Time( aDT.Hours, aDT.Minutes, aDT.Seconds, 10000000*aDT.NanoSeconds );
            aDate = Date( aDT.Day, aDT.Month, aDT.Year );
#endif
        }

        double fSerial = (double)GetDayDiff( aDate );
        long nSeconds = aTime.GetHour();
        nSeconds *= 3600;
        nSeconds += aTime.GetMin() * 60;
        nSeconds += aTime.GetSec();
        double nDays = ((double)nSeconds) / (double)(24.0*3600.0);
        fSerial += nDays;

        Color* pCol;

        // #39629 pINST pruefen, kann aus URL-Zeile gerufen werden
        SvNumberFormatter* pFormatter = NULL;
        sal_uInt32 nIndex;
        if( pINST )
        {
            pFormatter = pINST->GetNumberFormatter();
            nIndex = pINST->GetStdDateTimeIdx();
        }
        else
        {
            sal_uInt32 n;   // Dummy
            SbiInstance::PrepareNumberFormatter( pFormatter, n, n, nIndex );
        }

        String aRes;
        pFormatter->GetOutputString( fSerial, nIndex, aRes, &pCol );
        rPar.Get(0)->PutString( aRes );

        // #39629 pFormatter kann selbst angefordert sein
        if( !pINST )
            delete pFormatter;
    }
}


RTLFUNC(EOF)
{
    (void)pBasic;
    (void)bWrite;

    // AB 08/16/2000: No changes for UCB
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        // nChannel--;  // macht MD beim Oeffnen auch nicht
        SbiIoSystem* pIO = pINST->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( SbERR_BAD_CHANNEL );
            return;
        }
        sal_Bool bIsEof;
        SvStream* pSvStrm = pSbStrm->GetStrm();
        if ( pSbStrm->IsText() )
        {
            char cBla;
            (*pSvStrm) >> cBla; // koennen wir noch ein Zeichen lesen
            bIsEof = pSvStrm->IsEof();
            if ( !bIsEof )
                pSvStrm->SeekRel( -1 );
        }
        else
            bIsEof = pSvStrm->IsEof();  // fuer binaerdateien!
        rPar.Get(0)->PutBool( bIsEof );
    }
}

RTLFUNC(FileAttr)
{
    (void)pBasic;
    (void)bWrite;

    // AB 08/16/2000: No changes for UCB

    // #57064 Obwohl diese Funktion nicht mit DirEntry arbeitet, ist sie von
    // der Anpassung an virtuelle URLs nich betroffen, da sie nur auf bereits
    // geoeffneten Dateien arbeitet und der Name hier keine Rolle spielt.

    if ( rPar.Count() != 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
//      nChannel--;
        SbiIoSystem* pIO = pINST->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( SbERR_BAD_CHANNEL );
            return;
        }
        sal_Int16 nRet;
        if ( rPar.Get(2)->GetInteger() == 1 )
            nRet = (sal_Int16)(pSbStrm->GetMode());
        else
            nRet = 0; // System file handle not supported

        rPar.Get(0)->PutInteger( nRet );
    }
}
RTLFUNC(Loc)
{
    (void)pBasic;
    (void)bWrite;

    // AB 08/16/2000: No changes for UCB
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = pINST->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( SbERR_BAD_CHANNEL );
            return;
        }
        SvStream* pSvStrm = pSbStrm->GetStrm();
        sal_uIntPtr nPos;
        if( pSbStrm->IsRandom())
        {
            short nBlockLen = pSbStrm->GetBlockLen();
            nPos = nBlockLen ? (pSvStrm->Tell() / nBlockLen) : 0;
            nPos++; // Blockpositionen beginnen bei 1
        }
        else if ( pSbStrm->IsText() )
            nPos = pSbStrm->GetLine();
        else if( pSbStrm->IsBinary() )
            nPos = pSvStrm->Tell();
        else if ( pSbStrm->IsSeq() )
            nPos = ( pSvStrm->Tell()+1 ) / 128;
        else
            nPos = pSvStrm->Tell();
        rPar.Get(0)->PutLong( (sal_Int32)nPos );
    }
}

RTLFUNC(Lof)
{
    (void)pBasic;
    (void)bWrite;

    // AB 08/16/2000: No changes for UCB
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = pINST->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( SbERR_BAD_CHANNEL );
            return;
        }
        SvStream* pSvStrm = pSbStrm->GetStrm();
        sal_uIntPtr nOldPos = pSvStrm->Tell();
        sal_uIntPtr nLen = pSvStrm->Seek( STREAM_SEEK_TO_END );
        pSvStrm->Seek( nOldPos );
        rPar.Get(0)->PutLong( (sal_Int32)nLen );
    }
}


RTLFUNC(Seek)
{
    (void)pBasic;
    (void)bWrite;

    // AB 08/16/2000: No changes for UCB
    int nArgs = (int)rPar.Count();
    if ( nArgs < 2 || nArgs > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
//  nChannel--;
    SbiIoSystem* pIO = pINST->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nChannel );
    if ( !pSbStrm )
    {
        StarBASIC::Error( SbERR_BAD_CHANNEL );
        return;
    }
    SvStream* pStrm = pSbStrm->GetStrm();

    if ( nArgs == 2 )   // Seek-Function
    {
        sal_uIntPtr nPos = pStrm->Tell();
        if( pSbStrm->IsRandom() )
            nPos = nPos / pSbStrm->GetBlockLen();
        nPos++; // Basic zaehlt ab 1
        rPar.Get(0)->PutLong( (sal_Int32)nPos );
    }
    else                // Seek-Statement
    {
        sal_Int32 nPos = rPar.Get(2)->GetLong();
        if ( nPos < 1 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }
        nPos--; // Basic zaehlt ab 1, SvStreams zaehlen ab 0
        pSbStrm->SetExpandOnWriteTo( 0 );
        if ( pSbStrm->IsRandom() )
            nPos *= pSbStrm->GetBlockLen();
        pStrm->Seek( (sal_uIntPtr)nPos );
        pSbStrm->SetExpandOnWriteTo( nPos );
    }
}

RTLFUNC(Format)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nArgCount = (sal_uInt16)rPar.Count();
    if ( nArgCount < 2 || nArgCount > 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aResult;
        if( nArgCount == 2 )
            rPar.Get(1)->Format( aResult );
        else
        {
            String aFmt( rPar.Get(2)->GetString() );
            rPar.Get(1)->Format( aResult, &aFmt );
        }
        rPar.Get(0)->PutString( aResult );
    }
}

RTLFUNC(Randomize)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() > 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    sal_Int16 nSeed;
    if( rPar.Count() == 2 )
        nSeed = (sal_Int16)rPar.Get(1)->GetInteger();
    else
        nSeed = (sal_Int16)rand();
    srand( nSeed );
}

RTLFUNC(Rnd)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() > 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        double nRand = (double)rand();
        nRand = ( nRand / (double)RAND_MAX );
        rPar.Get(0)->PutDouble( nRand );
    }
}


//
//  Syntax: Shell("Path",[ Window-Style,[ "Params", [ bSync = sal_False ]]])
//
//  WindowStyles (VBA-kompatibel):
//      2 == Minimized
//      3 == Maximized
//     10 == Full-Screen (Textmodus-Anwendungen OS/2, WIN95, WNT)
//
// !!!HACK der WindowStyle wird im Creator an Application::StartApp
//         uebergeben. Format: "xxxx2"
//


RTLFUNC(Shell)
{
    (void)pBasic;
    (void)bWrite;

    // No shell command for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    sal_uIntPtr nArgCount = rPar.Count();
    if ( nArgCount < 2 || nArgCount > 5 )
    {
        rPar.Get(0)->PutLong(0);
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        sal_uInt16 nOptions = vos::OProcess::TOption_SearchPath|
                          vos::OProcess::TOption_Detached;
        String aCmdLine = rPar.Get(1)->GetString();
        // Zusaetzliche Parameter anhaengen, es muss eh alles geparsed werden
        if( nArgCount >= 4 )
        {
            aCmdLine.AppendAscii( " " );
            aCmdLine += rPar.Get(3)->GetString();
        }
        else if( !aCmdLine.Len() )
        {
            // Spezial-Behandlung (leere Liste) vermeiden
            aCmdLine.AppendAscii( " " );
        }
        sal_uInt16 nLen = aCmdLine.Len();

        // #55735 Wenn Parameter dabei sind, muessen die abgetrennt werden
        // #72471 Auch die einzelnen Parameter trennen
        std::list<String> aTokenList;
        String aToken;
        sal_uInt16 i = 0;
        sal_Unicode c;
        while( i < nLen )
        {
            // Spaces weg
            for ( ;; ++i )
            {
                c = aCmdLine.GetBuffer()[ i ];
                if ( c != ' ' && c != '\t' )
                    break;
            }

            if( c == '\"' || c == '\'' )
            {
                sal_uInt16 iFoundPos = aCmdLine.Search( c, i + 1 );

                // Wenn nichts gefunden wurde, Rest kopieren
                if( iFoundPos == STRING_NOTFOUND )
                {
                    aToken = aCmdLine.Copy( i, STRING_LEN );
                    i = nLen;
                }
                else
                {
                    aToken = aCmdLine.Copy( i + 1, (iFoundPos - i - 1) );
                    i = iFoundPos + 1;
                }
            }
            else
            {
                sal_uInt16 iFoundSpacePos = aCmdLine.Search( ' ', i );
                sal_uInt16 iFoundTabPos = aCmdLine.Search( '\t', i );
                sal_uInt16 iFoundPos = Min( iFoundSpacePos, iFoundTabPos );

                // Wenn nichts gefunden wurde, Rest kopieren
                if( iFoundPos == STRING_NOTFOUND )
                {
                    aToken = aCmdLine.Copy( i, STRING_LEN );
                    i = nLen;
                }
                else
                {
                    aToken = aCmdLine.Copy( i, (iFoundPos - i) );
                    i = iFoundPos;
                }
            }

            // In die Liste uebernehmen
            aTokenList.push_back( aToken );
        }
        // #55735 / #72471 Ende

        sal_Int16 nWinStyle = 0;
        if( nArgCount >= 3 )
        {
            nWinStyle = rPar.Get(2)->GetInteger();
            switch( nWinStyle )
            {
                case 2:
                    nOptions |= vos::OProcess::TOption_Minimized;
                    break;
                case 3:
                    nOptions |= vos::OProcess::TOption_Maximized;
                    break;
                case 10:
                    nOptions |= vos::OProcess::TOption_FullScreen;
                    break;
            }

            sal_Bool bSync = sal_False;
            if( nArgCount >= 5 )
                bSync = rPar.Get(4)->GetBool();
            if( bSync )
                nOptions |= vos::OProcess::TOption_Wait;
        }
        vos::OProcess::TProcessOption eOptions =
            (vos::OProcess::TProcessOption)nOptions;


        // #72471 Parameter aufbereiten
        std::list<String>::const_iterator iter = aTokenList.begin();
        const String& rStr = *iter;
        ::rtl::OUString aOUStrProg( rStr.GetBuffer(), rStr.Len() );
        String aOUStrProgUNC = getFullPathUNC( aOUStrProg );

        iter++;

        sal_uInt16 nParamCount = sal::static_int_cast< sal_uInt16 >(
            aTokenList.size() - 1 );
        ::rtl::OUString* pArgumentList = NULL;
        //const char** pParamList = NULL;
        if( nParamCount )
        {
            pArgumentList = new ::rtl::OUString[ nParamCount ];
            //pParamList = new const char*[ nParamCount ];
            sal_uInt16 iList = 0;
            while( iter != aTokenList.end() )
            {
                const String& rParamStr = (*iter);
                pArgumentList[iList++] = ::rtl::OUString( rParamStr.GetBuffer(), rParamStr.Len() );
                //pParamList[iList++] = (*iter).GetStr();
                iter++;
            }
        }

        //const char* pParams = aParams.Len() ? aParams.GetStr() : 0;
        vos::OProcess* pApp;
        pApp = new vos::OProcess( aOUStrProgUNC );
        sal_Bool bSucc;
        if( nParamCount == 0 )
        {
            bSucc = pApp->execute( eOptions ) == vos::OProcess::E_None;
        }
        else
        {
            vos::OArgumentList aArgList( pArgumentList, nParamCount );
            bSucc = pApp->execute( eOptions, aArgList ) == vos::OProcess::E_None;
        }

        /*
        if( nParamCount == 0 )
            pApp = new vos::OProcess( pProg );
        else
            pApp = new vos::OProcess( pProg, pParamList, nParamCount );
        sal_Bool bSucc = pApp->execute( eOptions ) == vos::OProcess::E_None;
        */

        delete pApp;
        delete[] pArgumentList;
        if( !bSucc )
            StarBASIC::Error( SbERR_FILE_NOT_FOUND );
        else
            rPar.Get(0)->PutLong( 0 );
    }
}

RTLFUNC(VarType)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        rPar.Get(0)->PutInteger( (sal_Int16)eType );
    }
}

// Exported function
String getBasicTypeName( SbxDataType eType )
{
    static const char* pTypeNames[] =
    {
        "Empty",            // SbxEMPTY
        "Null",             // SbxNULL
        "Integer",          // SbxINTEGER
        "Long",             // SbxLONG
        "Single",           // SbxSINGLE
        "Double",           // SbxDOUBLE
        "Currency",         // SbxCURRENCY
        "Date",             // SbxDATE
        "String",           // SbxSTRING
        "Object",           // SbxOBJECT
        "Error",            // SbxERROR
        "Boolean",          // SbxBOOL
        "Variant",          // SbxVARIANT
        "DataObject",       // SbxDATAOBJECT
        "Unknown Type",     //
        "Unknown Type",     //
        "Char",             // SbxCHAR
        "Byte",             // SbxBYTE
        "UShort",           // SbxUSHORT
        "ULong",            // SbxULONG
        "Long64",           // SbxLONG64
        "ULong64",          // SbxULONG64
        "Int",              // SbxINT
        "UInt",             // SbxUINT
        "Void",             // SbxVOID
        "HResult",          // SbxHRESULT
        "Pointer",          // SbxPOINTER
        "DimArray",         // SbxDIMARRAY
        "CArray",           // SbxCARRAY
        "Userdef",          // SbxUSERDEF
        "Lpstr",            // SbxLPSTR
        "Lpwstr",           // SbxLPWSTR
        "Unknown Type",     // SbxCoreSTRING
        "WString",          // SbxWSTRING
        "WChar",            // SbxWCHAR
        "Int64",            // SbxSALINT64
        "UInt64",           // SbxSALUINT64
        "Decimal",          // SbxDECIMAL
    };

    int nPos = ((int)eType) & 0x0FFF;
    sal_uInt16 nTypeNameCount = sizeof( pTypeNames ) / sizeof( char* );
    if ( nPos < 0 || nPos >= nTypeNameCount )
        nPos = nTypeNameCount - 1;
    String aRetStr = String::CreateFromAscii( pTypeNames[nPos] );
    return aRetStr;
}

RTLFUNC(TypeName)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        sal_Bool bIsArray = ( ( eType & SbxARRAY ) != 0 );
        String aRetStr = getBasicTypeName( eType );
        if( bIsArray )
            aRetStr.AppendAscii( "()" );
        rPar.Get(0)->PutString( aRetStr );
    }
}

RTLFUNC(Len)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        const String& rStr = rPar.Get(1)->GetString();
        rPar.Get(0)->PutLong( (sal_Int32)rStr.Len() );
    }
}

RTLFUNC(DDEInitiate)
{
    (void)pBasic;
    (void)bWrite;

    // No DDE for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    int nArgs = (int)rPar.Count();
    if ( nArgs != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    const String& rApp = rPar.Get(1)->GetString();
    const String& rTopic = rPar.Get(2)->GetString();

    SbiDdeControl* pDDE = pINST->GetDdeControl();
    sal_Int16 nChannel;
    SbError nDdeErr = pDDE->Initiate( rApp, rTopic, nChannel );
    if( nDdeErr )
        StarBASIC::Error( nDdeErr );
    else
        rPar.Get(0)->PutInteger( nChannel );
}

RTLFUNC(DDETerminate)
{
    (void)pBasic;
    (void)bWrite;

    // No DDE for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    SbiDdeControl* pDDE = pINST->GetDdeControl();
    SbError nDdeErr = pDDE->Terminate( nChannel );
    if( nDdeErr )
        StarBASIC::Error( nDdeErr );
}

RTLFUNC(DDETerminateAll)
{
    (void)pBasic;
    (void)bWrite;

    // No DDE for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbiDdeControl* pDDE = pINST->GetDdeControl();
    SbError nDdeErr = pDDE->TerminateAll();
    if( nDdeErr )
        StarBASIC::Error( nDdeErr );

}

RTLFUNC(DDERequest)
{
    (void)pBasic;
    (void)bWrite;

    // No DDE for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    int nArgs = (int)rPar.Count();
    if ( nArgs != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    const String& rItem = rPar.Get(2)->GetString();
    SbiDdeControl* pDDE = pINST->GetDdeControl();
    String aResult;
    SbError nDdeErr = pDDE->Request( nChannel, rItem, aResult );
    if( nDdeErr )
        StarBASIC::Error( nDdeErr );
    else
        rPar.Get(0)->PutString( aResult );
}

RTLFUNC(DDEExecute)
{
    (void)pBasic;
    (void)bWrite;

    // No DDE for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    const String& rCommand = rPar.Get(2)->GetString();
    SbiDdeControl* pDDE = pINST->GetDdeControl();
    SbError nDdeErr = pDDE->Execute( nChannel, rCommand );
    if( nDdeErr )
        StarBASIC::Error( nDdeErr );
}

RTLFUNC(DDEPoke)
{
    (void)pBasic;
    (void)bWrite;

    // No DDE for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    const String& rItem = rPar.Get(2)->GetString();
    const String& rData = rPar.Get(3)->GetString();
    SbiDdeControl* pDDE = pINST->GetDdeControl();
    SbError nDdeErr = pDDE->Poke( nChannel, rItem, rData );
    if( nDdeErr )
        StarBASIC::Error( nDdeErr );
}


RTLFUNC(FreeFile)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    SbiIoSystem* pIO = pINST->GetIoSystem();
    short nChannel = 1;
    while( nChannel < CHANNELS )
    {
        SbiStream* pStrm = pIO->GetStream( nChannel );
        if( !pStrm )
        {
            rPar.Get(0)->PutInteger( nChannel );
            return;
        }
        nChannel++;
    }
    StarBASIC::Error( SbERR_TOO_MANY_FILES );
}

RTLFUNC(LBound)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = PTR_CAST(SbxDimArray,pParObj);
    if( pArr )
    {
        sal_Int32 nLower, nUpper;
        short nDim = (nParCount == 3) ? (short)rPar.Get(2)->GetInteger() : 1;
        if( !pArr->GetDim32( nDim, nLower, nUpper ) )
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        else
            rPar.Get(0)->PutLong( nLower );
    }
    else
        StarBASIC::Error( SbERR_MUST_HAVE_DIMS );
}

RTLFUNC(UBound)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = PTR_CAST(SbxDimArray,pParObj);
    if( pArr )
    {
        sal_Int32 nLower, nUpper;
        short nDim = (nParCount == 3) ? (short)rPar.Get(2)->GetInteger() : 1;
        if( !pArr->GetDim32( nDim, nLower, nUpper ) )
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        else
            rPar.Get(0)->PutLong( nUpper );
    }
    else
        StarBASIC::Error( SbERR_MUST_HAVE_DIMS );
}

RTLFUNC(RGB)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_uIntPtr nRed     = rPar.Get(1)->GetInteger() & 0xFF;
    sal_uIntPtr nGreen = rPar.Get(2)->GetInteger() & 0xFF;
    sal_uIntPtr nBlue  = rPar.Get(3)->GetInteger() & 0xFF;
    sal_uIntPtr nRGB;

    SbiInstance* pInst = pINST;
    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
    if( bCompatibility )
    {
        nRGB   = (nBlue << 16) | (nGreen << 8) | nRed;
    }
    else
    {
        nRGB   = (nRed << 16) | (nGreen << 8) | nBlue;
    }
    rPar.Get(0)->PutLong( nRGB );
}

RTLFUNC(QBColor)
{
    (void)pBasic;
    (void)bWrite;

    static const sal_Int32 pRGB[] =
    {
        0x000000,
        0x800000,
        0x008000,
        0x808000,
        0x000080,
        0x800080,
        0x008080,
        0xC0C0C0,
        0x808080,
        0xFF0000,
        0x00FF00,
        0xFFFF00,
        0x0000FF,
        0xFF00FF,
        0x00FFFF,
        0xFFFFFF,
    };

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_Int16 nCol = rPar.Get(1)->GetInteger();
    if( nCol < 0 || nCol > 15 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int32 nRGB = pRGB[ nCol ];
    rPar.Get(0)->PutLong( nRGB );
}

// StrConv(string, conversion, LCID)
RTLFUNC(StrConv)
{
    (void)pBasic;
    (void)bWrite;

    sal_uIntPtr nArgCount = rPar.Count()-1;
    if( nArgCount < 2 || nArgCount > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    String aOldStr = rPar.Get(1)->GetString();
    sal_Int32 nConversion = rPar.Get(2)->GetLong();

    sal_uInt16 nLanguage = LANGUAGE_SYSTEM;
    if( nArgCount == 3 )
    {
        // LCID not supported now
        //nLanguage = rPar.Get(3)->GetInteger();
    }

    sal_uInt16 nOldLen = aOldStr.Len();
    if( nOldLen == 0 )
    {
        // null string,return
        rPar.Get(0)->PutString(aOldStr);
        return;
    }

    sal_Int32 nType = 0;
    if ( (nConversion & 0x03) == 3 ) //  vbProperCase
    {
        CharClass& rCharClass = GetCharClass();
        aOldStr = rCharClass.toTitle( aOldStr.ToLowerAscii(), 0, nOldLen );
    }
    else if ( (nConversion & 0x01) == 1 ) // vbUpperCase
        nType |= ::com::sun::star::i18n::TransliterationModules_LOWERCASE_UPPERCASE;
    else if ( (nConversion & 0x02) == 2 ) // vbLowerCase
        nType |= ::com::sun::star::i18n::TransliterationModules_UPPERCASE_LOWERCASE;

    if ( (nConversion & 0x04) == 4 ) // vbWide
        nType |= ::com::sun::star::i18n::TransliterationModules_HALFWIDTH_FULLWIDTH;
    else if ( (nConversion & 0x08) == 8 ) // vbNarrow
        nType |= ::com::sun::star::i18n::TransliterationModules_FULLWIDTH_HALFWIDTH;

    if ( (nConversion & 0x10) == 16) // vbKatakana
        nType |= ::com::sun::star::i18n::TransliterationModules_HIRAGANA_KATAKANA;
    else if ( (nConversion & 0x20) == 32 ) // vbHiragana
        nType |= ::com::sun::star::i18n::TransliterationModules_KATAKANA_HIRAGANA;

    String aNewStr( aOldStr );
    if( nType != 0 )
    {
        com::sun::star::uno::Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        ::utl::TransliterationWrapper aTransliterationWrapper( xSMgr,nType );
        com::sun::star::uno::Sequence<sal_Int32> aOffsets;
        aTransliterationWrapper.loadModuleIfNeeded( nLanguage );
        aNewStr = aTransliterationWrapper.transliterate( aOldStr, nLanguage, 0, nOldLen, &aOffsets );
    }

    if ( (nConversion & 0x40) == 64 ) // vbUnicode
    {
        // convert the string to byte string, preserving unicode (2 bytes per character)
        sal_uInt16 nSize = aNewStr.Len()*2;
        const sal_Unicode* pSrc = aNewStr.GetBuffer();
        sal_Char* pChar = new sal_Char[nSize+1];
        for( sal_uInt16 i=0; i < nSize; i++ )
        {
            pChar[i] = static_cast< sal_Char >( i%2 ? ((*pSrc) >> 8) & 0xff : (*pSrc) & 0xff );
            if( i%2 )
                pSrc++;
        }
        pChar[nSize] = '\0';
        ::rtl::OString aOStr(pChar);

        // there is no concept about default codepage in unix. so it is incorrectly in unix
        ::rtl::OUString aOUStr = ::rtl::OStringToOUString(aOStr, osl_getThreadTextEncoding());
        aNewStr = String(aOUStr);
        rPar.Get(0)->PutString( aNewStr );
        return;
    }
    else if ( (nConversion & 0x80) == 128 ) // vbFromUnicode
    {
        ::rtl::OUString aOUStr(aNewStr);
        // there is no concept about default codepage in unix. so it is incorrectly in unix
        ::rtl::OString aOStr = ::rtl::OUStringToOString(aNewStr,osl_getThreadTextEncoding());
        const sal_Char* pChar = aOStr.getStr();
        sal_uInt16 nArraySize = static_cast< sal_uInt16 >( aOStr.getLength() );
        SbxDimArray* pArray = new SbxDimArray(SbxBYTE);
        bool bIncIndex = (IsBaseIndexOne() && SbiRuntime::isVBAEnabled() );
        if(nArraySize)
        {
            if( bIncIndex )
                pArray->AddDim( 1, nArraySize );
            else
                pArray->AddDim( 0, nArraySize-1 );
        }
        else
        {
            pArray->unoAddDim( 0, -1 );
        }

        for( sal_uInt16 i=0; i< nArraySize; i++)
        {
            SbxVariable* pNew = new SbxVariable( SbxBYTE );
            pNew->PutByte(*pChar);
            pChar++;
            pNew->SetFlag( SBX_WRITE );
            short index = i;
            if( bIncIndex )
                ++index;
            pArray->Put( pNew, &index );
        }

        SbxVariableRef refVar = rPar.Get(0);
        sal_uInt16 nFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->PutObject( pArray );
        refVar->SetFlags( nFlags );
        refVar->SetParameters( NULL );
           return;
    }

    rPar.Get(0)->PutString(aNewStr);
}


RTLFUNC(Beep)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    Sound::Beep();
}

RTLFUNC(Load)
{
    (void)pBasic;
    (void)bWrite;

    if( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Diesen Call einfach an das Object weiterreichen
    SbxBase* pObj = (SbxObject*)rPar.Get(1)->GetObject();
    if ( pObj )
    {
        if( pObj->IsA( TYPE( SbUserFormModule ) ) )
        {
            ((SbUserFormModule*)pObj)->Load();
        }
        else if( pObj->IsA( TYPE( SbxObject ) ) )
        {
            SbxVariable* pVar = ((SbxObject*)pObj)->
                Find( String( RTL_CONSTASCII_USTRINGPARAM("Load") ), SbxCLASS_METHOD );
            if( pVar )
                pVar->GetInteger();
        }
    }
}

RTLFUNC(Unload)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // Diesen Call einfach an das Object weitereichen
    SbxBase* pObj = (SbxObject*)rPar.Get(1)->GetObject();
    if ( pObj )
    {
        if( pObj->IsA( TYPE( SbUserFormModule ) ) )
        {
            SbUserFormModule* pFormModule = ( SbUserFormModule* )pObj;
            pFormModule->Unload();
        }
        else if( pObj->IsA( TYPE( SbxObject ) ) )
        {
            SbxVariable* pVar = ((SbxObject*)pObj)->
                Find( String( RTL_CONSTASCII_USTRINGPARAM("Unload") ), SbxCLASS_METHOD );
            if( pVar )
                pVar->GetInteger();
        }
    }
}

RTLFUNC(LoadPicture)
{
    (void)pBasic;
    (void)bWrite;

    if( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    String aFileURL = getFullPath( rPar.Get(1)->GetString() );
    SvStream* pStream = utl::UcbStreamHelper::CreateStream( aFileURL, STREAM_READ );
    if( pStream != NULL )
    {
        Bitmap aBmp;
        ReadDIB(aBmp, *pStream, true);
        Graphic aGraphic(aBmp);

        SbxObjectRef xRef = new SbStdPicture;
        ((SbStdPicture*)(SbxObject*)xRef)->SetGraphic( aGraphic );
        rPar.Get(0)->PutObject( xRef );
    }
    delete pStream;
}

RTLFUNC(SavePicture)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbxBase* pObj = (SbxObject*)rPar.Get(1)->GetObject();
    if( pObj->IsA( TYPE( SbStdPicture ) ) )
    {
        SvFileStream aOStream( rPar.Get(2)->GetString(), STREAM_WRITE | STREAM_TRUNC );
        Graphic aGraphic = ((SbStdPicture*)pObj)->GetGraphic();
        aOStream << aGraphic;
    }
}


//-----------------------------------------------------------------------------------------

RTLFUNC(AboutStarBasic)
{
    (void)pBasic;
    (void)bWrite;
    (void)rPar;
}

RTLFUNC(MsgBox)
{
    (void)pBasic;
    (void)bWrite;

    static const WinBits nStyleMap[] =
    {
        WB_OK,              // MB_OK
        WB_OK_CANCEL,       // MB_OKCANCEL
        WB_ABORT_RETRY_IGNORE,    // MB_ABORTRETRYIGNORE
        WB_YES_NO_CANCEL,   // MB_YESNOCANCEL
        WB_YES_NO,          // MB_YESNO
        WB_RETRY_CANCEL     // MB_RETRYCANCEL
    };
    static const sal_Int16 nButtonMap[] =
    {
        2, // #define RET_CANCEL sal_False
        1, // #define RET_OK     sal_True
        6, // #define RET_YES    2
        7, // #define RET_NO     3
        4  // #define RET_RETRY  4
    };


    sal_uInt16 nArgCount = (sal_uInt16)rPar.Count();
    if( nArgCount < 2 || nArgCount > 6 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    WinBits nWinBits;
    WinBits nType = 0; // MB_OK
    if( nArgCount >= 3 )
        nType = (WinBits)rPar.Get(2)->GetInteger();
    WinBits nStyle = nType;
    nStyle &= 15; // Bits 4-16 loeschen
    if( nStyle > 5 )
        nStyle = 0;

    nWinBits = nStyleMap[ nStyle ];

    WinBits nWinDefBits;
    nWinDefBits = (WB_DEF_OK | WB_DEF_RETRY | WB_DEF_YES);
    if( nType & 256 )
    {
        if( nStyle == 5 )
            nWinDefBits = WB_DEF_CANCEL;
        else if( nStyle == 2 )
            nWinDefBits = WB_DEF_RETRY;
        else
            nWinDefBits = (WB_DEF_CANCEL | WB_DEF_RETRY | WB_DEF_NO);
    }
    else if( nType & 512 )
    {
        if( nStyle == 2)
            nWinDefBits = WB_DEF_IGNORE;
        else
            nWinDefBits = WB_DEF_CANCEL;
    }
    else if( nStyle == 2)
        nWinDefBits = WB_DEF_CANCEL;
    nWinBits |= nWinDefBits;

    String aMsg = rPar.Get(1)->GetString();
    String aTitle;
    if( nArgCount >= 4 )
        aTitle = rPar.Get(3)->GetString();
    else
        aTitle = GetpApp()->GetDisplayName();

    nType &= (16+32+64);
    MessBox* pBox = 0;
    Window* pParent = GetpApp()->GetDefDialogParent();
    switch( nType )
    {
        case 16:
            pBox = new ErrorBox( pParent, nWinBits, aMsg );
            break;
        case 32:
            pBox = new QueryBox( pParent, nWinBits, aMsg );
            break;
        case 48:
            pBox = new WarningBox( pParent, nWinBits, aMsg );
            break;
        case 64:
            pBox = new InfoBox( pParent, nWinBits, aMsg );
            break;
        default:
            pBox = new MessBox( pParent, nWinBits, aTitle, aMsg );
    }
    pBox->SetText( aTitle );
    sal_uInt16 nRet = (sal_uInt16)pBox->Execute();
    if( nRet == sal_True )
        nRet = 1;

    sal_Int16 nMappedRet;
    if( nStyle == 2 )
    {
        nMappedRet = nRet;
        if( nMappedRet == 0 )
            nMappedRet = 3; // Abort
    }
    else
        nMappedRet = nButtonMap[ nRet ];

    rPar.Get(0)->PutInteger( nMappedRet );
    delete pBox;
}

RTLFUNC(SetAttr) // JSM
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if ( rPar.Count() == 3 )
    {
        String aStr = rPar.Get(1)->GetString();
        sal_Int16 nFlags = rPar.Get(2)->GetInteger();

        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    sal_Bool bReadOnly = (nFlags & 0x0001) != 0; // ATTR_READONLY
                    xSFI->setReadOnly( aStr, bReadOnly );
                    sal_Bool bHidden   = (nFlags & 0x0002) != 0; // ATTR_HIDDEN
                    xSFI->setHidden( aStr, bHidden );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            // #57064 Bei virtuellen URLs den Real-Path extrahieren
            DirEntry aEntry( aStr );
            String aFile = aEntry.GetFull();
            ByteString aByteFile( aFile, gsl_getSystemTextEncoding() );
    #ifdef WNT
            if (!SetFileAttributes (aByteFile.GetBuffer(),(DWORD)nFlags))
                StarBASIC::Error(SbERR_FILE_NOT_FOUND);
    #endif
    #ifdef OS2
            FILESTATUS3 aFileStatus;
            APIRET rc = DosQueryPathInfo(aByteFile.GetBuffer(),1,
                                         &aFileStatus,sizeof(FILESTATUS3));
            if (!rc)
            {
                if (aFileStatus.attrFile != nFlags)
                {
                    aFileStatus.attrFile = nFlags;
                    rc = DosSetPathInfo(aFile.GetStr(),1,
                                        &aFileStatus,sizeof(FILESTATUS3),0);
                    if (rc)
                        StarBASIC::Error( SbERR_FILE_NOT_FOUND );
                }
            }
            else
                StarBASIC::Error( SbERR_FILE_NOT_FOUND );
    #endif
#else
            // Not implemented
#endif
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(Reset)  // JSM
{
    (void)pBasic;
    (void)bWrite;
    (void)rPar;

    SbiIoSystem* pIO = pINST->GetIoSystem();
    if (pIO)
        pIO->CloseAll();
}

RTLFUNC(DumpAllObjects)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nArgCount = (sal_uInt16)rPar.Count();
    if( nArgCount < 2 || nArgCount > 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else if( !pBasic )
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
    else
    {
        SbxObject* p = pBasic;
        while( p->GetParent() )
            p = p->GetParent();
        SvFileStream aStrm( rPar.Get( 1 )->GetString(),
                            STREAM_WRITE | STREAM_TRUNC );
        p->Dump( aStrm, rPar.Get( 2 )->GetBool() );
        aStrm.Close();
        if( aStrm.GetError() != SVSTREAM_OK )
            StarBASIC::Error( SbERR_IO_ERROR );
    }
}


RTLFUNC(FileExists)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        String aStr = rPar.Get(1)->GetString();
        sal_Bool bExists = sal_False;

        // <-- UCB
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    bExists = xSFI->exists( aStr );
                }
                catch( Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        // --> UCB
        {
#ifdef _OLD_FILE_IMPL
            DirEntry aEntry( aStr );
            bExists = aEntry.Exists();
#else
            DirectoryItem aItem;
            FileBase::RC nRet = DirectoryItem::get( getFullPathUNC( aStr ), aItem );
            bExists = (nRet == FileBase::E_None);
#endif
        }
        rPar.Get(0)->PutBool( bExists );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(Partition)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_Int32 nNumber = rPar.Get(1)->GetLong();
    sal_Int32 nStart = rPar.Get(2)->GetLong();
    sal_Int32 nStop = rPar.Get(3)->GetLong();
    sal_Int32 nInterval = rPar.Get(4)->GetLong();

    if( nStart < 0 || nStop <= nStart || nInterval < 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // the Partition function inserts leading spaces before lowervalue and uppervalue
    // so that they both have the same number of characters as the string
    // representation of the value (Stop + 1). This ensures that if you use the output
    // of the Partition function with several values of Number, the resulting text
    // will be handled properly during any subsequent sort operation.

    // calculate the  maximun number of characters before lowervalue and uppervalue
    ::rtl::OUString aBeforeStart = ::rtl::OUString::valueOf( nStart - 1 );
    ::rtl::OUString aAfterStop = ::rtl::OUString::valueOf( nStop + 1 );
    sal_Int32 nLen1 = aBeforeStart.getLength();
    sal_Int32 nLen2 = aAfterStop.getLength();
    sal_Int32 nLen = nLen1 >= nLen2 ? nLen1:nLen2;

    ::rtl::OUStringBuffer aRetStr( nLen * 2 + 1);
    ::rtl::OUString aLowerValue;
    ::rtl::OUString aUpperValue;
    if( nNumber < nStart )
    {
        aUpperValue = aBeforeStart;
    }
    else if( nNumber > nStop )
    {
        aLowerValue = aAfterStop;
    }
    else
    {
        sal_Int32 nLowerValue = nNumber;
        sal_Int32 nUpperValue = nLowerValue;
        if( nInterval > 1 )
        {
            nLowerValue = ((( nNumber - nStart ) / nInterval ) * nInterval ) + nStart;
            nUpperValue = nLowerValue + nInterval - 1;
        }

        aLowerValue = ::rtl::OUString::valueOf( nLowerValue );
        aUpperValue = ::rtl::OUString::valueOf( nUpperValue );
    }

    nLen1 = aLowerValue.getLength();
    nLen2 = aUpperValue.getLength();

    if( nLen > nLen1 )
    {
        // appending the leading spaces for the lowervalue
        for ( sal_Int32 i= (nLen - nLen1) ; i > 0; --i )
            aRetStr.appendAscii(" ");
    }
    aRetStr.append( aLowerValue ).appendAscii(":");
    if( nLen > nLen2 )
    {
        // appending the leading spaces for the uppervalue
        for ( sal_Int32 i= (nLen - nLen2) ; i > 0; --i )
            aRetStr.appendAscii(" ");
    }
    aRetStr.append( aUpperValue );
    rPar.Get(0)->PutString( String(aRetStr.makeStringAndClear()) );
}
