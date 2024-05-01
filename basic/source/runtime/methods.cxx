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
#include <vcl/wintypes.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>
#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <tools/urlobj.hxx>
#include <osl/time.h>
#include <unotools/charclass.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/wincodepage.hxx>
#include <tools/wldcrd.hxx>
#include <i18nlangtag/lang.h>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <comphelper/DirectoryHelper.hxx>

#include <runtime.hxx>
#include <sbunoobj.hxx>
#include <osl/file.hxx>
#include <errobject.hxx>

#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/script/XErrorQuery.hpp>
#include <ooo/vba/VbStrConv.hpp>
#include <ooo/vba/VbTriState.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <memory>
#include <random>
#include <string_view>
#include <o3tl/char16_t2wchar_t.hxx>

// include search util
#include <com/sun/star/i18n/Transliteration.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <i18nutil/searchopt.hxx>
#include <unotools/textsearch.hxx>
#include <svl/numformat.hxx>

#include <date.hxx>
#include <sbstdobj.hxx>
#include <rtlproto.hxx>
#include <image.hxx>
#include <iosys.hxx>
#include "ddectrl.hxx"
#include <sbintern.hxx>
#include <basic/vbahelper.hxx>

#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sbobjmod.hxx>
#include <sbxmod.hxx>

#ifdef _WIN32
#include <prewin.h>
#include <direct.h>
#include <io.h>
#include <postwin.h>
#else
#include <unistd.h>
#endif

#include <vcl/TypeSerializer.hxx>

using namespace comphelper;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

static sal_Int32 GetDayDiff(const Date& rDate) { return rDate - Date(1899'12'30); }

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

static const CharClass& GetCharClass()
{
    static CharClass aCharClass( Application::GetSettings().GetLanguageTag() );
    return aCharClass;
}

static bool isFolder( FileStatus::Type aType )
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
    static uno::Reference< ucb::XSimpleFileAccess3 > xSFI = ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() );
    return xSFI;
}


// Properties and methods lie down the return value at the Get (bPut = sal_False) in the
// element 0 of the Argv; the value of element 0 is saved at Put (bPut = sal_True)

// CreateObject( class )

void SbRtl_CreateObject(StarBASIC * pBasic, SbxArray & rPar, bool)
{
    if( rPar.Count() < 2 )
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    OUString aClass(rPar.Get(1)->GetOUString());
    SbxObjectRef p = SbxBase::CreateObject( aClass );
    if( !p.is() )
        return StarBASIC::Error( ERRCODE_BASIC_CANNOT_LOAD );

    // Convenience: enter BASIC as parent
    p->SetParent( pBasic );
    rPar.Get(0)->PutObject(p.get());
}

// Error( n )

void SbRtl_Error(StarBASIC * pBasic, SbxArray & rPar, bool)
{
    if( !pBasic )
        return StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );

    OUString aErrorMsg;
    ErrCode nErr = ERRCODE_NONE;
    sal_Int32 nCode = 0;
    if (rPar.Count() == 1)
    {
        nErr = StarBASIC::GetErrBasic();
        aErrorMsg = StarBASIC::GetErrorMsg();
    }
    else
    {
        nCode = rPar.Get(1)->GetLong();
        if( nCode > 65535 )
        {
            StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
        }
        else
        {
            nErr = StarBASIC::GetSfxFromVBError( static_cast<sal_uInt16>(nCode) );
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
    // If this rtlfunc 'Error' passed an errcode the same as the active Err Objects's
    // current err then  return the description for the error message if it is set
    // ( complicated isn't it ? )
    if (bVBA && rPar.Count() > 1)
    {
        uno::Reference< ooo::vba::XErrObject > xErrObj( SbxErrObject::getUnoErrObject() );
        if ( xErrObj.is() && xErrObj->getNumber() == nCode && !xErrObj->getDescription().isEmpty() )
        {
            tmpErrMsg = xErrObj->getDescription();
        }
    }
    rPar.Get(0)->PutString(tmpErrMsg);
}

// Sinus

void SbRtl_Sin(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    rPar.Get(0)->PutDouble(sin(pArg->GetDouble()));
}


void SbRtl_Cos(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    rPar.Get(0)->PutDouble(cos(pArg->GetDouble()));
}


void SbRtl_Atn(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    rPar.Get(0)->PutDouble(atan(pArg->GetDouble()));
}


void SbRtl_Abs(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    rPar.Get(0)->PutDouble(fabs(pArg->GetDouble()));
}


void SbRtl_Asc(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    OUString aStr( pArg->GetOUString() );
    if ( aStr.isEmpty())
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        rPar.Get(0)->PutEmpty();
        return;
    }
    sal_Unicode aCh = aStr[0];
    rPar.Get(0)->PutLong(aCh);
}

static void implChr( SbxArray& rPar, bool bChrW )
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);

    OUString aStr;
    if( !bChrW && SbiRuntime::isVBAEnabled() )
    {
        char c = static_cast<char>(pArg->GetByte());
        aStr = OUString(&c, 1, osl_getThreadTextEncoding());
    }
    else
    {
        // Map negative 16-bit values to large positive ones, so that code like Chr(&H8000)
        // still works after the fix for tdf#62326 changed those four-digit hex notations to
        // produce negative values:
        sal_Int32 aCh = pArg->GetLong();
        if (aCh < -0x8000 || aCh > 0xFFFF)
        {
            StarBASIC::Error(ERRCODE_BASIC_MATH_OVERFLOW);
            aCh = 0;
        }
        aStr = OUString(static_cast<sal_Unicode>(aCh));
    }
    rPar.Get(0)->PutString(aStr);
}

void SbRtl_Chr(StarBASIC *, SbxArray & rPar, bool)
{
    implChr( rPar, false/*bChrW*/ );
}

void SbRtl_ChrW(StarBASIC *, SbxArray & rPar, bool)
{
    implChr( rPar, true/*bChrW*/ );
}

#if defined _WIN32

namespace {

extern "C" void invalidParameterHandler(
    wchar_t const * expression, wchar_t const * function, wchar_t const * file, unsigned int line,
    uintptr_t)
{
    SAL_INFO(
        "basic",
        "invalid parameter during _wgetdcwd; \""
            << (expression ? OUString(o3tl::toU(expression)) : OUString("???"))
            << "\" (" << (function ? OUString(o3tl::toU(function)) : OUString("???")) << ") at "
            << (file ? OUString(o3tl::toU(file)) : OUString("???")) << ":" << line);
}

}

#endif

void SbRtl_CurDir(StarBASIC *, SbxArray & rPar, bool)
{
    // #57064 Although this function doesn't work with DirEntry, it isn't touched
    // by the adjustment to virtual URLs, as, using the DirEntry-functionality,
    // there's no possibility to detect the current one in a way that a virtual URL
    // could be delivered.

    if (rPar.Count() > 2)
       return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

#if defined(_WIN32)
    int nCurDir = 0;  // Current dir // JSM
    if (rPar.Count() == 2)
    {
        OUString aDrive = rPar.Get(1)->GetOUString();
        if ( aDrive.getLength() != 1 )
            return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

        auto c = rtl::toAsciiUpperCase(aDrive[0]);
        if ( !rtl::isAsciiUpperCase( c ) )
            return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

        nCurDir = c - 'A' + 1;
    }
    wchar_t pBuffer[ _MAX_PATH ];
    // _wgetdcwd calls the C runtime's invalid parameter handler (which by default terminates the
    // process) if nCurDir does not correspond to an existing drive, so temporarily set a "harmless"
    // handler:
    auto const handler = _set_thread_local_invalid_parameter_handler(&invalidParameterHandler);
    auto const ok = _wgetdcwd( nCurDir, pBuffer, _MAX_PATH ) != nullptr;
    _set_thread_local_invalid_parameter_handler(handler);
    if ( !ok )
        return StarBASIC::Error( ERRCODE_BASIC_NO_DEVICE );

    rPar.Get(0)->PutString(OUString(o3tl::toU(pBuffer)));

#else

    const int PATH_INCR = 250;

    int nSize = PATH_INCR;
    std::unique_ptr<char[]> pMem;
    while( true )
    {
        pMem.reset(new char[nSize]);
        if( !pMem )
            return StarBASIC::Error( ERRCODE_BASIC_NO_MEMORY );

        if( getcwd( pMem.get(), nSize-1 ) != nullptr )
        {
            rPar.Get(0)->PutString(OUString::createFromAscii(pMem.get()));
            return;
        }
        if( errno != ERANGE )
            return StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );

        nSize += PATH_INCR;
    };

#endif
}

void SbRtl_ChDir(StarBASIC * pBasic, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    // VBA: track current directory per document type (separately for Writer, Calc, Impress, etc.)
    if( SbiRuntime::isVBAEnabled() )
    {
        ::basic::vba::registerCurrentDirectory(getDocumentModel(pBasic),
                                                rPar.Get(1)->GetOUString());
    }
}

void SbRtl_ChDrive(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
}


// Implementation of StepRENAME with UCB
void implStepRenameUCB( const OUString& aSource, const OUString& aDest )
{
    const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
    if( !xSFI.is() )
        return;

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

// Implementation of StepRENAME with OSL
void implStepRenameOSL( const OUString& aSource, const OUString& aDest )
{
    FileBase::RC nRet = File::move( getFullPath( aSource ), getFullPath( aDest ) );
    if( nRet != FileBase::E_None )
    {
        StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
    }
}

void SbRtl_FileCopy(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 3)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    OUString aSource = rPar.Get(1)->GetOUString();
    OUString aDest = rPar.Get(2)->GetOUString();
    if( hasUno() )
    {
        const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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

void SbRtl_Kill(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    OUString aFileSpec = rPar.Get(1)->GetOUString();

    if( hasUno() )
    {
        const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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

void SbRtl_MkDir(StarBASIC * pBasic, SbxArray & rPar, bool bWrite)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

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
            pPar->Insert(pResult.get(), pPar->Count());
            pPar->Insert(pParam.get(), pPar->Count());
            SbRtl_CurDir( pBasic, *pPar, bWrite );

            OUString sCurPathURL;
            File::getFileURLFromSystemPath(pPar->Get(0)->GetOUString(), sCurPathURL);

            aURLObj.SetURL( sCurPathURL );
            aURLObj.Append( aPath );
            File::getSystemPathFromFileURL(aURLObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri  ),aPath ) ;
        }
    }

    if( hasUno() )
    {
        const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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


static void implRemoveDirRecursive( const OUString& aDirPath )
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
        return StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
    }

    Directory aDir( aDirPath );
    nRet = aDir.open();
    if( nRet != FileBase::E_None )
    {
        return StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
    }
    aDir.close();

    comphelper::DirectoryHelper::deleteDirRecursively(aDirPath);
}


