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
#include <prcntfld.hxx>
#include <swtypes.hxx>
#include <svx/frmdirlbox.hxx>

class SwWrtShell;
class SwTableRep;
struct ImplSVEvent;

class SwFormatTablePage : public SfxTabPage
{
    SwTableRep*     pTableData;
    std::unique_ptr<SwTableRep> m_xOrigTableData;
    SwTwips         nSaveWidth;
    SwTwips         nMinTableWidth;
    bool            bModified;
    bool            bFull:1;
    bool            bHtmlMode : 1;

    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Label> m_xWidthFT;
    std::unique_ptr<SwPercentField> m_xWidthMF;
    std::unique_ptr<weld::CheckButton> m_xRelWidthCB;

    std::unique_ptr<weld::RadioButton> m_xFullBtn;
    std::unique_ptr<weld::RadioButton> m_xLeftBtn;
    std::unique_ptr<weld::RadioButton> m_xFromLeftBtn;
    std::unique_ptr<weld::RadioButton> m_xRightBtn;
    std::unique_ptr<weld::RadioButton> m_xCenterBtn;
    std::unique_ptr<weld::RadioButton> m_xFreeBtn;

    std::unique_ptr<weld::Label> m_xLeftFT;
    std::unique_ptr<SwPercentField> m_xLeftMF;
    std::unique_ptr<weld::Label> m_xRightFT;
    std::unique_ptr<SwPercentField> m_xRightMF;
    std::unique_ptr<weld::Label> m_xTopFT;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMF;
    std::unique_ptr<weld::Label> m_xBottomFT;
    std::unique_ptr<weld::MetricSpinButton> m_xBottomMF;

    std::unique_ptr<svx::FrameDirectionListBox> m_xTextDirectionLB;
    std::unique_ptr<weld::Widget> m_xProperties;

    void        Init();
    void        ModifyHdl(const weld::MetricSpinButton& rEdit);

    DECL_LINK(AutoClickHdl, weld::ToggleButton&, void);
    DECL_LINK(RelWidthClickHdl, weld::ToggleButton&, void);
    void RightModify();
    DECL_LINK(ValueChangedHdl, weld::MetricSpinButton&, void);

public:
    SwFormatTablePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~SwFormatTablePage() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

// TabPage Format/Table/Columns
#define MET_FIELDS 6 //Number of the used MetricFields

class SwTableColumnPage : public SfxTabPage
{
    SwTableRep*     m_pTableData;
    std::unique_ptr<SwTableRep> m_xOrigTableData;
    ImplSVEvent*    m_pSizeHdlEvent;
    SwTwips         m_nTableWidth;
    SwTwips         m_nMinWidth;
    sal_uInt16      m_nMetFields;
    sal_uInt16      m_nNoOfCols;
    sal_uInt16      m_nNoOfVisibleCols;
    // Remember the width, when switching to autoalign
    sal_uInt16      m_aValueTable[MET_FIELDS];// primary assignment of the MetricFields
    bool            m_bModified:1;
    bool            m_bModifyTable:1;
    bool            m_bPercentMode:1;

    SwPercentField  m_aFieldArr[MET_FIELDS];
    std::unique_ptr<weld::Label> m_aTextArr[MET_FIELDS];
    std::unique_ptr<weld::CheckButton> m_xModifyTableCB;
    std::unique_ptr<weld::CheckButton> m_xProportionalCB;
    std::unique_ptr<weld::Label> m_xSpaceFT;
    std::unique_ptr<weld::Label> m_xSpaceSFT;
    std::unique_ptr<weld::MetricSpinButton> m_xSpaceED;
    std::unique_ptr<weld::Button> m_xUpBtn;
    std::unique_ptr<weld::Button> m_xDownBtn;

    void        Init(bool bWeb);
    DECL_LINK(AutoClickHdl, weld::Button&, void);
    void        ModifyHdl(const weld::MetricSpinButton* pEdit);
    DECL_LINK(ValueChangedHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModeHdl, weld::ToggleButton&, void);
    void        UpdateCols( sal_uInt16 nCurrentPos );
    SwTwips     GetVisibleWidth(sal_uInt16 nPos);
    void        SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth);
    DECL_LINK(SizeHdl, void*, void);

public:
    SwTableColumnPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~SwTableColumnPage() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

class SwTextFlowPage : public SfxTabPage
{
    SwWrtShell*     pShell;
    bool            bPageBreak;
    bool            bHtmlMode;

    std::unique_ptr<weld::CheckButton> m_xPgBrkCB;
    std::unique_ptr<weld::RadioButton> m_xPgBrkRB;
    std::unique_ptr<weld::RadioButton> m_xColBrkRB;
    std::unique_ptr<weld::RadioButton> m_xPgBrkBeforeRB;
    std::unique_ptr<weld::RadioButton> m_xPgBrkAfterRB;
    std::unique_ptr<weld::CheckButton> m_xPageCollCB;
    std::unique_ptr<weld::ComboBox> m_xPageCollLB;
    std::unique_ptr<weld::CheckButton> m_xPageNoCB;
    std::unique_ptr<weld::SpinButton> m_xPageNoNF;
    std::unique_ptr<weld::CheckButton> m_xSplitCB;
    std::unique_ptr<weld::CheckButton> m_xSplitRowCB;
    std::unique_ptr<weld::CheckButton> m_xKeepCB;
    std::unique_ptr<weld::CheckButton> m_xHeadLineCB;
    std::unique_ptr<weld::Widget> m_xRepeatHeaderCombo;
    std::unique_ptr<weld::SpinButton> m_xRepeatHeaderNF;
    std::unique_ptr<weld::ComboBox> m_xTextDirectionLB;
    std::unique_ptr<weld::ComboBox> m_xVertOrientLB;

    DECL_LINK(PageBreakHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ApplyCollClickHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(PageBreakPosHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(PageBreakTypeHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(PageNoClickHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(SplitHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(HeadLineCBClickHdl, weld::ToggleButton&, void);

public:
    SwTextFlowPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~SwTextFlowPage() override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                SetShell(SwWrtShell* pSh);

    void                DisablePageBreak();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
