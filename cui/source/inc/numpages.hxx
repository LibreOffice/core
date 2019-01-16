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
#ifndef INCLUDED_CUI_SOURCE_INC_NUMPAGES_HXX
#define INCLUDED_CUI_SOURCE_INC_NUMPAGES_HXX

#include <vector>
#include <memory>

#include <sfx2/tabdlg.hxx>
#include <svx/Palette.hxx>
#include <editeng/numdef.hxx>
#include <editeng/svxenum.hxx>
#include <svtools/ctrlbox.hxx>

#define MN_GALLERY_ENTRY 100

class ColorListBox;
class NumValueSet;
class SvxNumRule;
class SvxBmpNumValueSet;
class SvxBrushItem;
class ValueSet;

class SvxNumberingPreview : public weld::CustomWidgetController
{
    const SvxNumRule*   pActNum;
    vcl::Font           aStdFont;
    bool                bPosition;
    sal_uInt16          nActLevel;

protected:
    virtual void  Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect ) override;

public:
    SvxNumberingPreview();

    void    SetNumRule(const SvxNumRule* pNum)
                {pActNum = pNum; Invalidate();};
    void    SetPositionMode()
                    { bPosition = true;}
    void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};

struct SvxNumSettings_Impl
{
    SvxNumType nNumberType;
    short      nParentNumbering;
    OUString   sPrefix;
    OUString   sSuffix;
    OUString   sBulletChar;
    OUString   sBulletFont;
    SvxNumSettings_Impl() :
        nNumberType(SVX_NUM_CHARS_UPPER_LETTER),
        nParentNumbering(0)
        {}
};

typedef std::vector<std::unique_ptr<SvxNumSettings_Impl> > SvxNumSettingsArr_Impl;


class SvxSingleNumPickTabPage final : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    SvxNumSettingsArr_Impl  aNumSettingsArr;
    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;
    sal_uInt16              nActNumLvl;
    bool                    bModified   : 1;
    bool                    bPreset     : 1;

    sal_uInt16              nNumItemId;

    std::unique_ptr<NumValueSet> m_xExamplesVS;
    std::unique_ptr<weld::CustomWeld> m_xExamplesVSWin;

    DECL_LINK(NumSelectHdl_Impl, SvtValueSet*, void);
    DECL_LINK(DoubleClickHdl_Impl, SvtValueSet*, void);

public:
    SvxSingleNumPickTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxSingleNumPickTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SvxBulletPickTabPage final : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;
    sal_uInt16          nActNumLvl;
    bool                bModified   : 1;
    bool                bPreset     : 1;
    sal_uInt16          nNumItemId;

    OUString            sBulletCharFormatName;

    std::unique_ptr<NumValueSet> m_xExamplesVS;
    std::unique_ptr<weld::CustomWeld> m_xExamplesVSWin;

    DECL_LINK(NumSelectHdl_Impl, SvtValueSet*, void);
    DECL_LINK(DoubleClickHdl_Impl, SvtValueSet*, void);
public:
    SvxBulletPickTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxBulletPickTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

#define NUM_VALUSET_COUNT 16

/// TabPage for complete numeration
class SvxNumPickTabPage final : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    OUString            sNumCharFmtName;
    OUString            sBulletCharFormatName;

    SvxNumSettingsArr_Impl  aNumSettingsArrays[NUM_VALUSET_COUNT];  // is initialized with the five formats

    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    bool                bModified   : 1;
    bool                bPreset     : 1;

    std::unique_ptr<NumValueSet> m_xExamplesVS;
    std::unique_ptr<weld::CustomWeld> m_xExamplesVSWin;

    DECL_LINK(NumSelectHdl_Impl, SvtValueSet*, void);
    DECL_LINK(DoubleClickHdl_Impl, SvtValueSet*, void);

public:
    SvxNumPickTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxNumPickTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                SetCharFormatNames(const OUString& rCharName, const OUString& rBulName)
                            {   sNumCharFmtName = rCharName;
                                sBulletCharFormatName = rBulName;}
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

