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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OPTPAGE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OPTPAGE_HXX
#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>

#include <vcl/button.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/colorbox.hxx>
#include <svx/fntctrl.hxx>
#include "fontcfg.hxx"
class SfxPrinter;
class SwStdFontConfig;
class SwWrtShell;
class FontList;

// Tools->Options->Writer->View
// Tools->Options->Writer/Web->View
class SwContentOptPage : public SfxTabPage
{
    VclPtr<CheckBox>   m_pCrossCB;

    VclPtr<ListBox>    m_pHMetric;
    VclPtr<CheckBox>   m_pVRulerCBox;
    VclPtr<CheckBox>   m_pVRulerRightCBox;
    VclPtr<ListBox>    m_pVMetric;
    VclPtr<CheckBox>   m_pSmoothCBox;

    VclPtr<CheckBox>   m_pGrfCB;
    VclPtr<CheckBox>   m_pTableCB;
    VclPtr<CheckBox>   m_pDrwCB;
    VclPtr<CheckBox>   m_pFieldNameCB;
    VclPtr<CheckBox>   m_pPostItCB;

    VclPtr<VclFrame>   m_pSettingsFrame;
    VclPtr<FixedText>  m_pSettingsLabel;
    VclPtr<FixedText>  m_pMetricLabel;
    VclPtr<ListBox>    m_pMetricLB;

