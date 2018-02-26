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
        pSVData->maWinData.maMsgBoxImgList.emplace_back(BitmapEx(SV_RESID_BITMAP_ERRORBOX));
        pSVData->maWinData.maMsgBoxImgList.emplace_back(BitmapEx(SV_RESID_BITMAP_QUERYBOX));
        pSVData->maWinData.maMsgBoxImgList.emplace_back(BitmapEx(SV_RESID_BITMAP_WARNINGBOX));
        pSVData->maWinData.maMsgBoxImgList.emplace_back(BitmapEx(SV_RESID_BITMAP_INFOBOX));
    }
}

void MessBox::ImplInitButtons()
{
    ButtonDialogFlags nOKFlags = ButtonDialogFlags::OK;
    ButtonDialogFlags nCancelFlags = ButtonDialogFlags::Cancel;
    ButtonDialogFlags nRetryFlags = ButtonDialogFlags::NONE;
    ButtonDialogFlags nYesFlags = ButtonDialogFlags::NONE;
    ButtonDialogFlags nNoFlags = ButtonDialogFlags::NONE;

    if ( mnMessBoxStyle & MessBoxStyle::OkCancel )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultCancel )
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // MessBoxStyle::DefaultOk
            nOKFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::OK, RET_OK, nOKFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::YesNo )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultYes )
            nYesFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // MessBoxStyle::DefaultNo
            nNoFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        nNoFlags |= ButtonDialogFlags::Cancel;

        AddButton( StandardButtonType::Yes, RET_YES, nYesFlags );
        AddButton( StandardButtonType::No, RET_NO, nNoFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::YesNoCancel )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultYes )
            nYesFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( mnMessBoxStyle & MessBoxStyle::DefaultNo )
            nNoFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Yes, RET_YES, nYesFlags );
        AddButton( StandardButtonType::No, RET_NO, nNoFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::RetryCancel )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultCancel )
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // MessBoxStyle::DefaultRetry
            nRetryFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Retry, RET_RETRY, nRetryFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::AbortRetryIgnore )
    {
        ButtonDialogFlags nAbortFlags = ButtonDialogFlags::NONE;
        ButtonDialogFlags nIgnoreFlags = ButtonDialogFlags::NONE;

        if ( mnMessBoxStyle & MessBoxStyle::DefaultCancel )
            nAbortFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( mnMessBoxStyle & MessBoxStyle::DefaultRetry )
            nRetryFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( mnMessBoxStyle & MessBoxStyle::DefaultIgnore )
            nIgnoreFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Abort, RET_CANCEL, nAbortFlags );
        AddButton( StandardButtonType::Retry, RET_RETRY, nRetryFlags );
        AddButton( StandardButtonType::Ignore, RET_IGNORE, nIgnoreFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::Ok )
    {
        nOKFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::OK, RET_OK, nOKFlags );
    }
}

MessBox::MessBox(vcl::Window* pParent, MessBoxStyle nMessBoxStyle, WinBits nWinBits,
                 const OUString& rTitle, const OUString& rMessage) :
    ButtonDialog( WindowType::MESSBOX ),
    mbHelpBtn( false ),
    mnMessBoxStyle( nMessBoxStyle ),
    maMessText( rMessage )
{
    ImplLOKNotifier(pParent);
    ImplInit(pParent, nWinBits | WB_MOVEABLE | WB_HORZ | WB_CENTER);
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
        aImageSize.AdjustWidth(4 );
        aImageSize.AdjustHeight(4 );
        aTextPos.AdjustX(aImageSize.Width()+IMPL_SEP_MSGBOX_IMAGE );
        mpFixedImage = VclPtr<FixedImage>::Create( this );
        mpFixedImage->SetPosSizePixel( Point( IMPL_DIALOG_OFFSET-2+IMPL_MSGBOX_OFFSET_EXTRA_X,
                                              IMPL_DIALOG_OFFSET-2+IMPL_MSGBOX_OFFSET_EXTRA_Y ),
                                       aImageSize );
        mpFixedImage->SetImage( maImage );
        mpFixedImage->Show();
        nMaxWidth -= aImageSize.Width()+IMPL_SEP_MSGBOX_IMAGE;
    }
    else
        aTextPos.AdjustX(IMPL_MSGBOX_OFFSET_EXTRA_X );

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

    aRect.SetRight( nWidth );
    aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    if ( aTextInfo.GetMaxLineWidth() > nWidth )
    {
        nWidth = aTextInfo.GetMaxLineWidth()+8;
        aRect.SetRight( nWidth );
        aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    }

    // get Style for VCLMultiLineEdit
    aMEditSize.setWidth( aTextInfo.GetMaxLineWidth()+1 );
    aMEditSize.setHeight( aFormatRect.GetHeight() );
    aPageSize.setWidth( aImageSize.Width() );
    if ( aMEditSize.Height() < aImageSize.Height() )
    {
        nWinStyle |= WB_VCENTER;
        aPageSize.setHeight( aImageSize.Height() );
        aMEditSize.setHeight( aImageSize.Height() );
    }
    else
    {
        nWinStyle |= WB_TOP;
        aPageSize.setHeight( aMEditSize.Height() );
    }
    if ( aImageSize.Width() )
        aPageSize.AdjustWidth(IMPL_SEP_MSGBOX_IMAGE );
    aPageSize.AdjustWidth((IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_X*2) );
    aPageSize.AdjustWidth(aMEditSize.Width()+1 );
    aPageSize.AdjustHeight((IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_Y*2) );

    if ( aPageSize.Width() < IMPL_MINSIZE_MSGBOX_WIDTH )
        aPageSize.setWidth( IMPL_MINSIZE_MSGBOX_WIDTH );
    if ( aPageSize.Width() < nTitleWidth )
        aPageSize.setWidth( nTitleWidth );

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

Size MessBox::GetOptimalSize() const
{
    // FIXME: base me on the font size ?
    return Size( 250, 100 );
}

Image const & GetStandardInfoBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[3];
}

OUString GetStandardInfoBoxText()
{
    return VclResId(SV_MSGBOX_INFO);
}

WarningBox::WarningBox( vcl::Window* pParent, MessBoxStyle nStyle,
                        const OUString& rMessage ) :
    WarningBox( pParent, nStyle, 0, rMessage )
{
}

WarningBox::WarningBox( vcl::Window* pParent, MessBoxStyle nStyle, WinBits nWinBits,
                        const OUString& rMessage ) :
    MessBox( pParent, nStyle, nWinBits, OUString(), rMessage )
{
    // Default Text is the display title from the application
    if ( GetText().isEmpty() )
        SetText( GetStandardText() );

    SetImage( WarningBox::GetStandardImage() );
}

Image const & WarningBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[2];
}

OUString WarningBox::GetStandardText()
{
    return VclResId(SV_MSGBOX_WARNING);
}

Image const & GetStandardErrorBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[0];
}

OUString GetStandardErrorBoxText()
{
    return VclResId(SV_MSGBOX_ERROR);
}

Image const & GetStandardQueryBoxImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.maMsgBoxImgList[1];
}

OUString GetStandardQueryBoxText()
{
    return VclResId(SV_MSGBOX_QUERY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
