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
    std::unique_ptr<weld::CheckButton> m_xCrossCB;

    std::unique_ptr<weld::ComboBox> m_xHMetric;
    std::unique_ptr<weld::CheckButton> m_xVRulerCBox;
    std::unique_ptr<weld::CheckButton> m_xVRulerRightCBox;
    std::unique_ptr<weld::ComboBox> m_xVMetric;
    std::unique_ptr<weld::CheckButton> m_xSmoothCBox;

    std::unique_ptr<weld::CheckButton> m_xGrfCB;
    std::unique_ptr<weld::CheckButton> m_xTableCB;
    std::unique_ptr<weld::CheckButton> m_xDrwCB;
    std::unique_ptr<weld::CheckButton> m_xPostItCB;

    std::unique_ptr<weld::Frame> m_xSettingsFrame;
    std::unique_ptr<weld::Label> m_xSettingsLabel;
    std::unique_ptr<weld::Label> m_xMetricLabel;
    std::unique_ptr<weld::ComboBox> m_xMetricLB;

    std::unique_ptr<weld::CheckButton> m_xShowInlineTooltips;
    std::unique_ptr<weld::CheckButton> m_xShowOutlineContentVisibilityButton;
    std::unique_ptr<weld::CheckButton> m_xShowChangesInMargin;
    std::unique_ptr<weld::CheckButton> m_xFieldHiddenCB;
    std::unique_ptr<weld::CheckButton> m_xFieldHiddenParaCB;

    DECL_LINK(VertRulerHdl, weld::ToggleButton&, void);
public:
    SwContentOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwContentOptPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

// TabPage printer settings additions
class SwAddPrinterTabPage : public SfxTabPage
{
    OUString sNone;
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
    SwAddPrinterTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwAddPrinterTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
    void SetFax(const std::vector<OUString>& );
    void SetPreview(bool bPrev);
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

class SwStdFontTabPage : public SfxTabPage
{
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
    bool    m_bDisposePrinter :1;

    bool    m_bListHeightDefault    :1;
    bool    m_bLabelHeightDefault   :1;
    bool    m_bIndexHeightDefault     :1;

    sal_uInt8 m_nFontGroup; //fontcfg.hxx: FONT_GROUP_[STANDARD|CJK|CTL]

    OUString m_sScriptWestern;
    OUString m_sScriptAsian;
    OUString m_sScriptComplex;

    std::unique_ptr<weld::Label> m_xLabelFT;
    std::unique_ptr<weld::ComboBox> m_xStandardBox;
    std::unique_ptr<FontSizeBox> m_xStandardHeightLB;
    std::unique_ptr<weld::ComboBox> m_xTitleBox;
    std::unique_ptr<FontSizeBox> m_xTitleHeightLB;
    std::unique_ptr<weld::ComboBox> m_xListBox;
    std::unique_ptr<FontSizeBox> m_xListHeightLB;
    std::unique_ptr<weld::ComboBox> m_xLabelBox;
    std::unique_ptr<FontSizeBox> m_xLabelHeightLB;
    std::unique_ptr<weld::ComboBox> m_xIdxBox;
    std::unique_ptr<FontSizeBox> m_xIndexHeightLB;
    std::unique_ptr<weld::Button> m_xStandardPB;

    DECL_LINK(StandardHdl, weld::Button&, void );
    DECL_LINK(ModifyHdl, weld::ComboBox&, void );
    DECL_LINK(LoseFocusHdl, weld::Widget&, void );

public:
    SwStdFontTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);
    virtual ~SwStdFontTabPage() override;

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
    SwTableOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwTableOptionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

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
    std::unique_ptr<weld::Label> m_xTabLabel;
    std::unique_ptr<weld::CheckButton> m_xBreakCB;
    std::unique_ptr<weld::CheckButton> m_xCharHiddenCB;
    std::unique_ptr<weld::CheckButton> m_xBookmarkCB;
    std::unique_ptr<weld::Label> m_xBookmarkLabel;

    std::unique_ptr<weld::Frame> m_xDirectCursorFrame;
    std::unique_ptr<weld::CheckButton> m_xOnOffCB;

    std::unique_ptr<weld::ComboBox> m_xDirectCursorFillMode;
    std::unique_ptr<weld::Frame> m_xCursorProtFrame;
    std::unique_ptr<weld::Frame> m_xImageFrame;
    std::unique_ptr<weld::CheckButton> m_xCursorInProtCB;

    std::unique_ptr<weld::ComboBox> m_xDefaultAnchorType;

    std::unique_ptr<weld::CheckButton> m_xMathBaselineAlignmentCB;

public:
    SwShdwCursorOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwShdwCursorOptionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;

    virtual void PageCreated( const SfxAllItemSet& aSet ) override;
};

// mark preview
class SwMarkPreview : public weld::CustomWidgetController
{
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

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    void PaintPage(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect);
    void InitColors();

public:
    SwMarkPreview();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~SwMarkPreview() override;

    void SetColor(const Color& rCol) { m_aMarkCol = rCol; }
    void SetMarkPos(sal_uInt16 nPos) { nMarkPos = nPos; }
};

// redlining options
class SwRedlineOptionsTabPage : public SfxTabPage
{
    std::unique_ptr<weld::ComboBox> m_xInsertLB;
    std::unique_ptr<ColorListBox> m_xInsertColorLB;
    std::unique_ptr<SvxFontPrevWindow> m_xInsertedPreviewWN;
    std::unique_ptr<weld::CustomWeld> m_xInsertedPreview;

    std::unique_ptr<weld::ComboBox> m_xDeletedLB;
    std::unique_ptr<ColorListBox> m_xDeletedColorLB;
    std::unique_ptr<SvxFontPrevWindow> m_xDeletedPreviewWN;
    std::unique_ptr<weld::CustomWeld> m_xDeletedPreview;

    std::unique_ptr<weld::ComboBox> m_xChangedLB;
    std::unique_ptr<ColorListBox> m_xChangedColorLB;
    std::unique_ptr<SvxFontPrevWindow> m_xChangedPreviewWN;
    std::unique_ptr<weld::CustomWeld> m_xChangedPreview;

    std::unique_ptr<weld::ComboBox> m_xMarkPosLB;
    std::unique_ptr<ColorListBox> m_xMarkColorLB;
    std::unique_ptr<SwMarkPreview> m_xMarkPreviewWN;
    std::unique_ptr<weld::CustomWeld> m_xMarkPreview;

    DECL_LINK(AttribHdl, weld::ComboBox&, void);
    void ChangedMaskPrev();
    DECL_LINK(ChangedMaskPrevHdl, weld::ComboBox&, void);
    DECL_LINK(ChangedMaskColorPrevHdl, ColorListBox&, void);
    DECL_LINK(ColorHdl, ColorListBox&, void);

    static void InitFontStyle(SvxFontPrevWindow& rExampleWin, const OUString& rText);

public:
    SwRedlineOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwRedlineOptionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;
};

// TabPage test settings for SW

#ifdef DBG_UTIL

class SwTestTabPage : public SfxTabPage
{
public:
    SwTestTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwTestTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

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
    SwCompareOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwCompareOptionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual bool FillItemSet( SfxItemSet* rSet ) override;
    virtual void Reset( const SfxItemSet* rSet ) override;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
