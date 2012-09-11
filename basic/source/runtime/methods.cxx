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

#include <tools/date.hxx>
#include <basic/sbxvar.hxx>
#include <osl/process.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
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
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include "runtime.hxx"
#include "sbunoobj.hxx"
#ifdef WNT
#include <tools/fsys.hxx>
#else
#include <osl/file.hxx>
#endif
#include "errobject.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/script/XErrorQuery.hpp>
#include <ooo/vba/XHelperInterface.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
using namespace comphelper;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;
using namespace com::sun::star::script;
using namespace com::sun::star::frame;

#include <comphelper/string.hxx>

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

SbxVariable* getDefaultProp( SbxVariable* pRef );

#if defined (WNT)
#include <direct.h>   // _getdcwd get current work directory, _chdrive
#endif

#ifdef UNX
#include <errno.h>
#include <unistd.h>
#endif

#include <basic/sbobjmod.hxx>

#ifdef WNT
#define GradientStyle_RECT BLA_GradientStyle_RECT
#include <windows.h>
#include <io.h>
#undef GetObject
#undef GradientSyle_RECT
#endif

#ifndef DISABLE_SCRIPTING

// forward decl.
bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet );

// from source/classes/sbxmod.cxx
Reference< XModel > getDocumentModel( StarBASIC* );

static void FilterWhiteSpace( String& rStr )
{
    if (!rStr.Len())
        return;

    rtl::OUStringBuffer aRet;

    for (xub_StrLen i = 0; i < rStr.Len(); ++i)
    {
        sal_Unicode cChar = rStr.GetChar(i);
        if ((cChar != ' ') && (cChar != '\t') &&
           (cChar != '\n') && (cChar != '\r'))
        {
            aRet.append(cChar);
        }
    }

    rStr = aRet.makeStringAndClear();
}

static long GetDayDiff( const Date& rDate );

static const CharClass& GetCharClass( void )
{
    static bool bNeedsInit = true;
    static ::com::sun::star::lang::Locale aLocale;
    if( bNeedsInit )
    {
        bNeedsInit = false;
        aLocale = Application::GetSettings().GetLocale();
    }
    static CharClass aCharClass( aLocale );
    return aCharClass;
}

static inline bool isFolder( FileStatus::Type aType )
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
                ( ::rtl::OUString("com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
        }
    }
    return xSFI;
}



// Properties and methods lie down the return value at the Get (bPut = sal_False) in the
// element 0 of the Argv; the value of element 0 is saved at Put (bPut = sal_True)

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
        // Convenience: enter BASIC as parent
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

        rtl::OUString aStr;
        if( !bChrW && SbiRuntime::isVBAEnabled() )
        {
            sal_Char c = static_cast<sal_Char>(pArg->GetByte());
            aStr = rtl::OUString(&c, 1, osl_getThreadTextEncoding());
        }
        else
        {
            sal_Unicode aCh = static_cast<sal_Unicode>(pArg->GetUShort());
            aStr = rtl::OUString(aCh);
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

    // #57064 Although this function doesn't work with DirEntry, it isn't touched
    // by the adjustment to virtual URLs, as, using the DirEntry-functionality,
    // there's no possibility to detect the current one in a way that a virtual URL
    // could be delivered.

#if defined (WNT)
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
    if ( _getdcwd( nCurDir, pBuffer, _MAX_PATH ) != 0 )
        rPar.Get(0)->PutString( rtl::OUString::createFromAscii( pBuffer ) );
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
            rPar.Get(0)->PutString( rtl::OUString::createFromAscii(pMem) );
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

RTLFUNC(ChDir)
{
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        // VBA: track current directory per document type (separately for Writer, Calc, Impress, etc.)
        if( SbiRuntime::isVBAEnabled() )
            ::basic::vba::registerCurrentDirectory( getDocumentModel( pBasic ), rPar.Get(1)->GetString() );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(ChDrive)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
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
        catch(const Exception & )
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

RTLFUNC(FileCopy)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 3)
    {
        String aSource = rPar.Get(1)->GetString();
        String aDest = rPar.Get(2)->GetString();
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    xSFI->copy( getFullPath( aSource ), getFullPath( aDest ) );
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( SbERR_PATH_NOT_FOUND );
                }
            }
        }
        else
        {
            FileBase::RC nRet = File::copy( getFullPathUNC( aSource ), getFullPathUNC( aDest ) );
            if( nRet != FileBase::E_None )
            {
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
            }
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(Kill)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        String aFileSpec = rPar.Get(1)->GetString();

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
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            File::remove( getFullPathUNC( aFileSpec ) );
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(MkDir)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        String aPath = rPar.Get(1)->GetString();

        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    xSFI->createFolder( getFullPath( aPath ) );
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            Directory::create( getFullPathUNC( aPath ) );
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}


