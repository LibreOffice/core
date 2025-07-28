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

#include <editeng/shaditem.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/weld.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/algitem.hxx>
#include <svx/colorbox.hxx>
#include <svx/frmsel.hxx>
#include <svx/flagsdef.hxx>
#include <unotools/resmgr.hxx>

#include <set>

// forward ---------------------------------------------------------------

namespace editeng
{
    class SvxBorderLine;
}

class ShadowControlsWrapper
{
public:
    explicit ShadowControlsWrapper(weld::IconView& rIvPos, weld::MetricSpinButton& rMfSize, ColorListBox& rLbColor);

    SvxShadowItem GetControlValue(const SvxShadowItem& rItem) const;
    void SetControlValue(const SvxShadowItem& rItem);
    void SetControlDontKnow();

    bool get_value_changed_from_saved() const;

private:
    weld::IconView&                     mrIvPos;
    weld::MetricSpinButton&             mrMfSize;
    ColorListBox&                       mrLbColor;
    OUString                            msSavedShadowItemId;
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
    static const WhichRangesContainer pRanges;

public:
    SvxBorderTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs);
    virtual ~SvxBorderTabPage() override;
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                const SfxItemSet* rAttrSet);
    static const WhichRangesContainer & GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rCoreAttrs ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual void        ChangesApplied() override;

    void                HideShadowControls();
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
    void                SetTableMode();
protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

private:
    std::vector<Image> m_aShadowImgVec;
    std::vector<Image> m_aBorderImgVec;

    tools::Long                nMinValue;  ///< minimum distance
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
    std::unique_ptr<weld::IconView> m_xWndPresets;
    std::unique_ptr<weld::Label> m_xUserDefFT;
    std::unique_ptr<weld::CustomWeld> m_xFrameSelWin;

    std::unique_ptr<SvtLineListBox> m_xLbLineStyle;
    std::unique_ptr<ColorListBox> m_xLbLineColor;
    std::unique_ptr<weld::ComboBox> m_xLineWidthLB;
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
    std::unique_ptr<weld::IconView> m_xWndShadows;
    std::unique_ptr<weld::Label> m_xFtShadowSize;
    std::unique_ptr<weld::MetricSpinButton> m_xEdShadowSize;
    std::unique_ptr<weld::Label> m_xFtShadowColor;
    std::unique_ptr<ColorListBox> m_xLbShadowColor;

    std::unique_ptr<weld::Container> m_xPropertiesFrame;///< properties - "Merge with next paragraph" in Writer
    std::unique_ptr<weld::CheckButton> m_xMergeWithNextCB;
    // #i29550#
    std::unique_ptr<weld::CheckButton> m_xMergeAdjacentBordersCB;
    std::unique_ptr<weld::CheckButton> m_xRemoveAdjacentCellBordersCB;
    std::unique_ptr<weld::Label> m_xRemoveAdjacentCellBordersFT;
    std::unique_ptr<ShadowControlsWrapper> m_xShadowControls;
    std::unique_ptr<MarginControlsWrapper> m_xMarginControls;

    // Handler
    DECL_LINK(SelStyleHdl_Impl, SvtLineListBox&, void);
    DECL_LINK(SelColHdl_Impl, ColorListBox&, void);
    DECL_LINK(SelPreHdl_Impl, weld::IconView&, void);
    DECL_LINK(FocusOutPresets_Impl, weld::Widget&, void);
    DECL_LINK(SelSdwHdl_Impl, weld::IconView&, void);
    DECL_LINK(LinesChanged_Impl, LinkParamNone*, void);
    DECL_LINK(ModifyDistanceHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyWidthLBHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ModifyWidthMFHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(SyncHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(RemoveAdjacentCellBorderHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(QueryTooltipPreHdl, const weld::TreeIter&, OUString);
    DECL_LINK(QueryTooltipSdwHdl, const weld::TreeIter&, OUString);

    sal_uInt16          GetPresetImageId(sal_uInt16 nIconViewIdx) const;
    OUString            GetPresetName(sal_uInt16 nIconViewIdx) const;

    void                FillPresetIV();
    void                FillShadowIV();
    void                FillIconViews();
    void                SetLineWidth(sal_Int64 nWidth, sal_Int32 nRemovedType = 0);

    // Filler
    void                FillLineListBox_Impl();

    /// share for individual Frame-/Core-Line
    void                ResetFrameLine_Impl( svx::FrameBorderType eBorder,
                                             const editeng::SvxBorderLine* pCurLine,
                                             bool bValid );

    bool IsBorderLineStyleAllowed( SvxBorderLineStyle nStyle ) const;
    void UpdateRemoveAdjCellBorderCB( sal_uInt16 nPreset );

    static OUString GetShadowTypeName(sal_uInt16 nIconViewIdx);
    static Bitmap GetPreviewAsBitmap(const Image& rImage);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
