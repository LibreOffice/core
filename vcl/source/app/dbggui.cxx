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
#include "vcl/layout.hxx"
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
"FixedTexts, CheckBoxes, TriStateBoxes and RadioButtons are equipped with "
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
"Output is suppressed.\n",
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
"Therefore the message should also be directed to a file/debugger in case of "
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

class DbgInfoDialog : public ModalDialog
{
private:
    VclPtr<ListBox>   maListBox;
    VclPtr<OKButton>  maOKButton;
    bool              mbHelpText;

public:
                    DbgInfoDialog( vcl::Window* pParent, bool bHelpText = false );

    void            SetInfoText( const OUString& rStr );
private:
    virtual void    dispose() SAL_OVERRIDE;
    virtual         ~DbgInfoDialog() { disposeOnce(); }
};

class DbgDialog : public ModalDialog
{
private:
    VclPtr<CheckBox>     maRes;
    VclPtr<CheckBox>     maDialog;
    VclPtr<CheckBox>     maBoldAppFont;
    VclPtr<GroupBox>     maBox3;

    VclPtr<OKButton>     maOKButton;
    VclPtr<CancelButton> maCancelButton;
    VclPtr<HelpButton>   maHelpButton;

public:
                    DbgDialog();

                    DECL_LINK( ClickHdl, Button* );
    void            RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
private:
    virtual void    dispose() SAL_OVERRIDE;
    virtual         ~DbgDialog() { disposeOnce(); }
};

DbgDialog::DbgDialog() :
    ModalDialog( NULL, WB_STDMODAL | WB_SYSTEMWINDOW ),
    maRes(VclPtr<CheckBox>::Create(this)),
    maDialog(VclPtr<CheckBox>::Create(this)),
    maBoldAppFont(VclPtr<CheckBox>::Create(this)),
    maBox3(VclPtr<GroupBox>::Create(this)),
    maOKButton(VclPtr<OKButton>::Create(this, WB_DEFBUTTON)),
    maCancelButton(VclPtr<CancelButton>::Create(this)),
    maHelpButton(VclPtr<HelpButton>::Create(this))
{
    DbgData*    pData = DbgGetData();
    MapMode     aAppMap( MAP_APPFONT );
    Size        aButtonSize = LogicToPixel( Size( 60, 12 ), aAppMap );

    {
    maRes->Show();
    maRes->SetText("~Resourcen");
    if ( pData->nTestFlags & DBG_TEST_RESOURCE )
        maRes->Check( true );
    maRes->SetPosSizePixel( LogicToPixel( Point( 75, 95 ), aAppMap ),
                           aButtonSize );
    }

    {
    maDialog->Show();
    maDialog->SetText("~Dialog");
    if ( pData->nTestFlags & DBG_TEST_DIALOG )
        maDialog->Check( true );
    maDialog->SetPosSizePixel( LogicToPixel( Point( 140, 95 ), aAppMap ),
                              aButtonSize );
    }

    {
    maBoldAppFont->Show();
    maBoldAppFont->SetText("~Bold AppFont");
    if ( pData->nTestFlags & DBG_TEST_BOLDAPPFONT )
        maBoldAppFont->Check( true );
    maBoldAppFont->SetPosSizePixel( LogicToPixel( Point( 205, 95 ), aAppMap ),
                                   aButtonSize );
    maBoldAppFont->SaveValue();
    }

    {
    maBox3->Show();
    maBox3->SetText("Test Options");
    maBox3->SetPosSizePixel( LogicToPixel( Point( 5, 85 ), aAppMap ),
                            LogicToPixel( Size( 330, 30 ), aAppMap ) );
    }

    {
    maOKButton->Show();
    maOKButton->SetClickHdl( LINK( this, DbgDialog, ClickHdl ) );
    maOKButton->SetPosSizePixel( LogicToPixel( Point( 10, 260 ), aAppMap ),
                                LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maCancelButton->Show();
    maCancelButton->SetPosSizePixel( LogicToPixel( Point( 70, 260 ), aAppMap ),
                                    LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }
    {
    maHelpButton->Show();
    maHelpButton->SetPosSizePixel( LogicToPixel( Point( 190, 260 ), aAppMap ),
                                  LogicToPixel( Size( 50, 15 ), aAppMap ) );
    }

    {
    SetText("VCL Debug Options");
    SetOutputSizePixel( LogicToPixel( Size( 340, 280 ), aAppMap ) );
    }
}

IMPL_LINK( DbgDialog, ClickHdl, Button*, pButton )
{
    if ( pButton == maOKButton )
    {
        DbgData aData;

        memcpy( &aData, DbgGetData(), sizeof( DbgData ) );
        aData.nTestFlags = 0;

        if ( maRes->IsChecked() )
            aData.nTestFlags |= DBG_TEST_RESOURCE;

        if ( maDialog->IsChecked() )
            aData.nTestFlags |= DBG_TEST_DIALOG;

        if ( maBoldAppFont->IsChecked() )
            aData.nTestFlags |= DBG_TEST_BOLDAPPFONT;

        // Daten speichern
        DbgSaveData( aData );

        DbgData* pData = DbgGetData();
        #define IMMEDIATE_FLAGS (DBG_TEST_RESOURCE | DBG_TEST_DIALOG | DBG_TEST_BOLDAPPFONT)
        pData->nTestFlags &= ~IMMEDIATE_FLAGS;
        pData->nTestFlags |= aData.nTestFlags & IMMEDIATE_FLAGS;
        if ( maBoldAppFont->IsValueChangedFromSaved() )
        {
            AllSettings aSettings = Application::GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            vcl::Font aFont = aStyleSettings.GetAppFont();
            if ( maBoldAppFont->IsChecked() )
                aFont.SetWeight( WEIGHT_BOLD );
            else
                aFont.SetWeight( WEIGHT_NORMAL );
            aStyleSettings.SetAppFont( aFont );
            aSettings.SetStyleSettings( aStyleSettings );
            Application::SetSettings( aSettings );
        }
        if( (aData.nTestFlags & ~IMMEDIATE_FLAGS) != (pData->nTestFlags & ~IMMEDIATE_FLAGS) )
        {
            ScopedVclPtrInstance<MessageDialog> aBox(this, OUString(
                "Some of the changed settings will only be active after "
                "restarting the process"), VCL_MESSAGE_INFO);
            aBox->Execute();
        }
        EndDialog( RET_OK );
    }

    return 0;
}

void DbgDialog::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HelpEventMode::CONTEXT )
    {
        ScopedVclPtrInstance< DbgInfoDialog > aInfoDialog(  this, true  );
        OUString aHelpText;
        const sal_Char** pHelpStrs = pDbgHelpText;
        while ( *pHelpStrs )
        {
            aHelpText += OUString::createFromAscii(*pHelpStrs);
            pHelpStrs++;
        }
        aInfoDialog->SetText( "Debug Hilfe" );
        aInfoDialog->SetInfoText( aHelpText );
        aInfoDialog->Execute();
    }
}

