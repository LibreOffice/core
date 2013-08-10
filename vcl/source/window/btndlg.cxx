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

#include <svdata.hxx>

#include <vcl/button.hxx>
#include <vcl/btndlg.hxx>

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

void ButtonDialog::AddButton( const OUString& rText, sal_uInt16 nId,
                              sal_uInt16 nBtnFlags, long nSepPixel )
{
    // PageItem anlegen
    ImplBtnDlgItem* pItem   = new ImplBtnDlgItem;
    pItem->mnId             = nId;
    pItem->mbOwnButton      = sal_True;
    pItem->mnSepSize        = nSepPixel;
    pItem->mpPushButton     = ImplCreatePushButton( nBtnFlags );

    if (!rText.isEmpty())
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
    }

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
        SAL_WARN( "vcl.window", "ButtonDialog::RemoveButton(): ButtonId invalid" );
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

void ButtonDialog::SetButtonText( sal_uInt16 nId, const OUString& rText )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
    {
        pItem->mpPushButton->SetText( rText );
        mbFormat = sal_True;
    }
}

void ButtonDialog::SetButtonHelpText( sal_uInt16 nId, const OUString& rText )
{
    ImplBtnDlgItem* pItem = ImplGetItem( nId );

    if ( pItem )
        pItem->mpPushButton->SetHelpText( rText );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
