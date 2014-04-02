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

#include <tools/debug.hxx>
#include <tools/lineend.hxx>
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
#include "vcl/settings.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/threadex.hxx"

#include "svdata.hxx"
#include "dbggui.hxx"

#include "vcl/unohelp.hxx"
#include "vcl/unohelp2.hxx"

#include "salinst.hxx"
#include "svsys.h"

#include "com/sun/star/i18n/XCharacterClassification.hpp"

#include <algorithm>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;

static const sal_Char* pDbgHelpText[] =
{
"Object Test\n",
"------------------------------------------\n",
"\n",
"--- Macros ---\n",
"\n",
"Function\n",
"When a function is passed with macros, it will be called.\n",
"\n",
"Exit\n",
"This- and Func-Test will also run when exiting the function.\n",
"\n",
"\n",
"\nOther tests and macros\n",
"------------------------------------------\n",
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
"\n",
"Output\n",
"------------------------------------------\n",
"\n",
"You can indicate where the data will be output:\n",
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
"Where by default the INI file is read and written the following "
"can be set:\n",
"\n",
"WIN/WNT (WIN.INI, Group SV, Default: dbgsv.ini):\n",
"INI: dbgsv\n",
"\n",
"OS2 (OS2.INI, Application SV, Default: dbgsv.ini):\n",
"INI: DBGSV\n",
"\n",
"UNIX (Environment variable, Default: .dbgsv.init):\n",
"INI: DBGSV_INIT\n",
"\n",
"MAC (Default: dbgsv.ini):\n",
"INI: not possible\n",
"\n",
"The path and file name must always be specified.\n",
"\n",
"\n",
"Example\n",
"------------------------------------------\n",
"\n",
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
"    // ...\n",
"}\n",
"\n",
"String::~String()\n",
"{\n",
"    //...\n",
"}\n",
"\n",
"char& String::operator [] ( sal_uInt16 nIndex )\n",
"{\n",
"    DBG_ASSERT( nIndex <= pData->nLen, \"String::[] : nIndex > Len\" );\n",
"\n",
"    //...\n",
"}\n",
"\n",
"\n",
NULL
};

#define DBGWIN_MAXLINES     100

class DbgWindow : public WorkWindow
{
private:
    ListBox         maLstBox;

public:
                    DbgWindow();

    virtual bool    Close() SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    void            InsertLine( const OUString& rLine );
    void            Update() { WorkWindow::Update(); maLstBox.Update(); }

private:
    void            GetAssertionEntryRange( sal_uInt16 nInbetweenEntry, sal_uInt16& nFirst, sal_uInt16& nLast );
};

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

class DbgDialog : public ModalDialog
{
private:
    CheckBox        maRes;
    CheckBox        maDialog;
    CheckBox        maBoldAppFont;
    GroupBox        maBox3;

    OKButton        maOKButton;
    CancelButton    maCancelButton;
    HelpButton      maHelpButton;

public:
                    DbgDialog();

                    DECL_LINK( ClickHdl, Button* );
    void            RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
};

DbgWindow::DbgWindow() :
    WorkWindow( NULL, WB_STDWORK ),
    maLstBox( this, WB_AUTOHSCROLL )
{
    DbgData* pData = DbgGetData();

    maLstBox.Show();
    maLstBox.SetPosPixel( Point( 0, 0 ) );

    SetOutputSizePixel( Size( 600, 480 ) );
    if ( pData->aDbgWinState[0] )
    {
        OString aState( pData->aDbgWinState );
        SetWindowState( aState );
    }

    SetText("StarView Debug Window");
    Show();
    Update();
}

bool DbgWindow::Close()
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
    return true;
}

void DbgWindow::Resize()
{
    maLstBox.SetSizePixel( GetOutputSizePixel() );
}

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

bool DbgWindow::PreNotify( NotifyEvent& rNEvt )
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
            return true;   // handled
        }
    }
    return WorkWindow::PreNotify( rNEvt );
}

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

