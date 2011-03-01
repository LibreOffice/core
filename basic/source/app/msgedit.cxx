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
#include "precompiled_basic.hxx"
/*************************************************************************
File Versions so far:
No Version          Initial Implementation without Version Information
Version 2           changed order of entries(New Entries at the end)
Version 3           Changed Charset from CHARSET_IBMPC to RTL_TEXTENCODING_UTF8

*************************************************************************/
#include <cstdio>
#include <tools/time.hxx>
#include <tools/stream.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <tools/fsys.hxx>
#include <svtools/stringtransfer.hxx>
#include <unotools/syslocale.hxx>
#include <basic/ttstrhlp.hxx>
#include "basic.hrc"
#include "msgedit.hxx"
#include "app.hxx"
#include "apperror.hxx"
#include "appbased.hxx"
#include "basmsg.hrc"
#include "basrid.hxx"

USHORT MsgEdit::nMaxLogLen = 0;
BOOL MsgEdit::bLimitLogLen = FALSE;
BOOL MsgEdit::bPrintLogToStdout = FALSE;
BOOL MsgEdit::bPrintLogToStdoutSet = FALSE;

#define WARNING_PREFIX String( SttResId( S_WARNING_PREFIX ) )
#define VERSION_STRING CUniString("File Format Version: ")
#define THIS_VERSION 2

#define LOGTYPE( pEntry ) ((pEntry && pEntry->GetUserData())?((TTDebugData*)pEntry->GetUserData())->aLogType:LOG_ERROR)