void SbRtl_RmDir(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 2)
    {
        OUString aPath = rPar.Get(1)->GetOUString();
        if( hasUno() )
        {
            const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    if( !xSFI->isFolder( aPath ) )
                    {
                        return StarBASIC::Error( ERRCODE_BASIC_PATH_NOT_FOUND );
                    }
                    SbiInstance* pInst = GetSbData()->pInst;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    if( bCompatibility )
                    {
                        Sequence< OUString > aContent = xSFI->getFolderContents( aPath, true );
                        if( aContent.hasElements() )
                        {
                            return StarBASIC::Error( ERRCODE_BASIC_ACCESS_ERROR );
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

void SbRtl_SendKeys(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(ERRCODE_BASIC_NOT_IMPLEMENTED);
}

void SbRtl_Exp(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    double aDouble = rPar.Get(1)->GetDouble();
    aDouble = exp( aDouble );
    checkArithmeticOverflow( aDouble );
    rPar.Get(0)->PutDouble(aDouble);
}

void SbRtl_FileLen(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    SbxVariableRef pArg = rPar.Get(1);
    OUString aStr( pArg->GetOUString() );
    sal_Int32 nLen = 0;
    if( hasUno() )
    {
        const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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
        (void)DirectoryItem::get( getFullPath( aStr ), aItem );
        FileStatus aFileStatus( osl_FileStatus_Mask_FileSize );
        (void)aItem.getFileStatus( aFileStatus );
        nLen = static_cast<sal_Int32>(aFileStatus.getFileSize());
    }
    rPar.Get(0)->PutLong(nLen);
}



void SbRtl_Hex(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    SbxVariableRef pArg = rPar.Get(1);
    // converting value to unsigned and limit to 2 or 4 byte representation
    sal_uInt32 nVal = pArg->IsInteger() ?
        static_cast<sal_uInt16>(pArg->GetInteger()) :
        static_cast<sal_uInt32>(pArg->GetLong());
    rPar.Get(0)->PutString(OUString::number(nVal, 16).toAsciiUpperCase());
}

void SbRtl_FuncCaller(StarBASIC *, SbxArray & rPar, bool)
{
    if ( SbiRuntime::isVBAEnabled() &&  GetSbData()->pInst && GetSbData()->pInst->pRun )
    {
        if ( GetSbData()->pInst->pRun->GetExternalCaller() )
            *rPar.Get(0) = *GetSbData()->pInst->pRun->GetExternalCaller();
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

void SbRtl_InStr(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count() - 1;
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
        const OUString& rToken = rPar.Get(nFirstStringPos + 1)->GetOUString();

        // #97545 Always find empty string
        if( rToken.isEmpty() )
        {
            nPos = nStartPos;
        }
        else
        {
            const OUString& rStr1 = rPar.Get(nFirstStringPos)->GetOUString();
            const sal_Int32 nrStr1Len = rStr1.getLength();
            if (nStartPos > nrStr1Len)
            {
                // Start position is greater than the string being searched
                nPos = 0;
            }
            else
            {
                if( !bTextMode )
                {
                    nPos = rStr1.indexOf( rToken, nStartPos - 1 ) + 1;
                }
                else
                {
                    // tdf#139840 - case-insensitive operation for non-ASCII characters
                    i18nutil::SearchOptions2 aSearchOptions;
                    aSearchOptions.searchString = rToken;
                    aSearchOptions.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
                    aSearchOptions.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
                    utl::TextSearch textSearch(aSearchOptions);

                    sal_Int32 nStart = nStartPos - 1;
                    sal_Int32 nEnd = nrStr1Len;
                    nPos = textSearch.SearchForward(rStr1, &nStart, &nEnd) ? nStart + 1 : 0;
                }
            }
        }
        rPar.Get(0)->PutLong(nPos);
    }
}


// InstrRev(string1, string2[, start[, compare]])

void SbRtl_InStrRev(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count() - 1;
    if ( nArgCount < 2 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    const OUString aStr1 = rPar.Get(1)->GetOUString();
    const OUString aToken = rPar.Get(2)->GetOUString();

    sal_Int32 nStartPos = -1;
    if ( nArgCount >= 3 )
    {
        nStartPos = rPar.Get(3)->GetLong();
        if( nStartPos <= 0 && nStartPos != -1 )
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
    const sal_Int32 nStrLen = aStr1.getLength();
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
               // tdf#143332 - case-insensitive operation for non-ASCII characters
               i18nutil::SearchOptions2 aSearchOptions;
               aSearchOptions.searchString = aToken;
               aSearchOptions.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
               aSearchOptions.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
               utl::TextSearch textSearch(aSearchOptions);

               sal_Int32 nStart = 0;
               sal_Int32 nEnd = nStartPos;
               nPos = textSearch.SearchBackward(aStr1, &nEnd, &nStart) ? nStart : 0;
            }
        }
    }
    rPar.Get(0)->PutLong(nPos);
}


/*
    Int( 2.8 )  =  2.0
    Int( -2.8 ) = -3.0
    Fix( 2.8 )  =  2.0
    Fix( -2.8 ) = -2.0    <- !!
*/

void SbRtl_Int(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    double aDouble= pArg->GetDouble();
    /*
        floor( 2.8 ) =  2.0
        floor( -2.8 ) = -3.0
    */
    aDouble = floor( aDouble );
    rPar.Get(0)->PutDouble(aDouble);
}


void SbRtl_Fix(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    double aDouble = pArg->GetDouble();
    if ( aDouble >= 0.0 )
        aDouble = floor( aDouble );
    else
        aDouble = ceil( aDouble );
    rPar.Get(0)->PutDouble(aDouble);
}


void SbRtl_LCase(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    const CharClass& rCharClass = GetCharClass();
    OUString aStr(rPar.Get(1)->GetOUString());
    aStr = rCharClass.lowercase(aStr);
    rPar.Get(0)->PutString(aStr);
}

void SbRtl_Left(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 3)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    OUString aStr(rPar.Get(1)->GetOUString());
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
    rPar.Get(0)->PutString(aStr);
}

void SbRtl_Log(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    double aArg = rPar.Get(1)->GetDouble();
    if ( aArg <= 0 )
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    double d = log( aArg );
    checkArithmeticOverflow( d );
    rPar.Get(0)->PutDouble(d);
}

void SbRtl_LTrim(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    OUString aStr(comphelper::string::stripStart(rPar.Get(1)->GetOUString(), ' '));
    rPar.Get(0)->PutString(aStr);
}


// Mid( String, nStart, nLength )

void SbRtl_Mid(StarBASIC *, SbxArray & rPar, bool bWrite)
{
    int nArgCount = rPar.Count() - 1;
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
                sal_Int32 nArgLen = aArgStr.getLength();
                if( nStartPos > nArgLen )
                {
                    SbiInstance* pInst = GetSbData()->pInst;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    if( bCompatibility )
                    {
                        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                    }
                    nStartPos = nArgLen;
                }

                OUString aReplaceStr = rPar.Get(4)->GetOUString();
                sal_Int32 nReplaceStrLen = aReplaceStr.getLength();
                sal_Int32 nReplaceLen;
                if( bWriteNoLenParam )
                {
                    nReplaceLen = nArgLen - nStartPos;
                }
                else
                {
                    nReplaceLen = nLen;
                    if( nReplaceLen < 0 || nReplaceLen > nArgLen - nStartPos )
                    {
                        nReplaceLen = nArgLen - nStartPos;
                    }
                }

                OUStringBuffer aResultStr(aArgStr);
                sal_Int32 nErase = nReplaceLen;
                aResultStr.remove( nStartPos, nErase );
                aResultStr.insert(
                    nStartPos, aReplaceStr.getStr(), std::min(nReplaceLen, nReplaceStrLen));

                rPar.Get(1)->PutString(aResultStr.makeStringAndClear());
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
                rPar.Get(0)->PutString(aResultStr);
            }
        }
    }
}

void SbRtl_Oct(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariableRef pArg = rPar.Get(1);
    // converting value to unsigned and limit to 2 or 4 byte representation
    sal_uInt32 nVal = pArg->IsInteger() ?
        static_cast<sal_uInt16>(pArg->GetInteger()) :
        static_cast<sal_uInt32>(pArg->GetLong());
    rPar.Get(0)->PutString(OUString::number(nVal, 8));
}

// Replace(expression, find, replace[, start[, count[, compare]]])

void SbRtl_Replace(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count() - 1;
    if ( nArgCount < 3 || nArgCount > 6 )
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    sal_Int32 lStartPos = 1;
    if (nArgCount >= 4)
    {
        if (rPar.Get(4)->GetType() != SbxEMPTY)
        {
            lStartPos = rPar.Get(4)->GetLong();
        }
        if (lStartPos < 1)
        {
            return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }
    --lStartPos; // Make it 0-based

    sal_Int32 lCount = -1;
    if (nArgCount >= 5)
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
        {
            lCount = rPar.Get(5)->GetLong();
        }
        if (lCount < -1)
        {
            return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }

    bool bCaseInsensitive;
    if (nArgCount == 6)
    {
        bCaseInsensitive = rPar.Get(6)->GetInteger();
    }
    else
    {
        SbiInstance* pInst = GetSbData()->pInst;
        if (pInst && pInst->IsCompatibility())
        {
            SbiRuntime* pRT = pInst->pRun;
            bCaseInsensitive = pRT && pRT->IsImageFlag(SbiImageFlags::COMPARETEXT);
        }
        else
        {
            bCaseInsensitive = true;
        }
    }

    const OUString aExpStr = rPar.Get(1)->GetOUString();
    OUString aFindStr = rPar.Get(2)->GetOUString();
    const OUString aReplaceStr = rPar.Get(3)->GetOUString();

    OUString aSrcStr(aExpStr);
    sal_Int32 nPrevPos = std::min(lStartPos, aSrcStr.getLength());
    css::uno::Sequence<sal_Int32> aOffset;
    if (bCaseInsensitive)
    {
        // tdf#132389: case-insensitive operation for non-ASCII characters
        // tdf#142487: use css::i18n::Transliteration to correctly handle ß -> ss expansion
        // tdf#132388: We can't use utl::TextSearch (css::i18n::XTextSearch), because each call to
        //             css::i18n::XTextSearch::SearchForward transliterates input string, making
        //             performance of repeated calls unacceptable
        auto xTrans = css::i18n::Transliteration::create(comphelper::getProcessComponentContext());
        xTrans->loadModule(css::i18n::TransliterationModules_IGNORE_CASE, {});
        aFindStr = xTrans->transliterate(aFindStr, 0, aFindStr.getLength(), aOffset);
        aSrcStr = xTrans->transliterate(aSrcStr, nPrevPos, aSrcStr.getLength() - nPrevPos, aOffset);
        nPrevPos = std::distance(aOffset.begin(),
                                 std::lower_bound(aOffset.begin(), aOffset.end(), nPrevPos));
    }

    auto getExpStrPos = [aOffset, nExpLen = aExpStr.getLength()](sal_Int32 nSrcStrPos) -> sal_Int32
    {
        assert(!aOffset.hasElements() || aOffset.getLength() >= nSrcStrPos);
        if (!aOffset.hasElements())
            return nSrcStrPos;
        return aOffset.getLength() > nSrcStrPos ? aOffset[nSrcStrPos] : nExpLen;
    };

    // Note: the result starts from lStartPos, removing everything to the left. See i#94895.
    OUStringBuffer sResult(aSrcStr.getLength() - nPrevPos);
    sal_Int32 nCounts = 0;
    while (lCount == -1 || lCount > nCounts)
    {
        sal_Int32 nPos = aSrcStr.indexOf(aFindStr, nPrevPos);
        if (nPos < 0)
            break;

        lStartPos = getExpStrPos(nPrevPos);
        sResult.append(aExpStr.getStr() + lStartPos, getExpStrPos(nPos) - lStartPos);
        sResult.append(aReplaceStr);
        nPrevPos = nPos + aFindStr.getLength();
        nCounts++;
    }
    lStartPos = getExpStrPos(nPrevPos);
    sResult.append(aExpStr.getStr() + lStartPos, aExpStr.getLength() - lStartPos);
    rPar.Get(0)->PutString(sResult.makeStringAndClear());
}

void SbRtl_Right(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 3)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

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
    rPar.Get(0)->PutString(aResultStr);
}

void SbRtl_RTL(StarBASIC * pBasic, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutObject(pBasic->getRTL().get());
}