class SvxBitmapPickTabPage final : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    std::vector<OUString> aGrfNames;

    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;
    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    MapUnit             eCoreUnit;
    bool                bModified   : 1;
    bool                bPreset     : 1;

    std::unique_ptr<weld::Label> m_xErrorText;
    std::unique_ptr<weld::Button> m_xBtBrowseFile;
    std::unique_ptr<SvxBmpNumValueSet> m_xExamplesVS;
    std::unique_ptr<weld::CustomWeld> m_xExamplesVSWin;

    DECL_LINK(NumSelectHdl_Impl, SvtValueSet*, void);
    DECL_LINK(DoubleClickHdl_Impl, SvtValueSet*, void);
    DECL_LINK(ClickAddBrowseHdl_Impl, weld::Button&, void);

public:
    SvxBitmapPickTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxBitmapPickTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent,
                                      const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SvxNumOptionsTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    OUString        m_sNumCharFmtName;
    OUString        m_sBulletCharFormatName;

    Timer           aInvalidateTimer;

    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;

    Size                aInitSize[SVX_MAX_NUM];

    bool                bLastWidthModified  : 1;
    bool                bModified           : 1;
    bool                bPreset             : 1;
    bool                bAutomaticCharStyles: 1;
    bool                bHTMLMode           : 1;

    std::vector<OUString> aGrfNames;
    vcl::Font             aActBulletFont;

    sal_uInt8           nBullet;
    sal_uInt16          nActNumLvl;
    sal_uInt16          nNumItemId;
    MapUnit             eCoreUnit;

    SvxNumberingPreview m_aPreviewWIN;
    std::unique_ptr<weld::Widget> m_xGrid;
    std::unique_ptr<weld::TreeView> m_xLevelLB;
    std::unique_ptr<weld::ComboBox> m_xFmtLB;
    std::unique_ptr<weld::Label> m_xSeparatorFT;
    std::unique_ptr<weld::Label> m_xPrefixFT;
    std::unique_ptr<weld::Entry> m_xPrefixED;
    std::unique_ptr<weld::Label> m_xSuffixFT;
    std::unique_ptr<weld::Entry> m_xSuffixED;
    std::unique_ptr<weld::Label> m_xCharFmtFT;
    std::unique_ptr<weld::ComboBox> m_xCharFmtLB;
    std::unique_ptr<weld::Label> m_xBulColorFT;
    std::unique_ptr<ColorListBox> m_xBulColLB;
    std::unique_ptr<weld::Label> m_xBulRelSizeFT;
    std::unique_ptr<weld::MetricSpinButton> m_xBulRelSizeMF;
    std::unique_ptr<weld::Label> m_xAllLevelFT;
    std::unique_ptr<weld::SpinButton> m_xAllLevelNF;
    std::unique_ptr<weld::Label> m_xStartFT;
    std::unique_ptr<weld::SpinButton> m_xStartED;
    std::unique_ptr<weld::Label> m_xBulletFT;
    std::unique_ptr<weld::Button> m_xBulletPB;
    std::unique_ptr<weld::Label> m_xBitmapFT;
    std::unique_ptr<weld::MenuButton> m_xBitmapMB;
    std::unique_ptr<weld::Label> m_xWidthFT;
    std::unique_ptr<weld::MetricSpinButton> m_xWidthMF;
    std::unique_ptr<weld::Label> m_xHeightFT;
    std::unique_ptr<weld::MetricSpinButton> m_xHeightMF;
    std::unique_ptr<weld::CheckButton> m_xRatioCB;
    std::unique_ptr<weld::Label> m_xOrientFT;
    std::unique_ptr<weld::ComboBox> m_xOrientLB;
    std::unique_ptr<weld::Widget> m_xAllLevelsFrame;
    std::unique_ptr<weld::Menu> m_xGalleryMenu;
    std::unique_ptr<weld::CheckButton> m_xSameLevelCB;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWIN;

    void                InitControls();
    /** To switch between the numbering type
        0 - Number;
        1 - Bullet;
        2 - Bitmap; */
    void                SwitchNumberType( sal_uInt8 nType );
    void                CheckForStartValue_Impl(sal_uInt16 nNumberingType);

    DECL_LINK(NumberTypeSelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(LevelHdl_Impl, weld::TreeView&, void);
    DECL_LINK(PopupActivateHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(GraphicHdl_Impl, const OString&, void);
    DECL_LINK(BulletHdl_Impl, weld::Button&, void);
    DECL_LINK(SizeHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(RatioHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(CharFmtHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(EditModifyHdl_Impl, weld::Entry&, void);
    DECL_LINK(EditListBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(AllLevelHdl_Impl, weld::SpinButton&, void);
    DECL_LINK(OrientHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(SameLevelHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(BulColorHdl_Impl, ColorListBox&, void);
    DECL_LINK(BulRelSizeHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(PreviewInvalidateHdl_Impl, Timer*, void);
    void EditModifyHdl_Impl(const weld::Entry*);

public:
    SvxNumOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxNumOptionsTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                SetCharFmts(const OUString& rNumName, const OUString& rBulletName)
                        {
                            m_sNumCharFmtName = rNumName;
                            m_sBulletCharFormatName = rBulletName;
                        }
    void                SetMetric(FieldUnit eSet);

    void                SetModified(bool bRepaint = true);
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};


class SvxNumPositionTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;

    sal_uInt16              nActNumLvl;
    sal_uInt16              nNumItemId;
    MapUnit             eCoreUnit;

    bool                bModified           : 1;
    bool                bPreset             : 1;
    bool                bInInintControl     : 1;  // workaround for Modify-error, is said to be corrected from 391 on
    bool                bLabelAlignmentPosAndSpaceModeActive;

    SvxNumberingPreview m_aPreviewWIN;
    std::unique_ptr<weld::TreeView> m_xLevelLB;
    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    std::unique_ptr<weld::Label> m_xDistBorderFT;
    std::unique_ptr<weld::MetricSpinButton> m_xDistBorderMF;
    std::unique_ptr<weld::CheckButton> m_xRelativeCB;
    std::unique_ptr<weld::Label> m_xIndentFT;
    std::unique_ptr<weld::MetricSpinButton> m_xIndentMF;
    std::unique_ptr<weld::Label> m_xDistNumFT;
    std::unique_ptr<weld::MetricSpinButton> m_xDistNumMF;
    std::unique_ptr<weld::Label> m_xAlignFT;
    std::unique_ptr<weld::ComboBox> m_xAlignLB;
    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    std::unique_ptr<weld::Label> m_xLabelFollowedByFT;
    std::unique_ptr<weld::ComboBox> m_xLabelFollowedByLB;
    std::unique_ptr<weld::Label> m_xListtabFT;
    std::unique_ptr<weld::MetricSpinButton> m_xListtabMF;
    std::unique_ptr<weld::Label>m_xAlign2FT;
    std::unique_ptr<weld::ComboBox> m_xAlign2LB;
    std::unique_ptr<weld::Label> m_xAlignedAtFT;
    std::unique_ptr<weld::MetricSpinButton> m_xAlignedAtMF;
    std::unique_ptr<weld::Label> m_xIndentAtFT;
    std::unique_ptr<weld::MetricSpinButton> m_xIndentAtMF;
    std::unique_ptr<weld::Button> m_xStandardPB;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWIN;

    void                InitControls();

    DECL_LINK(LevelHdl_Impl, weld::TreeView&, void);
    DECL_LINK(EditModifyHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(DistanceHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(DistanceFocusHdl_Impl, Control&, void);
    DECL_LINK(RelativeHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(StandardHdl_Impl, weld::Button&, void);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK(LabelFollowedByHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ListtabPosHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(AlignAtHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(IndentAtHdl_Impl, weld::MetricSpinButton&, void);

public:
    SvxNumPositionTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxNumPositionTabPage() override;
    virtual void dispose() override;

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    void                SetMetric(FieldUnit eSet);
    void                SetModified();
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
