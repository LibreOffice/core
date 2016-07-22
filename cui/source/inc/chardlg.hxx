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
#ifndef INCLUDED_CUI_SOURCE_INC_CHARDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_CHARDLG_HXX

#include <svtools/ctrlbox.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/fntctrl.hxx>
#include <svx/checklbx.hxx>
#include <svx/langbox.hxx>
#include <vcl/layout.hxx>
#include <memory>

// forward ---------------------------------------------------------------

class SvxFontListItem;
class FontList;

// class SvxCharBasePage -------------------------------------------------

class SvxCharBasePage : public SfxTabPage
{
protected:
    VclPtr<SvxFontPrevWindow>  m_pPreviewWin;

    bool                m_bPreviewBackgroundToCharacter;

    SvxCharBasePage(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet& rItemset);

    void SetPrevFontWidthScale( const SfxItemSet& rSet );
    void SetPrevFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );

    inline SvxFont&     GetPreviewFont();
    inline SvxFont&     GetPreviewCJKFont();
    inline SvxFont&     GetPreviewCTLFont();

public:
    virtual ~SvxCharBasePage();
    virtual void dispose() override;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;

};

// class SvxCharNamePage -------------------------------------------------

struct SvxCharNamePage_Impl;

class SvxCharNamePage : public SvxCharBasePage
{
    friend class VclPtr<SvxCharNamePage>;

private:
    static const sal_uInt16 pNameRanges[];
    VclPtr<VclContainer>       m_pWestFrame;
    VclPtr<FixedText>          m_pWestFontNameFT;
    VclPtr<FontNameBox>        m_pWestFontNameLB;
    VclPtr<FixedText>          m_pWestFontStyleFT;
    VclPtr<FontStyleBox>       m_pWestFontStyleLB;
    VclPtr<FixedText>          m_pWestFontSizeFT;
    VclPtr<FontSizeBox>        m_pWestFontSizeLB;
    VclPtr<FixedText>          m_pWestFontLanguageFT;
    VclPtr<SvxLanguageComboBox> m_pWestFontLanguageLB;
    VclPtr<FixedText>          m_pWestFontTypeFT;

    VclPtr<VclContainer>       m_pEastFrame;
    VclPtr<FixedText>          m_pEastFontNameFT;
    VclPtr<FontNameBox>        m_pEastFontNameLB;
    VclPtr<FixedText>          m_pEastFontStyleFT;
    VclPtr<FontStyleBox>       m_pEastFontStyleLB;
    VclPtr<FixedText>          m_pEastFontSizeFT;
    VclPtr<FontSizeBox>        m_pEastFontSizeLB;
    VclPtr<FixedText>          m_pEastFontLanguageFT;
    VclPtr<SvxLanguageBox>     m_pEastFontLanguageLB;
    VclPtr<FixedText>          m_pEastFontTypeFT;

    VclPtr<VclContainer>       m_pCTLFrame;
    VclPtr<FixedText>          m_pCTLFontNameFT;
    VclPtr<FontNameBox>        m_pCTLFontNameLB;
    VclPtr<FixedText>          m_pCTLFontStyleFT;
    VclPtr<FontStyleBox>       m_pCTLFontStyleLB;
    VclPtr<FixedText>          m_pCTLFontSizeFT;
    VclPtr<FontSizeBox>        m_pCTLFontSizeLB;
    VclPtr<FixedText>          m_pCTLFontLanguageFT;
    VclPtr<SvxLanguageBox>     m_pCTLFontLanguageLB;
    VclPtr<FixedText>          m_pCTLFontTypeFT;

    std::unique_ptr<SvxCharNamePage_Impl>   m_pImpl;