void SbRtl_RTrim(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    OUString aStr(comphelper::string::stripEnd(rPar.Get(1)->GetOUString(), ' '));
    rPar.Get(0)->PutString(aStr);
}

void SbRtl_Sgn(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

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
    rPar.Get(0)->PutInteger(nResult);
}

void SbRtl_Space(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const sal_Int32 nCount = rPar.Get(1)->GetLong();
        OUStringBuffer aBuf(nCount);
        string::padToLength(aBuf, nCount, ' ');
        rPar.Get(0)->PutString(aBuf.makeStringAndClear());
    }
}

void SbRtl_Sqr(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double aDouble = rPar.Get(1)->GetDouble();
        if ( aDouble >= 0 )
        {
            rPar.Get(0)->PutDouble(sqrt(aDouble));
        }
        else
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        }
    }
}

void SbRtl_Str(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr;
        OUString aStrNew("");
        SbxVariableRef pArg = rPar.Get(1);
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
                aStrNew += aStr.subView(iZeroSearch);
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
        rPar.Get(0)->PutString(aStrNew);
    }
}

void SbRtl_StrComp(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 3)
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
    if (rPar.Count() == 4)
        bTextCompare = rPar.Get(3)->GetInteger();

    if( !bCompatibility )
    {
        bTextCompare = !bTextCompare;
    }
    sal_Int32 nRetValue = 0;
    if( bTextCompare )
    {
        ::utl::TransliterationWrapper* pTransliterationWrapper = GetSbData()->pTransliterationWrapper.get();
        if( !pTransliterationWrapper )
        {
            uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();
            GetSbData()->pTransliterationWrapper.reset(
                new ::utl::TransliterationWrapper( xContext,
                    TransliterationFlags::IGNORE_CASE |
                    TransliterationFlags::IGNORE_KANA |
                    TransliterationFlags::IGNORE_WIDTH ) );
            pTransliterationWrapper = GetSbData()->pTransliterationWrapper.get();
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
    rPar.Get(0)->PutInteger(sal::static_int_cast<sal_Int16>(nRetValue));
}

void SbRtl_String(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
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
        if (rPar.Get(2)->GetType() == SbxINTEGER)
        {
            aFiller = static_cast<sal_Unicode>(rPar.Get(2)->GetInteger());
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

void SbRtl_Tab(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    else
    {
        const sal_Int32 nCount = std::max(rPar.Get(1)->GetLong(), sal_Int32(0));
        OUStringBuffer aStr(nCount);
        comphelper::string::padToLength(aStr, nCount, '\t');
        rPar.Get(0)->PutString(aStr.makeStringAndClear());
    }
}

void SbRtl_Tan(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariableRef pArg = rPar.Get(1);
        rPar.Get(0)->PutDouble(tan(pArg->GetDouble()));
    }
}

void SbRtl_UCase(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const CharClass& rCharClass = GetCharClass();
        OUString aStr(rPar.Get(1)->GetOUString());
        aStr = rCharClass.uppercase( aStr );
        rPar.Get(0)->PutString(aStr);
    }
}


void SbRtl_Val(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double nResult = 0.0;
        char* pEndPtr;

        OUString aStr(rPar.Get(1)->GetOUString());

        FilterWhiteSpace( aStr );
        if ( aStr.getLength() > 1 && aStr[0] == '&' )
        {
            int nRadix = 10;
            char aChar = static_cast<char>(aStr[1]);
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
                sal_Int16 nlResult = static_cast<sal_Int16>(strtol( aByteStr.getStr()+2, &pEndPtr, nRadix));
                nResult = static_cast<double>(nlResult);
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

        rPar.Get(0)->PutDouble(nResult);
    }
}


// Helper functions for date conversion
sal_Int16 implGetDateDay( double aDate )
{
    aDate = floor( aDate );
    Date aRefDate(1899'12'30);
    aRefDate.AddDays( aDate );

    sal_Int16 nRet = static_cast<sal_Int16>( aRefDate.GetDay() );
    return nRet;
}

sal_Int16 implGetDateMonth( double aDate )
{
    Date aRefDate(1899'12'30);
    sal_Int32 nDays = static_cast<sal_Int32>(aDate);
    aRefDate.AddDays( nDays );
    sal_Int16 nRet = static_cast<sal_Int16>( aRefDate.GetMonth() );
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
    if( implDateSerial( aUnoDate.Year, aUnoDate.Month, aUnoDate.Day, false, SbDateCorrection::None, dDate ) )
    {
        pVal->PutDate( dDate );
    }
}

// Function to convert date to UNO date (com.sun.star.util.Date)
void SbRtl_CDateToUnoDate(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    unoToSbxValue(rPar.Get(0), Any(SbxDateToUNODate(rPar.Get(1))));
}

// Function to convert date from UNO date (com.sun.star.util.Date)
void SbRtl_CDateFromUnoDate(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2 || rPar.Get(1)->GetType() != SbxOBJECT)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    Any aAny(sbxToUnoValue(rPar.Get(1), cppu::UnoType<css::util::Date>::get()));
    css::util::Date aUnoDate;
    if(aAny >>= aUnoDate)
        SbxDateFromUNODate(rPar.Get(0), aUnoDate);
    else
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
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
void SbRtl_CDateToUnoTime(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    unoToSbxValue(rPar.Get(0), Any(SbxDateToUNOTime(rPar.Get(1))));
}

// Function to convert date from UNO time (com.sun.star.util.Time)
void SbRtl_CDateFromUnoTime(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2 || rPar.Get(1)->GetType() != SbxOBJECT)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    Any aAny(sbxToUnoValue(rPar.Get(1), cppu::UnoType<css::util::Time>::get()));
    css::util::Time aUnoTime;
    if(aAny >>= aUnoTime)
        SbxDateFromUNOTime(rPar.Get(0), aUnoTime);
    else
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
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
void SbRtl_CDateToUnoDateTime(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    unoToSbxValue(rPar.Get(0), Any(SbxDateToUNODateTime(rPar.Get(1))));
}

// Function to convert date from UNO date (com.sun.star.util.Date)
void SbRtl_CDateFromUnoDateTime(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2 || rPar.Get(1)->GetType() != SbxOBJECT)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    Any aAny(sbxToUnoValue(rPar.Get(1), cppu::UnoType<css::util::DateTime>::get()));
    css::util::DateTime aUnoDT;
    if(aAny >>= aUnoDT)
        SbxDateFromUNODateTime(rPar.Get(0), aUnoDT);
    else
        SbxBase::SetError( ERRCODE_BASIC_CONVERSION );
}

// Function to convert date to ISO 8601 date format YYYYMMDD
void SbRtl_CDateToIso(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        double aDate = rPar.Get(1)->GetDate();

        // Date may actually even be -YYYYYMMDD
        char Buffer[11];
        sal_Int16 nYear = implGetDateYear( aDate );
        snprintf( Buffer, sizeof( Buffer ), (nYear < 0 ? "%05d%02d%02d" : "%04d%02d%02d"),
                static_cast<int>(nYear),
                static_cast<int>(implGetDateMonth( aDate )),
                static_cast<int>(implGetDateDay( aDate )) );
        OUString aRetStr = OUString::createFromAscii( Buffer );
        rPar.Get(0)->PutString(aRetStr);
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

// Function to convert date from ISO 8601 date format YYYYMMDD or YYYY-MM-DD
// And even YYMMDD for compatibility, sigh...
void SbRtl_CDateFromIso(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        do
        {
            OUString aStr = rPar.Get(1)->GetOUString();
            if (aStr.isEmpty())
                break;

            // Valid formats are
            // YYYYMMDD    -YYYMMDD     YYYYYMMDD    -YYYYYMMDD    YYMMDD
            // YYYY-MM-DD  -YYYY-MM-DD  YYYYY-MM-DD  -YYYYY-MM-DD

            sal_Int32 nSign = 1;
            if (aStr[0] == '-')
            {
                nSign = -1;
                aStr = aStr.copy(1);
            }
            const sal_Int32 nLen = aStr.getLength();

            // Signed YYMMDD two digit year is invalid.
            if (nLen == 6 && nSign == -1)
                break;

            // Now valid
            // YYYYMMDD    YYYYYMMDD    YYMMDD
            // YYYY-MM-DD  YYYYY-MM-DD
            if (nLen != 6 && (nLen < 8 || 11 < nLen))
                break;

            bool bUseTwoDigitYear = false;
            std::u16string_view aYearStr, aMonthStr, aDayStr;
            if (nLen == 6 || nLen == 8 || nLen == 9)
            {
                // ((Y)YY)YYMMDD
                if (!comphelper::string::isdigitAsciiString(aStr))
                    break;

                const sal_Int32 nMonthPos = (nLen == 8 ? 4 : (nLen == 6 ? 2 : 5));
                if (nMonthPos == 2)
                    bUseTwoDigitYear = true;
                aYearStr  = aStr.subView( 0, nMonthPos );
                aMonthStr = aStr.subView( nMonthPos, 2 );
                aDayStr   = aStr.subView( nMonthPos + 2, 2 );
            }
            else
            {
                // (Y)YYYY-MM-DD
                const sal_Int32 nMonthSep = (nLen == 11 ? 5 : 4);
                if (aStr.indexOf('-') != nMonthSep)
                    break;
                if (aStr.indexOf('-', nMonthSep + 1) != nMonthSep + 3)
                    break;

                aYearStr  = aStr.subView( 0, nMonthSep );
                aMonthStr = aStr.subView( nMonthSep + 1, 2 );
                aDayStr   = aStr.subView( nMonthSep + 4, 2 );
                if (    !comphelper::string::isdigitAsciiString(aYearStr) ||
                        !comphelper::string::isdigitAsciiString(aMonthStr) ||
                        !comphelper::string::isdigitAsciiString(aDayStr))
                    break;
            }

            double dDate;
            if (!implDateSerial( static_cast<sal_Int16>(nSign * o3tl::toInt32(aYearStr)),
                        static_cast<sal_Int16>(o3tl::toInt32(aMonthStr)), static_cast<sal_Int16>(o3tl::toInt32(aDayStr)),
                        bUseTwoDigitYear, SbDateCorrection::None, dDate ))
                break;

            rPar.Get(0)->PutDate(dDate);

            return;
        }
        while (false);

        SbxBase::SetError( ERRCODE_BASIC_BAD_PARAMETER );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

void SbRtl_DateSerial(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 4)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    sal_Int16 nYear = rPar.Get(1)->GetInteger();
    sal_Int16 nMonth = rPar.Get(2)->GetInteger();
    sal_Int16 nDay = rPar.Get(3)->GetInteger();

    double dDate;
    if( implDateSerial( nYear, nMonth, nDay, true, SbDateCorrection::RollOver, dDate ) )
    {
        rPar.Get(0)->PutDate(dDate);
    }
}

void SbRtl_TimeSerial(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 4)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    rPar.Get(0)->PutDate(implTimeSerial(nHour, nMinute, nSecond)); // JSM
}

