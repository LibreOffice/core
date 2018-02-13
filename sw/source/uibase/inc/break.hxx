
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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_BREAK_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_BREAK_HXX

#include <vcl/weld.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <boost/optional.hpp>

class SwWrtShell;

class SwBreakDlg
{
    SwWrtShell     &rSh;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Dialog> m_xDialog;
    std::unique_ptr<weld::RadioButton> m_xLineBtn;
    std::unique_ptr<weld::RadioButton> m_xColumnBtn;
    std::unique_ptr<weld::RadioButton> m_xPageBtn;
    std::unique_ptr<weld::Label> m_xPageCollText;
    std::unique_ptr<weld::ComboBoxText> m_xPageCollBox;
    std::unique_ptr<weld::CheckButton> m_xPageNumBox;
    std::unique_ptr<weld::SpinButton> m_xPageNumEdit;
    std::unique_ptr<weld::Button> m_xOkBtn;

    OUString        aTemplate;
    sal_uInt16      nKind;
    ::boost::optional<sal_uInt16>      oPgNum;

    bool            bHtmlMode;

    DECL_LINK(ToggleHdl, weld::ToggleButton&, void);
    DECL_LINK(ChangeHdl, weld::ComboBoxText&, void);
    DECL_LINK(PageNumHdl, weld::ToggleButton&, void);
    DECL_LINK(PageNumModifyHdl, weld::SpinButton&, void);
    DECL_LINK(OkHdl, weld::Button&, void);

    void CheckEnable();

public:
    SwBreakDlg(weld::Window *pParent, SwWrtShell &rSh);
    short Execute();
    const OUString& GetTemplateName() const { return aTemplate; }
    sal_uInt16 GetKind() const { return nKind; }
    const ::boost::optional<sal_uInt16>&  GetPageNumber() const { return oPgNum; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
