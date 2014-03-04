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
#ifndef INCLUDED_SW_SOURCE_UI_INC_LINENUM_HXX
#define INCLUDED_SW_SOURCE_UI_INC_LINENUM_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <numberingtypelistbox.hxx>

class Window;
class SwView;
class SwWrtShell;

class SwLineNumberingDlg : public SfxModalDialog
{
private:
    SwWrtShell* pSh;
    VclContainer* m_pBodyContent;
    Window* m_pDivIntervalFT;
    NumericField* m_pDivIntervalNF;
    Window* m_pDivRowsFT;
    NumericField* m_pNumIntervalNF;
    ListBox* m_pCharStyleLB;
    SwNumberingTypeListBox* m_pFormatLB;
    ListBox* m_pPosLB;
    MetricField* m_pOffsetMF;
    Edit* m_pDivisorED;
    CheckBox* m_pCountEmptyLinesCB;
    CheckBox* m_pCountFrameLinesCB;
    CheckBox* m_pRestartEachPageCB;
    CheckBox* m_pNumberingOnCB;

    DECL_LINK(OKHdl, void *);
    DECL_LINK(LineOnOffHdl, void * = 0);
    DECL_LINK(ModifyHdl, void * = 0);

public:
    SwWrtShell* GetWrtShell() const { return pSh; }

    SwLineNumberingDlg(SwView *pVw);
    ~SwLineNumberingDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
