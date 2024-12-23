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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_INSFNOTE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_INSFNOTE_HXX

#include <vcl/weld.hxx>

class SwWrtShell;

class VclFrame;

class SwInsFootNoteDlg final : public weld::GenericDialogController
{
    SwWrtShell     &m_rSh;

    // everything for the character(s)
    OUString        m_aFontName;
    rtl_TextEncoding m_eCharSet;
    bool        m_bExtCharAvailable;
    bool        m_bEdit;

    std::unique_ptr<weld::Widget>     m_xNumberFrame;
    std::unique_ptr<weld::RadioButton>    m_xNumberAutoBtn;
    std::unique_ptr<weld::RadioButton>    m_xNumberCharBtn;
    std::unique_ptr<weld::Entry>      m_xNumberCharEdit;
    std::unique_ptr<weld::Button>     m_xNumberExtChar;

    // everything for the selection footnote/endnote
    std::unique_ptr<weld::RadioButton>    m_xFootnoteBtn;
    std::unique_ptr<weld::RadioButton>    m_xEndNoteBtn;

    std::unique_ptr<weld::Button>     m_xOkBtn;
    std::unique_ptr<weld::Button>     m_xPrevBT;
    std::unique_ptr<weld::Button>     m_xNextBT;

    DECL_LINK(NumberToggleHdl, weld::Toggleable&, void);
    DECL_LINK(NumberEditHdl, weld::Entry&, void);
    DECL_LINK(NumberExtCharHdl, weld::Button&, void);
    DECL_LINK(NextPrevHdl, weld::Button&, void);

    void            Init();

    void ImplDestroy();

public:
    SwInsFootNoteDlg(weld::Window * pParent, SwWrtShell &rSh, bool bEd);
    virtual ~SwInsFootNoteDlg() override;

    const OUString& GetFontName() const { return m_aFontName; }
    bool            IsEndNote() const { return m_xEndNoteBtn->get_active(); }
    OUString        GetStr() const
    {
        if (m_xNumberCharBtn->get_active())
            return m_xNumberCharEdit->get_text();
        return OUString();
    }
    void    Apply();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
