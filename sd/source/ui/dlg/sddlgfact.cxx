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
#include <pubdlg.hxx>
#include <masterlayoutdlg.hxx>
#include <headerfooterdlg.hxx>
#include "PhotoAlbumDialog.hxx"
#include <vcl/virdev.hxx>

short AbstractSvxBulletAndPositionDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short SdAbstractGenericDialog_Impl::Execute()
{
    return m_xDlg->run();
}

BitmapEx SdAbstractGenericDialog_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString SdAbstractGenericDialog_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

const SfxItemSet* AbstractSvxBulletAndPositionDlg_Impl::GetOutputItemSet( SfxItemSet* pSet ) const
{
    return m_xDlg->GetOutputItemSet( pSet );
}

bool AbstractSvxBulletAndPositionDlg_Impl::IsApplyToMaster()
{
    return m_xDlg->IsApplyToMaster();
}

bool AbstractSvxBulletAndPositionDlg_Impl::IsSlideScope()
{
    return m_xDlg->IsSlideScope();
}

short AbstractCopyDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdCustomShowDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short SdPresLayoutTemplateDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool SdPresLayoutTemplateDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSdModifyFieldDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdSnapLineDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdInsertLayerDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdInsertPagesObjsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMorphDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdStartPresDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdPresLayoutDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short SdAbstractSfxDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdVectorizeDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSdPublishingDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractHeaderFooterDialog_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractHeaderFooterDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::DialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

BitmapEx AbstractHeaderFooterDialog_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractHeaderFooterDialog_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

short AbstractBulletDialog_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractBulletDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

AbstractBreakDlg_Impl::AbstractBreakDlg_Impl(std::unique_ptr<::sd::BreakDlg> pDlg)
    : m_xDlg(std::move(pDlg))
{
}

short AbstractBreakDlg_Impl::Execute()
{
    return m_xDlg->run();
}

BitmapEx AbstractBreakDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractBreakDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

AbstractMasterLayoutDialog_Impl::AbstractMasterLayoutDialog_Impl(std::unique_ptr<::sd::MasterLayoutDialog> pDlg)
    : m_xDlg(std::move(pDlg))
{
}

short AbstractMasterLayoutDialog_Impl::Execute()
{
    return m_xDlg->run();
}

BitmapEx AbstractMasterLayoutDialog_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractMasterLayoutDialog_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractCopyDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    m_xDlg->GetAttr( rOutAttrs );
}

BitmapEx AbstractCopyDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractCopyDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

bool AbstractSdCustomShowDlg_Impl::IsCustomShow() const
{
    return m_xDlg->IsCustomShow();
}

BitmapEx AbstractSdCustomShowDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdCustomShowDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

short SdAbstractTabController_Impl::Execute()
{
    return m_xDlg->run();
}

void SdAbstractTabController_Impl::SetCurPageId( const OString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* SdAbstractTabController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer SdAbstractTabController_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void SdAbstractTabController_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

bool SdAbstractTabController_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

//From class Window.
void SdAbstractTabController_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

BitmapEx SdAbstractTabController_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString SdAbstractTabController_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractBulletDialog_Impl::SetCurPageId( const OString& rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractBulletDialog_Impl::GetOutputItemSet() const
{
    return static_cast< ::sd::OutlineBulletDlg*>(m_xDlg.get())->GetBulletOutputItemSet();
}

WhichRangesContainer AbstractBulletDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges(pItem);
}

void AbstractBulletDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet(pInSet);
}

void AbstractBulletDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

