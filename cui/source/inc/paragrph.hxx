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
#ifndef _SVX_PARAGRPH_HXX
#define _SVX_PARAGRPH_HXX

// include ---------------------------------------------------------------

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/relfld.hxx>       // SvxRelativeField
#include <svx/paraprev.hxx>     // Preview
#include <svx/frmdirlbox.hxx>
#include <vcl/lstbox.hxx>
#include <svx/flagsdef.hxx>

// define ----------------------------------------------------------------

// 1/2 cm in TWIPS
// wird auch fuer Minimalgrosse der LayFrms aller Arten benutzt
//CHINA001 #define MM50   283

// forward ---------------------------------------------------------------

class SvxLineSpacingItem;
class SvxParagraphControllerItem;

// class SvxStdParagraphTabPage ------------------------------------------

/*  {k:\svx\prototyp\dialog\parastd.bmp}

    [Beschreibung]
    Mit dieser TabPage koennen Standard-Attribute eines Absatzes eingestellt
    werden (Einzug, Abstand, Ausrichtung, Zeilenabstand).

    [Items]
    <SvxAdjustItem><SID_ATTR_PARA_ADJUST>
    <SvxLineSpacingItem><SID_ATTR_PARA_LINESPACE>
    <SvxULSpaceItem><SID_ATTR_ULSPACE>
    <SvxLRSpaceItem><SID_ATTR_LRSPACE>
*/

class SvxStdParagraphTabPage: public SfxTabPage
{
    using TabPage::DeactivatePage;

private:
    SvxStdParagraphTabPage( Window* pParent, const SfxItemSet& rSet );

    // Einzug
    FixedText               aLeftLabel;
    SvxRelativeField        aLeftIndent;

    FixedText               aRightLabel;
    SvxRelativeField        aRightIndent;

    FixedText               aFLineLabel;
    SvxRelativeField        aFLineIndent;
    CheckBox                aAutoCB;

    FixedLine               aIndentFrm;

    // Abstaende
    FixedText               aTopLabel;
    SvxRelativeField        aTopDist;
    FixedText               aBottomLabel;
    SvxRelativeField        aBottomDist;
    FixedLine               aDistFrm;

    // Zeilenabstand
    ListBox                 aLineDist;
    FixedText               aLineDistAtLabel;
    MetricField             aLineDistAtPercentBox;
    MetricField             aLineDistAtMetricBox;
    FixedLine               aLineDistFrm;
    String                  sAbsDist;
    SvxParaPrevWindow       aExampleWin;

    //Registerhaltigkeit - nur Writer
    CheckBox                aRegisterCB;
    FixedLine               aRegisterFL;

    Edit*                   pActLineDistFld;
    long                    nAbst;
    long                    nWidth;
    long                    nMinFixDist;
    sal_Bool                    bRelativeMode;
    sal_Bool                    bNegativeIndents;

#ifdef _SVX_PARAGRPH_CXX
    void                    SetLineSpacing_Impl( const SvxLineSpacingItem& rAttr );
    void                    Init_Impl();
    void                    UpdateExample_Impl( sal_Bool bAll = sal_False );

    DECL_LINK( LineDistHdl_Impl, ListBox* );
    DECL_LINK( ModifyHdl_Impl, SvxRelativeField* );
    DECL_LINK( AutoHdl_Impl, CheckBox* );
#endif

protected:
    virtual int             DeactivatePage( SfxItemSet* pSet = 0 );

public:
    DECL_LINK( ELRLoseFocusHdl, Edit* );

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*          GetRanges();

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );


    void                    SetPageWidth( sal_uInt16 nPageWidth );
    void                    EnableRelativeMode();
    void                    EnableRegisterMode();
    void                    EnableAutoFirstLine();
    void                    EnableAbsLineDist(long nMinTwip);
    void                    EnableNegativeMode();
    virtual void            PageCreated(SfxAllItemSet aSet); // add CHINA001
};

// class SvxParaAlignTabPage ------------------------------------------------

class SvxParaAlignTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

    // Ausrichtung
    FixedLine               aAlignFrm;
    RadioButton             aLeft;
    RadioButton             aRight;
    RadioButton             aCenter;
    RadioButton             aJustify;
    FixedText               aLastLineFT;
    ListBox                 aLastLineLB;
    CheckBox                aExpandCB;

    CheckBox                aSnapToGridCB;

    //preview
    SvxParaPrevWindow       aExampleWin;
    //vertical alignment
    FixedLine               aVertAlignFL;

    FixedText               aVertAlignFT;
    ListBox                 aVertAlignLB;

    FixedLine                   aPropertiesFL;
    FixedText                   aTextDirectionFT;
    svx::FrameDirectionListBox  aTextDirectionLB;

