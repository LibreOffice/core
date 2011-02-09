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
#ifndef _SWTABLEPG_HXX
#define _SWTABLEPG_HXX
#include <sfx2/tabdlg.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <actctrl.hxx>
#include "prcntfld.hxx"
#include "swtypes.hxx"
#include "textcontrolcombo.hxx"

class SwWrtShell;
class SwTableRep;

struct TColumn
{
    SwTwips nWidth;
    sal_Bool    bVisible;
};

class SwFormatTablePage : public SfxTabPage
{
    FixedLine        aOptionsFL;
    FixedText       aNameFT;
    TableNameEdit   aNameED;
    FixedText       aWidthFT;
    PercentField    aWidthMF;
    CheckBox        aRelWidthCB;

    FixedLine        aPosFL;
    RadioButton     aFullBtn;
    RadioButton     aLeftBtn;
    RadioButton     aFromLeftBtn;
    RadioButton     aRightBtn;
    RadioButton     aCenterBtn;
    RadioButton     aFreeBtn;

    FixedLine       aDistFL;
    FixedText       aLeftFT;
    PercentField    aLeftMF;
    FixedText       aRightFT;
    PercentField    aRightMF;
    FixedText       aTopFT;
    MetricField     aTopMF;
    FixedText       aBottomFT;
    MetricField     aBottomMF;

    FixedLine       aPropertiesFL;
    FixedText       aTextDirectionFT;
    ListBox         aTextDirectionLB;

    SwTableRep*     pTblData;
    SwTwips         nSaveWidth;
    SwTwips         nMinTableWidth;
    sal_uInt16          nOldAlign;
    sal_Bool            bModified;
    sal_Bool            bFull:1;
    sal_Bool            bHtmlMode : 1;

    void        Init();
    void        ModifyHdl( Edit* pEdit );

    DECL_LINK( AutoClickHdl, CheckBox * );
    DECL_LINK( RelWidthClickHdl, CheckBox * );
    DECL_LINK( RightModifyHdl, MetricField * );
    DECL_LINK( UpDownLoseFocusHdl, MetricField * );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwFormatTablePage( Window* pParent, const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
};

/*-------------------------------------------------------
 TabPage Format/Tabelle/Spalten
--------------------------------------------------------- */
#define MET_FIELDS 6 //Anzahl der verwendeten MetricFields

class SwTableColumnPage : public SfxTabPage
{
    CheckBox        aModifyTableCB;
    CheckBox        aProportionalCB;
    FixedText       aSpaceFT;
    MetricField     aSpaceED;

    ImageButton     aUpBtn;
    FixedText       aFT1;
    PercentField    aMF1;
    FixedText       aFT2;
    PercentField    aMF2;
    FixedText       aFT3;
    PercentField    aMF3;
    FixedText       aFT4;
    PercentField    aMF4;
    FixedText       aFT5;
    PercentField    aMF5;
    FixedText       aFT6;
    PercentField    aMF6;
    ImageButton     aDownBtn;
    FixedLine        aColFL;

    SwTableRep*     pTblData;
    PercentField*   pFieldArr[MET_FIELDS];
    FixedText*      pTextArr[MET_FIELDS];
    SwTwips         nTableWidth;
    SwTwips         nMinWidth;
    sal_uInt16          nNoOfCols;
    sal_uInt16          nNoOfVisibleCols;
    //Breite merken, wenn auf autom. Ausrichtung gestellt wird
    sal_uInt16          aValueTbl[MET_FIELDS];//primaere Zuordnung der MetricFields
    sal_Bool            bModified:1;
    sal_Bool            bModifyTbl:1;
    sal_Bool            bPercentMode:1;

    void        Init(sal_Bool bWeb);
    DECL_LINK( AutoClickHdl, CheckBox * );
    void        ModifyHdl( PercentField* pEdit );
    DECL_LINK( UpHdl, PercentField * );
    DECL_LINK( DownHdl, PercentField * );
    DECL_LINK( LoseFocusHdl, PercentField * );
    DECL_LINK( ModeHdl, CheckBox * );
    void        UpdateCols( sal_uInt16 nAktPos );
    SwTwips     GetVisibleWidth(sal_uInt16 nPos);
    void        SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwTableColumnPage( Window* pParent, const SfxItemSet& rSet );
    ~SwTableColumnPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

};

/*-----------------12.12.96 11.48-------------------
        Textfluss
--------------------------------------------------*/
class SwTextFlowPage : public SfxTabPage
{
    FixedLine       aFlowFL;
    CheckBox        aPgBrkCB;
    RadioButton     aPgBrkRB;
    RadioButton     aColBrkRB;
    RadioButton     aPgBrkBeforeRB;
    RadioButton     aPgBrkAfterRB;
    CheckBox        aPageCollCB;
    ListBox         aPageCollLB;
    FixedText       aPageNoFT;
    NumericField    aPageNoNF;
    CheckBox        aSplitCB;
    TriStateBox     aSplitRowCB;
    CheckBox        aKeepCB;
    CheckBox        aHeadLineCB;
    FixedText       aRepeatHeaderFT;    // "dummy" to build before and after FT
    FixedText       aRepeatHeaderBeforeFT;
    NumericField    aRepeatHeaderNF;
    FixedText       aRepeatHeaderAfterFT;
    TextControlCombo    aRepeatHeaderCombo;
    FixedText       aTextDirectionFT;
    ListBox         aTextDirectionLB;

    FixedLine       aVertOrientFL;
    FixedText       aVertOrientFT;
    ListBox         aVertOrientLB;

    SwWrtShell*     pShell;

    sal_Bool            bPageBreak;
    sal_Bool            bHtmlMode;


    DECL_LINK( PageBreakHdl_Impl, CheckBox* );
    DECL_LINK( ApplyCollClickHdl_Impl, CheckBox* );
    DECL_LINK( PageBreakPosHdl_Impl, RadioButton* );
    DECL_LINK( PageBreakTypeHdl_Impl, RadioButton* );
    DECL_LINK( SplitHdl_Impl, CheckBox* );
    DECL_LINK( SplitRowHdl_Impl, TriStateBox* );
    DECL_LINK( HeadLineCBClickHdl, void* p = 0 );


    SwTextFlowPage( Window* pParent, const SfxItemSet& rSet );
    ~SwTextFlowPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetShell(SwWrtShell* pSh);

    void                DisablePageBreak();
};

#endif