void SbRtl_DateValue(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // #39629 check GetSbData()->pInst, can be called from the URL line
        std::shared_ptr<SvNumberFormatter> pFormatter;
        if( GetSbData()->pInst )
        {
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
        }
        else
        {
            sal_uInt32 n;   // Dummy
            pFormatter = SbiInstance::PrepareNumberFormatter( n, n, n );
        }

        LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
        sal_uInt32 nIndex = pFormatter->GetStandardIndex( eLangType);
        double fResult;
        OUString aStr(rPar.Get(1)->GetOUString());
        bool bSuccess = pFormatter->IsNumberFormat( aStr, nIndex, fResult );
        SvNumFormatType nType = pFormatter->GetType( nIndex );

        // DateValue("February 12, 1969") raises error if the system locale is not en_US
        // It seems that both locale number formatter and English number
        // formatter are supported in Visual Basic.
        if( !bSuccess && ( eLangType != LANGUAGE_ENGLISH_US ) )
        {
            // Try using LANGUAGE_ENGLISH_US to get the date value.
            nIndex = pFormatter->GetStandardIndex( LANGUAGE_ENGLISH_US);
            bSuccess = pFormatter->IsNumberFormat( aStr, nIndex, fResult );
            nType = pFormatter->GetType( nIndex );
        }

        if(bSuccess && (nType==SvNumFormatType::DATE || nType==SvNumFormatType::DATETIME))
        {
            if ( nType == SvNumFormatType::DATETIME )
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
            rPar.Get(0)->PutDate(fResult);
        }
        else
        {
            StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
        }
    }
}

void SbRtl_TimeValue(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        std::shared_ptr<SvNumberFormatter> pFormatter;
        if( GetSbData()->pInst )
            pFormatter = GetSbData()->pInst->GetNumberFormatter();
        else
        {
            sal_uInt32 n;
            pFormatter = SbiInstance::PrepareNumberFormatter( n, n, n );
        }

        sal_uInt32 nIndex = 0;
        double fResult;
        bool bSuccess = pFormatter->IsNumberFormat(rPar.Get(1)->GetOUString(),
                                                   nIndex, fResult );
        SvNumFormatType nType = pFormatter->GetType(nIndex);
        if(bSuccess && (nType==SvNumFormatType::TIME||nType==SvNumFormatType::DATETIME))
        {
            if ( nType == SvNumFormatType::DATETIME )
            {
                // cut days
                fResult = fmod( fResult, 1 );
            }
            rPar.Get(0)->PutDate(fResult);
        }
        else
        {
            StarBASIC::Error( ERRCODE_BASIC_CONVERSION );
        }
    }
}

void SbRtl_Day(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxVariableRef pArg = rPar.Get(1);
        double aDate = pArg->GetDate();

        sal_Int16 nDay = implGetDateDay( aDate );
        rPar.Get(0)->PutInteger(nDay);
    }
}

void SbRtl_Year(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int16 nYear = implGetDateYear(rPar.Get(1)->GetDate());
        rPar.Get(0)->PutInteger(nYear);
    }
}

sal_Int16 implGetHour( double dDate )
{
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = static_cast<sal_Int32>(nFrac + 0.5);
    sal_Int16 nHour = static_cast<sal_Int16>(nSeconds / 3600);
    return nHour;
}

void SbRtl_Hour(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nHour = implGetHour( nArg );
        rPar.Get(0)->PutInteger(nHour);
    }
}

void SbRtl_Minute(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nMin = implGetMinute( nArg );
        rPar.Get(0)->PutInteger(nMin);
    }
}

void SbRtl_Month(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int16 nMonth = implGetDateMonth(rPar.Get(1)->GetDate());
        rPar.Get(0)->PutInteger(nMonth);
    }
}

sal_Int16 implGetSecond( double dDate )
{
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = static_cast<sal_Int32>(nFrac + 0.5);
    sal_Int16 nTemp = static_cast<sal_Int16>(nSeconds / 3600);
    nSeconds -= nTemp * 3600;
    nTemp = static_cast<sal_Int16>(nSeconds / 60);
    nSeconds -= nTemp * 60;

    sal_Int16 nRet = static_cast<sal_Int16>(nSeconds);
    return nRet;
}

void SbRtl_Second(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double nArg = rPar.Get(1)->GetDate();
        sal_Int16 nSecond = implGetSecond( nArg );
        rPar.Get(0)->PutInteger(nSecond);
    }
}

double Now_Impl()
{
    DateTime aDateTime( DateTime::SYSTEM );
    double aSerial = static_cast<double>(GetDayDiff( aDateTime ));
    tools::Long nSeconds = aDateTime.GetHour();
    nSeconds *= 3600;
    nSeconds += aDateTime.GetMin() * 60;
    nSeconds += aDateTime.GetSec();
    double nDays = static_cast<double>(nSeconds) / (24.0*3600.0);
    aSerial += nDays;
    return aSerial;
}

// Date Now()

void SbRtl_Now(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutDate(Now_Impl()); }

// Date Time()

void SbRtl_Time(StarBASIC *, SbxArray & rPar, bool bWrite)
{
    if ( !bWrite )
    {
        tools::Time aTime( tools::Time::SYSTEM );
        SbxVariable* pMeth = rPar.Get(0);
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
            tools::Long nSeconds=aTime.GetHour();
            nSeconds *= 3600;
            nSeconds += aTime.GetMin() * 60;
            nSeconds += aTime.GetSec();
            double nDays = static_cast<double>(nSeconds) * ( 1.0 / (24.0*3600.0) );
            const Color* pCol;

            std::shared_ptr<SvNumberFormatter> pFormatter;
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
        }
        pMeth->PutString( aRes );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_NOT_IMPLEMENTED );
    }
}

void SbRtl_Timer(StarBASIC *, SbxArray & rPar, bool)
{
    tools::Time aTime( tools::Time::SYSTEM );
    tools::Long nSeconds = aTime.GetHour();
    nSeconds *= 3600;
    nSeconds += aTime.GetMin() * 60;
    nSeconds += aTime.GetSec();
    rPar.Get(0)->PutDate(static_cast<double>(nSeconds));
}


void SbRtl_Date(StarBASIC *, SbxArray & rPar, bool bWrite)
{
    if ( !bWrite )
    {
        Date aToday( Date::SYSTEM );
        double nDays = static_cast<double>(GetDayDiff( aToday ));
        SbxVariable* pMeth = rPar.Get(0);
        if( pMeth->IsString() )
        {
            OUString aRes;
            const Color* pCol;

            std::shared_ptr<SvNumberFormatter> pFormatter;
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

void SbRtl_IsArray(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    rPar.Get(0)->PutBool((rPar.Get(1)->GetType() & SbxARRAY) != 0);
}

void SbRtl_IsObject(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariable* pVar = rPar.Get(1);
    bool bObject = pVar->IsObject();
    SbxBase* pObj = (bObject ? pVar->GetObject() : nullptr);

    if( auto pUnoClass = dynamic_cast<SbUnoClass*>( pObj) )
    {
        bObject = pUnoClass->getUnoClass().is();
    }
    rPar.Get(0)->PutBool(bObject);
}

void SbRtl_IsDate(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    // #46134 only string is converted, all other types result in sal_False
    SbxVariableRef xArg = rPar.Get(1);
    SbxDataType eType = xArg->GetType();
    bool bDate = false;

    if( eType == SbxDATE )
    {
        bDate = true;
    }
    else if( eType == SbxSTRING )
    {
        ErrCode nPrevError = SbxBase::GetError();
        SbxBase::ResetError();

        // force conversion of the parameter to SbxDATE
        xArg->SbxValue::GetDate();

        bDate = !SbxBase::IsError();

        SbxBase::ResetError();
        SbxBase::SetError( nPrevError );
    }
    rPar.Get(0)->PutBool(bDate);
}

void SbRtl_IsEmpty(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariable* pVar = nullptr;
    if( SbiRuntime::isVBAEnabled() )
    {
        pVar = getDefaultProp(rPar.Get(1));
    }
    if ( pVar )
    {
        pVar->Broadcast( SfxHintId::BasicDataWanted );
        rPar.Get(0)->PutBool(pVar->IsEmpty());
    }
    else
    {
        rPar.Get(0)->PutBool(rPar.Get(1)->IsEmpty());
    }
}

void SbRtl_IsError(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxVariable* pVar = rPar.Get(1);
    SbUnoObject* pObj = dynamic_cast<SbUnoObject*>( pVar  );
    if ( !pObj )
    {
        if ( SbxBase* pBaseObj = (pVar->IsObject() ? pVar->GetObject() : nullptr) )
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
        rPar.Get(0)->PutBool(xError->hasError());
    }
    else
    {
        rPar.Get(0)->PutBool(rPar.Get(1)->IsErr());
    }
}

void SbRtl_IsNull(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    // #51475 because of Uno-objects return true
    // even if the pObj value is NULL
    SbxVariableRef pArg = rPar.Get(1);
    bool bNull = rPar.Get(1)->IsNull();
    if( !bNull && pArg->GetType() == SbxOBJECT )
    {
        SbxBase* pObj = pArg->GetObject();
        if( !pObj )
        {
            bNull = true;
        }
    }
    rPar.Get(0)->PutBool(bNull);
}

void SbRtl_IsNumeric(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    rPar.Get(0)->PutBool(rPar.Get(1)->IsNumericRTL());
}


void SbRtl_IsMissing(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    // #57915 Missing is reported by an error
    rPar.Get(0)->PutBool(rPar.Get(1)->IsErr());
}

// Function looks for wildcards, removes them and always returns the pure path
static OUString implSetupWildcard(const OUString& rFileParam, SbiRTLData& rRTLData)
{
    static const char cDelim1 = '/';
    static const char cDelim2 = '\\';
    static const char cWild1 = '*';
    static const char cWild2 = '?';

    rRTLData.moWildCard.reset();
    rRTLData.sFullNameToBeChecked.clear();

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
            rRTLData.sFullNameToBeChecked = aPathStr;
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
        rRTLData.moWildCard.emplace(aPureFileName);
    }
    return aPathStr;
}

static bool implCheckWildcard(std::u16string_view rName, SbiRTLData const& rRTLData)
{
    bool bMatch = true;

    if (rRTLData.moWildCard)
    {
        bMatch = rRTLData.moWildCard->Matches(rName);
    }
    return bMatch;
}


static bool isRootDir( std::u16string_view aDirURLStr )
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
        if( aSeg1[1] == ':' )
        {
            bRoot = true;
        }
    }
    // More than one segments can never be root
    // so bRoot remains false

    return bRoot;
}

