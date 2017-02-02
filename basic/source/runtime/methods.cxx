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

#include <config_features.h>

#include <tools/date.hxx>
#include <basic/sbxvar.hxx>
#include <basic/sbuno.hxx>
#include <osl/process.h>
#include <vcl/dibtools.hxx>
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
#include <i18nlangtag/lang.h>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include "runtime.hxx"
#include "sbunoobj.hxx"
#include <osl/file.hxx>
#include "errobject.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/script/XErrorQuery.hpp>
#include <ooo/vba/XHelperInterface.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <memory>
#include <random>
#include <o3tl/make_unique.hxx>

using namespace comphelper;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

#include "date.hxx"
#include "stdobj.hxx"
#include "sbstdobj.hxx"
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
#include <errno.h>

#include "sbobjmod.hxx"
#include "sbxmod.hxx"

#ifdef _WIN32
#include <prewin.h>
#include <direct.h>
#include <io.h>
#include <postwin.h>
#else
#include <unistd.h>
#endif

#if HAVE_FEATURE_SCRIPTING

static void FilterWhiteSpace( OUString& rStr )
{
    if (rStr.isEmpty())
    {
        return;
    }
    OUStringBuffer aRet;

    for (sal_Int32 i = 0; i < rStr.getLength(); ++i)
    {
        sal_Unicode cChar = rStr[i];
        if ((cChar != ' ') && (cChar != '\t') &&
           (cChar != '\n') && (cChar != '\r'))
        {
            aRet.append(cChar);
        }
    }

    rStr = aRet.makeStringAndClear();
}

static long GetDayDiff( const Date& rDate );

static const CharClass& GetCharClass()
{
    static bool bNeedsInit = true;
    static LanguageTag aLanguageTag( LANGUAGE_SYSTEM);
    if( bNeedsInit )
    {
        bNeedsInit = false;
        aLanguageTag = Application::GetSettings().GetLanguageTag();
    }
    static CharClass aCharClass( aLanguageTag );
    return aCharClass;
}

static inline bool isFolder( FileStatus::Type aType )
{
    return ( aType == FileStatus::Directory || aType == FileStatus::Volume );
}


//*** UCB file access ***

// Converts possibly relative paths to absolute paths
// according to the setting done by ChDir/ChDrive
OUString getFullPath( const OUString& aRelPath )
{
    OUString aFileURL;

    // #80204 Try first if it already is a valid URL
    INetURLObject aURLObj( aRelPath );
    aFileURL = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if( aFileURL.isEmpty() )
    {
        File::getFileURLFromSystemPath( aRelPath, aFileURL );
    }

    return aFileURL;
}

// TODO: -> SbiGlobals
static uno::Reference< ucb::XSimpleFileAccess3 > const & getFileAccess()
{
    static uno::Reference< ucb::XSimpleFileAccess3 > xSFI;
    if( !xSFI.is() )
    {
        xSFI = ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() );
    }
    return xSFI;
}


// Properties and methods lie down the return value at the Get (bPut = sal_False) in the
// element 0 of the Argv; the value of element 0 is saved at Put (bPut = sal_True)

// CreateObject( class )

RTLFUNC(CreateObject)
{
    (void)bWrite;

    OUString aClass( rPar.Get( 1 )->GetOUString() );
    SbxObjectRef p = SbxBase::CreateObject( aClass );
    if( !p.is() )
        StarBASIC::Error( ERRCODE_BASIC_CANNOT_LOAD );
    else
    {
        // Convenience: enter BASIC as parent
        p->SetParent( pBasic );
        rPar.Get( 0 )->PutObject( p.get() );
    }
}

// Error( n )

RTLFUNC(Error)
{
    (void)bWrite;

    if( !pBasic )
        StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
    else
    {
        OUString aErrorMsg;
        SbError nErr = 0;
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
            {
                StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
            }
            else
            {
                nErr = StarBASIC::GetSfxFromVBError( (sal_uInt16)nCode );
            }
        }

        bool bVBA = SbiRuntime::isVBAEnabled();
        OUString tmpErrMsg;
        if( bVBA && !aErrorMsg.isEmpty())
        {
            tmpErrMsg = aErrorMsg;
        }
        else
        {
            StarBASIC::MakeErrorText( nErr, aErrorMsg );
            tmpErrMsg = StarBASIC::GetErrorText();
        }
        // If this rtlfunc 'Error'  passed a errcode the same as the active Err Objects's
        // current err then  return the description for the error message if it is set
        // ( complicated isn't it ? )
        if ( bVBA && rPar.Count() > 1 )
        {
            uno::Reference< ooo::vba::XErrObject > xErrObj( SbxErrObject::getUnoErrObject() );
            if ( xErrObj.is() && xErrObj->getNumber() == nCode && !xErrObj->getDescription().isEmpty() )
            {
                tmpErrMsg = xErrObj->getDescription();
            }
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        OUString aStr( pArg->GetOUString() );
        if ( aStr.isEmpty())
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
            rPar.Get(0)->PutEmpty();
        }
        else
        {
            sal_Unicode aCh = aStr[0];
            rPar.Get(0)->PutLong( aCh );
        }
    }
}

void implChr( SbxArray& rPar, bool bChrW )
{
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );

        OUString aStr;
        if( !bChrW && SbiRuntime::isVBAEnabled() )
        {
            sal_Char c = static_cast<sal_Char>(pArg->GetByte());
            aStr = OUString(&c, 1, osl_getThreadTextEncoding());
        }
        else
        {
            sal_Unicode aCh = static_cast<sal_Unicode>(pArg->GetUShort());
            aStr = OUString(aCh);
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

RTLFUNC(CurDir)
{
    (void)pBasic;
    (void)bWrite;

    // #57064 Although this function doesn't work with DirEntry, it isn't touched
    // by the adjustment to virtual URLs, as, using the DirEntry-functionality,
    // there's no possibility to detect the current one in a way that a virtual URL
    // could be delivered.

#if defined(_WIN32)
    int nCurDir = 0;  // Current dir // JSM
    if ( rPar.Count() == 2 )
    {
        OUString aDrive = rPar.Get(1)->GetOUString();
        if ( aDrive.getLength() != 1 )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
            return;
        }
        else
        {
            nCurDir = (int)aDrive[0];
            if ( !isalpha( nCurDir ) )
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                return;
            }
            else
            {
                nCurDir -= ( 'A' - 1 );
            }
        }
    }
    char pBuffer[ _MAX_PATH ];
    if ( _getdcwd( nCurDir, pBuffer, _MAX_PATH ) != nullptr )
    {
        rPar.Get(0)->PutString( OUString::createFromAscii( pBuffer ) );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_NO_DEVICE );
    }

#else

    const int PATH_INCR = 250;

    int nSize = PATH_INCR;
    std::unique_ptr<char[]> pMem;
    while( true )
      {
        pMem.reset(new char[nSize]);
        if( !pMem )
          {
            StarBASIC::Error( ERRCODE_BASIC_NO_MEMORY );
            return;
          }
        if( getcwd( pMem.get(), nSize-1 ) != nullptr )
          {
            rPar.Get(0)->PutString( OUString::createFromAscii(pMem.get()) );
            return;
          }
        if( errno != ERANGE )
          {
            StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
            return;
          }
        nSize += PATH_INCR;
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
        {
            ::basic::vba::registerCurrentDirectory( getDocumentModel( pBasic ), rPar.Get(1)->GetOUString() );
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

RTLFUNC(ChDrive)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}


// Implementation of StepRENAME with UCB
void implStepRenameUCB( const OUString& aSource, const OUString& aDest )
{
    uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
    if( xSFI.is() )
    {
        try
        {
            OUString aSourceFullPath = getFullPath( aSource );
            if( !xSFI->exists( aSourceFullPath ) )
            {
                StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
                return;
            }

            OUString aDestFullPath = getFullPath( aDest );
            if( xSFI->exists( aDestFullPath ) )
            {
                StarBASIC::Error( ERRCODE_BASIC_FILE_EXISTS );
            }
            else
            {
                xSFI->move( aSourceFullPath, aDestFullPath );
            }
        }
        catch(const Exception & )
        {
            StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
        }
    }
}

// Implementation of StepRENAME with OSL
void implStepRenameOSL( const OUString& aSource, const OUString& aDest )
{
    FileBase::RC nRet = File::move( getFullPath( aSource ), getFullPath( aDest ) );
    if( nRet != FileBase::E_None )
    {
        StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
    }
}

RTLFUNC(FileCopy)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 3)
    {
        OUString aSource = rPar.Get(1)->GetOUString();
        OUString aDest = rPar.Get(2)->GetOUString();
        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    xSFI->copy( getFullPath( aSource ), getFullPath( aDest ) );
                }
                catch(const Exception & )
                {
                    StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
                }
            }
        }
        else
        {
            FileBase::RC nRet = File::copy( getFullPath( aSource ), getFullPath( aDest ) );
            if( nRet != FileBase::E_None )
            {
                StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
            }
        }
    }
    else
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
}

RTLFUNC(Kill)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        OUString aFileSpec = rPar.Get(1)->GetOUString();

        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                OUString aFullPath = getFullPath( aFileSpec );
                if( !xSFI->exists( aFullPath ) || xSFI->isFolder( aFullPath ) )
                {
                    StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
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
            File::remove( getFullPath( aFileSpec ) );
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

RTLFUNC(MkDir)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        OUString aPath = rPar.Get(1)->GetOUString();
        if ( SbiRuntime::isVBAEnabled() )
        {
            // In vba if the full path is not specified then
            // folder is created relative to the curdir
            INetURLObject aURLObj( getFullPath( aPath ) );
            if ( aURLObj.GetProtocol() != INetProtocol::File )
            {
                SbxArrayRef pPar = new SbxArray();
                SbxVariableRef pResult = new SbxVariable();
                SbxVariableRef pParam = new SbxVariable();
                pPar->Insert( pResult.get(), pPar->Count() );
                pPar->Insert( pParam.get(), pPar->Count() );
                SbRtl_CurDir( pBasic, *pPar, bWrite );

                rtl::OUString sCurPathURL;
                File::getFileURLFromSystemPath( pPar->Get(0)->GetOUString(), sCurPathURL );

                aURLObj.SetURL( sCurPathURL );
                aURLObj.Append( aPath );
                File::getSystemPathFromFileURL(aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri  ),aPath ) ;
            }
        }

        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
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
            Directory::create( getFullPath( aPath ) );
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}


// In OSL only empty directories can be deleted
// so we have to delete all files recursively
void implRemoveDirRecursive( const OUString& aDirPath )
{
    DirectoryItem aItem;
    FileBase::RC nRet = DirectoryItem::get( aDirPath, aItem );
    bool bExists = (nRet == FileBase::E_None);

    FileStatus aFileStatus( osl_FileStatus_Mask_Type );
    nRet = aItem.getFileStatus( aFileStatus );
    bool bFolder = nRet == FileBase::E_None
        && isFolder( aFileStatus.getFileType() );

    if( !bExists || !bFolder )
    {
        StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
        return;
    }

    Directory aDir( aDirPath );
    nRet = aDir.open();
    if( nRet != FileBase::E_None )
    {
        StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
        return;
    }

    for( ;; )
    {
        DirectoryItem aItem2;
        nRet = aDir.getNextItem( aItem2 );
        if( nRet != FileBase::E_None )
        {
            break;
        }
        // Handle flags
        FileStatus aFileStatus2( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL );
        nRet = aItem2.getFileStatus( aFileStatus2 );
        if( nRet != FileBase::E_None )
        {
            SAL_WARN("basic", "getFileStatus failed");
            continue;
        }
        OUString aPath = aFileStatus2.getFileURL();

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
    aDir.close();

    Directory::remove( aDirPath );
}


RTLFUNC(RmDir)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        OUString aPath = rPar.Get(1)->GetOUString();
        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    if( !xSFI->isFolder( aPath ) )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
                        return;
                    }
                    SbiInstance* pInst = GetSbData()->pInst;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    if( bCompatibility )
                    {
                        Sequence< OUString > aContent = xSFI->getFolderContents( aPath, true );
                        sal_Int32 nCount = aContent.getLength();
                        if( nCount > 0 )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_ACCESS_ERROR );
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
            implRemoveDirRecursive( getFullPath( aPath ) );
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

