/*************************************************************************
 *
 *  $RCSfile: optpage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-03-22 09:40:18 $
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
#ifndef _OPTPAGE_HXX
#define _OPTPAGE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif

#ifndef _SVX_FNTCTRL_HXX //autogen
#include <svx/fntctrl.hxx>
#endif

class SvStringsDtor;
class SfxPrinter;
class SwStdFontConfig;
class SwWrtShell;

/*-----------------31.08.96 10.09-------------------

--------------------------------------------------*/

class SwContentOptPage : public SfxTabPage
{
    //view
    FixedLine   aWindowGB;
    CheckBox    aHScrollBox;
    CheckBox    aVScrollBox;
    CheckBox    aHRulerCBox;
    ListBox     aHMetric;
    CheckBox    aVRulerCBox;
    ListBox     aVMetric;
    CheckBox    aSmoothCBox;
    //visual aids
    FixedLine   aLineGB;
    CheckBox    aTxtbegCB;
    CheckBox    aTblbegCB;
    CheckBox    aSectBoundsCB;
    CheckBox    aCrossCB;
    CheckBox    aSolidHandleCB;
    CheckBox    aBigHandleCB;

    //display
    FixedLine   aDispBox;
    CheckBox    aGrfCB;
    CheckBox    aTblCB;
    CheckBox    aDrwCB;
    CheckBox    aFldNameCB;
    CheckBox    aPostItCB;
    //highlighting
    FixedLine   aBackBox;
    CheckBox    aIdxEntryCB;
    CheckBox    aIdxBackCB;
    CheckBox    aFootBackCB;
    CheckBox    aFldBackCB;

public:
                        SwContentOptPage( Window* pParent,
                                           const SfxItemSet& rSet );
                        ~SwContentOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

/*-----------------31.08.96 10.10-------------------

--------------------------------------------------*/
/*
class SwLayoutOptPage : public SfxTabPage
{
                        SwLayoutOptPage( Window* pParent,
                                           const SfxItemSet& rSet );
                        ~SwLayoutOptPage();
public:
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

/*-------- OS 27.01.95 -----------------------------------
 TabPage Druckereinstellungen Zusaetze
--------------------------------------------------------- */


class SwAddPrinterTabPage : public SfxTabPage
{
    CheckBox        aGrfCB;
    CheckBox        aTabCB;
    CheckBox        aDrawCB;
    CheckBox        aCtrlFldCB;
    CheckBox        aBackgroundCB;
    CheckBox        aBlackFontCB;
    FixedLine        aGroup1;
    CheckBox        aLeftPageCB;
    CheckBox        aRightPageCB;
    CheckBox        aReverseCB;
    CheckBox        aProspectCB;
    FixedLine        aGroup2;
    RadioButton     aNoRB;
    RadioButton     aOnlyRB;
    RadioButton     aEndRB;
    RadioButton     aEndPageRB;
    FixedLine        aGroup3;
    FixedLine        aGroup4;
    CheckBox        aSingleJobsCB;
    CheckBox        aPaperFromSetupCB;
    FixedText       aFaxFT;
    ListBox         aFaxLB;

    BOOL        bAttrModified;
    BOOL        bPreview;

    void        Init();
                DECL_LINK( AutoClickHdl, CheckBox * );
                DECL_LINK( SelectHdl, ListBox * );


                SwAddPrinterTabPage( Window* pParent,
                                           const SfxItemSet& rSet );

public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    void                SetFax( const SvStringsDtor& );
    void                SelectFax( const String& );
    void                SetPreview(BOOL bPrev);

};

/*-----------------03.09.96 11.50-------------------

--------------------------------------------------*/

class SwStdFontTabPage : public SfxTabPage
{
    FixedText       aStandardLbl;
    ComboBox        aStandardBox;
    FixedText       aTitleLbl   ;
    ComboBox        aTitleBox   ;
    FixedText       aListLbl    ;
    ComboBox        aListBox    ;
    FixedText       aLabelLbl   ;
    ComboBox        aLabelBox   ;
    FixedText       aIdxLbl     ;
    ComboBox        aIdxBox     ;
    CheckBox        aDocOnlyCB  ;
    FixedLine        aStdChrFrm  ;
    PushButton      aStandardPB;

