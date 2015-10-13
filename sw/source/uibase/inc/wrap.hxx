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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_WRAP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_WRAP_HXX

#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

namespace vcl { class Window; }
class SfxItemSet;
class SwWrtShell;

class SwWrapDlg : public SfxSingleTabDialog
{
    SwWrtShell*         pWrtShell;

public:
    SwWrapDlg(vcl::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, bool bDrawMode);
};

// circulation TabPage
class SwWrapTabPage: public SfxTabPage
{
    // WRAPPING
    VclPtr<RadioButton>   m_pNoWrapRB;
    VclPtr<RadioButton>   m_pWrapLeftRB;
    VclPtr<RadioButton>   m_pWrapRightRB;
    VclPtr<RadioButton>   m_pWrapParallelRB;
    VclPtr<RadioButton>   m_pWrapThroughRB;
    VclPtr<RadioButton>   m_pIdealWrapRB;

    // MARGIN
    VclPtr<MetricField>   m_pLeftMarginED;
    VclPtr<MetricField>   m_pRightMarginED;
    VclPtr<MetricField>   m_pTopMarginED;
    VclPtr<MetricField>   m_pBottomMarginED;

    // OPTIONS
    VclPtr<CheckBox>      m_pWrapAnchorOnlyCB;
    VclPtr<CheckBox>      m_pWrapTransparentCB;
    VclPtr<CheckBox>      m_pWrapOutlineCB;
    VclPtr<CheckBox>      m_pWrapOutsideCB;

    sal_uInt16              m_nOldLeftMargin;
    sal_uInt16              m_nOldRightMargin;
    sal_uInt16              m_nOldUpperMargin;
    sal_uInt16              m_nOldLowerMargin;

    RndStdIds           m_nAnchorId;
    sal_uInt16              m_nHtmlMode;

    Size m_aFrmSize;
    SwWrtShell*         m_pWrtSh;

    bool m_bFormat;
    bool m_bNew;
    bool m_bHtmlMode;
    bool m_bDrawMode;
    bool m_bContourImage;

    virtual ~SwWrapTabPage();
    virtual void dispose() override;

    void            ApplyImageList();
    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual sfxpg   DeactivatePage(SfxItemSet *pSet) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    DECL_LINK_TYPED( RangeModifyHdl, SpinField&, void );
    DECL_LINK_TYPED( RangeLoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( WrapTypeHdl, Button *, void );
    DECL_LINK_TYPED( ContourHdl, Button *, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    static const sal_uInt16 m_aWrapPageRg[];

public:
    SwWrapTabPage(vcl::Window *pParent, const SfxItemSet &rSet);

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);

    virtual bool    FillItemSet(SfxItemSet *rSet) override;
    virtual void    Reset(const SfxItemSet *rSet) override;

    static const sal_uInt16* GetRanges() { return m_aWrapPageRg; }
    inline void     SetNewFrame(bool bNewFrame) { m_bNew = bNewFrame; }
    inline void     SetFormatUsed(bool bFormat, bool bDrawMode) { m_bFormat = bFormat; m_bDrawMode = bDrawMode; }
    inline void     SetShell(SwWrtShell* pSh) { m_pWrtSh = pSh; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
