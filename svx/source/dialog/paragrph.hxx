/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paragrph.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:24:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_PARAGRPH_HXX
#define _SVX_PARAGRPH_HXX

// include ---------------------------------------------------------------

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_RELFLD_HXX
#include "relfld.hxx"       // SvxRelativeField
#endif
#ifndef _SVX_PARAPREV_HXX
#include "paraprev.hxx"     // Preview
#endif
#ifndef _SVX_FRMDIRLBOX_HXX
#include "frmdirlbox.hxx"
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include "flagsdef.hxx"
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
    BOOL                    bRelativeMode;
    BOOL                    bNegativeIndents;

#ifdef _SVX_PARAGRPH_CXX
    void                    SetLineSpacing_Impl( const SvxLineSpacingItem& rAttr );
    void                    Init_Impl();
    void                    UpdateExample_Impl( BOOL bAll = FALSE );

    DECL_LINK( LineDistHdl_Impl, ListBox* );
    DECL_LINK( ModifyHdl_Impl, SvxRelativeField* );
    DECL_LINK( AutoHdl_Impl, CheckBox* );
#endif

protected:
    virtual int             DeactivatePage( SfxItemSet* pSet = 0 );

public:
    DECL_LINK( ELRLoseFocusHdl, Edit* );

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*          GetRanges();

    virtual BOOL            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );


    void                    SetPageWidth( USHORT nPageWidth );
    void                    SetMaxDistance( USHORT nMaxDist );
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

    void                    UpdateExample_Impl( BOOL bAll = FALSE );
#endif

                            SvxParaAlignTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual                 ~SvxParaAlignTabPage();

protected:
    virtual int             DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*          GetRanges();

    virtual BOOL            FillItemSet( SfxItemSet& rSet );
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
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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

    BOOL                bPageBreak;
    BOOL                bHtmlMode;
    USHORT              nStdPos;

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
    TriStateBox     aAllowWordBreakCB;
    TriStateBox     aHangingPunctCB;

    FixedLine       aCharDistFL;
    TriStateBox     aPuntuationCB;
    TriStateBox     aScriptSpaceCB;
    TriStateBox     aAdjustNumbersCB;

    SvxAsianTabPage( Window* pParent, const SfxItemSet& rSet );

#ifdef _SVX_PARAGRPH_CXX
    DECL_LINK( ClickHdl_Impl, TriStateBox* );
#endif
public:
    ~SvxAsianTabPage();


    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rSet );
    static USHORT*      GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

#endif // #ifndef _SVX_PARAGRPH_HXX