    String          sShellStd;
    String          sShellTitle;
    String          sShellList;
    String          sShellLabel;
    String          sShellIndex;

    SfxPrinter*         pPrt;
    SwStdFontConfig*    pFontConfig;
    SwWrtShell*         pWrtShell;
    // waren nur defaults vorhanden? wurden sie mit den Boxen ueberschrieben
    BOOL    bListDefault    :1;
    BOOL    bSetListDefault :1;
    BOOL    bLabelDefault   :1;
    BOOL    bSetLabelDefault :1;
    BOOL    bIdxDefault     :1;
    BOOL    bSetIdxDefault  :1;
    BOOL    bDeletePrinter :1;



    DECL_LINK( StandardHdl, PushButton * );
    DECL_LINK( ModifyHdl, ComboBox * );

            SwStdFontTabPage( Window* pParent,
                                       const SfxItemSet& rSet );
            ~SwStdFontTabPage();

public:
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

/*-----------------18.01.97 12.10-------------------

--------------------------------------------------*/

class SwTableOptionsTabPage : public SfxTabPage
{
    CheckBox    aHeaderCB;
    CheckBox    aRepeatHeaderCB;
    CheckBox    aDontSplitCB;
    CheckBox    aBorderCB;
    FixedLine   aTableGB;

    FixedLine   aTableInsertGB;
    CheckBox    aNumFormattingCB;
    CheckBox    aNumFmtFormattingCB;
    CheckBox    aNumAlignmentCB;

    FixedLine   aMoveGB;
    FixedText   aMoveFT;
    FixedText   aRowMoveFT;
    MetricField aRowMoveMF;
    FixedText   aColMoveFT;
    MetricField aColMoveMF;

    FixedText   aInsertFT;
    FixedText   aRowInsertFT;
    MetricField aRowInsertMF;
    FixedText   aColInsertFT;
    MetricField aColInsertMF;

    FixedText   aHandlingFT;
    RadioButton aFixRB;
    RadioButton aFixPropRB;
    RadioButton aVarRB;
    FixedText   aFixFT;
    FixedText   aFixPropFT;
    FixedText   aVarFT;

    SwWrtShell* pWrtShell;
    BOOL        bHTMLMode;

    DECL_LINK(CheckBoxHdl, CheckBox *pCB);


                SwTableOptionsTabPage( Window* pParent,
                                           const SfxItemSet& rSet );
                ~SwTableOptionsTabPage();

public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void SetWrtShell(SwWrtShell* pSh) {pWrtShell = pSh;}


};
/*-----------------19.04.97 13:17-------------------

--------------------------------------------------*/
class SwSourceViewOptionsTabPage : public SfxTabPage
{
    FixedLine        aColorGB;
    FixedText       aSGMLFT;
    ColorListBox    aSGMLLB;
    FixedText       aCommentFT;
    ColorListBox    aCommentLB;
    FixedText       aKeywdFT;
    ColorListBox    aKeywdLB;
    FixedText       aUnknownFT;
    ColorListBox    aUnknownLB;

                SwSourceViewOptionsTabPage( Window* pParent,
                                           const SfxItemSet& rSet );
                ~SwSourceViewOptionsTabPage();

public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

/*-----------------31.10.97 17:55-------------------
 TabPage fuer ShadowCrsr
--------------------------------------------------*/

class SwShdwCrsrOptionsTabPage : public SfxTabPage
{
    //nonprinting characters
    FixedLine   aUnprintBox;
    CheckBox    aParaCB;
    CheckBox    aSHyphCB;
    CheckBox    aSpacesCB;
    CheckBox    aHSpacesCB;
    CheckBox    aTabCB;
    CheckBox    aBreakCB;
    CheckBox    aHiddenCB;
    CheckBox    aHiddenParaCB;

