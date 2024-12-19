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

#include "sddlgfact.hxx"
#include <BreakDlg.hxx>
#include <BulletAndPositionDlg.hxx>
#include <copydlg.hxx>
#include <custsdlg.hxx>
#include <dlg_char.hxx>
#include <dlgpage.hxx>
#include <dlgfield.hxx>
#include <dlgsnap.hxx>
#include <layeroptionsdlg.hxx>
#include <inspagob.hxx>
#include <morphdlg.hxx>
#include <OutlineBulletDlg.hxx>
#include <paragr.hxx>
#include <present.hxx>
#include "RemoteDialog.hxx"
#include <prltempl.hxx>
#include <sdpreslt.hxx>
#include <tabtempl.hxx>
#include <tpaction.hxx>
#include <vectdlg.hxx>
#include <tpoption.hxx>
#include <prntopts.hxx>
#include <masterlayoutdlg.hxx>
#include <headerfooterdlg.hxx>
#include "PhotoAlbumDialog.hxx"

#include <vcl/abstdlgimpl.hxx>
#include <vcl/virdev.hxx>

namespace
{
template <class Base, class Dialog, template <class...> class ImplTemplate>
class ScreenshottedDialog_Impl_BASE : public ImplTemplate<Base, Dialog>
{
public:
    using ImplTemplate<Base, Dialog>::ImplTemplate;

    // screenshotting
    BitmapEx createScreenshot() const override
    {
        VclPtr<VirtualDevice> xDialogSurface(this->m_pDlg->getDialog()->screenshot());
        return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
    }
    OUString GetScreenshotId() const override { return this->m_pDlg->get_help_id(); }
};

template <class Base, class Dialog>
using ScreenshottedDialog_Impl_Sync
    = ScreenshottedDialog_Impl_BASE<Base, Dialog, vcl::AbstractDialogImpl_Sync>;

template <class Base, class Dialog>
using ScreenshottedDialog_Impl_Async
    = ScreenshottedDialog_Impl_BASE<Base, Dialog, vcl::AbstractDialogImpl_Async>;

template <class Dialog>
using SdAbstractGenericDialog_Impl = ScreenshottedDialog_Impl_Sync<VclAbstractDialog, Dialog>;
}

//-------------- SdAbstractDialogFactory implementation--------------

namespace
{
/// Provides managing and getting information from the numbering and position dialog.
class AbstractSvxBulletAndPositionDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractSvxBulletAndPositionDlg, SvxBulletAndPositionDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    const SfxItemSet* GetOutputItemSet(SfxItemSet* pSet) const override
    {
        return m_pDlg->GetOutputItemSet(pSet);
    }
    bool IsApplyToMaster() override { return m_pDlg->IsApplyToMaster(); }
    bool IsSlideScope() override { return m_pDlg->IsSlideScope(); }
};
}

VclPtr<AbstractSvxBulletAndPositionDlg> SdAbstractDialogFactory_Impl::CreateSvxBulletAndPositionDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView)
{
    return VclPtr<AbstractSvxBulletAndPositionDlg_Impl>::Create(pParent, *pAttr, pView);
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateBreakDlg(
                                            weld::Window* pParent,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            sal_uLong nSumActionCount,
                                            sal_uLong nObjCount )
{
    return VclPtr<SdAbstractGenericDialog_Impl<sd::BreakDlg>>::Create(pParent, pDrView, pShell, nSumActionCount, nObjCount);
}

namespace
{
class AbstractCopyDlg_Impl : public ScreenshottedDialog_Impl_Sync<AbstractCopyDlg, sd::CopyDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetAttr(SfxItemSet& rOutAttrs) override { m_pDlg->GetAttr(rOutAttrs); }
};
}

VclPtr<AbstractCopyDlg> SdAbstractDialogFactory_Impl::CreateCopyDlg(weld::Window* pParent,
                                            const SfxItemSet& rInAttrs,
                                            ::sd::View* pView )
{
    return VclPtr<AbstractCopyDlg_Impl>::Create(pParent, rInAttrs, pView);
}

namespace
{
class AbstractSdCustomShowDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdCustomShowDlg, SdCustomShowDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    bool IsCustomShow() const override { return m_pDlg->IsCustomShow(); }
};
}

VclPtr<AbstractSdCustomShowDlg> SdAbstractDialogFactory_Impl::CreateSdCustomShowDlg(weld::Window* pParent, SdDrawDocument& rDrawDoc )
{
    return VclPtr<AbstractSdCustomShowDlg_Impl>::Create(pParent, rDrawDoc);
}

