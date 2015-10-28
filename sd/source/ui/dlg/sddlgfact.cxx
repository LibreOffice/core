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

#include <sfx2/objsh.hxx>
#include "sddlgfact.hxx"
#include "strings.hrc"
#include "BreakDlg.hxx"
#include "copydlg.hxx"
#include "custsdlg.hxx"
#include "dlg_char.hxx"
#include "dlgpage.hxx"
#include "dlgass.hxx"
#include "dlgfield.hxx"
#include "dlgsnap.hxx"
#include "layeroptionsdlg.hxx"
#include "ins_paste.hxx"
#include "inspagob.hxx"
#include "morphdlg.hxx"
#include "OutlineBulletDlg.hxx"
#include "paragr.hxx"
#include "present.hxx"
#include "RemoteDialog.hxx"
#include "prltempl.hxx"
#include "sdpreslt.hxx"
#include "tabtempl.hxx"
#include "tpaction.hxx"
#include "vectdlg.hxx"
#include "tpoption.hxx"
#include "prntopts.hxx"
#include "pubdlg.hxx"
#include "masterlayoutdlg.hxx"
#include "headerfooterdlg.hxx"
#include "PhotoAlbumDialog.hxx"

IMPL_ABSTDLG_BASE(SdVclAbstractDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractCopyDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl);
IMPL_ABSTDLG_BASE(SdAbstractTabDialog_Impl);
IMPL_ABSTDLG_BASE(SdPresLayoutTemplateDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractAssistentDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMorphDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl);
IMPL_ABSTDLG_BASE(SdAbstractSfxDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractHeaderFooterDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractBulletDialog_Impl);

void AbstractCopyDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}

bool AbstractSdCustomShowDlg_Impl::IsModified() const
{
    return pDlg->IsModified();
}

bool AbstractSdCustomShowDlg_Impl::IsCustomShow() const
{
    return pDlg->IsCustomShow();
}

void SdAbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void SdAbstractTabDialog_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* SdAbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* SdAbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void SdAbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

//From class Window.
void SdAbstractTabDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

OUString SdAbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

void AbstractBulletDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->SetCurPageId( nId );
}

void AbstractBulletDialog_Impl::SetCurPageId( const OString& rName )
{
    static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->SetCurPageId( rName );
}

const SfxItemSet* AbstractBulletDialog_Impl::GetOutputItemSet() const
{
    return static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->GetOutputItemSet();
}

const sal_uInt16* AbstractBulletDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->GetInputRanges( pItem );
}

void AbstractBulletDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->SetInputSet( pInSet );
}

void AbstractBulletDialog_Impl::SetText( const OUString& rStr )
{
    static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->SetText( rStr );
}

OUString AbstractBulletDialog_Impl::GetText() const
{
    return static_cast< ::sd::OutlineBulletDlg*>(pDlg.get())->GetText();
}

void SdPresLayoutTemplateDlg_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void SdPresLayoutTemplateDlg_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* SdPresLayoutTemplateDlg_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* SdPresLayoutTemplateDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void SdPresLayoutTemplateDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

void SdPresLayoutTemplateDlg_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

OUString SdPresLayoutTemplateDlg_Impl::GetText() const
{
    return pDlg->GetText();
}

SfxObjectShellLock AbstractAssistentDlg_Impl::GetDocument()
{
    return pDlg->GetDocument();
}

OutputType AbstractAssistentDlg_Impl::GetOutputMedium() const
{
    return pDlg->GetOutputMedium();
}

bool AbstractAssistentDlg_Impl::IsSummary() const
{
    return pDlg->IsSummary();
}

StartType AbstractAssistentDlg_Impl::GetStartType() const
{
    return pDlg->GetStartType();
}

OUString AbstractAssistentDlg_Impl::GetDocPath() const
{
    return pDlg->GetDocPath();
}

bool AbstractAssistentDlg_Impl::GetStartWithFlag() const
{
    return pDlg->GetStartWithFlag();
}

bool AbstractAssistentDlg_Impl::IsDocEmpty() const
{
    return pDlg->IsDocEmpty();
}

css::uno::Sequence< css::beans::NamedValue > AbstractAssistentDlg_Impl::GetPassword()
{
    return pDlg->GetPassword();
}

SvxFieldData* AbstractSdModifyFieldDlg_Impl::GetField()
{
    return pDlg->GetField();
}

SfxItemSet AbstractSdModifyFieldDlg_Impl::GetItemSet()
{
    return pDlg->GetItemSet();
}

void AbstractSdSnapLineDlg_Impl::GetAttr(SfxItemSet& rOutAttrs)
{
    pDlg->GetAttr(rOutAttrs);
}

void AbstractSdSnapLineDlg_Impl::HideRadioGroup()
{
    pDlg->HideRadioGroup();
}

