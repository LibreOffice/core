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

#include <sdabstdlg.hxx>

//AbstractDialogFactory_Impl implementations
class SdAbstractDialogFactory_Impl : public SdAbstractDialogFactory
{

public:
    virtual ~SdAbstractDialogFactory_Impl() {}

    virtual VclPtr<AbstractSvxBulletAndPositionDlg> CreateSvxBulletAndPositionDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) override;
    virtual VclPtr<VclAbstractDialog>          CreateBreakDlg(weld::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount) override;
    virtual VclPtr<AbstractCopyDlg>            CreateCopyDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView) override;
    virtual VclPtr<AbstractSdCustomShowDlg>    CreateSdCustomShowDlg(weld::Window* pParent, SdDrawDocument& rDrawDoc) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabCharDialog(weld::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabPageDialog(weld::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage, bool bIsImpressDoc) override;
    virtual VclPtr<AbstractSdModifyFieldDlg>   CreateSdModifyFieldDlg(weld::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet) override;
    virtual VclPtr<AbstractSdSnapLineDlg>      CreateSdSnapLineDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView) override;
    virtual VclPtr<AbstractSdInsertLayerDlg>   CreateSdInsertLayerDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr) override;
    virtual VclPtr<AbstractSdInsertPagesObjsDlg> CreateSdInsertPagesObjsDlg(weld::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName ) override;
    virtual VclPtr<AbstractMorphDlg>           CreateMorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdOutlineBulletTabDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdParagraphTabDlg(weld::Window* pParent, const SfxItemSet* pAttr) override;
    virtual VclPtr<AbstractSdStartPresDlg>     CreateSdStartPresentationDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) override;
    virtual VclPtr<VclAbstractDialog>          CreateRemoteDialog(weld::Window* pWindow) override; // add for RemoteDialog
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdPresLayoutTemplateDlg(SfxObjectShell* pDocSh, weld::Window* pParent, bool bBackgroundDlg, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool) override;
    virtual VclPtr<AbstractSdPresLayoutDlg>    CreateSdPresLayoutDlg(weld::Window* pParent, ::sd::DrawDocShell* pDocShell, const SfxItemSet& rInAttrs) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabTemplateDlg(weld::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView ) override;
    virtual VclPtr<SfxAbstractDialog>          CreatSdActionDialog(weld::Window* pParent, const SfxItemSet& rAttr, ::sd::View* pView) override;
    virtual VclPtr<AbstractSdVectorizeDlg>     CreateSdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell) override;

    virtual VclPtr<VclAbstractDialog>          CreateSdPhotoAlbumDialog(weld::Window* pWindow, SdDrawDocument* pDoc) override;

    virtual VclPtr<VclAbstractDialog>          CreateMasterLayoutDialog(weld::Window* pParent, SdDrawDocument* pDoc, SdPage*) override;

    virtual VclPtr<AbstractHeaderFooterDialog> CreateHeaderFooterDialog(sd::ViewShell* pViewShell,
                                                                        weld::Window* pParent,
                                                                        SdDrawDocument* pDoc,
                                                                        SdPage* pCurrentPage) override;

    // For TabPage
    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
