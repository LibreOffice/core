/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_CHARDLG_HXX
#define _SVX_CHARDLG_HXX

#include <svtools/ctrlbox.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/fntctrl.hxx>
#include <svx/checklbx.hxx>
#include <svx/langbox.hxx>
#include <vcl/layout.hxx>

// forward ---------------------------------------------------------------

class SvxFontListItem;
class FontList;

// -----------------------------------------------------------------------

#define DISABLE_CASEMAP             ((sal_uInt16)0x0001)
#define DISABLE_WORDLINE            ((sal_uInt16)0x0002)
#define DISABLE_BLINK               ((sal_uInt16)0x0004)
#define DISABLE_UNDERLINE_COLOR     ((sal_uInt16)0x0008)

#define DISABLE_LANGUAGE            ((sal_uInt16)0x0010)
#define DISABLE_HIDE_LANGUAGE       ((sal_uInt16)0x0020)

// class SvxCharBasePage -------------------------------------------------

class SvxCharBasePage : public SfxTabPage
{
protected:
    SvxFontPrevWindow*  m_pPreviewWin;
    FixedInfo*          m_pFontTypeFT;

    sal_Bool                m_bPreviewBackgroundToCharacter;

    SvxCharBasePage(Window* pParent, const ResId& rResIdTabPage, const SfxItemSet&);
    virtual             ~SvxCharBasePage();

    void makeWidgets(Window *pParent, const ResId& rResId,
        sal_uInt16 nResIdPrewievWin, sal_uInt16 nResIdFontTypeFT);

    void SetPrevFontWidthScale( const SfxItemSet& rSet );
    void SetPrevFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );

    inline SvxFont&     GetPreviewFont();
    inline SvxFont&     GetPreviewCJKFont();
    inline SvxFont&     GetPreviewCTLFont();

public:
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet );

};

// class SvxCharNamePage -------------------------------------------------

struct SvxCharNamePage_Impl;

class SvxCharNamePage : public SvxCharBasePage
{

private:
    VBox m_aBox;
    Grid *m_pGrid;

    FixedLine*          m_pWestLine;
    FixedText*          m_pWestFontNameFT;
    FontNameBox*        m_pWestFontNameLB;
    FixedText*          m_pWestFontStyleFT;
    FontStyleBox*       m_pWestFontStyleLB;
    FixedText*          m_pWestFontSizeFT;
    FontSizeBox*        m_pWestFontSizeLB;
    FixedText*          m_pWestFontLanguageFT;
    SvxLanguageBox*     m_pWestFontLanguageLB;

    FixedLine*          m_pEastLine;
    FixedText*          m_pEastFontNameFT;
    FontNameBox*        m_pEastFontNameLB;
    FixedText*          m_pEastFontStyleFT;
    FontStyleBox*       m_pEastFontStyleLB;
    FixedText*          m_pEastFontSizeFT;
    FontSizeBox*        m_pEastFontSizeLB;
    FixedText*          m_pEastFontLanguageFT;
    SvxLanguageBox*     m_pEastFontLanguageLB;

    FixedLine*          m_pCTLLine;
    FixedText*          m_pCTLFontNameFT;
    FontNameBox*        m_pCTLFontNameLB;
    FixedText*          m_pCTLFontStyleFT;
    FontStyleBox*       m_pCTLFontStyleLB;
    FixedText*          m_pCTLFontSizeFT;
    FontSizeBox*        m_pCTLFontSizeLB;
    FixedText*          m_pCTLFontLanguageFT;
    SvxLanguageBox*     m_pCTLFontLanguageLB;

    FixedLine*          m_pColorFL;
    FixedText*          m_pColorFT;
    ColorListBox*       m_pColorLB;

    SvxCharNamePage_Impl*   m_pImpl;

                        SvxCharNamePage( Window* pParent, const SfxItemSet& rSet );

    void                Initialize();
    const FontList*     GetFontList() const;
    void                UpdatePreview_Impl();
    void                FillStyleBox_Impl( const FontNameBox* rBox );
    void                FillSizeBox_Impl( const FontNameBox* rBox );

    enum LanguageGroup
    {
        /** Language for western text.
         */
        Western = 0,

        /** Language for asian text.
         */
        Asian,

        /** Language for ctl text.
         */
        Ctl
    };

    void                Reset_Impl( const SfxItemSet& rSet, LanguageGroup eLangGrp );
    sal_Bool                FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp );
    void                ResetColor_Impl( const SfxItemSet& rSet );

    DECL_LINK(UpdateHdl_Impl, void *);
    DECL_LINK(          FontModifyHdl_Impl, void* );
    DECL_LINK(          ColorBoxSelectHdl_Impl, ColorListBox* );

public:
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
                        ~SvxCharNamePage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual void        Reset( const SfxItemSet& rSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );

    void                SetFontList( const SvxFontListItem& rItem );
    void                EnableRelativeMode();
    void                EnableSearchMode();
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();

    void                DisableControls( sal_uInt16 nDisable );
    virtual void        PageCreated (SfxAllItemSet aSet);
};

// class SvxCharEffectsPage ----------------------------------------------

class SvxCharEffectsPage : public SvxCharBasePage
{

private:
    VBox m_aBox;
    Grid m_aGrid;

    FixedText           m_aFontColorFT;
    ColorListBox        m_aFontColorLB;

    FixedText           m_aEffectsFT;
    ListBox             m_aEffectsLB;

    FixedText           m_aReliefFT;
    ListBox             m_aReliefLB;

    TriStateBox         m_aOutlineBtn;
    TriStateBox         m_aShadowBtn;
    TriStateBox         m_aBlinkingBtn;
    TriStateBox         m_aHiddenBtn;

