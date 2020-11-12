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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SRTDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SRTDLG_HXX

#include <svtools/collatorres.hxx>
#include <svx/langbox.hxx>
#include <vcl/weld.hxx>

class SwWrtShell;

class SwSortDlg : public weld::GenericDialogController
{
    weld::Window* m_pParent;
    std::unique_ptr<weld::Label> m_xColLbl;

    std::unique_ptr<weld::CheckButton> m_xKeyCB1;
    std::unique_ptr<weld::SpinButton> m_xColEdt1;
    std::unique_ptr<weld::ComboBox> m_xTypDLB1;
    std::unique_ptr<weld::RadioButton> m_xSortUp1RB;
    std::unique_ptr<weld::RadioButton> m_xSortDn1RB;

    std::unique_ptr<weld::CheckButton> m_xKeyCB2;
    std::unique_ptr<weld::SpinButton> m_xColEdt2;
    std::unique_ptr<weld::ComboBox> m_xTypDLB2;
    std::unique_ptr<weld::RadioButton> m_xSortUp2RB;
    std::unique_ptr<weld::RadioButton> m_xSortDn2RB;

    std::unique_ptr<weld::CheckButton> m_xKeyCB3;
    std::unique_ptr<weld::SpinButton> m_xColEdt3;
    std::unique_ptr<weld::ComboBox> m_xTypDLB3;
    std::unique_ptr<weld::RadioButton> m_xSortUp3RB;
    std::unique_ptr<weld::RadioButton> m_xSortDn3RB;

    std::unique_ptr<weld::RadioButton> m_xColumnRB;
    std::unique_ptr<weld::RadioButton> m_xRowRB;

    std::unique_ptr<weld::RadioButton> m_xDelimTabRB;
    std::unique_ptr<weld::RadioButton> m_xDelimFreeRB;
    std::unique_ptr<weld::Entry> m_xDelimEdt;
    std::unique_ptr<weld::Button> m_xDelimPB;

    std::unique_ptr<SvxLanguageBox> m_xLangLB;

    std::unique_ptr<weld::CheckButton> m_xCaseCB;

    OUString aColText;
    OUString aRowText;
    OUString aNumericText;

    SwWrtShell& rSh;
    std::unique_ptr<CollatorResource> m_xColRes;

    sal_uInt16 nX;
    sal_uInt16 nY;

    void Apply();
    sal_Unicode GetDelimChar() const;

    DECL_LINK(CheckHdl, weld::ToggleButton&, void);
    DECL_LINK(DelimHdl, weld::ToggleButton&, void);
    DECL_LINK(LanguageListBoxHdl, weld::ComboBox&, void);
    void LanguageHdl(weld::ComboBox const*);
    DECL_LINK(DelimCharHdl, weld::Button&, void);

public:
    SwSortDlg(weld::Window* pParent, SwWrtShell& rSh);
    virtual short run() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
