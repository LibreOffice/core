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

#include <sal/config.h>

#ifdef DBG_UTIL

#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits.h>

#include "tools/debug.hxx"
#include "sal/log.hxx"

#include "vcl/svapp.hxx"
#include "vcl/event.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/button.hxx"
#include "vcl/edit.hxx"
#include "vcl/fixed.hxx"
#include "vcl/group.hxx"
#include "vcl/field.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/threadex.hxx"

#include "svdata.hxx"
#include "dbggui.hxx"

#include "vcl/unohelp.hxx"
#include "vcl/unohelp2.hxx"

#include "salinst.hxx"
#include "svsys.h"

#include "com/sun/star/i18n/XCharacterClassification.hpp"

#include <map>
#include <algorithm>

using namespace ::com::sun::star;

// =======================================================================

static const sal_Char* pDbgHelpText[] =
{
"Object Test\n",
"------------------------------------------\n",
"\n",
"--- Macros ---\n",
"DBG_NAME( aName )\n",
"Defines the administration data for a class. This macro may only be used "
" in a source file with the same name.\n",
"\n",
"DBG_NAMEEX( aName )\n",
"Like DBG_NAME, only for other source files.\n",
"\n",
"DBG_CTOR( aName, fTest )\n",
"Must be used in all constructors of a class (also in the CopyCtor). "
"The first parameter must be the registered name (best would be the "
"class name) and the second parameter the test function or 0.\n",
"\n",
"DBG_DTOR( aName, fTest )\n",
"Must be used in the destructor of the class. The first parameter is "
"the registered name and the second parameter is the test function or "
"0.\n",
"\n",
"DBG_CHKTHIS( aName, fTest )\n",
"Can be used in methods of the class when constructors and the "
"desctructor of the class are equiped with the corresponding macros. "
"The first parameter is the registered name, the second parameter is "
"the test function or 0.\n",
"\n",
"DBG_CHKOBJ( pObj, aName, fTest )\n",
"Can be used on instances of a class where the constructors and the "
"destructor of the class are equiped with the corresponding macros. "
"The first parameter is the registered name, the second parameter is "
"the test function or 0.\n",
"\n",
"To make the macros work DBG_UTIL must be defined.\n",
"\n",
"--- Options ---\n",
"This\n",
"The This pointer is validated. This way all objects that are equiped "
"with it can be tested to make sure one is working with existing objects. "
"This way it's easier to find bugs in case of multiple inheritence, "
"alignment or compiler errors. Since almost all standard classes of SV "
"(String, List, Pen, Brush, Polygon, ...) are equiped with DBG_CHKTHIS() "
"a lot of errors are found, although this test will impact performance "
"accordingly.\n",
"\n",
"Function\n",
"When a function is passed with macros, it will be called.\n",
"\n",
"Exit\n",
"This- and Func-Test will also run when exiting the function.\n",
"\n",
"Report\n",
"At the end of the program the number of generated objects is produced "
"as output. Because all important SV classes have at least DBG_CTOR() / "
"DBG_DTOR() it can checked so called resource leaks (system objects which "
" are not freed) exist. These include OutputDevice, Window, VirtualDevice, "
" Printer and Menu. Note: Dtor calls of static objects are not taken into "
" account. Therefor each SV program leaves 2 strings and a bitmap behind.\n",
"\n",
"Trace\n",
"Creation, destruction and usage of objects which are equiped with "
"DBG_XTOR is logged.\n",
"\n",
"\n",
"\nOther tests and macros\n",
"------------------------------------------\n",
"\n",
"Profiling\n",
"DBG_PROFSTART() / DBG_PROFSTOP() / DBG_PROFCONTINUE() / DBG_PROFPAUSE() "
"are evaluated and at the end of the program the number of run throughs "
"and the time this took (including calls to children) in milliseconds is "
"output. These macros can be used to check the same function runs over the "
"entire development period, for example the startup speed. The registered name "
"which was registered with DBG_NAME() must be passed to the macros.\n",
"\n",
"Resources\n",
"In case of resource errors an error dialog is produced before the "
"exception handler is called.\n",
"\n",
"Dialog\n",
"FixedTexts, CheckBoxes, TriStateBoxes and RadioButtons are equiped with "
"a different background color to determine the size of the controls. This "
"test also shows whether controls overlap, whether the tab order is correct "
"and whether the mnemonic characters are correctly assigned. With dialogs "
"it is indicated when no default button or no OK/CancelButton is present. "
"These tests are not 100% correct (e.g. too many warnings are given) and "
"do not form any guarantee that all problematic cases are covered. For "
"example only initial and only visible controls are tested. No errors are "
"found which will occur during the use of a dialog.\n",
"\n",
"Bold AppFont\n",
"The application font is set to bold to see if the position of texts is "
"sufficient for other systems or other system settings. With very narrow "
"fonts the dialogs are made wider because they otherwise appear too narrow.\n",
"\n",
"Trace output\n",
"DBG_TRACE() can be use to produce TRACE output. DBG_TRACEFILE() also outputs "
"the file and line number where the macro is located. DBG_TRACE1() to "
"DBG_TRACE5() can be used to produce formatted output (printf format string) "
"Trace output is enabled when the corresponding option is selected in the "
"dropdown list.\n"
"\n",
"Warnings\n",
"DBG_WARNING() can be used to output warnings. DBG_WARNING1() to "
"DBG_WARNING3() can be used to produce formatted output (printf format string). "
"In case you want to have conditional warnings DBG_ASSERTWARNING() can be "
"used. The warning will be produced if the condition was not met. The first "
"parameter is the condition and the second parameter is the message to be "
"produced. Warnings are enabled if the corresponding option is selected in the "
"dropdown box. When none are selected the condition with DBG_ASSERTWARNING() "
"is not evaluated.\n",
"\n",
"Errors\n",
"DBG_ERROR() can be used to produce error messages. DBG_ERRORFILE() also "
"produces the file and the line number where the macro is located. "
"DBG_ERROR1() bis DBG_ERROR5() can be used to produce formatted output "
"(print format string). "
"In case you want to have conditional warnings DBG_ASSERT() can be "
"used. The warning will be produced if the condition was not met. The first "
"parameter is the condition and the second parameter is the message to be "
"produced. Warnings are enabled if the corresponding option is selected in the "
"dropdown box. When none are selected the condition with DBG_ASSERT() "
"is not evaluated.\n",
"\n",
"\n",
"Output\n",
"------------------------------------------\n",
"\n",
"Overwrite - CheckBox\n",
"With every new program start the log file is overwritten if output has been "
"generated.\n",
"\n",
"Include ObjectTest filters\n",
"Only classes which contain one of the indicated filters are evaluated with "
"the object test. Filters are separated by ';' and are case sensitive. "
"Wildcards are not supported. If no text is indicated the filters are not "
"active.\n",
"\n",
"Exclude ObjectTest filters\n",
"Only classes which do not contain one of the indicated filters are evaluated "
"with the object test. Filters are separated by ';' and are case sensitive. "
"Wildcards are not supported. If no text is indicated the filters are not "
"active.\n",
"\n",
"Include filters\n",
"Only those texts which include the indicated filters are output. "
"Filters are separated by ';' and are case sensitive. "
"Wildcards are not supported. The filter is used for all output (except for "
"errors). If no text is indicated the filters are not active.\n",
"\n",
"Exclude filters\n",
"Only those texts which do not include the indicated filters are output. "
"Filters are separated by ';' and are case sensitive. "
"Wildcards are not supported. The filter is used for all output (except for "
"errors). If no text is indicated the filters are not active.\n",
"\n",
"Furthermore you can indicate where the data will be output:\n",
"\n",
"None\n",
"Output is surpressed.\n",
"\n",
"File\n",
"Outputi n debug file. Filename can be entered in the Editfield.\n",
"\n",
"Window\n",
"Output to a small debug window. The window size is stored if the debug "
"dialog is closed with OK and if the window is visible. Each assertion text can "
"be copied to the clipboard via the context menu of the respective entry.\n",
"\n",
"Shell\n",
"Output to a debug system (Windows debug window) when available or under "
"Unix in the shell window. Otherwise the same as Window.\n",
"\n",
"MessageBox\n",
"Output to a MessageBox. In this case you can select whether the program "
"must be continued, terminated (Application::Abort) or interrupted with "
"CoreDump. Additionally on some systems you get a \"Copy\" button pressing which "
"copies the text of the MessageBox to the clipboard. Because a MessageBox allows "
"further event processing other errors caused by Paint, Activate/Deactivate, "
"GetFocus/LoseFocus can cause more errors or incorrect errors and messages. "
"Therefor the message should also be directed to a file/debugger in case of "
"problems in order to produce the (right) error messages.\n",
"\n",
"TestTool\n",
"When the TestTool runs messages will be redirected inside the TestTool.\n",
"\n",
"Debugger\n",
"Attempt to activate the debugger and produce the message there, in order to "
"always obtain the corresponding stack trace in the debugger.\n",
"\n",
"Abort\n",
"Aborts the application\n",
"\n",
"\n",
"Reroute osl messages - Checkbox\n",
"OSL_ASSERT and similar messages can be intercepted by the general DBG GUI\n",
"or handled system specific as per normal handling in the sal library.\n",
"default is to reroute osl assertions\n",
"\n",
"\n",
"Settings\n",
"------------------------------------------\n",
"\n",
"Where by default the INI and LOG file is read and written the following "
"can be set:\n",
"\n",
"WIN/WNT (WIN.INI, Group SV, Default: dbgsv.ini and dbgsv.log):\n",
"INI: dbgsv\n",
"LOG: dbgsvlog\n",
"\n",
"OS2 (OS2.INI, Application SV, Default: dbgsv.ini and dbgsv.log):\n",
"INI: DBGSV\n",
"LOG: DBGSVLOG\n",
"\n",
"UNIX (Environment variable, Default: .dbgsv.init and dbgsv.log):\n",
"INI: DBGSV_INIT\n",
"LOG: DBGSV_LOG\n",
"\n",
"MAC (Default: dbgsv.ini and dbgsv.log):\n",
"INI: not possible\n",
"LOG: only debug dialog settings\n",
"\n",
"The path and file name must always be specified. The name of the log "
"file that was entered in the debug dialog has always priority.\n",
"\n",
"\n",
"Example\n",
"------------------------------------------\n",
"\n",
"DBG_NAME( String );\n",
"\n",
"#ifdef DBG_UTIL\n",
"const sal_Char* DbgCheckString( const void* pString )\n",
"{\n",
"    String* p = (String*)pString;\n",
"\n",
"    if ( p->mpData->maStr[p->mpData->mnLen] != 0 )\n",
"        return \"String damaged: aStr[nLen] != 0\";\n",
"\n",
"    return NULL;\n",
"}\n",
"#endif\n",
"\n",
"String::String()\n",
"{\n",
"    DBG_CTOR( String, DbgCheckString );\n",
"    // ...\n",
"}\n",
"\n",
"String::~String()\n",
"{\n",
"    DBG_DTOR( String, DbgCheckString );\n",
"    //...\n",
"}\n",
"\n",
"char& String::operator [] ( sal_uInt16 nIndex )\n",
"{\n",
"    DBG_CHKTHIS( String, DbgCheckString );\n",
"    DBG_ASSERT( nIndex <= pData->nLen, \"String::[] : nIndex > Len\" );\n",
"\n",
"    //...\n",
"}\n",
"\n",
"sal_uInt16 String::Search( const String& rStr, sal_uInt16 nIndex ) const\n",
"{\n",
"    DBG_CHKTHIS( String, DbgCheckString );\n",
"    DBG_CHKOBJ( &rStr, String, DbgCheckString );\n",
"\n",
"    //...\n",
"}",
"\n",
NULL
};

