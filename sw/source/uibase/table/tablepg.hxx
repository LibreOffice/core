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
#ifndef INCLUDED_SW_SOURCE_UIBASE_TABLE_TABLEPG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_TABLE_TABLEPG_HXX
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <actctrl.hxx>
#include "prcntfld.hxx"
#include "swtypes.hxx"

class SwWrtShell;
class SwTableRep;

struct TColumn
{
    SwTwips nWidth;
    bool    bVisible;
};

class SwFormatTablePage : public SfxTabPage
{
    VclPtr<Edit>           m_pNameED;
    VclPtr<FixedText>      m_pWidthFT;
    PercentField           m_aWidthMF;
    VclPtr<CheckBox>       m_pRelWidthCB;

    VclPtr<RadioButton>    m_pFullBtn;
    VclPtr<RadioButton>    m_pLeftBtn;
    VclPtr<RadioButton>    m_pFromLeftBtn;
    VclPtr<RadioButton>    m_pRightBtn;
    VclPtr<RadioButton>    m_pCenterBtn;
    VclPtr<RadioButton>    m_pFreeBtn;

    VclPtr<FixedText>      m_pLeftFT;
    PercentField m_aLeftMF;
    VclPtr<FixedText>      m_pRightFT;
    PercentField m_aRightMF;
    VclPtr<FixedText>      m_pTopFT;
    VclPtr<MetricField>    m_pTopMF;
    VclPtr<FixedText>      m_pBottomFT;
    VclPtr<MetricField>    m_pBottomMF;

    VclPtr<ListBox>        m_pTextDirectionLB;

    SwTableRep*     pTableData;
    SwTwips         nSaveWidth;
    SwTwips         nMinTableWidth;
    bool            bModified;
    bool            bFull:1;
    bool            bHtmlMode : 1;

    void        Init();
    void        ModifyHdl(const Edit* pEdit);

    DECL_LINK_TYPED( AutoClickHdl, Button*, void );
    DECL_LINK_TYPED( RelWidthClickHdl, Button*, void );
    void RightModify();
    DECL_LINK_TYPED( UpDownHdl, SpinField&, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwFormatTablePage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwFormatTablePage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

// TabPage Format/Table/Columns
#define MET_FIELDS 6 //Number of the used MetricFields

class SwTableColumnPage : public SfxTabPage
{
    VclPtr<CheckBox>       m_pModifyTableCB;
    VclPtr<CheckBox>       m_pProportionalCB;
    VclPtr<FixedText>      m_pSpaceFT;
    VclPtr<MetricField>    m_pSpaceED;
    VclPtr<PushButton>     m_pUpBtn;
    VclPtr<PushButton>     m_pDownBtn;

    SwTableRep*     pTableData;
    PercentField  m_aFieldArr[MET_FIELDS];
    VclPtr<FixedText>      m_pTextArr[MET_FIELDS];
    SwTwips         nTableWidth;
    SwTwips         nMinWidth;
    sal_uInt16          nNoOfCols;
    sal_uInt16          nNoOfVisibleCols;
    // Remember the width, when switching to autoalign
    sal_uInt16          aValueTable[MET_FIELDS];// primary assignment of the MetricFields
    bool            bModified:1;
    bool            bModifyTable:1;
    bool            bPercentMode:1;

    void        Init(bool bWeb);
    DECL_LINK_TYPED( AutoClickHdl, Button *, void );
    void        ModifyHdl( MetricField* pEdit );
    DECL_LINK_TYPED( UpHdl, SpinField&, void );
    DECL_LINK_TYPED( DownHdl, SpinField&, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( ModeHdl, Button *, void );
    void        UpdateCols( sal_uInt16 nAktPos );
    SwTwips     GetVisibleWidth(sal_uInt16 nPos);
    void        SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwTableColumnPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwTableColumnPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

class SwTextFlowPage : public SfxTabPage
{
    VclPtr<CheckBox>       m_pPgBrkCB;

    VclPtr<RadioButton>    m_pPgBrkRB;
    VclPtr<RadioButton>    m_pColBrkRB;

    VclPtr<RadioButton>    m_pPgBrkBeforeRB;
    VclPtr<RadioButton>    m_pPgBrkAfterRB;

    VclPtr<CheckBox>       m_pPageCollCB;
    VclPtr<ListBox>        m_pPageCollLB;
    VclPtr<FixedText>      m_pPageNoFT;
    VclPtr<NumericField>   m_pPageNoNF;
    VclPtr<CheckBox>       m_pSplitCB;
    VclPtr<TriStateBox>    m_pSplitRowCB;
    VclPtr<CheckBox>       m_pKeepCB;
    VclPtr<CheckBox>       m_pHeadLineCB;
    VclPtr<NumericField>   m_pRepeatHeaderNF;
    VclPtr<VclContainer>   m_pRepeatHeaderCombo;
    VclPtr<ListBox>        m_pTextDirectionLB;

    VclPtr<ListBox>        m_pVertOrientLB;

    SwWrtShell*     pShell;

    bool            bPageBreak;
    bool            bHtmlMode;

    DECL_LINK_TYPED(PageBreakHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ApplyCollClickHdl_Impl, Button*, void);
    DECL_LINK_TYPED( PageBreakPosHdl_Impl, Button*, void );
    DECL_LINK_TYPED( PageBreakTypeHdl_Impl, Button*, void );
    DECL_LINK_TYPED( SplitHdl_Impl, Button*, void );
    DECL_STATIC_LINK_TYPED( SwTextFlowPage, SplitRowHdl_Impl, Button*, void );
    DECL_LINK_TYPED( HeadLineCBClickHdl, Button* = nullptr, void );

public:
    SwTextFlowPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwTextFlowPage();
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                SetShell(SwWrtShell* pSh);

    void                DisablePageBreak();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
