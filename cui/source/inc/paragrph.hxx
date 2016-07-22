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
    using TabPage::DeactivatePage;
    static const sal_uInt16 pStdRanges[];

private:
    SvxStdParagraphTabPage( vcl::Window* pParent, const SfxItemSet& rSet );

    // indention
    VclPtr<SvxRelativeField>       m_pLeftIndent;

    VclPtr<FixedText>              m_pRightLabel;
    VclPtr<SvxRelativeField>       m_pRightIndent;

    VclPtr<FixedText>              m_pFLineLabel;
    VclPtr<SvxRelativeField>       m_pFLineIndent;
    VclPtr<CheckBox>               m_pAutoCB;

    // distance
    VclPtr<SvxRelativeField>       m_pTopDist;
    VclPtr<SvxRelativeField>       m_pBottomDist;
    VclPtr<CheckBox>               m_pContextualCB;

    // line spacing
    VclPtr<ListBox>                m_pLineDist;
    VclPtr<FixedText>              m_pLineDistAtLabel;
    VclPtr<MetricField>            m_pLineDistAtPercentBox;
    VclPtr<MetricField>            m_pLineDistAtMetricBox;
    VclPtr<FixedText>              m_pAbsDist;
    OUString                sAbsDist;
    VclPtr<SvxParaPrevWindow>      m_pExampleWin;

    // only writer
    VclPtr<VclFrame>               m_pRegisterFL;
    VclPtr<CheckBox>               m_pRegisterCB;

    long                    nAbst;
    long                    nWidth;
    long                    nMinFixDist;
    bool                    bRelativeMode;
    bool                    bNegativeIndents;

    void                    SetLineSpacing_Impl( const SvxLineSpacingItem& rAttr );
    void                    Init_Impl();
    void                    UpdateExample_Impl();

    DECL_LINK_TYPED( LineDistHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ModifyHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( AutoHdl_Impl, Button*, void );

protected:
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet ) override;


public:
    virtual ~SvxStdParagraphTabPage();
    virtual void dispose() override;

    DECL_LINK_TYPED(ELRLoseFocusHdl, Control&, void);

    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pStdRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            ChangesApplied() override;


    void                    SetPageWidth( sal_uInt16 nPageWidth );
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
    using TabPage::DeactivatePage;
    static const sal_uInt16 pAlignRanges[];

    // alignment
    VclPtr<RadioButton>             m_pLeft;
    VclPtr<RadioButton>             m_pRight;
    VclPtr<RadioButton>             m_pCenter;
    VclPtr<RadioButton>             m_pJustify;
    VclPtr<FixedText>               m_pLeftBottom;
    VclPtr<FixedText>               m_pRightTop;

    VclPtr<FixedText>               m_pLastLineFT;
    VclPtr<ListBox>                 m_pLastLineLB;
    VclPtr<CheckBox>                m_pExpandCB;

    VclPtr<CheckBox>                m_pSnapToGridCB;

    //preview
    VclPtr<SvxParaPrevWindow>       m_pExampleWin;
    //vertical alignment
    VclPtr<VclFrame>                m_pVertAlignFL;
    VclPtr<ListBox>                 m_pVertAlignLB;

    VclPtr<VclFrame>                m_pPropertiesFL;
    VclPtr<svx::FrameDirectionListBox>  m_pTextDirectionLB;

    DECL_LINK_TYPED(AlignHdl_Impl, Button*, void);
    DECL_LINK_TYPED(LastLineHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(TextDirectionHdl_Impl, ListBox&, void);

    void                    UpdateExample_Impl();

                            SvxParaAlignTabPage( vcl::Window* pParent, const SfxItemSet& rSet );

protected:
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet ) override;

public:
    virtual ~SvxParaAlignTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent, const SfxItemSet* rSet );
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
    virtual ~SvxExtParagraphTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pExtRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                DisablePageBreak();

protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

private:
                        SvxExtParagraphTabPage( vcl::Window* pParent, const SfxItemSet& rSet );

    // hyphenation
    VclPtr<TriStateBox>        m_pHyphenBox;
    VclPtr<FixedText>          m_pBeforeText;
    VclPtr<NumericField>       m_pExtHyphenBeforeBox;
    VclPtr<FixedText>          m_pAfterText;
    VclPtr<NumericField>       m_pExtHyphenAfterBox;
    VclPtr<FixedText>          m_pMaxHyphenLabel;
    VclPtr<NumericField>       m_pMaxHyphenEdit;

    // pagebreak
    VclPtr<TriStateBox>        m_pPageBreakBox;
    VclPtr<FixedText>          m_pBreakTypeFT;
    VclPtr<ListBox>            m_pBreakTypeLB;
    VclPtr<FixedText>          m_pBreakPositionFT;
    VclPtr<ListBox>            m_pBreakPositionLB;
    VclPtr<TriStateBox>        m_pApplyCollBtn;
    VclPtr<ListBox>            m_pApplyCollBox;
    VclPtr<FixedText>          m_pPagenumText;
    VclPtr<NumericField>       m_pPagenumEdit;

    // paragraph division
    VclPtr<TriStateBox>        m_pKeepTogetherBox;
    VclPtr<TriStateBox>        m_pKeepParaBox;

    // orphan/widow
    VclPtr<TriStateBox>        m_pOrphanBox;
    VclPtr<NumericField>       m_pOrphanRowNo;
    VclPtr<FixedText>          m_pOrphanRowLabel;

    VclPtr<TriStateBox>        m_pWidowBox;
    VclPtr<NumericField>       m_pWidowRowNo;
    VclPtr<FixedText>          m_pWidowRowLabel;

    bool                bPageBreak;
    bool                bHtmlMode;
    sal_uInt16              nStdPos;

    DECL_LINK_TYPED(PageBreakHdl_Impl, Button*, void);
    DECL_LINK_TYPED(KeepTogetherHdl_Impl, Button*, void);
    DECL_LINK_TYPED(WidowHdl_Impl, Button*, void);
    DECL_LINK_TYPED(OrphanHdl_Impl, Button*, void);
    DECL_LINK_TYPED(HyphenClickHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ApplyCollClickHdl_Impl, Button*, void);
    DECL_LINK_TYPED( PageBreakPosHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( PageBreakTypeHdl_Impl, ListBox&, void );

    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};


class SvxAsianTabPage : public SfxTabPage
{
    friend class VclPtr<SvxAsianTabPage>;

    VclPtr<CheckBox>     m_pForbiddenRulesCB;
    VclPtr<CheckBox>     m_pHangingPunctCB;
    VclPtr<CheckBox>     m_pScriptSpaceCB;

    SvxAsianTabPage( vcl::Window* pParent, const SfxItemSet& rSet );

    DECL_STATIC_LINK_TYPED( SvxAsianTabPage, ClickHdl_Impl, Button*, void );

public:
    virtual ~SvxAsianTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rSet );
    static const sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

};

#endif // INCLUDED_CUI_SOURCE_INC_PARAGRPH_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
