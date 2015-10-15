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

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

class SwWrtShell;

class VclFrame;

class SwInsFootNoteDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;

    // everything for the character(s)
    OUString        m_aFontName;
    rtl_TextEncoding eCharSet;
    bool        bExtCharAvailable;
    bool        bEdit;

    VclPtr<VclFrame>       m_pNumberFrame;
    VclPtr<RadioButton>    m_pNumberAutoBtn;
    VclPtr<RadioButton>    m_pNumberCharBtn;
    VclPtr<Edit>           m_pNumberCharEdit;
    VclPtr<PushButton>     m_pNumberExtChar;

    // everything for the selection footnote/endnote
    VclPtr<RadioButton>    m_pFootnoteBtn;
    VclPtr<RadioButton>    m_pEndNoteBtn;

    VclPtr<PushButton>     m_pOkBtn;
    VclPtr<PushButton>     m_pPrevBT;
    VclPtr<PushButton>     m_pNextBT;

    DECL_LINK_TYPED(NumberCharHdl, Button *, void);
    DECL_LINK_TYPED(NumberEditHdl, Edit&, void);
    DECL_LINK_TYPED(NumberAutoBtnHdl, Button *, void);
    DECL_LINK_TYPED(NumberExtCharHdl, Button *, void);
    DECL_LINK_TYPED(NextPrevHdl, Button *, void);

    virtual void    Apply() override;

    void            Init();

public:
    SwInsFootNoteDlg(vcl::Window * pParent, SwWrtShell &rSh, bool bEd = false);
    virtual ~SwInsFootNoteDlg();
    virtual void dispose() override;

    OUString        GetFontName() { return m_aFontName; }
    bool            IsEndNote() { return m_pEndNoteBtn->IsChecked(); }
    OUString        GetStr()
                    {
                        if ( m_pNumberCharBtn->IsChecked() )
                            return m_pNumberCharEdit->GetText();
                        return OUString();
                    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