RTLFUNC(SendKeys)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(ERRCODE_BASIC_NOT_IMPLEMENTED);
}

RTLFUNC(Exp)
{
    (void)pBasic;
    (void)bWrite;

    if( rPar.Count() < 2 )
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        OUString aStr( pArg->GetOUString() );
        sal_Int32 nLen = 0;
        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
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
            DirectoryItem::get( getFullPath( aStr ), aItem );
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariableRef pArg = rPar.Get( 1 );
        // converting value to unsigned and limit to 2 or 4 byte representation
        sal_uInt32 nVal = pArg->IsInteger() ?
            static_cast<sal_uInt16>(pArg->GetInteger()) :
            static_cast<sal_uInt32>(pArg->GetLong());
        OUString aStr(OUString::number( nVal, 16 ));
        aStr = aStr.toAsciiUpperCase();
        rPar.Get(0)->PutString( aStr );
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
        StarBASIC::Error( ERRCODE_BASIC_NOT_IMPLEMENTED );
    }

}
// InStr( [start],string,string,[compare] )

RTLFUNC(InStr)
{
    (void)pBasic;
    (void)bWrite;

    std::size_t nArgCount = rPar.Count()-1;
    if ( nArgCount < 2 )
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    else
    {
        sal_Int32 nStartPos = 1;
        sal_Int32 nFirstStringPos = 1;

        if ( nArgCount >= 3 )
        {
            nStartPos = rPar.Get(1)->GetLong();
            if( nStartPos <= 0 )
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                nStartPos = 1;
            }
            nFirstStringPos++;
        }

        SbiInstance* pInst = GetSbData()->pInst;
        bool bTextMode;
        bool bCompatibility = ( pInst && pInst->IsCompatibility() );
        if( bCompatibility )
        {
            SbiRuntime* pRT = pInst->pRun;
            bTextMode = pRT && pRT->IsImageFlag( SbiImageFlags::COMPARETEXT );
        }
        else
        {
            bTextMode = true;
        }
        if ( nArgCount == 4 )
        {
            bTextMode = rPar.Get(4)->GetInteger();
        }
        sal_Int32 nPos;
        const OUString& rToken = rPar.Get(nFirstStringPos+1)->GetOUString();

        // #97545 Always find empty string
        if( rToken.isEmpty() )
        {
            nPos = nStartPos;
        }
        else
        {
            if( !bTextMode )
            {
                const OUString& rStr1 = rPar.Get(nFirstStringPos)->GetOUString();
                nPos = rStr1.indexOf( rToken, nStartPos - 1 ) + 1;
            }
            else
            {
                OUString aStr1 = rPar.Get(nFirstStringPos)->GetOUString();
                OUString aToken = rToken;

                aStr1 = aStr1.toAsciiUpperCase();
                aToken = aToken.toAsciiUpperCase();

                nPos = aStr1.indexOf( aToken, nStartPos-1 ) + 1;
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

    std::size_t nArgCount = rPar.Count()-1;
    if ( nArgCount < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr1 = rPar.Get(1)->GetOUString();
        OUString aToken = rPar.Get(2)->GetOUString();

        sal_Int32 nStartPos = -1;
        if ( nArgCount >= 3 )
        {
            nStartPos = rPar.Get(3)->GetLong();
            if( (nStartPos <= 0 && nStartPos != -1))
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                nStartPos = -1;
            }
        }

        SbiInstance* pInst = GetSbData()->pInst;
        bool bTextMode;
        bool bCompatibility = ( pInst && pInst->IsCompatibility() );
        if( bCompatibility )
        {
            SbiRuntime* pRT = pInst->pRun;
            bTextMode = pRT && pRT->IsImageFlag( SbiImageFlags::COMPARETEXT );
        }
        else
        {
            bTextMode = true;
        }
        if ( nArgCount == 4 )
        {
            bTextMode = rPar.Get(4)->GetInteger();
        }
        sal_Int32 nStrLen = aStr1.getLength();
        if( nStartPos == -1 )
        {
            nStartPos = nStrLen;
        }

        sal_Int32 nPos = 0;
        if( nStartPos <= nStrLen )
        {
            sal_Int32 nTokenLen = aToken.getLength();
            if( !nTokenLen )
            {
                // Always find empty string
                nPos = nStartPos;
            }
            else if( nStrLen > 0 )
            {
                if( !bTextMode )
                {
                    nPos = aStr1.lastIndexOf( aToken, nStartPos ) + 1;
                }
                else
                {
                    aStr1 = aStr1.toAsciiUpperCase();
                    aToken = aToken.toAsciiUpperCase();

                    nPos = aStr1.lastIndexOf( aToken, nStartPos ) + 1;
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const CharClass& rCharClass = GetCharClass();
        OUString aStr( rPar.Get(1)->GetOUString() );
        aStr = rCharClass.lowercase(aStr);
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Left)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr( rPar.Get(1)->GetOUString() );
        sal_Int32 nResultLen = rPar.Get(2)->GetLong();
        if( nResultLen < 0 )
        {
            nResultLen = 0;
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
        else if(nResultLen > aStr.getLength())
        {
            nResultLen = aStr.getLength();
        }
        aStr = aStr.copy(0, nResultLen );
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(Log)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
    }
}

RTLFUNC(LTrim)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr(comphelper::string::stripStart(rPar.Get(1)->GetOUString(), ' '));
        rPar.Get(0)->PutString(aStr);
    }
}


// Mid( String, nStart, nLength )

RTLFUNC(Mid)
{
    (void)pBasic;
    (void)bWrite;

    int nArgCount = rPar.Count()-1;
    if ( nArgCount < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // #23178: replicate the functionality of Mid$ as a command
        // by adding a replacement-string as a fourth parameter.
        // In contrast to the original the third parameter (nLength)
        // can't be left out here. That's considered in bWrite already.
        if( nArgCount == 4 )
        {
            bWrite = true;
        }
        OUString aArgStr = rPar.Get(1)->GetOUString();
        sal_Int32 nStartPos = rPar.Get(2)->GetLong();
        if ( nStartPos < 1 )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
        else
        {
            nStartPos--;
            sal_Int32 nLen = -1;
            bool bWriteNoLenParam = false;
            if ( nArgCount == 3 || bWrite )
            {
                sal_Int32 n = rPar.Get(3)->GetLong();
                if( bWrite && n == -1 )
                {
                    bWriteNoLenParam = true;
                }
                nLen = n;
            }
            if ( bWrite )
            {
                OUStringBuffer aResultStr;
                SbiInstance* pInst = GetSbData()->pInst;
                bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                if( bCompatibility )
                {
                    sal_Int32 nArgLen = aArgStr.getLength();
                    if( nStartPos + 1 > nArgLen )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                        return;
                    }

                    OUString aReplaceStr = rPar.Get(4)->GetOUString();
                    sal_Int32 nReplaceStrLen = aReplaceStr.getLength();
                    sal_Int32 nReplaceLen;
                    if( bWriteNoLenParam )
                    {
                        nReplaceLen = nReplaceStrLen;
                    }
                    else
                    {
                        nReplaceLen = nLen;
                        if( nReplaceLen < 0 || nReplaceLen > nReplaceStrLen )
                        {
                            nReplaceLen = nReplaceStrLen;
                        }
                    }

                    sal_Int32 nReplaceEndPos = nStartPos + nReplaceLen;
                    if( nReplaceEndPos > nArgLen )
                    {
                        nReplaceLen -= (nReplaceEndPos - nArgLen);
                    }
                    aResultStr = aArgStr;
                    sal_Int32 nErase = nReplaceLen;
                    aResultStr.remove( nStartPos, nErase );
                    aResultStr.insert( nStartPos, aReplaceStr.getStr(), nReplaceLen);
                }
                else
                {
                    aResultStr = aArgStr;
                    sal_Int32 nTmpStartPos = nStartPos;
                    if ( nTmpStartPos > aArgStr.getLength() )
                        nTmpStartPos =  aArgStr.getLength();
                    else
                        aResultStr.remove( nTmpStartPos, nLen );
                    aResultStr.insert( nTmpStartPos, rPar.Get(4)->GetOUString().getStr(), std::min(nLen, rPar.Get(4)->GetOUString().getLength()));
                }

                rPar.Get(1)->PutString( aResultStr.makeStringAndClear() );
            }
            else
            {
                OUString aResultStr;
                if (nStartPos > aArgStr.getLength())
                {
                    // do nothing
                }
                else if(nArgCount == 2)
                {
                    aResultStr = aArgStr.copy( nStartPos);
                }
                else
                {
                    if (nLen < 0)
                        nLen = 0;
                    if(nStartPos + nLen > aArgStr.getLength())
                    {
                        nLen = aArgStr.getLength() - nStartPos;
                    }
                    if (nLen > 0)
                        aResultStr = aArgStr.copy( nStartPos, nLen );
                }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        char aBuffer[16];
        SbxVariableRef pArg = rPar.Get( 1 );
        if ( pArg->IsInteger() )
        {
            snprintf( aBuffer, sizeof(aBuffer), "%o", pArg->GetInteger() );
        }
        else
        {
            snprintf( aBuffer, sizeof(aBuffer), "%lo", static_cast<long unsigned int>(pArg->GetLong()) );
        }
        rPar.Get(0)->PutString( OUString::createFromAscii( aBuffer ) );
    }
}

// Replace(expression, find, replace[, start[, count[, compare]]])

RTLFUNC(Replace)
{
    (void)pBasic;
    (void)bWrite;

    std::size_t nArgCount = rPar.Count()-1;
    if ( nArgCount < 3 || nArgCount > 6 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aExpStr = rPar.Get(1)->GetOUString();
        OUString aFindStr = rPar.Get(2)->GetOUString();
        OUString aReplaceStr = rPar.Get(3)->GetOUString();

        sal_Int32 lStartPos = 1;
        if ( nArgCount >= 4 )
        {
            if( rPar.Get(4)->GetType() != SbxEMPTY )
            {
                lStartPos = rPar.Get(4)->GetLong();
            }
            if( lStartPos < 1)
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                lStartPos = 1;
            }
        }

        sal_Int32 lCount = -1;
        if( nArgCount >=5 )
        {
            if( rPar.Get(5)->GetType() != SbxEMPTY )
            {
                lCount = rPar.Get(5)->GetLong();
            }
            if( lCount < -1)
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                lCount = -1;
            }
        }

        SbiInstance* pInst = GetSbData()->pInst;
        bool bTextMode;
        bool bCompatibility = ( pInst && pInst->IsCompatibility() );
        if( bCompatibility )
        {
            SbiRuntime* pRT = pInst->pRun;
            bTextMode = pRT && pRT->IsImageFlag( SbiImageFlags::COMPARETEXT );
        }
        else
        {
            bTextMode = true;
        }
        if ( nArgCount == 6 )
        {
            bTextMode = rPar.Get(6)->GetInteger();
        }
        sal_Int32 nExpStrLen = aExpStr.getLength();
        sal_Int32 nFindStrLen = aFindStr.getLength();
        sal_Int32 nReplaceStrLen = aReplaceStr.getLength();

        if( lStartPos <= nExpStrLen )
        {
            sal_Int32 nPos = lStartPos - 1;
            sal_Int32 nCounts = 0;
            while( lCount == -1 || lCount > nCounts )
            {
                OUString aSrcStr( aExpStr );
                if( bTextMode )
                {
                    aSrcStr = aSrcStr.toAsciiUpperCase();
                    aFindStr = aFindStr.toAsciiUpperCase();
                }
                nPos = aSrcStr.indexOf( aFindStr, nPos );
                if( nPos >= 0 )
                {
                    aExpStr = aExpStr.replaceAt( nPos, nFindStrLen, aReplaceStr );
                    nPos = nPos + nReplaceStrLen;
                    nCounts++;
                }
                else
                {
                    break;
                }
            }
        }
        rPar.Get(0)->PutString( aExpStr.copy( lStartPos - 1 )  );
    }
}

RTLFUNC(Right)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const OUString& rStr = rPar.Get(1)->GetOUString();
        int nResultLen = rPar.Get(2)->GetLong();
        if( nResultLen < 0 )
        {
            nResultLen = 0;
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
        int nStrLen = rStr.getLength();
        if ( nResultLen > nStrLen )
        {
            nResultLen = nStrLen;
        }
        OUString aResultStr = rStr.copy( nStrLen - nResultLen );
        rPar.Get(0)->PutString( aResultStr );
    }
}

