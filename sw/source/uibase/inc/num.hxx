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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUM_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUM_HXX

#include <sfx2/tabdlg.hxx>
#include "numprevw.hxx"
#include <numrule.hxx>

class SwWrtShell;
class SwOutlineTabDialog;

class SwNumPositionTabPage final : public SfxTabPage
{
    std::unique_ptr<SwNumRule> m_pActNum;
    SwNumRule*          m_pSaveNum;
    SwWrtShell*         m_pWrtSh;

    SwOutlineTabDialog* m_pOutlineDlg;
    sal_uInt16          m_nActNumLvl;

    bool                m_bModified           : 1;
    bool                m_bPreset             : 1;
    bool                m_bInInintControl     : 1;  // work around modify-error; should be resolved from 391 on
    bool                m_bLabelAlignmentPosAndSpaceModeActive;

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
    DECL_LINK(RelativeHdl, weld::Toggleable&, void);
    DECL_LINK(StandardHdl, weld::Button&, void);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK(LabelFollowedByHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ListtabPosHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(AlignAtHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(IndentAtHdl_Impl, weld::MetricSpinButton&, void);

public:

    SwNumPositionTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwNumPositionTabPage() override;

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                      const SfxItemSet* rAttrSet);

    void                SetOutlineTabDialog(SwOutlineTabDialog* pDlg){m_pOutlineDlg = pDlg;}
    void                SetWrtShell(SwWrtShell* pSh);
#ifdef DBG_UTIL
    void                SetModified();
#else
    void                SetModified()
    {
        m_bModified = true;
        m_aPreviewWIN.SetLevel(m_nActNumLvl);
        m_aPreviewWIN.Invalidate();
    }
#endif
};

class SwSvxNumBulletTabDialog final : public SfxTabDialogController
{
    SwWrtShell&         m_rWrtSh;

    virtual short   Ok() override;
    virtual void    PageCreated(const OUString& rPageId, SfxTabPage& rPage) override;
    DECL_LINK(RemoveNumberingHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);

    std::unique_ptr<weld::ComboBox> m_xDummyCombo;

public:
    SwSvxNumBulletTabDialog(weld::Window* pParent,
                    const SfxItemSet& rSwItemSet,
                    SwWrtShell &);
    virtual ~SwSvxNumBulletTabDialog() override;
};
#endif // INCLUDED_SW_SOURCE_UIBASE_INC_NUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
