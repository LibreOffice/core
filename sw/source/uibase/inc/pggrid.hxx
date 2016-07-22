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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_PGGRID_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_PGGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <colex.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <svtools/ctrlbox.hxx>

// TabPage Format/(Styles/)Page/Text grid
class SwTextGridPage: public SfxTabPage
{
    VclPtr<RadioButton>     m_pNoGridRB;
    VclPtr<RadioButton>     m_pLinesGridRB;
    VclPtr<RadioButton>     m_pCharsGridRB;
    VclPtr<CheckBox>        m_pSnapToCharsCB;

    VclPtr<SwPageGridExample>   m_pExampleWN;

    VclPtr<VclFrame>        m_pLayoutFL;
    VclPtr<NumericField>    m_pLinesPerPageNF;
    VclPtr<FixedText>       m_pLinesRangeFT;

    VclPtr<MetricField>     m_pTextSizeMF;

    VclPtr<FixedText>       m_pCharsPerLineFT;
    VclPtr<NumericField>    m_pCharsPerLineNF;
    VclPtr<FixedText>       m_pCharsRangeFT;

    VclPtr<FixedText>       m_pCharWidthFT;
    VclPtr<MetricField>     m_pCharWidthMF;

    VclPtr<FixedText>       m_pRubySizeFT;
    VclPtr<MetricField>     m_pRubySizeMF;

    VclPtr<CheckBox>        m_pRubyBelowCB;

    VclPtr<VclFrame>        m_pDisplayFL;

    VclPtr<CheckBox>        m_pDisplayCB;
    VclPtr<CheckBox>        m_pPrintCB;
    VclPtr<ColorListBox>    m_pColorLB;

    sal_Int32       m_nRubyUserValue;
    bool        m_bRubyUserValue;
    Size            m_aPageSize;
    bool            m_bVertical;
    bool        m_bSquaredMode;
    bool        m_bHRulerChanged;
    bool        m_bVRulerChanged;

    void UpdatePageSize(const SfxItemSet& rSet);
    void PutGridItem(SfxItemSet& rSet);
    static void SetLinesOrCharsRanges(FixedText & rField, const sal_Int32 nValue );

    DECL_LINK_TYPED(GridTypeHdl, Button*, void);
    DECL_LINK_TYPED(CharorLineChangedHdl, SpinField&, void);
    DECL_LINK_TYPED(CharorLineLoseFocusdHdl, Control&, void);
    DECL_LINK_TYPED(TextSizeChangedHdl, SpinField&, void);
    DECL_LINK_TYPED(TextSizeLoseFocusHdl, Control&, void);
    DECL_LINK_TYPED(GridModifyHdl, ListBox&, void);
    DECL_LINK_TYPED(GridModifyClickHdl, Button*, void);
    DECL_LINK_TYPED(DisplayGridHdl, Button*, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTextGridPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwTextGridPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges();

    virtual bool    FillItemSet(SfxItemSet *rSet) override;
    virtual void    Reset(const SfxItemSet *rSet) override;

    virtual void    ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
