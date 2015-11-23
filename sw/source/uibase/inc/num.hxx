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
#include <vcl/menubtn.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svx/stddlg.hxx>
#include <numprevw.hxx>
#include "numrule.hxx"

class SwWrtShell;
class SvxBrushItem;
class SwOutlineTabDialog;

class SwNumPositionTabPage : public SfxTabPage
{
    VclPtr<ListBox> m_pLevelLB;
    VclPtr<VclFrame> m_pPositionFrame;

    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    VclPtr<FixedText>          m_pDistBorderFT;
    VclPtr<MetricField>        m_pDistBorderMF;
    VclPtr<CheckBox>           m_pRelativeCB;
    VclPtr<FixedText>          m_pIndentFT;
    VclPtr<MetricField>        m_pIndentMF;
    VclPtr<FixedText>          m_pDistNumFT;
    VclPtr<MetricField>        m_pDistNumMF;
    VclPtr<FixedText>          m_pAlignFT;
    VclPtr<ListBox>            m_pAlignLB;

    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    VclPtr<FixedText>          m_pLabelFollowedByFT;
    VclPtr<ListBox>            m_pLabelFollowedByLB;
    VclPtr<FixedText>          m_pListtabFT;
    VclPtr<MetricField>        m_pListtabMF;
    VclPtr<FixedText>          m_pAlign2FT;
    VclPtr<ListBox>            m_pAlign2LB;
    VclPtr<FixedText>          m_pAlignedAtFT;
    VclPtr<MetricField>        m_pAlignedAtMF;
    VclPtr<FixedText>          m_pIndentAtFT;
    VclPtr<MetricField>        m_pIndentAtMF;

    VclPtr<PushButton>         m_pStandardPB;

    VclPtr<NumberingPreview>   m_pPreviewWIN;

    SwNumRule*          pActNum;
    SwNumRule*          pSaveNum;
    SwWrtShell*         pWrtSh;

    VclPtr<SwOutlineTabDialog> pOutlineDlg;
    sal_uInt16              nActNumLvl;

    bool                bModified           : 1;
    bool                bPreset             : 1;
    bool                bInInintControl     : 1;  // work around modify-error; should be resolved from 391 on
    bool                bLabelAlignmentPosAndSpaceModeActive;

    void                InitControls();

    DECL_LINK_TYPED( LevelHdl, ListBox&, void );
    DECL_LINK_TYPED(EditModifyHdl, ListBox&, void);
    DECL_LINK_TYPED( DistanceHdl, SpinField&, void );
    DECL_LINK_TYPED( DistanceLoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( RelativeHdl, Button*, void );
    DECL_LINK_TYPED(StandardHdl, Button*, void);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK_TYPED(LabelFollowedByHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED( ListtabPosHdl_Impl, SpinField&, void );
    DECL_LINK_TYPED( AlignAtHdl_Impl, SpinField&, void );
    DECL_LINK_TYPED( IndentAtHdl_Impl, SpinField&, void );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    SwNumPositionTabPage(vcl::Window* pParent,
                               const SfxItemSet& rSet);
    virtual ~SwNumPositionTabPage();
    virtual void        dispose() override;

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual sfxpg       DeactivatePage(SfxItemSet *pSet) override;
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent,
                                      const SfxItemSet* rAttrSet);

    void                SetOutlineTabDialog(SwOutlineTabDialog* pDlg){pOutlineDlg = pDlg;}
    void                SetWrtShell(SwWrtShell* pSh);
#ifdef DBG_UTIL
    void                SetModified(bool bRepaint = true);
#else
    void                SetModified(bool bRepaint = true)
                            {   bModified = true;
                                if(bRepaint)
                                {
                                    m_pPreviewWIN->SetLevel(nActNumLvl);
                                    m_pPreviewWIN->Invalidate();
                                }
                            }
#endif
};

class SwSvxNumBulletTabDialog : public SfxTabDialog
{
    SwWrtShell&         rWrtSh;
    sal_uInt16 m_nSingleNumPageId;
    sal_uInt16 m_nBulletPageId;
    sal_uInt16 m_nOptionsPageId;
    sal_uInt16 m_nPositionPageId;

    protected:
        virtual short   Ok() override;
        virtual void    PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage) override;
        DECL_LINK_TYPED(RemoveNumberingHdl, Button*, void);
    public:
        SwSvxNumBulletTabDialog(vcl::Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
        virtual ~SwSvxNumBulletTabDialog();
};
#endif // INCLUDED_SW_SOURCE_UIBASE_INC_NUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
