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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_LINENUM_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_LINENUM_HXX

#include <sfx2/basedlgs.hxx>
#include "numberingtypelistbox.hxx"

class SwView;
class SwWrtShell;

class SwLineNumberingDlg : public SfxDialogController
{
private:
    SwWrtShell* m_pSh;
    std::unique_ptr<weld::Widget> m_xBodyContent;
    std::unique_ptr<weld::Widget> m_xDivIntervalFT;
    std::unique_ptr<weld::SpinButton> m_xDivIntervalNF;
    std::unique_ptr<weld::Widget> m_xDivRowsFT;
    std::unique_ptr<weld::SpinButton> m_xNumIntervalNF;
    std::unique_ptr<weld::ComboBox> m_xCharStyleLB;
    std::unique_ptr<SwNumberingTypeListBox> m_xFormatLB;
    std::unique_ptr<weld::ComboBox> m_xPosLB;
    std::unique_ptr<weld::MetricSpinButton> m_xOffsetMF;
    std::unique_ptr<weld::Entry> m_xDivisorED;
    std::unique_ptr<weld::CheckButton> m_xCountEmptyLinesCB;
    std::unique_ptr<weld::CheckButton> m_xCountFrameLinesCB;
    std::unique_ptr<weld::CheckButton> m_xRestartEachPageCB;
    std::unique_ptr<weld::CheckButton> m_xNumberingOnCB;
    std::unique_ptr<weld::CheckButton> m_xNumberingOnFooterHeader;
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Widget> m_xNumIntervalFT;
    std::unique_ptr<weld::Widget> m_xNumRowsFT;
    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(LineOnOffHdl, weld::Button&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);

public:
    SwLineNumberingDlg(const SwView& rVw);
    virtual ~SwLineNumberingDlg() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
