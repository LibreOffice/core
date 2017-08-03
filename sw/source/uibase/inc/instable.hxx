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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_INSTABLE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_INSTABLE_HXX

#include <rtl/ustring.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/weld.hxx>
#include <vcl/layout.hxx>
#include <sfx2/basedlgs.hxx>
#include <actctrl.hxx>
#include <tblafmt.hxx>

class SwWrtShell;
class SwView;
struct SwInsertTableOptions;

class SwInsTableDlg
{
    std::unique_ptr<Weld::Builder> m_xBuilder;
    std::unique_ptr<Weld::Dialog> m_xDialog;
    std::unique_ptr<Weld::Entry> m_xNameEdit;
    TextFilter      m_aTextFilter;

    std::unique_ptr<Weld::SpinButton> m_xColNF;
    std::unique_ptr<Weld::SpinButton> m_xRowNF;

    std::unique_ptr<Weld::CheckButton> m_xHeaderCB;
    std::unique_ptr<Weld::CheckButton> m_xRepeatHeaderCB;
    std::unique_ptr<Weld::SpinButton> m_xRepeatHeaderNF;
    std::unique_ptr<Weld::Widget> m_xRepeatGroup;

    std::unique_ptr<Weld::CheckButton> m_xDontSplitCB;
    std::unique_ptr<Weld::CheckButton> m_xBorderCB;

    std::unique_ptr<Weld::Button> m_xInsertBtn;
    std::unique_ptr<Weld::Button> m_xAutoFormatBtn;

    SwWrtShell*     pShell;
    std::unique_ptr<SwTableAutoFormat> m_xTAutoFormat;
    sal_Int64       nEnteredValRepeatHeaderNF;

    DECL_LINK(ModifyName, Weld::Entry&, void);
    DECL_LINK(ModifyRowCol, Weld::SpinButton&, void);
    DECL_LINK(AutoFormatHdl, Weld::Button&, void);
    DECL_LINK(OKHdl, Weld::Button&, void);
    DECL_LINK(CheckBoxHdl, Weld::ToggleButton&, void);
    DECL_LINK(RepeatHeaderCheckBoxHdl, Weld::ToggleButton&, void);
    DECL_LINK(ModifyRepeatHeaderNF_Hdl, Weld::SpinButton&, void);
    DECL_LINK(TextFilterHdl, OUString&, bool);

public:
    SwInsTableDlg(SwView& rView);

    void GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTableOpts, OUString& rTableAutoFormatName,
                    SwTableAutoFormat *& prTAFormat );
    short Execute()
    {
        short nRet = m_xDialog->run();
        m_xDialog->hide();
        return nRet;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