BitmapEx AbstractBulletDialog_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractBulletDialog_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void SdPresLayoutTemplateDlg_Impl::SetCurPageId( const OString& rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* SdPresLayoutTemplateDlg_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer SdPresLayoutTemplateDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void SdPresLayoutTemplateDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

void SdPresLayoutTemplateDlg_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

BitmapEx SdPresLayoutTemplateDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString SdPresLayoutTemplateDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

SvxFieldData* AbstractSdModifyFieldDlg_Impl::GetField()
{
    return m_xDlg->GetField();
}

SfxItemSet AbstractSdModifyFieldDlg_Impl::GetItemSet()
{
    return m_xDlg->GetItemSet();
}

BitmapEx AbstractSdModifyFieldDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdModifyFieldDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractSdSnapLineDlg_Impl::GetAttr(SfxItemSet& rOutAttrs)
{
    m_xDlg->GetAttr(rOutAttrs);
}

void AbstractSdSnapLineDlg_Impl::HideRadioGroup()
{
    m_xDlg->HideRadioGroup();
}

void AbstractSdSnapLineDlg_Impl::HideDeleteBtn()
{
    m_xDlg->HideDeleteBtn();
}

void AbstractSdSnapLineDlg_Impl::SetInputFields(bool bEnableX, bool bEnableY)
{
    m_xDlg->SetInputFields(bEnableX, bEnableY);
}

void AbstractSdSnapLineDlg_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

BitmapEx AbstractSdSnapLineDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdSnapLineDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractSdInsertLayerDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    m_xDlg->GetAttr(rOutAttrs);
}

void AbstractSdInsertLayerDlg_Impl::SetHelpId( const OString& rHelpId )
{
    m_xDlg->set_help_id(rHelpId);
}

BitmapEx AbstractSdInsertLayerDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdInsertLayerDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

std::vector<OUString> AbstractSdInsertPagesObjsDlg_Impl::GetList(const sal_uInt16 nType)
{
    return m_xDlg->GetList(nType);
}

bool AbstractSdInsertPagesObjsDlg_Impl::IsLink()
{
    return m_xDlg->IsLink();
}

bool AbstractSdInsertPagesObjsDlg_Impl::IsRemoveUnnecessaryMasterPages() const
{
    return m_xDlg->IsRemoveUnnecessaryMasterPages();
}

BitmapEx AbstractSdInsertPagesObjsDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdInsertPagesObjsDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractMorphDlg_Impl::SaveSettings() const
{
    m_xDlg->SaveSettings();
}

sal_uInt16 AbstractMorphDlg_Impl::GetFadeSteps() const
{
    return m_xDlg->GetFadeSteps();
}

bool AbstractMorphDlg_Impl::IsAttributeFade() const
{
    return m_xDlg->IsAttributeFade();
}

bool AbstractMorphDlg_Impl::IsOrientationFade() const
{
    return m_xDlg->IsOrientationFade();
}

BitmapEx AbstractMorphDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractMorphDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractSdStartPresDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    m_xDlg->GetAttr(rOutAttrs);
}

BitmapEx AbstractSdStartPresDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdStartPresDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractSdPresLayoutDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    m_xDlg->GetAttr(rOutAttrs);
}

BitmapEx AbstractSdPresLayoutDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdPresLayoutDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

const SfxItemSet* SdAbstractSfxDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void SdAbstractSfxDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

const GDIMetaFile& AbstractSdVectorizeDlg_Impl::GetGDIMetaFile() const
{
    return m_xDlg->GetGDIMetaFile();
}

BitmapEx AbstractSdVectorizeDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdVectorizeDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

void AbstractSdPublishingDlg_Impl::GetParameterSequence( css::uno::Sequence< css::beans::PropertyValue >& rParams )
{
    m_xDlg->GetParameterSequence( rParams );
}

BitmapEx AbstractSdPublishingDlg_Impl::createScreenshot() const
{
    VclPtr<VirtualDevice> xDialogSurface(m_xDlg->getDialog()->screenshot());
    return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
}

OString AbstractSdPublishingDlg_Impl::GetScreenshotId() const
{
    return m_xDlg->get_help_id();
}

//-------------- SdAbstractDialogFactory implementation--------------

