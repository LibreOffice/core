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
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <numberingtypelistbox.hxx>

namespace vcl { class Window; }
class SwView;
class SwWrtShell;

class SwLineNumberingDlg : public SfxModalDialog
{
private:
    SwWrtShell* pSh;
    VclPtr<VclContainer> m_pBodyContent;
    VclPtr<vcl::Window> m_pDivIntervalFT;
    VclPtr<NumericField> m_pDivIntervalNF;
    VclPtr<vcl::Window> m_pDivRowsFT;
    VclPtr<NumericField> m_pNumIntervalNF;
    VclPtr<ListBox> m_pCharStyleLB;
    VclPtr<SwNumberingTypeListBox> m_pFormatLB;
    VclPtr<ListBox> m_pPosLB;
    VclPtr<MetricField> m_pOffsetMF;
    VclPtr<Edit> m_pDivisorED;
    VclPtr<CheckBox> m_pCountEmptyLinesCB;
    VclPtr<CheckBox> m_pCountFrameLinesCB;
    VclPtr<CheckBox> m_pRestartEachPageCB;
    VclPtr<CheckBox> m_pNumberingOnCB;
    VclPtr<CheckBox> m_pNumberingOnFooterHeader;
    DECL_LINK_TYPED(OKHdl, Button*, void);
    DECL_LINK_TYPED(LineOnOffHdl, Button* = 0, void);
    DECL_LINK(ModifyHdl, void * = 0);

public:
    SwLineNumberingDlg(SwView *pVw);
    virtual ~SwLineNumberingDlg();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
