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

#include <vcl/event.hxx>

#include <formula/funcutl.hxx>
#include <formula/IControlReferenceHandler.hxx>
#include <vcl/svapp.hxx>
#include "ControlHelper.hxx"
#include "parawin.hxx"
#include <strings.hrc>
#include <bitmaps.hlst>
#include <core_resource.hxx>

namespace formula
{

ArgEdit::ArgEdit(std::unique_ptr<weld::Entry> xControl)
    : RefEdit(std::move(xControl))
    , pEdPrev(nullptr)
    , pEdNext(nullptr)
    , pSlider(nullptr)
    , pParaWin(nullptr)
    , nArgs(0)
{
}

void ArgEdit::Init(ArgEdit* pPrevEdit, ArgEdit* pNextEdit,
                   weld::ScrolledWindow& rArgSlider,
                   ParaWin& rParaWin, sal_uInt16 nArgCount)
{
    pEdPrev = pPrevEdit;
    pEdNext = pNextEdit;
    pSlider = &rArgSlider;
    pParaWin = &rParaWin;
    nArgs   = nArgCount;
}

// Cursor control for Edit Fields in Argument Dialog
bool ArgEdit::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bUp = (aCode.GetCode() == KEY_UP);
    bool bDown = (aCode.GetCode() == KEY_DOWN);

    if (   pSlider
        && ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() )
        && ( bUp || bDown ) )
    {
        if ( nArgs > 1 )
        {
            ArgEdit* pEd = nullptr;
            int nThumb = pSlider->vadjustment_get_value();
            bool bDoScroll = false;
            bool bChangeFocus = false;

            if ( bDown )
            {
                if ( nArgs > 4 )
                {
                    if ( !pEdNext )
                    {
                        nThumb++;
                        bDoScroll = ( nThumb+3 < static_cast<tools::Long>(nArgs) );
                    }
                    else
                    {
                        pEd = pEdNext;
                        bChangeFocus = true;
                    }
                }
                else if ( pEdNext )
                {
                    pEd = pEdNext;
                    bChangeFocus = true;
                }
            }
            else // if ( bUp )
            {
                if ( nArgs > 4 )
                {
                    if ( !pEdPrev )
                    {
                        nThumb--;
                        bDoScroll = ( nThumb >= 0 );
                    }
                    else
                    {
                        pEd = pEdPrev;
                        bChangeFocus = true;
                    }
                }
                else if ( pEdPrev )
                {
                    pEd = pEdPrev;
                    bChangeFocus = true;
                }
            }

            if ( bDoScroll )
            {
                pSlider->vadjustment_set_value( nThumb );
                pParaWin->SliderMoved();
            }
            else if ( bChangeFocus )
            {
                pEd->GrabFocus();
            }
        }
        return true;
    }
    return RefEdit::KeyInput(rKEvt);
}

ArgInput::ArgInput()
{
    m_pFtArg = nullptr;
    m_pBtnFx = nullptr;
    m_pEdArg = nullptr;
    m_pRefBtn = nullptr;
}

void ArgInput::InitArgInput(weld::Label& rArgLabel, weld::Button& rFxButton, ArgEdit& rArgEdit,
                            RefButton& rRefButton)
{
    m_pFtArg = &rArgLabel;
    m_pBtnFx = &rFxButton;
    m_pEdArg = &rArgEdit;
    m_pRefBtn = &rRefButton;

    m_pBtnFx->connect_clicked(LINK(this, ArgInput, FxBtnClickHdl));
    m_pBtnFx->connect_focus_in(LINK(this, ArgInput, FxBtnFocusHdl));

    m_pEdArg->SetGetFocusHdl(LINK(this, ArgInput, EdFocusHdl));
    m_pEdArg->SetModifyHdl(LINK(this, ArgInput, EdModifyHdl));
}

// Sets the Name for the Argument
void ArgInput::SetArgName(const OUString &aArg)
{
    if (m_pFtArg)
        m_pFtArg->set_label(aArg);
}

// Returns the Name for the Argument
OUString ArgInput::GetArgName() const
{
    OUString aPrivArgName;
    if (m_pFtArg)
        aPrivArgName = m_pFtArg->get_label();
    return aPrivArgName;
}

//Sets the Name for the Argument
void ArgInput::SetArgNameFont(const vcl::Font &aFont)
{
    if (m_pFtArg)
        m_pFtArg->set_font(aFont);
}

//Sets up the Selection for the EditBox.
void ArgInput::SelectAll()
{
    if (m_pEdArg)
        m_pEdArg->SelectAll();
}

//Sets the Value for the Argument
void ArgInput::SetArgVal(const OUString &rVal)
{
    if (m_pEdArg)
        m_pEdArg->SetRefString(rVal);
}

//Returns the Value for the Argument
OUString ArgInput::GetArgVal() const
{
    OUString aResult;
    if (m_pEdArg)
        aResult = m_pEdArg->GetText();
    return aResult;
}

