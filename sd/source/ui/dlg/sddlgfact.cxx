/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sddlgfact.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:37:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <sfx2/objsh.hxx>
#include <tools/rc.hxx>
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
#include "printdlg.hxx"
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

IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractCopyDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl);
IMPL_ABSTDLG_BASE(SdPresLayoutTemplateDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractAssistentDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMorphDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdPrintDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractHeaderFooterDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractBulletDialog_Impl);

//AbstractCopyDlg_Impl begin
void AbstractCopyDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}
// AbstractCopyDlg_Impl end

//AbstractSdCustomShowDlg_Impl begin
BOOL AbstractSdCustomShowDlg_Impl::IsModified() const
{
    return pDlg->IsModified();
}
BOOL AbstractSdCustomShowDlg_Impl::IsCustomShow() const
{
    return pDlg->IsCustomShow();
}
// AbstractSdCustomShowDlg_Impl end

// AbstractTabDialog_Impl begin
void AbstractTabDialog_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}
const SfxItemSet* AbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
const USHORT* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
void AbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}
//add for AbstractTabDialog_Impl end

// --------------------------------------------------------------------

// AbstractBulletDialog_Impl begin
void AbstractBulletDialog_Impl::SetCurPageId( USHORT nId )
{
    static_cast< ::sd::OutlineBulletDlg*>(pDlg)->SetCurPageId( nId );
}
const SfxItemSet* AbstractBulletDialog_Impl::GetOutputItemSet() const
{
    return static_cast< ::sd::OutlineBulletDlg*>(pDlg)->GetOutputItemSet();
}
const USHORT* AbstractBulletDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return static_cast< ::sd::OutlineBulletDlg*>(pDlg)->GetInputRanges( pItem );
}
void AbstractBulletDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     static_cast< ::sd::OutlineBulletDlg*>(pDlg)->SetInputSet( pInSet );
}
//From class Window.
void AbstractBulletDialog_Impl::SetText( const XubString& rStr )
{
    static_cast< ::sd::OutlineBulletDlg*>(pDlg)->SetText( rStr );
}
String AbstractBulletDialog_Impl::GetText() const
{
    return static_cast< ::sd::OutlineBulletDlg*>(pDlg)->GetText();
}
//add for AbstractBulletDialog_Impl end

// --------------------------------------------------------------------

void SdPresLayoutTemplateDlg_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* SdPresLayoutTemplateDlg_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const USHORT* SdPresLayoutTemplateDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void SdPresLayoutTemplateDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

void SdPresLayoutTemplateDlg_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}

String SdPresLayoutTemplateDlg_Impl::GetText() const
{
    return pDlg->GetText();
}

// --------------------------------------------------------------------

//AbstractAssistentDlg_Impl begin
SfxObjectShellLock AbstractAssistentDlg_Impl::GetDocument()
{
    return pDlg->GetDocument();
}
OutputType AbstractAssistentDlg_Impl::GetOutputMedium() const
{
    return pDlg->GetOutputMedium();
}
BOOL AbstractAssistentDlg_Impl::IsSummary() const
{
    return pDlg->IsSummary();
}
StartType AbstractAssistentDlg_Impl::GetStartType() const
{
    return pDlg->GetStartType();
}
String AbstractAssistentDlg_Impl::GetDocPath() const
{
    return pDlg->GetDocPath();
}
BOOL AbstractAssistentDlg_Impl::GetStartWithFlag() const
{
    return pDlg->GetStartWithFlag();
}
BOOL AbstractAssistentDlg_Impl::IsDocEmpty() const
{
    return pDlg->IsDocEmpty();
}
String AbstractAssistentDlg_Impl::GetPassword()
{
    return pDlg->GetPassword();
}
// AbstractAssistentDlg_Impl end

//AbstractSdModifyFieldDlg_Impl begin
SvxFieldData* AbstractSdModifyFieldDlg_Impl::GetField()
{
    return pDlg->GetField();
}
SfxItemSet AbstractSdModifyFieldDlg_Impl::GetItemSet()
{
    return pDlg->GetItemSet();
}
// AbstractSdModifyFieldDlg_Impl end