VclPtr<AbstractSvxBulletAndPositionDlg> SdAbstractDialogFactory_Impl::CreateSvxBulletAndPositionDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView)
{
    return VclPtr<AbstractSvxBulletAndPositionDlg_Impl>::Create(std::make_unique<SvxBulletAndPositionDlg>(pParent, *pAttr, pView));
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateBreakDlg(
                                            weld::Window* pParent,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            sal_uLong nSumActionCount,
                                            sal_uLong nObjCount )
{
    return VclPtr<AbstractBreakDlg_Impl>::Create(std::make_unique<::sd::BreakDlg>(pParent, pDrView, pShell, nSumActionCount, nObjCount));
}

VclPtr<AbstractCopyDlg> SdAbstractDialogFactory_Impl::CreateCopyDlg(weld::Window* pParent,
                                            const SfxItemSet& rInAttrs,
                                            ::sd::View* pView )
{
    return VclPtr<AbstractCopyDlg_Impl>::Create(std::make_unique<::sd::CopyDlg>(pParent, rInAttrs, pView));
}

VclPtr<AbstractSdCustomShowDlg> SdAbstractDialogFactory_Impl::CreateSdCustomShowDlg(weld::Window* pParent, SdDrawDocument& rDrawDoc )
{
    return VclPtr<AbstractSdCustomShowDlg_Impl>::Create(std::make_unique<SdCustomShowDlg>(pParent, rDrawDoc));
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdTabCharDialog(weld::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell)
{
    return VclPtr<SdAbstractTabController_Impl>::Create(std::make_shared<SdCharDlg>(pParent, pAttr, pDocShell));
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdTabPageDialog(weld::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage, bool bIsImpressDoc, bool bIsImpressMaster )
{
    return VclPtr<SdAbstractTabController_Impl>::Create(std::make_shared<SdPageDlg>(pDocShell, pParent, pAttr, bAreaPage, bIsImpressDoc, bIsImpressMaster));
}

VclPtr<AbstractSdModifyFieldDlg> SdAbstractDialogFactory_Impl::CreateSdModifyFieldDlg(weld::Window* pParent, const SvxFieldData* pInField, const SfxItemSet& rSet)
{
    return VclPtr<AbstractSdModifyFieldDlg_Impl>::Create(std::make_unique<SdModifyFieldDlg>(pParent, pInField, rSet));
}

VclPtr<AbstractSdSnapLineDlg> SdAbstractDialogFactory_Impl::CreateSdSnapLineDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView)
{
    return VclPtr<AbstractSdSnapLineDlg_Impl>::Create(std::make_unique<SdSnapLineDlg>(pParent, rInAttrs, pView));
}

VclPtr<AbstractSdInsertLayerDlg> SdAbstractDialogFactory_Impl::CreateSdInsertLayerDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr)
{
    return VclPtr<AbstractSdInsertLayerDlg_Impl>::Create(std::make_unique<SdInsertLayerDlg>(pParent, rInAttrs, bDeletable, aStr));
}

VclPtr<AbstractSdInsertPagesObjsDlg> SdAbstractDialogFactory_Impl::CreateSdInsertPagesObjsDlg(weld::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName)
{
    return VclPtr<AbstractSdInsertPagesObjsDlg_Impl>::Create(std::make_unique<SdInsertPagesObjsDlg>(pParent, pDoc, pSfxMedium, rFileName));
}

VclPtr<AbstractMorphDlg> SdAbstractDialogFactory_Impl::CreateMorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2)
{
    return VclPtr<AbstractMorphDlg_Impl>::Create(std::make_unique<::sd::MorphDlg>(pParent, pObj1, pObj2));
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdOutlineBulletTabDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView)
{
    return VclPtr<AbstractBulletDialog_Impl>::Create(std::make_shared<::sd::OutlineBulletDlg>(pParent, pAttr, pView));
}