RTLFUNC(RTL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get( 0 )->PutObject( pBasic->getRTL().get() );
}

RTLFUNC(RTrim)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr(comphelper::string::stripEnd(rPar.Get(1)->GetOUString(), ' '));
        rPar.Get(0)->PutString(aStr);
    }
}

RTLFUNC(Sgn)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double aDouble = rPar.Get(1)->GetDouble();
        sal_Int16 nResult = 0;
        if ( aDouble > 0 )
        {
            nResult = 1;
        }
        else if ( aDouble < 0 )
        {
            nResult = -1;
        }
        rPar.Get(0)->PutInteger( nResult );
    }
}

RTLFUNC(Space)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUStringBuffer aBuf;
        string::padToLength(aBuf, rPar.Get(1)->GetLong(), ' ');
        rPar.Get(0)->PutString(aBuf.makeStringAndClear());
    }
}

RTLFUNC(Spc)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUStringBuffer aBuf;
        string::padToLength(aBuf, rPar.Get(1)->GetLong(), ' ');
        rPar.Get(0)->PutString(aBuf.makeStringAndClear());
    }
}

RTLFUNC(Sqr)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double aDouble = rPar.Get(1)->GetDouble();
        if ( aDouble >= 0 )
        {
            rPar.Get(0)->PutDouble( sqrt( aDouble ));
        }
        else
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
    }
}

RTLFUNC(Str)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr;
        OUString aStrNew("");
        SbxVariableRef pArg = rPar.Get( 1 );
        pArg->Format( aStr );

        // Numbers start with a space
        if( pArg->IsNumericRTL() )
        {
            // replace commas by points so that it's symmetric to Val!
            aStr = aStr.replaceFirst( ",", "." );

            SbiInstance* pInst = GetSbData()->pInst;
            bool bCompatibility = ( pInst && pInst->IsCompatibility() );
            if( bCompatibility )
            {
                sal_Int32 nLen = aStr.getLength();

                const sal_Unicode* pBuf = aStr.getStr();

                bool bNeg = ( pBuf[0] == '-' );
                sal_Int32 iZeroSearch = 0;
                if( bNeg )
                {
                    aStrNew += "-";
                    iZeroSearch++;
                }
                else
                {
                    if( pBuf[0] != ' ' )
                    {
                        aStrNew += " ";
                    }
                }
                sal_Int32 iNext = iZeroSearch + 1;
                if( pBuf[iZeroSearch] == '0' && nLen > iNext && pBuf[iNext] == '.' )
                {
                    iZeroSearch += 1;
                }
                aStrNew += aStr.copy(iZeroSearch);
            }
            else
            {
                aStrNew = " " + aStr;
            }
        }
        else
        {
            aStrNew = aStr;
        }
        rPar.Get(0)->PutString( aStrNew );
    }
}

RTLFUNC(StrComp)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        rPar.Get(0)->PutEmpty();
        return;
    }
    const OUString& rStr1 = rPar.Get(1)->GetOUString();
    const OUString& rStr2 = rPar.Get(2)->GetOUString();

    SbiInstance* pInst = GetSbData()->pInst;
    bool bTextCompare;
    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
    if( bCompatibility )
    {
        SbiRuntime* pRT = pInst->pRun;
        bTextCompare = pRT && pRT->IsImageFlag( SbiImageFlags::COMPARETEXT );
    }
    else
    {
        bTextCompare = true;
    }
    if ( rPar.Count() == 4 )
        bTextCompare = rPar.Get(3)->GetInteger();

    if( !bCompatibility )
    {
        bTextCompare = !bTextCompare;
    }
    sal_Int32 nRetValue = 0;
    if( bTextCompare )
    {
        ::utl::TransliterationWrapper* pTransliterationWrapper = GetSbData()->pTransliterationWrapper;
        if( !pTransliterationWrapper )
        {
            uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();
            pTransliterationWrapper = GetSbData()->pTransliterationWrapper =
                new ::utl::TransliterationWrapper( xContext,
                    i18n::TransliterationModules_IGNORE_CASE |
                    i18n::TransliterationModules_IGNORE_KANA |
                    i18n::TransliterationModules_IGNORE_WIDTH );
        }

        LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
        pTransliterationWrapper->loadModuleIfNeeded( eLangType );
        nRetValue = pTransliterationWrapper->compareString( rStr1, rStr2 );
    }
    else
    {
        sal_Int32 aResult;
        aResult = rStr1.compareTo( rStr2 );
        if ( aResult < 0  )
        {
            nRetValue = -1;
        }
        else if ( aResult > 0)
        {
            nRetValue = 1;
        }
    }
    rPar.Get(0)->PutInteger( sal::static_int_cast< sal_Int16 >( nRetValue ) );
}

RTLFUNC(String)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Unicode aFiller;
        sal_Int32 lCount = rPar.Get(1)->GetLong();
        if( lCount < 0 || lCount > 0xffff )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
        if( rPar.Get(2)->GetType() == SbxINTEGER )
        {
            aFiller = (sal_Unicode)rPar.Get(2)->GetInteger();
        }
        else
        {
            const OUString& rStr = rPar.Get(2)->GetOUString();
            aFiller = rStr[0];
        }
        OUStringBuffer aBuf(lCount);
        string::padToLength(aBuf, lCount, aFiller);
        rPar.Get(0)->PutString(aBuf.makeStringAndClear());
    }
}

RTLFUNC(Tab)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    else
    {
        OUStringBuffer aStr;
        comphelper::string::padToLength(aStr, rPar.Get(1)->GetLong(), '\t');
        rPar.Get(0)->PutString(aStr.makeStringAndClear());
    }
}

RTLFUNC(Tan)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const CharClass& rCharClass = GetCharClass();
        OUString aStr( rPar.Get(1)->GetOUString() );
        aStr = rCharClass.uppercase( aStr );
        rPar.Get(0)->PutString( aStr );
    }
}


RTLFUNC(Val)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double nResult = 0.0;
        char* pEndPtr;

        OUString aStr( rPar.Get(1)->GetOUString() );

        FilterWhiteSpace( aStr );
        if ( aStr.getLength() > 1 && aStr[0] == '&' )
        {
            int nRadix = 10;
            char aChar = (char)aStr[1];
            if ( aChar == 'h' || aChar == 'H' )
            {
                nRadix = 16;
            }
            else if ( aChar == 'o' || aChar == 'O' )
            {
                nRadix = 8;
            }
            if ( nRadix != 10 )
            {
                OString aByteStr(OUStringToOString(aStr, osl_getThreadTextEncoding()));
                sal_Int16 nlResult = (sal_Int16)strtol( aByteStr.getStr()+2, &pEndPtr, nRadix);
                nResult = (double)nlResult;
            }
        }
        else
        {
            rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
            sal_Int32 nParseEnd = 0;
            nResult = ::rtl::math::stringToDouble( aStr, '.', ',', &eStatus, &nParseEnd );
            if ( eStatus != rtl_math_ConversionStatus_Ok )
                StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );
            /* TODO: we should check whether all characters were parsed here,
             * but earlier code silently ignored trailing nonsense such as "1x"
             * resulting in 1 with the side effect that any alpha-only-string
             * like "x" resulted in 0. Not changing that now (2013-03-22) as
             * user macros may rely on it. */
#if 0
            else if ( nParseEnd != aStr.getLength() )
                StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
#endif
        }

        rPar.Get(0)->PutDouble( nResult );
    }
}


// Helper functions for date conversion
sal_Int16 implGetDateDay( double aDate )
{
    aDate -= 2.0; // standardize: 1.1.1900 => 0.0
    aDate = floor( aDate );
    Date aRefDate( 1, 1, 1900 );
    aRefDate += static_cast<long>(aDate);

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

css::util::Date SbxDateToUNODate( const SbxValue* const pVal )
{
    double aDate = pVal->GetDate();

    css::util::Date aUnoDate;
    aUnoDate.Day   = implGetDateDay  ( aDate );
    aUnoDate.Month = implGetDateMonth( aDate );
    aUnoDate.Year  = implGetDateYear ( aDate );

    return aUnoDate;
}

void SbxDateFromUNODate( SbxValue *pVal, const css::util::Date& aUnoDate)
{
    double dDate;
    if( implDateSerial( aUnoDate.Year, aUnoDate.Month, aUnoDate.Day, dDate ) )
    {
        pVal->PutDate( dDate );
    }
}

// Function to convert date to UNO date (com.sun.star.util.Date)
RTLFUNC(CDateToUnoDate)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    unoToSbxValue(rPar.Get(0), Any(SbxDateToUNODate(rPar.Get(1))));
}

// Function to convert date from UNO date (com.sun.star.util.Date)
RTLFUNC(CDateFromUnoDate)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 || rPar.Get(1)->GetType() != SbxOBJECT )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    Any aAny (sbxToUnoValue(rPar.Get(1), cppu::UnoType<css::util::Date>::get()));
    css::util::Date aUnoDate;
    if(aAny >>= aUnoDate)
        SbxDateFromUNODate(rPar.Get(0), aUnoDate);
    else
        SbxBase::SetError( ERRCODE_SBX_CONVERSION );
}

css::util::Time SbxDateToUNOTime( const SbxValue* const pVal )
{
    double aDate = pVal->GetDate();

    css::util::Time aUnoTime;
    aUnoTime.Hours       = implGetHour      ( aDate );
    aUnoTime.Minutes     = implGetMinute    ( aDate );
    aUnoTime.Seconds     = implGetSecond    ( aDate );
    aUnoTime.NanoSeconds = 0;

    return aUnoTime;
}

void SbxDateFromUNOTime( SbxValue *pVal, const css::util::Time& aUnoTime)
{
    pVal->PutDate( implTimeSerial(aUnoTime.Hours, aUnoTime.Minutes, aUnoTime.Seconds) );
}

