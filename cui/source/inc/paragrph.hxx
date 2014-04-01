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
    using TabPage::DeactivatePage;

private:
    SvxStdParagraphTabPage( Window* pParent, const SfxItemSet& rSet );

    // indention
    SvxRelativeField*       m_pLeftIndent;

    FixedText*              m_pRightLabel;
    SvxRelativeField*       m_pRightIndent;

    FixedText*              m_pFLineLabel;
    SvxRelativeField*       m_pFLineIndent;
    CheckBox*               m_pAutoCB;

    // distance
    SvxRelativeField*       m_pTopDist;
    SvxRelativeField*       m_pBottomDist;
    CheckBox*               m_pContextualCB;

    // line spacing
    ListBox*                m_pLineDist;
    FixedText*              m_pLineDistAtLabel;
    MetricField*            m_pLineDistAtPercentBox;
    MetricField*            m_pLineDistAtMetricBox;
    FixedText*              m_pAbsDist;
    OUString                sAbsDist;
    SvxParaPrevWindow*      m_pExampleWin;

    // only writer
    VclFrame*               m_pRegisterFL;
    CheckBox*               m_pRegisterCB;

    long                    nAbst;
    long                    nWidth;
    long                    nMinFixDist;
    sal_Bool                bRelativeMode;
    sal_Bool                bNegativeIndents;

    void                    SetLineSpacing_Impl( const SvxLineSpacingItem& rAttr );
    void                    Init_Impl();
    void                    UpdateExample_Impl( sal_Bool bAll = sal_False );

    DECL_LINK( LineDistHdl_Impl, ListBox* );
    DECL_LINK(ModifyHdl_Impl, void *);
    DECL_LINK( AutoHdl_Impl, CheckBox* );

protected:
    virtual int             DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;


public:
    DECL_LINK(ELRLoseFocusHdl, void *);

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual bool            FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void            Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;


    void                    SetPageWidth( sal_uInt16 nPageWidth );
    void                    EnableRelativeMode();
    void                    EnableRegisterMode();
    void                    EnableContextualMode();
    void                    EnableAutoFirstLine();
    void                    EnableAbsLineDist(long nMinTwip);
    void                    EnableNegativeMode();
    virtual void            PageCreated(SfxAllItemSet aSet) SAL_OVERRIDE;

    virtual                ~SvxStdParagraphTabPage();
};

// class SvxParaAlignTabPage ------------------------------------------------

class SvxParaAlignTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

    // alignment
    RadioButton*             m_pLeft;
    RadioButton*             m_pRight;
    RadioButton*             m_pCenter;
    RadioButton*             m_pJustify;
    FixedText*               m_pLeftBottom;
    FixedText*               m_pRightTop;

    FixedText*               m_pLastLineFT;
    ListBox*                 m_pLastLineLB;
    CheckBox*                m_pExpandCB;

    CheckBox*                m_pSnapToGridCB;

    //preview
    SvxParaPrevWindow*       m_pExampleWin;
    //vertical alignment
    VclFrame*                m_pVertAlignFL;
    ListBox*                 m_pVertAlignLB;

    VclFrame*                m_pPropertiesFL;
    svx::FrameDirectionListBox*  m_pTextDirectionLB;

    DECL_LINK(AlignHdl_Impl, void *);
    DECL_LINK(LastLineHdl_Impl, void *);
    DECL_LINK(TextDirectionHdl_Impl, void *);

    void                    UpdateExample_Impl( sal_Bool bAll = sal_False );

                            SvxParaAlignTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual                 ~SvxParaAlignTabPage();

protected:
    virtual int             DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;

public:
    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual bool            FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void            Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    void                    EnableJustifyExt();
    virtual void            PageCreated(SfxAllItemSet aSet) SAL_OVERRIDE;
};

// class SvxExtParagraphTabPage ------------------------------------------
/*
    [Description]
    With this TabPage special attributes of a paragraph can be set
    (hyphenation, pagebreak, orphan, widow, ...).

    [Items]
    <SvxHyphenZoneItem><SID_ATTR_PARA_HYPHENZONE>
    <SvxFmtBreakItem><SID_ATTR_PARA_PAGEBREAK>
    <SvxFmtSplitItem><SID_ATTR_PARA_SPLIT>
    <SvxWidowsItem><SID_ATTR_PARA_WIDOWS>
    <SvxOrphansItem><SID_ATTR_PARA_ORPHANS>
*/

class SvxExtParagraphTabPage: public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    virtual             ~SvxExtParagraphTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    void                DisablePageBreak();

protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;

private:
                        SvxExtParagraphTabPage( Window* pParent, const SfxItemSet& rSet );

    // hyphenation
    TriStateBox*        m_pHyphenBox;
    FixedText*          m_pBeforeText;
    NumericField*       m_pExtHyphenBeforeBox;
    FixedText*          m_pAfterText;
    NumericField*       m_pExtHyphenAfterBox;
    FixedText*          m_pMaxHyphenLabel;
    NumericField*       m_pMaxHyphenEdit;

    // pagebreak
    TriStateBox*        m_pPageBreakBox;
    FixedText*          m_pBreakTypeFT;
    ListBox*            m_pBreakTypeLB;
    FixedText*          m_pBreakPositionFT;
    ListBox*            m_pBreakPositionLB;
    TriStateBox*        m_pApplyCollBtn;
    ListBox*            m_pApplyCollBox;
    FixedText*          m_pPagenumText;
    NumericField*       m_pPagenumEdit;

    // paragraph division
    TriStateBox*        m_pKeepTogetherBox;
    TriStateBox*        m_pKeepParaBox;

    // orphan/widow
    TriStateBox*        m_pOrphanBox;
    NumericField*       m_pOrphanRowNo;
    FixedText*          m_pOrphanRowLabel;

    TriStateBox*        m_pWidowBox;
    NumericField*       m_pWidowRowNo;
    FixedText*          m_pWidowRowLabel;

    sal_Bool                bPageBreak;
    sal_Bool                bHtmlMode;
    sal_uInt16              nStdPos;

    DECL_LINK(PageBreakHdl_Impl, void *);
    DECL_LINK(KeepTogetherHdl_Impl, void *);
    DECL_LINK(WidowHdl_Impl, void *);
    DECL_LINK(OrphanHdl_Impl, void *);
    DECL_LINK(HyphenClickHdl_Impl, void *);
    DECL_LINK(ApplyCollClickHdl_Impl, void *);
    DECL_LINK( PageBreakPosHdl_Impl, ListBox* );
    DECL_LINK( PageBreakTypeHdl_Impl, ListBox* );

    virtual void            PageCreated(SfxAllItemSet aSet) SAL_OVERRIDE;
};


class SvxAsianTabPage : public SfxTabPage
{

    CheckBox*     m_pForbiddenRulesCB;
    CheckBox*     m_pHangingPunctCB;
    CheckBox*     m_pScriptSpaceCB;

    SvxAsianTabPage( Window* pParent, const SfxItemSet& rSet );

    DECL_LINK( ClickHdl_Impl, CheckBox* );

public:
    virtual ~SvxAsianTabPage();


    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

};

#endif // INCLUDED_CUI_SOURCE_INC_PARAGRPH_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
