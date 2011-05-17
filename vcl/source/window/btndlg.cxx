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

#include <tools/debug.hxx>
#include <tools/rc.h>
#include <vcl/button.hxx>
#include <vcl/btndlg.hxx>
#include <vcl/svdata.hxx>

typedef boost::ptr_vector<ImplBtnDlgItem>::iterator btn_iterator;
typedef boost::ptr_vector<ImplBtnDlgItem>::const_iterator btn_const_iterator;

struct ImplBtnDlgItem
{
    sal_uInt16              mnId;
    bool                mbOwnButton;
    bool                mbDummyAlign;
    long                mnSepSize;
    PushButton*         mpPushButton;
};

void ButtonDialog::ImplInitButtonDialogData()
{
    mnButtonSize            = 0;
    mnCurButtonId           = 0;
    mnFocusButtonId         = BUTTONDIALOG_BUTTON_NOTFOUND;
    mbFormat                = sal_True;
}

ButtonDialog::ButtonDialog( WindowType nType ) :
    Dialog( nType )
{
    ImplInitButtonDialogData();
}

ButtonDialog::ButtonDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_BUTTONDIALOG )
{
    ImplInitButtonDialogData();
    ImplInit( pParent, nStyle );
}

ButtonDialog::ButtonDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_BUTTONDIALOG )
{
    ImplInitButtonDialogData();
    rResId.SetRT( RSC_DIALOG );     // !!!!!!!!!! RSC_BUTTONDIALOG !!!!!!!!
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

ButtonDialog::~ButtonDialog()
{
    for ( btn_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        if ( it->mpPushButton && it->mbOwnButton )
            delete it->mpPushButton;
    }
}

PushButton* ButtonDialog::ImplCreatePushButton( sal_uInt16 nBtnFlags )
{
    PushButton* pBtn;
    WinBits     nStyle = 0;

    if ( nBtnFlags & BUTTONDIALOG_DEFBUTTON )
        nStyle |= WB_DEFBUTTON;
    if ( nBtnFlags & BUTTONDIALOG_CANCELBUTTON )
        pBtn = new CancelButton( this, nStyle );
    else if ( nBtnFlags & BUTTONDIALOG_OKBUTTON )
        pBtn = new OKButton( this, nStyle );
    else if ( nBtnFlags & BUTTONDIALOG_HELPBUTTON )
        pBtn = new HelpButton( this, nStyle );
    else
        pBtn = new PushButton( this, nStyle );

    if ( !(nBtnFlags & BUTTONDIALOG_HELPBUTTON) )
        pBtn->SetClickHdl( LINK( this, ButtonDialog, ImplClickHdl ) );

    return pBtn;
}

ImplBtnDlgItem* ButtonDialog::ImplGetItem( sal_uInt16 nId ) const
{
    for ( btn_const_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        if (it->mnId == nId)
            return const_cast<ImplBtnDlgItem*>(&(*it));
    }

    return NULL;
}

long ButtonDialog::ImplGetButtonSize()
{
    if ( !mbFormat )
        return mnButtonSize;

    // Calculate ButtonSize
    long nLastSepSize = 0;
    long nSepSize = 0;
    maCtrlSize = Size( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );

    for ( btn_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        nSepSize += nLastSepSize;

        long nTxtWidth = it->mpPushButton->GetCtrlTextWidth(it->mpPushButton->GetText());
        nTxtWidth += IMPL_EXTRA_BUTTON_WIDTH;

        if ( nTxtWidth > maCtrlSize.Width() )
            maCtrlSize.Width() = nTxtWidth;

        long nTxtHeight = it->mpPushButton->GetTextHeight();
        nTxtHeight += IMPL_EXTRA_BUTTON_HEIGHT;

        if ( nTxtHeight > maCtrlSize.Height() )
            maCtrlSize.Height() = nTxtHeight;

        nSepSize += it->mnSepSize;

        if ( GetStyle() & WB_HORZ )
            nLastSepSize = IMPL_SEP_BUTTON_X;
        else
            nLastSepSize = IMPL_SEP_BUTTON_Y;
    }

    long nButtonCount = maItemList.size();

    if ( GetStyle() & WB_HORZ )
        mnButtonSize  = nSepSize + (nButtonCount*maCtrlSize.Width());
    else
        mnButtonSize = nSepSize + (nButtonCount*maCtrlSize.Height());

    return mnButtonSize;
}

void ButtonDialog::ImplPosControls()
{
    if ( !mbFormat )
        return;

    // Create PushButtons and determine Sizes
    ImplGetButtonSize();

    // determine dialog size
    Size            aDlgSize = maPageSize;
    long            nX;
    long            nY;
    if ( GetStyle() & WB_HORZ )
    {
        if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Width() )
            aDlgSize.Width() = mnButtonSize+(IMPL_DIALOG_OFFSET*2);
        if ( GetStyle() & WB_LEFT )
            nX = IMPL_DIALOG_OFFSET;
        else if ( GetStyle() & WB_RIGHT )
            nX = aDlgSize.Width()-mnButtonSize-IMPL_DIALOG_OFFSET;
        else
            nX = (aDlgSize.Width()-mnButtonSize)/2;

        aDlgSize.Height() += IMPL_DIALOG_OFFSET+maCtrlSize.Height();
        nY = aDlgSize.Height()-maCtrlSize.Height()-IMPL_DIALOG_OFFSET;
    }
    else
    {
        if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Height() )
            aDlgSize.Height() = mnButtonSize+(IMPL_DIALOG_OFFSET*2);
        if ( GetStyle() & WB_BOTTOM )
            nY = aDlgSize.Height()-mnButtonSize-IMPL_DIALOG_OFFSET;
        else if ( GetStyle() & WB_VCENTER )
            nY = (aDlgSize.Height()-mnButtonSize)/2;
        else
            nY = IMPL_DIALOG_OFFSET;

        aDlgSize.Width() += IMPL_DIALOG_OFFSET+maCtrlSize.Width();
        nX = aDlgSize.Width()-maCtrlSize.Width()-IMPL_DIALOG_OFFSET;
    }

    // Arrange PushButtons
    for ( btn_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        if ( GetStyle() & WB_HORZ )
            nX += it->mnSepSize;
        else
            nY += it->mnSepSize;

        it->mpPushButton->SetPosSizePixel( Point( nX, nY ), maCtrlSize );
        it->mpPushButton->Show();

        if ( GetStyle() & WB_HORZ )
            nX += maCtrlSize.Width()+IMPL_SEP_BUTTON_X;
        else
            nY += maCtrlSize.Height()+IMPL_SEP_BUTTON_Y;
    }

    SetOutputSizePixel( aDlgSize );

    mbFormat = sal_False;
}