// Function to convert date to UNO time (com.sun.star.util.Time)
RTLFUNC(CDateToUnoTime)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    unoToSbxValue(rPar.Get(0), Any(SbxDateToUNOTime(rPar.Get(1))));
}

// Function to convert date from UNO time (com.sun.star.util.Time)
RTLFUNC(CDateFromUnoTime)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 || rPar.Get(1)->GetType() != SbxOBJECT )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    Any aAny (sbxToUnoValue(rPar.Get(1), cppu::UnoType<css::util::Time>::get()));
    css::util::Time aUnoTime;
    if(aAny >>= aUnoTime)
        SbxDateFromUNOTime(rPar.Get(0), aUnoTime);
    else
        SbxBase::SetError( ERRCODE_SBX_CONVERSION );
}

css::util::DateTime SbxDateToUNODateTime( const SbxValue* const pVal )
{
    double aDate = pVal->GetDate();

    css::util::DateTime aUnoDT;
    aUnoDT.Day   = implGetDateDay  ( aDate );
    aUnoDT.Month = implGetDateMonth( aDate );
    aUnoDT.Year  = implGetDateYear ( aDate );
    aUnoDT.Hours       = implGetHour      ( aDate );
    aUnoDT.Minutes     = implGetMinute    ( aDate );
    aUnoDT.Seconds     = implGetSecond    ( aDate );
    aUnoDT.NanoSeconds = 0;

    return aUnoDT;
}

void SbxDateFromUNODateTime( SbxValue *pVal, const css::util::DateTime& aUnoDT)
{
    double dDate(0.0);
    if( implDateTimeSerial( aUnoDT.Year, aUnoDT.Month, aUnoDT.Day,
                            aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds,
                            dDate ) )
    {
        pVal->PutDate( dDate );
    }
}

// Function to convert date to UNO date (com.sun.star.util.Date)
RTLFUNC(CDateToUnoDateTime)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    unoToSbxValue(rPar.Get(0), Any(SbxDateToUNODateTime(rPar.Get(1))));
}

// Function to convert date from UNO date (com.sun.star.util.Date)
RTLFUNC(CDateFromUnoDateTime)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 || rPar.Get(1)->GetType() != SbxOBJECT )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    Any aAny (sbxToUnoValue(rPar.Get(1), cppu::UnoType<css::util::DateTime>::get()));
    css::util::DateTime aUnoDT;
    if(aAny >>= aUnoDT)
        SbxDateFromUNODateTime(rPar.Get(0), aUnoDT);
    else
        SbxBase::SetError( ERRCODE_SBX_CONVERSION );
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
        OUString aRetStr = OUString::createFromAscii( Buffer );
        rPar.Get(0)->PutString( aRetStr );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

// Function to convert date from ISO 8601 date format
RTLFUNC(CDateFromIso)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        const sal_Int32 iMonthStart = aStr.getLength() - 4;
        OUString aYearStr  = aStr.copy( 0, iMonthStart );
        OUString aMonthStr = aStr.copy( iMonthStart, 2 );
        OUString aDayStr   = aStr.copy( iMonthStart+2, 2 );

        double dDate;
        if( implDateSerial( (sal_Int16)aYearStr.toInt32(),
            (sal_Int16)aMonthStr.toInt32(), (sal_Int16)aDayStr.toInt32(), dDate ) )
        {
            rPar.Get(0)->PutDate( dDate );
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

RTLFUNC(DateSerial)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nYear = rPar.Get(1)->GetInteger();
    sal_Int16 nMonth = rPar.Get(2)->GetInteger();
    sal_Int16 nDay = rPar.Get(3)->GetInteger();

    double dDate;
    if( implDateSerial( nYear, nMonth, nDay, dDate ) )
    {
        rPar.Get(0)->PutDate( dDate );
    }
}

RTLFUNC(TimeSerial)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nHour = rPar.Get(1)->GetInteger();
    if ( nHour == 24 )
    {
        nHour = 0;                      // because of UNO DateTimes, which go till 24 o'clock
    }
    sal_Int16 nMinute = rPar.Get(2)->GetInteger();
    sal_Int16 nSecond = rPar.Get(3)->GetInteger();
    if ((nHour < 0 || nHour > 23)   ||
        (nMinute < 0 || nMinute > 59 )  ||
        (nSecond < 0 || nSecond > 59 ))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    rPar.Get(0)->PutDate( implTimeSerial(nHour, nMinute, nSecond) ); // JSM
}

RTLFUNC(DateValue)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // #39629 check GetSbData()->pInst, can be called from the URL line
        SvNumberFormatter* pFormatter = nullptr;
        if( GetSbData()->pInst )
        {
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
        }
        else
        {
            sal_uInt32 n;   // Dummy
            pFormatter = SbiInstance::PrepareNumberFormatter( n, n, n );
        }

        sal_uInt32 nIndex = 0;
        double fResult;
        OUString aStr( rPar.Get(1)->GetOUString() );
        bool bSuccess = pFormatter->IsNumberFormat( aStr, nIndex, fResult );
        short nType = pFormatter->GetType( nIndex );

        // DateValue("February 12, 1969") raises error if the system locale is not en_US
        // by using SbiInstance::GetNumberFormatter.
        // It seems that both locale number formatter and English number formatter
        // are supported in Visual Basic.
        LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
        if( !bSuccess && ( eLangType != LANGUAGE_ENGLISH_US ) )
        {
            // Create a new SvNumberFormatter by using LANGUAGE_ENGLISH to get the date value;
            SvNumberFormatter aFormatter( comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US );
            nIndex = 0;
            bSuccess = aFormatter.IsNumberFormat( aStr, nIndex, fResult );
            nType = aFormatter.GetType( nIndex );
        }

        if(bSuccess && (nType==css::util::NumberFormat::DATE || nType==css::util::NumberFormat::DATETIME))
        {
            if ( nType == css::util::NumberFormat::DATETIME )
            {
                // cut time
                if ( fResult  > 0.0 )
                {
                    fResult = floor( fResult );
                }
                else
                {
                    fResult = ceil( fResult );
                }
            }
            rPar.Get(0)->PutDate( fResult );
        }
        else
        {
            StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
        }
        // #39629 pFormatter can be requested itself
        if( !GetSbData()->pInst )
        {
            delete pFormatter;
        }
    }
}

RTLFUNC(TimeValue)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SvNumberFormatter* pFormatter = nullptr;
        if( GetSbData()->pInst )
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
        else
        {
            sal_uInt32 n;
            pFormatter = SbiInstance::PrepareNumberFormatter( n, n, n );
        }

        sal_uInt32 nIndex = 0;
        double fResult;
        bool bSuccess = pFormatter->IsNumberFormat( rPar.Get(1)->GetOUString(),
                                                   nIndex, fResult );
        short nType = pFormatter->GetType(nIndex);
        if(bSuccess && (nType==css::util::NumberFormat::TIME||nType==css::util::NumberFormat::DATETIME))
        {
            if ( nType == css::util::NumberFormat::DATETIME )
            {
                // cut days
                fResult = fmod( fResult, 1 );
            }
            rPar.Get(0)->PutDate( fResult );
        }
        else
        {
            StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
        }
        if( !GetSbData()->pInst )
        {
            delete pFormatter;
        }
    }
}

RTLFUNC(Day)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nHour = implGetHour( nArg );
        rPar.Get(0)->PutInteger( nHour );
    }
}

RTLFUNC(Minute)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
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
    tools::Time aTime( tools::Time::SYSTEM );
    double aSerial = (double)GetDayDiff( aDate );
    long nSeconds = aTime.GetHour();
    nSeconds *= 3600;
    nSeconds += aTime.GetMin() * 60;
    nSeconds += aTime.GetSec();
    double nDays = ((double)nSeconds) / (double)(24.0*3600.0);
    aSerial += nDays;
    return aSerial;
}

// Date Now()

RTLFUNC(Now)
{
    (void)pBasic;
    (void)bWrite;
    rPar.Get(0)->PutDate( Now_Impl() );
}

// Date Time()

RTLFUNC(Time)
{
    (void)pBasic;

    if ( !bWrite )
    {
        tools::Time aTime( tools::Time::SYSTEM );
        SbxVariable* pMeth = rPar.Get( 0 );
        OUString aRes;
        if( pMeth->IsFixed() )
        {
            // Time$: hh:mm:ss
            char buf[ 20 ];
            snprintf( buf, sizeof(buf), "%02d:%02d:%02d",
                      aTime.GetHour(), aTime.GetMin(), aTime.GetSec() );
            aRes = OUString::createFromAscii( buf );
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

            SvNumberFormatter* pFormatter = nullptr;
            sal_uInt32 nIndex;
            if( GetSbData()->pInst )
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
                nIndex = GetSbData()->pInst->GetStdTimeIdx();
            }
            else
            {
                sal_uInt32 n;   // Dummy
                pFormatter = SbiInstance::PrepareNumberFormatter( n, nIndex, n );
            }

            pFormatter->GetOutputString( nDays, nIndex, aRes, &pCol );

            if( !GetSbData()->pInst )
            {
                delete pFormatter;
            }
        }
        pMeth->PutString( aRes );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_NOT_IMPLEMENTED );
    }
}

RTLFUNC(Timer)
{
    (void)pBasic;
    (void)bWrite;

    tools::Time aTime( tools::Time::SYSTEM );
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
            OUString aRes;
            Color* pCol;

            SvNumberFormatter* pFormatter = nullptr;
            sal_uInt32 nIndex;
            if( GetSbData()->pInst )
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
                nIndex = GetSbData()->pInst->GetStdDateIdx();
            }
            else
            {
                sal_uInt32 n;
                pFormatter = SbiInstance::PrepareNumberFormatter( nIndex, n, n );
            }

            pFormatter->GetOutputString( nDays, nIndex, aRes, &pCol );
            pMeth->PutString( aRes );

            if( !GetSbData()->pInst )
            {
                delete pFormatter;
            }
        }
        else
        {
            pMeth->PutDate( nDays );
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_NOT_IMPLEMENTED );
    }
}

RTLFUNC(IsArray)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        rPar.Get(0)->PutBool((rPar.Get(1)->GetType() & SbxARRAY) != 0);
    }
}

RTLFUNC(IsObject)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariable* pVar = rPar.Get(1);
        SbxBase* pObj = pVar->GetObject();

        // #100385: GetObject can result in an error, so reset it
        SbxBase::ResetError();

        SbUnoClass* pUnoClass;
        bool bObject;
        if( pObj &&  nullptr != ( pUnoClass=dynamic_cast<SbUnoClass*>( pObj) )  )
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // #46134 only string is converted, all other types result in sal_False
        SbxVariableRef xArg = rPar.Get( 1 );
        SbxDataType eType = xArg->GetType();
        bool bDate = false;

        if( eType == SbxDATE )
        {
            bDate = true;
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariable* pVar = nullptr;
        if( SbiRuntime::isVBAEnabled() )
        {
            pVar = getDefaultProp( rPar.Get(1) );
        }
        if ( pVar )
        {
            pVar->Broadcast( SfxHintId::BasicDataWanted );
            rPar.Get( 0 )->PutBool( pVar->IsEmpty() );
        }
        else
        {
            rPar.Get( 0 )->PutBool( rPar.Get(1)->IsEmpty() );
        }
    }
}

