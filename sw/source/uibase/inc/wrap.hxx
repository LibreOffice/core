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

#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/swframetypes.hxx>

class SfxItemSet;
class SwWrtShell;

class SwWrapDlg final : public SfxSingleTabDialogController
{
public:
    SwWrapDlg(weld::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, bool bDrawMode);
};

// circulation TabPage
class SwWrapTabPage final : public SfxTabPage
{
    RndStdIds             m_nAnchorId;
    sal_uInt16            m_nHtmlMode;

    SwWrtShell*           m_pWrtSh;

    bool m_bFormat;
    bool m_bNew;
    bool m_bHtmlMode;
    bool m_bDrawMode;
    bool m_bContourImage;

    // WRAPPING
    std::unique_ptr<weld::Image> m_xNoWrapImg;
    std::unique_ptr<weld::RadioButton> m_xNoWrapRB;
    std::unique_ptr<weld::Image> m_xWrapLeftImg;
    std::unique_ptr<weld::RadioButton> m_xWrapLeftRB;
    std::unique_ptr<weld::Image> m_xWrapRightImg;
    std::unique_ptr<weld::RadioButton> m_xWrapRightRB;
    std::unique_ptr<weld::Image> m_xWrapParallelImg;
    std::unique_ptr<weld::RadioButton> m_xWrapParallelRB;
    std::unique_ptr<weld::Image> m_xWrapThroughImg;
    std::unique_ptr<weld::RadioButton> m_xWrapThroughRB;
    std::unique_ptr<weld::Image> m_xIdealWrapImg;
    std::unique_ptr<weld::RadioButton> m_xIdealWrapRB;

    // MARGIN
    std::unique_ptr<weld::MetricSpinButton> m_xLeftMarginED;
    std::unique_ptr<weld::MetricSpinButton> m_xRightMarginED;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMarginED;
    std::unique_ptr<weld::MetricSpinButton> m_xBottomMarginED;

    // OPTIONS
    std::unique_ptr<weld::CheckButton> m_xWrapAnchorOnlyCB;
    std::unique_ptr<weld::CheckButton> m_xWrapTransparentCB;
    std::unique_ptr<weld::CheckButton> m_xWrapOutlineCB;
    std::unique_ptr<weld::CheckButton> m_xWrapOutsideCB;
    std::unique_ptr<weld::CheckButton> m_xAllowOverlapCB;

    void            SetImages();
    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    DECL_LINK(RangeModifyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(WrapTypeHdl, weld::Toggleable&, void);
    DECL_LINK(ContourHdl, weld::Toggleable&, void);

    static const WhichRangesContainer m_aWrapPageRg;

public:
    SwWrapTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    virtual ~SwWrapTabPage() override;

    virtual bool    FillItemSet(SfxItemSet *rSet) override;
    virtual void    Reset(const SfxItemSet *rSet) override;

    static WhichRangesContainer GetRanges() { return m_aWrapPageRg; }
    void     SetNewFrame(bool bNewFrame) { m_bNew = bNewFrame; }
    void     SetFormatUsed(bool bFormat, bool bDrawMode) { m_bFormat = bFormat; m_bDrawMode = bDrawMode; }
    void     SetShell(SwWrtShell* pSh) { m_pWrtSh = pSh; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