void SbRtl_Dir(StarBASIC *, SbxArray & rPar, bool)
{
    OUString aPath;

    const sal_uInt32 nParCount = rPar.Count();
    if( nParCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbiRTLData& rRTLData = GetSbData()->pInst->GetRTLData();

        if( hasUno() )
        {
            const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
            if( xSFI.is() )
            {
                if ( nParCount >= 2 )
                {
                    OUString aFileParam = rPar.Get(1)->GetOUString();

                    OUString aFileURLStr = implSetupWildcard(aFileParam, rRTLData);
                    if (!rRTLData.sFullNameToBeChecked.isEmpty())
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
                        rPar.Get(0)->PutString(aNameOnlyStr);
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
                            rPar.Get(0)->PutString("");
                        }

                        SbAttributes nFlags = SbAttributes::NONE;
                        if ( nParCount > 2 )
                        {
                            rRTLData.nDirFlags = nFlags
                                = static_cast<SbAttributes>(rPar.Get(2)->GetInteger());
                        }
                        else
                        {
                            rRTLData.nDirFlags = SbAttributes::NONE;
                        }
                        // Read directory
                        bool bIncludeFolders = bool(nFlags & SbAttributes::DIRECTORY);
                        rRTLData.aDirSeq = xSFI->getFolderContents(aDirURLStr, bIncludeFolders);
                        rRTLData.nCurDirPos = 0;

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
                                rRTLData.nCurDirPos = -2;
                            }
                        }
                    }
                    catch(const Exception & )
                    {
                    }
                }


                if (rRTLData.aDirSeq.hasElements())
                {
                    bool bFolderFlag = bool(rRTLData.nDirFlags & SbAttributes::DIRECTORY);

                    SbiInstance* pInst = GetSbData()->pInst;
                    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
                    for( ;; )
                    {
                        if (rRTLData.nCurDirPos < 0)
                        {
                            if (rRTLData.nCurDirPos == -2)
                            {
                                aPath = ".";
                            }
                            else if (rRTLData.nCurDirPos == -1)
                            {
                                aPath = "..";
                            }
                            rRTLData.nCurDirPos++;
                        }
                        else if (rRTLData.nCurDirPos >= rRTLData.aDirSeq.getLength())
                        {
                            rRTLData.aDirSeq.realloc(0);
                            aPath.clear();
                            break;
                        }
                        else
                        {
                            OUString aFile
                                = rRTLData.aDirSeq.getConstArray()[rRTLData.nCurDirPos++];

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

                        bool bMatch = implCheckWildcard(aPath, rRTLData);
                        if( !bMatch )
                        {
                            continue;
                        }
                        break;
                    }
                }
                rPar.Get(0)->PutString(aPath);
            }
        }
        else
        {
            // TODO: OSL
            if ( nParCount >= 2 )
            {
                OUString aFileParam = rPar.Get(1)->GetOUString();

                OUString aDirURL = implSetupWildcard(aFileParam, rRTLData);

                SbAttributes nFlags = SbAttributes::NONE;
                if ( nParCount > 2 )
                {
                    rRTLData.nDirFlags = nFlags
                        = static_cast<SbAttributes>(rPar.Get(2)->GetInteger());
                }
                else
                {
                    rRTLData.nDirFlags = SbAttributes::NONE;
                }

                // Read directory
                bool bIncludeFolders = bool(nFlags & SbAttributes::DIRECTORY);
                rRTLData.pDir = std::make_unique<Directory>(aDirURL);
                FileBase::RC nRet = rRTLData.pDir->open();
                if( nRet != FileBase::E_None )
                {
                    rRTLData.pDir.reset();
                    rPar.Get(0)->PutString(OUString());
                    return;
                }

                // #86950 Add "." and ".." directories for VB compatibility
                rRTLData.nCurDirPos = 0;
                if( bIncludeFolders )
                {
                    bool bRoot = isRootDir( aDirURL );

                    // If it's no root directory we flag the need for
                    // the "." and ".." directories by the value -2
                    // for the actual position. Later for -2 will be
                    // returned "." and for -1 ".."
                    if( !bRoot )
                    {
                        rRTLData.nCurDirPos = -2;
                    }
                }

            }

            if (rRTLData.pDir)
            {
                bool bFolderFlag = bool(rRTLData.nDirFlags & SbAttributes::DIRECTORY);
                for( ;; )
                {
                    if (rRTLData.nCurDirPos < 0)
                    {
                        if (rRTLData.nCurDirPos == -2)
                        {
                            aPath = ".";
                        }
                        else if (rRTLData.nCurDirPos == -1)
                        {
                            aPath = "..";
                        }
                        rRTLData.nCurDirPos++;
                    }
                    else
                    {
                        DirectoryItem aItem;
                        FileBase::RC nRet = rRTLData.pDir->getNextItem(aItem);
                        if( nRet != FileBase::E_None )
                        {
                            rRTLData.pDir.reset();
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

                    bool bMatch = implCheckWildcard(aPath, rRTLData);
                    if( !bMatch )
                    {
                        continue;
                    }
                    break;
                }
            }
            rPar.Get(0)->PutString(aPath);
        }
    }
}


void SbRtl_GetAttr(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        sal_Int16 nFlags = 0;

        // In Windows, we want to use Windows API to get the file attributes
        // for VBA interoperability.
    #if defined(_WIN32)
        if( SbiRuntime::isVBAEnabled() )
        {
            OUString aPathURL = getFullPath(rPar.Get(1)->GetOUString());
            OUString aPath;
            FileBase::getSystemPathFromFileURL( aPathURL, aPath );
            DWORD nRealFlags = GetFileAttributesW (o3tl::toW(aPath.getStr()));
            if (nRealFlags != 0xffffffff)
            {
                if (nRealFlags == FILE_ATTRIBUTE_NORMAL)
                {
                    nRealFlags = 0;
                }
                nFlags = static_cast<sal_Int16>(nRealFlags);
            }
            else
            {
                StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
            }
            rPar.Get(0)->PutInteger(nFlags);

            return;
        }
    #endif

        if( hasUno() )
        {
            const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
            if( xSFI.is() )
            {
                try
                {
                    OUString aPath = getFullPath(rPar.Get(1)->GetOUString());
                    bool bExists = false;
                    try { bExists = xSFI->exists( aPath ); }
                    catch(const Exception & ) {}
                    if( !bExists )
                    {
                        return StarBASIC::Error( ERRCODE_BASIC_FILE_NOT_FOUND );
                    }

                    bool bReadOnly = xSFI->isReadOnly( aPath );
                    bool bHidden = xSFI->isHidden( aPath );
                    bool bDirectory = xSFI->isFolder( aPath );
                    if( bReadOnly )
                    {
                        nFlags |= sal_uInt16(SbAttributes::READONLY);
                    }
                    if( bHidden )
                    {
                        nFlags |= sal_uInt16(SbAttributes::HIDDEN);
                    }
                    if( bDirectory )
                    {
                        nFlags |= sal_uInt16(SbAttributes::DIRECTORY);
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
            (void)DirectoryItem::get(getFullPath(rPar.Get(1)->GetOUString()), aItem);
            FileStatus aFileStatus( osl_FileStatus_Mask_Attributes | osl_FileStatus_Mask_Type );
            (void)aItem.getFileStatus( aFileStatus );
            sal_uInt64 nAttributes = aFileStatus.getAttributes();
            bool bReadOnly = (nAttributes & osl_File_Attribute_ReadOnly) != 0;

            FileStatus::Type aType = aFileStatus.getFileType();
            bool bDirectory = isFolder( aType );
            if( bReadOnly )
            {
                nFlags |= sal_uInt16(SbAttributes::READONLY);
            }
            if( bDirectory )
            {
                nFlags |= sal_uInt16(SbAttributes::DIRECTORY);
            }
        }
        rPar.Get(0)->PutInteger(nFlags);
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}


void SbRtl_FileDateTime(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
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
            const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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
            bool bSuccess = false;
            do
            {
                DirectoryItem aItem;
                if (DirectoryItem::get( getFullPath( aPath ), aItem ) != FileBase::E_None)
                    break;

                FileStatus aFileStatus( osl_FileStatus_Mask_ModifyTime );
                if (aItem.getFileStatus( aFileStatus ) != FileBase::E_None)
                    break;

                TimeValue aTimeVal = aFileStatus.getModifyTime();
                oslDateTime aDT;
                if (!osl_getDateTimeFromTimeValue( &aTimeVal, &aDT ))
                    // Strictly spoken this is not an i/o error but some other failure.
                    break;

                aTime = tools::Time( aDT.Hours, aDT.Minutes, aDT.Seconds, aDT.NanoSeconds );
                aDate = Date( aDT.Day, aDT.Month, aDT.Year );
                bSuccess = true;
            }
            while(false);

            if (!bSuccess)
                StarBASIC::Error( ERRCODE_IO_GENERAL );
        }

        // An empty date shall not result in a formatted null-date (1899-12-30
        // or 1900-01-01) or even worse -0001-12-03 or some such due to how
        // GetDayDiff() treats things. There should be an error set in this
        // case anyway because of a missing file or other error above, but... so
        // do not even bother to use the number formatter.
        OUString aRes;
        if (aDate.IsEmpty())
        {
            aRes = "0000-00-00 00:00:00";
        }
        else
        {
            double fSerial = static_cast<double>(GetDayDiff( aDate ));
            tools::Long nSeconds = aTime.GetHour();
            nSeconds *= 3600;
            nSeconds += aTime.GetMin() * 60;
            nSeconds += aTime.GetSec();
            double nDays = static_cast<double>(nSeconds) / (24.0*3600.0);
            fSerial += nDays;

            const Color* pCol;

            std::shared_ptr<SvNumberFormatter> pFormatter;
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

            pFormatter->GetOutputString( fSerial, nIndex, aRes, &pCol );
        }
        rPar.Get(0)->PutString(aRes);
    }
}


void SbRtl_EOF(StarBASIC *, SbxArray & rPar, bool)
{
    // No changes for UCB
    if (rPar.Count() != 2)
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
            return StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
        }
        bool beof;
        SvStream* pSvStrm = pSbStrm->GetStrm();
        if ( pSbStrm->IsText() )
        {
            char cBla;
            (*pSvStrm).ReadChar( cBla ); // can we read another character?
            beof = pSvStrm->eof();
            if ( !beof )
            {
                pSvStrm->SeekRel( -1 );
            }
        }
        else
        {
            beof = pSvStrm->eof();  // for binary data!
        }
        rPar.Get(0)->PutBool(beof);
    }
}

void SbRtl_FileAttr(StarBASIC *, SbxArray & rPar, bool)
{
    // No changes for UCB
    // #57064 Although this function doesn't operate with DirEntry, it is
    // not touched by the adjustment to virtual URLs, as it only works on
    // already opened files and the name doesn't matter there.

    if (rPar.Count() != 3)
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
            return StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
        }
        sal_Int16 nRet;
        if (rPar.Get(2)->GetInteger() == 1)
        {
            nRet = static_cast<sal_Int16>(pSbStrm->GetMode());
        }
        else
        {
            nRet = 0; // System file handle not supported
        }
        rPar.Get(0)->PutInteger(nRet);
    }
}
void SbRtl_Loc(StarBASIC *, SbxArray & rPar, bool)
{
    // No changes for UCB
    if (rPar.Count() != 2)
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
            return StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
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
        rPar.Get(0)->PutLong(static_cast<sal_Int32>(nPos));
    }
}

void SbRtl_Lof(StarBASIC *, SbxArray & rPar, bool)
{
    // No changes for UCB
    if (rPar.Count() != 2)
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
            return StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
        }
        SvStream* pSvStrm = pSbStrm->GetStrm();
        sal_uInt64 const nLen = pSvStrm->TellEnd();
        rPar.Get(0)->PutLong(static_cast<sal_Int32>(nLen));
    }
}