// =======================================================================

namespace
{
    // -------------------------------------------------------------------
    typedef ::std::map< OUString, DbgChannelId > UserDefinedChannels;
    UserDefinedChannels& ImplDbgGetUserDefinedChannels()
    {
        static UserDefinedChannels s_aChannels;
        return s_aChannels;
    }

    // -------------------------------------------------------------------
    void ImplAppendUserDefinedChannels( ListBox& rList )
    {
        const UserDefinedChannels& rChannels = ImplDbgGetUserDefinedChannels();
        for ( UserDefinedChannels::const_iterator channel = rChannels.begin();
              channel != rChannels.end();
              ++channel
            )
        {
            sal_uInt16 nEntryPos = rList.InsertEntry( channel->first );
            rList.SetEntryData( nEntryPos, reinterpret_cast< void* >( channel->second ) );
        }
    }

    // -------------------------------------------------------------------
    void ImplSelectChannel( ListBox& rList, sal_uLong nChannelToSelect, sal_uInt16 nPositionOffset )
    {
        if ( nChannelToSelect < DBG_OUT_USER_CHANNEL_0 )
            rList.SelectEntryPos( (sal_uInt16)( nChannelToSelect - nPositionOffset ) );
        else
        {
            for ( sal_uInt16 pos = 0; pos < rList.GetEntryCount(); ++pos )
            {
                DbgChannelId nChannelId = static_cast< DbgChannelId >( reinterpret_cast<sal_IntPtr>(rList.GetEntryData( pos )) );
                if ( nChannelId == nChannelToSelect )
                {
                    rList.SelectEntryPos( pos );
                    return;
                }
            }
        }
    }
    // -------------------------------------------------------------------
    DbgChannelId ImplGetChannelId( const ListBox& rList, sal_uInt16 nPositionOffset )
    {
        sal_uInt16 nSelectedChannelPos = rList.GetSelectEntryPos();
        DbgChannelId nSelectedChannel = static_cast< DbgChannelId >( reinterpret_cast<sal_IntPtr>(rList.GetEntryData( nSelectedChannelPos )) );
        if ( nSelectedChannel == 0)
            return (DbgChannelId)( nSelectedChannelPos + nPositionOffset );
        return nSelectedChannel;
    }
}

// =======================================================================

// -------------
// - DbgWindow -
// -------------

#define DBGWIN_MAXLINES     100

class DbgWindow : public WorkWindow
{
private:
    ListBox         maLstBox;

public:
                    DbgWindow();

    virtual sal_Bool    Close();
    virtual void    Resize();
    virtual long    PreNotify( NotifyEvent& rNEvt );
    void            InsertLine( const OUString& rLine );
    void            Update() { WorkWindow::Update(); maLstBox.Update(); }

private:
    void            GetAssertionEntryRange( sal_uInt16 nInbetweenEntry, sal_uInt16& nFirst, sal_uInt16& nLast );
};

// -----------------
// - DbgInfoDialog -
// -----------------

class DbgInfoDialog : public ModalDialog
{
private:
    ListBox         maListBox;
    OKButton        maOKButton;
    bool            mbHelpText;

public:
                    DbgInfoDialog( Window* pParent, bool bHelpText = false );

    void            SetInfoText( const OUString& rStr );
};

// -------------
// - DbgDialog -
// -------------

class DbgDialog : public ModalDialog
{
private:
    CheckBox        maXtorThis;
    CheckBox        maXtorFunc;
    CheckBox        maXtorExit;
    CheckBox        maXtorReport;
    CheckBox        maXtorTrace;
    GroupBox        maBox1;

    CheckBox        maProf;
    CheckBox        maRes;
    CheckBox        maDialog;
    CheckBox        maBoldAppFont;
    GroupBox        maBox3;

