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
#include "precompiled_vcl.hxx"

#include <sal/config.h>

#ifdef DBG_UTIL

#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits.h>

#include "vcl/svdata.hxx"
#include "svsys.h"

#ifdef WNT
#undef min
#endif
#include "tools/debug.hxx"
#include "vcl/svdata.hxx"
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
#include "vcl/sound.hxx"
#include "vcl/threadex.hxx"
#include "vcl/dbggui.hxx"
#include "com/sun/star/i18n/XCharacterClassification.hpp"

#include "vcl/unohelp.hxx"
#include "vcl/unohelp2.hxx"
#include <osl/mutex.hxx>
#include "vcl/salinst.hxx"

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
"Memory Test\n",
"------------------------------------------\n",
"\n",
"--- Macros ---\n",
"DBG_MEMTEST()\n",
"Run the specified memory tests.\n",
"\n",
"DBG_MEMTEST_PTR( p )\n",
"Runs the specified memory tests and validates the pointer that was "
"passed if the pointer test is enabled.\n",
"\n",
"--- Options ---\n",
"Initialize\n",
"Allocated memory is initialized with 0x77 and free or freed memory "
"is initialized with 0x33. This option has almost no impact on performance "
"and should thus always be enabled during development. This will also "
"make crashes more often reproducable.\n",
"\n",
"Overwrite\n",
"This test check whether writes occur before or after the blocks. Before "
"and after the block memory is initialized with 0x55. This option costs "
"performance, but should be enabled once in a while to test for common "
"memory overwrites (+-1 errors). This option should also be enabled if the "
"program crashes in a new or delete operator.\n",
"\n",
"Free\n",
"This checks whether writes occur in free memory. This option costs lots "
" of performance and should thus only be used to test memory overwrites. "
" This option should perhaps also be enabled when the program crashes "
" in the new or delete operator.\n",
"\n",
"Pointer\n",
"The pointer is tested with delete and DBG_MEMTEST_PTR() to see if it was "
"created by new or SvMemAlloc(). When this option is enabled errors such as "
"double deletes, deletes on stack objects or invalid pointers will be found. "
"This option has an impact on performance and should therefor not be enabled "
"all the time. However, testing should be done with this option enabled once "
"in a while, because the memory manager does not always crash with delete and "
"invalid pointers. This option should also be enabled if the program crashes "
"in new or delete operators.\n",
"\n",
"Report\n",
"At the end of the program a small statistic and memory that was not freed are "
"output. Note: memory that is freed by global objects is also included in "
"the leak list.\n",
"\n",
"Trace\n",
"Allocating and freeing memory is logged.\n",
"\n",
"Leak report\n",
"Produces under WNT at the end of the program a list of memory leaks with "
"stack trace. Only blocks which were created inside Application::Execute() "
"are included. When this option and Overwrite are both enabled a memory "
"overwrite results in an attempt to output the stack where the block was "
"created. The output is included in the log file after the error message.\n"
"\n",
"New/Delete\n",
"Memory tests are performed on the entire memory with every new/delet. "
"Warning: this option makes programs very slow and should only be enabled "
"to track memory overwrites. Otherwise it is sufficient to enable "
"seperate options because (if no leak is present) every detectable "
"memory overwrite during run time should be found.\n",
"\n",
"Object Test\n",
"Memory test are performed on the entire memory with every object test. "
"Warning: this option makes programs very slow and should only be enabled "
"to track memory overwrite. Otherwise it is sufficient to enable "
"seperate options because (if no leak is present) every detectable "
"memory overwrite during run time should be found.\n",
"\n",
"Windows 16-bit and debug tests\n",
"Warning: when memory test are enabled (except for Initialize) memory with "
"offset 0 is never (even not in case of >= 64KB) returned. If necessary the "
"tests can be performed with 32-bit versions of the programs. To a certain "
"extend it is sufficient to create 64KB - 64 bytes instead of 64KB because "
"it will never come to a segment overflow.\n",
"Memory and object test should only be enabled when only one SV application "
"is running at one time. Otherwise uncontrolled errors may occur. In this "
"case only the use of 32-bit programs can help."
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
"DBG_WARNING() can be used to output warnings. DBG_WARNINGFILE() also outputs "
"the file and the line number where the macro is located. DBG_WARNING1() to "
"DBG_WARNING5() can be used to produce formatted output (printf format string). "
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
"the object test. Filters are seperated by ';' and are case sensitive. "
"Wildcards are not supported. If no text is indicated the filters are not "
"active.\n",
"\n",
"Exclude ObjectTest filters\n",
"Only classes which do not contain one of the indicated filters are evaluated "
"with the object test. Filters are seperated by ';' and are case sensitive. "
"Wildcards are not supported. If no text is indicated the filters are not "
"active.\n",
"\n",
"Include filters\n",
"Only those texts which include the indicated filters are output. "
"Filters are seperated by ';' and are case sensitive. "
"Wildcards are not supported. The filter is used for all output (except for "
"errors). If no text is indicated the filters are not active.\n",
"\n",
"Exclude filters\n",
"Only those texts which do not include the indicated filters are output. "
"Filters are seperated by ';' and are case sensitive. "
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
    typedef ::std::map< XubString, DbgChannelId > UserDefinedChannels;
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
    void            InsertLine( const XubString& rLine );
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
    sal_Bool            mbHelpText;

