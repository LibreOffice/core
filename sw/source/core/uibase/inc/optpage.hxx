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
#ifndef INCLUDED_SW_SOURCE_UI_INC_OPTPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_INC_OPTPAGE_HXX
#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>

#include <vcl/button.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/fntctrl.hxx>
#include <fontcfg.hxx>
class SfxPrinter;
class SwStdFontConfig;
class SwWrtShell;
class FontList;

/*--------------------------------------------------------
 Tools->Options->Writer->View
 Tools->Options->Writer/Web->View
--------------------------------------------------------- */
class SwContentOptPage : public SfxTabPage
{
    CheckBox*   m_pCrossCB;

    CheckBox*   m_pHScrollBox;
    CheckBox*   m_pVScrollBox;
    CheckBox*   m_pAnyRulerCB;
    CheckBox*   m_pHRulerCBox;
    ListBox*    m_pHMetric;
    CheckBox*   m_pVRulerCBox;
    CheckBox*   m_pVRulerRightCBox;
    ListBox*    m_pVMetric;
    CheckBox*   m_pSmoothCBox;

    CheckBox*   m_pGrfCB;
    CheckBox*   m_pTblCB;
    CheckBox*   m_pDrwCB;
    CheckBox*   m_pFldNameCB;
    CheckBox*   m_pPostItCB;

    VclFrame*   m_pSettingsFrame;
    FixedText*  m_pSettingsLabel;
    FixedText*  m_pMetricLabel;
    ListBox*    m_pMetricLB;

    DECL_LINK(VertRulerHdl, CheckBox*);
    DECL_LINK(AnyRulerHdl, CheckBox*);
public:
                        SwContentOptPage( Window* pParent,
                                           const SfxItemSet& rSet );
                        ~SwContentOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

};

/*--------------------------------------------------------
 TabPage printer settings additions
--------------------------------------------------------- */
class SwAddPrinterTabPage : public SfxTabPage
{
    CheckBox*       m_pGrfCB;
    CheckBox*       m_pCtrlFldCB;
    CheckBox*       m_pBackgroundCB;
    CheckBox*       m_pBlackFontCB;
    CheckBox*       m_pPrintHiddenTextCB;
    CheckBox*       m_pPrintTextPlaceholderCB;

    VclFrame*       m_pPagesFrame;
    CheckBox*       m_pLeftPageCB;
    CheckBox*       m_pRightPageCB;
    CheckBox*       m_pProspectCB;
    CheckBox*       m_pProspectCB_RTL;

    VclFrame*       m_pCommentsFrame;
    RadioButton*    m_pNoRB;
    RadioButton*    m_pOnlyRB;
    RadioButton*    m_pEndRB;
    RadioButton*    m_pEndPageRB;

    CheckBox*       m_pPrintEmptyPagesCB;
    CheckBox*       m_pPaperFromSetupCB;
    ListBox*        m_pFaxLB;

    OUString        sNone;

    sal_Bool        bAttrModified;
    sal_Bool        bPreview;

    void        Init();
                DECL_LINK(AutoClickHdl, void *);
                DECL_LINK(SelectHdl, void *);

                SwAddPrinterTabPage( Window* pParent,
                                           const SfxItemSet& rSet );
public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
    void                SetFax( const std::vector<OUString>& );
    void                SelectFax( const OUString& );
    void                SetPreview(sal_Bool bPrev);
    virtual void        PageCreated (SfxAllItemSet aSet) SAL_OVERRIDE;

};

class SwStdFontTabPage : public SfxTabPage
{
    FixedText*       pLabelFT;

    ComboBox*        pStandardBox;
    FontSizeBox*     pStandardHeightLB;
    ComboBox*        pTitleBox   ;
    FontSizeBox*     pTitleHeightLB;
    ComboBox*        pListBox    ;
    FontSizeBox*     pListHeightLB;
    ComboBox*        pLabelBox   ;
    FontSizeBox*     pLabelHeightLB;
    ComboBox*        pIdxBox     ;
    FontSizeBox*     pIndexHeightLB;
    CheckBox*        pDocOnlyCB  ;
    PushButton*      pStandardPB;