MsgEdit::MsgEdit( AppError* pParent, BasicFrame *pBF, const WinBits& aBits )
: pBasicFrame(pBF)
, pCurrentRun(NULL)
, pCurrentTestCase(NULL)
, pCurrentAssertion( NULL )
, pCurrentError(NULL)
, bModified(FALSE)
, bFileLoading(FALSE)
, nVersion(0)
, pAppError( pParent )
, aEditTree( pParent, pBF, aBits | WB_HASBUTTONS | WB_HASLINES | WB_HASBUTTONSATROOT )
{
//  SetFont( aEditTree.GetDefaultFont( DEFAULTFONT_FIXED, aEditTree.GetSettings().GetLanguage(), 0, &aEditTree ) );
    aEditTree.SetNodeBitmaps( Bitmap( SttResId (MBP_PLUS) ), Bitmap( SttResId (MBP_MINUS) ) );
    aEditTree.SetSelectionMode( MULTIPLE_SELECTION );
    if ( aEditTree.GetModel()->GetSortMode() != SortNone )
        aEditTree.GetModel()->SetSortMode( SortNone );

    if ( !bPrintLogToStdoutSet )
    {
        bPrintLogToStdoutSet = TRUE;
        for ( USHORT i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
        {
            if ( Application::GetCommandLineParam( i ).Copy(0,9).CompareIgnoreCaseToAscii("-printlog") == COMPARE_EQUAL
    #ifndef UNX
              || Application::GetCommandLineParam( i ).Copy(0,9).CompareIgnoreCaseToAscii("/printlog") == COMPARE_EQUAL
    #endif
              )
                bPrintLogToStdout = TRUE;
        }
    }
}

MsgEdit::~MsgEdit()
{}

// set the LogType since calling the add method can be from other add methods
#define COPY_TTDEBUGDATA( LOGTYPE )                     \
    TTDebugData *pTTDebugData = new TTDebugData;        \
    *pTTDebugData = aDebugData;                         \
    pTTDebugData->aLogType = LOGTYPE;                   \


void MsgEdit::AddAnyMsg( TTLogMsg *LogMsg )
{
    if ( LogMsg->aDebugData.aFilename.Copy(0,2).CompareToAscii( "--" ) == COMPARE_EQUAL )
        LogMsg->aDebugData.aFilename.Erase(0,2);

    if ( LogMsg->aDebugData.aFilename.Len() && LogMsg->aDebugData.aFilename.GetChar(0) != '~' ) // do we want to convert
    {
        DirEntry aConvert( LogMsg->aDebugData.aFilename );
        if ( pAppError->aBaseDir.Contains( aConvert ) )
        {
            aConvert.ToRel( pAppError->aBaseDir );
            LogMsg->aDebugData.aFilename = CUniString("~");         // mark as converted
            LogMsg->aDebugData.aFilename += aConvert.GetFull( FSYS_STYLE_VFAT );
        }
        else if ( !bFileLoading )
        {
            LogMsg->aDebugData.aFilename.Insert( CUniString("~-"), 0); // mark as unconvertable
        }
    }
    xub_StrLen nPos;
    LogMsg->aDebugData.aMsg.ConvertLineEnd();
    // does the message have several lines -> repeat the call for each line
    if ( (nPos = LogMsg->aDebugData.aMsg.Search( CUniString("\n").ConvertLineEnd() )) != STRING_NOTFOUND )
    {
        String aOriginalMsg = LogMsg->aDebugData.aMsg;
        xub_StrLen nSysLineEndLen = CUniString("\n").ConvertLineEnd().Len();
        String aLastPart = LogMsg->aDebugData.aMsg.Copy( nPos+nSysLineEndLen );
        LogMsg->aDebugData.aMsg.Erase( nPos );
        AddAnyMsg( LogMsg );
        if ( aLastPart.Len() )
        {
            LogMsg->aDebugData.aMsg = aLastPart;
            AddAnyMsg( LogMsg );
        }
        LogMsg->aDebugData.aMsg = aOriginalMsg;
    }
    else
    {
        String aUILogMsg( pBasicFrame->GenRealString( LogMsg->aDebugData.aMsg ) );
        switch ( LogMsg->aDebugData.aLogType )
        {
            case LOG_RUN:
                {
                    if ( LogMsg->aDebugData.aMsg.Len() == 0 )
                    {
                        SvtSysLocale aLocale;
                        LogMsg->aDebugData.aMsg = GEN_RES_STR2( S_PROG_START,
                                aLocale.GetLocaleData().getDate(Date()),
                                aLocale.GetLocaleData().getTime(Time()) );
                        aUILogMsg = pBasicFrame->GenRealString( LogMsg->aDebugData.aMsg );
                    }
                    AddRun( aUILogMsg, LogMsg->aDebugData ); break;
                }
            case LOG_TEST_CASE: AddTestCase( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_ERROR:     AddError( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_CALL_STACK:AddCallStack( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_MESSAGE:   AddMessage( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_WARNING:   AddWarning( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_ASSERTION: AddAssertion( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_ASSERTION_STACK:   AddAssertionStack( aUILogMsg, LogMsg->aDebugData ); break;
            case LOG_QA_ERROR:  AddQAError( aUILogMsg, LogMsg->aDebugData ); break;
            default:OSL_FAIL("Unbekannter Typ in ResultFile. Speichern des ResultFile resultiert in Informationsverlust");
        }

        if ( !bFileLoading )
        {   // Comes from Testtool and must be written immediately
            BOOL bFileWasChanged = pAppError->DiskFileChanged( SINCE_LAST_LOAD );

            DBG_ASSERT( aLogFileName == LogMsg->aLogFileName, "Logging to different logfile as before" );
            DirEntry aEntry( LogMsg->aLogFileName );
            BOOL bNewFile = !aEntry.Exists();
            SvFileStream aStrm( LogMsg->aLogFileName, STREAM_STD_WRITE );
            if ( bNewFile )
            {
                String aSave = VERSION_STRING.Append( UniString::CreateFromInt32( 3 ) ).AppendAscii("\n");      // Version 3
                aSave.ConvertLineEnd(LINEEND_CRLF);
                aStrm << ByteString( aSave, RTL_TEXTENCODING_IBM_850 ).GetBuffer();
            }

            String aLogMsg = Impl_MakeSaveText( LogMsg->aDebugData ).AppendAscii("\n");

            if( aStrm.IsOpen() )
            {
                aLogMsg.ConvertLineEnd(LINEEND_CRLF);
                aStrm.Seek(STREAM_SEEK_TO_END);
                aStrm << ByteString( aLogMsg, RTL_TEXTENCODING_UTF8 ).GetBuffer();
                aStrm.Close();
            }
            if ( !bFileWasChanged )
                pAppError->UpdateFileInfo( HAS_BEEN_LOADED );


            // now write to stdout
            if ( bPrintLogToStdout )
            {
                String aPrintMsg, aOriginalMsg;

                aOriginalMsg = LogMsg->aDebugData.aMsg;
                // converting to human readable string for adding errors to list in testobject
                LogMsg->aDebugData.aMsg = pBasicFrame->GenRealString( LogMsg->aDebugData.aMsg );

                aPrintMsg = Impl_MakeSaveText( LogMsg->aDebugData ).AppendAscii("\n");

                // restore Original Msg
                LogMsg->aDebugData.aMsg = aOriginalMsg;

                printf( "%s", ByteString( aPrintMsg, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
            }
        }
    }
    // converting to human readable string for adding errors to list in testobject
    LogMsg->aDebugData.aMsg = pBasicFrame->GenRealString( LogMsg->aDebugData.aMsg );
}

void MsgEdit::AddRun( String aMsg, TTDebugData aDebugData )
{
    if ( !bFileLoading && bLimitLogLen )
    {
        USHORT nSkip = nMaxLogLen;
        SvLBoxEntry *pRun = aEditTree.First();
        while ( nSkip-- && pRun )
            pRun = aEditTree.NextSibling( pRun );
        // Remove all Entries thereafter
        if ( pRun )
        {
            while ( pRun && aEditTree.NextSibling( pRun ) )
                aEditTree.GetModel()->Remove( aEditTree.NextSibling( pRun ) );

            aEditTree.GetModel()->Remove( pRun );
            bModified = TRUE;
            lModify.Call( NULL );
            Save( aLogFileName );
            pAppError->UpdateFileInfo( HAS_BEEN_LOADED );
        }
    }

    COPY_TTDEBUGDATA( LOG_RUN );
    if ( !bFileLoading || ( bFileLoading && nVersion >= 2 ) )
        pCurrentRun = aEditTree.InsertEntry( aMsg, NULL, FALSE, 0, pTTDebugData );
    else        // First file format
        pCurrentRun = aEditTree.InsertEntry( aMsg, NULL, FALSE, LIST_APPEND, pTTDebugData );    // and therefor at the end

    aEditTree.ShowEntry( pCurrentRun );
    pCurrentTestCase = NULL;
    pCurrentAssertion = NULL;
    pCurrentError = NULL;
}

void MsgEdit::AddTestCase( String aMsg, TTDebugData aDebugData )
{
    if ( pCurrentRun )
    {
        if ( aMsg.Len() == 0 )  // End of Testcase
        {
            pCurrentTestCase = NULL;
        }
        else
        {
            COPY_TTDEBUGDATA( LOG_TEST_CASE );
            pCurrentTestCase = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
            aEditTree.ShowEntry( pCurrentTestCase );
        }
    }
    pCurrentAssertion = NULL;
    pCurrentError = NULL;
}

void MsgEdit::AddError( String aMsg, TTDebugData aDebugData )
{
    if ( !pCurrentTestCase )
    {
        TTLogMsg aLogMsg;
        aLogMsg.aDebugData = aDebugData;
        aLogMsg.aDebugData.aMsg = GEN_RES_STR0( S_ERROR_OUTSIDE_TESTCASE );
        aLogMsg.aDebugData.aLogType = LOG_TEST_CASE;
        aLogMsg.aLogFileName = aLogFileName;
        AddAnyMsg( &aLogMsg );
    }
    if ( pCurrentTestCase )
    {
        COPY_TTDEBUGDATA( LOG_ERROR );
        pCurrentError = aEditTree.InsertEntry( aMsg, pCurrentTestCase, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pCurrentError );
    }
}

void MsgEdit::AddCallStack( String aMsg, TTDebugData aDebugData )
{
    DBG_ASSERT( pCurrentError, "Callstack ohne CurrentError im Journal" );
    if ( pCurrentError )
    {
        COPY_TTDEBUGDATA( LOG_CALL_STACK );
        aEditTree.InsertEntry( aMsg, pCurrentError, FALSE, LIST_APPEND, pTTDebugData );
    }
}

void MsgEdit::AddMessage( String aMsg, TTDebugData aDebugData )
{
    SvLBoxEntry *pThisEntry = NULL;
    COPY_TTDEBUGDATA( LOG_MESSAGE );
    if ( pCurrentTestCase )
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentTestCase, FALSE, LIST_APPEND, pTTDebugData );
    else if ( pCurrentRun )
    {
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }
    else
    {
        AddRun( aMsg, aDebugData );
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }
}

void MsgEdit::AddWarning( String aMsg, TTDebugData aDebugData )
{
    SvLBoxEntry *pThisEntry = NULL;
    String aCompleteMsg;
    aCompleteMsg = WARNING_PREFIX.Append( aMsg );
    COPY_TTDEBUGDATA( LOG_WARNING );

    if ( pCurrentTestCase )
        pThisEntry = aEditTree.InsertEntry( aCompleteMsg, pCurrentTestCase, FALSE, LIST_APPEND, pTTDebugData );
    else if ( pCurrentRun )
    {
        pThisEntry = aEditTree.InsertEntry( aCompleteMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }
    else
    {
        AddRun( aMsg, aDebugData );
        pThisEntry = aEditTree.InsertEntry( aCompleteMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }

    while ( !aEditTree.IsEntryVisible( pThisEntry ) && ( pThisEntry = aEditTree.GetParent( pThisEntry ) ) != NULL )
        aEditTree.InvalidateEntry( pThisEntry );
}

void MsgEdit::AddAssertion( String aMsg, TTDebugData aDebugData )
{
    const String aAssertionStackPrefix( CUniString(ASSERTION_STACK_PREFIX) );
    if ( aMsg.Match( aAssertionStackPrefix ) == aAssertionStackPrefix.Len() )
    {
        AddAssertionStack( aMsg, aDebugData );
        return;
    }
    SvLBoxEntry *pThisEntry = NULL;
    COPY_TTDEBUGDATA( LOG_ASSERTION );
    if ( pCurrentTestCase )
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentTestCase, FALSE, LIST_APPEND, pTTDebugData );
    else if ( pCurrentRun )
    {
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }
    else
    {
        AddRun( aMsg, aDebugData );
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }

    pCurrentAssertion = pThisEntry;

    while ( !aEditTree.IsEntryVisible( pThisEntry ) && ( pThisEntry = aEditTree.GetParent( pThisEntry ) ) != NULL )
        aEditTree.InvalidateEntry( pThisEntry );
}

void MsgEdit::AddAssertionStack( String aMsg, TTDebugData aDebugData )
{
    SvLBoxEntry *pThisEntry = NULL;
    COPY_TTDEBUGDATA( LOG_ASSERTION_STACK );
    if ( pCurrentAssertion )
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentAssertion, FALSE, LIST_APPEND, pTTDebugData );
    else if ( pCurrentTestCase )
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentTestCase, FALSE, LIST_APPEND, pTTDebugData );
    else if ( pCurrentRun )
    {
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }
    else
    {
        AddRun( aMsg, aDebugData );
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }

    while ( !aEditTree.IsEntryVisible( pThisEntry ) && ( pThisEntry = aEditTree.GetParent( pThisEntry ) ) != NULL )
        aEditTree.InvalidateEntry( pThisEntry );
}

void MsgEdit::AddQAError( String aMsg, TTDebugData aDebugData )
{
    SvLBoxEntry *pThisEntry = NULL;
    COPY_TTDEBUGDATA( LOG_QA_ERROR );
    if ( pCurrentTestCase )
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentTestCase, FALSE, LIST_APPEND, pTTDebugData );
    else if ( pCurrentRun )
    {
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }
    else
    {
        AddRun( aMsg, aDebugData );
        pThisEntry = aEditTree.InsertEntry( aMsg, pCurrentRun, FALSE, LIST_APPEND, pTTDebugData );
        aEditTree.ShowEntry( pThisEntry );
    }

    while ( !aEditTree.IsEntryVisible( pThisEntry ) && ( pThisEntry = aEditTree.GetParent( pThisEntry ) ) != NULL )
        aEditTree.InvalidateEntry( pThisEntry );
}

/*
    SvLBoxEntry*    GetEntry( SvLBoxEntry* pParent, ULONG nPos ) const { return SvLBox::GetEntry(pParent,nPos); }
    SvLBoxEntry*    GetEntry( ULONG nRootPos ) const { return SvLBox::GetEntry(nRootPos);}



    SvLBoxEntry*    FirstChild(SvLBoxEntry* pParent ) const { return (SvLBoxEntry*)(pModel->FirstChild(pParent)); }
    SvLBoxEntry*    NextSibling(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->NextSibling( pEntry )); }
    SvLBoxEntry*    PrevSibling(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->PrevSibling( pEntry )); }

    SvLBoxEntry*    FirstSelected() const { return (SvLBoxEntry*)SvListView::FirstSelected(); }
    SvLBoxEntry*    NextSelected( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(SvListView::NextSelected(pEntry)); }
    SvLBoxEntry*    PrevSelected( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(SvListView::PrevSelected(pEntry)); }
    SvLBoxEntry*    LastSelected() const { return (SvLBoxEntry*)(SvListView::LastSelected()); }

    SvLBoxEntry*    GetEntry( SvLBoxEntry* pParent, ULONG nPos ) const { return (SvLBoxEntry*)(pModel->GetEntry(pParent,nPos)); }
    SvLBoxEntry*    GetEntry( ULONG nRootPos ) const { return (SvLBoxEntry*)(pModel->GetEntry(nRootPos)); }

    SvLBoxEntry*    GetParent( SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->GetParent(pEntry)); }
    SvLBoxEntry*    GetRootLevelParent(SvLBoxEntry* pEntry ) const { return (SvLBoxEntry*)(pModel->GetRootLevelParent( pEntry ));}

    BOOL            IsInChildList( SvListEntry* pParent, SvListEntry* pChild) const;
    SvListEntry*    GetEntry( SvListEntry* pParent, ULONG nPos ) const;
    SvListEntry*    GetEntry( ULONG nRootPos ) const;
    SvListEntry*    GetEntryAtAbsPos( ULONG nAbsPos ) const;
    SvListEntry*    GetParent( SvListEntry* pEntry ) const;
    SvListEntry*    GetRootLevelParent( SvListEntry* pEntry ) const;
*/

//#define CHECK( pMemo ) if ( pMemo && !aEditTree.GetViewData( pMemo ) ) pMemo = NULL
#define CHECK( pMemo ) if ( pMemo && !aEditTree.GetModel()->IsInChildList( NULL, pMemo ) ) pMemo = NULL
void MsgEdit::Delete()
{
    aEditTree.RemoveSelection();
    CHECK( pCurrentRun );
    CHECK( pCurrentTestCase );
    CHECK( pCurrentAssertion );
    CHECK( pCurrentError );
    bModified = TRUE;
    lModify.Call( NULL );
}

void MsgEdit::Cut(){ Copy(); Delete(); bModified = TRUE; lModify.Call( NULL ); }
void MsgEdit::Copy(){ ::svt::OStringTransfer::CopyString( GetSelected(), &aEditTree ); }
/**/void MsgEdit::Paste(){ Sound::Beep(); }
void MsgEdit::Undo(){ Sound::Beep(); }
void MsgEdit::Redo(){ Sound::Beep(); }


String MsgEdit::Impl_MakeText( SvLBoxEntry *pEntry ) const
{
    String aRet;
    TTDebugData *aData = (TTDebugData*)pEntry->GetUserData();
    switch ( aData->aLogType )
    {
        case LOG_RUN:       aRet.AppendAscii("\n"); break;
        case LOG_TEST_CASE: break;
        case LOG_ERROR:     break;
        case LOG_CALL_STACK:aRet.AppendAscii("--> "); break;
        case LOG_MESSAGE:   break;
        case LOG_WARNING:   break;
        case LOG_ASSERTION: break;
        case LOG_ASSERTION_STACK:aRet.AppendAscii("--> ");  break;
        case LOG_QA_ERROR:  break;
        default:OSL_FAIL("Unknown type in ResultWindow!");
    }
    aRet += aEditTree.GetEntryText( pEntry );
    return aRet;
}

String MsgEdit::Impl_MakeSaveText( TTDebugData aData ) const
{
//      LogType;Filename;Line;Col1;Col2;Message
    String aRet;
    aRet += String::CreateFromInt32( (int)aData.aLogType );
    aRet += ';';
    aRet += aData.aFilename;
    aRet += ';';
    aRet += String::CreateFromInt32( aData.nLine );
    aRet += ';';
    aRet += String::CreateFromInt32( aData.nCol1 );
    aRet += ';';
    aRet += String::CreateFromInt32( aData.nCol2 );
    aRet += ';';
    aRet += '"';
    aRet += aData.aMsg;
    aRet += '"';
    return aRet;
}

String MsgEdit::Impl_MakeSaveText( SvLBoxEntry *pEntry ) const
{
//      LogType;Filename;Line;Col1;Col2;Message
    String aRet;
    TTDebugData *aData = (TTDebugData*)pEntry->GetUserData();

    if ( aEditTree.PrevSibling( pEntry ) && LOGTYPE( aEditTree.PrevSibling( pEntry ) ) == LOG_TEST_CASE )
    {   // To properly finish cases and warnings/msgs are in correct hierarchie
        aRet += String::CreateFromInt32( (int)LOG_TEST_CASE );
        aRet.AppendAscii(";;0;0;0;\"\"\n");
    }
    aRet += Impl_MakeSaveText( *aData );
    return aRet;
}

String MsgEdit::GetSelected()
{
    String aRet;
    SvLBoxEntry *pEntry = aEditTree.FirstSelected();
    while ( pEntry )
    {
        aRet += Impl_MakeText( pEntry );
        aRet += '\n';
        pEntry = aEditTree.NextSelected( pEntry );
    }
    aRet.ConvertLineEnd();
    return aRet;
}

TextSelection MsgEdit::GetSelection() const
{
    if ( aEditTree.FirstSelected() )
    {
        ULONG nStart=0,nEnd=0;
        nStart = aEditTree.GetModel()->GetAbsPos(aEditTree.FirstSelected() );
        if ( aEditTree.LastSelected() )
            nEnd = aEditTree.GetModel()->GetAbsPos(aEditTree.LastSelected() );
        return TextSelection( TextPaM( nStart, 0 ), TextPaM( nEnd, STRING_MAXLEN ) );
    }
    else
        return TextSelection();
}

void MsgEdit::SetSelection( const TextSelection& rSelection )
{
    ULONG nStart,nEnd;

    while ( aEditTree.GetSelectionCount() )
        aEditTree.Select( aEditTree.FirstSelected(), FALSE );

    if ( rSelection.HasRange() )
    {
        nStart = rSelection.GetStart().GetPara();
        nEnd = rSelection.GetEnd().GetPara();

        for ( ULONG i = nStart ; i <= nEnd ; i++ )
            aEditTree.Select( aEditTree.GetModel()->GetEntryAtAbsPos( i ), TRUE );
    }
}

USHORT MsgEdit::GetLineNr() const
{
    if ( aEditTree.GetCurEntry() )
        return (USHORT)aEditTree.GetModel()->GetAbsPos(aEditTree.GetCurEntry() ) + 1;
    else
        return 0;
}

void MsgEdit::ReplaceSelected( const String& rStr )
{
    (void) rStr; /* avoid warning about unused parameter */
    Sound::Beep();
    OSL_FAIL("Not Implemented");
}

BOOL MsgEdit::IsModified(){ return bModified; }
void MsgEdit::SetModifyHdl( Link l ){ lModify = l; }

String MsgEdit::GetText() const
{
    String aRet;
    SvLBoxEntry *pEntry = aEditTree.First();
    while ( pEntry )
    {
        aRet += Impl_MakeText( pEntry );
        aRet += '\n';
        pEntry = aEditTree.Next( pEntry );
    }
    aRet.ConvertLineEnd();
    return aRet;
}

void MsgEdit::SetText( const String& rStr )
{
    (void) rStr; /* avoid warning about unused parameter */
    Sound::Beep();
    OSL_FAIL("Not Implemented");
}

BOOL MsgEdit::HasText() const
{
  return aEditTree.First() != NULL;
}

// Search from the beginning or mark start + 1
BOOL MsgEdit::Find( const String& s )
{
    TextSelection r  = GetSelection();
    USHORT bgn   = (USHORT) r.GetStart().GetPara() + 1;
    if ( r.GetStart().GetPara() == 0 )
        bgn = 0;    // Search from the beginning

    SvLBoxEntry *pEntry = aEditTree.GetModel()->GetEntryAtAbsPos( bgn );
    while ( pEntry )
    {
        if( aEditTree.GetEntryText( pEntry ).Search( s, 0 ) != STRING_NOTFOUND )
        {
            aEditTree.SetCurEntry( pEntry );
            return TRUE;
        }
        pEntry = aEditTree.Next( pEntry );
    }
    return FALSE;
}

/******************************************************************

 Fileformat of *.res file:
 Information are stored as semicolon separated strings
 Order:

    LogType;Filename;Line;Col1;Col2;Message

******************************************************************/

BOOL MsgEdit::Load( const String& aName )
{
    aLogFileName = aName;
    BOOL bOk = TRUE, bFirstLine = TRUE;
    BOOL bLoadError = FALSE;
    SvFileStream aStrm( aName, STREAM_STD_READ );
    if( aStrm.IsOpen() )
    {
        aEditTree.Clear();
        String aLine;
        bFileLoading = TRUE;  // To avoid logging to disk
        TTLogMsg *pLogMsg = new TTLogMsg;
        while( !aStrm.IsEof() && bOk )
        {
            if ( nVersion >= 3 )    // utf8
                aStrm.ReadByteStringLine( aLine, RTL_TEXTENCODING_UTF8 );
            else
                aStrm.ReadByteStringLine( aLine, RTL_TEXTENCODING_IBM_850 );

            if( aStrm.GetError() != SVSTREAM_OK )
                bOk = FALSE;

#define TOKEN( n ) aLine.GetToken( n )

            if ( aLine.GetTokenCount() >= 6 )
            {
//          LogType;Filename;Line;Col1;Col2;Message
                TTDebugData aDebugData;
                aDebugData.aLogType = TTLogType( TOKEN(0).ToInt32() );
                aDebugData.aFilename = TOKEN(1);
                aDebugData.nLine = USHORT( TOKEN(2).ToInt32() );
                aDebugData.nCol1 = USHORT( TOKEN(3).ToInt32() );
                aDebugData.nCol2 = USHORT( TOKEN(4).ToInt32() );
                aDebugData.aMsg = aLine.GetQuotedToken( 5, CUniString("\"\"") );

                // Remove leading and trailing quotes
                aDebugData.aMsg.Erase(0,1);
                aDebugData.aMsg.Erase(aDebugData.aMsg.Len()-1,1);

                pLogMsg->aLogFileName.Erase();
                pLogMsg->aDebugData = aDebugData;

                AddAnyMsg( pLogMsg );
            }
            else if ( bFirstLine && (aLine.Search( VERSION_STRING ) == 0) )
                nVersion = USHORT( aLine.Copy( VERSION_STRING.Len() ).ToInt32() );
            else if ( aLine.Len() )
                bLoadError = TRUE;

            bFirstLine = FALSE;
        }
        bFileLoading = FALSE;
        delete pLogMsg;
        aStrm.Close();
        if ( nVersion < 2 && !bLoadError )
            Save( aName );  // Necessary to avoid mess

    }
    else
        bOk = FALSE;
    return bOk;
}

BOOL MsgEdit::Save( const String& aName )
{
    BOOL bOk = TRUE;
    BOOL bIsText = DirEntry( aName ).GetExtension().CompareIgnoreCaseToAscii("TXT") == COMPARE_EQUAL;
    if ( bIsText && !QueryBox( NULL, SttResId( IDS_LOSS_OF_INFORMATION ) ).Execute() )
        return FALSE;
    SvFileStream aStrm( aName, STREAM_STD_WRITE | STREAM_TRUNC );
    if( aStrm.IsOpen() )
    {
        if ( bIsText )
        {
            String aSave = GetText();
            aSave.ConvertLineEnd(LINEEND_CRLF);
            aStrm << ByteString( aSave, RTL_TEXTENCODING_UTF8 ).GetBuffer();
        }
        else
        {
//              LogType;Filename;Line;Col1;Col2;Message
            String aSave = VERSION_STRING.Append( UniString::CreateFromInt32( 3 ) ).AppendAscii("\n");      // Version 3
            SvLBoxEntry *pRun = aEditTree.First();
            while ( pRun && aEditTree.NextSibling( pRun ) )
                pRun = aEditTree.NextSibling( pRun );

            aSave.ConvertLineEnd(LINEEND_CRLF);
            aStrm << ByteString( aSave, RTL_TEXTENCODING_IBM_850 ).GetBuffer();

            SvLBoxEntry *pEntry;
            while ( pRun )
            {
                pEntry = pRun;
                while ( pEntry && ( pEntry == pRun || LOGTYPE( pEntry ) != LOG_RUN ) )
                {
                    aSave = Impl_MakeSaveText( pEntry );
                    aSave += '\n';
                    aSave.ConvertLineEnd(LINEEND_CRLF);
                    aStrm << ByteString( aSave, RTL_TEXTENCODING_UTF8 ).GetBuffer();
                    pEntry = aEditTree.Next( pEntry );
                }
                pRun = aEditTree.PrevSibling( pRun );

            }
        }
        if( aStrm.GetError() != SVSTREAM_OK )
            bOk = FALSE;
        else
        {
            bModified = FALSE;
            lModify.Call( NULL );
        }

    }
    else
        bOk = FALSE;
    return bOk;
}


TTTreeListBox::TTTreeListBox( AppError* pParent, BasicFrame* pBF, WinBits nWinStyle )
: SvTreeListBox( pParent, nWinStyle )
, pBasicFrame(pBF)
, pAppError( pParent )
//, nDeselectParent(0)
{}

BOOL TTTreeListBox::JumpToSourcecode( SvLBoxEntry *pThisEntry )
{
    if ( pThisEntry && pThisEntry->GetUserData() && ((TTDebugData*)pThisEntry->GetUserData())->aFilename.Len() > 0 )
    {
        TTDebugData *aData = (TTDebugData*)pThisEntry->GetUserData();
        String aFilename = aData->aFilename;
        if ( aData->aFilename.GetChar(0) == '~' )
        {
            if ( aData->aFilename.GetChar(1) == '-' )
            {
                aFilename.Erase( 0,2 );
            }
            else
            {
                aFilename.Erase( 0,1 );
                DirEntry aConvert( pAppError->aBaseDir );
                aConvert += DirEntry( aFilename, FSYS_STYLE_VFAT );
                aFilename = aConvert.GetFull();
            }
        }

        if ( pBasicFrame->FindModuleWin( aFilename ) )
        {
            AppWin *pWin = pBasicFrame->FindModuleWin( aFilename );
            pWin->ToTop();
        }
        else if ( pBasicFrame->Basic().FindModule( CUniString( "--" ).Append( aFilename ) ) )
        {
            SbModule* pMod = pBasicFrame->Basic().FindModule( CUniString( "--" ).Append( aFilename ) );
            pBasicFrame->CreateModuleWin( pMod );
        }
        else
            pBasicFrame->LoadFile( aFilename );

        if ( pBasicFrame->pWork && pBasicFrame->pWork->ISA(AppEdit) )
            ((AppEdit*)pBasicFrame->pWork)->Highlight( aData->nLine, aData->nCol1, aData->nCol2 );
        return FALSE;
    }
    return TRUE;
}

BOOL TTTreeListBox::DoubleClickHdl()
{
    return JumpToSourcecode( GetHdlEntry() );
}

/*ULONG TTTreeListBox::SelectChildren( SvLBoxEntry* pParent, BOOL bSelect )
{
    SvLBoxEntry *pEntry = FirstChild( pParent );
    ULONG nRet = 0;
    while ( pEntry )
    {
        nRet++;
        Select( pEntry, bSelect );
        pEntry = NextSibling( pEntry );
    }
    return nRet;
}


void TTTreeListBox::SelectHdl()
{
    SvLBoxEntry* pHdlEntry = GetHdlEntry();

    SelectChildren( pHdlEntry, TRUE );
    Select( pHdlEntry, TRUE );
//  InitMenu(pApp->GetAppMenu()->GetPopupMenu( RID_APPEDIT ));  // so that delete works correct
}

void TTTreeListBox::DeselectHdl()
{
    SvLBoxEntry* pHdlEntry = GetHdlEntry();
    if ( GetParent( pHdlEntry ) )
    {
        nDeselectParent++;
        Select( GetParent( pHdlEntry ), FALSE );
        nDeselectParent--;
    }
    if ( !nDeselectParent )
    {
        SelectChildren( pHdlEntry, FALSE );
        Select( pHdlEntry, FALSE );
    }
    Invalidate();
} */


void TTTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetFunction() )
    {
        case KEYFUNC_CUT:
            Control::GetParent()->Command( CommandEvent( Point(), RID_EDITCUT ) );
            break;
        case KEYFUNC_COPY:
            Control::GetParent()->Command( CommandEvent( Point(), RID_EDITCOPY ) );
            break;
        case KEYFUNC_PASTE:
            Control::GetParent()->Command( CommandEvent( Point(), RID_EDITPASTE ) );
            break;
        case KEYFUNC_DELETE:
            Control::GetParent()->Command( CommandEvent( Point(), RID_EDITDEL ) );
            break;
        default:
            if ( rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
                JumpToSourcecode( GetCurEntry() );
            else
                SvTreeListBox::KeyInput( rKEvt );
    }
}


TTFeatures TTTreeListBox::GetFeatures( SvLBoxEntry* pEntry )
{
    switch ( LOGTYPE( pEntry ) )
    {
        case LOG_MESSAGE:
            return HasNothing;
        case LOG_WARNING :
            return HasWarning;
        case LOG_ERROR:
        case LOG_CALL_STACK:
            return HasError;
        case LOG_RUN:
        case LOG_TEST_CASE:
            {
                SvLBoxEntry* pThisEntry = FirstChild( pEntry );
                TTFeatures aResult = HasNothing;
                while ( pThisEntry && !( (aResult & HasError) == HasError ) )
                {
                    if ( !IsEntryVisible( pThisEntry ) )
                        aResult |= GetFeatures( pThisEntry );
                    pThisEntry = NextSibling( pThisEntry );
                }
                return aResult;
            }
        case LOG_ASSERTION:
        case LOG_ASSERTION_STACK:
                return HasAssertion;
        case LOG_QA_ERROR:
                return HasQAError;
        default:
            OSL_FAIL("Unknown type in ResultWindow");
    }
    return HasNothing;
}


class TTLBoxString : public SvLBoxString
{
public:

    TTLBoxString( SvLBoxEntry* pEntry, USHORT nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
        SvLBoxEntry* pEntry);
};


void TTLBoxString::Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
    SvLBoxEntry* pEntry )
{
    TTFeatures aFeatures = ((TTTreeListBox*)&rDev)->GetFeatures( pEntry );

    Font aOldFont( rDev.GetFont());
    Font aFont( aOldFont );
    if ( aFeatures != HasNothing )
    {
        Color aCol;
        if ( ( aFeatures & HasError ) == HasError )
            aCol = Color( 255, 130, 130 );  // Red
        else if ( ( aFeatures & HasWarning ) == HasWarning )
            aCol = Color( 255, 200, 120 );  // Ochre or so
        else if ( ( aFeatures & HasAssertion ) == HasAssertion )
            aCol = Color( 0xd0, 0xd0, 0xff );   // blueish
        else
            aCol = Color( 0xd0, 0xff, 0xd0 );   // greenish

        if( rDev.IsSelected(pEntry) )
            aFont.SetColor( aCol );
        else
        {
            aFont.SetFillColor( aCol );
            aFont.SetTransparent( FALSE );
            Color aCol2( COL_BLACK );
            aFont.SetColor( aCol2 );
        }

        rDev.SetFont( aFont );
        rDev.DrawText( rPos, GetText() );
    }
    else
    {
        if( !rDev.IsSelected(pEntry) )
        {
            Color aCol( COL_BLACK );
            aFont.SetColor( aCol );
        }
        rDev.SetFont( aFont );
        SvLBoxString::Paint( rPos, rDev, nFlags, pEntry );
    }
    rDev.SetFont( aOldFont );
}


void TTTreeListBox::InitEntry(SvLBoxEntry* pEntry,
    const String& rStr ,const Image& rImg1, const Image& rImg2,
    SvLBoxButtonKind eButtonKind )
{
    USHORT nColToHilite = 1; //0==Bitmap;1=="Column1";2=="Column2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nColToHilite );
    TTLBoxString* pStr = new TTLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
