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

#include <tools/rc.h>

#include <svids.hrc>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>

static void ImplInitMsgBoxImageList()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maWinData.mpMsgBoxImgList )
    {
        ResMgr* pResMgr = ImplGetResMgr();
        pSVData->maWinData.mpMsgBoxImgList = new ImageList(4);
        if( pResMgr )
        {
            Color aNonAlphaMask( 0xC0, 0xC0, 0xC0 );
            pSVData->maWinData.mpMsgBoxImgList->InsertFromHorizontalBitmap
                ( ResId( SV_RESID_BITMAP_MSGBOX, *pResMgr ), 4, &aNonAlphaMask );
        }
    }
}

void MessBox::ImplInitMessBoxData()
{
    mpVCLMultiLineEdit  = NULL;
    mpFixedImage        = NULL;
    mbHelpBtn           = false;
    mpCheckBox          = NULL;
    mbCheck             = false;
}

void MessBox::ImplInitButtons()
{
    WinBits nStyle = GetStyle();
    sal_uInt16  nOKFlags = BUTTONDIALOG_OKBUTTON;
    sal_uInt16  nCancelFlags = BUTTONDIALOG_CANCELBUTTON;
    sal_uInt16  nRetryFlags = 0;
    sal_uInt16  nYesFlags = 0;
    sal_uInt16  nNoFlags = 0;

    if ( nStyle & WB_OK_CANCEL )
    {
        if ( nStyle & WB_DEF_CANCEL )
            nCancelFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else // WB_DEF_OK
            nOKFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_OK, RET_OK, nOKFlags );
        AddButton( BUTTON_CANCEL, RET_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_YES_NO )
    {
        if ( nStyle & WB_DEF_YES )
            nYesFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else // WB_DEF_NO
            nNoFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        nNoFlags |= BUTTONDIALOG_CANCELBUTTON;

        AddButton( BUTTON_YES, RET_YES, nYesFlags );
        AddButton( BUTTON_NO, RET_NO, nNoFlags );
    }
    else if ( nStyle & WB_YES_NO_CANCEL )
    {
        if ( nStyle & WB_DEF_YES )
            nYesFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else if ( nStyle & WB_DEF_NO )
            nNoFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else
            nCancelFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_YES, RET_YES, nYesFlags );
        AddButton( BUTTON_NO, RET_NO, nNoFlags );
        AddButton( BUTTON_CANCEL, RET_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_RETRY_CANCEL )
    {
        if ( nStyle & WB_DEF_CANCEL )
            nCancelFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else // WB_DEF_RETRY
            nRetryFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_RETRY, RET_RETRY, nRetryFlags );
        AddButton( BUTTON_CANCEL, RET_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_ABORT_RETRY_IGNORE )
    {
        sal_uInt16 nAbortFlags = 0;
        sal_uInt16 nIgnoreFlags = 0;

        if ( nStyle & WB_DEF_CANCEL )
            nAbortFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else if ( nStyle & WB_DEF_RETRY )
            nRetryFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else if ( nStyle & WB_DEF_IGNORE )
            nIgnoreFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_ABORT, RET_CANCEL, nAbortFlags );
        AddButton( BUTTON_RETRY, RET_RETRY, nRetryFlags );
        AddButton( BUTTON_IGNORE, RET_IGNORE, nIgnoreFlags );
    }
    else if ( nStyle & WB_OK )
    {
        nOKFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_OK, RET_OK, nOKFlags );
    }
}

MessBox::MessBox( vcl::Window* pParent, WinBits nStyle,
                  const OUString& rTitle, const OUString& rMessage ) :
    ButtonDialog( WINDOW_MESSBOX ),
    maMessText( rMessage )
{
    ImplInitMessBoxData();
    ImplInit( pParent, nStyle | WB_MOVEABLE | WB_HORZ | WB_CENTER );
    ImplInitButtons();

    if ( !rTitle.isEmpty() )
        SetText( rTitle );
}

MessBox::~MessBox()
{
    delete mpVCLMultiLineEdit;
    delete mpFixedImage;
    delete mpCheckBox;
}