    OUString        sShellStd;
    OUString        sShellTitle;
    OUString        sShellList;
    OUString        sShellLabel;
    OUString        sShellIndex;

    SfxPrinter*         pPrt;
    FontList*           pFontList;
    SwStdFontConfig*    pFontConfig;
    SwWrtShell*         pWrtShell;
    LanguageType        eLanguage;
    // only defaults were there? they were signed with the boxes
    sal_Bool    bListDefault    :1;
    sal_Bool    bSetListDefault :1;
    sal_Bool    bLabelDefault   :1;
    sal_Bool    bSetLabelDefault :1;
    sal_Bool    bIdxDefault     :1;
    sal_Bool    bSetIdxDefault  :1;
    sal_Bool    bDeletePrinter :1;

    sal_Bool    bListHeightDefault    :1;
    sal_Bool    bSetListHeightDefault :1;
    sal_Bool    bLabelHeightDefault   :1;
    sal_Bool    bSetLabelHeightDefault :1;
    sal_Bool    bIndexHeightDefault     :1;
    sal_Bool    bSetIndexHeightDefault  :1;

    sal_uInt8 nFontGroup; //fontcfg.hxx: FONT_GROUP_[STANDARD|CJK|CTL]

    OUString sScriptWestern;
    OUString sScriptAsian;
    OUString sScriptComplex;

    DECL_LINK(StandardHdl, void *);
    DECL_LINK( ModifyHdl, ComboBox * );
    DECL_LINK( ModifyHeightHdl, FontSizeBox * );
    DECL_LINK( LoseFocusHdl, ComboBox * );

            SwStdFontTabPage( Window* pParent,
                                       const SfxItemSet& rSet );
            ~SwStdFontTabPage();

public:
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    void    SetFontMode(sal_uInt8 nGroup) {nFontGroup = nGroup;}
    virtual void        PageCreated (SfxAllItemSet aSet) SAL_OVERRIDE;
};

class SwTableOptionsTabPage : public SfxTabPage
{
    CheckBox*    pHeaderCB;
    CheckBox*    pRepeatHeaderCB;
    CheckBox*    pDontSplitCB;
    CheckBox*    pBorderCB;

    CheckBox*    pNumFormattingCB;
    CheckBox*    pNumFmtFormattingCB;
    CheckBox*    pNumAlignmentCB;

    MetricField* pRowMoveMF;
    MetricField* pColMoveMF;

    MetricField* pRowInsertMF;
    MetricField* pColInsertMF;

    RadioButton* pFixRB;
    RadioButton* pFixPropRB;
    RadioButton* pVarRB;

    SwWrtShell* pWrtShell;
    sal_Bool        bHTMLMode;

    DECL_LINK(CheckBoxHdl, void *);

                SwTableOptionsTabPage( Window* pParent,
                                           const SfxItemSet& rSet );
                ~SwTableOptionsTabPage();

public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    void SetWrtShell(SwWrtShell* pSh) {pWrtShell = pSh;}
    virtual void        PageCreated (SfxAllItemSet aSet) SAL_OVERRIDE;

};

/*--------------------------------------------------
 TabPage for ShadowCrsr
--------------------------------------------------*/
class SwShdwCrsrOptionsTabPage : public SfxTabPage
{
    //nonprinting characters
    CheckBox* m_pParaCB;
    CheckBox* m_pSHyphCB;
    CheckBox* m_pSpacesCB;
    CheckBox* m_pHSpacesCB;
    CheckBox* m_pTabCB;
    CheckBox* m_pBreakCB;
    CheckBox* m_pCharHiddenCB;
    CheckBox* m_pFldHiddenCB;
    CheckBox* m_pFldHiddenParaCB;

    VclFrame* m_pDirectCursorFrame;
    CheckBox* m_pOnOffCB;

    RadioButton* m_pFillMarginRB;
    RadioButton* m_pFillIndentRB;
    RadioButton* m_pFillTabRB;
    RadioButton* m_pFillSpaceRB;

    VclFrame* m_pCursorProtFrame;
    CheckBox* m_pCrsrInProtCB;

    CheckBox* m_pMathBaselineAlignmentCB;

    SwWrtShell *    m_pWrtShell;

    SwShdwCrsrOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SwShdwCrsrOptionsTabPage();

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    void    SetWrtShell( SwWrtShell * pSh ) { m_pWrtShell = pSh; }
    virtual void        PageCreated( SfxAllItemSet aSet ) SAL_OVERRIDE;
};

/*-----------------------------------------------------------------------
    Description: mark preview
 -----------------------------------------------------------------------*/
class SwMarkPreview : public Window
{
    Size            m_aInitialSize;

    Color           m_aBgCol;           // background
    Color           m_aTransCol;        // transparency
    Color           m_aMarkCol;         // marks
    Color           m_aLineCol;         // general lines
    Color           m_aShadowCol;       // shadow
    Color           m_aTxtCol;          // text
    Color           m_aPrintAreaCol;    // frame for print area

    Rectangle       aPage;
    Rectangle       aLeftPagePrtArea;
    Rectangle       aRightPagePrtArea;

    sal_uInt16          nMarkPos;

    using OutputDevice::DrawRect;
    void            DrawRect(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor);

    void            Paint(const Rectangle&) SAL_OVERRIDE;
    void            PaintPage(const Rectangle &rRect);
    void            InitColors( void );

protected:
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

public:
                    SwMarkPreview(Window* pParent, WinBits nWinBits);
    virtual         ~SwMarkPreview();

    inline void     SetColor(const Color& rCol) { m_aMarkCol = rCol; }
    inline void     SetMarkPos(sal_uInt16 nPos) { nMarkPos = nPos; }
    virtual Size    GetOptimalSize() const SAL_OVERRIDE;
};

/*-----------------------------------------------------------------------
    Description: redlining options
 -----------------------------------------------------------------------*/
class SwRedlineOptionsTabPage : public SfxTabPage
{
    ListBox*             pInsertLB;
    ColorListBox*        pInsertColorLB;
    SvxFontPrevWindow*   pInsertedPreviewWN;

    ListBox*             pDeletedLB;
    ColorListBox*        pDeletedColorLB;
    SvxFontPrevWindow*   pDeletedPreviewWN;

    ListBox*             pChangedLB;
    ColorListBox*        pChangedColorLB;
    SvxFontPrevWindow*   pChangedPreviewWN;

    ListBox*             pMarkPosLB;
    ColorListBox*        pMarkColorLB;
    SwMarkPreview*       pMarkPreviewWN;

    OUString             sAuthor;
    OUString             sNone;

    SwRedlineOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SwRedlineOptionsTabPage();

    DECL_LINK( AttribHdl, ListBox *pLB );
    DECL_LINK(ChangedMaskPrevHdl, void * = 0);
    DECL_LINK( ColorHdl, ColorListBox *pColorLB );

    void                InitFontStyle(SvxFontPrevWindow& rExampleWin);

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

/*-------------------------------------------------------
 TabPage test settings for SW
--------------------------------------------------------- */

#ifdef DBG_UTIL

class SwTestTabPage : public SfxTabPage
{
public:
                        SwTestTabPage( Window* pParent,
                                           const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

private:
    CheckBox* m_pTest1CBox;
    CheckBox* m_pTest2CBox;
    CheckBox* m_pTest3CBox;
    CheckBox* m_pTest4CBox;
    CheckBox* m_pTest5CBox;
    CheckBox* m_pTest6CBox;
    CheckBox* m_pTest7CBox;
    CheckBox* m_pTest8CBox;
    CheckBox* m_pTest9CBox;
    CheckBox* m_pTest10CBox;

    sal_Bool        bAttrModified;

    void        Init();
    DECL_LINK(AutoClickHdl, void *);

};
#endif // DBG_UTIL

class SwCompareOptionsTabPage : public SfxTabPage
{
    RadioButton*  m_pAutoRB;
    RadioButton*  m_pWordRB;
    RadioButton*  m_pCharRB;

    CheckBox*     m_pRsidCB;
    CheckBox*     m_pIgnoreCB;
    NumericField* m_pLenNF;

    SwCompareOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SwCompareOptionsTabPage();

    DECL_LINK(ComparisonHdl, void *);
    DECL_LINK(IgnoreHdl, void *);

public:

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