//AbstractSdSnapLineDlg_Impl begin
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
void AbstractSdSnapLineDlg_Impl::SetInputFields(BOOL bEnableX, BOOL bEnableY)
{
    pDlg->SetInputFields(bEnableX, bEnableY);
}
void AbstractSdSnapLineDlg_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
// AbstractSdSnapLineDlg_Impl end

//AbstractSdInsertLayerDlg_Impl begin
void AbstractSdInsertLayerDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}
void AbstractSdInsertLayerDlg_Impl::SetHelpId( ULONG nHelpId )
{
    pDlg->SetHelpId( nHelpId );
}
// AbstractSdInsertLayerDlg_Impl end

//AbstractSdInsertPasteDlg_Impl begin
BOOL AbstractSdInsertPasteDlg_Impl::IsInsertBefore() const
{
    return pDlg->IsInsertBefore();
}
// AbstractSdInsertPasteDlg_Impl end

//AbstractSdInsertPagesObjsDlg_Impl begin
::Window* AbstractSdInsertPagesObjsDlg_Impl::GetWindow()
{
    return (::Window*)pDlg;
}
List* AbstractSdInsertPagesObjsDlg_Impl::GetList( USHORT nType )
{
    return pDlg->GetList( nType );
}
BOOL AbstractSdInsertPagesObjsDlg_Impl::IsLink()
{
    return pDlg->IsLink();
}
BOOL AbstractSdInsertPagesObjsDlg_Impl::IsRemoveUnnessesaryMasterPages() const
{
    return pDlg->IsRemoveUnnessesaryMasterPages();
}
// AbstractSdInsertPagesObjsDlg_Impl end

//AbstractMorphDlg_Impl begin
void AbstractMorphDlg_Impl::SaveSettings() const
{
    pDlg->SaveSettings();
}
USHORT AbstractMorphDlg_Impl::GetFadeSteps() const
{
    return pDlg->GetFadeSteps();
}
BOOL AbstractMorphDlg_Impl::IsAttributeFade() const
{
    return pDlg->IsAttributeFade();
}
BOOL AbstractMorphDlg_Impl::IsOrientationFade() const
{
    return pDlg->IsOrientationFade();
}
// AbstractMorphDlg_Impl end

//AbstractSdStartPresDlg_Impl begin
void AbstractSdStartPresDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}
// AbstractSdStartPresDlg_Impl end

//AbstractSdPrintDlg_Impl begin
USHORT AbstractSdPrintDlg_Impl::GetAttr()
{
    return pDlg->GetAttr();
}
// AbstractSdPrintDlg_Impl end

//AbstractSdPresLayoutDlg_Impl begin
void AbstractSdPresLayoutDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}
// AbstractSdPresLayoutDlg_Impl end

//AbstractSfxSingleTabDialog_Impl begin
const SfxItemSet* AbstractSfxSingleTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//AbstractSfxSingleTabDialog_Impl end

//AbstractSdVectorizeDlg_Impl begin
const GDIMetaFile& AbstractSdVectorizeDlg_Impl::GetGDIMetaFile() const
{
    return pDlg->GetGDIMetaFile();
}
//AbstractSdVectorizeDlg_Impl end

//AbstractSdPublishingDlg_Impl begin
void AbstractSdPublishingDlg_Impl::GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams )
{
    pDlg->GetParameterSequence( rParams );
}
//AbstractSdPublishingDlg_Impl end

//AbstractHeaderFooterDialog_Impl
void AbstractHeaderFooterDialog_Impl::ApplyToAll( TabPage* pPage )
{
  pDlg->ApplyToAll( pPage );
}

void AbstractHeaderFooterDialog_Impl::Apply( TabPage* pPage )
{
  pDlg->Apply( pPage );
}

void AbstractHeaderFooterDialog_Impl::Cancel( TabPage* pPage )
{
  pDlg->Cancel( pPage );
}
//AbstractHeaderFooterDialog_Impl

//-------------- SdAbstractDialogFactory implementation--------------