// In OSL only empty directories can be deleted
// so we have to delete all files recursively
void implRemoveDirRecursive( const String& aDirPath )
{
    DirectoryItem aItem;
    FileBase::RC nRet = DirectoryItem::get( aDirPath, aItem );
    bool bExists = (nRet == FileBase::E_None);

    FileStatus aFileStatus( osl_FileStatus_Mask_Type );
    nRet = aItem.getFileStatus( aFileStatus );
    FileStatus::Type aType = aFileStatus.getFileType();
    bool bFolder = isFolder( aType );

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
        FileStatus aFileStatus2( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL );
        nRet = aItem2.getFileStatus( aFileStatus2 );
        ::rtl::OUString aPath = aFileStatus2.getFileURL();

        // Directory?
        FileStatus::Type aType2 = aFileStatus2.getFileType();
        bool bFolder2 = isFolder( aType2 );
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


RTLFUNC(RmDir)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        String aPath = rPar.Get(1)->GetString();
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
                    SbiInstance* pInst = GetSbData()->pInst;
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
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            implRemoveDirRecursive( getFullPathUNC( aPath ) );
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(SendKeys)
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
        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    nLen = xSFI->getSize( getFullPath( aStr ) );
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            DirectoryItem aItem;
            DirectoryItem::get( getFullPathUNC( aStr ), aItem );
            FileStatus aFileStatus( osl_FileStatus_Mask_FileSize );
            aItem.getFileStatus( aFileStatus );
            nLen = (sal_Int32)aFileStatus.getFileSize();
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
        SbxVariableRef pArg = rPar.Get( 1 );
        // converting value to unsigned and limit to 2 or 4 byte representation
        sal_uInt32 nVal = pArg->IsInteger() ?
            static_cast<sal_uInt16>(pArg->GetInteger()) :
            static_cast<sal_uInt32>(pArg->GetLong());
        rPar.Get(0)->PutString( rtl::OUString::valueOf( sal_Int64(nVal), 16 ) );
    }
}

RTLFUNC(FuncCaller)
{
    (void)pBasic;
    (void)bWrite;
    if ( SbiRuntime::isVBAEnabled() &&  GetSbData()->pInst && GetSbData()->pInst->pRun )
    {
        if ( GetSbData()->pInst->pRun->GetExternalCaller() )
            *rPar.Get(0) =  *GetSbData()->pInst->pRun->GetExternalCaller();
        else
        {
            SbxVariableRef pVar = new SbxVariable(SbxVARIANT);
            *rPar.Get(0) = *pVar;
        }
    }
    else
    {
        StarBASIC::Error( SbERR_NOT_IMPLEMENTED );
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

        SbiInstance* pInst = GetSbData()->pInst;
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

        SbiInstance* pInst = GetSbData()->pInst;
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
        const CharClass& rCharClass = GetCharClass();
        String aStr( rPar.Get(1)->GetString() );
        aStr = rCharClass.lowercase(aStr);
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
        rtl::OUString aStr(comphelper::string::stripStart(rPar.Get(1)->GetString(), ' '));
        rPar.Get(0)->PutString(aStr);
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
        // #23178: replicate the functionality of Mid$ as a command
        // by adding a replacement-string as a fourth parameter.
        // In contrast to the original the third parameter (nLength)
        // can't be left out here. That's considered in bWrite already.
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
                SbiInstance* pInst = GetSbData()->pInst;
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
        rPar.Get(0)->PutString( rtl::OUString::createFromAscii( aBuffer ) );
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

        SbiInstance* pInst = GetSbData()->pInst;
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
                    nPos = nPos - nFindStrLen + nReplaceStrLen + 1;
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
        rtl::OUString aStr(comphelper::string::stripEnd(rPar.Get(1)->GetString(), ' '));
        rPar.Get(0)->PutString(aStr);
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
            // replace commas by points so that it's symmetric to Val!
            aStr.SearchAndReplace( ',', '.' );

            SbiInstance* pInst = GetSbData()->pInst;
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

    SbiInstance* pInst = GetSbData()->pInst;
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
        const CharClass& rCharClass = GetCharClass();
        String aStr( rPar.Get(1)->GetString() );
        aStr = rCharClass.uppercase( aStr );
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
                rtl::OString aByteStr(rtl::OUStringToOString(aStr, osl_getThreadTextEncoding()));
                sal_Int16 nlResult = (sal_Int16)strtol( aByteStr.getStr()+2, &pEndPtr, nRadix);
                nResult = (double)nlResult;
            }
        }
        else
        {
            // #57844 use localized function
            nResult = ::rtl::math::stringToDouble( aStr, '.', ',', NULL, NULL );
            checkArithmeticOverflow( nResult );
        }

        rPar.Get(0)->PutDouble( nResult );
    }
}


