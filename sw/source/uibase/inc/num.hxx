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

#include <sfx2/tabdlg.hxx>
#include "numprevw.hxx"
#include <numrule.hxx>

class SwWrtShell;
class SvxBrushItem;
class SwOutlineTabDialog;

class SwNumPositionTabPage : public SfxTabPage
{
    std::unique_ptr<SwNumRule> pActNum;
    SwNumRule*          pSaveNum;
    SwWrtShell*         pWrtSh;

    SwOutlineTabDialog* pOutlineDlg;
    sal_uInt16          nActNumLvl;

    bool                bModified           : 1;
    bool                bPreset             : 1;
    bool                bInInintControl     : 1;  // work around modify-error; should be resolved from 391 on
    bool                bLabelAlignmentPosAndSpaceModeActive;

    NumberingPreview  m_aPreviewWIN;

    std::unique_ptr<weld::TreeView> m_xLevelLB;
    std::unique_ptr<weld::Widget> m_xPositionFrame;

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
    std::unique_ptr<weld::Label> m_xAlign2FT;
    std::unique_ptr<weld::ComboBox> m_xAlign2LB;
    std::unique_ptr<weld::Label> m_xAlignedAtFT;
    std::unique_ptr<weld::MetricSpinButton> m_xAlignedAtMF;
    std::unique_ptr<weld::Label> m_xIndentAtFT;
    std::unique_ptr<weld::MetricSpinButton> m_xIndentAtMF;
    std::unique_ptr<weld::Button> m_xStandardPB;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWIN;


    void                InitControls();

    DECL_LINK(LevelHdl, weld::TreeView&, void);
    DECL_LINK(EditModifyHdl, weld::ComboBox&, void);
    DECL_LINK(DistanceHdl, weld::MetricSpinButton&, void);
    DECL_LINK(RelativeHdl, weld::ToggleButton&, void);
    DECL_LINK(StandardHdl, weld::Button&, void);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK(LabelFollowedByHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ListtabPosHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(AlignAtHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(IndentAtHdl_Impl, weld::MetricSpinButton&, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    SwNumPositionTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwNumPositionTabPage() override;
    virtual void        dispose() override;

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    static VclPtr<SfxTabPage> Create( TabPageParent pParent,
                                      const SfxItemSet* rAttrSet);

    void                SetOutlineTabDialog(SwOutlineTabDialog* pDlg){pOutlineDlg = pDlg;}
    void                SetWrtShell(SwWrtShell* pSh);
#ifdef DBG_UTIL
    void                SetModified();
#else
    void                SetModified()
    {
        bModified = true;
        m_aPreviewWIN.SetLevel(nActNumLvl);
        m_aPreviewWIN.Invalidate();
    }
#endif
};

class SwSvxNumBulletTabDialog final : public SfxTabDialogController
{
    SwWrtShell&         rWrtSh;

    virtual short   Ok() override;
    virtual void    PageCreated(const OString& rPageId, SfxTabPage& rPage) override;
    DECL_LINK(RemoveNumberingHdl, weld::Button&, void);

    std::unique_ptr<weld::ComboBox> m_xDummyCombo;

public:
    SwSvxNumBulletTabDialog(weld::Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
    virtual ~SwSvxNumBulletTabDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
