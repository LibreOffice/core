/*************************************************************************
 *
 *  $RCSfile: tablepg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWTABLEPG_HXX
#define _SWTABLEPG_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _ACTCTRL_HXX
#include <actctrl.hxx>
#endif
#include "prcntfld.hxx"
#include "swtypes.hxx"

class SwTabCols;
class SwWrtShell;
class SwTableRep;

struct TColumn
{
    SwTwips nWidth;
    BOOL    bVisible;
};

class SwFormatTablePage : public SfxTabPage
{
    GroupBox        aOptionsGB;
    FixedText       aNameFT;
    TableNameEdit   aNameED;
    FixedText       aWidthFT;
    PercentField    aWidthMF;
    CheckBox        aRelWidthCB;

    GroupBox        aPosFrm;
    RadioButton     aFullBtn;
    RadioButton     aLeftBtn;
    RadioButton     aFromLeftBtn;
    RadioButton     aRightBtn;
    RadioButton     aCenterBtn;
    RadioButton     aFreeBtn;

    GroupBox        aDistGB;
    FixedText       aLeftFT;
    PercentField    aLeftMF;
    FixedText       aRightFT;
    PercentField    aRightMF;
    FixedText       aTopFT;
    MetricField     aTopMF;
    FixedText       aBottomFT;
    MetricField     aBottomMF;

    SwTableRep*     pTblData;
    SwTwips         nSaveWidth;
    SwTwips         nMinTableWidth;
    USHORT          nOldAlign;
    BOOL            bModified;
    BOOL            bFull:1;

    void        Init();
    void        ModifyHdl( Edit* pEdit );

    DECL_LINK( AutoClickHdl, CheckBox * );
    DECL_LINK( RelWidthClickHdl, CheckBox * );
    DECL_LINK( RightModifyHdl, MetricField * );
    DECL_LINK( UpHdl, MetricField * );
    DECL_LINK( DownHdl, MetricField * );
    DECL_LINK( LoseFocusHdl, MetricField * );

public:
    SwFormatTablePage( Window* pParent, const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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
    CheckBox        aInverseCB;
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
    GroupBox        aColGB;

    SwTableRep*     pTblData;
    PercentField*   pFieldArr[MET_FIELDS];
    FixedText*      pTextArr[MET_FIELDS];
    SwTwips         nTableWidth;
    SwTwips         nMinWidth;
    USHORT          nNoOfCols;
    USHORT          nNoOfVisibleCols;
    //Breite merken, wenn auf autom. Ausrichtung gestellt wird
    USHORT          aValueTbl[MET_FIELDS];//primaere Zuordnung der MetricFields
    BOOL            bModified:1;
    BOOL            bModifyTbl:1;
    BOOL            bPercentMode:1;

    void        Init(BOOL bWeb);
    DECL_LINK( AutoClickHdl, CheckBox * );
    void        ModifyHdl( PercentField* pEdit );
    DECL_LINK( UpHdl, PercentField * );
    DECL_LINK( DownHdl, PercentField * );
    DECL_LINK( LoseFocusHdl, PercentField * );
    DECL_LINK( ModeHdl, CheckBox * );
    void        UpdateCols( USHORT nAktPos );
    SwTwips     GetVisibleWidth(USHORT nPos);
    void        SetVisibleWidth(USHORT nPos, SwTwips nNewWidth);
public:
    SwTableColumnPage( Window* pParent, const SfxItemSet& rSet );
    ~SwTableColumnPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

};

/*-----------------12.12.96 11.48-------------------
        Textfluá
--------------------------------------------------*/
class SwTextFlowPage : public SfxTabPage
{
    GroupBox        aFlowGB;
    CheckBox        aPgBrkCB;
    RadioButton     aPgBrkRB;
    RadioButton     aColBrkRB;
    RadioButton     aPgBrkBeforeRB;
    RadioButton     aPgBrkAfterRB;
    CheckBox        aPageCollCB;
    ListBox         aPageCollLB;
    FixedText       aPageNoFT;
    NumericField    aPageNoNF;
    CheckBox        aKeepCB;
    CheckBox        aSplitCB;
    CheckBox        aHeadLineCB;

    GroupBox        aVertOrientGB;
    RadioButton     aTopRB;
    RadioButton     aCenterRB;
    RadioButton     aBottomRB;

    SwWrtShell*     pShell;

    BOOL            bPageBreak;
    BOOL            bHtmlMode;


    DECL_LINK( PageBreakHdl_Impl, CheckBox* );
    DECL_LINK( ApplyCollClickHdl_Impl, CheckBox* );
    DECL_LINK( PageBreakPosHdl_Impl, RadioButton* );
    DECL_LINK( PageBreakTypeHdl_Impl, RadioButton* );


    SwTextFlowPage( Window* pParent, const SfxItemSet& rSet );
    ~SwTextFlowPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                SetShell(SwWrtShell* pSh);

    void                DisablePageBreak();
};

#endif
