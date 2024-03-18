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
#pragma once

#include <sfx2/tabdlg.hxx>
#include <svx/relfld.hxx>
#include <svx/paraprev.hxx>
#include <svx/frmdirlbox.hxx>

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
    static const WhichRangesContainer pStdRanges;

private:
    tools::Long                    nWidth;
    tools::Long                    nMinFixDist;
    bool                    bRelativeMode;
    OUString                sAbsDist;

    SvxParaPrevWindow m_aExampleWin;

    // indentation
    bool m_bSplitLRSpace = false; ///< which items to use?
    SvxRelativeField m_aLeftIndent;

    SvxRelativeField m_aRightIndent;

    std::unique_ptr<weld::Label> m_xFLineLabel;
    SvxRelativeField m_aFLineIndent;
    std::unique_ptr<weld::CheckButton> m_xAutoCB;

    // distance
    SvxRelativeField m_aTopDist;
    SvxRelativeField m_aBottomDist;
    std::unique_ptr<weld::CheckButton> m_xContextualCB;

    // line spacing
    std::unique_ptr<weld::ComboBox> m_xLineDist;
    std::unique_ptr<weld::MetricSpinButton> m_xLineDistAtPercentBox;
    std::unique_ptr<weld::MetricSpinButton> m_xLineDistAtMetricBox;
    std::unique_ptr<weld::MetricSpinButton> m_xLineDistAtPlaceHolderBox;
    std::unique_ptr<weld::Label> m_xLineDistAtLabel;
    std::unique_ptr<weld::Label> m_xAbsDist;

    // only writer
    std::unique_ptr<weld::CheckButton> m_xRegisterCB;

    // preview
    std::unique_ptr<weld::CustomWeld> m_xExampleWin;

    void                    SetLineSpacing_Impl( const SvxLineSpacingItem& rAttr );
    void                    Init_Impl();
    void                    UpdateExample_Impl();
    void                    ELRLoseFocus();

    DECL_LINK(LineDistPopupHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(LineDistHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ModifyHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(AutoHdl_Impl, weld::Toggleable&, void);

    bool m_bLineDistToggled = false;

protected:
    virtual void            ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet ) override;


public:
    SvxStdParagraphTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );
    virtual ~SvxStdParagraphTabPage() override;

    DECL_LINK(ELRLoseFocusHdl, weld::MetricSpinButton&, void);

    static WhichRangesContainer GetRanges() { return pStdRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            ChangesApplied() override;

    void                    EnableRelativeMode();
    void                    EnableRegisterMode();
    void                    EnableContextualMode();
    void                    EnableAutoFirstLine();
    void                    EnableAbsLineDist(tools::Long nMinTwip);
    void                    EnableNegativeMode();
    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxParaAlignTabPage ------------------------------------------------

class SvxParaAlignTabPage : public SfxTabPage
{
    static const WhichRangesContainer pAlignRanges, pSdrAlignRanges;

    bool m_bSdrVertAlign;

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
    std::unique_ptr<weld::Label> m_xVertAlign;
    std::unique_ptr<weld::Label> m_xVertAlignSdr;

    std::unique_ptr<svx::FrameDirectionListBox>  m_xTextDirectionLB;

    DECL_LINK(AlignHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(LastLineHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(TextDirectionHdl_Impl, weld::ComboBox&, void);

    void                    UpdateExample_Impl();

protected:
    virtual void            ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet ) override;

public:
    SvxParaAlignTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );
    virtual ~SvxParaAlignTabPage() override;

    static WhichRangesContainer GetRanges() { return pAlignRanges; }
    static WhichRangesContainer GetSdrRanges() { return pSdrAlignRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            ChangesApplied() override;

    void                    EnableJustifyExt();
    void                    EnableSdrVertAlign();
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
    static const WhichRangesContainer pExtRanges;

public:
    SvxExtParagraphTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                const SfxItemSet* rSet );
    virtual ~SvxExtParagraphTabPage() override;

    static WhichRangesContainer GetRanges() { return pExtRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                DisablePageBreak();

protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

private:
    weld::TriStateEnabled aHyphenState;
    weld::TriStateEnabled aPageBreakState;
    weld::TriStateEnabled aApplyCollState;
    weld::TriStateEnabled aPageNumState;
    weld::TriStateEnabled aAllowSplitState;
    weld::TriStateEnabled aKeepParaState;
    weld::TriStateEnabled aOrphanState;
    weld::TriStateEnabled aWidowState;
    weld::TriStateEnabled aKeepState;

    bool                bPageBreak;
    bool                bHtmlMode;
    sal_uInt16          nStdPos;

    // hyphenation
    std::unique_ptr<weld::CheckButton> m_xHyphenBox;
    std::unique_ptr<weld::CheckButton> m_xHyphenNoCapsBox;
    std::unique_ptr<weld::CheckButton> m_xHyphenNoLastWordBox;
    std::unique_ptr<weld::Label> m_xBeforeText;
    std::unique_ptr<weld::SpinButton> m_xExtHyphenBeforeBox;
    std::unique_ptr<weld::Label> m_xAfterText;
    std::unique_ptr<weld::SpinButton> m_xExtHyphenAfterBox;
    std::unique_ptr<weld::Label> m_xCompoundBeforeText;
    std::unique_ptr<weld::SpinButton> m_xExtCompoundHyphenBeforeBox;
    std::unique_ptr<weld::Label> m_xMaxHyphenLabel;
    std::unique_ptr<weld::SpinButton> m_xMaxHyphenEdit;
    std::unique_ptr<weld::Label> m_xMinWordLabel;
    std::unique_ptr<weld::SpinButton> m_xMinWordLength;
    std::unique_ptr<weld::Label> m_xHyphenZoneLabel;
    SvxRelativeField m_aHyphenZone;

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
    std::unique_ptr<weld::CheckButton> m_xAllowSplitBox;
    std::unique_ptr<weld::CheckButton> m_xKeepParaBox;

    // orphan/widow
    std::unique_ptr<weld::CheckButton> m_xOrphanBox;
    std::unique_ptr<weld::SpinButton> m_xOrphanRowNo;
    std::unique_ptr<weld::Label> m_xOrphanRowLabel;

    std::unique_ptr<weld::CheckButton> m_xWidowBox;
    std::unique_ptr<weld::SpinButton> m_xWidowRowNo;
    std::unique_ptr<weld::Label> m_xWidowRowLabel;

    // avoid hyphenation across
    std::unique_ptr<weld::CheckButton> m_xKeepBox;

    void HyphenClickHdl();
    void PageNumBoxClickHdl();
    void ApplyCollClickHdl();
    void PageBreakHdl();
    void AllowSplitHdl();
    void OrphanHdl();
    void WidowHdl();

    DECL_LINK(PageBreakHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(AllowSplitHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(WidowHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(OrphanHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(HyphenClickHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(ApplyCollClickHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(PageBreakPosHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(PageBreakTypeHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(PageNumBoxClickHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(KeepParaBoxClickHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(KeepHdl_Impl, weld::Toggleable&, void);

    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

class SvxAsianTabPage : public SfxTabPage
{
    std::unique_ptr<weld::CheckButton> m_xForbiddenRulesCB;
    std::unique_ptr<weld::CheckButton> m_xHangingPunctCB;
    std::unique_ptr<weld::CheckButton> m_xScriptSpaceCB;

public:
    SvxAsianTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);
    virtual ~SvxAsianTabPage() override;

    static WhichRangesContainer GetRanges();

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
