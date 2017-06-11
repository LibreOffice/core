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

#include <strings.hrc>
#include <bitmaps.hlst>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

#include <vcl/textrectinfo.hxx>
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
    if (pSVData->maWinData.maMsgBoxImgList.empty())
    {
        pSVData->maWinData.maMsgBoxImgList.push_back(Image(BitmapEx(SV_RESID_BITMAP_ERRORBOX)));
        pSVData->maWinData.maMsgBoxImgList.push_back(Image(BitmapEx(SV_RESID_BITMAP_QUERYBOX)));
        pSVData->maWinData.maMsgBoxImgList.push_back(Image(BitmapEx(SV_RESID_BITMAP_WARNINGBOX)));
        pSVData->maWinData.maMsgBoxImgList.push_back(Image(BitmapEx(SV_RESID_BITMAP_INFOBOX)));
    }
}

void MessBox::ImplInitButtons()
{
    WinBits nStyle = GetStyle();
    ButtonDialogFlags nOKFlags = ButtonDialogFlags::OK;
    ButtonDialogFlags nCancelFlags = ButtonDialogFlags::Cancel;
    ButtonDialogFlags nRetryFlags = ButtonDialogFlags::NONE;
    ButtonDialogFlags nYesFlags = ButtonDialogFlags::NONE;
    ButtonDialogFlags nNoFlags = ButtonDialogFlags::NONE;

    if ( nStyle & WB_OK_CANCEL )
    {
        if ( nStyle & WB_DEF_CANCEL )
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // WB_DEF_OK
            nOKFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::OK, RET_OK, nOKFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_YES_NO )
    {
        if ( nStyle & WB_DEF_YES )
            nYesFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // WB_DEF_NO
            nNoFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        nNoFlags |= ButtonDialogFlags::Cancel;

        AddButton( StandardButtonType::Yes, RET_YES, nYesFlags );
        AddButton( StandardButtonType::No, RET_NO, nNoFlags );
    }
    else if ( nStyle & WB_YES_NO_CANCEL )
    {
        if ( nStyle & WB_DEF_YES )
            nYesFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( nStyle & WB_DEF_NO )
            nNoFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Yes, RET_YES, nYesFlags );
        AddButton( StandardButtonType::No, RET_NO, nNoFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_RETRY_CANCEL )
    {
        if ( nStyle & WB_DEF_CANCEL )
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // WB_DEF_RETRY
            nRetryFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Retry, RET_RETRY, nRetryFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_ABORT_RETRY_IGNORE )
    {
        ButtonDialogFlags nAbortFlags = ButtonDialogFlags::NONE;
        ButtonDialogFlags nIgnoreFlags = ButtonDialogFlags::NONE;

        if ( nStyle & WB_DEF_CANCEL )
            nAbortFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( nStyle & WB_DEF_RETRY )
            nRetryFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( nStyle & WB_DEF_IGNORE )
            nIgnoreFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Abort, RET_CANCEL, nAbortFlags );
        AddButton( StandardButtonType::Retry, RET_RETRY, nRetryFlags );
        AddButton( StandardButtonType::Ignore, RET_IGNORE, nIgnoreFlags );
    }
    else if ( nStyle & WB_OK )
    {
        nOKFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::OK, RET_OK, nOKFlags );
    }
}

MessBox::MessBox( vcl::Window* pParent, WinBits nStyle,
                  const OUString& rTitle, const OUString& rMessage ) :
    ButtonDialog( WindowType::MESSBOX ),
    mbHelpBtn( false ),
    mbCheck( false ),
    maMessText( rMessage )
{
    ImplInit( pParent, nStyle | WB_MOVEABLE | WB_HORZ | WB_CENTER );
    ImplInitButtons();

    if ( !rTitle.isEmpty() )
        SetText( rTitle );
}

MessBox::~MessBox()
{
    disposeOnce();
}

