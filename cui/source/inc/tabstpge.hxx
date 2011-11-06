/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_TABSTPGE_HXX
#define _SVX_TABSTPGE_HXX

// include ---------------------------------------------------------------

#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>

#include <editeng/tstpitem.hxx>
#include <svx/flagsdef.hxx>

// forward ---------------------------------------------------------------

class TabWin_Impl;

// define ----------------------------------------------------------------

// Bitfelder f"ur DisableControls()
//CHINA001 #define TABTYPE_LEFT     0x0001
//CHINA001 #define TABTYPE_RIGHT        0x0002
//CHINA001 #define TABTYPE_CENTER       0x0004
//CHINA001 #define TABTYPE_DEZIMAL      0x0008
//CHINA001 #define TABTYPE_ALL          0x000F
//CHINA001
//CHINA001 #define TABFILL_NONE     0x0010
//CHINA001 #define TABFILL_POINT        0x0020
//CHINA001 #define TABFILL_DASHLINE     0x0040
//CHINA001 #define TABFILL_SOLIDLINE    0x0080
//CHINA001 #define TABFILL_SPECIAL      0x0100
//CHINA001 #define TABFILL_ALL          0x01F0

// class SvxTabulatorTabPage ---------------------------------------------
/*
    {k:\svx\prototyp\dialog\tabstop.bmp}

    [Beschreibung]
    In dieser TabPage werden Tabulatoren verwaltet.

    [Items]
    <SvxTabStopItem><SID_ATTR_TABSTOP>
    <SfxUInt16Item><SID_ATTR_TABSTOP_DEFAULTS>
    <SfxUInt16Item><SID_ATTR_TABSTOP_POS>
    <SfxInt32Item><SID_ATTR_TABSTOP_OFFSET>
*/

class SvxTabulatorTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    ~SvxTabulatorTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                DisableControls( const sal_uInt16 nFlag );

protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

private:
    SvxTabulatorTabPage( Window* pParent, const SfxItemSet& rSet );

    // Tabulatoren und Positionen
    FixedLine       aTabLabel;
    MetricBox       aTabBox;
    FixedLine       aTabLabelVert;

    FixedLine       aTabTypeLabel;
    // TabType
    RadioButton     aLeftTab;
    RadioButton     aRightTab;
    RadioButton     aCenterTab;
    RadioButton     aDezTab;

    TabWin_Impl*    pLeftWin;
    TabWin_Impl*    pRightWin;
    TabWin_Impl*    pCenterWin;
    TabWin_Impl*    pDezWin;

    FixedText       aDezCharLabel;
    Edit            aDezChar;

    FixedLine       aFillLabel;
    // Fuellzeichen
    RadioButton     aNoFillChar;
    RadioButton     aFillPoints;
    RadioButton     aFillDashLine ;
    RadioButton     aFillSolidLine;
    RadioButton     aFillSpecial;
    Edit            aFillChar;
    // Buttons
    PushButton      aNewBtn;
    PushButton      aDelAllBtn;
    PushButton      aDelBtn;

    // lokale Variablen, interne Funktionen
    SvxTabStop      aAktTab;
    SvxTabStopItem  aNewTabs;
    long            nDefDist;
    FieldUnit       eDefUnit;
    sal_Bool            bCheck;

#ifdef _SVX_TABSTPGE_CXX
    void            InitTabPos_Impl( sal_uInt16 nPos = 0 );
    void            SetFillAndTabType_Impl();

    // Handler
    DECL_LINK( NewHdl_Impl, Button* );
    DECL_LINK( DelHdl_Impl, Button* );
    DECL_LINK( DelAllHdl_Impl, Button* );

    DECL_LINK( FillTypeCheckHdl_Impl, RadioButton* );
    DECL_LINK( TabTypeCheckHdl_Impl, RadioButton* );

    DECL_LINK( SelectHdl_Impl, MetricBox* );
    DECL_LINK( ModifyHdl_Impl, MetricBox* );
    DECL_LINK( GetFillCharHdl_Impl, Edit* );
    DECL_LINK( GetDezCharHdl_Impl, Edit* );
#endif
    virtual void            PageCreated(SfxAllItemSet aSet); // add CHINA001
};

#endif // #ifndef _SVX_TABSTPGE_HXX