//Hides the Controls
void ArgInput::Hide()
{
    if (m_pFtArg && m_pBtnFx && m_pEdArg && m_pRefBtn)
    {
        m_pFtArg->hide();
        m_pBtnFx->hide();
        m_pEdArg->GetWidget()->hide();
        m_pRefBtn->GetWidget()->hide();
    }
}

//Casts the Controls again.
void ArgInput::Show()
{
    if (m_pFtArg && m_pBtnFx && m_pEdArg && m_pRefBtn)
    {
        m_pFtArg->show();
        m_pBtnFx->show();
        m_pEdArg->GetWidget()->show();
        m_pRefBtn->GetWidget()->show();
    }
}

void ArgInput::UpdateAccessibleNames()
{
    OUString aArgName = ":" + m_pFtArg->get_label();

    OUString aName = m_pBtnFx->get_tooltip_text() + aArgName;
    m_pBtnFx->set_accessible_name(aName);

    aName = m_pRefBtn->GetWidget()->get_tooltip_text() + aArgName;
    m_pRefBtn->GetWidget()->set_accessible_name(aName);
}

IMPL_LINK(ArgInput, FxBtnClickHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_pBtnFx)
        m_aFxClickLink.Call(*this);
}

IMPL_LINK( ArgInput, FxBtnFocusHdl, weld::Widget&, rControl, void )
{
    if (&rControl == m_pBtnFx)
        m_aFxFocusLink.Call(*this);
}

IMPL_LINK( ArgInput, EdFocusHdl, RefEdit&, rControl, void )
{
    if (&rControl == m_pEdArg)
        m_aEdFocusLink.Call(*this);
}

IMPL_LINK( ArgInput, EdModifyHdl, RefEdit&, rEdit, void )
{
    if (&rEdit == m_pEdArg)
        m_aEdModifyLink.Call(*this);
}

RefEdit::RefEdit(std::unique_ptr<weld::Entry> xControl)
    : mxEntry(std::move(xControl))
    , maIdle("formula RefEdit Idle")
    , mpAnyRefDlg(nullptr)
    , mpFocusInEvent(nullptr)
    , mpFocusOutEvent(nullptr)
{
    mxEntry->connect_focus_in(LINK(this, RefEdit, GetFocusHdl));
    mxEntry->connect_focus_out(LINK(this, RefEdit, LoseFocusHdl));
    mxEntry->connect_key_press(LINK(this, RefEdit, KeyInputHdl));
    mxEntry->connect_changed(LINK(this, RefEdit, Modify));
    maIdle.SetInvokeHandler(LINK(this, RefEdit, UpdateHdl));
}

RefEdit::~RefEdit()
{
    if (mpFocusInEvent)
        Application::RemoveUserEvent(mpFocusInEvent);
    if (mpFocusOutEvent)
        Application::RemoveUserEvent(mpFocusOutEvent);
    maIdle.ClearInvokeHandler();
    maIdle.Stop();
}

void RefEdit::SetRefString( const OUString& rStr )
{
    // Prevent unwanted side effects by setting only a differing string.
    // See commit message for reasons.
    if (mxEntry->get_text() != rStr)
        mxEntry->set_text(rStr);
}

void RefEdit::SetRefValid(bool bValid)
{
    mxEntry->set_message_type(bValid ? weld::EntryMessageType::Normal
                                     : weld::EntryMessageType::Error);
}

void RefEdit::SetText(const OUString& rStr)
{
    mxEntry->set_text(rStr);
    UpdateHdl(&maIdle);
}

void RefEdit::StartUpdateData()
{
    maIdle.Start();
}

void RefEdit::SetReferences(IControlReferenceHandler* pDlg, weld::Label* pLabel)
{
    mpAnyRefDlg = pDlg;
    if (pLabel)
        maGetLabelTextForShrinkModeFunc = [pLabel] { return pLabel->get_label(); };
    else
        maGetLabelTextForShrinkModeFunc = nullptr;

    if( pDlg )
    {
        maIdle.SetInvokeHandler(LINK(this, RefEdit, UpdateHdl));
    }
    else
    {
        maIdle.ClearInvokeHandler();
        maIdle.Stop();
    }
}

void RefEdit::SetReferences(IControlReferenceHandler* pDlg, weld::Frame* pFrame)
{
    SetReferences(pDlg);

    if (pFrame)
        maGetLabelTextForShrinkModeFunc = [pFrame] { return pFrame->get_label(); };
}

Selection RefEdit::GetSelection() const
{
    int nStartPos, nEndPos;
    if (mxEntry->get_selection_bounds(nStartPos, nEndPos))
        return Selection(nStartPos, nEndPos);

    return Selection();
}

OUString RefEdit::GetLabelTextForShrinkMode()
{
    if (maGetLabelTextForShrinkModeFunc)
        return maGetLabelTextForShrinkModeFunc();

    return OUString();
}

