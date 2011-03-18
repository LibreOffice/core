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
#ifndef _BASIC_TESTTOOL_HXX_
#define _BASIC_TESTTOOL_HXX_

#include <svl/smplhint.hxx>
#include <tools/string.hxx>

#define TESTTOOL_DEFAULT_PORT 12479
#define UNO_DEFAULT_PORT 12480
#define DEFAULT_HOST "localhost"

#define TT_SIGNATURE_FOR_UNICODE_TEXTFILES "'encoding UTF-8  Do not remove or change this line!"

#define ASSERTION_STACK_PREFIX "Backtrace:"

// #94145# Due to a tab in TT_SIGNATURE_FOR_UNICODE_TEXTFILES which is changed to blanks by some editors
// this routine became necessary
sal_Bool IsTTSignatureForUnicodeTextfile( String aLine );

#define ADD_ERROR_QUIET(nNr, aStr)                                      \
{                                                                       \
    ErrorEntry *pErr;                                                   \
    if ( BasicRuntimeAccess::HasRuntime() )                             \
    {                                                                   \
        BasicRuntime aRun = BasicRuntimeAccess::GetRuntime();           \
        xub_StrLen aErrLn = StarBASIC::GetErl();                        \
        if ( 0 == aErrLn )                                              \
            aErrLn = aRun.GetLine();                                    \
        pErr = new ErrorEntry(nNr, aStr,                                \
            aErrLn, aRun.GetCol1(), aRun.GetCol2());                    \
    }                                                                   \
    else                                                                \
    {                                                                   \
        pErr = new ErrorEntry(nNr, aStr);                               \
    }                                                                   \
    P_FEHLERLISTE->C40_INSERT(ErrorEntry, pErr, P_FEHLERLISTE->Count());\
}
// ??? Irgendwann noch was mit der UID anfangen !!
#define ADD_ERROR(nNr, aStr) { \
        if ( !SbxBase::IsError() )              \
            SbxBase::SetError( nNr );           \
        ADD_ERROR_QUIET(nNr, aStr); \
}

#define POP_ERROR() P_FEHLERLISTE->DeleteAndDestroy(0)
#define GET_ERROR() P_FEHLERLISTE->GetObject(0)
#define IS_ERROR() ( P_FEHLERLISTE->Count() > 0 )

// Transmission of error logs
enum TTLogType { LOG_RUN, LOG_TEST_CASE, LOG_ERROR, LOG_CALL_STACK, LOG_MESSAGE, LOG_WARNING, LOG_ASSERTION, LOG_QA_ERROR, LOG_ASSERTION_STACK };

struct TTDebugData
{
public:
    TTLogType aLogType;
    String aMsg;
    String aFilename;
    xub_StrLen nLine;
    xub_StrLen nCol1;
    xub_StrLen nCol2;
};

struct TTLogMsg
{
public:
    String aLogFileName;
    TTDebugData aDebugData;
};

// For transmission of window information from the Testapp
struct WinInfoRec
{
public:
    String aUId;
    String aKurzname;
    String aSlotname;
    String aLangname;
    sal_uInt16 nRType;
    String aRName;
    sal_Bool bIsReset;
};

// Defines for syntax Highlighting
#define TT_KEYWORD      ((SbTextType)100)   // Including locally executed commands like 'use' ...
#define TT_REMOTECMD    ((SbTextType)101)   // Remotely executed commands like 'nodebug'
#define TT_LOCALCMD     ((SbTextType)102)   // Locally executed commands like 'use'
#define TT_CONTROL      ((SbTextType)103)   // Possibly available control loaded by 'use'
#define TT_SLOT         ((SbTextType)104)   // Available Slots loaded by 'use'
#define TT_METHOD       ((SbTextType)105)   // Possibly allowed Method for controls
#define TT_NOMETHOD     ((SbTextType)106)   // No Possibly allowed Method for controls

#define FILELIST1       ((SbTextType)111)   // Symbols in file 1
#define FILELIST2       ((SbTextType)112)   // Symbols in file 2
#define FILELIST3       ((SbTextType)113)   // Symbols in file 3
#define FILELIST4       ((SbTextType)114)   // Symbols in file 4

/// defines for hints from TestToolObj to the Application
#define SBX_HINT_LANGUAGE_EXTENSION_LOADED      SFX_HINT_USER06
#define SBX_HINT_EXECUTION_STATUS_INFORMATION   SFX_HINT_USER07

#define TT_EXECUTION_ENTERWAIT  0x01
#define TT_EXECUTION_LEAVEWAIT  0x02
#define TT_EXECUTION_SHOW_ACTION    0x03
#define TT_EXECUTION_HIDE_ACTION    0x04

class TTExecutionStatusHint : public SfxSimpleHint
{
private:
    sal_uInt16 mnType;
    String maExecutionStatus;
    String maAdditionalExecutionStatus;

public:
            TYPEINFO();
            TTExecutionStatusHint( sal_uInt16 nType, sal_Char *pExecutionStatus, const sal_Char *pAdditionalExecutionStatus = "" )
                : SfxSimpleHint(SBX_HINT_EXECUTION_STATUS_INFORMATION)
                , mnType( nType )
                , maExecutionStatus( pExecutionStatus, RTL_TEXTENCODING_ASCII_US )
                , maAdditionalExecutionStatus( pAdditionalExecutionStatus, RTL_TEXTENCODING_ASCII_US )
                {;}

            TTExecutionStatusHint( sal_uInt16 nType, const String &aExecutionStatus = String(), const String &aAdditionalExecutionStatus = String() )
                : SfxSimpleHint(SBX_HINT_EXECUTION_STATUS_INFORMATION)
                , mnType( nType )
                , maExecutionStatus( aExecutionStatus )
                , maAdditionalExecutionStatus( aAdditionalExecutionStatus )
                {;}

    const String& GetExecutionStatus() const { return maExecutionStatus; }
    const String& GetAdditionalExecutionStatus() const { return maAdditionalExecutionStatus; }
    sal_uInt16 GetType(){ return mnType; }
};

#endif // _BASIC_TESTTOOL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
