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
#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_ENVFMT_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_ENVFMT_HXX

#include <svtools/stdctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/group.hxx>

#include "envlop.hxx"

class SwTxtFmtColl;

class SwEnvFmtPage : public SfxTabPage
{
    MetricField*  m_pAddrLeftField;
    MetricField*  m_pAddrTopField;
    MenuButton*   m_pAddrEditButton;
    MetricField*  m_pSendLeftField;
    MetricField*  m_pSendTopField;
    MenuButton*   m_pSendEditButton;
    ListBox*      m_pSizeFormatBox;
    MetricField*  m_pSizeWidthField;
    MetricField*  m_pSizeHeightField;
    SwEnvPreview* m_pPreview;

    std::vector<sal_uInt16>  aIDs;

    SwEnvFmtPage(vcl::Window* pParent, const SfxItemSet& rSet);

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( EditHdl, MenuButton * );
    DECL_LINK(FormatHdl, void *);

    void SetMinMax();

    SfxItemSet  *GetCollItemSet(SwTxtFmtColl* pColl, bool bSender);

    SwEnvDlg    *GetParentSwEnvDlg() {return (SwEnvDlg*) GetParentDialog();}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
            void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rSet) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
