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
#ifndef INSFNOTE_HXX
#define INSFNOTE_HXX

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
    CharSet         eCharSet;
    sal_Bool        bExtCharAvailable;
    sal_Bool        bEdit;

    VclFrame*       m_pNumberFrame;
    RadioButton*    m_pNumberAutoBtn;
    RadioButton*    m_pNumberCharBtn;
    Edit*           m_pNumberCharEdit;
    PushButton*     m_pNumberExtChar;

    // everything for the selection footnote/endnote
    RadioButton*    m_pFtnBtn;
    RadioButton*    m_pEndNoteBtn;

    PushButton*     m_pOkBtn;
    PushButton*     m_pPrevBT;
    PushButton*     m_pNextBT;

    DECL_LINK(NumberCharHdl, void *);
    DECL_LINK(NumberEditHdl, void *);
    DECL_LINK(NumberAutoBtnHdl, void *);
    DECL_LINK(NumberExtCharHdl, void *);
    DECL_LINK(NextPrevHdl, Button *);

    virtual void    Apply();

    void            Init();

public:
    SwInsFootNoteDlg(Window * pParent, SwWrtShell &rSh, sal_Bool bEd = sal_False);
    ~SwInsFootNoteDlg();

    CharSet         GetCharSet() { return eCharSet; }
    sal_Bool        IsExtCharAvailable() { return bExtCharAvailable; }
    OUString        GetFontName() { return m_aFontName; }
    sal_Bool        IsEndNote() { return m_pEndNoteBtn->IsChecked(); }
    OUString        GetStr()
                    {
                        if ( m_pNumberCharBtn->IsChecked() )
                            return m_pNumberCharEdit->GetText();
                        return OUString();
                    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
