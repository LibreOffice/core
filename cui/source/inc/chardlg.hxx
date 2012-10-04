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

    sal_Bool                m_bPreviewBackgroundToCharacter;

    SvxCharBasePage(Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription, const SfxItemSet& rItemset);

    virtual             ~SvxCharBasePage();

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
    VclContainer*       m_pWestFrame;
    FixedText*          m_pWestFontNameFT;
    FontNameBox*        m_pWestFontNameLB;
    FixedText*          m_pWestFontStyleFT;
    FontStyleBox*       m_pWestFontStyleLB;
    FixedText*          m_pWestFontSizeFT;
    FontSizeBox*        m_pWestFontSizeLB;
    FixedText*          m_pWestFontLanguageFT;
    SvxLanguageBox*     m_pWestFontLanguageLB;

    VclContainer*       m_pEastFrame;
    FixedText*          m_pEastFontNameFT;
    FontNameBox*        m_pEastFontNameLB;
    FixedText*          m_pEastFontStyleFT;
    FontStyleBox*       m_pEastFontStyleLB;
    FixedText*          m_pEastFontSizeFT;
    FontSizeBox*        m_pEastFontSizeLB;
    FixedText*          m_pEastFontLanguageFT;
    SvxLanguageBox*     m_pEastFontLanguageLB;

    VclContainer*       m_pCTLFrame;
    FixedText*          m_pCTLFontNameFT;
    FontNameBox*        m_pCTLFontNameLB;
    FixedText*          m_pCTLFontStyleFT;
    FontStyleBox*       m_pCTLFontStyleLB;
    FixedText*          m_pCTLFontSizeFT;
    FontSizeBox*        m_pCTLFontSizeLB;
    FixedText*          m_pCTLFontLanguageFT;
    SvxLanguageBox*     m_pCTLFontLanguageLB;

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
    sal_Bool            FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp );

    DECL_LINK(UpdateHdl_Impl, void *);
    DECL_LINK(          FontModifyHdl_Impl, void* );

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
    FixedText*          m_pFontColorFT;
    ColorListBox*       m_pFontColorLB;

    FixedText*          m_pEffectsFT;
    ListBox*            m_pEffectsLB;

    FixedText*          m_pReliefFT;
    ListBox*            m_pReliefLB;

    TriStateBox*        m_pOutlineBtn;
    TriStateBox*        m_pShadowBtn;
    TriStateBox*        m_pBlinkingBtn;
    TriStateBox*        m_pHiddenBtn;

    ListBox*            m_pOverlineLB;
    FixedText*          m_pOverlineColorFT;
    ColorListBox*       m_pOverlineColorLB;

    ListBox*            m_pStrikeoutLB;

    ListBox*            m_pUnderlineLB;
    FixedText*          m_pUnderlineColorFT;
    ColorListBox*       m_pUnderlineColorLB;

    CheckBox*           m_pIndividualWordsBtn;

    FixedLine*          m_pAsianLine;

    FixedText*          m_pEmphasisFT;
    ListBox*            m_pEmphasisLB;

    FixedText*          m_pPositionFT;
    ListBox*            m_pPositionLB;

    sal_uInt16              m_nHtmlMode;

    String              m_aTransparentColorName;

                        SvxCharEffectsPage( Window* pParent, const SfxItemSet& rSet );
                        ~SvxCharEffectsPage();

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
    RadioButton*        m_pHighPosBtn;
    RadioButton*        m_pNormalPosBtn;
    RadioButton*        m_pLowPosBtn;
    FixedText*          m_pHighLowFT;
    MetricField*        m_pHighLowMF;
    CheckBox*           m_pHighLowRB;
    FixedText*          m_pFontSizeFT;
    MetricField*        m_pFontSizeMF;

    VclContainer*       m_pRotationContainer;

    FixedText*          m_pScalingFT;
    FixedText*          m_pScalingAndRotationFT;
    RadioButton*        m_p0degRB;
    RadioButton*        m_p90degRB;
    RadioButton*        m_p270degRB;
    CheckBox*           m_pFitToLineCB;

    MetricField*        m_pScaleWidthMF;

    ListBox*            m_pKerningLB;
    FixedText*          m_pKerningFT;
    MetricField*        m_pKerningMF;
    CheckBox*           m_pPairKerningBtn;

    short               m_nSuperEsc;
    short               m_nSubEsc;

    sal_uInt16              m_nScaleWidthItemSetVal;
    sal_uInt16              m_nScaleWidthInitialVal;

    sal_uInt8                m_nSuperProp;
    sal_uInt8                m_nSubProp;

                        SvxCharPositionPage( Window* pParent, const SfxItemSet& rSet );
                        ~SvxCharPositionPage();

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
    CheckBox*           m_pTwoLinesBtn;
    VclContainer*       m_pEnclosingFrame;
    ListBox*            m_pStartBracketLB;
    ListBox*            m_pEndBracketLB;

    sal_uInt16              m_nStartBracketPosition;
    sal_uInt16              m_nEndBracketPosition;

    SvxCharTwoLinesPage(Window* pParent, const SfxItemSet& rSet);
    ~SvxCharTwoLinesPage();

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