    FixedLine       aFlagGB;
    CheckBox        aOnOffCB;

    FixedText       aFillModeFT;
    RadioButton     aFillMarginRB;
    RadioButton     aFillIndentRB;
    RadioButton     aFillTabRB;
    RadioButton     aFillSpaceRB;

    FixedText       aColorFT;
    ColorListBox    aColorLB;

    FixedLine       aCrsrOptGB;
    CheckBox        aCrsrInProtCB;

    SwShdwCrsrOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SwShdwCrsrOptionsTabPage();

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

/*-----------------------------------------------------------------------
    Beschreibung: Markierungsvorschau
 -----------------------------------------------------------------------*/

class SwMarkPreview : public Window
{
    Color       aTransColor;    // Transparente Farbe
    Color       aMarkColor;     // Farbe der Markierungen

    Rectangle   aPage;
    Rectangle   aLeftPagePrtArea;
    Rectangle   aRightPagePrtArea;

    USHORT      nMarkPos;

    void DrawRect(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor);
    void Paint(const Rectangle&);
    void PaintPage(const Rectangle &rRect);

public:

     SwMarkPreview(Window* pParent, const ResId& rResID);
    ~SwMarkPreview();

    inline void SetColor(Color aCol)    { aMarkColor = aCol; }
    inline void SetMarkPos(USHORT nPos) { nMarkPos = nPos; }
};

/*-----------------------------------------------------------------------
    Beschreibung: Redlining-Optionen
 -----------------------------------------------------------------------*/

class SwRedlineOptionsTabPage : public SfxTabPage
{
    FixedLine           aInsertGB;

    FixedText           aInsertFT;
    ListBox             aInsertLB;
    FixedText           aInsertColorFT;
    ColorListBox        aInsertColorLB;
    SvxFontPrevWindow   aInsertedPreviewWN;

    FixedText           aDeletedFT;
    ListBox             aDeletedLB;
    FixedText           aDeletedColorFT;
    ColorListBox        aDeletedColorLB;
    SvxFontPrevWindow   aDeletedPreviewWN;

    FixedText           aChangedFT;
    ListBox             aChangedLB;
    FixedText           aChangedColorFT;
    ColorListBox        aChangedColorLB;
    SvxFontPrevWindow   aChangedPreviewWN;

    FixedLine           aChangedGB;

    FixedText           aMarkPosFT;
    ListBox             aMarkPosLB;
    FixedText           aMarkColorFT;
    ColorListBox        aMarkColorLB;
    SwMarkPreview       aMarkPreviewWN;

    String              sAuthor;
    String              sNone;

    SwRedlineOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SwRedlineOptionsTabPage();

    DECL_LINK( AttribHdl, ListBox *pLB );
    DECL_LINK( ChangedMaskPrevHdl, ListBox *pLB = 0 );
    DECL_LINK( ColorHdl, ColorListBox *pColorLB );

    void                InitFontStyle(SvxFontPrevWindow& rExampleWin);

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

/*--------OS 11.01.95 -----------------------------------
 TabPage Testeinstellungen fuer SW
--------------------------------------------------------- */

#ifndef PRODUCT

class SwTestTabPage : public SfxTabPage
{
public:
                        SwTestTabPage( Window* pParent,
                                           const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

private:
    FixedLine aTestGBox;
    CheckBox aTest1CBox;
    CheckBox aTest2CBox;
    CheckBox aTest3CBox;
    CheckBox aTest4CBox;
    CheckBox aTest5CBox;
    CheckBox aTest6CBox;
    CheckBox aTest7CBox;
    CheckBox aTest8CBox;
    CheckBox aTest9CBox;
    CheckBox aTest10CBox;

    BOOL        bAttrModified;

    void        Init();
    DECL_LINK( AutoClickHdl, CheckBox * );

};
#endif //PRODUCT
#endif


