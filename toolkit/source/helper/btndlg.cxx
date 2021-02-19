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

#include <vcl/toolkit/button.hxx>
#include <vcl/stdtext.hxx>
#include <helper/btndlg.hxx>
#include <sal/log.hxx>
#include <map>
#include <memory>

struct ImplBtnDlgItem
{
    sal_uInt16              mnId;
    bool                    mbOwnButton;
    tools::Long                    mnSepSize;
    VclPtr<PushButton>      mpPushButton;

    ImplBtnDlgItem() : mnId(0), mbOwnButton(false), mnSepSize(0) {}
};

void ButtonDialog::ImplInitButtonDialogData()
{
    mnButtonSize            = 0;
    mnCurButtonId           = 0;
    mnFocusButtonId         = BUTTONDIALOG_BUTTON_NOTFOUND;
    mbFormat                = true;
}

ButtonDialog::ButtonDialog( WindowType nType ) :
    Dialog( nType )
{
    ImplInitButtonDialogData();
}

ButtonDialog::~ButtonDialog()
{
    disposeOnce();
}

void ButtonDialog::dispose()
{
    for (auto & it : m_ItemList)
    {
        if ( it->mbOwnButton )
            it->mpPushButton.disposeAndClear();
    }
    m_ItemList.clear();
    Dialog::dispose();
}

VclPtr<PushButton> ButtonDialog::ImplCreatePushButton( ButtonDialogFlags nBtnFlags )
{
    VclPtr<PushButton> pBtn;
    WinBits     nStyle = 0;

    if ( nBtnFlags & ButtonDialogFlags::Default )
        nStyle |= WB_DEFBUTTON;
    if ( nBtnFlags & ButtonDialogFlags::Cancel )
        pBtn = VclPtr<CancelButton>::Create( this, nStyle );
    else if ( nBtnFlags & ButtonDialogFlags::OK )
        pBtn = VclPtr<OKButton>::Create( this, nStyle );
    else if ( nBtnFlags & ButtonDialogFlags::Help )
        pBtn = VclPtr<HelpButton>::Create( this, nStyle );
    else
        pBtn = VclPtr<PushButton>::Create( this, nStyle );

    if ( !(nBtnFlags & ButtonDialogFlags::Help) )
        pBtn->SetClickHdl( LINK( this, ButtonDialog, ImplClickHdl ) );

    return pBtn;
}

tools::Long ButtonDialog::ImplGetButtonSize()
{
    if ( !mbFormat )
        return mnButtonSize;

    // Calculate ButtonSize
    tools::Long nLastSepSize = 0;
    tools::Long nSepSize = 0;
    maCtrlSize = Size( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );

    for (const auto & it : m_ItemList)
    {
        nSepSize += nLastSepSize;

        tools::Long nTxtWidth = it->mpPushButton->GetCtrlTextWidth(it->mpPushButton->GetText());
        nTxtWidth += IMPL_EXTRA_BUTTON_WIDTH;

        if ( nTxtWidth > maCtrlSize.Width() )
            maCtrlSize.setWidth( nTxtWidth );

        tools::Long nTxtHeight = it->mpPushButton->GetTextHeight();
        nTxtHeight += IMPL_EXTRA_BUTTON_HEIGHT;

        if ( nTxtHeight > maCtrlSize.Height() )
            maCtrlSize.setHeight( nTxtHeight );

        nSepSize += it->mnSepSize;

        if ( GetStyle() & WB_HORZ )
            nLastSepSize = IMPL_SEP_BUTTON_X;
        else
            nLastSepSize = IMPL_SEP_BUTTON_Y;
    }

    size_t const nButtonCount = m_ItemList.size();

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
    tools::Long            nX;
    tools::Long            nY;
    if ( GetStyle() & WB_HORZ )
    {
        if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Width() )
            aDlgSize.setWidth( mnButtonSize+(IMPL_DIALOG_OFFSET*2) );
        if ( GetStyle() & WB_LEFT )
            nX = IMPL_DIALOG_OFFSET;
        else if ( GetStyle() & WB_RIGHT )
            nX = aDlgSize.Width()-mnButtonSize-IMPL_DIALOG_OFFSET;
        else
            nX = (aDlgSize.Width()-mnButtonSize)/2;

        aDlgSize.AdjustHeight(IMPL_DIALOG_OFFSET+maCtrlSize.Height() );
        nY = aDlgSize.Height()-maCtrlSize.Height()-IMPL_DIALOG_OFFSET;
    }
    else
    {
        if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Height() )
            aDlgSize.setHeight( mnButtonSize+(IMPL_DIALOG_OFFSET*2) );
        if ( GetStyle() & WB_BOTTOM )
            nY = aDlgSize.Height()-mnButtonSize-IMPL_DIALOG_OFFSET;
        else if ( GetStyle() & WB_VCENTER )
            nY = (aDlgSize.Height()-mnButtonSize)/2;
        else
            nY = IMPL_DIALOG_OFFSET;

        aDlgSize.AdjustWidth(IMPL_DIALOG_OFFSET+maCtrlSize.Width() );
        nX = aDlgSize.Width()-maCtrlSize.Width()-IMPL_DIALOG_OFFSET;
    }

    // Arrange PushButtons
    for (auto & it : m_ItemList)
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

    SetOutputSizePixel(aDlgSize);
    SetMinOutputSizePixel(aDlgSize);

    mbFormat = false;
}

