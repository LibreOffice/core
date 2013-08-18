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

#ifndef _COLUMN_HXX
#define _COLUMN_HXX


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
    ListBox*            m_pApplyToLB;

    SwWrtShell&         rWrtShell;
    SwColumnPage*       pTabPage;
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


    DECL_LINK(ObjectHdl, ListBox*);
    DECL_LINK(OkHdl, void *);

public:
    SwColumnDlg(Window* pParent, SwWrtShell& rSh);
    virtual ~SwColumnDlg();

    SwWrtShell&     GetWrtShell()   { return rWrtShell; }
};

class ColumnValueSet : public ValueSet
{
public:
    ColumnValueSet(Window* pParent, const ResId& rResId)
        : ValueSet(pParent, rResId)
    {
    }
    ColumnValueSet(Window* pParent)
        : ValueSet(pParent, WB_TABSTOP | WB_ITEMBORDER | WB_DOUBLEBORDER)
    {
    }
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*--------------------------------------------------------------------
    Description:    column dialog now as TabPage
 --------------------------------------------------------------------*/
class SwColumnPage : public SfxTabPage
{
    NumericField*   m_pCLNrEdt;
    ColumnValueSet* m_pDefaultVS;
    CheckBox*       m_pBalanceColsCB;

    PushButton*     m_pBtnBack;
    FixedText*      m_pLbl1;
    PercentField aEd1;
    FixedText*      m_pLbl2;
    PercentField aEd2;
    FixedText*      m_pLbl3;
    PercentField aEd3;
    PushButton*     m_pBtnNext;
    PercentField aDistEd1;
    PercentField aDistEd2;
    CheckBox*       m_pAutoWidthBox;

    FixedText*      m_pLineTypeLbl;
    LineListBox*    m_pLineTypeDLB;
    FixedText*      m_pLineWidthLbl;
    MetricField*    m_pLineWidthEdit;
    FixedText*      m_pLineColorLbl;
    ColorListBox*   m_pLineColorDLB;
    FixedText*      m_pLineHeightLbl;
    MetricField*    m_pLineHeightEdit;
    FixedText*      m_pLinePosLbl;
    ListBox*        m_pLinePosDLB;

    FixedText*      m_pTextDirectionFT;
    ListBox*        m_pTextDirectionLB;

    // Example
    SwColExample*   m_pPgeExampleWN;
    SwColumnOnlyExample* m_pFrmExampleWN;

    SwColMgr*       pColMgr;

    sal_uInt16          nFirstVis;
    sal_uInt16          nCols;
    long            nColWidth[nMaxCols];
    long            nColDist[nMaxCols];
    sal_uInt16          nMinWidth;
    PercentField*   pModifiedField;

    std::map<MetricField*, PercentField*> m_aPercentFieldsMap;

    sal_Bool            bFormat;
    sal_Bool            bFrm;
    sal_Bool            bHtmlMode;
    bool            bLockUpdate;

    // Handler
    DECL_LINK( ColModify, NumericField * );
    DECL_LINK( GapModify, MetricField * );
    DECL_LINK( EdModify, MetricField * );
    DECL_LINK( AutoWidthHdl, CheckBox * );
    DECL_LINK( SetDefaultsHdl, ValueSet * );

    DECL_LINK(Up, void *);
    DECL_LINK(Down, void *);
    DECL_LINK( UpdateColMgr, void* );
    void Apply(Button *);
    void Timeout();

    void            Update();
    void            UpdateCols();
    void            Init();
    void            ResetColWidth();
    void            SetLabels( sal_uInt16 nVis );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void    ActivatePage(const SfxItemSet& rSet);
    virtual int     DeactivatePage(SfxItemSet *pSet);

    SwColumnPage(Window *pParent, const SfxItemSet &rSet);

    void connectPercentField(PercentField &rWrap, const OString &rName);

    bool isLineNotNone() const;

public:
    virtual ~SwColumnPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    void SetFrmMode(sal_Bool bMod);
    void SetPageWidth(long nPageWidth);

    void SetFormatUsed(sal_Bool bFmt)
    {
        bFormat = bFmt;
    }

    void ShowBalance(sal_Bool bShow)
    {
        m_pBalanceColsCB->Show(bShow);
    }

    void SetInSection(sal_Bool bSet);

    void ActivateColumnControl()
    {
        m_pCLNrEdt->GrabFocus();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