    DECL_LINK(VertRulerHdl, Button*, void);
public:
    SwContentOptPage( vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwContentOptPage() override;

    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

// TabPage printer settings additions
class SwAddPrinterTabPage : public SfxTabPage
{
    VclPtr<CheckBox>       m_pGrfCB;
    VclPtr<CheckBox>       m_pCtrlFieldCB;
    VclPtr<CheckBox>       m_pBackgroundCB;
    VclPtr<CheckBox>       m_pBlackFontCB;
    VclPtr<CheckBox>       m_pPrintHiddenTextCB;
    VclPtr<CheckBox>       m_pPrintTextPlaceholderCB;

    VclPtr<VclFrame>       m_pPagesFrame;
    VclPtr<CheckBox>       m_pLeftPageCB;
    VclPtr<CheckBox>       m_pRightPageCB;
    VclPtr<CheckBox>       m_pProspectCB;
    VclPtr<CheckBox>       m_pProspectCB_RTL;

    VclPtr<VclFrame>       m_pCommentsFrame;
    VclPtr<RadioButton>    m_pNoRB;
    VclPtr<RadioButton>    m_pOnlyRB;
    VclPtr<RadioButton>    m_pEndRB;
    VclPtr<RadioButton>    m_pEndPageRB;
    VclPtr<RadioButton>    m_pInMarginsRB;

    VclPtr<CheckBox>       m_pPrintEmptyPagesCB;
    VclPtr<CheckBox>       m_pPaperFromSetupCB;
    VclPtr<ListBox>        m_pFaxLB;

    OUString sNone;

    bool bAttrModified;
    bool bPreview;

    DECL_LINK(AutoClickHdl, Button*, void);
    DECL_LINK(SelectHdl, ListBox&, void);

public:
    SwAddPrinterTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwAddPrinterTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
    void SetFax(const std::vector<OUString>& );
    void SetPreview(bool bPrev);
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

class SwStdFontTabPage : public SfxTabPage
{
    friend class VclPtr<SwStdFontTabPage>;
    VclPtr<FixedText>       m_pLabelFT;

    VclPtr<ComboBox>        m_pStandardBox;
    VclPtr<FontSizeBox>     m_pStandardHeightLB;
    VclPtr<ComboBox>        m_pTitleBox   ;
    VclPtr<FontSizeBox>     m_pTitleHeightLB;
    VclPtr<ComboBox>        m_pListBox    ;
    VclPtr<FontSizeBox>     m_pListHeightLB;
    VclPtr<ComboBox>        m_pLabelBox   ;
    VclPtr<FontSizeBox>     m_pLabelHeightLB;
    VclPtr<ComboBox>        m_pIdxBox     ;
    VclPtr<FontSizeBox>     m_pIndexHeightLB;
    VclPtr<PushButton>      m_pStandardPB;

    OUString m_sShellStd;
    OUString m_sShellTitle;
    OUString m_sShellList;
    OUString m_sShellLabel;
    OUString m_sShellIndex;

    VclPtr<SfxPrinter> m_pPrt;
    FontList* m_pFontList;
    SwStdFontConfig* m_pFontConfig;
    SwWrtShell* m_pWrtShell;
    LanguageType m_eLanguage;

    // only defaults were there? they were signed with the boxes
    bool    m_bListDefault    :1;
    bool    m_bSetListDefault :1;
    bool    m_bLabelDefault   :1;
    bool    m_bSetLabelDefault :1;
    bool    m_bIdxDefault     :1;
    bool    m_bSetIdxDefault  :1;

    bool    m_bListHeightDefault    :1;
    bool    m_bSetListHeightDefault :1;
    bool    m_bLabelHeightDefault   :1;
    bool    m_bSetLabelHeightDefault :1;
    bool    m_bIndexHeightDefault     :1;
    bool    m_bSetIndexHeightDefault  :1;

    sal_uInt8 m_nFontGroup; //fontcfg.hxx: FONT_GROUP_[STANDARD|CJK|CTL]

    OUString m_sScriptWestern;
    OUString m_sScriptAsian;
    OUString m_sScriptComplex;

    DECL_LINK(StandardHdl, Button*, void );
    DECL_LINK(ModifyHdl, Edit&, void );
    DECL_LINK(ModifyHeightHdl, Edit&, void );
    DECL_LINK(LoseFocusHdl, Control&, void );

    SwStdFontTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwStdFontTabPage() override;
    virtual void dispose() override;

public:
    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

class SwTableOptionsTabPage : public SfxTabPage
{
    VclPtr<CheckBox>    m_pHeaderCB;
    VclPtr<CheckBox>    m_pRepeatHeaderCB;
    VclPtr<CheckBox>    m_pDontSplitCB;
    VclPtr<CheckBox>    m_pBorderCB;

    VclPtr<CheckBox>    m_pNumFormattingCB;
    VclPtr<CheckBox>    m_pNumFormatFormattingCB;
    VclPtr<CheckBox>    m_pNumAlignmentCB;

    VclPtr<MetricField> m_pRowMoveMF;
    VclPtr<MetricField> m_pColMoveMF;

    VclPtr<MetricField> m_pRowInsertMF;
    VclPtr<MetricField> m_pColInsertMF;

    VclPtr<RadioButton> m_pFixRB;
    VclPtr<RadioButton> m_pFixPropRB;
    VclPtr<RadioButton> m_pVarRB;

    SwWrtShell* m_pWrtShell;
    bool        m_bHTMLMode;

    DECL_LINK(CheckBoxHdl, Button*, void);

public:
    SwTableOptionsTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwTableOptionsTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    virtual void PageCreated( const SfxAllItemSet& aSet) override;

};

// TabPage for ShadowCursor
class SwShdwCursorOptionsTabPage : public SfxTabPage
{
    //nonprinting characters
    VclPtr<CheckBox> m_pParaCB;
    VclPtr<CheckBox> m_pSHyphCB;
    VclPtr<CheckBox> m_pSpacesCB;
    VclPtr<CheckBox> m_pHSpacesCB;
    VclPtr<CheckBox> m_pTabCB;
    VclPtr<CheckBox> m_pBreakCB;
    VclPtr<CheckBox> m_pCharHiddenCB;
    VclPtr<CheckBox> m_pFieldHiddenCB;
    VclPtr<CheckBox> m_pFieldHiddenParaCB;

    VclPtr<VclFrame> m_pDirectCursorFrame;
    VclPtr<CheckBox> m_pOnOffCB;

    VclPtr<RadioButton> m_pFillMarginRB;
    VclPtr<RadioButton> m_pFillIndentRB;
    VclPtr<RadioButton> m_pFillTabRB;
    VclPtr<RadioButton> m_pFillTabAndSpaceRB;
    VclPtr<RadioButton> m_pFillSpaceRB;

    VclPtr<VclFrame> m_pCursorProtFrame;
    VclPtr<CheckBox> m_pCursorInProtCB;

    VclPtr<CheckBox> m_pMathBaselineAlignmentCB;

    SwWrtShell *    m_pWrtShell;

public:
    SwShdwCursorOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwShdwCursorOptionsTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;

    virtual void PageCreated( const SfxAllItemSet& aSet ) override;
};

// mark preview
class SwMarkPreview : public vcl::Window
{
    Size m_aInitialSize;

    Color m_aBgCol;    // background
    Color m_aTransCol; // transparency
    Color m_aMarkCol;  // marks
    Color m_aLineCol;  // general lines
    Color m_aShadowCol; // shadow
    Color m_aTextCol;    // text
    Color m_aPrintAreaCol; // frame for print area

    tools::Rectangle aPage;
    tools::Rectangle aLeftPagePrtArea;
    tools::Rectangle aRightPagePrtArea;

    sal_uInt16 nMarkPos;

    void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    void PaintPage(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect);
    void InitColors();

protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SwMarkPreview(vcl::Window* pParent, WinBits nWinBits);
    virtual ~SwMarkPreview() override;

    void SetColor(const Color& rCol) { m_aMarkCol = rCol; }
    void SetMarkPos(sal_uInt16 nPos) { nMarkPos = nPos; }
    virtual Size GetOptimalSize() const override;
};

// redlining options
class SwRedlineOptionsTabPage : public SfxTabPage
{
    VclPtr<ListBox>             m_pInsertLB;
    VclPtr<SvxColorListBox>     m_pInsertColorLB;
    VclPtr<SvxFontPrevWindow>   m_pInsertedPreviewWN;

    VclPtr<ListBox>             m_pDeletedLB;
    VclPtr<SvxColorListBox>     m_pDeletedColorLB;
    VclPtr<SvxFontPrevWindow>   m_pDeletedPreviewWN;

    VclPtr<ListBox>             m_pChangedLB;
    VclPtr<SvxColorListBox>     m_pChangedColorLB;
    VclPtr<SvxFontPrevWindow>   m_pChangedPreviewWN;

    VclPtr<ListBox>             m_pMarkPosLB;
    VclPtr<SvxColorListBox>     m_pMarkColorLB;
    VclPtr<SwMarkPreview>       m_pMarkPreviewWN;

    VclPtr<CheckBox>            m_pShowChangesTooltip;

    DECL_LINK(AttribHdl, ListBox&, void);
    void ChangedMaskPrev();
    DECL_LINK(ChangedMaskPrevHdl, ListBox&, void);
    DECL_LINK(ChangedMaskColorPrevHdl, SvxColorListBox&, void);
    DECL_LINK(ColorHdl, SvxColorListBox&, void);

    static void InitFontStyle(SvxFontPrevWindow& rExampleWin);

public:
    SwRedlineOptionsTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwRedlineOptionsTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;
};

// TabPage test settings for SW

#ifdef DBG_UTIL

class SwTestTabPage : public SfxTabPage
{
public:
    SwTestTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwTestTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;

private:
    VclPtr<CheckBox> m_pTest1CBox;
    VclPtr<CheckBox> m_pTest2CBox;
    VclPtr<CheckBox> m_pTest3CBox;
    VclPtr<CheckBox> m_pTest4CBox;
    VclPtr<CheckBox> m_pTest5CBox;
    VclPtr<CheckBox> m_pTest6CBox;
    VclPtr<CheckBox> m_pTest7CBox;
    VclPtr<CheckBox> m_pTest8CBox;
    VclPtr<CheckBox> m_pTest9CBox;
    VclPtr<CheckBox> m_pTest10CBox;

    bool bAttrModified;

    void Init();
    DECL_LINK(AutoClickHdl, Button*, void);
};
#endif // DBG_UTIL

class SwCompareOptionsTabPage : public SfxTabPage
{
    VclPtr<RadioButton>  m_pAutoRB;
    VclPtr<RadioButton>  m_pWordRB;
    VclPtr<RadioButton>  m_pCharRB;

    VclPtr<CheckBox>     m_pRsidCB;
    VclPtr<CheckBox>     m_pIgnoreCB;
    VclPtr<NumericField> m_pLenNF;
    VclPtr<CheckBox>     m_pStoreRsidCB;

    DECL_LINK(ComparisonHdl, Button*, void);
    DECL_LINK(IgnoreHdl, Button*, void);

public:
    SwCompareOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwCompareOptionsTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
