
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

#include <vcl/hackery.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <boost/optional.hpp>

class SwWrtShell;

class SwBreakDlg
{
    SwWrtShell     &rSh;
    std::unique_ptr<Hackery::Builder> m_xBuilder;
    std::unique_ptr<Hackery::Dialog> m_xDialog;
    std::unique_ptr<Hackery::RadioButton> m_xLineBtn;
    std::unique_ptr<Hackery::RadioButton> m_xColumnBtn;
    std::unique_ptr<Hackery::RadioButton> m_xPageBtn;
    std::unique_ptr<Hackery::Label> m_xPageCollText;
    std::unique_ptr<Hackery::ComboBoxText> m_xPageCollBox;
    std::unique_ptr<Hackery::CheckButton> m_xPageNumBox;
    std::unique_ptr<Hackery::SpinButton> m_xPageNumEdit;
    std::unique_ptr<Hackery::Button> m_xOkBtn;

    OUString        aTemplate;
    sal_uInt16      nKind;
    ::boost::optional<sal_uInt16>      oPgNum;

    bool            bHtmlMode;

    DECL_LINK(ToggleHdl, Hackery::RadioButton&, void);
    DECL_LINK(ChangeHdl, Hackery::ComboBoxText&, void);
    DECL_LINK(PageNumHdl, Hackery::CheckButton&, void);
    DECL_LINK(PageNumModifyHdl, Hackery::SpinButton&, void);
    DECL_LINK(OkHdl, Hackery::Button&, void);

    void CheckEnable();

public:
    SwBreakDlg(vcl::Window *pParent, SwWrtShell &rSh);
    short Execute();
    const OUString& GetTemplateName() const { return aTemplate; }
    sal_uInt16 GetKind() const { return nKind; }
    const ::boost::optional<sal_uInt16>&  GetPageNumber() const { return oPgNum; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
