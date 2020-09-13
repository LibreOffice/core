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
#pragma once

#include <memory>
#include <sfx2/basedlgs.hxx>
#include <svl/itemset.hxx>
#include <svx/zoom_def.hxx>
#include <vcl/weld.hxx>

class SvxZoomDialog : public SfxDialogController
{
private:
    const SfxItemSet&           m_rSet;
    std::unique_ptr<SfxItemSet> m_pOutSet;
    bool                        m_bModified;

    std::unique_ptr<weld::RadioButton>        m_xOptimalBtn;
    std::unique_ptr<weld::RadioButton>        m_xWholePageBtn;
    std::unique_ptr<weld::RadioButton>        m_xPageWidthBtn;
    std::unique_ptr<weld::RadioButton>        m_x100Btn;
    std::unique_ptr<weld::RadioButton>        m_xUserBtn;
    std::unique_ptr<weld::MetricSpinButton>   m_xUserEdit;
    std::unique_ptr<weld::Widget>             m_xViewFrame;
    std::unique_ptr<weld::RadioButton>        m_xAutomaticBtn;
    std::unique_ptr<weld::RadioButton>        m_xSingleBtn;
    std::unique_ptr<weld::RadioButton>        m_xColumnsBtn;
    std::unique_ptr<weld::SpinButton>         m_xColumnsEdit;
    std::unique_ptr<weld::CheckButton>        m_xBookModeChk;
    std::unique_ptr<weld::Button>             m_xOKBtn;

    DECL_LINK(UserHdl, weld::ToggleButton&, void);
    DECL_LINK(SpinHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ViewLayoutUserHdl, weld::ToggleButton&, void);
    DECL_LINK(ViewLayoutSpinHdl, weld::SpinButton&, void);
    DECL_LINK(ViewLayoutCheckHdl, weld::ToggleButton&, void);
    DECL_LINK(OKHdl, weld::Button&, void);

public:
    SvxZoomDialog(weld::Window* pParent, const SfxItemSet& rCoreSet);

    const SfxItemSet* GetOutputItemSet() const;

    sal_uInt16 GetFactor() const;
    void       SetFactor(sal_uInt16 nNewFactor, ZoomButtonId nButtonId = ZoomButtonId::NONE);

    void HideButton(ZoomButtonId nButtonId);
    void SetLimits(sal_uInt16 nMin, sal_uInt16 nMax);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