RTLFUNC(IsError)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariable* pVar =rPar.Get( 1 );
        SbUnoObject* pObj = dynamic_cast<SbUnoObject*>( pVar  );
        if ( !pObj )
        {
            if ( SbxBase* pBaseObj = pVar->GetObject() )
            {
                pObj = dynamic_cast<SbUnoObject*>( pBaseObj  );
            }
        }
        uno::Reference< script::XErrorQuery > xError;
        if ( pObj )
        {
            xError.set( pObj->getUnoAny(), uno::UNO_QUERY );
        }
        if ( xError.is() )
        {
            rPar.Get( 0 )->PutBool( xError->hasError() );
        }
        else
        {
            rPar.Get( 0 )->PutBool( rPar.Get(1)->IsErr() );
        }
    }
}

RTLFUNC(IsNull)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // #51475 because of Uno-objects return true
        // even if the pObj value is NULL
        SbxVariableRef pArg = rPar.Get( 1 );
        bool bNull = rPar.Get(1)->IsNull();
        if( !bNull && pArg->GetType() == SbxOBJECT )
        {
            SbxBase* pObj = pArg->GetObject();
            if( !pObj )
            {
                bNull = true;
            }
        }
        rPar.Get( 0 )->PutBool( bNull );
    }
}

RTLFUNC(IsNumeric)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        rPar.Get( 0 )->PutBool( rPar.Get( 1 )->IsNumericRTL() );
    }
}


RTLFUNC(IsMissing)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // #57915 Missing is reported by an error
        rPar.Get( 0 )->PutBool( rPar.Get(1)->IsErr() );
    }
}

// Function looks for wildcards, removes them and always returns the pure path
OUString implSetupWildcard( const OUString& rFileParam, SbiRTLData* pRTLData )
{
    static sal_Char cDelim1 = (sal_Char)'/';
    static sal_Char cDelim2 = (sal_Char)'\\';
    static sal_Char cWild1 = '*';
    static sal_Char cWild2 = '?';

    pRTLData->pWildCard.reset();
    pRTLData->sFullNameToBeChecked.clear();

    OUString aFileParam = rFileParam;
    sal_Int32 nLastWild = aFileParam.lastIndexOf( cWild1 );
    if( nLastWild < 0 )
    {
        nLastWild = aFileParam.lastIndexOf( cWild2 );
    }
    bool bHasWildcards = ( nLastWild >= 0 );


    sal_Int32 nLastDelim = aFileParam.lastIndexOf( cDelim1 );
    if( nLastDelim < 0 )
    {
        nLastDelim = aFileParam.lastIndexOf( cDelim2 );
    }
    if( bHasWildcards )
    {
        // Wildcards in path?
        if( nLastDelim >= 0 && nLastDelim > nLastWild )
        {
            return aFileParam;
        }
    }
    else
    {
        OUString aPathStr = getFullPath( aFileParam );
        if( nLastDelim != aFileParam.getLength() - 1 )
        {
            pRTLData->sFullNameToBeChecked = aPathStr;
        }
        return aPathStr;
    }

    OUString aPureFileName;
    if( nLastDelim < 0 )
    {
        aPureFileName = aFileParam;
        aFileParam.clear();
    }
    else
    {
        aPureFileName = aFileParam.copy( nLastDelim + 1 );
        aFileParam = aFileParam.copy( 0, nLastDelim );
    }

    // Try again to get a valid URL/UNC-path with only the path
    OUString aPathStr = getFullPath( aFileParam );

    // Is there a pure file name left? Otherwise the path is
    // invalid anyway because it was not accepted by OSL before
    if (aPureFileName != "*")
    {
        pRTLData->pWildCard = o3tl::make_unique<WildCard>( aPureFileName );
    }
    return aPathStr;
}

inline bool implCheckWildcard( const OUString& rName, SbiRTLData* pRTLData )
{
    bool bMatch = true;

    if( pRTLData->pWildCard )
    {
        bMatch = pRTLData->pWildCard->Matches( rName );
    }
    return bMatch;
}


bool isRootDir( const OUString& aDirURLStr )
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
        OUString aSeg1 = aDirURLObj.getName( 0, true,
                                             INetURLObject::DecodeMechanism::WithCharset );
        if( aSeg1[1] == (sal_Unicode)':' )
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

    OUString aPath;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbiRTLData* pRTLData = GetSbData()->pInst->GetRTLData();

        // #34645: can also be called from the URL line via 'macro: Dir'
        // there's no pRTLDate existing in that case and the method must be left
        if( !pRTLData )
        {
            return;
        }
        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                if ( nParCount >= 2 )
                {
                    OUString aFileParam = rPar.Get(1)->GetOUString();

                    OUString aFileURLStr = implSetupWildcard( aFileParam, pRTLData );
                    if( !pRTLData->sFullNameToBeChecked.isEmpty())
                    {
                        bool bExists = false;
                        try { bExists = xSFI->exists( aFileURLStr ); }
                        catch(const Exception & ) {}

                        OUString aNameOnlyStr;
                        if( bExists )
                        {
                            INetURLObject aFileURL( aFileURLStr );
                            aNameOnlyStr = aFileURL.getName( INetURLObject::LAST_SEGMENT,
                                                             true, INetURLObject::DecodeMechanism::WithCharset );
                        }
                        rPar.Get(0)->PutString( aNameOnlyStr );
                        return;
                    }

                    try
                    {
                        OUString aDirURLStr;
                        bool bFolder = xSFI->isFolder( aFileURLStr );

                        if( bFolder )
                        {
                            aDirURLStr = aFileURLStr;
                        }
                        else
                        {
                            rPar.Get(0)->PutString( "" );
                        }

                        SbAttributes nFlags = SbAttributes::NONE;
                        if ( nParCount > 2 )
                        {
                            pRTLData->nDirFlags = nFlags = static_cast<SbAttributes>(rPar.Get(2)->GetInteger());
                        }
                        else
                        {
                            pRTLData->nDirFlags = SbAttributes::NONE;
                        }
                        // Read directory
                        bool bIncludeFolders = bool(nFlags & SbAttributes::DIRECTORY);
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
                    bool bFolderFlag = bool(pRTLData->nDirFlags & SbAttributes::DIRECTORY);

                    SbiInstance* pInst = GetSbData()->pInst;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    for( ;; )
                    {
                        if( pRTLData->nCurDirPos < 0 )
                        {
                            if( pRTLData->nCurDirPos == -2 )
                            {
                                aPath = ".";
                            }
                            else if( pRTLData->nCurDirPos == -1 )
                            {
                                aPath = "..";
                            }
                            pRTLData->nCurDirPos++;
                        }
                        else if( pRTLData->nCurDirPos >= pRTLData->aDirSeq.getLength() )
                        {
                            pRTLData->aDirSeq.realloc( 0 );
                            aPath.clear();
                            break;
                        }
                        else
                        {
                            OUString aFile = pRTLData->aDirSeq.getConstArray()[pRTLData->nCurDirPos++];

                            if( bCompatibility )
                            {
                                if( !bFolderFlag )
                                {
                                    bool bFolder = xSFI->isFolder( aFile );
                                    if( bFolder )
                                    {
                                        continue;
                                    }
                                }
                            }
                            else
                            {
                                // Only directories
                                if( bFolderFlag )
                                {
                                    bool bFolder = xSFI->isFolder( aFile );
                                    if( !bFolder )
                                    {
                                        continue;
                                    }
                                }
                            }

                            INetURLObject aURL( aFile );
                            aPath = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                                                  INetURLObject::DecodeMechanism::WithCharset );
                        }

                        bool bMatch = implCheckWildcard( aPath, pRTLData );
                        if( !bMatch )
                        {
                            continue;
                        }
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
                OUString aFileParam = rPar.Get(1)->GetOUString();

                OUString aDirURL = implSetupWildcard( aFileParam, pRTLData );

                SbAttributes nFlags = SbAttributes::NONE;
                if ( nParCount > 2 )
                {
                    pRTLData->nDirFlags = nFlags = static_cast<SbAttributes>( rPar.Get(2)->GetInteger() );
                }
                else
                {
                    pRTLData->nDirFlags = SbAttributes::NONE;
                }

                // Read directory
                bool bIncludeFolders = bool(nFlags & SbAttributes::DIRECTORY);
                pRTLData->pDir = o3tl::make_unique<Directory>( aDirURL );
                FileBase::RC nRet = pRTLData->pDir->open();
                if( nRet != FileBase::E_None )
                {
                    pRTLData->pDir.reset();
                    rPar.Get(0)->PutString( OUString() );
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
                bool bFolderFlag = bool(pRTLData->nDirFlags & SbAttributes::DIRECTORY);
                for( ;; )
                {
                    if( pRTLData->nCurDirPos < 0 )
                    {
                        if( pRTLData->nCurDirPos == -2 )
                        {
                            aPath = ".";
                        }
                        else if( pRTLData->nCurDirPos == -1 )
                        {
                            aPath = "..";
                        }
                        pRTLData->nCurDirPos++;
                    }
                    else
                    {
                        DirectoryItem aItem;
                        FileBase::RC nRet = pRTLData->pDir->getNextItem( aItem );
                        if( nRet != FileBase::E_None )
                        {
                            pRTLData->pDir.reset();
                            aPath.clear();
                            break;
                        }

                        // Handle flags
                        FileStatus aFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName );
                        nRet = aItem.getFileStatus( aFileStatus );
                        if( nRet != FileBase::E_None )
                        {
                            SAL_WARN("basic", "getFileStatus failed");
                            continue;
                        }

                        // Only directories?
                        if( bFolderFlag )
                        {
                            FileStatus::Type aType = aFileStatus.getFileType();
                            bool bFolder = isFolder( aType );
                            if( !bFolder )
                            {
                                continue;
                            }
                        }

                        aPath = aFileStatus.getFileName();
                    }

                    bool bMatch = implCheckWildcard( aPath, pRTLData );
                    if( !bMatch )
                    {
                        continue;
                    }
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

        // In Windows, we want to use Windows API to get the file attributes
        // for VBA interoperability.
    #if defined(_WIN32)
        if( SbiRuntime::isVBAEnabled() )
        {
            OUString aPathURL = getFullPath( rPar.Get(1)->GetOUString() );
            OUString aPath;
            FileBase::getSystemPathFromFileURL( aPathURL, aPath );
            OString aSystemPath(OUStringToOString(aPath, osl_getThreadTextEncoding()));
            DWORD nRealFlags = GetFileAttributes (aSystemPath.getStr());
            if (nRealFlags != 0xffffffff)
            {
                if (nRealFlags == FILE_ATTRIBUTE_NORMAL)
                {
                    nRealFlags = 0;
                }
                nFlags = (sal_Int16) (nRealFlags);
            }
            else
            {
                StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
            }
            rPar.Get(0)->PutInteger( nFlags );

            return;
        }
    #endif

        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    OUString aPath = getFullPath( rPar.Get(1)->GetOUString() );
                    bool bExists = false;
                    try { bExists = xSFI->exists( aPath ); }
                    catch(const Exception & ) {}
                    if( !bExists )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
                        return;
                    }

                    bool bReadOnly = xSFI->isReadOnly( aPath );
                    bool bHidden = xSFI->isHidden( aPath );
                    bool bDirectory = xSFI->isFolder( aPath );
                    if( bReadOnly )
                    {
                        nFlags |= (sal_uInt16)SbAttributes::READONLY;
                    }
                    if( bHidden )
                    {
                        nFlags |= (sal_uInt16)SbAttributes::HIDDEN;
                    }
                    if( bDirectory )
                    {
                        nFlags |= (sal_uInt16)SbAttributes::DIRECTORY;
                    }
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
            DirectoryItem::get( getFullPath( rPar.Get(1)->GetOUString() ), aItem );
            FileStatus aFileStatus( osl_FileStatus_Mask_Attributes | osl_FileStatus_Mask_Type );
            aItem.getFileStatus( aFileStatus );
            sal_uInt64 nAttributes = aFileStatus.getAttributes();
            bool bReadOnly = (nAttributes & osl_File_Attribute_ReadOnly) != 0;

            FileStatus::Type aType = aFileStatus.getFileType();
            bool bDirectory = isFolder( aType );
            if( bReadOnly )
            {
                nFlags |= (sal_uInt16)SbAttributes::READONLY;
            }
            if( bDirectory )
            {
                nFlags |= (sal_uInt16)SbAttributes::DIRECTORY;
            }
        }
        rPar.Get(0)->PutInteger( nFlags );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}