void AbstractSdSnapLineDlg_Impl::HideDeleteBtn()
{
    pDlg->HideDeleteBtn();
}

void AbstractSdSnapLineDlg_Impl::SetInputFields(bool bEnableX, bool bEnableY)
{
    pDlg->SetInputFields(bEnableX, bEnableY);
}

void AbstractSdSnapLineDlg_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

void AbstractSdInsertLayerDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}

void AbstractSdInsertLayerDlg_Impl::SetHelpId( const OString& rHelpId )
{
    pDlg->SetHelpId( rHelpId );
}

vcl::Window* AbstractSdInsertPagesObjsDlg_Impl::GetWindow()
{
    return static_cast<vcl::Window*>(pDlg);
}

std::vector<OUString> AbstractSdInsertPagesObjsDlg_Impl::GetList(const sal_uInt16 nType)
{
    return pDlg->GetList(nType);
}

bool AbstractSdInsertPagesObjsDlg_Impl::IsLink()
{
    return pDlg->IsLink();
}

bool AbstractSdInsertPagesObjsDlg_Impl::IsRemoveUnnessesaryMasterPages() const
{
    return pDlg->IsRemoveUnnessesaryMasterPages();
}

void AbstractMorphDlg_Impl::SaveSettings() const
{
    pDlg->SaveSettings();
}

sal_uInt16 AbstractMorphDlg_Impl::GetFadeSteps() const
{
    return pDlg->GetFadeSteps();
}

bool AbstractMorphDlg_Impl::IsAttributeFade() const
{
    return pDlg->IsAttributeFade();
}

bool AbstractMorphDlg_Impl::IsOrientationFade() const
{
    return pDlg->IsOrientationFade();
}

void AbstractSdStartPresDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}

void AbstractSdPresLayoutDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}

const SfxItemSet* SdAbstractSfxDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void SdAbstractSfxDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

OUString SdAbstractSfxDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

const GDIMetaFile& AbstractSdVectorizeDlg_Impl::GetGDIMetaFile() const
{
    return pDlg->GetGDIMetaFile();
}

void AbstractSdPublishingDlg_Impl::GetParameterSequence( css::uno::Sequence< css::beans::PropertyValue >& rParams )
{
    pDlg->GetParameterSequence( rParams );
}

//-------------- SdAbstractDialogFactory implementation--------------

VclAbstractDialog * SdAbstractDialogFactory_Impl::CreateBreakDlg(
                                            vcl::Window* pWindow,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            sal_uLong nSumActionCount,
                                            sal_uLong nObjCount )
{
    return new SdVclAbstractDialog_Impl( VclPtr< ::sd::BreakDlg>::Create( pWindow, pDrView, pShell, nSumActionCount, nObjCount ) );
}

AbstractCopyDlg * SdAbstractDialogFactory_Impl::CreateCopyDlg(
                                            vcl::Window* pWindow, const SfxItemSet& rInAttrs,
                                            const rtl::Reference<XColorList> &pColTab,
                                            ::sd::View* pView )
{
    return new AbstractCopyDlg_Impl( VclPtr< ::sd::CopyDlg>::Create( pWindow, rInAttrs, pColTab, pView ) );
}

