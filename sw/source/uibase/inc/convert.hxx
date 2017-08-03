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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONVERT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONVERT_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/weld.hxx>
#include <vcl/edit.hxx>
#include <sfx2/basedlgs.hxx>

class SwTableAutoFormat;
class SwView;
class SwWrtShell;
class NumericField;
struct SwInsertTableOptions;

class SwConvertTableDlg
{
    std::unique_ptr<Weld::Builder> m_xBuilder;
    std::unique_ptr<Weld::Dialog> m_xDialog;
    std::unique_ptr<Weld::RadioButton> m_xTabBtn;
    std::unique_ptr<Weld::RadioButton> m_xSemiBtn;
    std::unique_ptr<Weld::RadioButton> m_xParaBtn;
    std::unique_ptr<Weld::RadioButton> m_xOtherBtn;
    std::unique_ptr<Weld::Entry> m_xOtherEd;
    std::unique_ptr<Weld::CheckButton> m_xKeepColumn;

    std::unique_ptr<Weld::Widget> m_xOptions;

    std::unique_ptr<Weld::CheckButton> m_xHeaderCB;
    std::unique_ptr<Weld::CheckButton> m_xRepeatHeaderCB;

    std::unique_ptr<Weld::Widget> m_xRepeatRows;
    std::unique_ptr<Weld::SpinButton> m_xRepeatHeaderNF;

    std::unique_ptr<Weld::CheckButton> m_xDontSplitCB;
    std::unique_ptr<Weld::CheckButton> m_xBorderCB;
    std::unique_ptr<Weld::Button> m_xAutoFormatBtn;

    OUString        sConvertTextTable;
    std::unique_ptr<SwTableAutoFormat> mxTAutoFormat;
    SwWrtShell*     pShell;

    DECL_LINK(AutoFormatHdl, Weld::Button&, void);
    DECL_LINK(BtnHdl, Weld::ToggleButton&, void);
    DECL_LINK(CheckBoxHdl, Weld::ToggleButton&, void);
    DECL_LINK(RepeatHeaderCheckBoxHdl, Weld::ToggleButton&, void);

public:
    SwConvertTableDlg(SwView& rView, bool bToTable);
    short Execute()
    {
        short nRet = m_xDialog->run();
        m_xDialog->hide();
        return nRet;
    }
    void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTableOpts,
                    SwTableAutoFormat const*& prTAFormat );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