// Helper functions for date conversion
sal_Int16 implGetDateDay( double aDate )
{
    aDate -= 2.0; // standardize: 1.1.1900 => 0.0
    Date aRefDate( 1, 1, 1900 );
    if ( aDate >= 0.0 )
    {
        aDate = floor( aDate );
        aRefDate += (sal_uIntPtr)aDate;
    }
    else
    {
        aDate = ceil( aDate );
        aRefDate -= (sal_uIntPtr)(-1.0 * aDate);
    }

    sal_Int16 nRet = (sal_Int16)( aRefDate.GetDay() );
    return nRet;
}

sal_Int16 implGetDateMonth( double aDate )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long)aDate;
    nDays -= 2; // standardize: 1.1.1900 => 0.0
    aRefDate += nDays;
    sal_Int16 nRet = (sal_Int16)( aRefDate.GetMonth() );
    return nRet;
}

sal_Int16 implGetDateYear( double aDate );

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
        String aRetStr = rtl::OUString::createFromAscii( Buffer );
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
        nHour = 0;                      // because of UNO DateTimes, which go till 24 o'clock
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
        // #39629 check GetSbData()->pInst, can be called from the URL line
        SvNumberFormatter* pFormatter = NULL;
        if( GetSbData()->pInst )
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
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
                // cut time
                if ( fResult  > 0.0 )
                    fResult = floor( fResult );
                else
                    fResult = ceil( fResult );
            }
            rPar.Get(0)->PutDate( fResult );
        }
        else
            StarBASIC::Error( SbERR_CONVERSION );

        // #39629 pFormatter can be requested itself
        if( !GetSbData()->pInst )
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
        SvNumberFormatter* pFormatter = NULL;
        if( GetSbData()->pInst )
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
        else
        {
            sal_uInt32 n;
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
                // cut days
                fResult = fmod( fResult, 1 );
            rPar.Get(0)->PutDate( fResult );
        }
        else
            StarBASIC::Error( SbERR_CONVERSION );

        if( !GetSbData()->pInst )
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
    if( dDate < 0.0 )
        dDate *= -1.0;
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

