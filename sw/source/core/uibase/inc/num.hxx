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
#ifndef INCLUDED_SW_SOURCE_UI_INC_NUM_HXX
#define INCLUDED_SW_SOURCE_UI_INC_NUM_HXX

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

struct SwBmpItemInfo
{
    SvxBrushItem*   pBrushItem;
    sal_uInt16          nItemId;
};

#define NUM_PAGETYPE_BULLET         0
#define NUM_PAGETYPE_SINGLENUM      1
#define NUM_PAGETYPE_NUM            2
#define NUM_PAGETYPE_BMP            3
#define PAGETYPE_USER_START         10

class SwNumPositionTabPage : public SfxTabPage
{
    ListBox* m_pLevelLB;
    VclFrame* m_pPositionFrame;

    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    FixedText*          m_pDistBorderFT;
    MetricField*        m_pDistBorderMF;
    CheckBox*           m_pRelativeCB;
    FixedText*          m_pIndentFT;
    MetricField*        m_pIndentMF;
    FixedText*          m_pDistNumFT;
    MetricField*        m_pDistNumMF;
    FixedText*          m_pAlignFT;
    ListBox*            m_pAlignLB;

    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    FixedText*          m_pLabelFollowedByFT;
    ListBox*            m_pLabelFollowedByLB;
    FixedText*          m_pListtabFT;
    MetricField*        m_pListtabMF;
    FixedText*          m_pAlign2FT;
    ListBox*            m_pAlign2LB;
    FixedText*          m_pAlignedAtFT;
    MetricField*        m_pAlignedAtMF;
    FixedText*          m_pIndentAtFT;
    MetricField*        m_pIndentAtMF;

    PushButton*         m_pStandardPB;

    NumberingPreview*   m_pPreviewWIN;

    SwNumRule*          pActNum;
    SwNumRule*          pSaveNum;
    SwWrtShell*         pWrtSh;

    SwOutlineTabDialog* pOutlineDlg;
    sal_uInt16              nActNumLvl;

    sal_Bool                bModified           : 1;
    sal_Bool                bPreset             : 1;
    sal_Bool                bInInintControl     : 1;  // work around modify-error; should be resolved from 391 on
    bool                bLabelAlignmentPosAndSpaceModeActive;

    void                InitControls();

    DECL_LINK( LevelHdl, ListBox * );
    DECL_LINK(EditModifyHdl, void *);
    DECL_LINK( DistanceHdl, MetricField * );
    DECL_LINK( RelativeHdl, CheckBox * );
    DECL_LINK(StandardHdl, void *);

    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK(LabelFollowedByHdl_Impl, void *);
    DECL_LINK( ListtabPosHdl_Impl, MetricField* );
    DECL_LINK( AlignAtHdl_Impl, MetricField* );
    DECL_LINK( IndentAtHdl_Impl, MetricField* );

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet);
    virtual ~SwNumPositionTabPage();

    virtual void        ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int         DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetOutlineTabDialog(SwOutlineTabDialog* pDlg){pOutlineDlg = pDlg;}
    void                SetWrtShell(SwWrtShell* pSh);
#ifdef DBG_UTIL
    void                SetModified(sal_Bool bRepaint = sal_True);
#else
    void                SetModified(sal_Bool bRepaint = sal_True)
                            {   bModified = sal_True;
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
        virtual short   Ok() SAL_OVERRIDE;
        virtual void    PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage) SAL_OVERRIDE;
        DECL_LINK(RemoveNumberingHdl, void *);
    public:
        SwSvxNumBulletTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
        virtual ~SwSvxNumBulletTabDialog();
};
#endif // INCLUDED_SW_SOURCE_UI_INC_NUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
