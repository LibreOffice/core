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



// =======================================================================

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

// =======================================================================

void MessBox::ImplInitMessBoxData()
{
    mpVCLMultiLineEdit  = NULL;
    mpFixedImage        = NULL;
    mbHelpBtn           = sal_False;
    mpCheckBox          = NULL;
    mbCheck             = sal_False;
}

// -----------------------------------------------------------------------

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

        AddButton( BUTTON_OK, BUTTONID_OK, nOKFlags );
        AddButton( BUTTON_CANCEL, BUTTONID_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_YES_NO )
    {
        if ( nStyle & WB_DEF_YES )
            nYesFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else // WB_DEF_NO
            nNoFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        nNoFlags |= BUTTONDIALOG_CANCELBUTTON;

        AddButton( BUTTON_YES, BUTTONID_YES, nYesFlags );
        AddButton( BUTTON_NO, BUTTONID_NO, nNoFlags );
    }
    else if ( nStyle & WB_YES_NO_CANCEL )
    {
        if ( nStyle & WB_DEF_YES )
            nYesFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else if ( nStyle & WB_DEF_NO )
            nNoFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else
            nCancelFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_YES, BUTTONID_YES, nYesFlags );
        AddButton( BUTTON_NO, BUTTONID_NO, nNoFlags );
        AddButton( BUTTON_CANCEL, BUTTONID_CANCEL, nCancelFlags );
    }
    else if ( nStyle & WB_RETRY_CANCEL )
    {
        if ( nStyle & WB_DEF_CANCEL )
            nCancelFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;
        else // WB_DEF_RETRY
            nRetryFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_RETRY, BUTTONID_RETRY, nRetryFlags );
        AddButton( BUTTON_CANCEL, BUTTONID_CANCEL, nCancelFlags );
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

        AddButton( BUTTON_ABORT, BUTTONID_CANCEL, nAbortFlags );
        AddButton( BUTTON_RETRY, BUTTONID_RETRY, nRetryFlags );
        AddButton( BUTTON_IGNORE, BUTTONID_IGNORE, nIgnoreFlags );
    }
    else if ( nStyle & WB_OK )
    {
        nOKFlags |= BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON;

        AddButton( BUTTON_OK, BUTTONID_OK, nOKFlags );
    }
}

// -----------------------------------------------------------------------

MessBox::MessBox( Window* pParent, WinBits nStyle,
                  const XubString& rTitle, const XubString& rMessage ) :
    ButtonDialog( WINDOW_MESSBOX ),
    maMessText( rMessage )
{
    ImplInitMessBoxData();
    ImplInit( pParent, nStyle | WB_MOVEABLE | WB_HORZ | WB_CENTER );
    ImplInitButtons();

    if ( rTitle.Len() )
        SetText( rTitle );
}

// -----------------------------------------------------------------------

MessBox::MessBox( Window* pParent, const ResId& rResId ) :
    ButtonDialog( WINDOW_MESSBOX )
{
    ImplInitMessBoxData();

    GetRes( rResId.SetRT( RSC_MESSBOX ) );
    sal_uInt16 nHiButtons   = ReadShortRes();
    sal_uInt16 nLoButtons   = ReadShortRes();
    sal_uInt16 nHiDefButton = ReadShortRes();
    sal_uInt16 nLoDefButton = ReadShortRes();
    rtl::OString aHelpId( ReadByteStringRes() );
    /* sal_uInt16 bSysModal = */ ReadShortRes();
    SetHelpId( aHelpId );
    WinBits nBits = (((sal_uLong)nHiButtons << 16) + nLoButtons) |
                    (((sal_uLong)nHiDefButton << 16) + nLoDefButton);
    ImplInit( pParent, nBits | WB_MOVEABLE | WB_HORZ | WB_CENTER );

    ImplLoadRes( rResId );
    ImplInitButtons();
}

// -----------------------------------------------------------------------

void MessBox::ImplLoadRes( const ResId& )
{
    SetText(     ReadStringRes() );
    SetMessText( ReadStringRes() );
    SetHelpText( ReadStringRes() );
}