void DbgDialog::dispose()
{
    maRes.disposeAndClear();
    maDialog.disposeAndClear();
    maBoldAppFont.disposeAndClear();
    maBox3.disposeAndClear();
    maOKButton.disposeAndClear();
    maCancelButton.disposeAndClear();
    maHelpButton.disposeAndClear();
    ModalDialog::dispose();
}

DbgInfoDialog::DbgInfoDialog( vcl::Window* pParent, bool bHelpText ) :
    ModalDialog( pParent, WB_STDMODAL ),
    maListBox(VclPtr<ListBox>::Create( this, WB_BORDER | WB_AUTOHSCROLL )),
    maOKButton(VclPtr<OKButton>::Create(this, WB_DEFBUTTON))
{
    mbHelpText = bHelpText;

    if ( !bHelpText )
    {
        vcl::Font aFont = GetDefaultFont( DefaultFontType::FIXED, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::NONE );
        aFont.SetHeight( 8 );
        aFont.SetPitch( PITCH_FIXED );
        maListBox->SetControlFont( aFont );
    }
    maListBox->SetPosSizePixel( Point( 5, 5 ), Size( 630, 380 ) );
    maListBox->Show();

    maOKButton->SetPosSizePixel( Point( 290, 390 ), Size( 60, 25 ) );
    maOKButton->Show();

    SetOutputSizePixel( Size( 640, 420 ) );
}

