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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_COLUMN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_COLUMN_HXX

#include <vcl/timer.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/colorbox.hxx>
#include <svx/frmdirlbox.hxx>
#include <fmtclbl.hxx>
#include "colex.hxx"
#include "prcntfld.hxx"

const int nMaxCols = 99;
class SwColMgr;
class SwWrtShell;
class SwColumnPage;

class SwColumnDlg : public SfxDialogController
{
    SwWrtShell&         m_rWrtShell;
    VclPtr<SwColumnPage>       m_pTabPage;
    std::unique_ptr<SfxItemSet> m_pPageSet;
    std::unique_ptr<SfxItemSet> m_pSectionSet;
    std::unique_ptr<SfxItemSet> m_pSelectionSet;
    SfxItemSet*         m_pFrameSet;

    long                m_nOldSelection;
    long                m_nSelectionWidth;
    long                m_nPageWidth;

    bool                m_bPageChanged : 1;
    bool                m_bSectionChanged : 1;
    bool                m_bSelSectionChanged : 1;
    bool                m_bFrameChanged : 1;

    std::unique_ptr<weld::Container> m_xContentArea;
    std::unique_ptr<weld::Button> m_xOkButton;

    DECL_LINK(ObjectListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(OkHdl, weld::Button&, void);
    void ObjectHdl(const weld::ComboBox*);
    SfxItemSet* EvalCurrentSelection(void);

public:
    SwColumnDlg(weld::Window* pParent, SwWrtShell& rSh);
    virtual ~SwColumnDlg() override;
};

class ColumnValueSet : public SvtValueSet
{
public:
    ColumnValueSet()
        : SvtValueSet(nullptr)
    {
    }
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        SvtValueSet::SetDrawingArea(pDrawingArea);
        SetStyle(WB_TABSTOP | WB_ITEMBORDER | WB_DOUBLEBORDER);
    }
    virtual void UserDraw(const UserDrawEvent& rUDEvt) override;
    virtual void StyleUpdated() override;
};

// column dialog now as TabPage
class SwColumnPage : public SfxTabPage
{
    std::unique_ptr<SwColMgr> m_xColMgr;

    sal_uInt16      m_nFirstVis;
    sal_uInt16      m_nCols;
    long            m_nColWidth[nMaxCols];
    long            m_nColDist[nMaxCols];
    SwPercentField* m_pModifiedField;

    std::map<weld::MetricSpinButton*, SwPercentField*> m_aPercentFieldsMap;

    bool            m_bFormat;
    bool            m_bFrame;
    bool            m_bHtmlMode;
    bool            m_bLockUpdate;

    ColumnValueSet m_aDefaultVS;
    SwColExample m_aPgeExampleWN;
    SwColumnOnlyExample m_aFrameExampleWN;

    std::unique_ptr<weld::SpinButton>  m_xCLNrEdt;
    std::unique_ptr<weld::CheckButton> m_xBalanceColsCB;
    std::unique_ptr<weld::Button> m_xBtnBack;
    std::unique_ptr<weld::Label> m_xLbl1;
    std::unique_ptr<weld::Label> m_xLbl2;
    std::unique_ptr<weld::Label> m_xLbl3;
    std::unique_ptr<weld::Button> m_xBtnNext;
    std::unique_ptr<weld::CheckButton> m_xAutoWidthBox;
    std::unique_ptr<weld::Label> m_xLineTypeLbl;
    std::unique_ptr<weld::Label> m_xLineWidthLbl;
    std::unique_ptr<weld::MetricSpinButton> m_xLineWidthEdit;
    std::unique_ptr<weld::Label> m_xLineColorLbl;
    std::unique_ptr<weld::Label> m_xLineHeightLbl;
    std::unique_ptr<weld::MetricSpinButton> m_xLineHeightEdit;
    std::unique_ptr<weld::Label> m_xLinePosLbl;
    std::unique_ptr<weld::ComboBox> m_xLinePosDLB;
    std::unique_ptr<weld::Label> m_xTextDirectionFT;
    std::unique_ptr<svx::FrameDirectionListBox> m_xTextDirectionLB;
    std::unique_ptr<ColorListBox> m_xLineColorDLB;
    std::unique_ptr<SvtLineListBox> m_xLineTypeDLB;
    std::unique_ptr<SwPercentField> m_xEd1;
    std::unique_ptr<SwPercentField> m_xEd2;
    std::unique_ptr<SwPercentField> m_xEd3;
    std::unique_ptr<SwPercentField> m_xDistEd1;
    std::unique_ptr<SwPercentField> m_xDistEd2;
    std::unique_ptr<weld::CustomWeld> m_xDefaultVS;
    // Example
    std::unique_ptr<weld::CustomWeld> m_xPgeExampleWN;
    std::unique_ptr<weld::CustomWeld> m_xFrameExampleWN;

    std::unique_ptr<weld::Label> m_xApplyToFT;
    std::unique_ptr<weld::ComboBox> m_xApplyToLB;

    // Handler
    DECL_LINK(ColModify, weld::SpinButton&, void);
    void ColModify(const weld::SpinButton*);
    DECL_LINK(GapModify, weld::MetricSpinButton&, void);
    DECL_LINK(EdModify, weld::MetricSpinButton&, void);
    DECL_LINK(AutoWidthHdl, weld::ToggleButton&, void );
    DECL_LINK(SetDefaultsHdl, SvtValueSet *, void);

    DECL_LINK(Up, weld::Button&, void);
    DECL_LINK(Down, weld::Button&, void);
    DECL_LINK(UpdateColMgr, weld::MetricSpinButton&, void);
    DECL_LINK(UpdateColMgrListBox, weld::ComboBox&, void);
    DECL_LINK(UpdateColMgrLineBox, SvtLineListBox&, void);
    DECL_LINK(UpdateColMgrColorBox, ColorListBox&, void);
    void Timeout();

    void            Update(const weld::MetricSpinButton* pInteractiveField);
    void            UpdateCols();
    void            Init();
    void            ResetColWidth();
    void            SetLabels( sal_uInt16 nVis );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    void connectPercentField(SwPercentField &rWrap);

    bool isLineNotNone() const;

    static const sal_uInt16 aPageRg[];

public:
    SwColumnPage(TabPageParent pParent, const SfxItemSet &rSet);
    virtual ~SwColumnPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool    FillItemSet(SfxItemSet *rSet) override;
    virtual void    Reset(const SfxItemSet *rSet) override;

    void SetFrameMode(bool bMod);
    void SetPageWidth(long nPageWidth);

    void SetFormatUsed(bool bFormatUsed)
    {
        m_bFormat = bFormatUsed;
    }

    void ShowBalance(bool bShow)
    {
        m_xBalanceColsCB->set_visible(bShow);
    }

    void SetInSection(bool bSet);

    void ActivateColumnControl()
    {
        m_xCLNrEdt->grab_focus();
    }

    weld::Label* GetApplyLabel() { return m_xApplyToFT.get(); }
    weld::ComboBox* GetApplyComboBox() { return m_xApplyToLB.get(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
