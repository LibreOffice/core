/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _COLUMN_HXX
#define _COLUMN_HXX


#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/image.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/timer.hxx>
#include <vcl/button.hxx>
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
    OKButton            aOK;
    CancelButton        aCancel;
    HelpButton          aHelp;

    FixedText           aApplyToFT;
    ListBox             aApplyToLB;

    SwWrtShell&         rWrtShell;
    SwColumnPage*       pTabPage;
    SfxItemSet*         pPageSet;
    SfxItemSet*         pSectionSet;
    SfxItemSet*         pSelectionSet;
    SfxItemSet*         pFrameSet;

    long                nOldSelection;
    long                nSelectionWidth;
    long                nPageWidth;

    sal_Bool                bPageChanged : 1;
    sal_Bool                bSectionChanged : 1;
    sal_Bool                bSelSectionChanged : 1;
    sal_Bool                bFrameChanged : 1;


    DECL_LINK(ObjectHdl, ListBox*);
    DECL_LINK(OkHdl, OKButton*);

public:
    SwColumnDlg(Window* pParent, SwWrtShell& rSh);
    virtual ~SwColumnDlg();

    SwWrtShell&     GetWrtShell()   { return rWrtShell; }
};

class ColumnValueSet : public ValueSet
{
    public:
        ColumnValueSet(Window* pParent, const ResId& rResId) :
            ValueSet(pParent, rResId){}
        ~ColumnValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*--------------------------------------------------------------------
    Beschreibung:   Spaltendialog jetzt als TabPage
 --------------------------------------------------------------------*/
class SwColumnPage : public SfxTabPage
{
    FixedLine       aFLGroup;
    FixedText       aClNrLbl;
    NumericField    aCLNrEdt;
    ColumnValueSet  aDefaultVS;
    ImageList       aPreColsIL;
    CheckBox        aBalanceColsCB;

    FixedLine       aFLLayout;
    ImageButton     aBtnUp;
    FixedText       aColumnFT;
    FixedText       aWidthFT;
    FixedText       aDistFT;
    FixedText       aLbl1;
    PercentField    aEd1;
    PercentField    aDistEd1;
    FixedText       aLbl2;
    PercentField    aEd2;
    PercentField    aDistEd2;
    FixedText       aLbl3;
    PercentField    aEd3;
    ImageButton     aBtnDown;
    CheckBox        aAutoWidthBox;


    FixedLine       aFLLineType;
    FixedText       aLineTypeLbl;
    LineListBox     aLineTypeDLB;
    FixedText       aLineWidthLbl;
    MetricField     aLineWidthEdit;
    FixedText       aLineColorLbl;
    ColorListBox    aLineColorDLB;
    FixedText       aLineHeightLbl;
    MetricField     aLineHeightEdit;
    FixedText       aLinePosLbl;
    ListBox         aLinePosDLB;

    FixedLine       aVertFL;
    FixedLine       aPropertiesFL;
    FixedText       aTextDirectionFT;
    ListBox         aTextDirectionLB;

    // Example
    SwColExample        aPgeExampleWN;
    SwColumnOnlyExample aFrmExampleWN;

    SwColMgr*       pColMgr;

    sal_uInt16          nFirstVis;
    sal_uInt16          nCols;
    long            nColWidth[nMaxCols];
    long            nColDist[nMaxCols];
    sal_uInt16          nMinWidth;
    PercentField    *pModifiedField;
    sal_Bool            bFormat;
    sal_Bool            bFrm;
    sal_Bool            bHtmlMode;
    sal_Bool            bLockUpdate;

    // Handler
    DECL_LINK( ColModify, NumericField * );
    DECL_LINK( GapModify, PercentField * );
    DECL_LINK( EdModify, PercentField * );
    DECL_LINK( AutoWidthHdl, CheckBox * );
    DECL_LINK( Timeout, Timer * );
    DECL_LINK( SetDefaultsHdl, ValueSet * );

    DECL_LINK( Up, Button * );
    DECL_LINK( Down, Button * );
    void            Apply(Button *);
    DECL_LINK( UpdateColMgr, void* );

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

public:
    virtual ~SwColumnPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    void SetFrmMode(sal_Bool bMod);
    void SetPageWidth(long nPageWidth);

    void SetFormatUsed(sal_Bool bFmt) { bFormat = bFmt; }

    void ShowBalance(sal_Bool bShow) {aBalanceColsCB.Show(bShow);}
    void SetInSection(sal_Bool bSet);

    void ActivateColumnControl() {aCLNrEdt.GrabFocus();}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