void DbgInfoDialog::SetInfoText( const OUString& rStr )
{
    maListBox->SetUpdateMode( false );
    maListBox->Clear();
    OUString aStr = convertLineEnd(rStr, LINEEND_LF);
    sal_Int32 nStrIndex = 0;
    sal_Int32 nFoundIndex;
    do
    {
        nFoundIndex = aStr.indexOf( '\n', nStrIndex );
        OUString aTextParagraph = aStr.copy( nStrIndex, nFoundIndex-nStrIndex );
        if ( mbHelpText )
        {
            long    nMaxWidth = maListBox->GetOutputSizePixel().Width()-30;
            sal_Int32  nLastIndex = 0;
            sal_Int32  nIndex = aTextParagraph.indexOf( ' ' );
            while ( nIndex != -1 )
            {
                if ( maListBox->GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
                {
                    if ( !nLastIndex )
                        nLastIndex = nIndex+1;
                    OUString aTempStr = aTextParagraph.copy( 0, nLastIndex );
                    aTextParagraph = aTextParagraph.replaceAt( 0, nLastIndex, "" );
                    maListBox->InsertEntry( aTempStr );
                    nLastIndex = 0;
                }
                else
                    nLastIndex = nIndex+1;
                nIndex = aTextParagraph.indexOf( ' ', nLastIndex );
            }

            if ( maListBox->GetTextWidth( aTextParagraph, 0, nIndex ) > nMaxWidth )
            {
                if ( !nLastIndex )
                    nLastIndex = nIndex+1;
                OUString aTempStr = aTextParagraph.copy( 0, nLastIndex );
                aTextParagraph = aTextParagraph.replaceAt( 0, nLastIndex, "" );
                maListBox->InsertEntry( aTempStr );
            }
        }
        maListBox->InsertEntry( aTextParagraph );
        nStrIndex = nFoundIndex+1;
    }
    while ( nFoundIndex != -1 );
    maListBox->SetUpdateMode( true );
}

void DbgInfoDialog::dispose()
{
    maListBox.disposeAndClear();
    maOKButton.disposeAndClear();
    ModalDialog::dispose();
}

void DbgDialogTest( vcl::Window* pWindow )
{
    bool        aAccelBuf[65536] = {false};
    sal_uInt16      nChildCount = pWindow->GetChildCount();
    vcl::Window*     pGetChild = pWindow->GetWindow( GetWindowType::FirstChild );
    vcl::Window*     pChild;
    Point       aTabPos;

    if ( !pGetChild )
        return;

    boost::scoped_array<Rectangle> pRectAry(reinterpret_cast<Rectangle*>(new long[(sizeof(Rectangle)*nChildCount)/sizeof(long)]));
    memset( pRectAry.get(), 0, sizeof(Rectangle)*nChildCount );

    if ( pWindow->IsDialog() )
    {
        bool    bOKCancelButton = false;
        bool    bDefPushButton = false;
        bool    bButton = false;
        pGetChild = pWindow->GetWindow( GetWindowType::FirstChild );
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

            pGetChild = pGetChild->GetWindow( GetWindowType::Next );
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
    pGetChild = pWindow->GetWindow( GetWindowType::FirstChild );
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
                            aWidth = static_cast<RadioButton*>(pChild)->CalcMinimumSize(0).Width();
                            break;
                        case WINDOW_CHECKBOX:
                        case WINDOW_TRISTATEBOX:
                            aWidth = static_cast<CheckBox*>(pChild)->CalcMinimumSize(0).Width();
                            break;
                        case WINDOW_PUSHBUTTON:
                            aWidth = static_cast<PushButton*>(pChild)->CalcMinimumSize(0).Width();
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
                    vcl::Window* pTempChild = pGetChild->GetWindow( GetWindowType::Next )->ImplGetWindow();
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
                            "Labels before Fields (Edit,ListBox,...) should have a mnemonic char (~): "
                                << aErrorText);
                        SAL_WARN_IF(
                            !pTempChild->IsEnabled() && pChild->IsEnabled(),
                            "vcl.app",
                            "Labels before Fields (Edit,ListBox,...) should be disabled, when the field is disabled: "
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
                    NumericField* pField = static_cast<NumericField*>(pChild);
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_METRICFIELD )
                {
                    MetricField* pField = static_cast<MetricField*>(pChild);
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYFIELD )
                {
                    CurrencyField* pField = static_cast<CurrencyField*>(pChild);
                    if ( pField->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_TIMEFIELD )
                {
                    TimeField* pField = static_cast<TimeField*>(pChild);
                    if ( pField->GetMax() == tools::Time( 23, 59, 59, 99 ) )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_DATEFIELD )
                {
                    DateField* pField = static_cast<DateField*>(pChild);
                    if ( pField->GetMax() == Date( 31, 12, 9999 ) )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_NUMERICBOX )
                {
                    NumericBox* pBox = static_cast<NumericBox*>(pChild);
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_METRICBOX )
                {
                    MetricBox* pBox = static_cast<MetricBox*>(pChild);
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_CURRENCYBOX )
                {
                    CurrencyBox* pBox = static_cast<CurrencyBox*>(pChild);
                    if ( pBox->GetMax() == LONG_MAX )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_TIMEBOX )
                {
                    TimeBox* pBox = static_cast<TimeBox*>(pChild);
                    if ( pBox->GetMax() == tools::Time( 23, 59, 59, 99 ) )
                        bMaxWarning = true;
                }
                else if ( pChild->GetType() == WINDOW_DATEBOX )
                {
                    DateBox* pBox = static_cast<DateBox*>(pChild);
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

        pGetChild = pGetChild->GetWindow( GetWindowType::Next );
        i++;
    }
}

void ImplDbgTestSolarMutex()
{
    assert(ImplGetSVData()->mpDefInst->CheckYieldMutex() && "SolarMutex not locked");
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
        ScopedVclPtrInstance< DbgDialog > xDialog;
        // we switch off dialog tests for the debug dialog
        sal_uLong nOldFlags = pData->nTestFlags;
        pData->nTestFlags &= ~DBG_TEST_DIALOG;
        if (!xDialog->Execute())
            pData->nTestFlags |= (nOldFlags & DBG_TEST_DIALOG);
    }
    else
    {
        MessageDialog(0, OUString("TOOLS Library has no Debug-Routines")).Execute();
    }
}

#endif // DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
