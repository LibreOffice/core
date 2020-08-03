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

#include <vector>
#include <memory>

#include <editeng/numdef.hxx>
#include <editeng/svxenum.hxx>
#include <vcl/weld.hxx>
#include "View.hxx"
#include <cui/numberingpreview.hxx>

#define MN_GALLERY_ENTRY 100

class ColorListBox;
class SvxNumValueSet;
class SvxNumRule;
class SvxBmpNumValueSet;
class SvxBrushItem;
class SdDrawDocument;

namespace sd
{
class View;
}

/// Main class for handling the bullets, numbering format and their position.
class SvxBulletAndPositionDlg : public weld::GenericDialogController
{
    OUString m_sNumCharFmtName;
    OUString m_sBulletCharFormatName;

    Timer aInvalidateTimer;

    std::unique_ptr<SvxNumRule> pActNum;
    std::unique_ptr<SvxNumRule> pSaveNum;
    const SfxItemSet& rFirstStateSet;

    Size aInitSize[SVX_MAX_NUM];

    bool bLastWidthModified : 1;
    bool bModified : 1;
    bool bInInitControl : 1; // workaround for Modify-error, is said to be corrected from 391 on
    bool bLabelAlignmentPosAndSpaceModeActive;
    bool bApplyToMaster;

    std::vector<OUString> aGrfNames;
    vcl::Font aActBulletFont;

    sal_uInt8 nBullet;
    sal_uInt16 nActNumLvl;
    weld::Window* p_Window;
    sal_uInt16 nNumItemId;
    MapUnit eCoreUnit;

    SvxNumberingPreview m_aPreviewWIN;
    std::unique_ptr<weld::Widget> m_xGrid;
    std::unique_ptr<weld::TreeView> m_xLevelLB;
    std::unique_ptr<weld::ComboBox> m_xFmtLB;
    std::unique_ptr<weld::Label> m_xPrefixFT;
    std::unique_ptr<weld::Entry> m_xPrefixED;
    std::unique_ptr<weld::Label> m_xSuffixFT;
    std::unique_ptr<weld::Entry> m_xSuffixED;
    std::unique_ptr<weld::Frame> m_xBeforeAfter;
    std::unique_ptr<weld::Label> m_xBulColorFT;
    std::unique_ptr<ColorListBox> m_xBulColLB;
    std::unique_ptr<weld::Label> m_xBulRelSizeFT;
    std::unique_ptr<weld::MetricSpinButton> m_xBulRelSizeMF;
    std::unique_ptr<weld::Label> m_xStartFT;
    std::unique_ptr<weld::SpinButton> m_xStartED;
    std::unique_ptr<weld::Label> m_xBulletFT;
    std::unique_ptr<weld::Button> m_xBulletPB;
    std::unique_ptr<weld::MenuButton> m_xBitmapMB;
    std::unique_ptr<weld::Label> m_xWidthFT;
    std::unique_ptr<weld::MetricSpinButton> m_xWidthMF;
    std::unique_ptr<weld::Label> m_xHeightFT;
    std::unique_ptr<weld::MetricSpinButton> m_xHeightMF;
    std::unique_ptr<weld::CheckButton> m_xRatioCB;
    std::unique_ptr<weld::Menu> m_xGalleryMenu;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWIN;
    std::unique_ptr<weld::Label> m_xDistBorderFT;
    std::unique_ptr<weld::MetricSpinButton> m_xDistBorderMF;
    std::unique_ptr<weld::CheckButton> m_xRelativeCB;
    std::unique_ptr<weld::Label> m_xIndentFT;
    std::unique_ptr<weld::MetricSpinButton> m_xIndentMF;
    std::unique_ptr<weld::ToggleButton> m_xLeftTB;
    std::unique_ptr<weld::ToggleButton> m_xCenterTB;
    std::unique_ptr<weld::ToggleButton> m_xRightTB;
    std::unique_ptr<weld::RadioButton> m_xSlideRB;
    std::unique_ptr<weld::RadioButton> m_xSelectionRB;
    std::unique_ptr<weld::ToggleButton> m_xApplyToMaster;
    std::unique_ptr<weld::Button> m_xReset;

    void InitControls();
    /** To switch between the numbering type
        0 - Number;
        1 - Bullet;
        2 - Bitmap; */
    void SwitchNumberType(sal_uInt8 nType);
    void CheckForStartValue_Impl(sal_uInt16 nNumberingType);

    DECL_LINK(NumberTypeSelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(LevelHdl_Impl, weld::TreeView&, void);
    DECL_LINK(PopupActivateHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(GraphicHdl_Impl, const OString&, void);
    DECL_LINK(BulletHdl_Impl, weld::Button&, void);
    DECL_LINK(SizeHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(RatioHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(EditModifyHdl_Impl, weld::Entry&, void);
    DECL_LINK(BulColorHdl_Impl, ColorListBox&, void);
    DECL_LINK(BulRelSizeHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(PreviewInvalidateHdl_Impl, Timer*, void);
    DECL_LINK(DistanceHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(RelativeHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(SelectLeftAlignmentHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(SelectCenterAlignmentHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(SelectRightAlignmentHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ApplyToMasterHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ResetHdl_Impl, weld::Button&, void);
    void EditModifyHdl_Impl(const weld::Entry*);
    void InitPosAndSpaceMode();
    void SetAlignmentHdl_Impl(SvxAdjust);

public:
    SvxBulletAndPositionDlg(weld::Window* pWindow, const SfxItemSet& rSet, const ::sd::View* pView);
    virtual ~SvxBulletAndPositionDlg() override;

    SfxItemSet* GetOutputItemSet(SfxItemSet* rSet);
    bool IsApplyToMaster() const;
    bool IsSlideScope() const;
    void Reset(const SfxItemSet* rSet);

    void SetCharFmts(const OUString& rNumName, const OUString& rBulletName)
    {
        m_sNumCharFmtName = rNumName;
        m_sBulletCharFormatName = rBulletName;
    }
    void SetMetric(FieldUnit eSet);

    void SetModified(bool bRepaint = true);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