namespace
{
template <class Dialog>
class SdAbstractTabController_Impl
    : public ScreenshottedDialog_Impl_Async<SfxAbstractTabDialog, Dialog>
{
public:
    using ScreenshottedDialog_Impl_Async<SfxAbstractTabDialog, Dialog>::ScreenshottedDialog_Impl_Async;
    void SetCurPageId(const OUString& rName) override { this->m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return this->m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& rPool) override
    {
        return this->m_pDlg->GetInputRanges(rPool);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { this->m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { this->m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdTabCharDialog(weld::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell)
{
    return VclPtr<SdAbstractTabController_Impl<SdCharDlg>>::Create(pParent, pAttr, pDocShell);
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdTabPageDialog(weld::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage, bool bIsImpressDoc)
{
    return VclPtr<SdAbstractTabController_Impl<SdPageDlg>>::Create(pDocShell, pParent, pAttr, bAreaPage, bIsImpressDoc);
}

namespace
{
class AbstractSdModifyFieldDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdModifyFieldDlg, SdModifyFieldDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    SvxFieldData* GetField() override { return m_pDlg->GetField(); }
    SfxItemSet GetItemSet() override { return m_pDlg->GetItemSet(); }
};
}

VclPtr<AbstractSdModifyFieldDlg> SdAbstractDialogFactory_Impl::CreateSdModifyFieldDlg(weld::Window* pParent, const SvxFieldData* pInField, const SfxItemSet& rSet)
{
    return VclPtr<AbstractSdModifyFieldDlg_Impl>::Create(pParent, pInField, rSet);
}

namespace
{
class AbstractSdSnapLineDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdSnapLineDlg, SdSnapLineDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetAttr(SfxItemSet& rOutAttrs) override { m_pDlg->GetAttr(rOutAttrs); }
    void HideRadioGroup() override { m_pDlg->HideRadioGroup(); }
    void HideDeleteBtn() override { m_pDlg->HideDeleteBtn(); }
    void SetInputFields(bool bEnableX, bool bEnableY) override
    {
        m_pDlg->SetInputFields(bEnableX, bEnableY);
    }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<AbstractSdSnapLineDlg> SdAbstractDialogFactory_Impl::CreateSdSnapLineDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView)
{
    return VclPtr<AbstractSdSnapLineDlg_Impl>::Create(pParent, rInAttrs, pView);
}

namespace
{
class AbstractSdInsertLayerDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdInsertLayerDlg, SdInsertLayerDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetAttr(SfxItemSet& rOutAttrs) override { m_pDlg->GetAttr(rOutAttrs); }
    void SetHelpId(const OUString& rHelpId) override { m_pDlg->set_help_id(rHelpId); }
};
}

VclPtr<AbstractSdInsertLayerDlg> SdAbstractDialogFactory_Impl::CreateSdInsertLayerDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr)
{
    return VclPtr<AbstractSdInsertLayerDlg_Impl>::Create(pParent, rInAttrs, bDeletable, aStr);
}

namespace
{
class AbstractSdInsertPagesObjsDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdInsertPagesObjsDlg, SdInsertPagesObjsDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    std::vector<OUString> GetList(const sal_uInt16 nType) override
    {
        return m_pDlg->GetList(nType);
    }
    bool IsLink() override { return m_pDlg->IsLink(); }
    bool IsRemoveUnnecessaryMasterPages() const override
    {
        return m_pDlg->IsRemoveUnnecessaryMasterPages();
    }
};
}

VclPtr<AbstractSdInsertPagesObjsDlg> SdAbstractDialogFactory_Impl::CreateSdInsertPagesObjsDlg(weld::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName)
{
    return VclPtr<AbstractSdInsertPagesObjsDlg_Impl>::Create(pParent, pDoc, pSfxMedium, rFileName);
}

namespace
{
class AbstractMorphDlg_Impl : public ScreenshottedDialog_Impl_Sync<AbstractMorphDlg, sd::MorphDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void SaveSettings() const override { m_pDlg->SaveSettings(); }
    sal_uInt16 GetFadeSteps() const override { return m_pDlg->GetFadeSteps(); }
    bool IsAttributeFade() const override { return m_pDlg->IsAttributeFade(); }
    bool IsOrientationFade() const override { return m_pDlg->IsOrientationFade(); }
};
}

VclPtr<AbstractMorphDlg> SdAbstractDialogFactory_Impl::CreateMorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2)
{
    return VclPtr<AbstractMorphDlg_Impl>::Create(pParent, pObj1, pObj2);
}

namespace
{
class AbstractBulletDialog_Impl
    : public ScreenshottedDialog_Impl_Async<SfxAbstractTabDialog, sd::OutlineBulletDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetBulletOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdOutlineBulletTabDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView)
{
    return VclPtr<AbstractBulletDialog_Impl>::Create(pParent, pAttr, pView);
}

VclPtr<SfxAbstractTabDialog> SdAbstractDialogFactory_Impl::CreateSdParagraphTabDlg(weld::Window* pParent, const SfxItemSet* pAttr )
{
    return VclPtr<SdAbstractTabController_Impl<SdParagraphDlg>>::Create(pParent, pAttr);
}

