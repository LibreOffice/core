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
#ifndef INCLUDED_CUI_SOURCE_INC_PARAGRPH_HXX
#define INCLUDED_CUI_SOURCE_INC_PARAGRPH_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/relfld.hxx>
#include <svx/paraprev.hxx>
#include <svx/frmdirlbox.hxx>
#include <vcl/lstbox.hxx>
#include <svx/flagsdef.hxx>

class SvxLineSpacingItem;

// class SvxStdParagraphTabPage ------------------------------------------
/*
    [Description]
    With this TabPage standard attributes of a paragraph can be set
    (indention, distance, alignment, line spacing).

    [Items]
    <SvxAdjustItem><SID_ATTR_PARA_ADJUST>
    <SvxLineSpacingItem><SID_ATTR_PARA_LINESPACE>
    <SvxULSpaceItem><SID_ATTR_ULSPACE>
    <SvxLRSpaceItem><SID_ATTR_LRSPACE>
*/

class SvxStdParagraphTabPage: public SfxTabPage
{
    friend class VclPtr<SvxStdParagraphTabPage>;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pStdRanges[];

private:
    SvxStdParagraphTabPage(TabPageParent pParent, const SfxItemSet& rSet);

    long                    nAbst;
    long                    nWidth;
    long                    nMinFixDist;
    bool                    bRelativeMode;
    OUString                sAbsDist;

    SvxParaPrevWindow m_aExampleWin;

    // indention
    std::unique_ptr<RelativeField> m_xLeftIndent;

    std::unique_ptr<weld::Label> m_xRightLabel;
    std::unique_ptr<RelativeField> m_xRightIndent;

    std::unique_ptr<weld::Label> m_xFLineLabel;
    std::unique_ptr<RelativeField> m_xFLineIndent;
    std::unique_ptr<weld::CheckButton> m_xAutoCB;

    // distance
    std::unique_ptr<RelativeField> m_xTopDist;
    std::unique_ptr<RelativeField> m_xBottomDist;
    std::unique_ptr<weld::CheckButton> m_xContextualCB;

    // line spacing
    std::unique_ptr<weld::ComboBox> m_xLineDist;
    std::unique_ptr<weld::MetricSpinButton> m_xLineDistAtPercentBox;
    std::unique_ptr<weld::MetricSpinButton> m_xLineDistAtMetricBox;
    std::unique_ptr<weld::Label> m_xLineDistAtLabel;
    std::unique_ptr<weld::Label> m_xAbsDist;

    // only writer
    std::unique_ptr<weld::Widget> m_xRegisterFL;
    std::unique_ptr<weld::CheckButton> m_xRegisterCB;

    // preview
    std::unique_ptr<weld::CustomWeld> m_xExampleWin;

    void                    SetLineSpacing_Impl( const SvxLineSpacingItem& rAttr );
    void                    Init_Impl();
    void                    UpdateExample_Impl();
    void                    ELRLoseFocus();

    DECL_LINK(LineDistHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ModifyHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(AutoHdl_Impl, weld::ToggleButton&, void);

protected:
    virtual void            ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet ) override;


public:
    virtual ~SvxStdParagraphTabPage() override;

    DECL_LINK(ELRLoseFocusHdl, weld::MetricSpinButton&, void);

    static VclPtr<SfxTabPage>      Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pStdRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            ChangesApplied() override;

    void                    EnableRelativeMode();
    void                    EnableRegisterMode();
    void                    EnableContextualMode();
    void                    EnableAutoFirstLine();
    void                    EnableAbsLineDist(long nMinTwip);
    void                    EnableNegativeMode();
    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxParaAlignTabPage ------------------------------------------------

class SvxParaAlignTabPage : public SfxTabPage
{
    friend class VclPtr<SvxParaAlignTabPage>;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pAlignRanges[];

    SvxParaPrevWindow m_aExampleWin;

    // alignment
    std::unique_ptr<weld::RadioButton> m_xLeft;
    std::unique_ptr<weld::RadioButton> m_xRight;
    std::unique_ptr<weld::RadioButton> m_xCenter;
    std::unique_ptr<weld::RadioButton> m_xJustify;
    std::unique_ptr<weld::Label> m_xLeftBottom;
    std::unique_ptr<weld::Label> m_xRightTop;

    std::unique_ptr<weld::Label> m_xLastLineFT;
    std::unique_ptr<weld::ComboBox> m_xLastLineLB;
    std::unique_ptr<weld::CheckButton> m_xExpandCB;

    std::unique_ptr<weld::CheckButton> m_xSnapToGridCB;

    //preview
    std::unique_ptr<weld::CustomWeld> m_xExampleWin;
    //vertical alignment
    std::unique_ptr<weld::Widget> m_xVertAlignFL;
    std::unique_ptr<weld::ComboBox> m_xVertAlignLB;

