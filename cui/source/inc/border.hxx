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
#ifndef INCLUDED_CUI_SOURCE_INC_BORDER_HXX
#define INCLUDED_CUI_SOURCE_INC_BORDER_HXX

#include <editeng/shaditem.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/weld.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/algitem.hxx>
#include <svx/colorbox.hxx>
#include <svx/frmsel.hxx>
#include <svx/flagsdef.hxx>

#include <set>

// forward ---------------------------------------------------------------

namespace editeng
{
    class SvxBorderLine;
}

class ShadowControlsWrapper
{
public:
    explicit ShadowControlsWrapper(SvtValueSet& rVsPos, weld::MetricSpinButton& rMfSize, ColorListBox& rLbColor);

    SvxShadowItem GetControlValue(const SvxShadowItem& rItem) const;
    void SetControlValue(const SvxShadowItem& rItem);
    void SetControlDontKnow();

private:
    SvtValueSet&                        mrVsPos;
    weld::MetricSpinButton&             mrMfSize;
    ColorListBox&                       mrLbColor;
};

class MarginControlsWrapper
{
public:
    explicit MarginControlsWrapper(weld::MetricSpinButton& rMfLeft, weld::MetricSpinButton& rMfRight,
                                   weld::MetricSpinButton& rMfTop, weld::MetricSpinButton& rMfBottom);

    SvxMarginItem GetControlValue(const SvxMarginItem& rItem) const;
    void SetControlValue(const SvxMarginItem& rItem);
    void SetControlDontKnow();

    bool get_value_changed_from_saved() const;

private:
    weld::MetricSpinButton& mrLeftWrp;
    weld::MetricSpinButton& mrRightWrp;
    weld::MetricSpinButton& mrTopWrp;
    weld::MetricSpinButton& mrBottomWrp;
};

class SvxBorderTabPage : public SfxTabPage
{
    friend class VclPtr<SvxBorderTabPage>;
    using TabPage::DeactivatePage;

    static const sal_uInt16 pRanges[];

public:
    virtual ~SvxBorderTabPage() override;
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);
    static const sal_uInt16*      GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rCoreAttrs ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual void        ChangesApplied() override;

    void                HideShadowControls();
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
    void                SetTableMode();
protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    SvxBorderTabPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs);

    std::vector<Image> m_aShadowImgVec;
    std::vector<Image> m_aBorderImgVec;

    long                nMinValue;  ///< minimum distance
    SwBorderModes       nSWMode;    ///< table, textframe, paragraph
    sal_uInt16          mnBoxSlot;
    sal_uInt16          mnShadowSlot;

    bool                mbHorEnabled;       ///< true = Inner horizontal border enabled.
    bool                mbVerEnabled;       ///< true = Inner vertical border enabled.
    bool                mbTLBREnabled;      ///< true = Top-left to bottom-right border enabled.
    bool                mbBLTREnabled;      ///< true = Bottom-left to top-right border enabled.
    bool                mbUseMarginItem;
    bool                mbLeftModified;
    bool                mbRightModified;
    bool                mbTopModified;
    bool                mbBottomModified;
    bool                mbSync;
    bool                mbRemoveAdjacentCellBorders;
    bool                bIsCalcDoc;

    std::set<SvxBorderLineStyle> maUsedBorderStyles;

    // Controls
    svx::FrameSelector m_aFrameSel;
    std::unique_ptr<SvtValueSet> m_xWndPresets;
    std::unique_ptr<weld::CustomWeld> m_xWndPresetsWin;
    std::unique_ptr<weld::Label> m_xUserDefFT;
    std::unique_ptr<weld::CustomWeld> m_xFrameSelWin;

    std::unique_ptr<SvtLineListBox> m_xLbLineStyle;
    std::unique_ptr<ColorListBox> m_xLbLineColor;
    std::unique_ptr<weld::MetricSpinButton> m_xLineWidthMF;

    std::unique_ptr<weld::Container> m_xSpacingFrame;
    std::unique_ptr<weld::Label> m_xLeftFT;
    std::unique_ptr<weld::MetricSpinButton> m_xLeftMF;
    std::unique_ptr<weld::Label> m_xRightFT;
    std::unique_ptr<weld::MetricSpinButton> m_xRightMF;
    std::unique_ptr<weld::Label> m_xTopFT;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMF;
    std::unique_ptr<weld::Label> m_xBottomFT;
    std::unique_ptr<weld::MetricSpinButton> m_xBottomMF;
    std::unique_ptr<weld::CheckButton> m_xSynchronizeCB;

    std::unique_ptr<weld::Container> m_xShadowFrame;
    std::unique_ptr<SvtValueSet> m_xWndShadows;
    std::unique_ptr<weld::CustomWeld> m_xWndShadowsWin;
    std::unique_ptr<weld::Label> m_xFtShadowSize;
    std::unique_ptr<weld::MetricSpinButton> m_xEdShadowSize;
    std::unique_ptr<weld::Label> m_xFtShadowColor;
    std::unique_ptr<ColorListBox> m_xLbShadowColor;

    std::unique_ptr<weld::Container> m_xPropertiesFrame;///< properties - "Merge with next paragraph" in Writer
    std::unique_ptr<weld::CheckButton> m_xMergeWithNextCB;
    // #i29550#
    std::unique_ptr<weld::CheckButton> m_xMergeAdjacentBordersCB;
    std::unique_ptr<weld::CheckButton> m_xRemoveAdjcentCellBordersCB;
    std::unique_ptr<weld::Label> m_xRemoveAdjcentCellBordersFT;
    std::unique_ptr<ShadowControlsWrapper> m_xShadowControls;
    std::unique_ptr<MarginControlsWrapper> m_xMarginControls;

    // Handler
    DECL_LINK(SelStyleHdl_Impl, SvtLineListBox&, void);
    DECL_LINK(SelColHdl_Impl, ColorListBox&, void);
    DECL_LINK(SelPreHdl_Impl, SvtValueSet*, void);
    DECL_LINK(SelSdwHdl_Impl, SvtValueSet*, void);
    DECL_LINK(LinesChanged_Impl, LinkParamNone*, void);
    DECL_LINK(ModifyDistanceHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyWidthHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(SyncHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(RemoveAdjacentCellBorderHdl_Impl, weld::ToggleButton&, void);

    sal_uInt16          GetPresetImageId(sal_uInt16 nValueSetIdx) const;
    const char*         GetPresetStringId(sal_uInt16 nValueSetIdx) const;

    void                FillPresetVS();
    void                FillShadowVS();
    void                FillValueSets();

    // Filler
    void                FillLineListBox_Impl();

    /// share for individual Frame-/Core-Line
    void                ResetFrameLine_Impl( svx::FrameBorderType eBorder,
                                             const editeng::SvxBorderLine* pCurLine,
                                             bool bValid );

    bool IsBorderLineStyleAllowed( SvxBorderLineStyle nStyle ) const;
    void UpdateRemoveAdjCellBorderCB( sal_uInt16 nPreset );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
