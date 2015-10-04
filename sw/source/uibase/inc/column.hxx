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
#include <fmtclbl.hxx>
#include <colex.hxx>
#include <prcntfld.hxx>

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

    DECL_LINK_TYPED(ObjectListBoxHdl, ListBox&, void);
    DECL_LINK_TYPED(OkHdl, Button*, void);
    void ObjectHdl(ListBox*);

public:
    SwColumnDlg(vcl::Window* pParent, SwWrtShell& rSh);
    virtual ~SwColumnDlg();
    virtual void dispose() SAL_OVERRIDE;
};

class ColumnValueSet : public ValueSet
{
public:
    ColumnValueSet(vcl::Window* pParent, const ResId& rResId)
        : ValueSet(pParent, rResId)
    {
    }
    ColumnValueSet(vcl::Window* pParent)
        : ValueSet(pParent, WB_TABSTOP | WB_ITEMBORDER | WB_DOUBLEBORDER)
    {
    }
    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
};

// column dialog now as TabPage
class SwColumnPage : public SfxTabPage
{
    VclPtr<NumericField>   m_pCLNrEdt;
    VclPtr<ColumnValueSet> m_pDefaultVS;
    VclPtr<CheckBox>       m_pBalanceColsCB;

    VclPtr<PushButton>     m_pBtnBack;
    VclPtr<FixedText>      m_pLbl1;
    PercentField aEd1;
    VclPtr<FixedText>      m_pLbl2;
    PercentField aEd2;
    VclPtr<FixedText>      m_pLbl3;
    PercentField aEd3;
    VclPtr<PushButton>     m_pBtnNext;
    PercentField aDistEd1;
    PercentField aDistEd2;
    VclPtr<CheckBox>       m_pAutoWidthBox;

    VclPtr<FixedText>      m_pLineTypeLbl;
    VclPtr<LineListBox>    m_pLineTypeDLB;
    VclPtr<FixedText>      m_pLineWidthLbl;
    VclPtr<MetricField>    m_pLineWidthEdit;
    VclPtr<FixedText>      m_pLineColorLbl;
    VclPtr<ColorListBox>   m_pLineColorDLB;
    VclPtr<FixedText>      m_pLineHeightLbl;
    VclPtr<MetricField>    m_pLineHeightEdit;
    VclPtr<FixedText>      m_pLinePosLbl;
    VclPtr<ListBox>        m_pLinePosDLB;

    VclPtr<FixedText>      m_pTextDirectionFT;
    VclPtr<ListBox>        m_pTextDirectionLB;

    // Example
    VclPtr<SwColExample>   m_pPgeExampleWN;
    VclPtr<SwColumnOnlyExample> m_pFrmExampleWN;

    SwColMgr*       pColMgr;

    sal_uInt16          nFirstVis;
    sal_uInt16          nCols;
    long            nColWidth[nMaxCols];
    long            nColDist[nMaxCols];
    sal_uInt16          nMinWidth;
    PercentField*   pModifiedField;

    std::map<VclPtr<MetricField>, PercentField*> m_aPercentFieldsMap;

    bool            bFormat;
    bool            bFrm;
    bool            bHtmlMode;
    bool            bLockUpdate;

    // Handler
    DECL_LINK( ColModify, NumericField * );
    DECL_LINK( GapModify, MetricField * );
    DECL_LINK( EdModify, MetricField * );
    DECL_LINK_TYPED( AutoWidthHdl, Button *, void );
    DECL_LINK_TYPED( SetDefaultsHdl, ValueSet *, void );

    DECL_LINK_TYPED(Up, Button *, void);
    DECL_LINK_TYPED(Down, Button *, void);
    DECL_LINK( UpdateColMgr, void* );
    DECL_LINK_TYPED( UpdateColMgrListBox, ListBox&, void );
    void Timeout();

    void            Update(MetricField *pInteractiveField);
    void            UpdateCols();
    void            Init();
    void            ResetColWidth();
    void            SetLabels( sal_uInt16 nVis );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual sfxpg   DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;

    void connectPercentField(PercentField &rWrap, const OString &rName);

    bool isLineNotNone() const;

    static const sal_uInt16 aPageRg[];

public:
    SwColumnPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwColumnPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool    FillItemSet(SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void    Reset(const SfxItemSet *rSet) SAL_OVERRIDE;

    void SetFrmMode(bool bMod);
    void SetPageWidth(long nPageWidth);

    void SetFormatUsed(bool bFormatUsed)
    {
        bFormat = bFormatUsed;
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