RTLFUNC(FileDateTime)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aPath = rPar.Get(1)->GetOUString();
        tools::Time aTime( tools::Time::EMPTY );
        Date aDate( Date::EMPTY );
        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    util::DateTime aUnoDT = xSFI->getDateTimeModified( aPath );
                    aTime = tools::Time( aUnoDT );
                    aDate = Date( aUnoDT );
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
            DirectoryItem::get( getFullPath( aPath ), aItem );
            FileStatus aFileStatus( osl_FileStatus_Mask_ModifyTime );
            aItem.getFileStatus( aFileStatus );
            TimeValue aTimeVal = aFileStatus.getModifyTime();
            oslDateTime aDT;
            osl_getDateTimeFromTimeValue( &aTimeVal, &aDT );

            aTime = tools::Time( aDT.Hours, aDT.Minutes, aDT.Seconds, aDT.NanoSeconds );
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

        SvNumberFormatter* pFormatter = nullptr;
        sal_uInt32 nIndex;
        if( GetSbData()->pInst )
        {
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
            nIndex = GetSbData()->pInst->GetStdDateTimeIdx();
        }
        else
        {
            sal_uInt32 n;
            pFormatter = SbiInstance::PrepareNumberFormatter( n, n, nIndex );
        }

        OUString aRes;
        pFormatter->GetOutputString( fSerial, nIndex, aRes, &pCol );
        rPar.Get(0)->PutString( aRes );

        if( !GetSbData()->pInst )
        {
            delete pFormatter;
        }
    }
}


RTLFUNC(EOF)
{
    (void)pBasic;
    (void)bWrite;

    // No changes for UCB
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
            return;
        }
        bool bIsEof;
        SvStream* pSvStrm = pSbStrm->GetStrm();
        if ( pSbStrm->IsText() )
        {
            char cBla;
            (*pSvStrm).ReadChar( cBla ); // can we read another character?
            bIsEof = pSvStrm->IsEof();
            if ( !bIsEof )
            {
                pSvStrm->SeekRel( -1 );
            }
        }
        else
        {
            bIsEof = pSvStrm->IsEof();  // for binary data!
        }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
            return;
        }
        sal_Int16 nRet;
        if ( rPar.Get(2)->GetInteger() == 1 )
        {
            nRet = (sal_Int16)(pSbStrm->GetMode());
        }
        else
        {
            nRet = 0; // System file handle not supported
        }
        rPar.Get(0)->PutInteger( nRet );
    }
}
RTLFUNC(Loc)
{
    (void)pBasic;
    (void)bWrite;

    // No changes for UCB
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
            return;
        }
        SvStream* pSvStrm = pSbStrm->GetStrm();
        std::size_t nPos;
        if( pSbStrm->IsRandom())
        {
            short nBlockLen = pSbStrm->GetBlockLen();
            nPos = nBlockLen ? (pSvStrm->Tell() / nBlockLen) : 0;
            nPos++; // block positions starting at 1
        }
        else if ( pSbStrm->IsText() )
        {
            nPos = pSbStrm->GetLine();
        }
        else if( pSbStrm->IsBinary() )
        {
            nPos = pSvStrm->Tell();
        }
        else if ( pSbStrm->IsSeq() )
        {
            nPos = ( pSvStrm->Tell()+1 ) / 128;
        }
        else
        {
            nPos = pSvStrm->Tell();
        }
        rPar.Get(0)->PutLong( (sal_Int32)nPos );
    }
}

RTLFUNC(Lof)
{
    (void)pBasic;
    (void)bWrite;

    // No changes for UCB
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int16 nChannel = rPar.Get(1)->GetInteger();
        SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
        SbiStream* pSbStrm = pIO->GetStream( nChannel );
        if ( !pSbStrm )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
            return;
        }
        SvStream* pSvStrm = pSbStrm->GetStrm();
        sal_uInt64 const nOldPos = pSvStrm->Tell();
        sal_uInt64 const nLen = pSvStrm->Seek( STREAM_SEEK_TO_END );
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nChannel );
    if ( !pSbStrm )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
        return;
    }
    SvStream* pStrm = pSbStrm->GetStrm();

    if ( nArgs == 2 )   // Seek-Function
    {
        sal_uInt64 nPos = pStrm->Tell();
        if( pSbStrm->IsRandom() )
        {
            nPos = nPos / pSbStrm->GetBlockLen();
        }
        nPos++; // Basic counts from 1
        rPar.Get(0)->PutLong( (sal_Int32)nPos );
    }
    else                // Seek-Statement
    {
        sal_Int32 nPos = rPar.Get(2)->GetLong();
        if ( nPos < 1 )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
            return;
        }
        nPos--; // Basic counts from 1, SvStreams count from 0
        pSbStrm->SetExpandOnWriteTo( 0 );
        if ( pSbStrm->IsRandom() )
        {
            nPos *= pSbStrm->GetBlockLen();
        }
        pStrm->Seek( static_cast<sal_uInt64>(nPos) );
        pSbStrm->SetExpandOnWriteTo( nPos );
    }
}

RTLFUNC(Format)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nArgCount = (sal_uInt16)rPar.Count();
    if ( nArgCount < 2 || nArgCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aResult;
        if( nArgCount == 2 )
        {
            rPar.Get(1)->Format( aResult );
        }
        else
        {
            OUString aFmt( rPar.Get(2)->GetOUString() );
            rPar.Get(1)->Format( aResult, &aFmt );
        }
        rPar.Get(0)->PutString( aResult );
    }
}

namespace {

// note: BASIC does not use comphelper::random, because
// Randomize(int) must be supported and should not affect non-BASIC random use
struct RandomNumberGenerator
{
    std::mt19937 global_rng;

    RandomNumberGenerator()
    {
        try
        {
            std::random_device rd;
            // initialises the state of the global random number generator
            // should only be called once.
            // (note, a few std::variate_generator<> (like normal) have their
            // own state which would need a reset as well to guarantee identical
            // sequence of numbers, e.g. via myrand.distribution().reset())
            global_rng.seed(rd() ^ time(nullptr));
        }
        catch (std::runtime_error& e)
        {
            SAL_WARN("basic", "Using std::random_device failed: " << e.what());
            global_rng.seed(time(nullptr));
        }
    }
};

class theRandomNumberGenerator : public rtl::Static<RandomNumberGenerator, theRandomNumberGenerator> {};

}

RTLFUNC(Randomize)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() > 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    if( rPar.Count() == 2 )
    {
        int nSeed = (int)rPar.Get(1)->GetInteger();
        theRandomNumberGenerator::get().global_rng.seed(nSeed);
    }
    // without parameter, no need to do anything - RNG is seeded at first use
}

RTLFUNC(Rnd)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() > 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double const tmp(dist(theRandomNumberGenerator::get().global_rng));
        rPar.Get(0)->PutDouble(tmp);
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

    std::size_t nArgCount = rPar.Count();
    if ( nArgCount < 2 || nArgCount > 5 )
    {
        rPar.Get(0)->PutLong(0);
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        oslProcessOption nOptions = osl_Process_SEARCHPATH | osl_Process_DETACHED;

        OUString aCmdLine = rPar.Get(1)->GetOUString();
        // attach additional parameters - everything must be parsed anyway
        if( nArgCount >= 4 )
        {
            OUString tmp = rPar.Get(3)->GetOUString().trim();
            if (!tmp.isEmpty())
            {
                aCmdLine += " ";
                aCmdLine += tmp;
            }
        }
        else if( aCmdLine.isEmpty() )
        {
            // avoid special treatment (empty list)
            aCmdLine += " ";
        }
        sal_Int32 nLen = aCmdLine.getLength();

        // #55735 if there are parameters, they have to be separated
        // #72471 also separate the single parameters
        std::list<OUString> aTokenList;
        OUString aToken;
        sal_Int32 i = 0;
        sal_Unicode c;
        while( i < nLen )
        {
            for ( ;; ++i )
            {
                c = aCmdLine[ i ];
                if ( c != ' ' && c != '\t' )
                {
                    break;
                }
            }

            if( c == '\"' || c == '\'' )
            {
                sal_Int32 iFoundPos = aCmdLine.indexOf( c, i + 1 );

                if( iFoundPos < 0 )
                {
                    aToken = aCmdLine.copy( i);
                    i = nLen;
                }
                else
                {
                    aToken = aCmdLine.copy( i + 1, (iFoundPos - i - 1) );
                    i = iFoundPos + 1;
                }
            }
            else
            {
                sal_Int32 iFoundSpacePos = aCmdLine.indexOf( ' ', i );
                sal_Int32 iFoundTabPos = aCmdLine.indexOf( '\t', i );
                sal_Int32 iFoundPos = iFoundSpacePos >= 0 ? iFoundTabPos >= 0 ? std::min( iFoundSpacePos, iFoundTabPos ) : iFoundSpacePos : -1;

                if( iFoundPos < 0 )
                {
                    aToken = aCmdLine.copy( i );
                    i = nLen;
                }
                else
                {
                    aToken = aCmdLine.copy( i, (iFoundPos - i) );
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

            bool bSync = false;
            if( nArgCount >= 5 )
            {
                bSync = rPar.Get(4)->GetBool();
            }
            if( bSync )
            {
                nOptions |= osl_Process_WAIT;
            }
        }

        // #72471 work parameter(s) up
        std::list<OUString>::const_iterator iter = aTokenList.begin();
        const OUString& rStr = *iter;
        OUString aOUStrProg( rStr.getStr(), rStr.getLength() );
        OUString aOUStrProgURL = getFullPath( aOUStrProg );

        ++iter;

        sal_uInt16 nParamCount = sal::static_int_cast< sal_uInt16 >(aTokenList.size() - 1 );
        std::unique_ptr<rtl_uString*[]> pParamList;
        if( nParamCount )
        {
            pParamList.reset( new rtl_uString*[nParamCount]);
            for(int iList = 0; iter != aTokenList.end(); ++iList, ++iter)
            {
                const OUString& rParamStr = (*iter);
                const OUString aTempStr( rParamStr.getStr(), rParamStr.getLength());
                pParamList[iList] = nullptr;
                rtl_uString_assign(&(pParamList[iList]), aTempStr.pData);
            }
        }

        oslProcess pApp;
        bool bSucc = osl_executeProcess(
                    aOUStrProgURL.pData,
                    pParamList.get(),
                    nParamCount,
                    nOptions,
                    nullptr,
                    nullptr,
                    nullptr, 0,
                    &pApp ) == osl_Process_E_None;

        // 53521 only free process handle on success
        if (bSucc)
        {
            osl_freeProcessHandle( pApp );
        }

        for(int j = 0; j < nParamCount; ++j)
        {
            rtl_uString_release(pParamList[j]);
        }

        if( !bSucc )
        {
            StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
        }
        else
        {
            rPar.Get(0)->PutLong( 0 );
        }
    }
}

RTLFUNC(VarType)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        rPar.Get(0)->PutInteger( (sal_Int16)eType );
    }
}

