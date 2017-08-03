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
#include <vcl/weld.hxx>
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

class SwFormatTablePage : public NewSfxTabPage
{
    std::unique_ptr<Weld::Entry> m_xNameED;
    std::unique_ptr<Weld::Label> m_xWidthFT;
    NewPercentField           m_aWidthMF;
    std::unique_ptr<Weld::CheckButton> m_xRelWidthCB;

    std::unique_ptr<Weld::RadioButton> m_xFullBtn;
    std::unique_ptr<Weld::RadioButton> m_xLeftBtn;
    std::unique_ptr<Weld::RadioButton> m_xFromLeftBtn;
    std::unique_ptr<Weld::RadioButton> m_xRightBtn;
    std::unique_ptr<Weld::RadioButton> m_xCenterBtn;
    std::unique_ptr<Weld::RadioButton> m_xFreeBtn;

    std::unique_ptr<Weld::Label> m_xLeftFT;
    NewPercentField m_aLeftMF;
    std::unique_ptr<Weld::Label> m_xRightFT;
    NewPercentField m_aRightMF;
    std::unique_ptr<Weld::Label> m_xTopFT;
    std::unique_ptr<Weld::MetricSpinButton> m_xTopMF;
    std::unique_ptr<Weld::Label> m_xBottomFT;
    std::unique_ptr<Weld::MetricSpinButton> m_xBottomMF;
    std::unique_ptr<Weld::Widget> m_xContainer;

    std::unique_ptr<Weld::ComboBoxText> m_xTextDirectionLB;

    SwTableRep*     pTableData;
    SwTwips         nSaveWidth;
    SwTwips         nMinTableWidth;
    bool            bModified;
    bool            bModifying;
    bool            bFull:1;
    bool            bHtmlMode : 1;

    void        Init();
    void        ModifyHdl(const Weld::MetricSpinButton& rEdit);

    DECL_LINK(AutoClickHdl, Weld::Button&, void);
    DECL_LINK(RelWidthClickHdl, Weld::Button&, void);
    void RightModify();
    DECL_LINK(UpDownHdl, Weld::MetricSpinButton&, void);

public:
    SwFormatTablePage(Weld::Container* pParent, const SfxItemSet& rSet);

    static NewSfxTabPage* Create(Weld::Container* pParent, const SfxItemSet* pAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

// TabPage Format/Table/Columns
#define MET_FIELDS 6 //Number of the used MetricFields

class SwTableColumnPage : public NewSfxTabPage
{
    std::unique_ptr<Weld::CheckButton> m_xModifyTableCB;
    std::unique_ptr<Weld::CheckButton> m_xProportionalCB;
    std::unique_ptr<Weld::Label> m_xSpaceFT;
    std::unique_ptr<Weld::MetricSpinButton> m_xSpaceED;
    std::unique_ptr<Weld::Button> m_xUpBtn;
    std::unique_ptr<Weld::Button> m_xDownBtn;

    SwTableRep*     pTableData;
    NewPercentField m_aFieldArr[MET_FIELDS];
    std::unique_ptr<Weld::Label> m_aTextArr[MET_FIELDS];
    SwTwips         nTableWidth;
    SwTwips         nMinWidth;
    sal_uInt16      nNoOfCols;
    sal_uInt16      nNoOfVisibleCols;
    // Remember the width, when switching to autoalign
    sal_uInt16      aValueTable[MET_FIELDS];// primary assignment of the MetricFields
    bool            bModified:1;
    bool            bModifyTable:1;
    bool            bPercentMode:1;

    void        Init(bool bWeb);
    DECL_LINK(AutoClickHdl, Weld::Button&, void);
    void ModifyHdl(const Weld::MetricSpinButton* pEdit);
    DECL_LINK(ModifyHdl, Weld::MetricSpinButton&, void);
    DECL_LINK(ModeHdl, Weld::Button&, void);
    void        UpdateCols( sal_uInt16 nAktPos );
    SwTwips     GetVisibleWidth(sal_uInt16 nPos);
    void        SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth);

public:
    SwTableColumnPage(Weld::Container* pParent, const SfxItemSet& rSet);

    static NewSfxTabPage* Create(Weld::Container* pParent, const SfxItemSet* pAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

class SwTextFlowPage : public NewSfxTabPage
{
    std::unique_ptr<Weld::CheckButton> m_xPgBrkCB;
    std::unique_ptr<Weld::RadioButton> m_xPgBrkRB;
    std::unique_ptr<Weld::RadioButton> m_xColBrkRB;
    std::unique_ptr<Weld::RadioButton> m_xPgBrkBeforeRB;
    std::unique_ptr<Weld::RadioButton> m_xPgBrkAfterRB;
    std::unique_ptr<Weld::CheckButton> m_xPageCollCB;
    std::unique_ptr<Weld::ComboBoxText> m_xPageCollLB;
    std::unique_ptr<Weld::CheckButton> m_xPageNoCB;
    std::unique_ptr<Weld::SpinButton> m_xPageNoNF;
    std::unique_ptr<Weld::CheckButton> m_xSplitCB;
    std::unique_ptr<Weld::CheckButton> m_xSplitRowCB;
    std::unique_ptr<Weld::CheckButton> m_xKeepCB;
    std::unique_ptr<Weld::CheckButton> m_xHeadLineCB;
    std::unique_ptr<Weld::SpinButton> m_xRepeatHeaderNF;
    std::unique_ptr<Weld::Widget> m_xRepeatHeaderCombo;
    std::unique_ptr<Weld::ComboBoxText> m_xTextDirectionLB;
    std::unique_ptr<Weld::ComboBoxText> m_xVertOrientLB;

    SwWrtShell*     pShell;

    bool            bPageBreak;
    bool            bHtmlMode;

    DECL_LINK(PageBreakHdl_Impl, Weld::Button&, void);
    DECL_LINK(ApplyCollClickHdl_Impl, Weld::Button&, void);
    DECL_LINK(PageBreakPosHdl_Impl, Weld::Button&, void);
    DECL_LINK(PageBreakTypeHdl_Impl, Weld::Button&, void);
    DECL_LINK(PageNoClickHdl_Impl, Weld::Button&, void);
    DECL_LINK(SplitHdl_Impl, Weld::Button&, void);
    DECL_STATIC_LINK(SwTextFlowPage, SplitRowHdl_Impl, Weld::Button&, void);
    DECL_LINK(HeadLineCBClickHdl, Weld::Button&, void);

public:
    SwTextFlowPage(Weld::Container* pParent, const SfxItemSet& rSet);
    static NewSfxTabPage*  Create(Weld::Container* pParent, const SfxItemSet* pAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                SetShell(SwWrtShell* pSh);

    void                DisablePageBreak();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