namespace
{
class AbstractSdStartPresDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdStartPresDlg, SdStartPresentationDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetAttr(SfxItemSet& rOutAttrs) override { m_pDlg->GetAttr(rOutAttrs); }
};
}

VclPtr<AbstractSdStartPresDlg> SdAbstractDialogFactory_Impl::CreateSdStartPresentationDlg(weld::Window* pParent,
        const SfxItemSet& rInAttrs, const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList)
{
    return VclPtr<AbstractSdStartPresDlg_Impl>::Create(pParent, rInAttrs, rPageNames, pCSList);
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateRemoteDialog(weld::Window* pParent)
{
    return VclPtr<SdAbstractGenericDialog_Impl<sd::RemoteDialog>>::Create(pParent);
}

namespace
{
class SdPresLayoutTemplateDlg_Impl
    : public ScreenshottedDialog_Impl_Async<SfxAbstractTabDialog, SdPresLayoutTemplateDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdPresLayoutTemplateDlg(SfxObjectShell* pDocSh, weld::Window* pParent, bool bBackgroundDlg, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool)
{
    return VclPtr<SdPresLayoutTemplateDlg_Impl>::Create(pDocSh, pParent, bBackgroundDlg, rStyleBase, ePO, pSSPool);
}

namespace
{
class AbstractSdPresLayoutDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdPresLayoutDlg, SdPresLayoutDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetAttr(SfxItemSet& rOutAttrs) override { m_pDlg->GetAttr(rOutAttrs); }
};
}

VclPtr<AbstractSdPresLayoutDlg> SdAbstractDialogFactory_Impl::CreateSdPresLayoutDlg(weld::Window* pParent, ::sd::DrawDocShell* pDocShell, const SfxItemSet& rInAttrs)
{
    return VclPtr<AbstractSdPresLayoutDlg_Impl>::Create(pDocShell, pParent, rInAttrs);
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdTabTemplateDlg(weld::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView)
{
    return VclPtr<SdAbstractTabController_Impl<SdTabTemplateDlg>>::Create(pParent, pDocShell, rStyleBase, pModel, pView);
}

namespace
{
class SdAbstractActionDialog_Impl
    : public ScreenshottedDialog_Impl_Async<SfxAbstractDialog, SdActionDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractDialog> SdAbstractDialogFactory_Impl::CreatSdActionDialog(weld::Window* pParent, const SfxItemSet& rAttr, ::sd::View* pView )
{
    return VclPtr<SdAbstractActionDialog_Impl>::Create(pParent, rAttr, pView);
}

namespace
{
class AbstractSdVectorizeDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractSdVectorizeDlg, SdVectorizeDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    const GDIMetaFile& GetGDIMetaFile() const override { return m_pDlg->GetGDIMetaFile(); }
};
}

VclPtr<AbstractSdVectorizeDlg>  SdAbstractDialogFactory_Impl::CreateSdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell)
{
    return VclPtr<AbstractSdVectorizeDlg_Impl>::Create(pParent, rBmp, pDocShell);
}

// Factories for TabPages
CreateTabPage SdAbstractDialogFactory_Impl::GetSdOptionsContentsTabPageCreatorFunc()
{
    return SdTpOptionsContents::Create;
}

CreateTabPage SdAbstractDialogFactory_Impl::GetSdPrintOptionsTabPageCreatorFunc()
{
    return SdPrintOptions::Create;
}

CreateTabPage SdAbstractDialogFactory_Impl::GetSdOptionsMiscTabPageCreatorFunc()
{
    return SdTpOptionsMisc::Create;
}

CreateTabPage SdAbstractDialogFactory_Impl::GetSdOptionsSnapTabPageCreatorFunc()
{
    return SdTpOptionsSnap::Create;
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateMasterLayoutDialog(weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage)
{
    return VclPtr<SdAbstractGenericDialog_Impl<sd::MasterLayoutDialog>>::Create(pParent, pDoc, pCurrentPage);
}

VclPtr<AbstractHeaderFooterDialog> SdAbstractDialogFactory_Impl::CreateHeaderFooterDialog(sd::ViewShell* pViewShell,
    weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage)
{
    using AbstractHeaderFooterDialog_Impl
        = ScreenshottedDialog_Impl_Async<AbstractHeaderFooterDialog, sd::HeaderFooterDialog>;
    return VclPtr<AbstractHeaderFooterDialog_Impl>::Create(pViewShell, pParent, pDoc, pCurrentPage);
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateSdPhotoAlbumDialog(weld::Window* pParent, SdDrawDocument* pDoc)
{
    return VclPtr<SdAbstractGenericDialog_Impl<sd::SdPhotoAlbumDialog>>::Create(pParent, pDoc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