// Exported function
OUString getBasicTypeName( SbxDataType eType )
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
        "Unknown Type",
        "Unknown Type",
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

    size_t nPos = static_cast<size_t>(eType) & 0x0FFF;
    const size_t nTypeNameCount = SAL_N_ELEMENTS( pTypeNames );
    if ( nPos >= nTypeNameCount )
    {
        nPos = nTypeNameCount - 1;
    }
    return OUString::createFromAscii(pTypeNames[nPos]);
}

OUString getObjectTypeName( SbxVariable* pVar )
{
    OUString sRet( "Object" );
    if ( pVar )
    {
        SbxBase* pObj = pVar->GetObject();
        if( !pObj )
        {
           sRet = "Nothing";
        }
        else
        {
            SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>( pVar  );
            if ( !pUnoObj )
            {
                if ( SbxBase* pBaseObj = pVar->GetObject() )
                {
                    pUnoObj = dynamic_cast<SbUnoObject*>( pBaseObj  );
                }
            }
            if ( pUnoObj )
            {
                Any aObj = pUnoObj->getUnoAny();
                // For upstreaming unless we start to build oovbaapi by default
                // we need to get detect the vba-ness of the object in some
                // other way
                // note: Automation objects do not support XServiceInfo
                uno::Reference< XServiceInfo > xServInfo( aObj, uno::UNO_QUERY );
                if ( xServInfo.is() )
                {
                    // is this a VBA object ?
                    uno::Reference< ooo::vba::XHelperInterface > xVBA( aObj, uno::UNO_QUERY );
                    Sequence< OUString > sServices = xServInfo->getSupportedServiceNames();
                    if ( sServices.getLength() )
                    {
                        sRet = sServices[ 0 ];
                    }
                }
                else
                {
                    uno::Reference< bridge::oleautomation::XAutomationObject > xAutoMation( aObj, uno::UNO_QUERY );
                    if ( xAutoMation.is() )
                    {
                        uno::Reference< script::XInvocation > xInv( aObj, uno::UNO_QUERY );
                        if ( xInv.is() )
                        {
                            try
                            {
                                xInv->getValue( "$GetTypeName" ) >>= sRet;
                            }
                            catch(const Exception& )
                            {
                            }
                        }
                    }
                }
                sal_Int32 nDot = sRet.lastIndexOf( '.' );
                if ( nDot != -1 && nDot < sRet.getLength() )
                {
                    sRet = sRet.copy( nDot + 1 );
                }
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        bool bIsArray = ( ( eType & SbxARRAY ) != 0 );

        OUString aRetStr;
        if ( SbiRuntime::isVBAEnabled() && eType == SbxOBJECT )
        {
            aRetStr = getObjectTypeName( rPar.Get(1) );
        }
        else
        {
            aRetStr = getBasicTypeName( eType );
        }
        if( bIsArray )
        {
            aRetStr += "()";
        }
        rPar.Get(0)->PutString( aRetStr );
    }
}

RTLFUNC(Len)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const OUString& rStr = rPar.Get(1)->GetOUString();
        rPar.Get(0)->PutLong( rStr.getLength() );
    }
}

RTLFUNC(DDEInitiate)
{
    (void)pBasic;
    (void)bWrite;

    int nArgs = (int)rPar.Count();
    if ( nArgs != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    const OUString& rApp = rPar.Get(1)->GetOUString();
    const OUString& rTopic = rPar.Get(2)->GetOUString();

    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    size_t nChannel;
    SbError nDdeErr = pDDE->Initiate( rApp, rTopic, nChannel );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
    else
    {
        rPar.Get(0)->PutInteger( static_cast<sal_Int16>(nChannel) );
    }
}

RTLFUNC(DDETerminate)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    SbError nDdeErr = pDDE->Terminate( nChannel );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}

RTLFUNC(DDETerminateAll)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    SbError nDdeErr = pDDE->TerminateAll();
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}

RTLFUNC(DDERequest)
{
    (void)pBasic;
    (void)bWrite;

    int nArgs = (int)rPar.Count();
    if ( nArgs != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    const OUString& rItem = rPar.Get(2)->GetOUString();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    OUString aResult;
    SbError nDdeErr = pDDE->Request( nChannel, rItem, aResult );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
    else
    {
        rPar.Get(0)->PutString( aResult );
    }
}

RTLFUNC(DDEExecute)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    const OUString& rCommand = rPar.Get(2)->GetOUString();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    SbError nDdeErr = pDDE->Execute( nChannel, rCommand );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}

RTLFUNC(DDEPoke)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    int nArgs = (int)rPar.Count();
    if ( nArgs != 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    const OUString& rItem = rPar.Get(2)->GetOUString();
    const OUString& rData = rPar.Get(3)->GetOUString();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    SbError nDdeErr = pDDE->Poke( nChannel, rItem, rData );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}


RTLFUNC(FreeFile)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
    StarBASIC::Error( ERRCODE_BASIC_TOO_MANY_FILES );
}

RTLFUNC(LBound)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = dynamic_cast<SbxDimArray*>( pParObj );
    if( pArr )
    {
        sal_Int32 nLower, nUpper;
        short nDim = (nParCount == 3) ? (short)rPar.Get(2)->GetInteger() : 1;
        if( !pArr->GetDim32( nDim, nLower, nUpper ) )
            StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
        else
            rPar.Get(0)->PutLong( nLower );
    }
    else
        StarBASIC::Error( ERRCODE_BASIC_MUST_HAVE_DIMS );
}

RTLFUNC(UBound)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = dynamic_cast<SbxDimArray*>( pParObj );
    if( pArr )
    {
        sal_Int32 nLower, nUpper;
        short nDim = (nParCount == 3) ? (short)rPar.Get(2)->GetInteger() : 1;
        if( !pArr->GetDim32( nDim, nLower, nUpper ) )
            StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
        else
            rPar.Get(0)->PutLong( nUpper );
    }
    else
        StarBASIC::Error( ERRCODE_BASIC_MUST_HAVE_DIMS );
}

