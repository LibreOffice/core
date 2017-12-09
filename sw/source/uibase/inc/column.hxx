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

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/image.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
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

class SwColumnDlg : public SfxModalDialog
{
    VclPtr<ListBox>            m_pApplyToLB;

    SwWrtShell&         rWrtShell;
    VclPtr<SwColumnPage>       pTabPage;
    SfxItemSet*         pPageSet;
    SfxItemSet*         pSectionSet;
    SfxItemSet*         pSelectionSet;
    SfxItemSet*         pFrameSet;

    long                nOldSelection;
    long                nSelectionWidth;
    long                nPageWidth;

    bool                bPageChanged : 1;
    bool                bSectionChanged : 1;
    bool                bSelSectionChanged : 1;
    bool                bFrameChanged : 1;

    DECL_LINK(ObjectListBoxHdl, ListBox&, void);
    DECL_LINK(OkHdl, Button*, void);
    void ObjectHdl(ListBox const *);
    SfxItemSet* EvalCurrentSelection(void);

public:
    SwColumnDlg(vcl::Window* pParent, SwWrtShell& rSh);
    virtual ~SwColumnDlg() override;
    virtual void dispose() override;
};

class ColumnValueSet : public ValueSet
{
public:
    ColumnValueSet(vcl::Window* pParent)
        : ValueSet(pParent, WB_TABSTOP | WB_ITEMBORDER | WB_DOUBLEBORDER)
    {
    }
    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
};

// column dialog now as TabPage
class SwColumnPage : public SfxTabPage
{
    VclPtr<NumericField>   m_pCLNrEdt;
    VclPtr<ColumnValueSet> m_pDefaultVS;
    VclPtr<CheckBox>       m_pBalanceColsCB;

    VclPtr<PushButton>     m_pBtnBack;
    VclPtr<FixedText>      m_pLbl1;
    PercentField m_aEd1;
    VclPtr<FixedText>      m_pLbl2;
    PercentField m_aEd2;
    VclPtr<FixedText>      m_pLbl3;
    PercentField m_aEd3;
    VclPtr<PushButton>     m_pBtnNext;
    PercentField m_aDistEd1;
    PercentField m_aDistEd2;
    VclPtr<CheckBox>       m_pAutoWidthBox;

    VclPtr<FixedText>      m_pLineTypeLbl;
    VclPtr<LineListBox>    m_pLineTypeDLB;
    VclPtr<FixedText>      m_pLineWidthLbl;
    VclPtr<MetricField>    m_pLineWidthEdit;
    VclPtr<FixedText>      m_pLineColorLbl;
    VclPtr<SvxColorListBox> m_pLineColorDLB;
    VclPtr<FixedText>      m_pLineHeightLbl;
    VclPtr<MetricField>    m_pLineHeightEdit;
    VclPtr<FixedText>      m_pLinePosLbl;
    VclPtr<ListBox>        m_pLinePosDLB;

    VclPtr<FixedText>      m_pTextDirectionFT;
    VclPtr<svx::FrameDirectionListBox>        m_pTextDirectionLB;

    // Example
    VclPtr<SwColExample>   m_pPgeExampleWN;
    VclPtr<SwColumnOnlyExample> m_pFrameExampleWN;

    SwColMgr*       m_pColMgr;

    sal_uInt16          m_nFirstVis;
    sal_uInt16          m_nCols;
    long            m_nColWidth[nMaxCols];
    long            m_nColDist[nMaxCols];
    sal_uInt16          m_nMinWidth;
    PercentField*   m_pModifiedField;

    std::map<VclPtr<MetricField>, PercentField*> m_aPercentFieldsMap;

    bool            m_bFormat;
    bool            m_bFrame;
    bool            m_bHtmlMode;
    bool            m_bLockUpdate;

    // Handler
    DECL_LINK( ColModify, Edit&, void );
    void ColModify(NumericField const *);
    DECL_LINK( GapModify, Edit&, void );
    DECL_LINK( EdModify, Edit&, void );
    DECL_LINK( AutoWidthHdl, Button *, void );
    DECL_LINK( SetDefaultsHdl, ValueSet *, void );

    DECL_LINK( Up, Button *, void );
    DECL_LINK( Down, Button *, void );
    DECL_LINK( UpdateColMgr, Edit&, void );
    DECL_LINK( UpdateColMgrListBox, ListBox&, void );
    DECL_LINK( UpdateColMgrColorBox, SvxColorListBox&, void );
    void Timeout();

    void            Update(MetricField const *pInteractiveField);
    void            UpdateCols();
    void            Init();
    void            ResetColWidth();
    void            SetLabels( sal_uInt16 nVis );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    void connectPercentField(PercentField &rWrap, const OString &rName);

    bool isLineNotNone() const;

    static const sal_uInt16 aPageRg[];

public:
    SwColumnPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwColumnPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
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
        m_pBalanceColsCB->Show(bShow);
    }

    void SetInSection(bool bSet);

    void ActivateColumnControl()
    {
        m_pCLNrEdt->GrabFocus();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