void MessBox::ImplPosControls()
{
    if ( !GetHelpId().isEmpty() )
    {
        if ( !mbHelpBtn )
        {
            AddButton( BUTTON_HELP, RET_HELP, BUTTONDIALOG_HELPBUTTON, 3 );
            mbHelpBtn = true;
        }
    }
    else
    {
        if ( mbHelpBtn )
        {
            RemoveButton( RET_HELP );
            mbHelpBtn = false;
        }
    }

    TextRectInfo    aTextInfo;
    Rectangle       aRect( 0, 0, 30000, 30000 );
    Rectangle       aFormatRect;
    Point           aTextPos( IMPL_DIALOG_OFFSET, IMPL_DIALOG_OFFSET+IMPL_MSGBOX_OFFSET_EXTRA_Y );
    Size            aImageSize;
    Size            aPageSize;
    Size            aMEditSize;
    long            nTitleWidth;
    long            nButtonSize = ImplGetButtonSize();
    long            nMaxWidth = GetDesktopRectPixel().GetWidth()-8;
    long            nMaxLineWidth;
    long            nWidth;
    WinBits         nWinStyle = WB_LEFT | WB_NOLABEL;
    sal_uInt16          nTextStyle = TEXT_DRAW_MULTILINE | TEXT_DRAW_TOP | TEXT_DRAW_LEFT;

    delete mpVCLMultiLineEdit;
    if ( mpFixedImage )
    {
        delete mpFixedImage;
        mpFixedImage = NULL;
    }
    if ( mpCheckBox )
    {
        mbCheck = mpCheckBox->IsChecked();
        delete mpCheckBox;
        mpCheckBox = NULL;
    }

    // Clean up message text with tabs
    OUString aMessText(maMessText.replaceAll("\t", "    "));

    //If window too small, we make dialog box be wider
    if ( mpWindowImpl->mbFrame )
    {
        nMaxWidth = 630 * GetDPIScaleFactor();
    }
    else if ( nMaxWidth < 120 )
        nMaxWidth = 120;

    nMaxWidth -= mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder+4;

    // MessagBox should be at least as wide as to see the title
    // Extra-Width for Close button, because Close button is set after this call
    nTitleWidth = CalcTitleWidth();
    nTitleWidth += mpWindowImpl->mnTopBorder;

    nMaxWidth -= (IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_X*2);

    // for an image, get its size, create a suitable control and position it
    aImageSize = maImage.GetSizePixel();
    if ( aImageSize.Width() )
    {
        aImageSize.Width()  += 4;
        aImageSize.Height() += 4;
        aTextPos.X() += aImageSize.Width()+IMPL_SEP_MSGBOX_IMAGE;
        mpFixedImage = new FixedImage( this );
        mpFixedImage->SetPosSizePixel( Point( IMPL_DIALOG_OFFSET-2+IMPL_MSGBOX_OFFSET_EXTRA_X,
                                              IMPL_DIALOG_OFFSET-2+IMPL_MSGBOX_OFFSET_EXTRA_Y ),
                                       aImageSize );
        mpFixedImage->SetImage( maImage );
        mpFixedImage->Show();
        nMaxWidth -= aImageSize.Width()+IMPL_SEP_MSGBOX_IMAGE;
    }
    else
        aTextPos.X() += IMPL_MSGBOX_OFFSET_EXTRA_X;

    // Determine maximum line length without wordbreak
    aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    nMaxLineWidth = aFormatRect.GetWidth();
    nTextStyle |= TEXT_DRAW_WORDBREAK;

    // Determine the width for text formatting
    if ( nMaxLineWidth > 450 )
        nWidth = 450;
    else if ( nMaxLineWidth > 300 )
        nWidth = nMaxLineWidth+5;
    else
        nWidth = 300;

    nWidth *= GetDPIScaleFactor();

    if ( nButtonSize > nWidth )
        nWidth = nButtonSize-(aTextPos.X()-IMPL_DIALOG_OFFSET);
    if ( nWidth > nMaxWidth )
        nWidth = nMaxWidth;

    aRect.Right() = nWidth;
    aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    if ( aTextInfo.GetMaxLineWidth() > nWidth )
    {
        nWidth = aTextInfo.GetMaxLineWidth()+8;
        aRect.Right() = nWidth;
        aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    }

    // get Style for VCLMultiLineEdit
    aMEditSize.Width()  = aTextInfo.GetMaxLineWidth()+1;
    aMEditSize.Height() = aFormatRect.GetHeight();
    aPageSize.Width()   = aImageSize.Width();
    if ( aMEditSize.Height() < aImageSize.Height() )
    {
        nWinStyle |= WB_VCENTER;
        aPageSize.Height()  = aImageSize.Height();
        aMEditSize.Height() = aImageSize.Height();
    }
    else
    {
        nWinStyle |= WB_TOP;
        aPageSize.Height()  = aMEditSize.Height();
    }
    if ( aImageSize.Width() )
        aPageSize.Width() += IMPL_SEP_MSGBOX_IMAGE;
    aPageSize.Width()  += (IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_X*2);
    aPageSize.Width()  += aMEditSize.Width()+1;
    aPageSize.Height() += (IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_Y*2);

    if ( aPageSize.Width() < IMPL_MINSIZE_MSGBOX_WIDTH )
        aPageSize.Width() = IMPL_MINSIZE_MSGBOX_WIDTH;
    if ( aPageSize.Width() < nTitleWidth )
        aPageSize.Width() = nTitleWidth;

    if ( !maCheckBoxText.isEmpty() )
    {
        Size aMinCheckboxSize ( aMEditSize );
        if ( aPageSize.Width() < IMPL_MINSIZE_MSGBOX_WIDTH+80 )
        {
            aPageSize.Width() = IMPL_MINSIZE_MSGBOX_WIDTH+80;
            aMinCheckboxSize.Width() += 80;
        }

        // #104492# auto mnemonics for CJK strings may increase the length, so measure the
        // checkbox length including a temporary mnemonic, the correct auto mnemonic will be
        // generated later in the dialog (see init_show)

        OUString aMnemonicString( maCheckBoxText );
        if( GetSettings().GetStyleSettings().GetAutoMnemonic() )
        {
            if( aMnemonicString == GetNonMnemonicString( maCheckBoxText ) )
            {
                // no mnemonic found -> create one
                MnemonicGenerator aMnemonicGenerator;
                aMnemonicString = aMnemonicGenerator.CreateMnemonic( aMnemonicString );
            }
        }

        mpCheckBox = new CheckBox( this );
        mpCheckBox->Check( mbCheck );
        mpCheckBox->SetText( aMnemonicString );
        mpCheckBox->SetStyle( mpCheckBox->GetStyle() | WB_WORDBREAK );
        mpCheckBox->SetHelpId( GetHelpId() );   // DR: Check box and dialog have same HID

        // align checkbox with message text
        Size aSize = mpCheckBox->CalcMinimumSize( aMinCheckboxSize.Width() );

        // now set the original non-mnemonic string
        mpCheckBox->SetText( maCheckBoxText );

        Point aPos( aTextPos );
        aPos.Y() += aMEditSize.Height() + (IMPL_DIALOG_OFFSET)+(IMPL_MSGBOX_OFFSET_EXTRA_Y*2);

        // increase messagebox
        aPageSize.Height() += aSize.Height() + (IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_Y*2);

        mpCheckBox->SetPosSizePixel( aPos, aSize );
        mpCheckBox->Show();
    }

    mpVCLMultiLineEdit = new VclMultiLineEdit( this, nWinStyle );
    mpVCLMultiLineEdit->SetText( aMessText );
    mpVCLMultiLineEdit->SetPosSizePixel( aTextPos, aMEditSize );
    mpVCLMultiLineEdit->Show();
    mpVCLMultiLineEdit->SetPaintTransparent(true);
    mpVCLMultiLineEdit->EnableCursor(false);
    SetPageSizePixel( aPageSize );
}

