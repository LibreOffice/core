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

#include <vcl/weld.hxx>
#include <envlop.hxx>

class SwTextFormatColl;

class SwEnvFormatPage : public SfxTabPage
{
    SwEnvDlg* m_pDialog;
    std::vector<sal_uInt16> m_aIDs;

    SwEnvPreview m_aPreview;
    std::unique_ptr<weld::MetricSpinButton> m_xAddrLeftField;
    std::unique_ptr<weld::MetricSpinButton> m_xAddrTopField;
    std::unique_ptr<weld::MenuButton> m_xAddrEditButton;
    std::unique_ptr<weld::MetricSpinButton> m_xSendLeftField;
    std::unique_ptr<weld::MetricSpinButton> m_xSendTopField;
    std::unique_ptr<weld::MenuButton> m_xSendEditButton;
    std::unique_ptr<weld::ComboBox> m_xSizeFormatBox;
    std::unique_ptr<weld::MetricSpinButton> m_xSizeWidthField;
    std::unique_ptr<weld::MetricSpinButton> m_xSizeHeightField;
    std::unique_ptr<weld::CustomWeld> m_xPreview;

    DECL_LINK(ModifyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(AddrEditHdl, const OString&, void);
    DECL_LINK(SendEditHdl, const OString&, void);
    DECL_LINK(FormatHdl, weld::ComboBox&, void);

    void SetMinMax();

    SfxItemSet* GetCollItemSet(SwTextFormatColl const* pColl, bool bSender);

    void Edit(const OString& rIdent, bool bSender);

    SwEnvDlg* GetParentSwEnvDlg() { return m_pDialog; }

public:
    SwEnvFormatPage(weld::Container* pPage, weld::DialogController* pController,
                    const SfxItemSet& rSet);
    void Init(SwEnvDlg* pDialog);
    virtual ~SwEnvFormatPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