VclPtr<SfxAbstractTabDialog> SdAbstractDialogFactory_Impl::CreateSdParagraphTabDlg(weld::Window* pParent, const SfxItemSet* pAttr )
{
    return VclPtr<SdAbstractTabController_Impl>::Create(std::make_shared<SdParagraphDlg>(pParent, pAttr));
}

VclPtr<AbstractSdStartPresDlg> SdAbstractDialogFactory_Impl::CreateSdStartPresentationDlg(weld::Window* pParent,
        const SfxItemSet& rInAttrs, const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList)
{
    return VclPtr<AbstractSdStartPresDlg_Impl>::Create(std::make_unique<SdStartPresentationDlg>(pParent, rInAttrs, rPageNames, pCSList));
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateRemoteDialog(weld::Window* pParent)
{
     return VclPtr<SdAbstractGenericDialog_Impl>::Create(std::make_unique<::sd::RemoteDialog>(pParent));
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdPresLayoutTemplateDlg(SfxObjectShell* pDocSh, weld::Window* pParent, bool bBackgroundDlg, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool)
{
    return VclPtr<SdPresLayoutTemplateDlg_Impl>::Create(std::make_shared<SdPresLayoutTemplateDlg>(pDocSh, pParent, bBackgroundDlg, rStyleBase, ePO, pSSPool));
}

VclPtr<AbstractSdPresLayoutDlg> SdAbstractDialogFactory_Impl::CreateSdPresLayoutDlg(weld::Window* pParent, ::sd::DrawDocShell* pDocShell, const SfxItemSet& rInAttrs)
{
    return VclPtr<AbstractSdPresLayoutDlg_Impl>::Create(std::make_unique<SdPresLayoutDlg>(pDocShell, pParent, rInAttrs));
}

VclPtr<SfxAbstractTabDialog>  SdAbstractDialogFactory_Impl::CreateSdTabTemplateDlg(weld::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView)
{
    return VclPtr<SdAbstractTabController_Impl>::Create(std::make_shared<SdTabTemplateDlg>(pParent, pDocShell, rStyleBase, pModel, pView));
}

VclPtr<SfxAbstractDialog> SdAbstractDialogFactory_Impl::CreatSdActionDialog(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView )
{
    return VclPtr<SdAbstractSfxDialog_Impl>::Create(std::make_unique<SdActionDlg>(pParent, pAttr, pView));
}

VclPtr<AbstractSdVectorizeDlg>  SdAbstractDialogFactory_Impl::CreateSdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell)
{
    return VclPtr<AbstractSdVectorizeDlg_Impl>::Create(std::make_unique<SdVectorizeDlg>(pParent, rBmp, pDocShell));
}

VclPtr<AbstractSdPublishingDlg>  SdAbstractDialogFactory_Impl::CreateSdPublishingDlg(weld::Window* pParent, DocumentType eDocType)
{
    return VclPtr<AbstractSdPublishingDlg_Impl>::Create(std::make_unique<SdPublishingDlg>(pParent, eDocType));
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
  return VclPtr<AbstractMasterLayoutDialog_Impl>::Create(std::make_unique<::sd::MasterLayoutDialog>(pParent, pDoc, pCurrentPage));
}

VclPtr<AbstractHeaderFooterDialog> SdAbstractDialogFactory_Impl::CreateHeaderFooterDialog(sd::ViewShell* pViewShell,
    weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage)
{
    return VclPtr<AbstractHeaderFooterDialog_Impl>::Create(std::make_shared<::sd::HeaderFooterDialog>(pViewShell, pParent, pDoc, pCurrentPage));
}

VclPtr<VclAbstractDialog> SdAbstractDialogFactory_Impl::CreateSdPhotoAlbumDialog(weld::Window* pParent, SdDrawDocument* pDoc)
{
    return VclPtr<SdAbstractGenericDialog_Impl>::Create(std::make_unique<sd::SdPhotoAlbumDialog>(pParent, pDoc));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