//add for BreakDlg begin
VclAbstractDialog * SdAbstractDialogFactory_Impl::CreateBreakDlg(
                                            ::Window* pWindow,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            ULONG nSumActionCount,
                                            ULONG nObjCount ) //add for BreakDlg
{
    return new VclAbstractDialog_Impl( new ::sd::BreakDlg( pWindow, pDrView, pShell, nSumActionCount, nObjCount ) );
}
//add for BreakDlg end

//add for CopyDlg begin
AbstractCopyDlg * SdAbstractDialogFactory_Impl::CreateCopyDlg(
                                            ::Window* pWindow, const SfxItemSet& rInAttrs,
                                            XColorTable* pColTab, ::sd::View* pView ) //add for CopyDlg
{
    return new AbstractCopyDlg_Impl( new ::sd::CopyDlg( pWindow, rInAttrs, pColTab, pView ) );
}
//add for CopyDlg end

//add for SdCustomShowDlg begin
AbstractSdCustomShowDlg * SdAbstractDialogFactory_Impl::CreateSdCustomShowDlg( ::Window* pWindow, SdDrawDocument& rDrawDoc ) //add for SdCustomShowDlg
{
    return new AbstractSdCustomShowDlg_Impl( new SdCustomShowDlg( pWindow, rDrawDoc ) );
}
//add for SdCustomShowDlg end

// add for SdCharDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabCharDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell )
{
    return new AbstractTabDialog_Impl( new SdCharDlg( pParent, pAttr, pDocShell ) );
}


SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabPageDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, BOOL bAreaPage )
{
    return new AbstractTabDialog_Impl( new SdPageDlg( pDocShell, pParent, pAttr, bAreaPage ) );
}
// add for SdCharDlg end

//add for AssistentDlg begin
AbstractAssistentDlg * SdAbstractDialogFactory_Impl::CreateAssistentDlg( ::Window* pParent, BOOL bAutoPilot)
{
    return new AbstractAssistentDlg_Impl( new AssistentDlg( pParent, bAutoPilot ) );
}
//add for AssistentDlg end

//add for SdModifyFieldDlg begin
AbstractSdModifyFieldDlg * SdAbstractDialogFactory_Impl::CreateSdModifyFieldDlg( ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet )
{
    return new AbstractSdModifyFieldDlg_Impl( new SdModifyFieldDlg( pWindow, pInField, rSet ) );
}
//add for SdModifyFieldDlg end

//add for SdSnapLineDlg begin
AbstractSdSnapLineDlg * SdAbstractDialogFactory_Impl::CreateSdSnapLineDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView)
{
    return new AbstractSdSnapLineDlg_Impl( new SdSnapLineDlg( pWindow, rInAttrs, pView ) );
}
//add for SdSnapLineDlg end

//add for SdInsertLayerDlg begin
AbstractSdInsertLayerDlg * SdAbstractDialogFactory_Impl::CreateSdInsertLayerDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr ) //add for SdInsertLayerDlg
{
    return new AbstractSdInsertLayerDlg_Impl( new SdInsertLayerDlg( pWindow, rInAttrs, bDeletable, aStr ) );
}
//add for SdInsertLayerDlg end

//add for SdInsertPasteDlg begin
AbstractSdInsertPasteDlg * SdAbstractDialogFactory_Impl::CreateSdInsertPasteDlg( ::Window* pWindow )
{
    return new AbstractSdInsertPasteDlg_Impl( new SdInsertPasteDlg( pWindow ) );
}
//add for SdInsertPasteDlg end

//add for SdInsertPagesObjsDlg begin
AbstractSdInsertPagesObjsDlg * SdAbstractDialogFactory_Impl::CreateSdInsertPagesObjsDlg( ::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const String& rFileName )
{
    return new AbstractSdInsertPagesObjsDlg_Impl( new SdInsertPagesObjsDlg( pParent, pDoc, pSfxMedium, rFileName ) );
}
//add for SdInsertPagesObjsDlg end