    std::unique_ptr<weld::Widget> m_xPropertiesFL;
    std::unique_ptr<svx::SvxFrameDirectionListBox>  m_xTextDirectionLB;

    DECL_LINK(AlignHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(LastLineHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(TextDirectionHdl_Impl, weld::ComboBox&, void);

    void                    UpdateExample_Impl();

    SvxParaAlignTabPage(TabPageParent pParent, const SfxItemSet& rSet);

protected:
    virtual void            ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet ) override;

public:
    virtual ~SvxParaAlignTabPage() override;

    static VclPtr<SfxTabPage>      Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pAlignRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            ChangesApplied() override;

    void                    EnableJustifyExt();
    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxExtParagraphTabPage ------------------------------------------
/*
    [Description]
    With this TabPage special attributes of a paragraph can be set
    (hyphenation, pagebreak, orphan, widow, ...).

    [Items]
    <SvxHyphenZoneItem><SID_ATTR_PARA_HYPHENZONE>
    <SvxFormatBreakItem><SID_ATTR_PARA_PAGEBREAK>
    <SvxFormatSplitItem><SID_ATTR_PARA_SPLIT>
    <SvxWidowsItem><SID_ATTR_PARA_WIDOWS>
    <SvxOrphansItem><SID_ATTR_PARA_ORPHANS>
*/

class SvxExtParagraphTabPage: public SfxTabPage
{
    friend class VclPtr<SvxExtParagraphTabPage>;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pExtRanges[];

public:
    virtual ~SvxExtParagraphTabPage() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pExtRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                DisablePageBreak();

protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

private:
    SvxExtParagraphTabPage(TabPageParent pParent, const SfxItemSet& rSet);

    bool                bPageBreak;
    bool                bHtmlMode;
    sal_uInt16          nStdPos;

    // hyphenation
    std::unique_ptr<weld::CheckButton> m_xHyphenBox;
    std::unique_ptr<weld::Label> m_xBeforeText;
    std::unique_ptr<weld::SpinButton> m_xExtHyphenBeforeBox;
    std::unique_ptr<weld::Label> m_xAfterText;
    std::unique_ptr<weld::SpinButton> m_xExtHyphenAfterBox;
    std::unique_ptr<weld::Label> m_xMaxHyphenLabel;
    std::unique_ptr<weld::SpinButton> m_xMaxHyphenEdit;

    // pagebreak
    std::unique_ptr<weld::CheckButton> m_xPageBreakBox;
    std::unique_ptr<weld::Label> m_xBreakTypeFT;
    std::unique_ptr<weld::ComboBox> m_xBreakTypeLB;
    std::unique_ptr<weld::Label> m_xBreakPositionFT;
    std::unique_ptr<weld::ComboBox> m_xBreakPositionLB;
    std::unique_ptr<weld::CheckButton> m_xApplyCollBtn;
    std::unique_ptr<weld::ComboBox> m_xApplyCollBox;
    std::unique_ptr<weld::CheckButton> m_xPageNumBox;
    std::unique_ptr<weld::SpinButton> m_xPagenumEdit;

    // paragraph division
    std::unique_ptr<weld::CheckButton> m_xKeepTogetherBox;
    std::unique_ptr<weld::CheckButton> m_xKeepParaBox;

    // orphan/widow
    std::unique_ptr<weld::CheckButton> m_xOrphanBox;
    std::unique_ptr<weld::SpinButton> m_xOrphanRowNo;
    std::unique_ptr<weld::Label> m_xOrphanRowLabel;

    std::unique_ptr<weld::CheckButton> m_xWidowBox;
    std::unique_ptr<weld::SpinButton> m_xWidowRowNo;
    std::unique_ptr<weld::Label> m_xWidowRowLabel;

    DECL_LINK(PageBreakHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(KeepTogetherHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(WidowHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(OrphanHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(HyphenClickHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ApplyCollClickHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(PageBreakPosHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(PageBreakTypeHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(PageNumBoxClickHdl_Impl, weld::ToggleButton&, void);

    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

class SvxAsianTabPage : public SfxTabPage
{
    friend class VclPtr<SvxAsianTabPage>;

    std::unique_ptr<weld::CheckButton> m_xForbiddenRulesCB;
    std::unique_ptr<weld::CheckButton> m_xHangingPunctCB;
    std::unique_ptr<weld::CheckButton> m_xScriptSpaceCB;

    SvxAsianTabPage(TabPageParent pParent, const SfxItemSet& rSet);

public:
    virtual ~SvxAsianTabPage() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rSet);
    static const sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

};

#endif // INCLUDED_CUI_SOURCE_INC_PARAGRPH_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