sal_Int16 implGetMinute( double dDate );

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
    if( dDate < 0.0 )
        dDate *= -1.0;
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
    Date aDate( Date::SYSTEM );
    Time aTime( Time::SYSTEM );
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
        Time aTime( Time::SYSTEM );
        SbxVariable* pMeth = rPar.Get( 0 );
        String aRes;
        if( pMeth->IsFixed() )
        {
            // Time$: hh:mm:ss
            char buf[ 20 ];
            snprintf( buf, sizeof(buf), "%02d:%02d:%02d",
                aTime.GetHour(), aTime.GetMin(), aTime.GetSec() );
            aRes = rtl::OUString::createFromAscii( buf );
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

            SvNumberFormatter* pFormatter = NULL;
            sal_uInt32 nIndex;
            if( GetSbData()->pInst )
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
                nIndex = GetSbData()->pInst->GetStdTimeIdx();
            }
            else
            {
                sal_uInt32 n;   // Dummy
                SbiInstance::PrepareNumberFormatter( pFormatter, n, nIndex, n );
            }

            pFormatter->GetOutputString( nDays, nIndex, aRes, &pCol );

            if( !GetSbData()->pInst )
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

    Time aTime( Time::SYSTEM );
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
        Date aToday( Date::SYSTEM );
        double nDays = (double)GetDayDiff( aToday );
        SbxVariable* pMeth = rPar.Get( 0 );
        if( pMeth->IsString() )
        {
            String aRes;
            Color* pCol;

            SvNumberFormatter* pFormatter = NULL;
            sal_uInt32 nIndex;
            if( GetSbData()->pInst )
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
                nIndex = GetSbData()->pInst->GetStdDateIdx();
            }
            else
            {
                sal_uInt32 n;
                SbiInstance::PrepareNumberFormatter( pFormatter, nIndex, n, n );
            }

            pFormatter->GetOutputString( nDays, nIndex, aRes, &pCol );
            pMeth->PutString( aRes );

            if( !GetSbData()->pInst )
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
        // #46134 only string is converted, all other types result in sal_False
        SbxVariableRef xArg = rPar.Get( 1 );
        SbxDataType eType = xArg->GetType();
        sal_Bool bDate = sal_False;

        if( eType == SbxDATE )
        {
            bDate = sal_True;
        }
        else if( eType == SbxSTRING )
        {
            SbxError nPrevError = SbxBase::GetError();
            SbxBase::ResetError();

            // force conversion of the parameter to SbxDATE
            xArg->SbxValue::GetDate();

            bDate = !SbxBase::IsError();

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
    {
        SbxVariable* pVar = NULL;
        if( SbiRuntime::isVBAEnabled() )
            pVar = getDefaultProp( rPar.Get(1) );
        if ( pVar )
        {
            pVar->Broadcast( SBX_HINT_DATAWANTED );
            rPar.Get( 0 )->PutBool( pVar->IsEmpty() );
        }
        else
            rPar.Get( 0 )->PutBool( rPar.Get(1)->IsEmpty() );
    }
}

RTLFUNC(IsError)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxVariable* pVar =rPar.Get( 1 );
        SbUnoObject* pObj = PTR_CAST(SbUnoObject,pVar );
                if ( !pObj )
                {
                    if ( SbxBase* pBaseObj = pVar->GetObject() )
                        pObj = PTR_CAST(SbUnoObject, pBaseObj );
                }
        Reference< XErrorQuery > xError;
        if ( pObj )
            xError.set( pObj->getUnoAny(), UNO_QUERY );
        if ( xError.is() )
            rPar.Get( 0 )->PutBool( xError->hasError() );
        else
            rPar.Get( 0 )->PutBool( rPar.Get(1)->IsErr() );
    }
}

RTLFUNC(IsNull)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // #51475 because of Uno-objects return true
        // even if the pObj value is NULL
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



RTLFUNC(IsMissing)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        // #57915 Missing is reported by an error
        rPar.Get( 0 )->PutBool( rPar.Get(1)->IsErr() );
}

