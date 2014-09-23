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
    Edit*           m_pNameED;
    TextFilter      m_aTextFilter;
    FixedText*      m_pWidthFT;
    PercentField m_aWidthMF;
    CheckBox*       m_pRelWidthCB;

    RadioButton*    m_pFullBtn;
    RadioButton*    m_pLeftBtn;
    RadioButton*    m_pFromLeftBtn;
    RadioButton*    m_pRightBtn;
    RadioButton*    m_pCenterBtn;
    RadioButton*    m_pFreeBtn;

    FixedText*      m_pLeftFT;
    PercentField m_aLeftMF;
    FixedText*      m_pRightFT;
    PercentField m_aRightMF;
    FixedText*      m_pTopFT;
    MetricField*    m_pTopMF;
    FixedText*      m_pBottomFT;
    MetricField*    m_pBottomMF;

    ListBox*        m_pTextDirectionLB;

    SwTableRep*     pTblData;
    SwTwips         nSaveWidth;
    SwTwips         nMinTableWidth;
    bool            bModified;
    bool            bFull:1;
    bool            bHtmlMode : 1;

    void        Init();
    void        ModifyHdl(const Edit* pEdit);

    DECL_LINK( AutoClickHdl, CheckBox * );
    DECL_LINK( RelWidthClickHdl, CheckBox * );
    void RightModify();
    DECL_LINK( UpDownLoseFocusHdl, MetricField * );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwFormatTablePage( vcl::Window* pParent, const SfxItemSet& rSet );

    static SfxTabPage*  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;
};

// TabPage Format/Table/Columns
#define MET_FIELDS 6 //Number of the used MetricFields

class SwTableColumnPage : public SfxTabPage
{
    CheckBox*       m_pModifyTableCB;
    CheckBox*       m_pProportionalCB;
    FixedText*      m_pSpaceFT;
    MetricField*    m_pSpaceED;
    PushButton*     m_pUpBtn;
    PushButton*     m_pDownBtn;

    SwTableRep*     pTblData;
    PercentField  m_aFieldArr[MET_FIELDS];
    FixedText*      m_pTextArr[MET_FIELDS];
    SwTwips         nTableWidth;
    SwTwips         nMinWidth;
    sal_uInt16          nNoOfCols;
    sal_uInt16          nNoOfVisibleCols;
    // Remember the width, when switching to autoalign
    sal_uInt16          aValueTbl[MET_FIELDS];// primary assignment of the MetricFields
    bool            bModified:1;
    bool            bModifyTbl:1;
    bool            bPercentMode:1;

    void        Init(bool bWeb);
    DECL_LINK( AutoClickHdl, CheckBox * );
    void        ModifyHdl( MetricField* pEdit );
    DECL_LINK( UpHdl, MetricField * );
    DECL_LINK( DownHdl, MetricField * );
    DECL_LINK( LoseFocusHdl, MetricField * );
    DECL_LINK( ModeHdl, CheckBox * );
    void        UpdateCols( sal_uInt16 nAktPos );
    SwTwips     GetVisibleWidth(sal_uInt16 nPos);
    void        SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwTableColumnPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwTableColumnPage();

    static SfxTabPage*  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;

};

class SwTextFlowPage : public SfxTabPage
{
    CheckBox*       m_pPgBrkCB;

    RadioButton*    m_pPgBrkRB;
    RadioButton*    m_pColBrkRB;

    RadioButton*    m_pPgBrkBeforeRB;
    RadioButton*    m_pPgBrkAfterRB;

    CheckBox*       m_pPageCollCB;
    ListBox*        m_pPageCollLB;
    FixedText*      m_pPageNoFT;
    NumericField*   m_pPageNoNF;
    CheckBox*       m_pSplitCB;
    TriStateBox*    m_pSplitRowCB;
    CheckBox*       m_pKeepCB;
    CheckBox*       m_pHeadLineCB;
    NumericField*   m_pRepeatHeaderNF;
    VclContainer*   m_pRepeatHeaderCombo;
    ListBox*        m_pTextDirectionLB;

    ListBox*        m_pVertOrientLB;

    SwWrtShell*     pShell;

    bool            bPageBreak;
    bool            bHtmlMode;

    DECL_LINK(PageBreakHdl_Impl, void *);
    DECL_LINK(ApplyCollClickHdl_Impl, void *);
    DECL_LINK( PageBreakPosHdl_Impl, RadioButton* );
    DECL_LINK( PageBreakTypeHdl_Impl, RadioButton* );
    DECL_LINK( SplitHdl_Impl, CheckBox* );
    DECL_LINK( SplitRowHdl_Impl, TriStateBox* );
    DECL_LINK( HeadLineCBClickHdl, void* p = 0 );

    SwTextFlowPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwTextFlowPage();

public:
    static SfxTabPage*  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    void                SetShell(SwWrtShell* pSh);

    void                DisablePageBreak();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
