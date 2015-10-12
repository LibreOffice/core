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
#ifndef INCLUDED_CUI_SOURCE_INC_ZOOM_HXX
#define INCLUDED_CUI_SOURCE_INC_ZOOM_HXX

#include <sfx2/basedlgs.hxx>
#include <svx/zoom_def.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

class SvxZoomDialog : public SfxModalDialog
{
private:
    VclPtr<RadioButton>        m_pOptimalBtn;
    VclPtr<RadioButton>        m_pWholePageBtn;
    VclPtr<RadioButton>        m_pPageWidthBtn;
    VclPtr<RadioButton>        m_p100Btn;
    VclPtr<RadioButton>        m_pUserBtn;
    VclPtr<MetricField>        m_pUserEdit;

    VclPtr<VclContainer>       m_pViewFrame;
    VclPtr<RadioButton>        m_pAutomaticBtn;
    VclPtr<RadioButton>        m_pSingleBtn;
    VclPtr<RadioButton>        m_pColumnsBtn;
    VclPtr<NumericField>       m_pColumnsEdit;
    VclPtr<CheckBox>           m_pBookModeChk;

    VclPtr<OKButton>           m_pOKBtn;

    const SfxItemSet&           mrSet;
    std::unique_ptr<SfxItemSet> mpOutSet;
    bool                        mbModified;

    DECL_LINK_TYPED(UserHdl, Button*, void);
    DECL_LINK(SpinHdl, void*);
    DECL_LINK_TYPED(ViewLayoutUserHdl, Button*, void);
    DECL_LINK(ViewLayoutSpinHdl, NumericField*);
    DECL_LINK_TYPED(ViewLayoutCheckHdl, Button*, void);
    DECL_LINK_TYPED(OKHdl, Button*, void);

public:
    SvxZoomDialog(vcl::Window* pParent, const SfxItemSet& rCoreSet);
    virtual ~SvxZoomDialog();
    virtual void dispose() override;

    const SfxItemSet* GetOutputItemSet() const;

    sal_uInt16 GetFactor() const;
    void       SetFactor(sal_uInt16 nNewFactor, ZoomButtonId nButtonId = ZoomButtonId::NONE);

    void HideButton(ZoomButtonId nButtonId);
    void SetLimits(sal_uInt16 nMin, sal_uInt16 nMax);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