IMPL_LINK( ButtonDialog, ImplClickHdl, PushButton*, pBtn )
{
    for ( btn_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        if ( it->mpPushButton == pBtn )
        {
            mnCurButtonId = it->mnId;
            Click();
            break;
        }
    }

    return 0;
}

void ButtonDialog::Resize()
{
}

void ButtonDialog::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        ImplPosControls();

        // Set focus on default button.
        if ( mnFocusButtonId != BUTTONDIALOG_BUTTON_NOTFOUND )
        {
            for ( btn_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
            {
                if (it->mnId == mnFocusButtonId )
                {
                    if (it->mpPushButton->IsVisible())
                        it->mpPushButton->GrabFocus();

                    break;
                }
            }
        }
    }

    Dialog::StateChanged( nType );
}

void ButtonDialog::Click()
{
    if ( !maClickHdl )
    {
        if ( IsInExecute() )
            EndDialog( GetCurButtonId() );
    }
    else
        maClickHdl.Call( this );
}

void ButtonDialog::AddButton( const XubString& rText, sal_uInt16 nId,
                              sal_uInt16 nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = sal_True;
    pItem->mnSepSize        = nSepPixel;
    pItem->mpPushButton     = ImplCreatePushButton( nBtnFlags );

    if ( rText.Len() )
        pItem->mpPushButton->SetText( rText );

    maItemList.push_back(pItem);

    if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
        mnFocusButtonId = nId;

    mbFormat = sal_True;
}

