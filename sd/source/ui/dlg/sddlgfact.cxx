/*************************************************************************
 *
 *  $RCSfile: sddlgfact.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:45:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "sddlgfact.hxx"

// class ResId
#include <tools/rc.hxx>
//#include <sfx2/basedlgs.hxx>
#include "strings.hrc"

#include "BreakDlg.hxx" //add for BreakDlg
#include "brkdlg.hrc"
#include "copydlg.hxx" //add for CopyDlg
#include "copydlg.hrc"
#include "custsdlg.hxx" //add for SdCustomShowDlg
#include "custsdlg.hrc"
#include "dlg_char.hxx" //add for SdCharDlg
#include "dlgpage.hxx" //add for SdPageDlg
#include "dlgass.hxx" //add for AssistentDlg
#include "dlgass.hrc"
#include "dlgfield.hxx" //add for SdModifyFieldDlg
#include "dlgfield.hrc"
#include "dlgsnap.hxx" //add for SdSnapLineDlg
#include "dlgsnap.hrc"
#include "ins_page.hxx" //add for SdInsertLayerDlg
#include "ins_page.hrc"
#include "ins_paste.hxx" //add for SdInsertPasteDlg
#include "ins_paste.hrc"
#include "inspagob.hxx" //add for SdInsertPagesObjsDlg
#include "inspagob.hrc"
#include "morphdlg.hxx" //add for MorphDlg
#include "morphdlg.hrc"
#include "new_foil.hxx" //add for SdNewFoilDlg
#include "new_foil.hrc"
#include "OutlineBulletDlg.hxx" //add for OutlineBulletDlg
#include "paragr.hxx" //add for SdParagraphDlg
#include "paragr.hrc"
#include "present.hxx" //add for SdStartPresentationDlg
#include "present.hrc"
#include "printdlg.hxx" //add for SdPrintDlg
#include "printdlg.hrc"
#include "prltempl.hxx" //add for SdPresLayoutTemplateDlg
#include "prltempl.hrc"
#include "sdpreslt.hxx" //add for SdPresLayoutDlg
#include "sdpreslt.hrc"
#include "tabtempl.hxx" //add for SdTabTemplateDlg
#include "tabtempl.hrc"
#include "tpaction.hxx" //add for SdActionDlg
#include "tpaction.hrc"
#include "vectdlg.hxx" //add for SdVectorizeDlg
#include "vectdlg.hrc"
#include "tpoption.hxx" //add for SdTpOptionsSnap, SdTpOptionsContents, SdTpOptionsMisc
#include "tpoption.hrc"
#include "prntopts.hxx" //add for SdPrintOptions
#include "prntopts.hrc"
#include "pubdlg.hxx" //add for SdPublishingDlg
#include "pubdlg.hrc"

IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl); // add for BreakDlg
IMPL_ABSTDLG_BASE(AbstractCopyDlg_Impl); // add for CopyDlg
IMPL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl); // CHINA001 add for SdCustomShowDlg
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl); //add for SdCharDlg
IMPL_ABSTDLG_BASE(AbstractAssistentDlg_Impl); //add for AssistentDlg
IMPL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl); //add for SdModifyFieldDlg
IMPL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl); //add for SdSnapLineDlg
IMPL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl); //add for SdInsertLayerDlg
IMPL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl); //add for SdInsertPasteDlg
IMPL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl); //add for SdInsertPagesObjsDlg
IMPL_ABSTDLG_BASE(AbstractMorphDlg_Impl); //add for MorphDlg
IMPL_ABSTDLG_BASE(AbstractSdNewFoilDlg_Impl); //add for SdNewFoilDlg
IMPL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl); //add for SdStartPresentationDlg
IMPL_ABSTDLG_BASE(AbstractSdPrintDlg_Impl); //add for SdPrintDlg
IMPL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl); //add for SdPresLayoutDlg
IMPL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl); //add for SdActionDlg
IMPL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl); //add for SdVectorizeDlg
IMPL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl); //add for SdPublishingDlg

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
//add by CHINA001
const USHORT* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//add by CHINA001
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

//AbstractSdNewFoilDlg_Impl begin
void AbstractSdNewFoilDlg_Impl::GetAttr( SfxItemSet& rOutAttrs )
{
    pDlg->GetAttr( rOutAttrs );
}
// AbstractSdNewFoilDlg_Impl end

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

//-------------- SdAbstractDialogFactory implementation--------------

//add for BreakDlg begin
VclAbstractDialog * SdAbstractDialogFactory_Impl::CreateBreakDlg( const ResId& rResId,
                                            ::Window* pWindow,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            ULONG nSumActionCount,
                                            ULONG nObjCount ) //add for BreakDlg
{
    Dialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_BREAK :
            pDlg = new ::sd::BreakDlg( pWindow, pDrView, pShell, nSumActionCount, nObjCount );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}
//add for BreakDlg end

//add for CopyDlg begin
AbstractCopyDlg * SdAbstractDialogFactory_Impl::CreateCopyDlg( const ResId& rResId,
                                            ::Window* pWindow, const SfxItemSet& rInAttrs,
                                            XColorTable* pColTab, ::sd::View* pView ) //add for CopyDlg
{
    ::sd::CopyDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_COPY :
            pDlg = new ::sd::CopyDlg( pWindow, rInAttrs, pColTab, pView );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractCopyDlg_Impl( pDlg );
    return 0;
}
//add for CopyDlg end

//add for SdCustomShowDlg begin
AbstractSdCustomShowDlg * SdAbstractDialogFactory_Impl::CreateSdCustomShowDlg( const ResId& rResId,
                                            ::Window* pWindow, SdDrawDocument& rDrawDoc ) //add for SdCustomShowDlg
{
    SdCustomShowDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_CUSTOMSHOW :
            pDlg = new SdCustomShowDlg( pWindow, rDrawDoc );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdCustomShowDlg_Impl( pDlg );
    return 0;
}
//add for SdCustomShowDlg end

// add for SdCharDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabDialog( const ResId& rResId,
                                            ::Window* pParent, const SfxItemSet* pAttr,
                                            SfxObjectShell* pDocShell, BOOL bAreaPage ) //add for SdCharDlg, SdPageDlg
{

    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case TAB_CHAR :
            pDlg = new SdCharDlg( pParent, pAttr, pDocShell );
            break;
        case TAB_PAGE :
            pDlg = new SdPageDlg( pDocShell, pParent, pAttr, bAreaPage );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;

}
// add for SdCharDlg end

//add for AssistentDlg begin
AbstractAssistentDlg * SdAbstractDialogFactory_Impl::CreateAssistentDlg( const ResId& rResId,
                                            ::Window* pParent, BOOL bAutoPilot) //add for AssistentDlg
{
    AssistentDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_ASS :
            pDlg = new AssistentDlg( pParent, bAutoPilot );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractAssistentDlg_Impl( pDlg );
    return 0;
}
//add for AssistentDlg end

//add for SdModifyFieldDlg begin
AbstractSdModifyFieldDlg * SdAbstractDialogFactory_Impl::CreateSdModifyFieldDlg( const ResId& rResId,
                                            ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) //add for SdModifyFieldDlg
{
    SdModifyFieldDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_FIELD_MODIFY :
            pDlg = new SdModifyFieldDlg( pWindow, pInField, rSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdModifyFieldDlg_Impl( pDlg );
    return 0;
}
//add for SdModifyFieldDlg end

//add for SdSnapLineDlg begin
AbstractSdSnapLineDlg * SdAbstractDialogFactory_Impl::CreateSdSnapLineDlg( const ResId& rResId,
                                            ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView) //add for SdSnapLineDlg
{
    SdSnapLineDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_SNAPLINE :
            pDlg = new SdSnapLineDlg( pWindow, rInAttrs, pView );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdSnapLineDlg_Impl( pDlg );
    return 0;
}
//add for SdSnapLineDlg end

//add for SdInsertLayerDlg begin
AbstractSdInsertLayerDlg * SdAbstractDialogFactory_Impl::CreateSdInsertLayerDlg( const ResId& rResId,
                                            ::Window* pWindow,
                                            const SfxItemSet& rInAttrs,
                                            BOOL bDeletable,
                                            String aStr ) //add for SdInsertLayerDlg
{
    SdInsertLayerDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_INSERT_LAYER :
            pDlg = new SdInsertLayerDlg( pWindow, rInAttrs, bDeletable, aStr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdInsertLayerDlg_Impl( pDlg );
    return 0;
}
//add for SdInsertLayerDlg end

//add for SdInsertPasteDlg begin
AbstractSdInsertPasteDlg * SdAbstractDialogFactory_Impl::CreateSdInsertPasteDlg( const ResId& rResId, ::Window* pWindow ) //add for SdInsertPasteDlg
{
    SdInsertPasteDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_INSERT_PASTE :
            pDlg = new SdInsertPasteDlg( pWindow );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdInsertPasteDlg_Impl( pDlg );
    return 0;
}
//add for SdInsertPasteDlg end

//add for SdInsertPagesObjsDlg begin
AbstractSdInsertPagesObjsDlg * SdAbstractDialogFactory_Impl::CreateSdInsertPagesObjsDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SdDrawDocument* pDoc,
                                                SfxMedium* pSfxMedium,
                                                const String& rFileName ) //add for SdInsertPagesObjsDlg
{
    SdInsertPagesObjsDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_INSERT_PAGES_OBJS :
            pDlg = new SdInsertPagesObjsDlg( pParent, pDoc, pSfxMedium, rFileName );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdInsertPagesObjsDlg_Impl( pDlg );
    return 0;
}
//add for SdInsertPagesObjsDlg end

//add for MorphDlg begin
AbstractMorphDlg * SdAbstractDialogFactory_Impl::CreateMorphDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SdrObject* pObj1,
                                                const SdrObject* pObj2) //add for MorphDlg
{
    ::sd::MorphDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_MORPH :
            pDlg = new ::sd::MorphDlg( pParent, pObj1, pObj2 );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractMorphDlg_Impl( pDlg );
    return 0;
}
//add for MorphDlg end

//add for SdNewFoilDlg begin
AbstractSdNewFoilDlg * SdAbstractDialogFactory_Impl::CreateSdNewFoilDlg( const ResId& rResId,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs,
                                                PageKind ePgKind,
                                                ::sd::DrawDocShell* pDocShell,
                                                BOOL bChangeFoil ) //add for SdNewFoilDlg
{
    SdNewFoilDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_NEW_FOIL :
            pDlg = new SdNewFoilDlg( pWindow, rInAttrs, ePgKind, pDocShell, bChangeFoil );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdNewFoilDlg_Impl( pDlg );
    return 0;
}
//add for SdNewFoilDlg end

// add for OutlineBulletDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdItemSetTabDlg ( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxItemSet* pAttr,
                                                ::sd::View* pView ) //add for OutlineBulletDlg, SdParagraphDlg
{

    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case TAB_OUTLINEBULLET :
            pDlg = new ::sd::OutlineBulletDlg( pParent, pAttr, pView );
            break;
        case TAB_PARAGRAPH :
            pDlg = new SdParagraphDlg( pParent, pAttr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;

}
// add for OutlineBulletDlg end

// add for SdStartPresentationDlg begin
AbstractSdStartPresDlg *  SdAbstractDialogFactory_Impl::CreateSdStartPresentationDlg( const ResId& rResId,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs,
                                                List& rPageNames,
                                                List* pCSList ) //add for SdStartPresentationDlg
{

    SdStartPresentationDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_START_PRESENTATION :
            pDlg = new SdStartPresentationDlg( pWindow, rInAttrs, rPageNames, pCSList );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdStartPresDlg_Impl( pDlg );
    return 0;
}
// add for SdStartPresentationDlg end

// add for SdPrintDlg begin
AbstractSdPrintDlg *  SdAbstractDialogFactory_Impl::CreateSdPrintDlg( const ResId& rResId, ::Window* pWindow ) //add for SdPrintDlg
{

    SdPrintDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_PRINT_WARNINGS :
            pDlg = new SdPrintDlg( pWindow );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdPrintDlg_Impl( pDlg );
    return 0;
}
// add for SdPrintDlg end

// add for SdPresLayoutTemplateDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdPresLayoutTemplateDlg( const ResId& rResId,
                                                SfxObjectShell* pDocSh, ::Window* pParent,
                                                SdResId DlgId, SfxStyleSheetBase& rStyleBase,
                                                PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ) //add for SdPresLayoutTemplateDlg
{

    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case TAB_PRES_LAYOUT_TEMPLATE :
            pDlg = new SdPresLayoutTemplateDlg( pDocSh, pParent, DlgId, rStyleBase, ePO, pSSPool );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;

}
// add for SdPresLayoutTemplateDlg end

// add for SdPresLayoutDlg begin
AbstractSdPresLayoutDlg *  SdAbstractDialogFactory_Impl::CreateSdPresLayoutDlg( const ResId& rResId,
                                                ::sd::DrawDocShell* pDocShell,
                                                ::sd::ViewShell* pViewShell,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs) //add for SdPresLayoutDlg
{
    SdPresLayoutDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_PRESLT :
            pDlg = new SdPresLayoutDlg( pDocShell, pViewShell, pWindow, rInAttrs );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdPresLayoutDlg_Impl( pDlg );
    return 0;

}
// add for SdPresLayoutDlg end

// add for SdTabTemplateDlg begin
SfxAbstractTabDialog *  SdAbstractDialogFactory_Impl::CreateSdTabTemplateDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxObjectShell* pDocShell,
                                                SfxStyleSheetBase& rStyleBase,
                                                SdrModel* pModel,
                                                SdrView* pView ) //add for SdTabTemplateDlg
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case TAB_TEMPLATE :
            pDlg = new SdTabTemplateDlg( pParent, pDocShell, rStyleBase, pModel, pView );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
// add for SdTabTemplateDlg end

AbstractSfxSingleTabDialog* SdAbstractDialogFactory_Impl::CreateSfxSingleTabDialog( const ResId& rResId,
                                            ::Window* pParent,
                                            const SfxItemSet* pAttr,
                                            ::sd::View* pView ) //add for SdActionDlg
{
    SfxSingleTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case TP_ANIMATION_ACTION :
            pDlg = new SdActionDlg( pParent, pAttr, pView );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSfxSingleTabDialog_Impl( pDlg );
    return 0;
}

// add for SdVectorizeDlg begin
AbstractSdVectorizeDlg *  SdAbstractDialogFactory_Impl::CreateSdVectorizeDlg( const ResId& rResId,
                                                ::Window* pParent, const Bitmap& rBmp,
                                                ::sd::DrawDocShell* pDocShell ) //add for SdVectorizeDlg
{
    SdVectorizeDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_VECTORIZE :
            pDlg = new SdVectorizeDlg( pParent, rBmp, pDocShell );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdVectorizeDlg_Impl( pDlg );
    return 0;
}
// add for SdVectorizeDlg end

// add for SdPublishingDlg begin
AbstractSdPublishingDlg *  SdAbstractDialogFactory_Impl::CreateSdPublishingDlg( const ResId& rResId,
                                                ::Window* pWindow, DocumentType eDocType) //add for SdPublishingDlg
{
    SdPublishingDlg* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case DLG_PUBLISHING :
            pDlg = new SdPublishingDlg( pWindow, eDocType );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractSdPublishingDlg_Impl( pDlg );
    return 0;
}
// add for SdPublishingDlg end

// Factories for TabPages
CreateTabPage SdAbstractDialogFactory_Impl::GetTabPageCreatorFunc( USHORT nId )
{
    switch ( nId )
    {
        case TP_OPTIONS_CONTENTS:
            return SdTpOptionsContents::Create;
            break;
        case TP_PRINT_OPTIONS:
            return SdPrintOptions::Create;
            break;
        case TP_OPTIONS_MISC:
            return SdTpOptionsMisc::Create;
            break;
        case TP_OPTIONS_SNAP:
            return SdTpOptionsSnap::Create;
            break;
        default:
            break;
    }

    return 0;
}

GetTabPageRanges SdAbstractDialogFactory_Impl::GetTabPageRangesFunc( USHORT nId )
{
    switch ( nId )
    {
    case 1 : //RID_SVXPAGE_TEXTANIMATION :
            //return SvxTextAnimationPage::GetRanges;
            break;
        default:
            break;
    }

    return 0;
}
