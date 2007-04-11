/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testtool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:55:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _BASIC_TESTTOOL_HXX_
#define _BASIC_TESTTOOL_HXX_

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#define TESTTOOL_DEFAULT_PORT 12479
#define UNO_DEFAULT_PORT 12480
#define DEFAULT_HOST "localhost"

#define TT_SIGNATURE_FOR_UNICODE_TEXTFILES "'encoding UTF-8  Do not remove or change this line!"

#define ASSERTION_STACK_PREFIX "Backtrace:"

// #94145# Due to a tab in TT_SIGNATURE_FOR_UNICODE_TEXTFILES which is changed to blanks by some editors
// this routine became necessary
BOOL IsTTSignatureForUnicodeTextfile( String aLine );

//#include "testapp.hxx"
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
// Irgendwann noch was mit der UID anfangen !!
#define ADD_ERROR(nNr, aStr) { \
        if ( !SbxBase::IsError() )              \
            SbxBase::SetError( nNr );           \
        ADD_ERROR_QUIET(nNr, aStr); \
}

#define POP_ERROR() P_FEHLERLISTE->DeleteAndDestroy(0)
#define GET_ERROR() P_FEHLERLISTE->GetObject(0)
#define IS_ERROR() ( P_FEHLERLISTE->Count() > 0 )

//  Übertragen des Fehlerlogs
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

//  Zum übertragen der Fensterinformation aus der Testapp
struct WinInfoRec
{
public:
    String aUId;
    String aKurzname;
    String aSlotname;
    String aLangname;
    USHORT nRType;
    String aRName;
    BOOL bIsReset;
};

/// defines für syntax Highlighting
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

/// defines für hints vom TestToolObj an die Applikation
#define SBX_HINT_LANGUAGE_EXTENSION_LOADED      SFX_HINT_USER06
#define SBX_HINT_EXECUTION_STATUS_INFORMATION   SFX_HINT_USER07

#define TT_EXECUTION_ENTERWAIT  0x01
#define TT_EXECUTION_LEAVEWAIT  0x02
#define TT_EXECUTION_SHOW_ACTION    0x03
#define TT_EXECUTION_HIDE_ACTION    0x04

class TTExecutionStatusHint : public SfxSimpleHint
{
private:
    USHORT mnType;
    String maExecutionStatus;
    String maAdditionalExecutionStatus;

public:
            TYPEINFO();
            TTExecutionStatusHint( USHORT nType, sal_Char *pExecutionStatus, const sal_Char *pAdditionalExecutionStatus = "" )
                : SfxSimpleHint(SBX_HINT_EXECUTION_STATUS_INFORMATION)
                , mnType( nType )
                , maExecutionStatus( pExecutionStatus, RTL_TEXTENCODING_ASCII_US )
                , maAdditionalExecutionStatus( pAdditionalExecutionStatus, RTL_TEXTENCODING_ASCII_US )
                {;}

            TTExecutionStatusHint( USHORT nType, const String &aExecutionStatus = String(), const String &aAdditionalExecutionStatus = String() )
                : SfxSimpleHint(SBX_HINT_EXECUTION_STATUS_INFORMATION)
                , mnType( nType )
                , maExecutionStatus( aExecutionStatus )
                , maAdditionalExecutionStatus( aAdditionalExecutionStatus )
                {;}

    const String& GetExecutionStatus() const { return maExecutionStatus; }
    const String& GetAdditionalExecutionStatus() const { return maAdditionalExecutionStatus; }
    USHORT GetType(){ return mnType; }
};

#endif //#ifndef _BASIC_TESTTOOL_HXX_
