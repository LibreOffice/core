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
#include <sfx2/tabdlg.hxx>
#include <svx/fntctrl.hxx>
#include <svx/checklbx.hxx>
#include <svx/colorbox.hxx>
#include <svx/langbox.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>
#include <vcl/button.hxx>
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

    inline SvxFont&     GetPreviewFont();
    inline SvxFont&     GetPreviewCJKFont();
    inline SvxFont&     GetPreviewCTLFont();

public:
    virtual ~SvxCharBasePage() override;
    virtual void dispose() override;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;

};

class CharBasePage : public SfxTabPage
{
protected:
    FontPrevWindow   m_aPreviewWin;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin;

    bool                m_bPreviewBackgroundToCharacter;

    CharBasePage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet& rAttrSet);

    void SetPrevFontWidthScale( const SfxItemSet& rSet );
    void SetPrevFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );

    inline SvxFont&     GetPreviewFont();
    inline SvxFont&     GetPreviewCJKFont();
    inline SvxFont&     GetPreviewCTLFont();

public:
    virtual ~CharBasePage() override;

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
    VclPtr<PushButton>         m_pWestFontFeaturesButton;

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
    VclPtr<PushButton>         m_pEastFontFeaturesButton;

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
    VclPtr<PushButton>         m_pCTLFontFeaturesButton;

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

    DECL_LINK( UpdateHdl_Impl, Timer *, void );
    DECL_LINK( FontModifyEditHdl_Impl, Edit&, void );
    DECL_LINK( FontModifyListBoxHdl_Impl, ListBox&, void );
    DECL_LINK( FontModifyComboBoxHdl_Impl, ComboBox&, void );
    DECL_LINK(FontFeatureButtonClicked, Button*, void);

    void FontModifyHdl_Impl(void const *);

public:
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
                        virtual ~SvxCharNamePage() override;
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pNameRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                SetFontList( const SvxFontListItem& rItem );
    void                EnableRelativeMode();
    void                EnableSearchMode();

    void                DisableControls( sal_uInt16 nDisable );
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxCharEffectsPage ----------------------------------------------

class SvxCharEffectsPage : public SvxCharBasePage
{
    friend class VclPtr<SvxCharEffectsPage>;

private:
    static const sal_uInt16 pEffectsRanges[];
    bool                       m_bOrigFontColor;
    bool                       m_bNewFontColor;
    bool                       m_bEnableNoneFontColor;
    Color                      m_aOrigFontColor;
    VclPtr<FixedText>          m_pFontColorFT;
    VclPtr<SvxColorListBox>    m_pFontColorLB;

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
    VclPtr<SvxColorListBox>    m_pOverlineColorLB;

    VclPtr<ListBox>            m_pStrikeoutLB;

    VclPtr<ListBox>            m_pUnderlineLB;
    VclPtr<FixedText>          m_pUnderlineColorFT;
    VclPtr<SvxColorListBox>    m_pUnderlineColorLB;

    VclPtr<CheckBox>           m_pIndividualWordsBtn;

    VclPtr<FixedText>          m_pEmphasisFT;
    VclPtr<ListBox>            m_pEmphasisLB;

    VclPtr<FixedText>          m_pPositionFT;
    VclPtr<ListBox>            m_pPositionLB;

    VclPtr<FixedText>          m_pA11yWarningFT;

    sal_uInt16          m_nHtmlMode;

                        SvxCharEffectsPage( vcl::Window* pParent, const SfxItemSet& rSet );

    void                Initialize();
    void                UpdatePreview_Impl();
    void                SetCaseMap_Impl( SvxCaseMap eCaseMap );
    void                ResetColor_Impl( const SfxItemSet& rSet );
    bool                FillItemSetColor_Impl( SfxItemSet& rSet );
    Color               GetPreviewFontColor(const Color& rColor) const;
    void                EnableNoneFontColor();

