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

#include <tools/lineend.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <runtime.hxx>
#include <stdobj.hxx>
#include <rtlproto.hxx>
#include <memory>

class SvRTLInputBox : public weld::GenericDialogController
{
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::Button> m_xCancel;
    std::unique_ptr<weld::Label> m_xPromptText;
    OUString m_aText;

    void PositionDialog( long nXTwips, long nYTwips );
    void InitButtons();
    void SetPrompt(const OUString& rPrompt);
    DECL_LINK( OkHdl, weld::Button&, void );
    DECL_LINK( CancelHdl, weld::Button&, void );

public:
    SvRTLInputBox(weld::Window* pParent, const OUString& rPrompt, const OUString& rTitle,
        const OUString& rDefault, long nXTwips, long nYTwips );
    OUString const & GetText() const { return m_aText; }
};

SvRTLInputBox::SvRTLInputBox(weld::Window* pParent, const OUString& rPrompt,
        const OUString& rTitle, const OUString& rDefault,
        long nXTwips, long nYTwips)
    : GenericDialogController(pParent, "svt/ui/inputbox.ui", "InputBox")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xCancel(m_xBuilder->weld_button("cancel"))
    , m_xPromptText(m_xBuilder->weld_label("prompt"))
{
    PositionDialog( nXTwips, nYTwips );
    InitButtons();
    SetPrompt(rPrompt);
    m_xDialog->set_title(rTitle);
    m_xEdit->set_text(rDefault);
    m_xEdit->select_region(0, -1);
}

void SvRTLInputBox::InitButtons()
{
    m_xOk->connect_clicked(LINK(this,SvRTLInputBox, OkHdl));
    m_xCancel->connect_clicked(LINK(this,SvRTLInputBox,CancelHdl));
}

void SvRTLInputBox::PositionDialog(long nXTwips, long nYTwips)
{
    if( nXTwips != -1 && nYTwips != -1 )
    {
        Point aDlgPosApp( nXTwips, nYTwips );
        OutputDevice* pDefaultDevice = Application::GetDefaultDevice();
        pDefaultDevice->Push(PushFlags::MAPMODE);
        pDefaultDevice->SetMapMode(MapMode( MapUnit::MapAppFont));
        aDlgPosApp = pDefaultDevice->LogicToPixel(aDlgPosApp, MapMode(MapUnit::MapTwip));
        pDefaultDevice->Pop();
        m_xDialog->window_move(aDlgPosApp.X(), aDlgPosApp.Y());
    }
}

void SvRTLInputBox::SetPrompt(const OUString& rPrompt)
{
    if (rPrompt.isEmpty())
        return;
    OUString aText_(convertLineEnd(rPrompt, LINEEND_CR));
    m_xPromptText->set_label( aText_ );
}

IMPL_LINK_NOARG( SvRTLInputBox, OkHdl, weld::Button&, void )
{
    m_aText = m_xEdit->get_text();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG( SvRTLInputBox, CancelHdl, weld::Button&, void )
{
    m_aText.clear();
    m_xDialog->response(RET_CANCEL);
}

// Syntax: String InputBox( Prompt, [Title], [Default] [, nXpos, nYpos ] )

void SbRtl_InputBox(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count();
    if ( nArgCount < 2 )
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    else
    {
        OUString aTitle;
        OUString aDefault;
        sal_Int32 nX = -1, nY = -1;  // center
        const OUString& rPrompt = rPar.Get(1)->GetOUString();
        if ( nArgCount > 2 && !rPar.Get(2)->IsErr() )
            aTitle = rPar.Get(2)->GetOUString();
        if ( nArgCount > 3 && !rPar.Get(3)->IsErr() )
            aDefault = rPar.Get(3)->GetOUString();
        if ( nArgCount > 4 )
        {
            if ( nArgCount != 6 )
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                return;
            }
            nX = rPar.Get(4)->GetLong();
            nY = rPar.Get(5)->GetLong();
        }
        vcl::Window* pParent = Application::GetDefDialogParent();
        SvRTLInputBox aDlg(pParent ? pParent->GetFrameWeld() : nullptr,rPrompt,aTitle,aDefault,nX,nY);
        aDlg.run();
        rPar.Get(0)->PutString(aDlg.GetText());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