#ifdef _SVX_PARAGRPH_CXX
    DECL_LINK(              AlignHdl_Impl, RadioButton* );
    DECL_LINK(              LastLineHdl_Impl, ListBox* );
    DECL_LINK(              TextDirectionHdl_Impl, ListBox* );

    void                    UpdateExample_Impl( sal_Bool bAll = sal_False );
#endif

                            SvxParaAlignTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual                 ~SvxParaAlignTabPage();

protected:
    virtual int             DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*          GetRanges();

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );

    void                    EnableJustifyExt();
    virtual void            PageCreated(SfxAllItemSet aSet); // add CHINA001
};

// class SvxExtParagraphTabPage ------------------------------------------

/*  {k:\svx\prototyp\dialog\paraext.bmp}

    [Beschreibung]
    Mit dieser TabPage koennen Spezial-Attribute eines Absatzes eingestellt
    werden (Silbentrennung, Seitenumbruch, Schusterjungen, Hurenkinder, ...).

    [Items]
    <SvxHyphenZoneItem><SID_ATTR_PARA_HYPHENZONE>
    <SvxFmtBreakItem><SID_ATTR_PARA_PAGEBREAK>
    <SvxFmtSplitItem><SID_ATTR_PARA_SPLIT>
    <SvxWidowsItem><SID_ATTR_PARA_WIDOWS>
    <SvxOrphansItem><SID_ATTR_PARA_ORPHANS>
*/

class SvxExtParagraphTabPage: public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    virtual             ~SvxExtParagraphTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                DisablePageBreak();

protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

private:
                        SvxExtParagraphTabPage( Window* pParent, const SfxItemSet& rSet );

    // Silbentrennung
    TriStateBox         aHyphenBox;
    FixedText           aBeforeText;
    NumericField        aExtHyphenBeforeBox;
//  FixedText           aHyphenBeforeLabel;
    FixedText           aAfterText;
    NumericField        aExtHyphenAfterBox;
//  FixedText           aHyphenAfterLabel;
    FixedText           aMaxHyphenLabel;
    NumericField        aMaxHyphenEdit;
    FixedLine           aExtFL;

    // Seitenumbruch
    FixedLine           aBreaksFL;
    TriStateBox         aPageBreakBox;
    FixedText           aBreakTypeFT;
    ListBox             aBreakTypeLB;
    FixedText           aBreakPositionFT;
    ListBox             aBreakPositionLB;
    TriStateBox         aApplyCollBtn;
    ListBox             aApplyCollBox;
    FixedText           aPagenumText;
    NumericField        aPagenumEdit;

    FixedLine           aExtendFL;

    // Absatzteilung
    TriStateBox         aKeepTogetherBox;
    TriStateBox         aKeepParaBox;

    // Witwen/Waisen
    TriStateBox         aOrphanBox;
    NumericField        aOrphanRowNo;
    FixedText           aOrphanRowLabel;

    TriStateBox         aWidowBox;
    NumericField        aWidowRowNo;
    FixedText           aWidowRowLabel;

    sal_Bool                bPageBreak;
    sal_Bool                bHtmlMode;
    sal_uInt16              nStdPos;

#ifdef _SVX_PARAGRPH_CXX
    DECL_LINK( PageBreakHdl_Impl, TriStateBox* );
    DECL_LINK( KeepTogetherHdl_Impl, TriStateBox* );
    DECL_LINK( WidowHdl_Impl, TriStateBox* );
    DECL_LINK( OrphanHdl_Impl, TriStateBox* );
    DECL_LINK( HyphenClickHdl_Impl, TriStateBox* );
    DECL_LINK( ApplyCollClickHdl_Impl, TriStateBox* );
    DECL_LINK( PageBreakPosHdl_Impl, ListBox* );
    DECL_LINK( PageBreakTypeHdl_Impl, ListBox* );
#endif
    virtual void            PageCreated(SfxAllItemSet aSet); // add CHINA001
};
/* -----------------------------29.11.00 11:33--------------------------------

 ---------------------------------------------------------------------------*/
class SvxAsianTabPage : public SfxTabPage
{
    FixedLine       aOptionsFL;

    TriStateBox     aForbiddenRulesCB;
    TriStateBox     aHangingPunctCB;

    TriStateBox     aScriptSpaceCB;

    SvxAsianTabPage( Window* pParent, const SfxItemSet& rSet );

#ifdef _SVX_PARAGRPH_CXX
    DECL_LINK( ClickHdl_Impl, TriStateBox* );
#endif
public:
    ~SvxAsianTabPage();


    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

#endif // #ifndef _SVX_PARAGRPH_HXX