    void SelectHdl_Impl(ListBox*);
    DECL_LINK(SelectListBoxHdl_Impl, ListBox&, void);
    DECL_LINK(CbClickHdl_Impl, Button*, void);
    DECL_LINK(TristClickHdl_Impl, Button*, void);
    DECL_LINK(UpdatePreview_Impl, ListBox&, void);
    DECL_LINK(ColorBoxSelectHdl_Impl, SvxColorListBox&, void);

public:
    virtual ~SvxCharEffectsPage() override;
    virtual void dispose() override;

    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pEffectsRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                DisableControls( sal_uInt16 nDisable );
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxCharPositionPage ---------------------------------------------


class SvxCharPositionPage : public CharBasePage
{
    friend class VclPtr<SvxCharPositionPage>;
    static const sal_uInt16 pPositionRanges[];

private:
    short               m_nSuperEsc;
    short               m_nSubEsc;

    sal_uInt16              m_nScaleWidthItemSetVal;
    sal_uInt16              m_nScaleWidthInitialVal;

    sal_uInt8                m_nSuperProp;
    sal_uInt8                m_nSubProp;

    std::unique_ptr<weld::RadioButton> m_xHighPosBtn;
    std::unique_ptr<weld::RadioButton> m_xNormalPosBtn;
    std::unique_ptr<weld::RadioButton> m_xLowPosBtn;
    std::unique_ptr<weld::Label> m_xHighLowFT;
    std::unique_ptr<weld::MetricSpinButton> m_xHighLowMF;
    std::unique_ptr<weld::CheckButton> m_xHighLowRB;
    std::unique_ptr<weld::Label> m_xFontSizeFT;
    std::unique_ptr<weld::MetricSpinButton> m_xFontSizeMF;

    std::unique_ptr<weld::Widget> m_xRotationContainer;

    std::unique_ptr<weld::Label> m_xScalingFT;
    std::unique_ptr<weld::Label> m_xScalingAndRotationFT;
    std::unique_ptr<weld::RadioButton> m_x0degRB;
    std::unique_ptr<weld::RadioButton> m_x90degRB;
    std::unique_ptr<weld::RadioButton> m_x270degRB;
    std::unique_ptr<weld::CheckButton> m_xFitToLineCB;

    std::unique_ptr<weld::MetricSpinButton> m_xScaleWidthMF;

    std::unique_ptr<weld::MetricSpinButton> m_xKerningMF;
    std::unique_ptr<weld::CheckButton> m_xPairKerningBtn;

                        SvxCharPositionPage(TabPageParent pParent, const SfxItemSet& rSet);

    void                Initialize();
    void                UpdatePreview_Impl( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc );
    void                SetEscapement_Impl( SvxEscapement nEsc );

    DECL_LINK(PositionHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(RotationHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(AutoPositionHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(FitToLineHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(KerningSelectHdl_Impl, weld::ComboBoxText&, void);
    DECL_LINK(KerningModifyHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ValueChangedHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ScaleWidthModifyHdl_Impl, weld::MetricSpinButton&, void);
    void FontModifyHdl_Impl();

public:
    virtual ~SvxCharPositionPage() override;

    using SfxTabPage::DeactivatePage;

    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16*      GetRanges() { return pPositionRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;
    virtual void        FillUserData() override;
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

// class SvxCharTwoLinesPage ---------------------------------------------

class SvxCharTwoLinesPage : public CharBasePage
{
    friend class VclPtr<SvxCharTwoLinesPage>;
private:
    static const sal_uInt16 pTwoLinesRanges[];
    sal_uInt16              m_nStartBracketPosition;
    sal_uInt16              m_nEndBracketPosition;

    std::unique_ptr<weld::CheckButton>  m_xTwoLinesBtn;
    std::unique_ptr<weld::Widget> m_xEnclosingFrame;
    std::unique_ptr<weld::TreeView> m_xStartBracketLB;
    std::unique_ptr<weld::TreeView> m_xEndBracketLB;

    SvxCharTwoLinesPage(TabPageParent pParent, const SfxItemSet& rSet);

    void                UpdatePreview_Impl();
    void                Initialize();
    void                SelectCharacter(weld::TreeView* pBox);
    void                SetBracket(sal_Unicode cBracket, bool bStart);

    DECL_LINK(TwoLinesHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(CharacterMapHdl_Impl, weld::TreeView&, void);

public:
    virtual ~SvxCharTwoLinesPage() override;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16*  GetRanges() { return pTwoLinesRanges; }

    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CHARDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