void SbRtl_Seek(StarBASIC *, SbxArray & rPar, bool)
{
    // No changes for UCB
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs < 2 || nArgs > 3 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    sal_Int16 nChannel = rPar.Get(1)->GetInteger();
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nChannel );
    if ( !pSbStrm )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
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
        rPar.Get(0)->PutLong(static_cast<sal_Int32>(nPos));
    }
    else                // Seek-Statement
    {
        sal_Int32 nPos = rPar.Get(2)->GetLong();
        if ( nPos < 1 )
        {
            return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
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

void SbRtl_Format(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count();
    if ( nArgCount < 2 || nArgCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aResult;
        if( nArgCount == 2 )
        {
            rPar.Get(1)->Format(aResult);
        }
        else
        {
            OUString aFmt(rPar.Get(2)->GetOUString());
            rPar.Get(1)->Format(aResult, &aFmt);
        }
        rPar.Get(0)->PutString(aResult);
    }
}

static bool IsMissing(SbxArray& rPar, const sal_uInt32 i)
{
    const sal_uInt32 nArgCount = rPar.Count();
    if (nArgCount <= i)
        return true;

    SbxVariable* aPar = rPar.Get(i);
    return (aPar->GetType() == SbxERROR && SbiRuntime::IsMissing(aPar, 1));
}

static sal_Int16 GetOptionalIntegerParamOrDefault(SbxArray& rPar, const sal_uInt32 i,
                                                  const sal_Int16 defaultValue)
{
    return IsMissing(rPar, i) ? defaultValue : rPar.Get(i)->GetInteger();
}

static OUString GetOptionalOUStringParamOrDefault(SbxArray& rPar, const sal_uInt32 i,
                                                  const OUString& defaultValue)
{
    return IsMissing(rPar, i) ? defaultValue : rPar.Get(i)->GetOUString();
}

static void lcl_FormatNumberPercent(SbxArray& rPar, bool isPercent)
{
    const sal_uInt32 nArgCount = rPar.Count();
    if (nArgCount < 2 || nArgCount > 6)
    {
        return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
    }

    // The UI locale never changes -> we can use static value here
    static const LocaleDataWrapper localeData(Application::GetSettings().GetUILanguageTag());
    sal_Int16 nNumDigitsAfterDecimal = -1;
    if (nArgCount > 2 && !rPar.Get(2)->IsEmpty())
    {
        nNumDigitsAfterDecimal = rPar.Get(2)->GetInteger();
        if (nNumDigitsAfterDecimal < -1)
        {
            return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
        }
        else if (nNumDigitsAfterDecimal > 255)
            nNumDigitsAfterDecimal %= 256;
    }
    if (nNumDigitsAfterDecimal == -1)
        nNumDigitsAfterDecimal = LocaleDataWrapper::getNumDigits();

    bool bIncludeLeadingDigit = LocaleDataWrapper::isNumLeadingZero();
    if (nArgCount > 3 && !rPar.Get(3)->IsEmpty())
    {
        switch (rPar.Get(3)->GetInteger())
        {
            case ooo::vba::VbTriState::vbFalse:
                bIncludeLeadingDigit = false;
                break;
            case ooo::vba::VbTriState::vbTrue:
                bIncludeLeadingDigit = true;
                break;
            case ooo::vba::VbTriState::vbUseDefault:
                // do nothing;
                break;
            default:
                return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }

    bool bUseParensForNegativeNumbers = false;
    if (nArgCount > 4 && !rPar.Get(4)->IsEmpty())
    {
        switch (rPar.Get(4)->GetInteger())
        {
            case ooo::vba::VbTriState::vbFalse:
            case ooo::vba::VbTriState::vbUseDefault:
                // do nothing
                break;
            case ooo::vba::VbTriState::vbTrue:
                bUseParensForNegativeNumbers = true;
                break;
            default:
                return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }

    bool bGroupDigits = false;
    if (nArgCount > 5 && !rPar.Get(5)->IsEmpty())
    {
        switch (rPar.Get(5)->GetInteger())
        {
            case ooo::vba::VbTriState::vbFalse:
            case ooo::vba::VbTriState::vbUseDefault:
                // do nothing
                break;
            case ooo::vba::VbTriState::vbTrue:
                bGroupDigits = true;
                break;
            default:
                return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);
        }
    }

    double fVal = rPar.Get(1)->GetDouble();
    if (isPercent)
        fVal *= 100;
    const bool bNegative = fVal < 0;
    if (bNegative)
        fVal = fabs(fVal); // Always work with non-negatives, to easily handle leading zero

    static const sal_Unicode decSep = localeData.getNumDecimalSep().toChar();
    OUStringBuffer aResult;
    rtl::math::doubleToUStringBuffer(aResult,
        fVal, rtl_math_StringFormat_F, nNumDigitsAfterDecimal, decSep,
        bGroupDigits ? localeData.getDigitGrouping().getConstArray() : nullptr,
        localeData.getNumThousandSep().toChar());

    if (!bIncludeLeadingDigit && aResult.getLength() > 1)
        aResult.stripStart('0');

    if (nNumDigitsAfterDecimal > 0)
    {
        const sal_Int32 nSepPos = aResult.indexOf(decSep);

        // VBA allows up to 255 digits; rtl::math::doubleToUString outputs up to 15 digits
        // for ~small numbers, so pad them as appropriate.
        if (nSepPos >= 0)
            comphelper::string::padToLength(aResult, nSepPos + nNumDigitsAfterDecimal + 1, '0');
    }

    if (bNegative)
    {
        if (bUseParensForNegativeNumbers)
            aResult.insert(0, '(').append(')');
        else
            aResult.insert(0, '-');
    }
    if (isPercent)
        aResult.append('%');
    rPar.Get(0)->PutString(aResult.makeStringAndClear());
}

// https://docs.microsoft.com/en-us/office/vba/Language/Reference/User-Interface-Help/formatnumber-function
void SbRtl_FormatNumber(StarBASIC*, SbxArray& rPar, bool)
{
    return lcl_FormatNumberPercent(rPar, false);
}

// https://docs.microsoft.com/en-us/office/vba/Language/Reference/User-Interface-Help/formatpercent-function
void SbRtl_FormatPercent(StarBASIC*, SbxArray& rPar, bool)
{
    return lcl_FormatNumberPercent(rPar, true);
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

RandomNumberGenerator& theRandomNumberGenerator()
{
    static RandomNumberGenerator theGenerator;
    return theGenerator;
}

}

void SbRtl_Randomize(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() > 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    if (rPar.Count() == 2)
    {
        int nSeed = static_cast<int>(rPar.Get(1)->GetInteger());
        theRandomNumberGenerator().global_rng.seed(nSeed);
    }
    // without parameter, no need to do anything - RNG is seeded at first use
}

void SbRtl_Rnd(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() > 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double const tmp(dist(theRandomNumberGenerator().global_rng));
        rPar.Get(0)->PutDouble(tmp);
    }
}


//  Syntax: Shell("Path",[ Window-Style,[ "Params", [ bSync = sal_False ]]])
//  WindowStyles (VBA compatible):
//      2 == Minimized
//      3 == Maximized
//     10 == Full-Screen (text mode applications OS/2, WIN95, WNT)
//     HACK: The WindowStyle will be passed to
//     Application::StartApp in Creator. Format: "xxxx2"


void SbRtl_Shell(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count();
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
                aCmdLine += " " + tmp;
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
        std::vector<OUString> aTokenVector;
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
            aTokenVector.push_back( aToken );
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
        std::vector<OUString>::const_iterator iter = aTokenVector.begin();
        OUString aOUStrProgURL = getFullPath( *iter );

        ++iter;

        sal_uInt16 nParamCount = sal::static_int_cast< sal_uInt16 >(aTokenVector.size() - 1 );
        std::unique_ptr<rtl_uString*[]> pParamList;
        if( nParamCount )
        {
            pParamList.reset( new rtl_uString*[nParamCount]);
            for(int iVector = 0; iter != aTokenVector.end(); ++iVector, ++iter)
            {
                const OUString& rParamStr = *iter;
                pParamList[iVector] = nullptr;
                rtl_uString_assign(&(pParamList[iVector]), rParamStr.pData);
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
            rPar.Get(0)->PutLong(0);
        }
    }
}

void SbRtl_VarType(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        rPar.Get(0)->PutInteger(static_cast<sal_Int16>(eType));
    }
}

// Exported function
OUString getBasicTypeName( SbxDataType eType )
{
    static constexpr OUString pTypeNames[] =
    {
        u"Empty"_ustr,            // SbxEMPTY
        u"Null"_ustr,             // SbxNULL
        u"Integer"_ustr,          // SbxINTEGER
        u"Long"_ustr,             // SbxLONG
        u"Single"_ustr,           // SbxSINGLE
        u"Double"_ustr,           // SbxDOUBLE
        u"Currency"_ustr,         // SbxCURRENCY
        u"Date"_ustr,             // SbxDATE
        u"String"_ustr,           // SbxSTRING
        u"Object"_ustr,           // SbxOBJECT
        u"Error"_ustr,            // SbxERROR
        u"Boolean"_ustr,          // SbxBOOL
        u"Variant"_ustr,          // SbxVARIANT
        u"DataObject"_ustr,       // SbxDATAOBJECT
        u"Unknown Type"_ustr,
        u"Unknown Type"_ustr,
        u"Char"_ustr,             // SbxCHAR
        u"Byte"_ustr,             // SbxBYTE
        u"UShort"_ustr,           // SbxUSHORT
        u"ULong"_ustr,            // SbxULONG
        u"Long64"_ustr,           // SbxLONG64
        u"ULong64"_ustr,          // SbxULONG64
        u"Int"_ustr,              // SbxINT
        u"UInt"_ustr,             // SbxUINT
        u"Void"_ustr,             // SbxVOID
        u"HResult"_ustr,          // SbxHRESULT
        u"Pointer"_ustr,          // SbxPOINTER
        u"DimArray"_ustr,         // SbxDIMARRAY
        u"CArray"_ustr,           // SbxCARRAY
        u"Userdef"_ustr,          // SbxUSERDEF
        u"Lpstr"_ustr,            // SbxLPSTR
        u"Lpwstr"_ustr,           // SbxLPWSTR
        u"Unknown Type"_ustr,     // SbxCoreSTRING
        u"WString"_ustr,          // SbxWSTRING
        u"WChar"_ustr,            // SbxWCHAR
        u"Int64"_ustr,            // SbxSALINT64
        u"UInt64"_ustr,           // SbxSALUINT64
        u"Decimal"_ustr,          // SbxDECIMAL
    };

    size_t nPos = static_cast<size_t>(eType) & 0x0FFF;
    const size_t nTypeNameCount = std::size( pTypeNames );
    if ( nPos >= nTypeNameCount )
    {
        nPos = nTypeNameCount - 1;
    }
    return pTypeNames[nPos];
}

static OUString getObjectTypeName( SbxVariable* pVar )
{
    OUString sRet( "Object" );
    if ( pVar )
    {
        SbxBase* pBaseObj = pVar->GetObject();
        if( !pBaseObj )
        {
           sRet = "Nothing";
        }
        else
        {
            SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>( pVar  );
            if ( !pUnoObj )
            {
                pUnoObj = dynamic_cast<SbUnoObject*>( pBaseObj  );
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
                    Sequence< OUString > sServices = xServInfo->getSupportedServiceNames();
                    if ( sServices.hasElements() )
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

void SbRtl_TypeName(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
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
            aRetStr = getObjectTypeName(rPar.Get(1));
        }
        else
        {
            aRetStr = getBasicTypeName( eType );
        }
        if( bIsArray )
        {
            aRetStr += "()";
        }
        rPar.Get(0)->PutString(aRetStr);
    }
}

void SbRtl_Len(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        const OUString& rStr = rPar.Get(1)->GetOUString();
        rPar.Get(0)->PutLong(rStr.getLength());
    }
}

void SbRtl_DDEInitiate(StarBASIC *, SbxArray & rPar, bool)
{
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs != 3 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    const OUString& rApp = rPar.Get(1)->GetOUString();
    const OUString& rTopic = rPar.Get(2)->GetOUString();

    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    size_t nChannel;
    ErrCode nDdeErr = pDDE->Initiate( rApp, rTopic, nChannel );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
    else
    {
        rPar.Get(0)->PutInteger(static_cast<sal_Int16>(nChannel));
    }
}

void SbRtl_DDETerminate(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    ErrCode nDdeErr = pDDE->Terminate( nChannel );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}

void SbRtl_DDETerminateAll(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs != 1 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    ErrCode nDdeErr = pDDE->TerminateAll();
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}

void SbRtl_DDERequest(StarBASIC *, SbxArray & rPar, bool)
{
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs != 3 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    const OUString& rItem = rPar.Get(2)->GetOUString();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    OUString aResult;
    ErrCode nDdeErr = pDDE->Request( nChannel, rItem, aResult );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
    else
    {
        rPar.Get(0)->PutString(aResult);
    }
}

void SbRtl_DDEExecute(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs != 3 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    const OUString& rCommand = rPar.Get(2)->GetOUString();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    ErrCode nDdeErr = pDDE->Execute( nChannel, rCommand );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}

void SbRtl_DDEPoke(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    int nArgs = static_cast<int>(rPar.Count());
    if ( nArgs != 4 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    size_t nChannel = rPar.Get(1)->GetInteger();
    const OUString& rItem = rPar.Get(2)->GetOUString();
    const OUString& rData = rPar.Get(3)->GetOUString();
    SbiDdeControl* pDDE = GetSbData()->pInst->GetDdeControl();
    ErrCode nDdeErr = pDDE->Poke( nChannel, rItem, rData );
    if( nDdeErr )
    {
        StarBASIC::Error( nDdeErr );
    }
}


void SbRtl_FreeFile(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    short nChannel = 1;
    while( nChannel < CHANNELS )
    {
        SbiStream* pStrm = pIO->GetStream( nChannel );
        if( !pStrm )
        {
            rPar.Get(0)->PutInteger(nChannel);
            return;
        }
        nChannel++;
    }
    StarBASIC::Error( ERRCODE_BASIC_TOO_MANY_FILES );
}

void SbRtl_LBound(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = dynamic_cast<SbxDimArray*>( pParObj );
    if( !pArr )
        return StarBASIC::Error( ERRCODE_BASIC_MUST_HAVE_DIMS );

    sal_Int32 nLower, nUpper;
    short nDim = (nParCount == 3) ? static_cast<short>(rPar.Get(2)->GetInteger()) : 1;
    if (!pArr->GetDim(nDim, nLower, nUpper))
        return StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
    rPar.Get(0)->PutLong(nLower);
}

void SbRtl_UBound(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = dynamic_cast<SbxDimArray*>( pParObj );
    if( !pArr )
        return StarBASIC::Error( ERRCODE_BASIC_MUST_HAVE_DIMS );

    sal_Int32 nLower, nUpper;
    short nDim = (nParCount == 3) ? static_cast<short>(rPar.Get(2)->GetInteger()) : 1;
    if (!pArr->GetDim(nDim, nLower, nUpper))
        return StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
    rPar.Get(0)->PutLong(nUpper);
}

void SbRtl_RGB(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 4)
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    sal_Int32 nRed = rPar.Get(1)->GetInteger() & 0xFF;
    sal_Int32 nGreen = rPar.Get(2)->GetInteger() & 0xFF;
    sal_Int32 nBlue = rPar.Get(3)->GetInteger() & 0xFF;
    sal_Int32 nRGB;

    SbiInstance* pInst = GetSbData()->pInst;
    bool bCompatibility = ( pInst && pInst->IsCompatibility() );
    // See discussion in tdf#145725, here's the quotation from a link indicated in the bugtracker
    // which explains why we need to manage RGB differently according to VB compatibility
    // "In other words, the individual color components are stored in the opposite order one would expect.
    //  VB stores the red color component in the low-order byte of the long integer's low-order word,
    //  the green color in the high-order byte of the low-order word, and the blue color in the low-order byte of the high-order word"
    if( bCompatibility )
    {
        nRGB   = (nBlue << 16) | (nGreen << 8) | nRed;
    }
    else
    {
        nRGB   = (nRed << 16) | (nGreen << 8) | nBlue;
    }
    rPar.Get(0)->PutLong(nRGB);
}

void SbRtl_QBColor(StarBASIC *, SbxArray & rPar, bool)
{
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

    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    sal_Int16 nCol = rPar.Get(1)->GetInteger();
    if( nCol < 0 || nCol > 15 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    sal_Int32 nRGB = pRGB[ nCol ];
    rPar.Get(0)->PutLong(nRGB);
}

static std::vector<sal_uInt8> byteArray2Vec(SbxArray* pArr)
{
    std::vector<sal_uInt8> result;
    if (pArr)
    {
        const sal_uInt32 nCount = pArr->Count();
        result.reserve(nCount + 1); // to avoid reallocation when padding in vbFromUnicode
        for (sal_uInt32 i = 0; i < nCount; i++)
            result.push_back(pArr->Get(i)->GetByte());
    }
    return result;
}

// Makes sure to get the byte array if passed, or the string converted to the bytes using
// StringToByteArray in basic/source/sbx/sbxstr.cxx
static std::vector<sal_uInt8> getByteArray(SbxValue& val)
{
    if (val.GetFullType() == SbxOBJECT)
        if (auto pObj = val.GetObject())
            if (pObj->GetType() == (SbxARRAY | SbxBYTE))
                if (auto pArr = dynamic_cast<SbxArray*>(pObj))
                    return byteArray2Vec(pArr);

    // Convert to string
    tools::SvRef<SbxValue> pStringValue(new SbxValue(SbxSTRING));
    *pStringValue = val;

    // Convert string to byte array
    tools::SvRef<SbxValue> pValue(new SbxValue(SbxOBJECT));
    pValue->PutObject(new SbxArray(SbxBYTE));
    *pValue = *pStringValue; // Does the magic of conversion of strings to byte arrays
    return byteArray2Vec(dynamic_cast<SbxArray*>(pValue->GetObject()));
}

// StrConv(string, conversion, LCID)
void SbRtl_StrConv(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count() - 1;
    if( nArgCount < 2 || nArgCount > 3 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    sal_Int32 nConversion = rPar.Get(2)->GetLong();
    LanguageType nLanguage = LANGUAGE_SYSTEM;
    if (nArgCount == 3)
    {
        sal_Int32 lcid = rPar.Get(3)->GetLong();
        nLanguage = LanguageType(lcid);
    }

    if (nConversion == ooo::vba::VbStrConv::vbUnicode) // This mode does not combine
    {
        // Assume that the passed byte array is encoded in the defined encoding, convert to
        // UTF-16 and store as string. Passed strings are converted to byte array first.
        auto inArray = getByteArray(*rPar.Get(1));
        std::string_view s(reinterpret_cast<char*>(inArray.data()), inArray.size() / sizeof(char));
        const auto encoding = utl_getWinTextEncodingFromLangStr(LanguageTag(nLanguage).getBcp47());
        OUString aOUStr = OStringToOUString(s, encoding);
        rPar.Get(0)->PutString(aOUStr);
        return;
    }

    if (nConversion == ooo::vba::VbStrConv::vbFromUnicode) // This mode does not combine
    {
        // Assume that the passed byte array is UTF-16-encoded (system-endian), convert to specified
        // encoding and store as byte array. Passed strings are converted to byte array first.
        auto inArray = getByteArray(*rPar.Get(1));
        while (inArray.size() % sizeof(sal_Unicode))
            inArray.push_back('\0');
        std::u16string_view s(reinterpret_cast<sal_Unicode*>(inArray.data()),
                              inArray.size() / sizeof(sal_Unicode));
        const auto encoding = utl_getWinTextEncodingFromLangStr(LanguageTag(nLanguage).getBcp47());
        OString aOStr = OUStringToOString(s, encoding);
        const sal_Int32 lb = IsBaseIndexOne() ? 1 : 0;
        const sal_Int32 ub = lb + aOStr.getLength() - 1;
        SbxDimArray* pArray = new SbxDimArray(SbxBYTE);
        pArray->unoAddDim(lb, ub);

        for (sal_Int32 i = 0; i < aOStr.getLength(); ++i)
        {
            SbxVariable* pNew = new SbxVariable(SbxBYTE);
            pNew->PutByte(aOStr[i]);
            pArray->Put(pNew, i);
        }

        SbxVariable* retVar = rPar.Get(0);
        SbxFlagBits nFlags = retVar->GetFlags();
        retVar->ResetFlag(SbxFlagBits::Fixed);
        retVar->PutObject(pArray);
        retVar->SetFlags(nFlags);
        retVar->SetParameters(nullptr);
        return;
    }

    std::vector<TransliterationFlags> aTranslitSet;
    auto check = [&nConversion, &aTranslitSet](sal_Int32 conv, TransliterationFlags flag)
    {
        if ((nConversion & conv) != conv)
            return false;

        aTranslitSet.push_back(flag);
        nConversion &= ~conv;
        return true;
    };

    // Check mutually exclusive bits together

    if (!check(ooo::vba::VbStrConv::vbProperCase, TransliterationFlags::TITLE_CASE))
        if (!check(ooo::vba::VbStrConv::vbUpperCase, TransliterationFlags::LOWERCASE_UPPERCASE))
            check(ooo::vba::VbStrConv::vbLowerCase, TransliterationFlags::UPPERCASE_LOWERCASE);

    if (!check(ooo::vba::VbStrConv::vbWide, TransliterationFlags::HALFWIDTH_FULLWIDTH))
        check(ooo::vba::VbStrConv::vbNarrow, TransliterationFlags::FULLWIDTH_HALFWIDTH);

    if (!check(ooo::vba::VbStrConv::vbKatakana, TransliterationFlags::HIRAGANA_KATAKANA))
        check(ooo::vba::VbStrConv::vbHiragana, TransliterationFlags::KATAKANA_HIRAGANA);

    if (nConversion) // unknown / incorrectly combined bits
        return StarBASIC::Error(ERRCODE_BASIC_BAD_ARGUMENT);

    OUString aStr = rPar.Get(1)->GetOUString();
    if (!aStr.isEmpty() && !aTranslitSet.empty())
    {
        uno::Reference< uno::XComponentContext > xContext = getProcessComponentContext();

        for (auto transliterationFlag : aTranslitSet)
        {
            if (transliterationFlag == TransliterationFlags::TITLE_CASE)
            {
                // TransliterationWrapper only handles the first character of the passed string
                // when handling TITLE_CASE; see Transliteration_titlecase::transliterateImpl in
                // i18npool/source/transliteration/transliteration_body.cxx
                CharClass aCharClass{ xContext, LanguageTag(nLanguage) };
                aStr = aCharClass.titlecase(aCharClass.lowercase(aStr));
            }
            else
            {
                utl::TransliterationWrapper aWrapper(xContext, transliterationFlag);
                aStr = aWrapper.transliterate(aStr, nLanguage, 0, aStr.getLength(), nullptr);
            }
        }
    }

    rPar.Get(0)->PutString(aStr);
}


void SbRtl_Beep(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    Sound::Beep();
}

void SbRtl_Load(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }


    SbxBase* pObj = rPar.Get(1)->GetObject();
    if ( !pObj )
        return;

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

void SbRtl_Unload(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }


    SbxBase* pObj = rPar.Get(1)->GetObject();
    if ( !pObj )
        return;

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

void SbRtl_LoadPicture(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    OUString aFileURL = getFullPath(rPar.Get(1)->GetOUString());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream( aFileURL, StreamMode::READ ));
    if( pStream )
    {
        Bitmap aBmp;
        ReadDIB(aBmp, *pStream, true);
        BitmapEx aBitmapEx(aBmp);
        Graphic aGraphic(aBitmapEx);

        SbxObjectRef xRef = new SbStdPicture;
        static_cast<SbStdPicture*>(xRef.get())->SetGraphic( aGraphic );
        rPar.Get(0)->PutObject(xRef.get());
    }
}

void SbRtl_SavePicture(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 3)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    SbxBase* pObj = rPar.Get(1)->GetObject();
    if (SbStdPicture *pPicture = dynamic_cast<SbStdPicture*>(pObj))
    {
        SvFileStream aOStream(rPar.Get(2)->GetOUString(), StreamMode::WRITE | StreamMode::TRUNC);
        const Graphic& aGraphic = pPicture->GetGraphic();
        TypeSerializer aSerializer(aOStream);
        aSerializer.writeGraphic(aGraphic);
    }
}

void SbRtl_MsgBox(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count();
    if( nArgCount < 2 || nArgCount > 6 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    // tdf#147529 - check for missing parameters
    if (IsMissing(rPar, 1))
    {
        return StarBASIC::Error(ERRCODE_BASIC_NOT_OPTIONAL);
    }

    // tdf#151012 - initialize optional parameters with their default values (number of buttons)
    WinBits nType = static_cast<WinBits>(GetOptionalIntegerParamOrDefault(rPar, 2, 0)); // MB_OK
    WinBits nStyle = nType;
    nStyle &= 15; // delete bits 4-16
    if (nStyle > 5)
        nStyle = 0;

    enum BasicResponse
    {
        Ok = 1,
        Cancel = 2,
        Abort = 3,
        Retry = 4,
        Ignore = 5,
        Yes = 6,
        No = 7
    };

    OUString aMsg = rPar.Get(1)->GetOUString();
    // tdf#151012 - initialize optional parameters with their default values (title of dialog box)
    OUString aTitle = GetOptionalOUStringParamOrDefault(rPar, 3, Application::GetDisplayName());

    WinBits nDialogType = nType & (16+32+64);

    SolarMutexGuard aSolarGuard;
    weld::Widget* pParent = Application::GetDefDialogParent();

    VclMessageType eType = VclMessageType::Other;

    switch (nDialogType)
    {
        case 16:
            eType = VclMessageType::Error;
            break;
        case 32:
            eType = VclMessageType::Question;
            break;
        case 48:
            eType = VclMessageType::Warning;
            break;
        case 64:
            eType = VclMessageType::Info;
            break;
    }

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                eType, VclButtonsType::NONE, aMsg, GetpApp()));

    switch (nStyle)
    {
        case 0: // MB_OK
        default:
            xBox->add_button(GetStandardText(StandardButtonType::OK), BasicResponse::Ok);
            break;
        case 1: // MB_OKCANCEL
            xBox->add_button(GetStandardText(StandardButtonType::OK), BasicResponse::Ok);
            xBox->add_button(GetStandardText(StandardButtonType::Cancel), BasicResponse::Cancel);

            if (nType & 256 || nType & 512)
                xBox->set_default_response(BasicResponse::Cancel);
            else
                xBox->set_default_response(BasicResponse::Ok);

            break;
        case 2: // MB_ABORTRETRYIGNORE
            xBox->add_button(GetStandardText(StandardButtonType::Abort), BasicResponse::Abort);
            xBox->add_button(GetStandardText(StandardButtonType::Retry), BasicResponse::Retry);
            xBox->add_button(GetStandardText(StandardButtonType::Ignore), BasicResponse::Ignore);

            if (nType & 256)
                xBox->set_default_response(BasicResponse::Retry);
            else if (nType & 512)
                xBox->set_default_response(BasicResponse::Ignore);
            else
                xBox->set_default_response(BasicResponse::Cancel);

            break;
        case 3: // MB_YESNOCANCEL
            xBox->add_button(GetStandardText(StandardButtonType::Yes), BasicResponse::Yes);
            xBox->add_button(GetStandardText(StandardButtonType::No), BasicResponse::No);
            xBox->add_button(GetStandardText(StandardButtonType::Cancel), BasicResponse::Cancel);

            if (nType & 256 || nType & 512)
                xBox->set_default_response(BasicResponse::Cancel);
            else
                xBox->set_default_response(BasicResponse::Yes);

            break;
        case 4: // MB_YESNO
            xBox->add_button(GetStandardText(StandardButtonType::Yes), BasicResponse::Yes);
            xBox->add_button(GetStandardText(StandardButtonType::No), BasicResponse::No);

            if (nType & 256 || nType & 512)
                xBox->set_default_response(BasicResponse::No);
            else
                xBox->set_default_response(BasicResponse::Yes);

            break;
        case 5: // MB_RETRYCANCEL
            xBox->add_button(GetStandardText(StandardButtonType::Retry), BasicResponse::Retry);
            xBox->add_button(GetStandardText(StandardButtonType::Cancel), BasicResponse::Cancel);

            if (nType & 256 || nType & 512)
                xBox->set_default_response(BasicResponse::Cancel);
            else
                xBox->set_default_response(BasicResponse::Retry);

            break;
    }

    xBox->set_title(aTitle);
    sal_Int16 nRet = xBox->run();
    rPar.Get(0)->PutInteger(nRet);
}

void SbRtl_SetAttr(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() == 3)
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        SbAttributes nFlags = static_cast<SbAttributes>(rPar.Get(2)->GetInteger());

        if( hasUno() )
        {
            const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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

void SbRtl_Reset(StarBASIC *, SbxArray &, bool)
{
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    if (pIO)
    {
        pIO->CloseAll();
    }
}

void SbRtl_DumpAllObjects(StarBASIC * pBasic, SbxArray & rPar, bool)
{
    const sal_uInt32 nArgCount = rPar.Count();
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
        SvFileStream aStrm(rPar.Get(1)->GetOUString(),
                            StreamMode::WRITE | StreamMode::TRUNC );
        p->Dump(aStrm, rPar.Get(2)->GetBool());
        aStrm.Close();
        if( aStrm.GetError() != ERRCODE_NONE )
        {
            StarBASIC::Error( ERRCODE_BASIC_IO_ERROR );
        }
    }
}


void SbRtl_FileExists(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        bool bExists = false;

        if( hasUno() )
        {
            const uno::Reference< ucb::XSimpleFileAccess3 >& xSFI = getFileAccess();
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
        rPar.Get(0)->PutBool(bExists);
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

void SbRtl_Partition(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 5)
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    sal_Int32 nNumber = rPar.Get(1)->GetLong();
    sal_Int32 nStart = rPar.Get(2)->GetLong();
    sal_Int32 nStop = rPar.Get(3)->GetLong();
    sal_Int32 nInterval = rPar.Get(4)->GetLong();

    if( nStart < 0 || nStop <= nStart || nInterval < 1 )
    {
        return StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    // the Partition function inserts leading spaces before lowervalue and uppervalue
    // so that they both have the same number of characters as the string
    // representation of the value (Stop + 1). This ensures that if you use the output
    // of the Partition function with several values of Number, the resulting text
    // will be handled properly during any subsequent sort operation.

    // calculate the maximum number of characters before lowervalue and uppervalue
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
        for ( sal_Int32 i= nLen - nLen1; i > 0; --i )
        {
            aRetStr.append(" ");
        }
    }
    aRetStr.append( aLowerValue + ":");
    if( nLen > nLen2 )
    {
        // appending the leading spaces for the uppervalue
        for ( sal_Int32 i= nLen - nLen2; i > 0; --i )
        {
            aRetStr.append(" ");
        }
    }
    aRetStr.append( aUpperValue );
    rPar.Get(0)->PutString(aRetStr.makeStringAndClear());
}

#endif

sal_Int16 implGetDateYear( double aDate )
{
    Date aRefDate(1899'12'30);
    sal_Int32 nDays = static_cast<sal_Int32>(aDate);
    aRefDate.AddDays( nDays );
    sal_Int16 nRet = aRefDate.GetYear();
    return nRet;
}

bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay,
        bool bUseTwoDigitYear, SbDateCorrection eCorr, double& rdRet )
{
    // XXX NOTE: For VBA years<0 are invalid and years in the range 0..29 and
    // 30..99 can not be input as they are 2-digit for 2000..2029 and
    // 1930..1999, VBA mode overrides bUseTwoDigitYear (as if that was always
    // true). For VBA years > 9999 are invalid.
    // For StarBASIC, if bUseTwoDigitYear==true then years in the range 0..99
    // can not be input as they are 2-digit for 1900..1999, years<0 are
    // accepted. If bUseTwoDigitYear==false then all years are accepted, but
    // year 0 is invalid (last day BCE -0001-12-31, first day CE 0001-01-01).
#if HAVE_FEATURE_SCRIPTING
    if ( (nYear < 0 || 9999 < nYear) && SbiRuntime::isVBAEnabled() )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return false;
    }
    else if ( nYear < 30 && SbiRuntime::isVBAEnabled() )
    {
        nYear += 2000;
    }
    else
#endif
    {
        if ( 0 <= nYear && nYear < 100 &&
#if HAVE_FEATURE_SCRIPTING
                                          (bUseTwoDigitYear || SbiRuntime::isVBAEnabled())
#else
                                          bUseTwoDigitYear
#endif
           )
        {
            nYear += 1900;
        }
    }

    sal_Int32 nAddMonths = 0;
    sal_Int32 nAddDays = 0;
    // Always sanitize values to set date and to use for validity detection.
    if (nMonth < 1 || 12 < nMonth)
    {
        sal_Int16 nM = ((nMonth < 1) ? (12 + (nMonth % 12)) : (nMonth % 12));
        nAddMonths = nMonth - nM;
        nMonth = nM;
    }
    // Day 0 would already be normalized during Date::Normalize(), include
    // it in negative days, also to detect non-validity. The actual day of
    // month is 1+(nDay-1)
    if (nDay < 1)
    {
        nAddDays = nDay - 1;
        nDay = 1;
    }
    else if (nDay > 31)
    {
        nAddDays = nDay - 31;
        nDay = 31;
    }

    Date aCurDate( nDay, nMonth, nYear );

    /* TODO: we could enable the same rollover mechanism for StarBASIC to be
     * compatible with VBA (just with our wider supported date range), then
     * documentation would need to be adapted. As is, the DateSerial() runtime
     * function works as dumb as documented... (except that the resulting date
     * is checked for validity now and not just day<=31 and month<=12).
     * If change wanted then simply remove overriding RollOver here and adapt
     * documentation.*/
#if HAVE_FEATURE_SCRIPTING
    if (eCorr == SbDateCorrection::RollOver && !SbiRuntime::isVBAEnabled())
        eCorr = SbDateCorrection::None;
#endif

    if (nYear == 0 || (eCorr == SbDateCorrection::None && (nAddMonths || nAddDays || !aCurDate.IsValidDate())))
    {
#if HAVE_FEATURE_SCRIPTING
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
#endif
        return false;
    }

    if (eCorr != SbDateCorrection::None)
    {
        aCurDate.Normalize();
        if (nAddMonths)
            aCurDate.AddMonths( nAddMonths);
        if (nAddDays)
            aCurDate.AddDays( nAddDays);
        if (eCorr == SbDateCorrection::TruncateToMonth && aCurDate.GetMonth() != nMonth)
        {
            if (aCurDate.GetYear() == SAL_MAX_INT16 && nMonth == 12)
            {
                // Roll over and back not possible, hard max.
                aCurDate.SetMonth(12);
                aCurDate.SetDay(31);
            }
            else
            {
                aCurDate.SetMonth(nMonth);
                aCurDate.SetDay(1);
                aCurDate.AddMonths(1);
                aCurDate.AddDays(-1);
            }
        }
    }

    rdRet = GetDayDiff(aCurDate);
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
    if(!implDateSerial(nYear, nMonth, nDay, false/*bUseTwoDigitYear*/, SbDateCorrection::None, dDate))
        return false;
    rdRet += dDate + implTimeSerial(nHour, nMinute, nSecond);
    return true;
}

sal_Int16 implGetMinute( double dDate )
{
    double nFrac = dDate - floor( dDate );
    nFrac *= 86400.0;
    sal_Int32 nSeconds = static_cast<sal_Int32>(nFrac + 0.5);
    sal_Int16 nTemp = static_cast<sal_Int16>(nSeconds % 3600);
    sal_Int16 nMin = nTemp / 60;
    return nMin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