//add for MorphDlg begin
AbstractMorphDlg * SdAbstractDialogFactory_Impl::CreateMorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2)
{
    return new AbstractMorphDlg_Impl( new ::sd::MorphDlg( pParent, pObj1, pObj2 ) );
}
//add for MorphDlg end

// add for OutlineBulletDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdOutlineBulletTabDlg( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView )
{
    return new AbstractBulletDialog_Impl( new ::sd::OutlineBulletDlg( pParent, pAttr, pView ) );
}

SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdParagraphTabDlg( ::Window* pParent, const SfxItemSet* pAttr )
{
    return new AbstractTabDialog_Impl( new SdParagraphDlg( pParent, pAttr ) );
}
// add for OutlineBulletDlg end

// add for SdStartPresentationDlg begin
AbstractSdStartPresDlg *  SdAbstractDialogFactory_Impl::CreateSdStartPresentationDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, List& rPageNames, List* pCSList )
{
    return new AbstractSdStartPresDlg_Impl( new SdStartPresentationDlg( pWindow, rInAttrs, rPageNames, pCSList ) );
}
// add for SdStartPresentationDlg end

// add for SdPrintDlg begin
AbstractSdPrintDlg *  SdAbstractDialogFactory_Impl::CreateSdPrintDlg( ::Window* pWindow )
{
    return new AbstractSdPrintDlg_Impl( new SdPrintDlg( pWindow ) );
}
// add for SdPrintDlg end

// add for SdPresLayoutTemplateDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, ::Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool )
{
    return new SdPresLayoutTemplateDlg_Impl( new SdPresLayoutTemplateDlg( pDocSh, pParent, DlgId, rStyleBase, ePO, pSSPool ) );
}

// add for SdPresLayoutDlg begin
AbstractSdPresLayoutDlg *  SdAbstractDialogFactory_Impl::CreateSdPresLayoutDlg(  ::sd::DrawDocShell* pDocShell, ::sd::ViewShell* pViewShell, ::Window* pWindow, const SfxItemSet& rInAttrs)
{
    return new AbstractSdPresLayoutDlg_Impl( new SdPresLayoutDlg( pDocShell, pViewShell, pWindow, rInAttrs ) );
}
// add for SdPresLayoutDlg end

// add for SdTabTemplateDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabTemplateDlg( ::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView )
{
    return new AbstractTabDialog_Impl( new SdTabTemplateDlg( pParent, pDocShell, rStyleBase, pModel, pView ) );
}
// add for SdTabTemplateDlg end

AbstractSfxSingleTabDialog* SdAbstractDialogFactory_Impl::CreatSdActionDialog( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView )
{
    return new AbstractSfxSingleTabDialog_Impl( new SdActionDlg( pParent, pAttr, pView ) );
}

// add for SdVectorizeDlg begin
AbstractSdVectorizeDlg *  SdAbstractDialogFactory_Impl::CreateSdVectorizeDlg( ::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell )
{
    return new AbstractSdVectorizeDlg_Impl( new SdVectorizeDlg( pParent, rBmp, pDocShell ) );
}
// add for SdVectorizeDlg end

// add for SdPublishingDlg begin
AbstractSdPublishingDlg *  SdAbstractDialogFactory_Impl::CreateSdPublishingDlg( ::Window* pWindow, DocumentType eDocType)
{
    return new AbstractSdPublishingDlg_Impl( new SdPublishingDlg( pWindow, eDocType ) );
}
// add for SdPublishingDlg end

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

VclAbstractDialog* SdAbstractDialogFactory_Impl::CreateMasterLayoutDialog( ::Window* pParent,
    SdDrawDocument* pDoc, SdPage* pCurrentPage )
{
  return new VclAbstractDialog_Impl( new ::sd::MasterLayoutDialog( pParent, pDoc, pCurrentPage ));
}

AbstractHeaderFooterDialog* SdAbstractDialogFactory_Impl::CreateHeaderFooterDialog( ViewShell* pViewShell,
  ::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage )
{
    return new AbstractHeaderFooterDialog_Impl( new ::sd::HeaderFooterDialog( (::sd::ViewShell*)pViewShell, pParent, pDoc, pCurrentPage ));
}