public:
                    DbgInfoDialog( Window* pParent, sal_Bool bHelpText = sal_False );

    void            SetInfoText( const XubString& rStr );
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

    CheckBox        maMemInit;
    CheckBox        maMemOverwrite;
    CheckBox        maMemOverwriteFree;
    CheckBox        maMemPtr;
    CheckBox        maMemReport;
    CheckBox        maMemTrace;
    CheckBox        maMemLeakReport;
    CheckBox        maMemNewDel;
    CheckBox        maMemXtor;
    GroupBox        maBox2;

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
        ByteString aState( pData->aDbgWinState );
        SetWindowState( aState );
    }

    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "StarView Debug Window" ) ) );
    Show();
    Update();
}

// -----------------------------------------------------------------------

sal_Bool DbgWindow::Close()
{
    // remember window position
    ByteString aState( GetWindowState() );
    DbgData* pData = DbgGetData();
    strncpy( pData->aDbgWinState,
         aState.GetBuffer(),
         std::min( sizeof( pData->aDbgWinState ),
               size_t(aState.Len() + 1U )) );
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
                aMenu.InsertItem( 1, String::CreateFromAscii( "copy to clipboard" ) );

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
                    String sAssertion;
                    String sLineFeed = String::CreateFromAscii( "\n" );
                    sLineFeed.ConvertLineEnd( GetSystemLineEnd() );
                    while ( nAssertionFirst < nAssertionLast )
                    {
                        sAssertion += maLstBox.GetEntry( nAssertionFirst++ );
                        sAssertion += sLineFeed;
                    }

                    ::vcl::unohelper::TextDataObject::CopyStringTo( sAssertion, GetClipboard() );
                }
            }
            return 1;   // handled
        }
    }
    return WorkWindow::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void DbgWindow::InsertLine( const XubString& rLine )
{
    XubString aStr = rLine;
    aStr.ConvertLineEnd( LINEEND_LF );
    xub_StrLen  nPos = aStr.Search( _LF );
    sal_Bool bFirstEntry = sal_True;
    while ( nPos != STRING_NOTFOUND )
    {
        if ( maLstBox.GetEntryCount() >= DBGWIN_MAXLINES )
            maLstBox.RemoveEntry( 0 );

        sal_uInt16 nInsertionPos = maLstBox.InsertEntry( aStr.Copy( 0, nPos ) );
        if ( bFirstEntry )
            maLstBox.SetEntryData( nInsertionPos, reinterpret_cast< void* >( 0x00000001 ) );
        bFirstEntry = sal_False;

        aStr.Erase( 0, nPos+1 );
        nPos = aStr.Search( _LF );
    }
    if ( maLstBox.GetEntryCount() >= DBGWIN_MAXLINES )
        maLstBox.RemoveEntry( 0 );
    sal_uInt16 nInsertionPos = maLstBox.InsertEntry( aStr );
    if ( bFirstEntry )
        maLstBox.SetEntryData( nInsertionPos, reinterpret_cast< void* >( 0x00000001 ) );
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
    maMemInit( this ),
    maMemOverwrite( this ),
    maMemOverwriteFree( this ),
    maMemPtr( this ),
    maMemReport( this ),
    maMemTrace( this ),
    maMemLeakReport( this ),
    maMemNewDel( this ),
    maMemXtor( this ),
    maBox2( this ),
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
    maXtorThis.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "T~his" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_THIS )
        maXtorThis.Check( sal_True );
    maXtorThis.SetPosSizePixel( LogicToPixel( Point( 10, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorFunc.Show();
    maXtorFunc.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Function" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_FUNC )
        maXtorFunc.Check( sal_True );
    maXtorFunc.SetPosSizePixel( LogicToPixel( Point( 75, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorExit.Show();
    maXtorExit.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "E~xit" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_EXIT )
        maXtorExit.Check( sal_True );
    maXtorExit.SetPosSizePixel( LogicToPixel( Point( 140, 15 ), aAppMap ),
                                aButtonSize );
    }

    {
    maXtorReport.Show();
    maXtorReport.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Report" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_REPORT )
        maXtorReport.Check( sal_True );
    maXtorReport.SetPosSizePixel( LogicToPixel( Point( 205, 15 ), aAppMap ),
                                  aButtonSize );
    }

    {
    maXtorTrace.Show();
    maXtorTrace.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Trace" ) ) );
    if ( pData->nTestFlags & DBG_TEST_XTOR_TRACE )
        maXtorTrace.Check( sal_True );
    maXtorTrace.SetPosSizePixel( LogicToPixel( Point( 270, 15 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maBox1.Show();
    maBox1.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Object Tests" ) ) );
    maBox1.SetPosSizePixel( LogicToPixel( Point( 5, 5 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maMemInit.Show();
    maMemInit.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Initialize" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_INIT )
        maMemInit.Check( sal_True );
    maMemInit.SetPosSizePixel( LogicToPixel( Point( 10, 50 ), aAppMap ),
                               aButtonSize );
    }

    {
    maMemOverwrite.Show();
    maMemOverwrite.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Overwrite" )) );
    if ( pData->nTestFlags & DBG_TEST_MEM_OVERWRITE )
        maMemOverwrite.Check( sal_True );
    maMemOverwrite.SetPosSizePixel( LogicToPixel( Point( 75, 50 ), aAppMap ),
                                    aButtonSize );
    }

    {
    maMemOverwriteFree.Show();
    maMemOverwriteFree.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Free" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_OVERWRITEFREE )
        maMemOverwriteFree.Check( sal_True );
    maMemOverwriteFree.SetPosSizePixel( LogicToPixel( Point( 140, 50 ), aAppMap ),
                                        aButtonSize );
    }

    {
    maMemPtr.Show();
    maMemPtr.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Pointer" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_POINTER )
        maMemPtr.Check( sal_True );
    maMemPtr.SetPosSizePixel( LogicToPixel( Point( 205, 50 ), aAppMap ),
                              aButtonSize );
    }

    {
    maMemReport.Show();
    maMemReport.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Report" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_REPORT )
        maMemReport.Check( sal_True );
    maMemReport.SetPosSizePixel( LogicToPixel( Point( 270, 50 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maMemTrace.Show();
    maMemTrace.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Trace" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_TRACE )
        maMemTrace.Check( sal_True );
    maMemTrace.SetPosSizePixel( LogicToPixel( Point( 10, 65 ), aAppMap ),
                                aButtonSize );
    }

    {
    maMemLeakReport.Show();
    maMemLeakReport.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Leak-Report" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_LEAKREPORT )
        maMemLeakReport.Check( sal_True );
    maMemLeakReport.SetPosSizePixel( LogicToPixel( Point( 75, 65 ), aAppMap ),
                                     aButtonSize );
    }

    {
    maMemNewDel.Show();
    maMemNewDel.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~New/Delete" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_NEWDEL )
        maMemNewDel.Check( sal_True );
    maMemNewDel.SetPosSizePixel( LogicToPixel( Point( 140, 65 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maMemXtor.Show();
    maMemXtor.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Ob~ject Test" ) ) );
    if ( pData->nTestFlags & DBG_TEST_MEM_XTOR )
        maMemXtor.Check( sal_True );
    maMemXtor.SetPosSizePixel( LogicToPixel( Point( 205, 65 ), aAppMap ),
                               aButtonSize );
    }

    {
    maBox2.Show();
    maBox2.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Memory Tests" ) ) );
    maBox2.SetPosSizePixel( LogicToPixel( Point( 5, 40 ), aAppMap ),
                            LogicToPixel( Size( 330, 40 ), aAppMap ) );
    }

    {
    maProf.Show();
    maProf.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Profiling" ) ) );
    if ( pData->nTestFlags & DBG_TEST_PROFILING )
        maProf.Check( sal_True );
    maProf.SetPosSizePixel( LogicToPixel( Point( 10, 95 ), aAppMap ),
                            aButtonSize );
    }

    {
    maRes.Show();
    maRes.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Resourcen" ) ) );
    if ( pData->nTestFlags & DBG_TEST_RESOURCE )
        maRes.Check( sal_True );
    maRes.SetPosSizePixel( LogicToPixel( Point( 75, 95 ), aAppMap ),
                           aButtonSize );
    }

    {
    maDialog.Show();
    maDialog.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Dialog" ) ) );
    if ( pData->nTestFlags & DBG_TEST_DIALOG )
        maDialog.Check( sal_True );
    maDialog.SetPosSizePixel( LogicToPixel( Point( 140, 95 ), aAppMap ),
                              aButtonSize );
    }

    {
    maBoldAppFont.Show();
    maBoldAppFont.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Bold AppFont" ) ) );
    if ( pData->nTestFlags & DBG_TEST_BOLDAPPFONT )
        maBoldAppFont.Check( sal_True );
    maBoldAppFont.SetPosSizePixel( LogicToPixel( Point( 205, 95 ), aAppMap ),
                                   aButtonSize );
    maBoldAppFont.SaveValue();
    }

    {
    maBox3.Show();
    maBox3.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Test Options" ) ) );
    maBox3.SetPosSizePixel( LogicToPixel( Point( 5, 85 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maDebugName.Show();
    maDebugName.SetText( XubString( pData->aDebugName, RTL_TEXTENCODING_UTF8 ) );
    maDebugName.SetMaxTextLen( sizeof( pData->aDebugName ) );
    maDebugName.SetPosSizePixel( LogicToPixel( Point( 10, 130 ), aAppMap ),
                                 LogicToPixel( Size( 185, 14 ), aAppMap ) );
    }

    {
    maOverwrite.Show();
    maOverwrite.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Overwrite ~File" ) ) );
    if ( pData->bOverwrite )
        maOverwrite.Check( sal_True );
    maOverwrite.SetPosSizePixel( LogicToPixel( Point( 205, 130 ), aAppMap ),
                                 aButtonSize );
    }

    {
    maHookOSLBox.Show();
    maHookOSLBox.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Reroute osl debug ~messages" ) ) );
    if ( pData->bHookOSLAssert )
        maHookOSLBox.Check( sal_True );
    maHookOSLBox.SetPosSizePixel( LogicToPixel( Point( 10, 240 ), aAppMap ),
                                  LogicToPixel( Size( 100, 12 ), aAppMap ) );
    }

    {
    maInclClassText.Show();
    maInclClassText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Include-ObjectTest-Filter" ) ) );
    maInclClassText.SetPosSizePixel( LogicToPixel( Point( 10, 150 ), aAppMap ),
                                     LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maInclClassFilter.Show();
    maInclClassFilter.SetText( XubString( pData->aInclClassFilter, RTL_TEXTENCODING_UTF8 ) );
    maInclClassFilter.SetMaxTextLen( sizeof( pData->aInclClassFilter ) );
    maInclClassFilter.SetPosSizePixel( LogicToPixel( Point( 10, 160 ), aAppMap ),
                                       LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maExclClassText.Show();
    maExclClassText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Exclude-ObjectTest-Filter" ) ) );
    maExclClassText.SetPosSizePixel( LogicToPixel( Point( 115, 150 ), aAppMap ),
                                     LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maExclClassFilter.Show();
    maExclClassFilter.SetText( XubString( pData->aExclClassFilter, RTL_TEXTENCODING_UTF8 ) );
    maExclClassFilter.SetMaxTextLen( sizeof( pData->aExclClassFilter ) );
    maExclClassFilter.SetPosSizePixel( LogicToPixel( Point( 115, 160 ), aAppMap ),
                                       LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maInclText.Show();
    maInclText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Include-Filter" ) ) );
    maInclText.SetPosSizePixel( LogicToPixel( Point( 10, 180 ), aAppMap ),
                                LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maInclFilter.Show();
    maInclFilter.SetText( XubString( pData->aInclFilter, RTL_TEXTENCODING_UTF8 ) );
    maInclFilter.SetMaxTextLen( sizeof( pData->aInclFilter ) );
    maInclFilter.SetPosSizePixel( LogicToPixel( Point( 10, 190 ), aAppMap ),
                                  LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maExclText.Show();
    maExclText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Exclude-Filter" ) ) );
    maExclText.SetPosSizePixel( LogicToPixel( Point( 115, 180 ), aAppMap ),
                                LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maExclFilter.Show();
    maExclFilter.SetText( XubString( pData->aExclFilter, RTL_TEXTENCODING_UTF8 ) );
    maExclFilter.SetMaxTextLen( sizeof( pData->aExclFilter ) );
    maExclFilter.SetPosSizePixel( LogicToPixel( Point( 115, 190 ), aAppMap ),
                                  LogicToPixel( Size( 95, 14 ), aAppMap ) );
    }

    {
    maTraceText.Show();
    maTraceText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Trace" ) ) );
    maTraceText.SetPosSizePixel( LogicToPixel( Point( 10, 210 ), aAppMap ),
                                 LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Window" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Shell" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "MessageBox" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "TestTool" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debugger" ) ) );
    maTraceBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Abort" ) ) );
    ImplAppendUserDefinedChannels( maTraceBox );
    ImplSelectChannel( maTraceBox, pData->nTraceOut, 0 );
    maTraceBox.Show();
    maTraceBox.SetPosSizePixel( LogicToPixel( Point( 10, 220 ), aAppMap ),
                                LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maWarningText.Show();
    maWarningText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Warning" ) ) );
    maWarningText.SetPosSizePixel( LogicToPixel( Point( 115, 210 ), aAppMap ),
                                   LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Window" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Shell" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "MessageBox" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "TestTool" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debugger" ) ) );
    maWarningBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Abort" ) ) );
    ImplAppendUserDefinedChannels( maWarningBox );
    ImplSelectChannel( maWarningBox, pData->nWarningOut, 0 );
    maWarningBox.Show();
    maWarningBox.SetPosSizePixel( LogicToPixel( Point( 115, 220 ), aAppMap ),
                                  LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maErrorText.Show();
    maErrorText.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Error" ) ) );
    maErrorText.SetPosSizePixel( LogicToPixel( Point( 220, 210 ), aAppMap ),
                                 LogicToPixel( Size( 95, 9 ), aAppMap ) );
    }

    {
    if ( DbgIsAllErrorOut() )
    {
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) );
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Window" ) ) );
        maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Shell" ) ) );
        mnErrorOff = 0;
    }
    else
        mnErrorOff = 4;
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "MessageBox" ) ) );
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "TestTool" ) ) );
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debugger" ) ) );
    maErrorBox.InsertEntry( XubString( RTL_CONSTASCII_USTRINGPARAM( "Abort" ) ) );
    ImplAppendUserDefinedChannels( maErrorBox );
    ImplSelectChannel( maErrorBox, pData->nErrorOut, mnErrorOff );
    maErrorBox.Show();
    maErrorBox.SetPosSizePixel( LogicToPixel( Point( 220, 220 ), aAppMap ),
                                LogicToPixel( Size( 95, 80 ), aAppMap ) );
    }

    {
    maBox4.Show();
    maBox4.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Output" ) ) );
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
    maInfoButton.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "~Info..." ) ) );
    maInfoButton.SetPosSizePixel( LogicToPixel( Point( 130, 260 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maHelpButton.Show();
    maHelpButton.SetPosSizePixel( LogicToPixel( Point( 190, 260 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }

    {
    SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "VCL Debug Options" ) ) );
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

        strncpy( aData.aDebugName, ByteString( maDebugName.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aDebugName ) );
        strncpy( aData.aInclClassFilter, ByteString( maInclClassFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aInclClassFilter ) );
        strncpy( aData.aExclClassFilter, ByteString( maExclClassFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aExclClassFilter ) );
        strncpy( aData.aInclFilter, ByteString( maInclFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aInclFilter ) );
        strncpy( aData.aExclFilter, ByteString( maExclFilter.GetText(), RTL_TEXTENCODING_UTF8 ).GetBuffer(), sizeof( aData.aExclFilter ) );
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

        if ( maMemInit.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_INIT;

        if ( maMemOverwrite.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_OVERWRITE;

        if ( maMemOverwriteFree.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_OVERWRITEFREE;

        if ( maMemPtr.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_POINTER;

        if ( maMemReport.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_REPORT;

        if ( maMemTrace.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_TRACE;

        if ( maMemLeakReport.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_LEAKREPORT;

        if ( maMemNewDel.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_NEWDEL;

        if ( maMemXtor.IsChecked() )
            aData.nTestFlags |= DBG_TEST_MEM_XTOR;

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
        #define IMMEDIATE_FLAGS (DBG_TEST_MEM_INIT | DBG_TEST_RESOURCE | DBG_TEST_DIALOG | DBG_TEST_BOLDAPPFONT)
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
            InfoBox aBox( this, String( RTL_CONSTASCII_USTRINGPARAM(
                "Some of the changed settings will only be active after "
                "restarting the process"
                ) ) );
            aBox.Execute();
        }
        EndDialog( sal_True );
    }
    else if ( pButton == &maInfoButton )
    {
        DbgInfoDialog aInfoDialog( this );
        aDbgInfoBuf[0] = '\0';
        DbgMemInfo( aDbgInfoBuf );
        DbgXtorInfo( aDbgInfoBuf );
        XubString aInfoText( aDbgInfoBuf, RTL_TEXTENCODING_UTF8 );
        aInfoDialog.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debug InfoReport" ) ) );
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
        DbgInfoDialog aInfoDialog( this, sal_True );
        XubString aHelpText;
        const sal_Char** pHelpStrs = pDbgHelpText;
        while ( *pHelpStrs )
        {
            aHelpText.AppendAscii( *pHelpStrs );
            pHelpStrs++;
        }
        aInfoDialog.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debug Hilfe" ) ) );
        aInfoDialog.SetInfoText( aHelpText );
        aInfoDialog.Execute();
    }
}

// =======================================================================

DbgInfoDialog::DbgInfoDialog( Window* pParent, sal_Bool bHelpText ) :
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

void DbgInfoDialog::SetInfoText( const XubString& rStr )
{
    maListBox.SetUpdateMode( sal_False );
    maListBox.Clear();
    XubString aStr = rStr;
    aStr.ConvertLineEnd( LINEEND_LF );
    sal_uInt16 nStrIndex = 0;
    sal_uInt16 nFoundIndex;
    do
    {
        nFoundIndex = aStr.Search( _LF, nStrIndex );
        XubString aTextParagraph = aStr.Copy( nStrIndex, nFoundIndex-nStrIndex );
        if ( mbHelpText )
        {
            long    nMaxWidth = maListBox.GetOutputSizePixel().Width()-30;
            sal_uInt16  nLastIndex = 0;
            sal_uInt16  nIndex = aTextParagraph.Search( ' ' );
            while ( nIndex != STRING_NOTFOUND )
            {
                if ( maListBox.GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
                {
                    if ( !nLastIndex )
                        nLastIndex = nIndex+1;
                    XubString aTempStr = aTextParagraph.Copy( 0, nLastIndex );
                    aTextParagraph.Erase( 0, nLastIndex );
                    maListBox.InsertEntry( aTempStr );
                    nLastIndex = 0;
                }
                else
                    nLastIndex = nIndex+1;
                nIndex = aTextParagraph.Search( ' ', nLastIndex );
            }

            if ( maListBox.GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
            {
                if ( !nLastIndex )
                    nLastIndex = nIndex+1;
                XubString aTempStr = aTextParagraph.Copy( 0, nLastIndex );
                aTextParagraph.Erase( 0, nLastIndex );
                maListBox.InsertEntry( aTempStr );
            }
        }
        maListBox.InsertEntry( aTextParagraph );
        nStrIndex = nFoundIndex+1;
    }
    while ( nFoundIndex != STRING_NOTFOUND );
    maListBox.SetUpdateMode( sal_True );
}

// =======================================================================

void DbgDialogTest( Window* pWindow )
{
    sal_Bool        aAccelBuf[65536];
    sal_uInt16      nChildCount = pWindow->GetChildCount();
    Window*     pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    Window*     pChild;
    Point       aTabPos;

    if ( !pGetChild )
        return;

    Rectangle*  pRectAry = (Rectangle*)new long[(sizeof(Rectangle)*nChildCount)/sizeof(long)];
    memset( aAccelBuf, 0, sizeof( aAccelBuf ) );
    memset( pRectAry, 0, sizeof(Rectangle)*nChildCount );

    if ( pWindow->IsDialog() )
    {
        sal_Bool    bOKCancelButton = sal_False;
        sal_Bool    bDefPushButton = sal_False;
        sal_Bool    bButton = sal_False;
        pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        while ( pGetChild )
        {
            pChild = pGetChild->ImplGetWindow();

            if ( pChild->ImplIsPushButton() )
            {
                bButton = sal_True;
                if ( (pChild->GetType() == WINDOW_OKBUTTON) || (pChild->GetType() == WINDOW_CANCELBUTTON) )
                    bOKCancelButton = sal_True;
                if ( pChild->GetStyle() & WB_DEFBUTTON )
                    bDefPushButton = sal_True;
            }

            pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
        }

        if ( bButton )
        {
            if ( !bOKCancelButton )
                DbgError( "Dialogs should have a OK- or CancelButton" );
            if ( !bDefPushButton )
                DbgError( "Dialogs should have a Button with WB_DEFBUTTON" );
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
            XubString       aText = pChild->GetText();
            XubString       aErrorText = aText;
            sal_uInt16          nAccelPos = STRING_NOTFOUND;
            xub_Unicode     cAccel = 0;
            if ( aErrorText.Len() > 128 )
            {
                aErrorText.Erase( 128 );
                aErrorText.AppendAscii( "..." );
            }
            if ( aText.Len() && (aText.Len() < 1024) )
            {
                nAccelPos = aText.Search( '~' );
                if ( nAccelPos != STRING_NOTFOUND )
                {
                    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLocale();
                    uno::Reference < i18n::XCharacterClassification > xCharClass = vcl::unohelper::CreateCharacterClassification();
                    XubString aUpperText = xCharClass->toUpper( aText, 0, aText.Len(), rLocale );
                    cAccel = aUpperText.GetChar( nAccelPos+1 );
                    if ( pChild->IsVisible() )
                    {
                        if ( aAccelBuf[cAccel] )
                            DbgOutTypef( DBG_OUT_ERROR, "Double mnemonic char: %c", cAccel );
                        else
                            aAccelBuf[cAccel] = sal_True;
                    }
                }
            }

            if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_IMAGERADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_CHECKBOX) ||
                 (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                 (pChild->GetType() == WINDOW_PUSHBUTTON) )
            {
                if ( aText.Len() && !aText.EqualsAscii( "..." ) )
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
                                 ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );

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
                                 ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                }
            }

            if ( pChild->GetType() == WINDOW_FIXEDLINE )
            {
                if ( pChild->GetSizePixel().Width() < pChild->GetTextWidth( aText ) )
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "FixedLine exceeds window width: %s",
                                 ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
            }

            if ( pChild->GetType() == WINDOW_FIXEDTEXT )
            {
                if ( (pChild->GetSizePixel().Height() >= pChild->GetTextHeight()*2) &&
                     !(pChild->GetStyle() & WB_WORDBREAK) )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "FixedText greater than one line, but WordBreak is not set: %s",
                                 ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                }

                if ( pChild->IsVisible() )
                {
                    int aWidth=0;
                    if( nAccelPos != STRING_NOTFOUND )
                    {
                        aWidth = pChild->GetTextWidth( aText, 0, nAccelPos ) +
                                 pChild->GetTextWidth( aText, nAccelPos+1, aText.Len() - nAccelPos - 1);
                    }
                    else
                        aWidth = pChild->GetTextWidth( aText );

                    if ( pChild->GetSizePixel().Width() < aWidth && !(pChild->GetStyle() & WB_WORDBREAK) )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "FixedText exceeds window width: %s",
                                         ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                }

                if ( (i+1 < nChildCount) && aText.Len() )
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
                                         ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                        if ( !pTempChild->IsEnabled() && pChild->IsEnabled() )
                        {
                            DbgOutTypef( DBG_OUT_ERROR,
                                         "Labels befor Fields (Edit,ListBox,...) should be disabled, when the field is disabled: %s",
                                         ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                        }
                    }
                }
            }

            if ( pChild->GetType() == WINDOW_MULTILINEEDIT )
            {
                if  (   ( 0 == ( pChild->GetStyle() & WB_IGNORETAB ) )
                    &&  ( 0 == ( pChild->GetStyle() & WB_READONLY ) )
                    )
                {
                    DbgError( "editable MultiLineEdits in Dialogs should have the Style WB_IGNORETAB" );
                }
            }

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
                sal_Bool bMaxWarning = sal_False;
                if ( pChild->GetType() == WINDOW_NUMERICFIELD )
                {
                    NumericField* pField = (NumericField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_METRICFIELD )
                {
                    MetricField* pField = (MetricField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYFIELD )
                {
                    CurrencyField* pField = (CurrencyField*)pChild;
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_TIMEFIELD )
                {
                    TimeField* pField = (TimeField*)pChild;
                    if ( pField->GetMax() == Time( 23, 59, 59, 99 ) )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_DATEFIELD )
                {
                    DateField* pField = (DateField*)pChild;
                    if ( pField->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_NUMERICBOX )
                {
                    NumericBox* pBox = (NumericBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_METRICBOX )
                {
                    MetricBox* pBox = (MetricBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYBOX )
                {
                    CurrencyBox* pBox = (CurrencyBox*)pChild;
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_TIMEBOX )
                {
                    TimeBox* pBox = (TimeBox*)pChild;
                    if ( pBox->GetMax() == Time( 23, 59, 59, 99 ) )
                        bMaxWarning = sal_True;
                }
                else if ( pChild->GetType() == WINDOW_DATEBOX )
                {
                    DateBox* pBox = (DateBox*)pChild;
                    if ( pBox->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = sal_True;
                }
                if ( bMaxWarning )
                {
                    DbgOutTypef( DBG_OUT_ERROR,
                                 "No Max-Value is set: %s",
                                 ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
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
                                         ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
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
                                         ByteString( aErrorText, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
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
    String m_aMessage;
    public:
    DbgMessageBox( const String& rMessage ) :
       ErrorBox( NULL, WB_YES_NO_CANCEL | WB_DEF_NO, rMessage ),
       m_aMessage( rMessage )
    {
        SetText( String( RTL_CONSTASCII_USTRINGPARAM("Debug Output") ) );
        AddButton( String( RTL_CONSTASCII_USTRINGPARAM( "Copy" ) ), COPY_BUTTON_ID, 0 );
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
    String  m_sDebugMessage;

public:
    SolarMessageBoxExecutor( const String& _rDebugMessage )
        :m_sDebugMessage( _rDebugMessage )
    {
    }

protected:
    virtual long doIt();
};

long SolarMessageBoxExecutor::doIt()
{
    long nResult = RET_NO;

    // Tracking beenden und Mouse freigeben, damit die Boxen nicht haengen
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();

#if ! defined USE_VCL_MSGBOX
#ifdef WNT
    sal_Bool bOldCallTimer = pSVData->mbNoCallTimer;
    pSVData->mbNoCallTimer = sal_True;
    MessageBeep( MB_ICONHAND );
    nResult = MessageBoxW( 0, (LPWSTR)m_sDebugMessage.GetBuffer(), L"Debug Output",
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
            MessageBeep( MB_ICONHAND );
            strcpy( aDbgOutBuf, pLine );
            strcat( aDbgOutBuf, "\r\n" );
            OutputDebugString( aDbgOutBuf );
            return;
        }
#endif

        Sound::Beep( SOUND_ERROR );
#ifdef UNX
        fprintf( stderr, "%s\n", pLine );
        return;
#else
        DbgPrintFile( pLine );
        return;
#endif
    }

    strcpy( aDbgOutBuf, pLine );
    strcat( aDbgOutBuf, "\nAbort ? (Yes=abort / No=ignore / Cancel=core dump)" );

    SolarMessageBoxExecutor aMessageBox( String( aDbgOutBuf, RTL_TEXTENCODING_UTF8 ) );
    TimeValue aTimeout; aTimeout.Seconds = 2; aTimeout.Nanosec = 0;
    long nResult = aMessageBox.execute( aTimeout );

    if ( aMessageBox.didTimeout() )
        DbgPrintShell( pLine );
    else if ( nResult == RET_YES )
        GetpApp()->Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "Debug-Utilities-Error" ) ) );
    else if ( nResult == RET_CANCEL )
        DbgCoreDump();
}

// -----------------------------------------------------------------------

class SolarWindowPrinter : public ::vcl::SolarThreadExecutor
{
private:
    String  m_sDebugMessage;

public:
    SolarWindowPrinter( const String& _rDebugMessage )
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
    static sal_Bool bIn = sal_False;

    // keine rekursiven Traces
    if ( bIn )
        return;
    bIn = sal_True;

    SolarWindowPrinter aPrinter( String( pLine, RTL_TEXTENCODING_UTF8 ) );
    TimeValue aTimeout; aTimeout.Seconds = 2; aTimeout.Nanosec = 0;
    aPrinter.execute( aTimeout );

    if ( aPrinter.didTimeout() )
        DbgPrintShell( pLine );

    bIn = sal_False;
}

// -----------------------------------------------------------------------

void DbgAbort( char const * i_message )
{
    ::rtl::OUString const message( i_message, strlen( i_message ), osl_getThreadTextEncoding() );
    Application::Abort( message );
}

// =======================================================================

void ImplDbgTestSolarMutex()
{
    bool bCheck = ImplGetSVData()->mpDefInst->CheckYieldMutex();
    OSL_ENSURE( bCheck, "SolarMutex not locked" );
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
    if ( pDbgWindow )
        delete pDbgWindow;
}

// -----------------------------------------------------------------------

void DbgGUIStart()
{
    DbgData* pData = DbgGetData();

    if ( pData )
    {
        DbgDialog* pDialog = new DbgDialog;
        // Fuer den Debug-Dialog schalten wir Dialogtests aus
        sal_uLong nOldFlags = pData->nTestFlags;
        pData->nTestFlags &= ~DBG_TEST_DIALOG;
        if ( !pDialog->Execute() )
            pData->nTestFlags |= (nOldFlags & DBG_TEST_DIALOG);
        delete pDialog;
    }
    else
    {
        ErrorBox( 0, WB_OK,
                  XubString( RTL_CONSTASCII_USTRINGPARAM( "TOOLS Library has no Debug-Routines" ) ) ).Execute();
    }
}

// -----------------------------------------------------------------------

sal_uInt16 DbgRegisterNamedUserChannel( const XubString& _rChannelUIName, DbgPrintLine pProc )
{
    DbgChannelId nChannelId = DbgRegisterUserChannel( pProc );
    UserDefinedChannels& rChannels = ImplDbgGetUserDefinedChannels();
    rChannels[ _rChannelUIName ] = nChannelId;
    return nChannelId;
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
