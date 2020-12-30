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

#include <sfx2/basedlgs.hxx>
#include <tblafmt.hxx>

class VclContainer;
class SwTableAutoFormat;
class SwView;
class SwWrtShell;
struct SwInsertTableOptions;

class SwConvertTableDlg : public SfxDialogController
{
    std::unique_ptr<weld::RadioButton> m_xTabBtn;
    std::unique_ptr<weld::RadioButton> m_xSemiBtn;
    std::unique_ptr<weld::RadioButton> m_xParaBtn;
    std::unique_ptr<weld::RadioButton> m_xOtherBtn;
    std::unique_ptr<weld::Entry> m_xOtherEd;
    std::unique_ptr<weld::CheckButton> m_xKeepColumn;

    std::unique_ptr<weld::Container> m_xOptions;

    std::unique_ptr<weld::CheckButton> m_xHeaderCB;
    std::unique_ptr<weld::CheckButton> m_xRepeatHeaderCB;

    std::unique_ptr<weld::Container> m_xRepeatRows;
    std::unique_ptr<weld::SpinButton> m_xRepeatHeaderNF;

    std::unique_ptr<weld::CheckButton> m_xDontSplitCB;
    std::unique_ptr<weld::Button> m_xAutoFormatBtn;

    std::unique_ptr<SwTableAutoFormat> mxTAutoFormat;
    SwWrtShell* pShell;

    DECL_LINK(AutoFormatHdl, weld::Button&, void);
    DECL_LINK(BtnHdl, weld::Button&, void);
    DECL_LINK(CheckBoxHdl, weld::Button&, void);
    DECL_LINK(RepeatHeaderCheckBoxHdl, weld::Button&, void);

public:
    SwConvertTableDlg(SwView& rView, bool bToTable);

    void GetValues(sal_Unicode& rDelim, SwInsertTableOptions& rInsTableOpts,
                   SwTableAutoFormat const*& prTAFormat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