RTLFUNC(RGB)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    sal_Int32 nRed     = rPar.Get(1)->GetInteger() & 0xFF;
    sal_Int32 nGreen = rPar.Get(2)->GetInteger() & 0xFF;
    sal_Int32 nBlue  = rPar.Get(3)->GetInteger() & 0xFF;
    sal_Int32 nRGB;

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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    sal_Int16 nCol = rPar.Get(1)->GetInteger();
    if( nCol < 0 || nCol > 15 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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

    std::size_t nArgCount = rPar.Count()-1;
    if( nArgCount < 2 || nArgCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aOldStr = rPar.Get(1)->GetOUString();
    sal_Int32 nConversion = rPar.Get(2)->GetLong();

    sal_uInt16 nLanguage = LANGUAGE_SYSTEM;

    sal_Int32 nOldLen = aOldStr.getLength();
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
        aOldStr = rCharClass.titlecase( aOldStr.toAsciiLowerCase(), 0, nOldLen );
    }
    else if ( (nConversion & 0x01) == 1 ) // vbUpperCase
    {
        nType |= i18n::TransliterationModules_LOWERCASE_UPPERCASE;
    }
    else if ( (nConversion & 0x02) == 2 ) // vbLowerCase
    {
        nType |= i18n::TransliterationModules_UPPERCASE_LOWERCASE;
    }
    if ( (nConversion & 0x04) == 4 ) // vbWide
    {
        nType |= i18n::TransliterationModules_HALFWIDTH_FULLWIDTH;
    }
    else if ( (nConversion & 0x08) == 8 ) // vbNarrow
    {
        nType |= i18n::TransliterationModules_FULLWIDTH_HALFWIDTH;
    }
    if ( (nConversion & 0x10) == 16) // vbKatakana
    {
        nType |= i18n::TransliterationModules_HIRAGANA_KATAKANA;
    }
    else if ( (nConversion & 0x20) == 32 ) // vbHiragana
    {
        nType |= i18n::TransliterationModules_KATAKANA_HIRAGANA;
    }
    OUString aNewStr( aOldStr );
    if( nType != 0 )
    {
        uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();
        ::utl::TransliterationWrapper aTransliterationWrapper( xContext, nType );
        uno::Sequence<sal_Int32> aOffsets;
        aTransliterationWrapper.loadModuleIfNeeded( nLanguage );
        aNewStr = aTransliterationWrapper.transliterate( aOldStr, nLanguage, 0, nOldLen, &aOffsets );
    }

    if ( (nConversion & 0x40) == 64 ) // vbUnicode
    {
        // convert the string to byte string, preserving unicode (2 bytes per character)
        sal_Int32 nSize = aNewStr.getLength()*2;
        const sal_Unicode* pSrc = aNewStr.getStr();
        std::unique_ptr<sal_Char[]> pChar(new sal_Char[nSize+1]);
        for( sal_Int32 i=0; i < nSize; i++ )
        {
            pChar[i] = static_cast< sal_Char >( (i%2) ? ((*pSrc) >> 8) & 0xff : (*pSrc) & 0xff );
            if( i%2 )
            {
                pSrc++;
            }
        }
        pChar[nSize] = '\0';
        OString aOStr(pChar.get());

        // there is no concept about default codepage in unix. so it is incorrectly in unix
        OUString aOUStr = OStringToOUString(aOStr, osl_getThreadTextEncoding());
        rPar.Get(0)->PutString( aOUStr );
        return;
    }
    else if ( (nConversion & 0x80) == 128 ) // vbFromUnicode
    {
        // there is no concept about default codepage in unix. so it is incorrectly in unix
        OString aOStr = OUStringToOString(aNewStr,osl_getThreadTextEncoding());
        const sal_Char* pChar = aOStr.getStr();
        sal_Int32 nArraySize = aOStr.getLength();
        SbxDimArray* pArray = new SbxDimArray(SbxBYTE);
        bool bIncIndex = (IsBaseIndexOne() && SbiRuntime::isVBAEnabled() );
        if(nArraySize)
        {
            if( bIncIndex )
            {
                pArray->AddDim( 1, nArraySize );
            }
            else
            {
                pArray->AddDim( 0, nArraySize-1 );
            }
        }
        else
        {
            pArray->unoAddDim( 0, -1 );
        }

        for( sal_Int32 i=0; i< nArraySize; i++)
        {
            SbxVariable* pNew = new SbxVariable( SbxBYTE );
            pNew->PutByte(*pChar);
            pChar++;
            pNew->SetFlag( SbxFlagBits::Write );
            short aIdx[1];
            aIdx[0] = i;
            if( bIncIndex )
            {
                ++aIdx[0];
            }
            pArray->Put(pNew, aIdx);
        }

        SbxVariableRef refVar = rPar.Get(0);
        SbxFlagBits nFlags = refVar->GetFlags();
        refVar->ResetFlag( SbxFlagBits::Fixed );
        refVar->PutObject( pArray );
        refVar->SetFlags( nFlags );
        refVar->SetParameters( nullptr );
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }


    SbxBase* pObj = static_cast<SbxObject*>(rPar.Get(1)->GetObject());
    if ( pObj )
    {
        if (SbUserFormModule* pModule = dynamic_cast<SbUserFormModule*>(pObj))
        {
            pModule->Load();
        }
        else if (SbxObject* pSbxObj = dynamic_cast<SbxObject*>(pObj))
        {
            SbxVariable* pVar = pSbxObj->Find("Load", SbxClassType::Method);
            if( pVar )
            {
                pVar->GetInteger();
            }
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
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }


    SbxBase* pObj = static_cast<SbxObject*>(rPar.Get(1)->GetObject());
    if ( pObj )
    {
        if (SbUserFormModule* pFormModule = dynamic_cast<SbUserFormModule*>(pObj))
        {
            pFormModule->Unload();
        }
        else if (SbxObject *pSbxObj = dynamic_cast<SbxObject*>(pObj))
        {
            SbxVariable* pVar = pSbxObj->Find("Unload", SbxClassType::Method);
            if( pVar )
            {
                pVar->GetInteger();
            }
        }
    }
}

RTLFUNC(LoadPicture)
{
    (void)pBasic;
    (void)bWrite;

    if( rPar.Count() != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aFileURL = getFullPath( rPar.Get(1)->GetOUString() );
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream( aFileURL, StreamMode::READ ));
    if( pStream )
    {
        Bitmap aBmp;
        ReadDIB(aBmp, *pStream, true);
        Graphic aGraphic(aBmp);

        SbxObjectRef xRef = new SbStdPicture;
        static_cast<SbStdPicture*>(xRef.get())->SetGraphic( aGraphic );
        rPar.Get(0)->PutObject( xRef.get() );
    }
}

RTLFUNC(SavePicture)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if( rPar.Count() != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbxBase* pObj = static_cast<SbxObject*>(rPar.Get(1)->GetObject());
    if (SbStdPicture *pPicture = dynamic_cast<SbStdPicture*>(pObj))
    {
        SvFileStream aOStream( rPar.Get(2)->GetOUString(), StreamMode::WRITE | StreamMode::TRUNC );
        Graphic aGraphic = pPicture->GetGraphic();
        WriteGraphic( aOStream, aGraphic );
    }
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
        2, // RET_CANCEL is 0
        1, // RET_OK     is 1
        6, // RET_YES    is 2
        7, // RET_NO     is 3
        4  // RET_RETRY  is 4
    };


    sal_uInt16 nArgCount = (sal_uInt16)rPar.Count();
    if( nArgCount < 2 || nArgCount > 6 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    WinBits nWinBits;
    WinBits nType = 0; // MB_OK
    if( nArgCount >= 3 )
        nType = (WinBits)rPar.Get(2)->GetInteger();
    WinBits nStyle = nType;
    nStyle &= 15; // delete bits 4-16
    if( nStyle > 5 )
    {
        nStyle = 0;
    }
    nWinBits = nStyleMap[ nStyle ];

    WinBits nWinDefBits;
    nWinDefBits = (WB_DEF_OK | WB_DEF_RETRY | WB_DEF_YES);
    if( nType & 256 )
    {
        if( nStyle == 5 )
        {
            nWinDefBits = WB_DEF_CANCEL;
        }
        else if( nStyle == 2 )
        {
            nWinDefBits = WB_DEF_RETRY;
        }
        else
        {
            nWinDefBits = (WB_DEF_CANCEL | WB_DEF_RETRY | WB_DEF_NO);
        }
    }
    else if( nType & 512 )
    {
        if( nStyle == 2)
        {
            nWinDefBits = WB_DEF_IGNORE;
        }
        else
        {
            nWinDefBits = WB_DEF_CANCEL;
        }
    }
    else if( nStyle == 2)
    {
        nWinDefBits = WB_DEF_CANCEL;
    }
    nWinBits |= nWinDefBits;

    OUString aMsg = rPar.Get(1)->GetOUString();
    OUString aTitle;
    if( nArgCount >= 4 )
    {
        aTitle = rPar.Get(3)->GetOUString();
    }
    else
    {
        aTitle = Application::GetDisplayName();
    }

    nType &= (16+32+64);
    VclPtr<MessBox> pBox;

    SolarMutexGuard aSolarGuard;

    vcl::Window* pParent = Application::GetDefDialogParent();
    switch( nType )
    {
    case 16:
        pBox.reset(VclPtr<ErrorBox>::Create( pParent, nWinBits, aMsg ));
        break;
    case 32:
        pBox.reset(VclPtr<QueryBox>::Create( pParent, nWinBits, aMsg ));
        break;
    case 48:
        pBox.reset(VclPtr<WarningBox>::Create( pParent, nWinBits, aMsg ));
        break;
    case 64:
        pBox.reset(VclPtr<InfoBox>::Create( pParent, nWinBits, aMsg ));
        break;
    default:
        pBox.reset(VclPtr<MessBox>::Create( pParent, nWinBits, aTitle, aMsg ));
    }
    pBox->SetText( aTitle );
    short nRet = pBox->Execute();
    sal_Int16 nMappedRet;
    if( nStyle == 2 )
    {
        nMappedRet = nRet;
        if( nMappedRet == 0 )
        {
            nMappedRet = 3; // Abort
        }
    }
    else
    {
        nMappedRet = nButtonMap[ nRet ];
    }
    rPar.Get(0)->PutInteger( nMappedRet );
    pBox.disposeAndClear();
}

RTLFUNC(SetAttr)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if ( rPar.Count() == 3 )
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        SbAttributes nFlags = static_cast<SbAttributes>( rPar.Get(2)->GetInteger() );

        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    bool bReadOnly = bool(nFlags & SbAttributes::READONLY);
                    xSFI->setReadOnly( aStr, bReadOnly );
                    bool bHidden   = bool(nFlags & SbAttributes::HIDDEN);
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
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

RTLFUNC(Reset)
{
    (void)pBasic;
    (void)bWrite;
    (void)rPar;

    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    if (pIO)
    {
        pIO->CloseAll();
    }
}

RTLFUNC(DumpAllObjects)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nArgCount = (sal_uInt16)rPar.Count();
    if( nArgCount < 2 || nArgCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else if( !pBasic )
    {
        StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
    }
    else
    {
        SbxObject* p = pBasic;
        while( p->GetParent() )
        {
            p = p->GetParent();
        }
        SvFileStream aStrm( rPar.Get( 1 )->GetOUString(),
                            StreamMode::WRITE | StreamMode::TRUNC );
        p->Dump( aStrm, rPar.Get( 2 )->GetBool() );
        aStrm.Close();
        if( aStrm.GetError() != SVSTREAM_OK )
        {
            StarBASIC::Error( ERRCODE_BASIC_IO_ERROR );
        }
    }
}


RTLFUNC(FileExists)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        bool bExists = false;

        if( hasUno() )
        {
            uno::Reference< ucb::XSimpleFileAccess3 > xSFI = getFileAccess();
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
            FileBase::RC nRet = DirectoryItem::get( getFullPath( aStr ), aItem );
            bExists = (nRet == FileBase::E_None);
        }
        rPar.Get(0)->PutBool( bExists );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

RTLFUNC(Partition)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    sal_Int32 nNumber = rPar.Get(1)->GetLong();
    sal_Int32 nStart = rPar.Get(2)->GetLong();
    sal_Int32 nStop = rPar.Get(3)->GetLong();
    sal_Int32 nInterval = rPar.Get(4)->GetLong();

    if( nStart < 0 || nStop <= nStart || nInterval < 1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // the Partition function inserts leading spaces before lowervalue and uppervalue
    // so that they both have the same number of characters as the string
    // representation of the value (Stop + 1). This ensures that if you use the output
    // of the Partition function with several values of Number, the resulting text
    // will be handled properly during any subsequent sort operation.

    // calculate the  maximun number of characters before lowervalue and uppervalue
    OUString aBeforeStart = OUString::number( nStart - 1 );
    OUString aAfterStop = OUString::number( nStop + 1 );
    sal_Int32 nLen1 = aBeforeStart.getLength();
    sal_Int32 nLen2 = aAfterStop.getLength();
    sal_Int32 nLen = nLen1 >= nLen2 ? nLen1:nLen2;

    OUStringBuffer aRetStr( nLen * 2 + 1);
    OUString aLowerValue;
    OUString aUpperValue;
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
        aLowerValue = OUString::number( nLowerValue );
        aUpperValue = OUString::number( nUpperValue );
    }

    nLen1 = aLowerValue.getLength();
    nLen2 = aUpperValue.getLength();

    if( nLen > nLen1 )
    {
        // appending the leading spaces for the lowervalue
        for ( sal_Int32 i= (nLen - nLen1) ; i > 0; --i )
        {
            aRetStr.append(" ");
        }
    }
    aRetStr.append( aLowerValue ).append(":");
    if( nLen > nLen2 )
    {
        // appending the leading spaces for the uppervalue
        for ( sal_Int32 i= (nLen - nLen2) ; i > 0; --i )
        {
            aRetStr.append(" ");
        }
    }
    aRetStr.append( aUpperValue );
    rPar.Get(0)->PutString( aRetStr.makeStringAndClear());
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
    {
        nDiffDays = (long)(rDate - aRefDate);
    }
    nDiffDays += 2; // adjustment VisualBasic: 1.Jan.1900 == 2
    return nDiffDays;
}

sal_Int16 implGetDateYear( double aDate )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long) aDate;
    nDays -= 2; // standardize: 1.1.1900 => 0.0
    aRefDate += nDays;
    sal_Int16 nRet = aRefDate.GetYear();
    return nRet;
}

bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet )
{
#if HAVE_FEATURE_SCRIPTING
    if ( nYear < 30 && SbiRuntime::isVBAEnabled() )
    {
        nYear += 2000;
    }
    else
#endif
    {
        if ( nYear < 100 )
        {
            nYear += 1900;
        }
    }
    Date aCurDate( nDay, nMonth, nYear );
    if ((nYear < 100 || nYear > 9999) )
    {
#if HAVE_FEATURE_SCRIPTING
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
#endif
        return false;
    }

#if HAVE_FEATURE_SCRIPTING
    if ( !SbiRuntime::isVBAEnabled() )
#endif
    {
        if ( (nMonth < 1 || nMonth > 12 )||
             (nDay < 1 || nDay > 31 ) )
        {
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
#endif
            return false;
        }
    }
#if HAVE_FEATURE_SCRIPTING
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
            {
                nYearAdj = ( ( nMonth -12 ) / 12 );
            }
            aCurDate.AddYears( nYearAdj );
        }

        // adjust day value,
        // e.g. 2000, 2, 0 = 2000, 1, 31 or the last day of the previous month
        //      2000, 1, 32 = 2000, 2, 1 or the first day of the following month
        if( ( nDay < 1 ) || ( nDay > aCurDate.GetDaysInMonth() ) )
        {
            aCurDate += nDay - 1;
        }
        else
        {
            aCurDate.SetDay( nDay );
        }
    }
#endif

    long nDiffDays = GetDayDiff( aCurDate );
    rdRet = (double)nDiffDays;
    return true;
}

double implTimeSerial( sal_Int16 nHours, sal_Int16 nMinutes, sal_Int16 nSeconds )
{
    return
        static_cast<double>( nHours * ::tools::Time::secondPerHour +
                             nMinutes * ::tools::Time::secondPerMinute +
                             nSeconds)
        /
        static_cast<double>( ::tools::Time::secondPerDay );
}

bool implDateTimeSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay,
                         sal_Int16 nHour, sal_Int16 nMinute, sal_Int16 nSecond,
                         double& rdRet )
{
    double dDate;
    if(!implDateSerial(nYear, nMonth, nDay, dDate))
        return false;
    rdRet += dDate + implTimeSerial(nHour, nMinute, nSecond);
    return true;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