                        SvxCharNamePage( vcl::Window* pParent, const SfxItemSet& rSet );

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
    bool                FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp );

    DECL_LINK_TYPED( UpdateHdl_Impl, Idle *, void );
    DECL_LINK_TYPED( FontModifyEditHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( FontModifyListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( FontModifyComboBoxHdl_Impl, ComboBox&, void );
    void FontModifyHdl_Impl(void*);

public:
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
                        virtual ~SvxCharNamePage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pNameRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                SetFontList( const SvxFontListItem& rItem );
    void                EnableRelativeMode();
    void                EnableSearchMode();
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();

    void                DisableControls( sal_uInt16 nDisable );
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxCharEffectsPage ----------------------------------------------

class SvxCharEffectsPage : public SvxCharBasePage
{
    friend class VclPtr<SvxCharEffectsPage>;

private:
    static const sal_uInt16 pEffectsRanges[];
    VclPtr<FixedText>          m_pFontColorFT;
    VclPtr<ColorListBox>       m_pFontColorLB;

    VclPtr<FixedText>          m_pEffectsFT;
    VclPtr<ListBox>            m_pEffectsLB;

    VclPtr<FixedText>          m_pReliefFT;
    VclPtr<ListBox>            m_pReliefLB;

    VclPtr<TriStateBox>        m_pOutlineBtn;
    VclPtr<TriStateBox>        m_pShadowBtn;
    VclPtr<TriStateBox>        m_pBlinkingBtn;
    VclPtr<TriStateBox>        m_pHiddenBtn;

    VclPtr<ListBox>            m_pOverlineLB;
    VclPtr<FixedText>          m_pOverlineColorFT;
    VclPtr<ColorListBox>       m_pOverlineColorLB;

    VclPtr<ListBox>            m_pStrikeoutLB;

    VclPtr<ListBox>            m_pUnderlineLB;
    VclPtr<FixedText>          m_pUnderlineColorFT;
    VclPtr<ColorListBox>       m_pUnderlineColorLB;

    VclPtr<CheckBox>           m_pIndividualWordsBtn;

    VclPtr<FixedText>          m_pEmphasisFT;
    VclPtr<ListBox>            m_pEmphasisLB;

    VclPtr<FixedText>          m_pPositionFT;
    VclPtr<ListBox>            m_pPositionLB;

    VclPtr<FixedText>          m_pA11yWarningFT;

    sal_uInt16          m_nHtmlMode;

    OUString            m_aTransparentColorName;

                        SvxCharEffectsPage( vcl::Window* pParent, const SfxItemSet& rSet );

    void                Initialize();
    void                UpdatePreview_Impl();
    void                SetCaseMap_Impl( SvxCaseMap eCaseMap );
    void                ResetColor_Impl( const SfxItemSet& rSet );
    bool                FillItemSetColor_Impl( SfxItemSet& rSet );

    void SelectHdl_Impl(ListBox*);
    DECL_LINK_TYPED(SelectListBoxHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(CbClickHdl_Impl, Button*, void);
    DECL_LINK_TYPED(TristClickHdl_Impl, Button*, void);
    DECL_LINK_TYPED(UpdatePreview_Impl, ListBox&, void);
    DECL_LINK_TYPED(ColorBoxSelectHdl_Impl, ListBox&, void);

public:
    virtual ~SvxCharEffectsPage();
    virtual void dispose() override;

    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pEffectsRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                DisableControls( sal_uInt16 nDisable );
    void                EnableFlash();
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxCharPositionPage ---------------------------------------------


class SvxCharPositionPage : public SvxCharBasePage
{
    friend class VclPtr<SvxCharPositionPage>;
    static const sal_uInt16 pPositionRanges[];

private:
    VclPtr<RadioButton>        m_pHighPosBtn;
    VclPtr<RadioButton>        m_pNormalPosBtn;
    VclPtr<RadioButton>        m_pLowPosBtn;
    VclPtr<FixedText>          m_pHighLowFT;
    VclPtr<MetricField>        m_pHighLowMF;
    VclPtr<CheckBox>           m_pHighLowRB;
    VclPtr<FixedText>          m_pFontSizeFT;
    VclPtr<MetricField>        m_pFontSizeMF;

    VclPtr<VclContainer>       m_pRotationContainer;

    VclPtr<FixedText>          m_pScalingFT;
    VclPtr<FixedText>          m_pScalingAndRotationFT;
    VclPtr<RadioButton>        m_p0degRB;
    VclPtr<RadioButton>        m_p90degRB;
    VclPtr<RadioButton>        m_p270degRB;
    VclPtr<CheckBox>           m_pFitToLineCB;

    VclPtr<MetricField>        m_pScaleWidthMF;

    VclPtr<MetricField>        m_pKerningMF;
    VclPtr<CheckBox>           m_pPairKerningBtn;

    short               m_nSuperEsc;
    short               m_nSubEsc;

    sal_uInt16              m_nScaleWidthItemSetVal;
    sal_uInt16              m_nScaleWidthInitialVal;

    sal_uInt8                m_nSuperProp;
    sal_uInt8                m_nSubProp;

                        SvxCharPositionPage( vcl::Window* pParent, const SfxItemSet& rSet );

    void                Initialize();
    void                UpdatePreview_Impl( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );
    void                SetEscapement_Impl( sal_uInt16 nEsc );

    DECL_LINK_TYPED(    PositionHdl_Impl, Button*, void );
    DECL_LINK_TYPED(    RotationHdl_Impl, Button*, void );
    DECL_LINK_TYPED(    FontModifyHdl_Impl, Edit&, void );
    DECL_LINK_TYPED(    AutoPositionHdl_Impl, Button*, void );
    DECL_LINK_TYPED(    FitToLineHdl_Impl, Button*, void );
    DECL_LINK_TYPED(    KerningSelectHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED(    KerningModifyHdl_Impl, Edit&, void );
    DECL_LINK_TYPED(    LoseFocusHdl_Impl, Control&, void );
    DECL_LINK_TYPED(    ScaleWidthModifyHdl_Impl, Edit&, void );

public:
    virtual ~SvxCharPositionPage();
    virtual void dispose() override;

    using SfxTabPage::DeactivatePage;

    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );
    static const sal_uInt16*      GetRanges() { return pPositionRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;
    virtual void        FillUserData() override;
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxCharTwoLinesPage ---------------------------------------------

class SvxCharTwoLinesPage : public SvxCharBasePage
{
    friend class VclPtr<SvxCharTwoLinesPage>;
private:
    static const sal_uInt16 pTwoLinesRanges[];
    VclPtr<CheckBox>           m_pTwoLinesBtn;
    VclPtr<VclContainer>       m_pEnclosingFrame;
    VclPtr<ListBox>            m_pStartBracketLB;
    VclPtr<ListBox>            m_pEndBracketLB;

    sal_uInt16              m_nStartBracketPosition;
    sal_uInt16              m_nEndBracketPosition;

    SvxCharTwoLinesPage(vcl::Window* pParent, const SfxItemSet& rSet);

    void                UpdatePreview_Impl();
    void                Initialize();
    void                SelectCharacter( ListBox* pBox );
    void                SetBracket( sal_Unicode cBracket, bool bStart );

    DECL_LINK_TYPED(TwoLinesHdl_Impl, Button*, void);
    DECL_LINK_TYPED(CharacterMapHdl_Impl, ListBox&, void );

public:
    virtual ~SvxCharTwoLinesPage();
    virtual void dispose() override;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );
    static const sal_uInt16*  GetRanges() { return pTwoLinesRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    ///                  the writer uses SID_ATTR_BRUSH as font background
    void                SetPreviewBackgroundToCharacter();
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CHARDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