void MessBox::dispose()
{
    mpVCLMultiLineEdit.disposeAndClear();
    mpFixedImage.disposeAndClear();
    mpCheckBox.disposeAndClear();
    ButtonDialog::dispose();
}

void MessBox::ImplPosControls()
{
    if ( !GetHelpId().isEmpty() )
    {
        if ( !mbHelpBtn )
        {
            AddButton( StandardButtonType::Help, RET_HELP, ButtonDialogFlags::Help, 3 );
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
    tools::Rectangle       aRect( 0, 0, 30000, 30000 );
    tools::Rectangle       aFormatRect;
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
    DrawTextFlags   nTextStyle = DrawTextFlags::MultiLine | DrawTextFlags::Top | DrawTextFlags::Left;

    mpVCLMultiLineEdit.disposeAndClear();
    mpFixedImage.disposeAndClear();
    if ( mpCheckBox )
    {
        mbCheck = mpCheckBox->IsChecked();
        mpCheckBox.disposeAndClear();
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
        mpFixedImage = VclPtr<FixedImage>::Create( this );
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
    nTextStyle |= DrawTextFlags::WordBreak;

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

        mpCheckBox = VclPtr<CheckBox>::Create( this );
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

    mpVCLMultiLineEdit = VclPtr<VclMultiLineEdit>::Create( this, nWinStyle );
    mpVCLMultiLineEdit->SetText( aMessText );
    mpVCLMultiLineEdit->SetPosSizePixel( aTextPos, aMEditSize );
    mpVCLMultiLineEdit->Show();
    mpVCLMultiLineEdit->SetPaintTransparent(true);
    mpVCLMultiLineEdit->EnableCursor(false);
    SetPageSizePixel( aPageSize );
}

void MessBox::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
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

InfoBox::InfoBox( vcl::Window* pParent, const OUString& rMessage ) :
    MessBox( pParent, WB_OK | WB_DEF_OK, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( InfoBox::GetStandardImage() );
}

InfoBox::InfoBox( vcl::Window* pParent, WinBits nStyle, const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( InfoBox::GetStandardImage() );
}

Image InfoBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[3];
}

OUString InfoBox::GetStandardText()
{
    return VclResId(SV_MSGBOX_INFO);
}

WarningBox::WarningBox( vcl::Window* pParent, WinBits nStyle,
                        const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( WarningBox::GetStandardImage() );
}

void WarningBox::SetDefaultCheckBoxText()
{
    maCheckBoxText = VclResId(SV_STDTEXT_DONTWARNAGAIN);
}

Image WarningBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[2];
}

OUString WarningBox::GetStandardText()
{
    return VclResId(SV_MSGBOX_WARNING);
}

ErrorBox::ErrorBox( vcl::Window* pParent, const OUString& rMessage ) :
    MessBox( pParent, WB_OK | WB_DEF_OK, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( ErrorBox::GetStandardImage() );
}

ErrorBox::ErrorBox( vcl::Window* pParent, WinBits nStyle,
                    const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( ErrorBox::GetStandardImage() );
}

Image ErrorBox::GetStandardImage()
{
    try
    {
        ImplInitMsgBoxImageList();
    }
    catch (const css::uno::Exception &)
    {
        // During early bootstrap we can have no initialized
        // ucb and hence no ability to get this image, so nop.
        return Image();
    }
    return ImplGetSVData()->maWinData.maMsgBoxImgList[0];
}

OUString ErrorBox::GetStandardText()
{
    return VclResId(SV_MSGBOX_ERROR);
}

QueryBox::QueryBox( vcl::Window* pParent, WinBits nStyle, const OUString& rMessage ) :
    MessBox( pParent, nStyle, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( QueryBox::GetStandardImage() );
}

void QueryBox::SetDefaultCheckBoxText()
{
    maCheckBoxText = VclResId(SV_STDTEXT_DONTASKAGAIN);
}

Image QueryBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[1];
}

OUString QueryBox::GetStandardText()
{
    return VclResId(SV_MSGBOX_QUERY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