IMPL_LINK_NOARG(RefEdit, Modify, weld::TextWidget&, void)
{
    maModifyHdl.Call(*this);
    if (mpAnyRefDlg)
        mpAnyRefDlg->HideReference();
}

IMPL_LINK(RefEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return KeyInput(rKEvt);
}

bool RefEdit::KeyInput(const KeyEvent& rKEvt)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if (mpAnyRefDlg && !rKeyCode.GetModifier() && rKeyCode.GetCode() == KEY_F2)
    {
        mpAnyRefDlg->ReleaseFocus(this);
        return true;
    }

    switch (rKeyCode.GetCode())
    {
        case KEY_RETURN:
        case KEY_ESCAPE:
            return maActivateHdl.Call(*GetWidget());
    }

    return false;
}

void RefEdit::GetFocus()
{
    maGetFocusHdl.Call(*this);
    StartUpdateData();
}

void RefEdit::LoseFocus()
{
    maLoseFocusHdl.Call(*this);
    if (mpAnyRefDlg)
        mpAnyRefDlg->HideReference();
}

IMPL_LINK_NOARG(RefEdit, GetFocusHdl, weld::Widget&, void)
{
    // in e.g. function wizard RefEdits we want to select all when we get focus
    // but in the gtk case there are pending gtk handlers which change selection
    // after our handler, so post our focus in event to happen after those complete
    if (mpFocusInEvent)
        Application::RemoveUserEvent(mpFocusInEvent);
    mpFocusInEvent = Application::PostUserEvent(LINK(this, RefEdit, AsyncFocusInHdl));
}

IMPL_LINK_NOARG(RefEdit, LoseFocusHdl, weld::Widget&, void)
{
    // tdf#127262 because focus in is async, focus out must not appear out
    // of sequence to focus in
    if (mpFocusOutEvent)
        Application::RemoveUserEvent(mpFocusOutEvent);
    mpFocusOutEvent = Application::PostUserEvent(LINK(this, RefEdit, AsyncFocusOutHdl));
}

IMPL_LINK_NOARG(RefEdit, AsyncFocusInHdl, void*, void)
{
    mpFocusInEvent = nullptr;
    GetFocus();
}

IMPL_LINK_NOARG(RefEdit, AsyncFocusOutHdl, void*, void)
{
    mpFocusOutEvent = nullptr;
    LoseFocus();
}

IMPL_LINK_NOARG(RefEdit, UpdateHdl, Timer *, void)
{
    if (mpAnyRefDlg)
        mpAnyRefDlg->ShowReference(mxEntry->get_text());
}

RefButton::RefButton(std::unique_ptr<weld::Button> xControl)
    : xButton(std::move(xControl))
    , pAnyRefDlg( nullptr )
    , pRefEdit( nullptr )
{
    xButton->connect_focus_in(LINK(this, RefButton, GetFocus));
    xButton->connect_focus_out(LINK(this, RefButton, LoseFocus));
    xButton->connect_key_press(LINK(this, RefButton, KeyInput));
    xButton->connect_clicked(LINK(this, RefButton, Click));
    SetStartImage();
}

RefButton::~RefButton()
{
}

void RefButton::SetStartImage()
{
    xButton->set_from_icon_name(RID_BMP_REFBTN1);
    xButton->set_tooltip_text(ForResId(RID_STR_SHRINK));
}

void RefButton::SetEndImage()
{
    xButton->set_from_icon_name(RID_BMP_REFBTN2);
    xButton->set_tooltip_text(ForResId(RID_STR_EXPAND));
}

void RefButton::SetReferences( IControlReferenceHandler* pDlg, RefEdit* pEdit )
{
    pAnyRefDlg = pDlg;
    pRefEdit = pEdit;
}

IMPL_LINK_NOARG(RefButton, Click, weld::Button&, void)
{
    maClickHdl.Call(*this);
    if( pAnyRefDlg )
        pAnyRefDlg->ToggleCollapsed( pRefEdit, this );
}

IMPL_LINK(RefButton, KeyInput, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if (pAnyRefDlg && !rKeyCode.GetModifier() && rKeyCode.GetCode() == KEY_F2)
    {
        pAnyRefDlg->ReleaseFocus( pRefEdit );
        return true;
    }

    switch (rKeyCode.GetCode())
    {
        case KEY_RETURN:
        case KEY_ESCAPE:
            return maActivateHdl.Call(*GetWidget());
    }

    return false;
}

IMPL_LINK_NOARG(RefButton, GetFocus, weld::Widget&, void)
{
    maGetFocusHdl.Call(*this);
    if (pRefEdit)
        pRefEdit->StartUpdateData();
}

IMPL_LINK_NOARG(RefButton, LoseFocus, weld::Widget&, void)
{
    maLoseFocusHdl.Call(*this);
    if (pRefEdit)
        pRefEdit->DoModify();
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