    Edit            maDebugName;
    CheckBox        maOverwrite;
    FixedText       maInclClassText;
    Edit            maInclClassFilter;
    FixedText       maExclClassText;
    Edit            maExclClassFilter;
    FixedText       maInclText;
    Edit            maInclFilter;
    FixedText       maExclText;
    Edit            maExclFilter;
    FixedText       maTraceText;
    ListBox         maTraceBox;
    FixedText       maWarningText;
    ListBox         maWarningBox;
    FixedText       maErrorText;
    ListBox         maErrorBox;
    CheckBox        maHookOSLBox;
    GroupBox        maBox4;

    OKButton        maOKButton;
    CancelButton    maCancelButton;
    PushButton      maInfoButton;
    HelpButton      maHelpButton;
    sal_uInt16          mnErrorOff;

public:
                    DbgDialog();

                    DECL_LINK( ClickHdl, Button* );
    void            RequestHelp( const HelpEvent& rHEvt );
};

// =======================================================================

static sal_Char aDbgInfoBuf[12288];
static sal_Char aDbgOutBuf[DBG_BUF_MAXLEN];

// =======================================================================

DbgWindow::DbgWindow() :
    WorkWindow( NULL, WB_STDWORK ),
    maLstBox( this, WB_AUTOHSCROLL )
{
    DbgData* pData = DbgGetData();

    maLstBox.Show();
    maLstBox.SetPosPixel( Point( 0, 0 ) );

    SetOutputSizePixel( Size( 600, 480 ) );
    if ( pData->aDbgWinState )
    {
        OString aState( pData->aDbgWinState );
        SetWindowState( aState );
    }

    SetText("StarView Debug Window");
    Show();
    Update();
}

// -----------------------------------------------------------------------

sal_Bool DbgWindow::Close()
{
    // remember window position
    OString aState( GetWindowState() );
    DbgData* pData = DbgGetData();
    size_t nCopy = (sizeof( pData->aDbgWinState ) < size_t(aState.getLength() + 1U ))
    ? sizeof( pData->aDbgWinState ) : size_t(aState.getLength() + 1U );
    strncpy( pData->aDbgWinState, aState.getStr(), nCopy );
    pData->aDbgWinState[ sizeof( pData->aDbgWinState ) - 1 ] = 0;
    // and save for next session
    DbgSaveData( *pData );

    delete this;
    ImplGetSVData()->maWinData.mpDbgWin = NULL;
    return sal_True;
}

// -----------------------------------------------------------------------