// -----------------------------------------------------------------------

MessBox::~MessBox()
{
    delete mpVCLMultiLineEdit;
    delete mpFixedImage;
    delete mpCheckBox;
}

// -----------------------------------------------------------------------

void MessBox::ImplPosControls()
{
    if ( !GetHelpId().isEmpty() )
    {
        if ( !mbHelpBtn )
        {
            AddButton( BUTTON_HELP, BUTTONID_HELP, BUTTONDIALOG_HELPBUTTON, 3 );
            mbHelpBtn = sal_True;
        }
    }
    else
    {
        if ( mbHelpBtn )
        {
            RemoveButton( BUTTONID_HELP );
            mbHelpBtn = sal_False;
        }
    }

    XubString       aMessText( maMessText );
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
    WinBits         nWinStyle = WB_LEFT | WB_WORDBREAK | WB_NOLABEL;
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


    // Message-Text um Tabs bereinigen
    rtl::OUString   aTabStr("    ");
    sal_uInt16      nIndex = 0;
    while ( nIndex != STRING_NOTFOUND )
        nIndex = aMessText.SearchAndReplace( rtl::OUString('\t'), aTabStr, nIndex );

    // Wenn Fenster zu schmall, machen wir Dialog auch breiter
    if ( mpWindowImpl->mbFrame )
        nMaxWidth = 630;
    else if ( nMaxWidth < 120 )
        nMaxWidth = 120;

    nMaxWidth -= mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder+4;

    // MessageBox sollte min. so breit sein, das auch Title sichtbar ist
    // Extra-Width for Closer, because Closer is set after this call
    nTitleWidth = CalcTitleWidth();
    nTitleWidth += mpWindowImpl->mnTopBorder;

    nMaxWidth -= (IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_X*2);

    // Wenn wir ein Image haben, dann deren Groesse ermitteln und das
    // entsprechende Control anlegen und positionieren
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

    // Maximale Zeilenlaenge ohne Wordbreak ermitteln
    aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    nMaxLineWidth = aFormatRect.GetWidth();
    nTextStyle |= TEXT_DRAW_WORDBREAK;

    // Breite fuer Textformatierung ermitteln
    if ( nMaxLineWidth > 450 )
        nWidth = 450;
    else if ( nMaxLineWidth > 300 )
        nWidth = nMaxLineWidth+5;
    else
        nWidth = 300;
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

    // Style fuer VCLMultiLineEdit ermitteln
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

    if ( maCheckBoxText.Len() )
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

        String aMnemonicString( maCheckBoxText );
        if( GetSettings().GetStyleSettings().GetAutoMnemonic() )
        {
            if( aMnemonicString == GetNonMnemonicString( maCheckBoxText ) )
            {
                // no mnemonic found -> create one
                MnemonicGenerator aMnemonicGenerator;
                aMnemonicGenerator.CreateMnemonic( aMnemonicString );
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
    mpVCLMultiLineEdit->SetPaintTransparent(sal_True);
    mpVCLMultiLineEdit->EnableCursor(sal_False);
    SetPageSizePixel( aPageSize );
}

// -----------------------------------------------------------------------

void MessBox::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        ImplPosControls();
    }
    ButtonDialog::StateChanged( nType );
}

// -----------------------------------------------------------------------

sal_Bool MessBox::GetCheckBoxState() const
{
    return mpCheckBox ? mpCheckBox->IsChecked() : mbCheck;
}

// -----------------------------------------------------------------------

void MessBox::SetCheckBoxState( sal_Bool bCheck )
{
    if( mpCheckBox ) mpCheckBox->Check( bCheck );
    mbCheck = bCheck;
}

// -----------------------------------------------------------------------

Size MessBox::GetOptimalSize(WindowSizeType eType) const
{
    switch( eType ) {
    case WINDOWSIZE_MINIMUM:
        // FIXME: base me on the font size ?
        return Size( 250, 100 );
    default:
        return Window::GetOptimalSize( eType );
    }
}

// ============================================================================