void MessBox::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::INITSHOW )
    {
        ImplPosControls();
    }
    ButtonDialog::StateChanged( nType );
}

bool MessBox::GetCheckBoxState() const
{
    return mpCheckBox ? mpCheckBox->IsChecked() : mbCheck;
}

void MessBox::SetCheckBoxState( bool bCheck )
{
    if( mpCheckBox ) mpCheckBox->Check( bCheck );
    mbCheck = bCheck;
}

Size MessBox::GetOptimalSize() const
{
    // FIXME: base me on the font size ?
    return Size( 250, 100 );
}

void InfoBox::ImplInitInfoBoxData()
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( Application::GetDisplayName() );

    SetImage( InfoBox::GetStandardImage() );
}

InfoBox::InfoBox( vcl::Window* pParent, const OUString& rMessage ) :
    MessBox( pParent, WB_OK | WB_DEF_OK, OUString(), rMessage )
{
    ImplInitInfoBoxData();
}

InfoBox::InfoBox( vcl::Window* pParent, WinBits nStyle, const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    ImplInitInfoBoxData();
}

Image InfoBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 4 );
}

void WarningBox::ImplInitWarningBoxData()
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( Application::GetDisplayName() );

    SetImage( WarningBox::GetStandardImage() );
}

WarningBox::WarningBox( vcl::Window* pParent, WinBits nStyle,
                        const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    ImplInitWarningBoxData();
}

void WarningBox::SetDefaultCheckBoxText()
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
        maCheckBoxText = ResId(SV_STDTEXT_DONTWARNAGAIN, *pResMgr).toString();
}

Image WarningBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 3 );
}

void ErrorBox::ImplInitErrorBoxData()
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( Application::GetDisplayName() );

    SetImage( ErrorBox::GetStandardImage() );
}

ErrorBox::ErrorBox( vcl::Window* pParent, WinBits nStyle,
                    const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    ImplInitErrorBoxData();
}

Image ErrorBox::GetStandardImage()
{
    try
    {
        ImplInitMsgBoxImageList();
    }
    catch (const ::com::sun::star::uno::Exception &)
    {
        // During early bootstrap we can have no initialized
        // ucb and hence no ability to get this image, so nop.
        return Image();
    }
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 1 );
}

void QueryBox::ImplInitQueryBoxData()
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( Application::GetDisplayName() );

    SetImage( QueryBox::GetStandardImage() );
}

QueryBox::QueryBox( vcl::Window* pParent, WinBits nStyle, const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    ImplInitQueryBoxData();
}

void QueryBox::SetDefaultCheckBoxText()
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
        maCheckBoxText = ResId(SV_STDTEXT_DONTASKAGAIN, *pResMgr).toString();
}

Image QueryBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 2 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
