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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWUIPARDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWUIPARDLG_HXX
#include "pardlg.hxx"

class SwParaDlg: public SfxTabDialog
{
    SwView& rView;
    sal_uInt16 nHtmlMode;
    sal_uInt8 nDlgMode;
    bool bDrawParaDlg;

    sal_uInt16 m_nParaStd;
    sal_uInt16 m_nParaAlign;
    sal_uInt16 m_nParaExt;
    sal_uInt16 m_nParaNumPara;
    sal_uInt16 m_nParaDrpCps;
    sal_uInt16 m_nParaBckGrnd;
    sal_uInt16 m_nParaBorder;
    sal_uInt16 m_nAreaId;
    sal_uInt16 m_nTransparenceId;

    void PageCreated(sal_uInt16 nId, SfxTabPage& rPage) override;

public:
    SwParaDlg(  vcl::Window *pParent,
                SwView& rVw,
                const SfxItemSet&,
                sal_uInt8 nDialogMode,
                const OUString *pCollName = 0,
                bool bDraw = false,
                const OString& sDefPage = OString());
    virtual ~SwParaDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