void InfoBox::ImplInitInfoBoxData()
{
    // Default Text is the display title from the application
    if ( !GetText().Len() )
        SetText( Application::GetDisplayName() );

    SetImage( InfoBox::GetStandardImage() );
}

// -----------------------------------------------------------------------

InfoBox::InfoBox( Window* pParent, const XubString& rMessage ) :
    MessBox( pParent, WB_OK | WB_DEF_OK, ImplGetSVEmptyStr(), rMessage )
{
    ImplInitInfoBoxData();
}

// -----------------------------------------------------------------------

InfoBox::InfoBox( Window* pParent, const ResId & rResId ) :
    MessBox( pParent, rResId.SetRT( RSC_INFOBOX ) )
{
    ImplInitInfoBoxData();
}

// -----------------------------------------------------------------------

Image InfoBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 4 );
}

// ============================================================================

void WarningBox::ImplInitWarningBoxData()
{
    // Default Text is the display title from the application
    if ( !GetText().Len() )
        SetText( Application::GetDisplayName() );

    SetImage( WarningBox::GetStandardImage() );
}

// -----------------------------------------------------------------------

WarningBox::WarningBox( Window* pParent, WinBits nStyle,
                        const XubString& rMessage ) :
    MessBox( pParent, nStyle, ImplGetSVEmptyStr(), rMessage )
{
    ImplInitWarningBoxData();
}

// -----------------------------------------------------------------------

WarningBox::WarningBox( Window* pParent, const ResId& rResId ) :
    MessBox( pParent, rResId.SetRT( RSC_WARNINGBOX ) )
{
    ImplInitWarningBoxData();
}

// -----------------------------------------------------------------------

void WarningBox::SetDefaultCheckBoxText()
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
        maCheckBoxText = ResId(SV_STDTEXT_DONTWARNAGAIN, *pResMgr).toString();
}

// -----------------------------------------------------------------------

Image WarningBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 3 );
}

// ============================================================================

void ErrorBox::ImplInitErrorBoxData()
{
    // Default Text is the display title from the application
    if ( !GetText().Len() )
        SetText( Application::GetDisplayName() );

    SetImage( ErrorBox::GetStandardImage() );
}

// -----------------------------------------------------------------------

ErrorBox::ErrorBox( Window* pParent, WinBits nStyle,
                    const XubString& rMessage ) :
    MessBox( pParent, nStyle, ImplGetSVEmptyStr(), rMessage )
{
    ImplInitErrorBoxData();
}

// -----------------------------------------------------------------------

ErrorBox::ErrorBox( Window* pParent, const ResId& rResId ) :
    MessBox( pParent, rResId.SetRT( RSC_ERRORBOX ) )
{
    ImplInitErrorBoxData();
}

// -----------------------------------------------------------------------

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

// ============================================================================

void QueryBox::ImplInitQueryBoxData()
{
    // Default Text is the display title from the application
    if ( !GetText().Len() )
        SetText( Application::GetDisplayName() );

    SetImage( QueryBox::GetStandardImage() );
}

// -----------------------------------------------------------------------

QueryBox::QueryBox( Window* pParent, WinBits nStyle, const XubString& rMessage ) :
    MessBox( pParent, nStyle, ImplGetSVEmptyStr(), rMessage )
{
    ImplInitQueryBoxData();
}

// -----------------------------------------------------------------------

QueryBox::QueryBox( Window* pParent, const ResId& rResId ) :
    MessBox( pParent, rResId.SetRT( RSC_QUERYBOX ) )
{
    ImplInitQueryBoxData();
}

// -----------------------------------------------------------------------

void QueryBox::SetDefaultCheckBoxText()
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( pResMgr )
        maCheckBoxText = ResId(SV_STDTEXT_DONTASKAGAIN, *pResMgr).toString();
}

// -----------------------------------------------------------------------

Image QueryBox::GetStandardImage()
{
    ImplInitMsgBoxImageList();
    return ImplGetSVData()->maWinData.mpMsgBoxImgList->GetImage( 2 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