DbgDialog::DbgDialog() :
    ModalDialog( NULL, WB_STDMODAL | WB_SYSTEMWINDOW ),
    maRes( this ),
    maDialog( this ),
    maBoldAppFont( this ),
    maBox3( this ),
    maOKButton( this, WB_DEFBUTTON ),
    maCancelButton( this ),
    maHelpButton( this )
{
    DbgData*    pData = DbgGetData();
    MapMode     aAppMap( MAP_APPFONT );
    Size        aButtonSize = LogicToPixel( Size( 60, 12 ), aAppMap );

    {
    maRes.Show();
    maRes.SetText("~Resourcen");
    if ( pData->nTestFlags & DBG_TEST_RESOURCE )
        maRes.Check( true );
    maRes.SetPosSizePixel( LogicToPixel( Point( 75, 95 ), aAppMap ),
                           aButtonSize );
    }

    {
    maDialog.Show();
    maDialog.SetText("~Dialog");
    if ( pData->nTestFlags & DBG_TEST_DIALOG )
        maDialog.Check( true );
    maDialog.SetPosSizePixel( LogicToPixel( Point( 140, 95 ), aAppMap ),
                              aButtonSize );
    }

    {
    maBoldAppFont.Show();
    maBoldAppFont.SetText("~Bold AppFont");
    if ( pData->nTestFlags & DBG_TEST_BOLDAPPFONT )
        maBoldAppFont.Check( true );
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
    maHelpButton.Show();
    maHelpButton.SetPosSizePixel( LogicToPixel( Point( 190, 260 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }

    {
    SetText("VCL Debug Options");
    SetOutputSizePixel( LogicToPixel( Size( 340, 280 ), aAppMap ) );
    }
}

IMPL_LINK( DbgDialog, ClickHdl, Button*, pButton )
{
    if ( pButton == &maOKButton )
    {
        DbgData aData;

        memcpy( &aData, DbgGetData(), sizeof( DbgData ) );
        aData.nTestFlags = 0;

        if ( maRes.IsChecked() )
            aData.nTestFlags |= DBG_TEST_RESOURCE;

        if ( maDialog.IsChecked() )
            aData.nTestFlags |= DBG_TEST_DIALOG;

        if ( maBoldAppFont.IsChecked() )
            aData.nTestFlags |= DBG_TEST_BOLDAPPFONT;

        // Daten speichern
        DbgSaveData( aData );

        DbgData* pData = DbgGetData();
        #define IMMEDIATE_FLAGS (DBG_TEST_RESOURCE | DBG_TEST_DIALOG | DBG_TEST_BOLDAPPFONT)
        pData->nTestFlags &= ~IMMEDIATE_FLAGS;
        pData->nTestFlags |= aData.nTestFlags & IMMEDIATE_FLAGS;
        if ( maBoldAppFont.GetSavedValue() != TriState(maBoldAppFont.IsChecked()) )
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
        EndDialog( RET_OK );
    }

    return 0;
}

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

void DbgInfoDialog::SetInfoText( const OUString& rStr )
{
    maListBox.SetUpdateMode( false );
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
    maListBox.SetUpdateMode( true );
}

void DbgDialogTest( Window* pWindow )
{
    bool        aAccelBuf[65536] = {false};
    sal_uInt16      nChildCount = pWindow->GetChildCount();
    Window*     pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    Window*     pChild;
    Point       aTabPos;

    if ( !pGetChild )
        return;

    boost::scoped_array<Rectangle> pRectAry((Rectangle*)new long[(sizeof(Rectangle)*nChildCount)/sizeof(long)]);
    memset( pRectAry.get(), 0, sizeof(Rectangle)*nChildCount );

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
                            SAL_WARN(
                                "vcl.app", "Double mnemonic char: " << cAccel);
                        else
                            aAccelBuf[cAccel] = true;
                    }
                }
            }

            if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
                 (pChild->GetType() == WINDOW_CHECKBOX) ||
                 (pChild->GetType() == WINDOW_TRISTATEBOX) ||
                 (pChild->GetType() == WINDOW_PUSHBUTTON) )
            {
                if ( !aText.isEmpty() && aText != "..." )
                {
                    const char* pClass;
                    if ( pChild->GetType() == WINDOW_RADIOBUTTON )
                        pClass = "RadioButton";
                    else if ( pChild->GetType() == WINDOW_CHECKBOX )
                        pClass = "CheckBox";
                    else if ( pChild->GetType() == WINDOW_TRISTATEBOX )
                        pClass = "TriStateBox";
                    else if ( pChild->GetType() == WINDOW_PUSHBUTTON )
                        pClass = "PushButton";
                    else
                        pClass = "Dontknow";
                    SAL_WARN_IF(
                        !cAccel, "vcl.app",
                        pClass << " should have a mnemonic char (~): "
                            << aErrorText);

                    // check text width
                    int aWidth=0;
                    switch( pChild->GetType() )
                    {
                        case WINDOW_RADIOBUTTON:
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
                    SAL_WARN_IF(
                        pChild->IsVisible() && pChild->GetSizePixel().Width() < aWidth,
                        "vcl.app",
                        pClass << " exceeds window width: " << aErrorText);
                }
            }

            SAL_WARN_IF(
                (pChild->GetType() == WINDOW_FIXEDLINE
                 && pChild->GetSizePixel().Width() < pChild->GetTextWidth( aText )),
                "vcl.app", "FixedLine exceeds window width: " << aErrorText);

            if ( pChild->GetType() == WINDOW_FIXEDTEXT )
            {
                SAL_WARN_IF(
                    (pChild->GetSizePixel().Height() >= pChild->GetTextHeight()*2) && !(pChild->GetStyle() & WB_WORDBREAK),
                    "vcl.app",
                    "FixedText greater than one line, but WordBreak is not set: "
                        << aErrorText);

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

                    SAL_WARN_IF(
                        pChild->GetSizePixel().Width() < aWidth && !(pChild->GetStyle() & WB_WORDBREAK),
                        "vcl.app",
                        "FixedText exceeds window width: " << aErrorText);
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
                        SAL_WARN_IF(
                            !cAccel, "vcl.app",
                            "Labels befor Fields (Edit,ListBox,...) should have a mnemonic char (~): "
                                << aErrorText);
                        SAL_WARN_IF(
                            !pTempChild->IsEnabled() && pChild->IsEnabled(),
                            "vcl.app",
                            "Labels befor Fields (Edit,ListBox,...) should be disabled, when the field is disabled: "
                                << aErrorText);
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
                SAL_WARN_IF(
                    bMaxWarning, "vcl.app",
                    "No Max-Value is set: " << aErrorText);

                if ( (pChild->GetType() == WINDOW_RADIOBUTTON) ||
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
                         (pChild->GetType() == WINDOW_RADIOBUTTON) )
                    {
                        SAL_WARN_IF(
                            (aNewPos.X() <= aTabPos.X()) && (aNewPos.Y() <= aTabPos.Y()),
                            "vcl.app",
                            "Possible wrong childorder for dialogcontrol: "
                                << aErrorText);
                        aTabPos = aNewPos;
                    }

                    for ( sal_uInt16 j = 0; j < i; j++ )
                    {
                        SAL_WARN_IF(
                            ((pRectAry[j].Right() != 0) || (pRectAry[j].Bottom() != 0)) && aChildRect.IsOver( pRectAry[j] ),
                            "vcl.app",
                            "Window overlaps with sibling window: "
                                << aErrorText);
                    }
                    pRectAry[i] = aChildRect;
                }
            }
        }

        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
        i++;
    }
}

void ImplDbgTestSolarMutex()
{
    assert(ImplGetSVData()->mpDefInst->CheckYieldMutex());
}

void DbgGUIInitSolarMutexCheck()
{
    DbgSetTestSolarMutex( ImplDbgTestSolarMutex );
}

void DbgGUIDeInitSolarMutexCheck()
{
    DbgSetTestSolarMutex( NULL );
}

void DbgGUIStart()
{
    DbgData* pData = DbgGetData();

    if ( pData )
    {
        boost::scoped_ptr<DbgDialog> pDialog(new DbgDialog);
        // we switch off dialog tests for the debug dialog
        sal_uLong nOldFlags = pData->nTestFlags;
        pData->nTestFlags &= ~DBG_TEST_DIALOG;
        if ( !pDialog->Execute() )
            pData->nTestFlags |= (nOldFlags & DBG_TEST_DIALOG);
    }
    else
    {
        ErrorBox( 0, WB_OK, OUString("TOOLS Library has no Debug-Routines") ).Execute();
    }
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