void DbgWindow::Resize()
{
    maLstBox.SetSizePixel( GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void DbgWindow::GetAssertionEntryRange( sal_uInt16 nInbetweenEntry, sal_uInt16& nFirst, sal_uInt16& nLast )
{
    nFirst = nInbetweenEntry;
    while ( nFirst > 0 )
    {
        if ( maLstBox.GetEntryData( nFirst ) != NULL )
            break;
        --nFirst;
    }
    sal_uInt16 nEntryCount = maLstBox.GetEntryCount();
    nLast = nInbetweenEntry + 1;
    while ( nLast < nEntryCount )
    {
        if ( maLstBox.GetEntryData( nLast ) != NULL )
            break;
        ++nLast;
    }
}

// -----------------------------------------------------------------------

long DbgWindow::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_COMMAND )
    {
        if ( maLstBox.IsWindowOrChild( rNEvt.GetWindow() ) )
        {
            const CommandEvent& rCommand = *rNEvt.GetCommandEvent();
            if ( rCommand.GetCommand() == COMMAND_CONTEXTMENU )
            {
                PopupMenu aMenu;
                aMenu.InsertItem( 1, OUString("copy to clipboard") );

                Point aPos;
                if ( rCommand.IsMouseEvent() )
                    aPos = rCommand.GetMousePosPixel();
                else
                {
                    Rectangle aEntryRect( maLstBox.GetBoundingRectangle( maLstBox.GetSelectEntryPos() ) );
                    aPos = aEntryRect.Center();
                }
                sal_uInt16 nSelected = aMenu.Execute( rNEvt.GetWindow(), aPos );
                if ( nSelected == 1 )
                {
                    // search all entries which belong to this assertion
                    sal_uInt16 nAssertionFirst = 0;
                    sal_uInt16 nAssertionLast = 0;
                    GetAssertionEntryRange( maLstBox.GetSelectEntryPos(), nAssertionFirst, nAssertionLast );

                    // build the string to copy to the clipboard
                    OUStringBuffer sAssertion;
                    OUString sLineFeed = convertLineEnd(
                        OUString("\n"),
                        GetSystemLineEnd());
                    while ( nAssertionFirst < nAssertionLast )
                    {
                        sAssertion.append(maLstBox.GetEntry( nAssertionFirst++ ));
                        sAssertion.append(sLineFeed);
                    }

                    ::vcl::unohelper::TextDataObject::CopyStringTo(
                        sAssertion.makeStringAndClear(), GetClipboard());
                }
            }
            return 1;   // handled
        }
    }
    return WorkWindow::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void DbgWindow::InsertLine( const OUString& rLine )
{
    OUString   aStr = convertLineEnd(rLine, LINEEND_LF);
    sal_Int32  nPos = aStr.indexOf( '\n' );
    bool       bFirstEntry = true;
    while ( nPos != -1 )
    {
        if ( maLstBox.GetEntryCount() >= DBGWIN_MAXLINES )
            maLstBox.RemoveEntry( 0 );

        sal_uInt16 nInsertionPos = maLstBox.InsertEntry( aStr.copy( 0, nPos ) );
        if ( bFirstEntry )
            maLstBox.SetEntryData( nInsertionPos, reinterpret_cast< void* >( 1 ) );
        bFirstEntry = false;

        aStr = aStr.replaceAt( 0, nPos+1, "" );
        nPos = aStr.indexOf( '\n' );
    }
    if ( maLstBox.GetEntryCount() >= DBGWIN_MAXLINES )
        maLstBox.RemoveEntry( 0 );
    sal_uInt16 nInsertionPos = maLstBox.InsertEntry( aStr );
    if ( bFirstEntry )
        maLstBox.SetEntryData( nInsertionPos, reinterpret_cast< void* >( 1 ) );
    maLstBox.SetTopEntry( DBGWIN_MAXLINES-1 );
    maLstBox.Update();
}

// =======================================================================

DbgDialog::DbgDialog() :
    ModalDialog( NULL, WB_STDMODAL | WB_SYSTEMWINDOW ),
    maXtorThis( this ),
    maXtorFunc( this ),
    maXtorExit( this ),
    maXtorReport( this ),
    maXtorTrace( this ),
    maBox1( this ),
    maProf( this ),
    maRes( this ),
    maDialog( this ),
    maBoldAppFont( this ),
    maBox3( this ),
    maDebugName( this ),
    maOverwrite( this ),
    maInclClassText( this ),
    maInclClassFilter( this ),
    maExclClassText( this ),
    maExclClassFilter( this ),
    maInclText( this ),
    maInclFilter( this ),
    maExclText( this ),
    maExclFilter( this ),
    maTraceText( this ),
    maTraceBox( this, WB_DROPDOWN ),
    maWarningText( this ),
    maWarningBox( this, WB_DROPDOWN ),
    maErrorText( this ),
    maErrorBox( this, WB_DROPDOWN ),
    maHookOSLBox( this ),
    maBox4( this ),
    maOKButton( this, WB_DEFBUTTON ),
    maCancelButton( this ),
    maInfoButton( this ),
    maHelpButton( this )
{
    DbgData*    pData = DbgGetData();
    MapMode     aAppMap( MAP_APPFONT );
    Size        aButtonSize = LogicToPixel( Size( 60, 12 ), aAppMap );

    {
    maXtorThis.Show();
    maXtorThis.SetText("T~his");
    if ( pData->nTestFlags & DBG_TEST_XTOR_THIS )
        maXtorThis.Check( sal_True );
    maXtorThis.SetPosSizePixel( LogicToPixel( Point( 10, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorFunc.Show();
    maXtorFunc.SetText("~Function");
    if ( pData->nTestFlags & DBG_TEST_XTOR_FUNC )
        maXtorFunc.Check( sal_True );
    maXtorFunc.SetPosSizePixel( LogicToPixel( Point( 75, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorExit.Show();
    maXtorExit.SetText("E~xit");
    if ( pData->nTestFlags & DBG_TEST_XTOR_EXIT )
        maXtorExit.Check( sal_True );
    maXtorExit.SetPosSizePixel( LogicToPixel( Point( 140, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorReport.Show();
    maXtorReport.SetText("~Report");
    if ( pData->nTestFlags & DBG_TEST_XTOR_REPORT )
        maXtorReport.Check( sal_True );
    maXtorReport.SetPosSizePixel( LogicToPixel( Point( 205, 15 ), aAppMap ),
                                  aButtonSize );
    }

    {
    maXtorTrace.Show();
    maXtorTrace.SetText("~Trace");
    if ( pData->nTestFlags & DBG_TEST_XTOR_TRACE )
        maXtorTrace.Check( sal_True );
    maXtorTrace.SetPosSizePixel( LogicToPixel( Point( 270, 15 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maBox1.Show();
    maBox1.SetText("Object Tests");
    maBox1.SetPosSizePixel( LogicToPixel( Point( 5, 5 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maProf.Show();
    maProf.SetText("~Profiling");
    if ( pData->nTestFlags & DBG_TEST_PROFILING )
        maProf.Check( sal_True );
    maProf.SetPosSizePixel( LogicToPixel( Point( 10, 95 ), aAppMap ),
                            aButtonSize );
    }

    {
    maRes.Show();
    maRes.SetText("~Resourcen");
    if ( pData->nTestFlags & DBG_TEST_RESOURCE )
        maRes.Check( sal_True );
    maRes.SetPosSizePixel( LogicToPixel( Point( 75, 95 ), aAppMap ),
                           aButtonSize );
    }

    {
    maDialog.Show();
    maDialog.SetText("~Dialog");
    if ( pData->nTestFlags & DBG_TEST_DIALOG )
        maDialog.Check( sal_True );
    maDialog.SetPosSizePixel( LogicToPixel( Point( 140, 95 ), aAppMap ),
                              aButtonSize );
    }

    {
    maBoldAppFont.Show();
    maBoldAppFont.SetText("~Bold AppFont");
    if ( pData->nTestFlags & DBG_TEST_BOLDAPPFONT )
        maBoldAppFont.Check( sal_True );
    maBoldAppFont.SetPosSizePixel( LogicToPixel( Point( 205, 95 ), aAppMap ),
                                   aButtonSize );
    maBoldAppFont.SaveValue();
    }

    {
    maBox3.Show();
    maBox3.SetText("Test Options");
    maBox3.SetPosSizePixel( LogicToPixel( Point( 5, 85 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maDebugName.Show();
    maDebugName.SetText(OStringToOUString(pData->aDebugName, RTL_TEXTENCODING_UTF8));
    maDebugName.SetMaxTextLen( sizeof( pData->aDebugName ) );
    maDebugName.SetPosSizePixel( LogicToPixel( Point( 10, 130 ), aAppMap ),
                                 LogicToPixel( Size( 185, 14 ), aAppMap ) );
    }

    {
    maOverwrite.Show();
    maOverwrite.SetText("Overwrite ~File");
    if ( pData->bOverwrite )
        maOverwrite.Check( sal_True );
    maOverwrite.SetPosSizePixel( LogicToPixel( Point( 205, 130 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maHookOSLBox.Show();
    maHookOSLBox.SetText("Reroute osl debug ~messages");
    if ( pData->bHookOSLAssert )
        maHookOSLBox.Check( sal_True );
    maHookOSLBox.SetPosSizePixel( LogicToPixel( Point( 10, 240 ), aAppMap ),
                                  LogicToPixel( Size( 100, 12 ), aAppMap ) );
    }

    {
    maInclClassText.Show();
    maInclClassText.SetText("~Include-ObjectTest-Filter");
    maInclClassText.SetPosSizePixel( LogicToPixel( Point( 10, 150 ), aAppMap ),
                                     LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maInclClassFilter.Show();
    maInclClassFilter.SetText(OStringToOUString(pData->aInclClassFilter, RTL_TEXTENCODING_UTF8));
    maInclClassFilter.SetMaxTextLen( sizeof( pData->aInclClassFilter ) );
    maInclClassFilter.SetPosSizePixel( LogicToPixel( Point( 10, 160 ), aAppMap ),
                                       LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maExclClassText.Show();
    maExclClassText.SetText("~Exclude-ObjectTest-Filter");
    maExclClassText.SetPosSizePixel( LogicToPixel( Point( 115, 150 ), aAppMap ),
                                     LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maExclClassFilter.Show();
    maExclClassFilter.SetText(OStringToOUString(pData->aExclClassFilter, RTL_TEXTENCODING_UTF8));
    maExclClassFilter.SetMaxTextLen( sizeof( pData->aExclClassFilter ) );
    maExclClassFilter.SetPosSizePixel( LogicToPixel( Point( 115, 160 ), aAppMap ),
                                       LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maInclText.Show();
    maInclText.SetText("~Include-Filter");
    maInclText.SetPosSizePixel( LogicToPixel( Point( 10, 180 ), aAppMap ),
                                LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maInclFilter.Show();
    maInclFilter.SetText(OStringToOUString(pData->aInclFilter, RTL_TEXTENCODING_UTF8));
    maInclFilter.SetMaxTextLen( sizeof( pData->aInclFilter ) );
    maInclFilter.SetPosSizePixel( LogicToPixel( Point( 10, 190 ), aAppMap ),
                                  LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maExclText.Show();
    maExclText.SetText("~Exclude-Filter");
    maExclText.SetPosSizePixel( LogicToPixel( Point( 115, 180 ), aAppMap ),
                                LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maExclFilter.Show();
    maExclFilter.SetText(OStringToOUString(pData->aExclFilter, RTL_TEXTENCODING_UTF8));
    maExclFilter.SetMaxTextLen( sizeof( pData->aExclFilter ) );
    maExclFilter.SetPosSizePixel( LogicToPixel( Point( 115, 190 ), aAppMap ),
                                  LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maTraceText.Show();
    maTraceText.SetText("~Trace");
    maTraceText.SetPosSizePixel( LogicToPixel( Point( 10, 210 ), aAppMap ),
                                 LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maTraceBox.InsertEntry(OUString("None"));
    maTraceBox.InsertEntry(OUString("File"));
    maTraceBox.InsertEntry(OUString("Window"));
    maTraceBox.InsertEntry(OUString("Shell"));
    maTraceBox.InsertEntry(OUString("MessageBox"));
    maTraceBox.InsertEntry(OUString("TestTool"));
    maTraceBox.InsertEntry(OUString("Debugger"));
    maTraceBox.InsertEntry(OUString("Abort"));
    ImplAppendUserDefinedChannels( maTraceBox );
    ImplSelectChannel( maTraceBox, pData->nTraceOut, 0 );
    maTraceBox.Show();
    maTraceBox.SetPosSizePixel( LogicToPixel( Point( 10, 220 ), aAppMap ),
                                LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maWarningText.Show();
    maWarningText.SetText("~Warning");
    maWarningText.SetPosSizePixel( LogicToPixel( Point( 115, 210 ), aAppMap ),
                                   LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maWarningBox.InsertEntry(OUString("None"));
    maWarningBox.InsertEntry(OUString("File"));
    maWarningBox.InsertEntry(OUString("Window"));
    maWarningBox.InsertEntry(OUString("Shell"));
    maWarningBox.InsertEntry(OUString("MessageBox"));
    maWarningBox.InsertEntry(OUString("TestTool"));
    maWarningBox.InsertEntry(OUString("Debugger"));
    maWarningBox.InsertEntry(OUString("Abort"));
    ImplAppendUserDefinedChannels( maWarningBox );
    ImplSelectChannel( maWarningBox, pData->nWarningOut, 0 );
    maWarningBox.Show();
    maWarningBox.SetPosSizePixel( LogicToPixel( Point( 115, 220 ), aAppMap ),
                                  LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maErrorText.Show();
    maErrorText.SetText("~Error");
    maErrorText.SetPosSizePixel( LogicToPixel( Point( 220, 210 ), aAppMap ),
                                 LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    if ( DbgIsAllErrorOut() )
    {
        maErrorBox.InsertEntry( OUString("None") );
        maErrorBox.InsertEntry( OUString("File") );
        maErrorBox.InsertEntry( OUString("Window") );
        maErrorBox.InsertEntry( OUString("Shell") );
        mnErrorOff = 0;
    }
    else
        mnErrorOff = 4;
    maErrorBox.InsertEntry( OUString("MessageBox") );
    maErrorBox.InsertEntry( OUString("TestTool") );
    maErrorBox.InsertEntry( OUString("Debugger") );
    maErrorBox.InsertEntry( OUString("Abort") );
    ImplAppendUserDefinedChannels( maErrorBox );
    ImplSelectChannel( maErrorBox, pData->nErrorOut, mnErrorOff );
    maErrorBox.Show();
    maErrorBox.SetPosSizePixel( LogicToPixel( Point( 220, 220 ), aAppMap ),
                                LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maBox4.Show();
    maBox4.SetText("Output");
    maBox4.SetPosSizePixel( LogicToPixel( Point( 5, 120 ), aAppMap ),
                            LogicToPixel( Size( 330, 135 ), aAppMap ) );
    }

    {
    maOKButton.Show();
    maOKButton.SetClickHdl( LINK( this, DbgDialog, ClickHdl ) );
    maOKButton.SetPosSizePixel( LogicToPixel( Point( 10, 260 ), aAppMap ),
                                LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maCancelButton.Show();
    maCancelButton.SetPosSizePixel( LogicToPixel( Point( 70, 260 ), aAppMap ),
                                    LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maInfoButton.Show();
    maInfoButton.SetClickHdl( LINK( this, DbgDialog, ClickHdl ) );
    maInfoButton.SetText("~Info...");
    maInfoButton.SetPosSizePixel( LogicToPixel( Point( 130, 260 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maHelpButton.Show();
    maHelpButton.SetPosSizePixel( LogicToPixel( Point( 190, 260 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }

    {
    SetText("VCL Debug Options");
    SetOutputSizePixel( LogicToPixel( Size( 340, 280 ), aAppMap ) );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( DbgDialog, ClickHdl, Button*, pButton )
{
    if ( pButton == &maOKButton )
    {
        DbgData aData;

        memcpy( &aData, DbgGetData(), sizeof( DbgData ) );
        aData.nTestFlags = 0;

        aData.nTraceOut   = ImplGetChannelId( maTraceBox, 0 );
        aData.nWarningOut = ImplGetChannelId( maWarningBox, 0 );
        aData.nErrorOut   = ImplGetChannelId( maErrorBox, mnErrorOff );

        strncpy( aData.aDebugName, OUStringToOString(maDebugName.GetText(), RTL_TEXTENCODING_UTF8).getStr(), sizeof( aData.aDebugName ) );
        strncpy( aData.aInclClassFilter, OUStringToOString(maInclClassFilter.GetText(), RTL_TEXTENCODING_UTF8).getStr(), sizeof( aData.aInclClassFilter ) );
        strncpy( aData.aExclClassFilter, OUStringToOString(maExclClassFilter.GetText(), RTL_TEXTENCODING_UTF8).getStr(), sizeof( aData.aExclClassFilter ) );
        strncpy( aData.aInclFilter, OUStringToOString(maInclFilter.GetText(), RTL_TEXTENCODING_UTF8).getStr(), sizeof( aData.aInclFilter ) );
        strncpy( aData.aExclFilter, OUStringToOString(maExclFilter.GetText(), RTL_TEXTENCODING_UTF8).getStr(), sizeof( aData.aExclFilter ) );
        aData.aDebugName[sizeof( aData.aDebugName )-1] = '\0';
        aData.aInclClassFilter[sizeof( aData.aInclClassFilter )-1] = '\0';
        aData.aExclClassFilter[sizeof( aData.aExclClassFilter )-1] = '\0';
        aData.aInclFilter[sizeof( aData.aInclFilter )-1] = '\0';
        aData.aExclFilter[sizeof( aData.aExclFilter )-1] = '\0';

        aData.bOverwrite = maOverwrite.IsChecked() ? sal_True : sal_False;
        aData.bHookOSLAssert = maHookOSLBox.IsChecked() ? sal_True : sal_False;

        if ( maXtorThis.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_THIS;

        if ( maXtorFunc.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_FUNC;

        if ( maXtorExit.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_EXIT;

        if ( maXtorReport.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_REPORT;

        if ( maXtorTrace.IsChecked() )
            aData.nTestFlags |= DBG_TEST_XTOR_TRACE;

        if ( maProf.IsChecked() )
            aData.nTestFlags |= DBG_TEST_PROFILING;

        if ( maRes.IsChecked() )
            aData.nTestFlags |= DBG_TEST_RESOURCE;

        if ( maDialog.IsChecked() )
            aData.nTestFlags |= DBG_TEST_DIALOG;

        if ( maBoldAppFont.IsChecked() )
            aData.nTestFlags |= DBG_TEST_BOLDAPPFONT;

        // Daten speichern
        DbgSaveData( aData );

        // Umschalten der Laufzeitwerte
        DBG_INSTOUTTRACE( aData.nTraceOut );
        DBG_INSTOUTWARNING( aData.nWarningOut );
        DBG_INSTOUTERROR( aData.nErrorOut );
        DbgUpdateOslHook( &aData );

        DbgData* pData = DbgGetData();
        #define IMMEDIATE_FLAGS (DBG_TEST_RESOURCE | DBG_TEST_DIALOG | DBG_TEST_BOLDAPPFONT)
        pData->nTestFlags &= ~IMMEDIATE_FLAGS;
        pData->nTestFlags |= aData.nTestFlags & IMMEDIATE_FLAGS;
        strncpy( pData->aInclClassFilter, aData.aInclClassFilter, sizeof( pData->aInclClassFilter ) );
        strncpy( pData->aExclClassFilter, aData.aExclClassFilter, sizeof( pData->aExclClassFilter ) );
        strncpy( pData->aInclFilter, aData.aInclFilter, sizeof( pData->aInclFilter ) );
        strncpy( pData->aExclFilter, aData.aExclFilter, sizeof( pData->aExclFilter ) );
        if ( maBoldAppFont.GetSavedValue() != maBoldAppFont.IsChecked() )
        {
            AllSettings aSettings = Application::GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            Font aFont = aStyleSettings.GetAppFont();
            if ( maBoldAppFont.IsChecked() )
                aFont.SetWeight( WEIGHT_BOLD );
            else
                aFont.SetWeight( WEIGHT_NORMAL );
            aStyleSettings.SetAppFont( aFont );
            aSettings.SetStyleSettings( aStyleSettings );
            Application::SetSettings( aSettings );
        }
        if( (aData.nTestFlags & ~IMMEDIATE_FLAGS) != (pData->nTestFlags & ~IMMEDIATE_FLAGS) )
        {
            InfoBox aBox( this, OUString(
                "Some of the changed settings will only be active after "
                "restarting the process"
                ) );
            aBox.Execute();
        }
        EndDialog( sal_True );
    }
    else if ( pButton == &maInfoButton )
    {
        DbgInfoDialog aInfoDialog( this );
        aDbgInfoBuf[0] = '\0';
        DbgXtorInfo( aDbgInfoBuf );
        OUString aInfoText( aDbgInfoBuf, strlen(aDbgInfoBuf), RTL_TEXTENCODING_UTF8 );
        aInfoDialog.SetText( "Debug InfoReport" );
        aInfoDialog.SetInfoText( aInfoText );
        aInfoDialog.Execute();
    }

    return 0;
}

// -----------------------------------------------------------------------

void DbgDialog::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HELPMODE_CONTEXT )
    {
        DbgInfoDialog aInfoDialog( this, true );
        OUString aHelpText;
        const sal_Char** pHelpStrs = pDbgHelpText;
        while ( *pHelpStrs )
        {
            aHelpText += OUString::createFromAscii(*pHelpStrs);
            pHelpStrs++;
        }
        aInfoDialog.SetText( "Debug Hilfe" );
        aInfoDialog.SetInfoText( aHelpText );
        aInfoDialog.Execute();
    }
}

// =======================================================================

DbgInfoDialog::DbgInfoDialog( Window* pParent, bool bHelpText ) :
    ModalDialog( pParent, WB_STDMODAL ),
    maListBox( this, WB_BORDER | WB_AUTOHSCROLL ),
    maOKButton( this, WB_DEFBUTTON )
{
    mbHelpText = bHelpText;

    if ( !bHelpText )
    {
        Font aFont = GetDefaultFont( DEFAULTFONT_FIXED, LANGUAGE_ENGLISH_US, 0 );
        aFont.SetHeight( 8 );
        aFont.SetPitch( PITCH_FIXED );
        maListBox.SetControlFont( aFont );
    }
    maListBox.SetPosSizePixel( Point( 5, 5 ), Size( 630, 380 ) );
    maListBox.Show();

    maOKButton.SetPosSizePixel( Point( 290, 390 ), Size( 60, 25 ) );
    maOKButton.Show();

    SetOutputSizePixel( Size( 640, 420 ) );
}

// -----------------------------------------------------------------------

void DbgInfoDialog::SetInfoText( const OUString& rStr )
{
    maListBox.SetUpdateMode( sal_False );
    maListBox.Clear();
    OUString aStr = convertLineEnd(rStr, LINEEND_LF);
    sal_Int32 nStrIndex = 0;
    sal_Int32 nFoundIndex;
    do
    {
        nFoundIndex = aStr.indexOf( '\n', nStrIndex );
        OUString aTextParagraph = aStr.copy( nStrIndex, nFoundIndex-nStrIndex );
        if ( mbHelpText )
        {
            long    nMaxWidth = maListBox.GetOutputSizePixel().Width()-30;
            sal_Int32  nLastIndex = 0;
            sal_Int32  nIndex = aTextParagraph.indexOf( ' ' );
            while ( nIndex != -1 )
            {
                if ( maListBox.GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
                {
                    if ( !nLastIndex )
                        nLastIndex = nIndex+1;
                    OUString aTempStr = aTextParagraph.copy( 0, nLastIndex );
                    aTextParagraph = aTextParagraph.replaceAt( 0, nLastIndex, "" );
                    maListBox.InsertEntry( aTempStr );
                    nLastIndex = 0;
                }
                else
                    nLastIndex = nIndex+1;
                nIndex = aTextParagraph.indexOf( ' ', nLastIndex );
            }

            if ( maListBox.GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
            {
                if ( !nLastIndex )
                    nLastIndex = nIndex+1;
                OUString aTempStr = aTextParagraph.copy( 0, nLastIndex );
                aTextParagraph = aTextParagraph.replaceAt( 0, nLastIndex, "" );
                maListBox.InsertEntry( aTempStr );
            }
        }
        maListBox.InsertEntry( aTextParagraph );
        nStrIndex = nFoundIndex+1;
    }
    while ( nFoundIndex != -1 );
    maListBox.SetUpdateMode( sal_True );
}

// =======================================================================

void DbgDialogTest( Window* pWindow )
{
    bool        aAccelBuf[65536] = {0};
    sal_uInt16      nChildCount = pWindow->GetChildCount();
    Window*     pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    Window*     pChild;
    Point       aTabPos;

    if ( !pGetChild )
        return;

    Rectangle*  pRectAry = (Rectangle*)new long[(sizeof(Rectangle)*nChildCount)/sizeof(long)];
    memset( pRectAry, 0, sizeof(Rectangle)*nChildCount );

    if ( pWindow->IsDialog() )
    {
        bool    bOKCancelButton = false;
        bool    bDefPushButton = false;
        bool    bButton = false;
        pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        while ( pGetChild )
        {
            pChild = pGetChild->ImplGetWindow();

            if ( pChild->ImplIsPushButton() )
            {
                bButton = true;
                if ( (pChild->GetType() == WINDOW_OKBUTTON) || (pChild->GetType() == WINDOW_CANCELBUTTON) )
                    bOKCancelButton = true;
                if ( pChild->GetStyle() & WB_DEFBUTTON )
                    bDefPushButton = true;
            }

            pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
        }

        if ( bButton )
        {
            SAL_WARN_IF(
                !bOKCancelButton, "vcl",
                "Dialogs should have a OK- or CancelButton");
            SAL_WARN_IF(
                !bDefPushButton, "vcl",
                "Dialogs should have a Button with WB_DEFBUTTON");
        }
    }

    sal_uInt16 i = 0;
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();

        if ( (pChild->GetType() != WINDOW_TABCONTROL) &&
             (pChild->GetType() != WINDOW_TABPAGE) &&
             (pChild->GetType() != WINDOW_GROUPBOX) )
        {
            OUString        aText = pChild->GetText();
            OUString        aErrorText = aText;
            sal_Int32       nAccelPos = -1;
            sal_Unicode     cAccel = 0;
            if ( aErrorText.getLength() > 128 )
            {
                aErrorText = aErrorText.replaceAt( 128, aErrorText.getLength()-128, "" );
                aErrorText += "...";
            }
            if ( !aText.isEmpty() && (aText.getLength() < 1024) )
            {
                nAccelPos = aText.indexOf( '~' );
                if ( nAccelPos != -1 )
                {
                    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLanguageTag().getLocale();
                    uno::Reference < i18n::XCharacterClassification > xCharClass = vcl::unohelper::CreateCharacterClassification();
                    OUString aUpperText = xCharClass->toUpper( aText, 0, aText.getLength(), rLocale );
                    cAccel = aUpperText[nAccelPos+1];
                    if ( pChild->IsVisible() )
                    {
                        if ( aAccelBuf[cAccel] )
                            DbgOutTypef( DBG_OUT_ERROR, "Double mnemonic char: %c", cAccel );
                        else
                            aAccelBuf[cAccel] = true;
                    }
                }
            }

            if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_CHECKBOX) ||
                 (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                 (pChild->GetType() == WINDOW_PUSHBUTTON) )
            {
                if ( !aText.isEmpty() && aText != "..." )
                {
                    const char* pClass;
                    if ( pChild->GetType() == WINDOW_RADIOBUTTON )
                        pClass = "RadioButton";
                    else if ( pChild->GetType() == WINDOW_IMAGERADIOBUTTON )
                        pClass = "ImageRadioButton";
                    else if ( pChild->GetType() == WINDOW_CHECKBOX )
                        pClass = "CheckBox";
                    else if ( pChild->GetType() == WINDOW_TRISTATEBOX )
                        pClass = "TriStateBox";
                    else if ( pChild->GetType() == WINDOW_PUSHBUTTON )
                        pClass = "PushButton";
                    else
                        pClass = "Dontknow";
                    if( !cAccel )
                        DbgOutTypef( DBG_OUT_ERROR,
                                 "%s should have a mnemonic char (~): %s",
                                 pClass,
                                 OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );

                    // check text width
                    int aWidth=0;
                    switch( pChild->GetType() )
                    {
                        case WINDOW_RADIOBUTTON:
                        case WINDOW_IMAGERADIOBUTTON:
                            aWidth = ((RadioButton*)pChild)->CalcMinimumSize(0).Width();
                            break;
                        case WINDOW_CHECKBOX:
                        case WINDOW_TRISTATEBOX:
                            aWidth = ((CheckBox*)pChild)->CalcMinimumSize(0).Width();
                            break;
                        case WINDOW_PUSHBUTTON:
                            aWidth = ((PushButton*)pChild)->CalcMinimumSize(0).Width();
                            break;
                        default: break;
                    }
                    if( pChild->IsVisible() && pChild->GetSizePixel().Width() < aWidth )
                        DbgOutTypef( DBG_OUT_ERROR,
                                 "%s exceeds window width: %s",
                                 pClass,
                                 OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                }
            }

            if ( pChild->GetType() == WINDOW_FIXEDLINE )
            {
                if ( pChild->GetSizePixel().Width() < pChild->GetTextWidth( aText ) )
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "FixedLine exceeds window width: %s",
                                 OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
            }

            if ( pChild->GetType() == WINDOW_FIXEDTEXT )
            {
                if ( (pChild->GetSizePixel().Height() >= pChild->GetTextHeight()*2) &&
                     !(pChild->GetStyle() & WB_WORDBREAK) )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "FixedText greater than one line, but WordBreak is not set: %s",
                                 OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                }

                if ( pChild->IsVisible() )
                {
                    int aWidth=0;
                    if( nAccelPos != -1 )
                    {
                        aWidth = pChild->GetTextWidth( aText, 0, nAccelPos ) +
                                 pChild->GetTextWidth( aText, nAccelPos+1, aText.getLength() - nAccelPos - 1);
                    }
                    else
                        aWidth = pChild->GetTextWidth( aText );

                    if ( pChild->GetSizePixel().Width() < aWidth && !(pChild->GetStyle() & WB_WORDBREAK) )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "FixedText exceeds window width: %s",
                                         OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                        }
                }

                if ( (i+1 < nChildCount) && !aText.isEmpty() )
                {
                    Window* pTempChild = pGetChild->GetWindow( WINDOW_NEXT )->ImplGetWindow();
                    if ( (pTempChild->GetType() == WINDOW_EDIT) ||
                         (pTempChild->GetType() == WINDOW_MULTILINEEDIT) ||
                         (pTempChild->GetType() == WINDOW_SPINFIELD) ||
                         (pTempChild->GetType() == WINDOW_PATTERNFIELD) ||
                         (pTempChild->GetType() == WINDOW_NUMERICFIELD) ||
                         (pTempChild->GetType() == WINDOW_METRICFIELD) ||
                         (pTempChild->GetType() == WINDOW_CURRENCYFIELD) ||
                         (pTempChild->GetType() == WINDOW_DATEFIELD) ||
                         (pTempChild->GetType() == WINDOW_TIMEFIELD) ||
                         (pTempChild->GetType() == WINDOW_LISTBOX) ||
                         (pTempChild->GetType() == WINDOW_MULTILISTBOX) ||
                         (pTempChild->GetType() == WINDOW_COMBOBOX) ||
                         (pTempChild->GetType() == WINDOW_PATTERNBOX) ||
                         (pTempChild->GetType() == WINDOW_NUMERICBOX) ||
                         (pTempChild->GetType() == WINDOW_METRICBOX) ||
                         (pTempChild->GetType() == WINDOW_CURRENCYBOX) ||
                         (pTempChild->GetType() == WINDOW_DATEBOX) ||
                         (pTempChild->GetType() == WINDOW_TIMEBOX) )
                    {
                        if ( !cAccel )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Labels befor Fields (Edit,ListBox,...) should have a mnemonic char (~): %s",
                                         OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                        }
                        if ( !pTempChild->IsEnabled() && pChild->IsEnabled() )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Labels befor Fields (Edit,ListBox,...) should be disabled, when the field is disabled: %s",
                                         OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                        }
                    }
                }
            }

            SAL_WARN_IF(
                (pChild->GetType() == WINDOW_MULTILINEEDIT
                 && (pChild->GetStyle() & (WB_IGNORETAB | WB_READONLY)) == 0),
                "vcl",
                ("editable MultiLineEdits in Dialogs should have the Style"
                 " WB_IGNORETAB"));

            if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_CHECKBOX) ||
                 (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                 (pChild->GetType() == WINDOW_FIXEDTEXT) )
            {
                pChild->SetBackground( Wallpaper( Color( COL_LIGHTGREEN ) ) );
            }

            if ( pChild->IsVisible() )
            {
                bool bMaxWarning = false;
                if ( pChild->GetType() == WINDOW_NUMERICFIELD )
                {
                    NumericField* pField = (NumericField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_METRICFIELD )
                {
                    MetricField* pField = (MetricField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYFIELD )
                {
                    CurrencyField* pField = (CurrencyField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_TIMEFIELD )
                {
                    TimeField* pField = (TimeField*)pChild;
                    if ( pField->GetMax() == Time( 23, 59, 59, 99 ) )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_DATEFIELD )
                {
                    DateField* pField = (DateField*)pChild;
                    if ( pField->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_NUMERICBOX )
                {
                    NumericBox* pBox = (NumericBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_METRICBOX )
                {
                    MetricBox* pBox = (MetricBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYBOX )
                {
                    CurrencyBox* pBox = (CurrencyBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_TIMEBOX )
                {
                    TimeBox* pBox = (TimeBox*)pChild;
                    if ( pBox->GetMax() == Time( 23, 59, 59, 99 ) )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_DATEBOX )
                {
                    DateBox* pBox = (DateBox*)pChild;
                    if ( pBox->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = true;
                }
                if ( bMaxWarning )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "No Max-Value is set: %s",
                                 OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                }

                if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                     (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                     (pChild->GetType() == WINDOW_CHECKBOX) ||
                     (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                     (pChild->GetType() == WINDOW_PUSHBUTTON) ||
                     (pChild->GetType() == WINDOW_OKBUTTON) ||
                     (pChild->GetType() == WINDOW_CANCELBUTTON) ||
                     (pChild->GetType() == WINDOW_HELPBUTTON) ||
                     (pChild->GetType() == WINDOW_IMAGEBUTTON) ||
                     (pChild->GetType() == WINDOW_FIXEDTEXT) ||
                     (pChild->GetType() == WINDOW_EDIT) ||
                     (pChild->GetType() == WINDOW_MULTILINEEDIT) ||
                     (pChild->GetType() == WINDOW_SPINFIELD) ||
                     (pChild->GetType() == WINDOW_PATTERNFIELD) ||
                     (pChild->GetType() == WINDOW_NUMERICFIELD) ||
                     (pChild->GetType() == WINDOW_METRICFIELD) ||
                     (pChild->GetType() == WINDOW_CURRENCYFIELD) ||
                     (pChild->GetType() == WINDOW_DATEFIELD) ||
                     (pChild->GetType() == WINDOW_TIMEFIELD) ||
                     (pChild->GetType() == WINDOW_LISTBOX) ||
                     (pChild->GetType() == WINDOW_MULTILISTBOX) ||
                     (pChild->GetType() == WINDOW_COMBOBOX) ||
                     (pChild->GetType() == WINDOW_PATTERNBOX) ||
                     (pChild->GetType() == WINDOW_NUMERICBOX) ||
                     (pChild->GetType() == WINDOW_METRICBOX) ||
                     (pChild->GetType() == WINDOW_CURRENCYBOX) ||
                     (pChild->GetType() == WINDOW_DATEBOX) ||
                     (pChild->GetType() == WINDOW_TIMEBOX) )
                {
                    Point       aNewPos = pChild->GetPosPixel();
                    Rectangle   aChildRect( aNewPos, pChild->GetSizePixel() );

                    if ( cAccel || (pChild->GetStyle() & WB_TABSTOP) ||
                         (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                         (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) )
                    {
                        if ( (aNewPos.X() <= aTabPos.X()) && (aNewPos.Y() <= aTabPos.Y()) )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Possible wrong childorder for dialogcontrol: %s",
                                         OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                        }
                        aTabPos = aNewPos;
                    }

                    for ( sal_uInt16 j = 0; j < i; j++ )
                    {
                        if ( ((pRectAry[j].Right() != 0) || (pRectAry[j].Bottom() != 0)) &&
                             aChildRect.IsOver( pRectAry[j] ) )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Window overlaps with sibling window: %s",
                                         OUStringToOString(aErrorText, RTL_TEXTENCODING_UTF8).getStr() );
                        }
                    }
                    pRectAry[i] = aChildRect;
                }
            }
        }

        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
        i++;
    }

    delete [] pRectAry;
}

// =======================================================================
#ifndef WNT
#define USE_VCL_MSGBOX
#define COPY_BUTTON_ID 25

class DbgMessageBox : public ErrorBox
{
    OUString m_aMessage;
    public:
    DbgMessageBox( const OUString& rMessage ) :
       ErrorBox( NULL, WB_YES_NO_CANCEL | WB_DEF_NO, rMessage ),
       m_aMessage( rMessage )
    {
        SetText("Debug Output");
        AddButton(OUString("Copy"), COPY_BUTTON_ID, 0);
    }

    virtual void Click()
    {
        if( GetCurButtonId() == COPY_BUTTON_ID )
            vcl::unohelper::TextDataObject::CopyStringTo( m_aMessage, GetClipboard() );
        else
            ErrorBox::Click();
    }
};

#endif

class SolarMessageBoxExecutor : public ::vcl::SolarThreadExecutor
{
private:
    OUString  m_sDebugMessage;

public:
    SolarMessageBoxExecutor( const OUString& _rDebugMessage )
        :m_sDebugMessage( _rDebugMessage )
    {
    }

protected:
    virtual long doIt();
};

long SolarMessageBoxExecutor::doIt()
{
    long nResult = RET_NO;

    // Stop tracking and release mouse, to assure boxes do not hang
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();

#if ! defined USE_VCL_MSGBOX
#ifdef WNT
    sal_Bool bOldCallTimer = pSVData->mbNoCallTimer;
    pSVData->mbNoCallTimer = sal_True;
    nResult = MessageBoxW( 0, (LPWSTR)m_sDebugMessage.getStr(), L"Debug Output",
                                     MB_TASKMODAL | MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP );
    pSVData->mbNoCallTimer = bOldCallTimer;
    switch ( nResult )
    {
        case IDYES:
            nResult = RET_YES;
            break;
        case IDNO:
            nResult = RET_NO;
            break;
        case IDCANCEL:
            nResult = RET_CANCEL;
            break;
    }
#endif // WNT
#else
    sal_uInt16 nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    DbgMessageBox aBox( m_sDebugMessage );
    Application::SetSystemWindowMode( nOldMode );
    nResult = aBox.Execute();
#endif

    return nResult;
}

void DbgPrintMsgBox( const char* pLine )
{
    // are modal message boxes prohibited at the moment?
    if ( Application::IsDialogCancelEnabled() )
    {
#if defined( WNT )
        // TODO: Shouldn't this be a IsDebuggerPresent()?
        if ( GetSystemMetrics( SM_DEBUG ) )
        {
            strcpy( aDbgOutBuf, pLine );
            strcat( aDbgOutBuf, "\r\n" );
            OutputDebugString( aDbgOutBuf );
            return;
        }
#endif

#ifdef UNX
        fprintf( stderr, "%s\n", pLine );
        return;
#else
        DbgPrintFile( pLine );
        return;
#endif
    }

    strcpy( aDbgOutBuf, pLine );
#if defined UNX && HAVE_FEATURE_DESKTOP
    strcat( aDbgOutBuf, "\nAbort ? (Yes=abort / No=ignore / Cancel=core dump)" );
#elif defined _WIN32
    strcat( aDbgOutBuf, "\nAbort ? (Yes=abort / No=ignore / Cancel=try to invoke debugger)" );
#else
    strcat( aDbgOutBuf, "\nAbort ? (Yes=abort / No=ignore / Cancel=crash)" );
#endif

    SolarMessageBoxExecutor aMessageBox( OUString( aDbgOutBuf, strlen(aDbgOutBuf), RTL_TEXTENCODING_UTF8 ) );
    TimeValue aTimeout; aTimeout.Seconds = 2; aTimeout.Nanosec = 0;
    long nResult = aMessageBox.execute( aTimeout );

    if ( aMessageBox.didTimeout() )
        DbgPrintShell( pLine );
    else if ( nResult == RET_YES )
        GetpApp()->Abort( OUString("Debug-Utilities-Error") );
    else if ( nResult == RET_CANCEL )
        DbgCoreDump();
}

// -----------------------------------------------------------------------

class SolarWindowPrinter : public ::vcl::SolarThreadExecutor
{
private:
    OUString  m_sDebugMessage;

public:
    SolarWindowPrinter( const OUString& _rDebugMessage )
        :m_sDebugMessage( _rDebugMessage )
    {
    }

protected:
    virtual long doIt();
};

long SolarWindowPrinter::doIt()
{
    DbgWindow* pDbgWindow = ImplGetSVData()->maWinData.mpDbgWin;
    if ( !pDbgWindow )
    {
        pDbgWindow = new DbgWindow;
        ImplGetSVData()->maWinData.mpDbgWin = pDbgWindow;
    }
    pDbgWindow->InsertLine( m_sDebugMessage );

    return 0L;
}

// -----------------------------------------------------------------------

void DbgPrintWindow( const char* pLine )
{
    static bool bIn = false;

    // keine rekursiven Traces
    if ( bIn )
        return;
    bIn = true;

    SolarWindowPrinter aPrinter( OUString( pLine, strlen(pLine), RTL_TEXTENCODING_UTF8 ) );
    TimeValue aTimeout; aTimeout.Seconds = 2; aTimeout.Nanosec = 0;
    aPrinter.execute( aTimeout );

    if ( aPrinter.didTimeout() )
        DbgPrintShell( pLine );

    bIn = false;
}

// -----------------------------------------------------------------------

void DbgAbort( char const * i_message )
{
    OUString const message( i_message, strlen( i_message ), osl_getThreadTextEncoding() );
    Application::Abort( message );
}

// =======================================================================

void ImplDbgTestSolarMutex()
{
    assert(ImplGetSVData()->mpDefInst->CheckYieldMutex());
}

// =======================================================================

void DbgGUIInit()
{
    DbgSetPrintMsgBox( DbgPrintMsgBox );
    DbgSetPrintWindow( DbgPrintWindow );
    DbgSetTestSolarMutex( ImplDbgTestSolarMutex );
    DbgSetAbort( DbgAbort );
}

// -----------------------------------------------------------------------

void DbgGUIDeInit()
{
    DbgSetPrintMsgBox( NULL );
    DbgSetPrintWindow( NULL );
    DbgSetTestSolarMutex( NULL );
    DbgSetAbort( NULL );

    DbgWindow* pDbgWindow = ImplGetSVData()->maWinData.mpDbgWin;
    delete pDbgWindow;
}

// -----------------------------------------------------------------------

void DbgGUIStart()
{
    DbgData* pData = DbgGetData();

    if ( pData )
    {
        DbgDialog* pDialog = new DbgDialog;
        // we switch off dialog tests for the debug dialog
        sal_uLong nOldFlags = pData->nTestFlags;
        pData->nTestFlags &= ~DBG_TEST_DIALOG;
        if ( !pDialog->Execute() )
            pData->nTestFlags |= (nOldFlags & DBG_TEST_DIALOG);
        delete pDialog;
    }
    else
    {
        ErrorBox( 0, WB_OK, OUString("TOOLS Library has no Debug-Routines") ).Execute();
    }
}

// -----------------------------------------------------------------------

sal_uInt16 DbgRegisterNamedUserChannel( const OUString& _rChannelUIName, DbgPrintLine pProc )
{
    DbgChannelId nChannelId = DbgRegisterUserChannel( pProc );
    UserDefinedChannels& rChannels = ImplDbgGetUserDefinedChannels();
    rChannels[ _rChannelUIName ] = nChannelId;
    return nChannelId;
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