void ButtonDialog::AddButton( StandardButtonType eType, sal_uInt16 nId,
                              sal_uInt16 nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = sal_True;
    pItem->mnSepSize        = nSepPixel;

    if ( eType == BUTTON_OK )
        nBtnFlags |= BUTTONDIALOG_OKBUTTON;
    else if ( eType == BUTTON_HELP )
        nBtnFlags |= BUTTONDIALOG_HELPBUTTON;
    else if ( (eType == BUTTON_CANCEL) || (eType == BUTTON_CLOSE) )
        nBtnFlags |= BUTTONDIALOG_CANCELBUTTON;
    pItem->mpPushButton = ImplCreatePushButton( nBtnFlags );

    // Standard-Buttons have the right text already
    if ( !((eType == BUTTON_OK)     && (pItem->mpPushButton->GetType() == WINDOW_OKBUTTON)) ||
         !((eType == BUTTON_CANCEL) && (pItem->mpPushButton->GetType() == WINDOW_CANCELBUTTON)) ||
         !((eType == BUTTON_HELP)   && (pItem->mpPushButton->GetType() == WINDOW_HELPBUTTON)) )
    {
        pItem->mpPushButton->SetText( Button::GetStandardText( eType ) );
        pItem->mpPushButton->SetHelpText( Button::GetStandardHelpText( eType ) );
    }

    if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
        mnFocusButtonId = nId;

    maItemList.push_back(pItem);

    mbFormat = sal_True;
}

void ButtonDialog::AddButton( PushButton* pBtn, sal_uInt16 nId,
                              sal_uInt16 nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = sal_False;
    pItem->mnSepSize        = nSepPixel;
    pItem->mpPushButton     = pBtn;

    if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
        mnFocusButtonId = nId;

    maItemList.push_back(pItem);

    mbFormat = sal_True;
}

void ButtonDialog::RemoveButton( sal_uInt16 nId )
{
    btn_iterator it;
    for (it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        if (it->mnId == nId)
        {
            it->mpPushButton->Hide();

            if (it->mbOwnButton )
                delete it->mpPushButton;

            maItemList.erase(it);
            break;
        }
    }

    if (it == maItemList.end())
        DBG_ERRORFILE( "ButtonDialog::RemoveButton(): ButtonId invalid" );
}

void ButtonDialog::Clear()
{
    for (btn_iterator it = maItemList.begin(); it != maItemList.end(); ++it)
    {
        it->mpPushButton->Hide();

        if (it->mbOwnButton )
            delete it->mpPushButton;
    }

    maItemList.clear();
    mbFormat = sal_True;
}

sal_uInt16 ButtonDialog::GetButtonCount() const
{
    return (sal_uInt16)maItemList.size();
}

sal_uInt16 ButtonDialog::GetButtonId( sal_uInt16 nButton ) const
{
    if ( nButton < maItemList.size() )
        return maItemList[nButton].mnId;
    else
        return BUTTONDIALOG_BUTTON_NOTFOUND;
}

PushButton* ButtonDialog::GetPushButton( sal_uInt16 nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton;
    else
        return NULL;
}

void ButtonDialog::SetButtonText( sal_uInt16 nId, const XubString& rText )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
    {
        pItem->mpPushButton->SetText( rText );
        mbFormat = sal_True;
    }
}

XubString ButtonDialog::GetButtonText( sal_uInt16 nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton->GetText();
    else
        return ImplGetSVEmptyStr();
}

void ButtonDialog::SetButtonHelpText( sal_uInt16 nId, const XubString& rText )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        pItem->mpPushButton->SetHelpText( rText );
}

XubString ButtonDialog::GetButtonHelpText( sal_uInt16 nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        return pItem->mpPushButton->GetHelpText();
    else
        return ImplGetSVEmptyStr();
}

void ButtonDialog::SetButtonHelpId( sal_uInt16 nId, const rtl::OString& rHelpId )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        pItem->mpPushButton->SetHelpId( rHelpId );
}

rtl::OString ButtonDialog::GetButtonHelpId( sal_uInt16 nId ) const
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    return pItem ? rtl::OString( pItem->mpPushButton->GetHelpId() ) : rtl::OString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