    FixedLine           m_aVerticalLine;

    FixedText           m_aOverlineFT;
    ListBox             m_aOverlineLB;
    FixedText           m_aOverlineColorFT;
    ColorListBox        m_aOverlineColorLB;

    FixedText           m_aStrikeoutFT;
    ListBox             m_aStrikeoutLB;

    FixedText           m_aUnderlineFT;
    ListBox             m_aUnderlineLB;
    FixedText           m_aUnderlineColorFT;
    ColorListBox        m_aUnderlineColorLB;

    CheckBox            m_aIndividualWordsBtn;

    FixedLine           m_aAsianLine;

    FixedText           m_aEmphasisFT;
    ListBox             m_aEmphasisLB;

    FixedText           m_aPositionFT;
    ListBox             m_aPositionLB;

    sal_uInt16              m_nHtmlMode;

    String              m_aTransparentColorName;

                        SvxCharEffectsPage( Window* pParent, const SfxItemSet& rSet );

    void                Initialize();
    void                UpdatePreview_Impl();
    void                SetCaseMap_Impl( SvxCaseMap eCaseMap );
    void                ResetColor_Impl( const SfxItemSet& rSet );
    sal_Bool                FillItemSetColor_Impl( SfxItemSet& rSet );

    DECL_LINK(          SelectHdl_Impl, ListBox* );
    DECL_LINK(CbClickHdl_Impl, void *);
    DECL_LINK(TristClickHdl_Impl, void *);
    DECL_LINK(UpdatePreview_Impl, void *);
    DECL_LINK(          ColorBoxSelectHdl_Impl, ColorListBox* );

public:
    using SfxTabPage::DeactivatePage;

    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual void        Reset( const SfxItemSet& rSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );

    void                DisableControls( sal_uInt16 nDisable );
    void                EnableFlash();
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();
    virtual void        PageCreated (SfxAllItemSet aSet);
};

// class SvxCharPositionPage ---------------------------------------------

class SvxCharPositionPage : public SvxCharBasePage
{

private:
    FixedLine           m_aPositionLine;
    RadioButton         m_aHighPosBtn;
    RadioButton         m_aNormalPosBtn;
    RadioButton         m_aLowPosBtn;
    FixedText           m_aHighLowFT;
    MetricField         m_aHighLowEdit;
    CheckBox            m_aHighLowRB;
    FixedText           m_aFontSizeFT;
    MetricField         m_aFontSizeEdit;
    FixedLine           m_aRotationScalingFL;
    FixedLine           m_aScalingFL;
    RadioButton         m_a0degRB;
    RadioButton         m_a90degRB;
    RadioButton         m_a270degRB;
    CheckBox            m_aFitToLineCB;
    FixedText           m_aScaleWidthFT;
    MetricField         m_aScaleWidthMF;

    FixedLine           m_aKerningLine;
    ListBox             m_aKerningLB;
    FixedText           m_aKerningFT;
    MetricField         m_aKerningEdit;
    CheckBox            m_aPairKerningBtn;

    short               m_nSuperEsc;
    short               m_nSubEsc;

    sal_uInt16              m_nScaleWidthItemSetVal;
    sal_uInt16              m_nScaleWidthInitialVal;

    sal_uInt8                m_nSuperProp;
    sal_uInt8                m_nSubProp;

                        SvxCharPositionPage( Window* pParent, const SfxItemSet& rSet );

    void                Initialize();
    void                UpdatePreview_Impl( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );
    void                SetEscapement_Impl( sal_uInt16 nEsc );

    DECL_LINK(          PositionHdl_Impl, RadioButton* );
    DECL_LINK(          RotationHdl_Impl, RadioButton* );
    DECL_LINK(FontModifyHdl_Impl, void *);
    DECL_LINK(          AutoPositionHdl_Impl, CheckBox* );
    DECL_LINK(          FitToLineHdl_Impl, CheckBox* );
    DECL_LINK(KerningSelectHdl_Impl, void *);
    DECL_LINK(KerningModifyHdl_Impl, void *);
    DECL_LINK(PairKerningHdl_Impl, void *);
    DECL_LINK(          LoseFocusHdl_Impl, MetricField* );
    DECL_LINK(ScaleWidthModifyHdl_Impl, void *);

public:
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    virtual void        ActivatePage( const SfxItemSet& rSet );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual void        Reset( const SfxItemSet& rSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        FillUserData();
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();
    virtual void        PageCreated (SfxAllItemSet aSet);
};

// class SvxCharTwoLinesPage ---------------------------------------------

class SvxCharTwoLinesPage : public SvxCharBasePage
{
private:
    FixedLine           m_aSwitchOnLine;
    CheckBox            m_aTwoLinesBtn;

    FixedLine           m_aEncloseLine;
    FixedText           m_aStartBracketFT;
    ListBox             m_aStartBracketLB;
    FixedText           m_aEndBracketFT;
    ListBox             m_aEndBracketLB;

    sal_uInt16              m_nStartBracketPosition;
    sal_uInt16              m_nEndBracketPosition;

                        SvxCharTwoLinesPage( Window* pParent, const SfxItemSet& rSet );

    void                UpdatePreview_Impl();
    void                Initialize();
    void                SelectCharacter( ListBox* pBox );
    void                SetBracket( sal_Unicode cBracket, sal_Bool bStart );

    DECL_LINK(TwoLinesHdl_Impl, void *);
    DECL_LINK(          CharacterMapHdl_Impl, ListBox* );

public:
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual void        Reset( const SfxItemSet& rSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();
    virtual void        PageCreated (SfxAllItemSet aSet);
};

#endif // #ifndef _SVX_CHARDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