// Function looks for wildcards, removes them and always returns the pure path
String implSetupWildcard( const String& rFileParam, SbiRTLData* pRTLData )
{
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
    bool bHasWildcards = ( nLastWild != STRING_NOTFOUND );


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

    rtl::OUString aPureFileName;
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

    // Is there a pure file name left? Otherwise the path is
    // invalid anyway because it was not accepted by OSL before
    if (comphelper::string::equals(aPureFileName, '*'))
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
    bool bRoot = false;

    // Check if it's a root directory
    sal_Int32 nCount = aDirURLObj.getSegmentCount();

    // No segment means Unix root directory "file:///"
    if( nCount == 0 )
    {
        bRoot = true;
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
            bRoot = true;
        }
    }
    // More than one segments can never be root
    // so bRoot remains false

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
        SbiRTLData* pRTLData = GetSbData()->pInst->GetRTLData();

        // #34645: can also be called from the URL line via 'macro: Dir'
        // there's no pRTLDate existing in that case and the method must be left
        if( !pRTLData )
            return;

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
                        catch(const Exception & ) {}

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
                            bool bRoot = isRootDir( aDirURLStr );

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
                    catch(const Exception & )
                    {
                    }
                }


                if( pRTLData->aDirSeq.getLength() > 0 )
                {
                    bool bFolderFlag = ((pRTLData->nDirFlags & Sb_ATTR_DIRECTORY) != 0);

                    SbiInstance* pInst = GetSbData()->pInst;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    for( ;; )
                    {
                        if( pRTLData->nCurDirPos < 0 )
                        {
                            if( pRTLData->nCurDirPos == -2 )
                            {
                                aPath = ::rtl::OUString("." );
                            }
                            else if( pRTLData->nCurDirPos == -1 )
                            {
                                aPath = ::rtl::OUString(".." );
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
        {
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
                bool bIncludeFolders = ((nFlags & Sb_ATTR_DIRECTORY) != 0);
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
                    bool bRoot = isRootDir( aDirURL );

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
                bool bFolderFlag = ((pRTLData->nDirFlags & Sb_ATTR_DIRECTORY) != 0);
                for( ;; )
                {
                    if( pRTLData->nCurDirPos < 0 )
                    {
                        if( pRTLData->nCurDirPos == -2 )
                        {
                            aPath = ::rtl::OUString("." );
                        }
                        else if( pRTLData->nCurDirPos == -1 )
                        {
                            aPath = ::rtl::OUString(".." );
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
                        FileStatus aFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName );
                        nRet = aItem.getFileStatus( aFileStatus );

                        // Only directories?
                        if( bFolderFlag )
                        {
                            FileStatus::Type aType = aFileStatus.getFileType();
                            bool bFolder = isFolder( aType );
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

            // #57064 extract the real-path for virtual URLs
            rtl::OString aByteStrFullPath(rtl::OUStringToOString(aEntry.GetFull(),
                osl_getThreadTextEncoding()));
            DWORD nRealFlags = GetFileAttributes (aByteStrFullPath.getStr());
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
                    catch(const Exception & ) {}
                    if( !bExists )
                    {
                        StarBASIC::Error( SbERR_FILE_NOT_FOUND );
                        return;
                    }

                    sal_Bool bReadOnly = xSFI->isReadOnly( aPath );
                    sal_Bool bHidden = xSFI->isHidden( aPath );
                    sal_Bool bDirectory = xSFI->isFolder( aPath );
                    if( bReadOnly )
                        nFlags |= Sb_ATTR_READONLY;
                    if( bHidden )
                        nFlags |= Sb_ATTR_HIDDEN;
                    if( bDirectory )
                        nFlags |= Sb_ATTR_DIRECTORY;
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            DirectoryItem aItem;
            DirectoryItem::get( getFullPathUNC( rPar.Get(1)->GetString() ), aItem );
            FileStatus aFileStatus( osl_FileStatus_Mask_Attributes | osl_FileStatus_Mask_Type );
            aItem.getFileStatus( aFileStatus );
            sal_uInt64 nAttributes = aFileStatus.getAttributes();
            bool bReadOnly = (nAttributes & osl_File_Attribute_ReadOnly) != 0;

            FileStatus::Type aType = aFileStatus.getFileType();
            bool bDirectory = isFolder( aType );
            if( bReadOnly )
                nFlags |= Sb_ATTR_READONLY;
            if( bDirectory )
                nFlags |= Sb_ATTR_DIRECTORY;
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
        String aPath = rPar.Get(1)->GetString();
        Time aTime( Time::EMPTY );
        Date aDate( Date::EMPTY );
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
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            DirectoryItem aItem;
            DirectoryItem::get( getFullPathUNC( aPath ), aItem );
            FileStatus aFileStatus( osl_FileStatus_Mask_ModifyTime );
            aItem.getFileStatus( aFileStatus );
            TimeValue aTimeVal = aFileStatus.getModifyTime();
            oslDateTime aDT;
            osl_getDateTimeFromTimeValue( &aTimeVal, &aDT );

            aTime = Time( aDT.Hours, aDT.Minutes, aDT.Seconds, 10000000*aDT.NanoSeconds );
            aDate = Date( aDT.Day, aDT.Month, aDT.Year );
        }

        double fSerial = (double)GetDayDiff( aDate );
        long nSeconds = aTime.GetHour();
        nSeconds *= 3600;
        nSeconds += aTime.GetMin() * 60;
        nSeconds += aTime.GetSec();
        double nDays = ((double)nSeconds) / (double)(24.0*3600.0);
        fSerial += nDays;

        Color* pCol;

        SvNumberFormatter* pFormatter = NULL;
        sal_uInt32 nIndex;
        if( GetSbData()->pInst )
        {
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
            nIndex = GetSbData()->pInst->GetStdDateTimeIdx();
        }
        else
        {
            sal_uInt32 n;
            SbiInstance::PrepareNumberFormatter( pFormatter, n, n, nIndex );
        }

        String aRes;
        pFormatter->GetOutputString( fSerial, nIndex, aRes, &pCol );
        rPar.Get(0)->PutString( aRes );

        if( !GetSbData()->pInst )
            delete pFormatter;
    }
}


RTLFUNC(EOF)
{
    (void)pBasic;
    (void)bWrite;

    // No changes for UCB
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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
            (*pSvStrm) >> cBla; // can we read another character?
            bIsEof = pSvStrm->IsEof();
            if ( !bIsEof )
                pSvStrm->SeekRel( -1 );
        }
        else
            bIsEof = pSvStrm->IsEof();  // for binary data!
        rPar.Get(0)->PutBool( bIsEof );
    }
}

RTLFUNC(FileAttr)
{
    (void)pBasic;
    (void)bWrite;

    // No changes for UCB
    // #57064 Although this function doesn't operate with DirEntry, it is
    // not touched by the adjustment to virtual URLs, as it only works on
    // already opened files and the name doesn't matter there.

    if ( rPar.Count() != 3 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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

    // No changes for UCB
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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
            nPos++; // block positions starting at 1
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

    // No changes for UCB
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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

    // No changes for UCB
    int nArgs = (int)rPar.Count();
    if ( nArgs < 2 || nArgs > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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
        nPos++; // Basic counts from 1
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
        nPos--; // Basic counts from 1, SvStreams count from 0
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
        nRand = ( nRand / ((double)RAND_MAX + 1.0));
        rPar.Get(0)->PutDouble( nRand );
    }
}


//  Syntax: Shell("Path",[ Window-Style,[ "Params", [ bSync = sal_False ]]])
//  WindowStyles (VBA-kompatibel):
//      2 == Minimized
//      3 == Maximized
//     10 == Full-Screen (text mode applications OS/2, WIN95, WNT)
//     HACK: The WindowStyle will be passed to
//     Application::StartApp in Creator. Format: "xxxx2"


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
        oslProcessOption nOptions = osl_Process_SEARCHPATH | osl_Process_DETACHED;

        String aCmdLine = rPar.Get(1)->GetString();
        // attach additional parameters - everything must be parsed anyway
        if( nArgCount >= 4 )
        {
            aCmdLine.AppendAscii( " " );
            aCmdLine += rPar.Get(3)->GetString();
        }
        else if( !aCmdLine.Len() )
        {
            // avaoid special treatment (empty list)
            aCmdLine.AppendAscii( " " );
        }
        sal_uInt16 nLen = aCmdLine.Len();

        // #55735 if there are parameters, they have to be seperated
        // #72471 also seperate the single parameters
        std::list<String> aTokenList;
        String aToken;
        sal_uInt16 i = 0;
        sal_Unicode c;
        while( i < nLen )
        {
            for ( ;; ++i )
            {
                c = aCmdLine.GetBuffer()[ i ];
                if ( c != ' ' && c != '\t' )
                    break;
            }

            if( c == '\"' || c == '\'' )
            {
                sal_uInt16 iFoundPos = aCmdLine.Search( c, i + 1 );

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

            // insert into the list
            aTokenList.push_back( aToken );
        }
        // #55735 / #72471 end

        sal_Int16 nWinStyle = 0;
        if( nArgCount >= 3 )
        {
            nWinStyle = rPar.Get(2)->GetInteger();
            switch( nWinStyle )
            {
            case 2:
                nOptions |= osl_Process_MINIMIZED;
                break;
            case 3:
                nOptions |= osl_Process_MAXIMIZED;
                break;
            case 10:
                nOptions |= osl_Process_FULLSCREEN;
                break;
            }

            sal_Bool bSync = sal_False;
            if( nArgCount >= 5 )
                bSync = rPar.Get(4)->GetBool();
            if( bSync )
                nOptions |= osl_Process_WAIT;
        }

        // #72471 work parameter(s) up
        std::list<String>::const_iterator iter = aTokenList.begin();
        const String& rStr = *iter;
        ::rtl::OUString aOUStrProg( rStr.GetBuffer(), rStr.Len() );
        ::rtl::OUString aOUStrProgUNC = getFullPathUNC( aOUStrProg );

        ++iter;

        sal_uInt16 nParamCount = sal::static_int_cast< sal_uInt16 >(
            aTokenList.size() - 1 );
        rtl_uString** pParamList = NULL;
        if( nParamCount )
        {
            pParamList = new rtl_uString*[nParamCount];
            for(int iList = 0; iter != aTokenList.end(); ++iList, ++iter)
            {
                const String& rParamStr = (*iter);
                const ::rtl::OUString aTempStr( rParamStr.GetBuffer(), rParamStr.Len());
                pParamList[iList] = NULL;
                rtl_uString_assign(&(pParamList[iList]), aTempStr.pData);
            }
        }

        oslProcess pApp;
        sal_Bool bSucc = osl_executeProcess(
                    aOUStrProgUNC.pData,
                    pParamList,
                    nParamCount,
                    nOptions,
                    NULL,
                    NULL,
                    NULL, 0,
                    &pApp ) == osl_Process_E_None;

        osl_freeProcessHandle( pApp );

        for(int j = 0; i < nParamCount; i++)
        {
            rtl_uString_release(pParamList[j]);
            pParamList[j] = NULL;
        }

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
rtl::OUString getBasicTypeName( SbxDataType eType )
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
    return rtl::OUString::createFromAscii(pTypeNames[nPos]);
}

String getObjectTypeName( SbxVariable* pVar )
{
    rtl::OUString sRet( "Object" );
    if ( pVar )
    {
        SbxBase* pObj = pVar->GetObject();
        if( !pObj )
           sRet = String( RTL_CONSTASCII_USTRINGPARAM("Nothing") );
        else
        {
            SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pVar );
            if ( !pUnoObj )
            {
                if ( SbxBase* pBaseObj = pVar->GetObject() )
                    pUnoObj = PTR_CAST(SbUnoObject, pBaseObj );
            }
            if ( pUnoObj )
            {
                Any aObj = pUnoObj->getUnoAny();
                // For upstreaming unless we start to build oovbaapi by default
                // we need to get detect the vba-ness of the object in some
                // other way
                // note: Automation objects do not support XServiceInfo
                Reference< XServiceInfo > xServInfo( aObj, UNO_QUERY );
                if ( xServInfo.is() )
                {
                    // is this a VBA object ?
                    Reference< ooo::vba::XHelperInterface > xVBA( aObj, UNO_QUERY );
                    Sequence< rtl::OUString > sServices = xServInfo->getSupportedServiceNames();
                    if ( sServices.getLength() )
                        sRet = sServices[ 0 ];
                }
                else
                {
                    Reference< com::sun::star::bridge::oleautomation::XAutomationObject > xAutoMation( aObj, UNO_QUERY );
                    if ( xAutoMation.is() )
                    {
                        Reference< XInvocation > xInv( aObj, UNO_QUERY );
                        if ( xInv.is() )
                        {
                            try
                            {
                                xInv->getValue( rtl::OUString( "$GetTypeName" ) ) >>= sRet;
                            }
                            catch(const Exception& )
                            {
                            }
                        }
                    }
                }
                sal_Int32 nDot = sRet.lastIndexOf( '.' );
                if ( nDot != -1 && nDot < sRet.getLength() )
                       sRet = sRet.copy( nDot + 1 );
            }
        }
    }
    return sRet;
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
        bool bIsArray = ( ( eType & SbxARRAY ) != 0 );

        String aRetStr;
        if ( SbiRuntime::isVBAEnabled() && eType == SbxOBJECT )
            aRetStr = getObjectTypeName( rPar.Get(1) );
        else
            aRetStr = getBasicTypeName( eType );
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

    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
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
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
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

    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
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
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
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
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
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
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
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
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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

    SbiInstance* pInst = GetSbData()->pInst;
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
        const CharClass& rCharClass = GetCharClass();
        aOldStr = rCharClass.titlecase( aOldStr.ToLowerAscii(), 0, nOldLen );
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
            pChar[i] = static_cast< sal_Char >( (i%2) ? ((*pSrc) >> 8) & 0xff : (*pSrc) & 0xff );
            if( i%2 )
                pSrc++;
        }
        pChar[nSize] = '\0';
        ::rtl::OString aOStr(pChar);
        delete[] pChar;

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
        *pStream >> aBmp;
        Graphic aGraphic( aBmp );

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
    nStyle &= 15; // delete bits 4-16
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
        aTitle = GetpApp()->GetAppName();

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
            pBox = new InfoBox( pParent, aMsg );
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

RTLFUNC(SetAttr)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if ( rPar.Count() == 3 )
    {
        String aStr = rPar.Get(1)->GetString();
        sal_Int16 nFlags = rPar.Get(2)->GetInteger();

        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    sal_Bool bReadOnly = (nFlags & Sb_ATTR_READONLY) != 0;
                    xSFI->setReadOnly( aStr, bReadOnly );
                    sal_Bool bHidden   = (nFlags & Sb_ATTR_HIDDEN) != 0;
                    xSFI->setHidden( aStr, bHidden );
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(Reset)
{
    (void)pBasic;
    (void)bWrite;
    (void)rPar;

    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
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

        if( hasUno() )
        {
            com::sun::star::uno::Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    bExists = xSFI->exists( aStr );
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_IO_GENERAL );
                }
            }
        }
        else
        {
            DirectoryItem aItem;
            FileBase::RC nRet = DirectoryItem::get( getFullPathUNC( aStr ), aItem );
            bExists = (nRet == FileBase::E_None);
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

#endif

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
    nDiffDays += 2; // adjustment VisualBasic: 1.Jan.1900 == 2
    return nDiffDays;
}

sal_Int16 implGetDateYear( double aDate )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long) aDate;
    nDays -= 2; // standardize: 1.1.1900 => 0.0
    aRefDate += nDays;
    sal_Int16 nRet = (sal_Int16)( aRefDate.GetYear() );
    return nRet;
}

bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet )
{
#ifndef DISABLE_SCRIPTING
    if ( nYear < 30 && SbiRuntime::isVBAEnabled() )
        nYear += 2000;
    else
#endif
        if ( nYear < 100 )
            nYear += 1900;
    Date aCurDate( nDay, nMonth, nYear );
    if ((nYear < 100 || nYear > 9999) )
    {
#ifndef DISABLE_SCRIPTING
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
#endif
        return false;
    }

#ifndef DISABLE_SCRIPTING
    if ( !SbiRuntime::isVBAEnabled() )
#endif
    {
        if ( (nMonth < 1 || nMonth > 12 )||
        (nDay < 1 || nDay > 31 ) )
        {
#ifndef DISABLE_SCRIPTING
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
#endif
            return false;
        }
    }
#ifndef DISABLE_SCRIPTING
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
#endif

    long nDiffDays = GetDayDiff( aCurDate );
    rdRet = (double)nDiffDays;
    return true;
}

sal_Int16 implGetMinute( double dDate )
{
    if( dDate < 0.0 )
        dDate *= -1.0;
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = (sal_Int32)(nFrac + 0.5);
    sal_Int16 nTemp = (sal_Int16)(nSeconds % 3600);
    sal_Int16 nMin = nTemp / 60;
    return nMin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
