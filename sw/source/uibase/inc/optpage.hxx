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

#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/weld.hxx>
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

    VclPtr<CheckBox>   m_pShowInlineTooltips;
    VclPtr<CheckBox>   m_pFieldHiddenCB;
    VclPtr<CheckBox>   m_pFieldHiddenParaCB;
    DECL_LINK(VertRulerHdl, Button*, void);
public:
    SwContentOptPage( vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwContentOptPage() override;

    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

// TabPage printer settings additions
class SwAddPrinterTabPage : public SfxTabPage
{
    OUString const sNone;
    bool bAttrModified;
    bool bPreview;

    std::unique_ptr<weld::CheckButton>  m_xGrfCB;
    std::unique_ptr<weld::CheckButton>  m_xCtrlFieldCB;
    std::unique_ptr<weld::CheckButton>  m_xBackgroundCB;
    std::unique_ptr<weld::CheckButton>  m_xBlackFontCB;
    std::unique_ptr<weld::CheckButton>  m_xPrintHiddenTextCB;
    std::unique_ptr<weld::CheckButton>  m_xPrintTextPlaceholderCB;

    std::unique_ptr<weld::Widget>       m_xPagesFrame;
    std::unique_ptr<weld::CheckButton>  m_xLeftPageCB;
    std::unique_ptr<weld::CheckButton>  m_xRightPageCB;
    std::unique_ptr<weld::CheckButton>  m_xProspectCB;
    std::unique_ptr<weld::CheckButton>  m_xProspectCB_RTL;

    std::unique_ptr<weld::Widget>       m_xCommentsFrame;
    std::unique_ptr<weld::RadioButton>  m_xNoRB;
    std::unique_ptr<weld::RadioButton>  m_xOnlyRB;
    std::unique_ptr<weld::RadioButton>  m_xEndRB;
    std::unique_ptr<weld::RadioButton>  m_xEndPageRB;
    std::unique_ptr<weld::RadioButton>  m_xInMarginsRB;

    std::unique_ptr<weld::CheckButton>  m_xPrintEmptyPagesCB;
    std::unique_ptr<weld::CheckButton>  m_xPaperFromSetupCB;
    std::unique_ptr<weld::ComboBox> m_xFaxLB;

    DECL_LINK(AutoClickHdl, weld::ToggleButton&, void);
    DECL_LINK(SelectHdl, weld::ComboBox&, void);

public:
    SwAddPrinterTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwAddPrinterTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

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
    std::unique_ptr<FontList> m_pFontList;
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
    bool    m_bLabelHeightDefault   :1;
    bool    m_bIndexHeightDefault     :1;

    sal_uInt8 m_nFontGroup; //fontcfg.hxx: FONT_GROUP_[STANDARD|CJK|CTL]

    OUString const m_sScriptWestern;
    OUString const m_sScriptAsian;
    OUString const m_sScriptComplex;

    DECL_LINK(StandardHdl, Button*, void );
    DECL_LINK(ModifyHdl, Edit&, void );
    DECL_LINK(LoseFocusHdl, Control&, void );

    SwStdFontTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwStdFontTabPage() override;
    virtual void dispose() override;

public:
    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

class SwTableOptionsTabPage : public SfxTabPage
{
    SwWrtShell* m_pWrtShell;
    bool        m_bHTMLMode;

    std::unique_ptr<weld::CheckButton> m_xHeaderCB;
    std::unique_ptr<weld::CheckButton> m_xRepeatHeaderCB;
    std::unique_ptr<weld::CheckButton> m_xDontSplitCB;
    std::unique_ptr<weld::CheckButton> m_xBorderCB;

    std::unique_ptr<weld::CheckButton> m_xNumFormattingCB;
    std::unique_ptr<weld::CheckButton> m_xNumFormatFormattingCB;
    std::unique_ptr<weld::CheckButton> m_xNumAlignmentCB;

    std::unique_ptr<weld::MetricSpinButton> m_xRowMoveMF;
    std::unique_ptr<weld::MetricSpinButton> m_xColMoveMF;

    std::unique_ptr<weld::MetricSpinButton> m_xRowInsertMF;
    std::unique_ptr<weld::MetricSpinButton> m_xColInsertMF;

    std::unique_ptr<weld::RadioButton> m_xFixRB;
    std::unique_ptr<weld::RadioButton> m_xFixPropRB;
    std::unique_ptr<weld::RadioButton> m_xVarRB;

    DECL_LINK(CheckBoxHdl, weld::Button&, void);

public:
    SwTableOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwTableOptionsTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    virtual void PageCreated( const SfxAllItemSet& aSet) override;

};

// TabPage for ShadowCursor
class SwShdwCursorOptionsTabPage : public SfxTabPage
{
    SwWrtShell *    m_pWrtShell;

    //nonprinting characters
    std::unique_ptr<weld::CheckButton> m_xParaCB;
    std::unique_ptr<weld::CheckButton> m_xSHyphCB;
    std::unique_ptr<weld::CheckButton> m_xSpacesCB;
    std::unique_ptr<weld::CheckButton> m_xHSpacesCB;
    std::unique_ptr<weld::CheckButton> m_xTabCB;
    std::unique_ptr<weld::CheckButton> m_xBreakCB;
    std::unique_ptr<weld::CheckButton> m_xCharHiddenCB;

    std::unique_ptr<weld::Frame> m_xDirectCursorFrame;
    std::unique_ptr<weld::CheckButton> m_xOnOffCB;

    std::unique_ptr<weld::RadioButton> m_xFillMarginRB;
    std::unique_ptr<weld::RadioButton> m_xFillIndentRB;
    std::unique_ptr<weld::RadioButton> m_xFillTabRB;
    std::unique_ptr<weld::RadioButton> m_xFillTabAndSpaceRB;
    std::unique_ptr<weld::RadioButton> m_xFillSpaceRB;

    std::unique_ptr<weld::Frame> m_xCursorProtFrame;
    std::unique_ptr<weld::CheckButton> m_xCursorInProtCB;

    std::unique_ptr<weld::CheckButton> m_xMathBaselineAlignmentCB;

public:
    SwShdwCursorOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwShdwCursorOptionsTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;

    virtual void PageCreated( const SfxAllItemSet& aSet ) override;
};

// mark preview
class SwMarkPreview : public vcl::Window
{
    Size m_aInitialSize;

    Color m_aBgCol;    // background
    Color const m_aTransCol; // transparency
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

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;
};

// TabPage test settings for SW

#ifdef DBG_UTIL

class SwTestTabPage : public SfxTabPage
{
public:
    SwTestTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwTestTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;

private:
    bool bAttrModified;

    std::unique_ptr<weld::CheckButton> m_xTest1CBox;
    std::unique_ptr<weld::CheckButton> m_xTest2CBox;
    std::unique_ptr<weld::CheckButton> m_xTest3CBox;
    std::unique_ptr<weld::CheckButton> m_xTest4CBox;
    std::unique_ptr<weld::CheckButton> m_xTest5CBox;
    std::unique_ptr<weld::CheckButton> m_xTest6CBox;
    std::unique_ptr<weld::CheckButton> m_xTest7CBox;
    std::unique_ptr<weld::CheckButton> m_xTest8CBox;
    std::unique_ptr<weld::CheckButton> m_xTest9CBox;
    std::unique_ptr<weld::CheckButton> m_xTest10CBox;

    void Init();
    DECL_LINK(AutoClickHdl, weld::Button&, void);
};
#endif // DBG_UTIL

class SwCompareOptionsTabPage : public SfxTabPage
{
    std::unique_ptr<weld::RadioButton> m_xAutoRB;
    std::unique_ptr<weld::RadioButton> m_xWordRB;
    std::unique_ptr<weld::RadioButton> m_xCharRB;
    std::unique_ptr<weld::CheckButton> m_xRsidCB;
    std::unique_ptr<weld::CheckButton> m_xIgnoreCB;
    std::unique_ptr<weld::SpinButton> m_xLenNF;
    std::unique_ptr<weld::CheckButton> m_xStoreRsidCB;

    DECL_LINK(ComparisonHdl, weld::Button&, void);
    DECL_LINK(IgnoreHdl, weld::Button&, void);

public:
    SwCompareOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwCompareOptionsTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