AbstractSdCustomShowDlg * SdAbstractDialogFactory_Impl::CreateSdCustomShowDlg( vcl::Window* pWindow, SdDrawDocument& rDrawDoc )
{
    return new AbstractSdCustomShowDlg_Impl( VclPtr<SdCustomShowDlg>::Create( pWindow, rDrawDoc ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabCharDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell )
{
    return new SdAbstractTabDialog_Impl( VclPtr<SdCharDlg>::Create( pParent, pAttr, pDocShell ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabPageDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage )
{
    return new SdAbstractTabDialog_Impl( VclPtr<SdPageDlg>::Create( pDocShell, pParent, pAttr, bAreaPage ) );
}

AbstractAssistentDlg * SdAbstractDialogFactory_Impl::CreateAssistentDlg( vcl::Window* pParent, bool bAutoPilot)
{
    return new AbstractAssistentDlg_Impl( VclPtr<AssistentDlg>::Create( pParent, bAutoPilot ) );
}

AbstractSdModifyFieldDlg * SdAbstractDialogFactory_Impl::CreateSdModifyFieldDlg( vcl::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet )
{
    return new AbstractSdModifyFieldDlg_Impl( VclPtr<SdModifyFieldDlg>::Create( pWindow, pInField, rSet ) );
}

AbstractSdSnapLineDlg * SdAbstractDialogFactory_Impl::CreateSdSnapLineDlg( vcl::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView)
{
    return new AbstractSdSnapLineDlg_Impl( VclPtr<SdSnapLineDlg>::Create( pWindow, rInAttrs, pView ) );
}

AbstractSdInsertLayerDlg * SdAbstractDialogFactory_Impl::CreateSdInsertLayerDlg( vcl::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr )
{
    return new AbstractSdInsertLayerDlg_Impl( VclPtr<SdInsertLayerDlg>::Create( pWindow, rInAttrs, bDeletable, aStr ) );
}

AbstractSdInsertPagesObjsDlg * SdAbstractDialogFactory_Impl::CreateSdInsertPagesObjsDlg( vcl::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName )
{
    return new AbstractSdInsertPagesObjsDlg_Impl( VclPtr<SdInsertPagesObjsDlg>::Create( pParent, pDoc, pSfxMedium, rFileName ) );
}

AbstractMorphDlg * SdAbstractDialogFactory_Impl::CreateMorphDlg( vcl::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2)
{
    return new AbstractMorphDlg_Impl( VclPtr< ::sd::MorphDlg>::Create( pParent, pObj1, pObj2 ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdOutlineBulletTabDlg( vcl::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView )
{
    return new AbstractBulletDialog_Impl( VclPtr< ::sd::OutlineBulletDlg>::Create( pParent, pAttr, pView ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdParagraphTabDlg( vcl::Window* pParent, const SfxItemSet* pAttr )
{
    return new SdAbstractTabDialog_Impl( VclPtr<SdParagraphDlg>::Create( pParent, pAttr ) );
}

AbstractSdStartPresDlg *  SdAbstractDialogFactory_Impl::CreateSdStartPresentationDlg( vcl::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList )
{
    return new AbstractSdStartPresDlg_Impl( VclPtr<SdStartPresentationDlg>::Create( pWindow, rInAttrs, rPageNames, pCSList ) );
}

VclAbstractDialog * SdAbstractDialogFactory_Impl::CreateRemoteDialog( vcl::Window* pWindow )
{
     return new SdVclAbstractDialog_Impl( VclPtr< ::sd::RemoteDialog>::Create( pWindow ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, vcl::Window* pParent, const SdResId& DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool )
{
    return new SdPresLayoutTemplateDlg_Impl( VclPtr<SdPresLayoutTemplateDlg>::Create( pDocSh, pParent, DlgId, rStyleBase, ePO, pSSPool ) );
}

AbstractSdPresLayoutDlg *  SdAbstractDialogFactory_Impl::CreateSdPresLayoutDlg(  ::sd::DrawDocShell* pDocShell, vcl::Window* pWindow, const SfxItemSet& rInAttrs)
{
    return new AbstractSdPresLayoutDlg_Impl( VclPtr<SdPresLayoutDlg>::Create( pDocShell, pWindow, rInAttrs ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabTemplateDlg( vcl::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView )
{
    return new SdAbstractTabDialog_Impl( VclPtr<SdTabTemplateDlg>::Create( pParent, pDocShell, rStyleBase, pModel, pView ) );
}

SfxAbstractDialog* SdAbstractDialogFactory_Impl::CreatSdActionDialog( vcl::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView )
{
    return new SdAbstractSfxDialog_Impl( VclPtr<SdActionDlg>::Create( pParent, pAttr, pView ) );
}

AbstractSdVectorizeDlg *  SdAbstractDialogFactory_Impl::CreateSdVectorizeDlg( vcl::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell )
{
    return new AbstractSdVectorizeDlg_Impl( VclPtr<SdVectorizeDlg>::Create( pParent, rBmp, pDocShell ) );
}

AbstractSdPublishingDlg *  SdAbstractDialogFactory_Impl::CreateSdPublishingDlg( vcl::Window* pWindow, DocumentType eDocType)
{
    return new AbstractSdPublishingDlg_Impl( VclPtr<SdPublishingDlg>::Create( pWindow, eDocType ) );
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

VclAbstractDialog* SdAbstractDialogFactory_Impl::CreateMasterLayoutDialog( vcl::Window* pParent,
    SdDrawDocument* pDoc, SdPage* pCurrentPage )
{
  return new SdVclAbstractDialog_Impl( VclPtr< ::sd::MasterLayoutDialog>::Create( pParent, pDoc, pCurrentPage ));
}

AbstractHeaderFooterDialog* SdAbstractDialogFactory_Impl::CreateHeaderFooterDialog( sd::ViewShell* pViewShell,
  vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage )
{
    return new AbstractHeaderFooterDialog_Impl( VclPtr< ::sd::HeaderFooterDialog>::Create( pViewShell, pParent, pDoc, pCurrentPage ));
}

VclAbstractDialog * SdAbstractDialogFactory_Impl::CreateSdPhotoAlbumDialog( vcl::Window* pWindow, SdDrawDocument* pDoc )
{
     return new SdVclAbstractDialog_Impl( VclPtr< ::sd::SdPhotoAlbumDialog>::Create( pWindow, pDoc ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