IMPL_LINK( ButtonDialog, ImplClickHdl, Button*, pBtn, void )
{
    for (const auto & it : m_ItemList)
    {
        if ( it->mpPushButton == pBtn )
        {
            mnCurButtonId = it->mnId;
            if ( IsInExecute() )
                EndDialog( mnCurButtonId );
            break;
        }
    }
}

void ButtonDialog::Resize()
{
}

void ButtonDialog::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
    {
        ImplPosControls();
        for (auto & it : m_ItemList)
        {
            if ( it->mpPushButton && it->mbOwnButton )
                it->mpPushButton->SetZOrder(nullptr, ZOrderFlags::Last);
        }

        // Set focus on default button.
        if ( mnFocusButtonId != BUTTONDIALOG_BUTTON_NOTFOUND )
        {
            for (auto & it : m_ItemList)
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

void ButtonDialog::AddButton( StandardButtonType eType, sal_uInt16 nId,
                              ButtonDialogFlags nBtnFlags, tools::Long nSepPixel )
{
    // PageItem anlegen
    std::unique_ptr<ImplBtnDlgItem> pItem(new ImplBtnDlgItem);
    pItem->mnId             = nId;
    pItem->mbOwnButton      = true;
    pItem->mnSepSize        = nSepPixel;

    if ( eType == StandardButtonType::OK )
        nBtnFlags |= ButtonDialogFlags::OK;
    else if ( eType == StandardButtonType::Help )
        nBtnFlags |= ButtonDialogFlags::Help;
    else if ( (eType == StandardButtonType::Cancel) || (eType == StandardButtonType::Close) )
        nBtnFlags |= ButtonDialogFlags::Cancel;
    pItem->mpPushButton = ImplCreatePushButton( nBtnFlags );

    // Standard-Buttons have the right text already
    if ( !((eType == StandardButtonType::OK     && pItem->mpPushButton->GetType() == WindowType::OKBUTTON) ||
           (eType == StandardButtonType::Cancel && pItem->mpPushButton->GetType() == WindowType::CANCELBUTTON) ||
           (eType == StandardButtonType::Help   && pItem->mpPushButton->GetType() == WindowType::HELPBUTTON)) )
    {
        std::map<StandardButtonType, OUString> mapButtonTypeToID = {{StandardButtonType::Yes, "yes"},
            {StandardButtonType::No, "no"}, {StandardButtonType::Retry, "retry"},
            {StandardButtonType::Close, "close"}, {StandardButtonType::More, "more"},
            {StandardButtonType::Ignore, "ignore"}, {StandardButtonType::Abort, "abort"},
            {StandardButtonType::Less, "less"}, {StandardButtonType::Count, "count"}};
        auto itr = mapButtonTypeToID.find(eType);
        if (itr != mapButtonTypeToID.end())
            pItem->mpPushButton->set_id(itr->second);
        pItem->mpPushButton->SetText( GetStandardText( eType ) );
    }

    if ( nBtnFlags & ButtonDialogFlags::Focus )
        mnFocusButtonId = nId;

    m_ItemList.push_back(std::move(pItem));

    mbFormat = true;
}

void ButtonDialog::RemoveButton( sal_uInt16 nId )
{
    auto it = std::find_if(m_ItemList.begin(), m_ItemList.end(),
        [&nId](const std::unique_ptr<ImplBtnDlgItem>& rItem) { return rItem->mnId == nId; });
    if (it != m_ItemList.end())
    {
        (*it)->mpPushButton->Hide();
        if ((*it)->mbOwnButton)
            (*it)->mpPushButton.disposeAndClear();
        else
            (*it)->mpPushButton.clear();
        m_ItemList.erase(it);
        return;
    }

    SAL_WARN( "vcl.window", "ButtonDialog::RemoveButton(): ButtonId invalid" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
