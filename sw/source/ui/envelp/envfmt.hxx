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

class SwTextFormatColl;

class SwEnvFormatPage : public SfxTabPage
{
    VclPtr<MetricField>  m_pAddrLeftField;
    VclPtr<MetricField>  m_pAddrTopField;
    VclPtr<MenuButton>   m_pAddrEditButton;
    VclPtr<MetricField>  m_pSendLeftField;
    VclPtr<MetricField>  m_pSendTopField;
    VclPtr<MenuButton>   m_pSendEditButton;
    VclPtr<ListBox>      m_pSizeFormatBox;
    VclPtr<MetricField>  m_pSizeWidthField;
    VclPtr<MetricField>  m_pSizeHeightField;
    VclPtr<SwEnvPreview> m_pPreview;

    std::vector<sal_uInt16>  aIDs;

    DECL_LINK_TYPED( ModifyHdl, SpinField&, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( EditHdl, MenuButton *, void );
    DECL_LINK_TYPED(FormatHdl, ListBox&, void);

    void SetMinMax();

    SfxItemSet  *GetCollItemSet(SwTextFormatColl* pColl, bool bSender);

    SwEnvDlg    *GetParentSwEnvDlg() {return static_cast<SwEnvDlg*>(GetParentDialog());}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwEnvFormatPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwEnvFormatPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
