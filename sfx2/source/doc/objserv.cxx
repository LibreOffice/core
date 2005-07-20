/*************************************************************************
 *
 *  $RCSfile: objserv.cxx,v $
 *
 *  $Revision: 1.86 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-20 12:26:35 $
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

#include <sot/storage.hxx>

#ifndef  _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_SECURITY_DOCUMENTSIGNATURESINFORMATION_HPP_
#include <com/sun/star/security/DocumentSignaturesInformation.hpp>
#endif

#ifndef _COM_SUN_STAR_SECURITY_XDOCUMENTDIGITALSIGNATURES_HPP_
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFX_INIMGR_HXX //autogen
    #include <inimgr.hxx>
    #endif
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#include <vcl/wrkwin.hxx>
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <basic/sbx.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/saveopt.hxx>

#ifndef GCC
#pragma hdrstop
#endif

#include "sfxresid.hxx"
#include "event.hxx"
#include "request.hxx"
#include "printer.hxx"
#include "viewsh.hxx"
#include "doctdlg.hxx"
#include "docfilt.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "dispatch.hxx"
#include "dinfdlg.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"
//#include "interno.hxx"
#include "module.hxx"
#include "topfrm.hxx"
#include "versdlg.hxx"
#include "doc.hrc"
#include "docfac.hxx"
#include "fcontnr.hxx"
#include "filedlghelper.hxx"
#include "sfxhelp.hxx"
#include "msgpool.hxx"
#include "objface.hxx"

#include "../appl/app.hrc"
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>


#ifndef _SFX_HELPID_HRC
#include "helpid.hrc"
#endif

#include "guisaveas.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::task;

//====================================================================

class SfxSaveAsContext_Impl
{
    String&     _rNewNameVar;
    String      _aNewName;

public:
                SfxSaveAsContext_Impl( String &rNewNameVar,
                                       const String &rNewName )
                :   _rNewNameVar( rNewNameVar ),
                    _aNewName( rNewName )
                { rNewNameVar = rNewName; }
                ~SfxSaveAsContext_Impl()
                { _rNewNameVar.Erase(); }
};

//====================================================================

#define SfxObjectShell
#include "sfxslots.hxx"

svtools::AsynchronLink* pPendingCloser = 0;

//=========================================================================



SFX_IMPL_INTERFACE(SfxObjectShell,SfxShell,SfxResId(0))
{
}

long SfxObjectShellClose_Impl( void* pObj, void* pArg )
{
    SfxObjectShell *pObjSh = (SfxObjectShell*) pArg;
    if ( pObjSh->Get_Impl()->bHiddenLockedByAPI )
    {
        pObjSh->Get_Impl()->bHiddenLockedByAPI = FALSE;
        pObjSh->OwnerLock(FALSE);
    }
    else if ( !pObjSh->Get_Impl()->bClosing )
        // GCC stuerzt ab, wenn schon im dtor, also vorher Flag abfragen
        pObjSh->DoClose();
    return 0;
}

//=========================================================================

void SfxObjectShell::PrintExec_Impl(SfxRequest &rReq)
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this);
    if ( pFrame )
    {
        rReq.SetSlot( SID_PRINTDOC );
        pFrame->GetViewShell()->ExecuteSlot(rReq);
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::PrintState_Impl(SfxItemSet &rSet)
{
    FASTBOOL bPrinting = FALSE;
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this, TYPE(SfxTopViewFrame));
    if ( pFrame )
    {
        SfxPrinter *pPrinter = pFrame->GetViewShell()->GetPrinter();
        bPrinting = pPrinter && pPrinter->IsPrinting();
    }
    rSet.Put( SfxBoolItem( SID_PRINTOUT, bPrinting ) );
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::APISaveAs_Impl
(
    const String& aFileName,
    SfxItemSet*   aParams
)
{
    BOOL bOk = sal_False;

    {DBG_CHKTHIS(SfxObjectShell, 0);}

    if ( GetMedium() )
    {
        SFX_ITEMSET_ARG( aParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
        sal_Bool bSaveTo = pSaveToItem && pSaveToItem->GetValue();

        String aFilterName;
        SFX_ITEMSET_ARG( aParams, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if( pFilterNameItem )
        {
            aFilterName = pFilterNameItem->GetValue();
        }
        else
        {
            SFX_ITEMSET_ARG( aParams, pContentTypeItem, SfxStringItem, SID_CONTENTTYPE, sal_False );
            if ( pContentTypeItem )
            {
                const SfxFilter* pFilter = SfxFilterMatcher( String::CreateFromAscii(GetFactory().GetShortName()) ).GetFilter4Mime( pContentTypeItem->GetValue(), SFX_FILTER_EXPORT );
                if ( pFilter )
                    aFilterName = pFilter->GetName();
            }
        }

        // in case no filter defined use default one
        if( !aFilterName.Len() )
        {
            const SfxFilter* pFilt = SfxFilter::GetDefaultFilterFromFactory(GetFactory().GetFactoryName());

            DBG_ASSERT( pFilt, "No default filter!\n" );
            if( pFilt )
                aFilterName = pFilt->GetFilterName();

            aParams->Put(SfxStringItem( SID_FILTER_NAME, aFilterName));
        }


        {
            SfxObjectShellRef xLock( this ); // ???

            // since saving a document modified its DocumentInfo, the current DocumentInfo must be saved on "SaveTo", because
            // it must be restored after saving
            SfxDocumentInfo aSavedInfo;
            sal_Bool bCopyTo =  bSaveTo || GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
            if ( bCopyTo )
                aSavedInfo = GetDocInfo();

            bOk = CommonSaveAs_Impl( INetURLObject(aFileName), aFilterName,
                aParams );

            if ( bCopyTo )
            {
                // restore DocumentInfo if only a copy was created
                SfxDocumentInfo &rDocInfo = GetDocInfo();
                rDocInfo = aSavedInfo;
            }
        }

        // Picklisten-Eintrag verhindern
        GetMedium()->SetUpdatePickList( FALSE );
    }

    return bOk;
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecFile_Impl(SfxRequest &rReq)
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}

    USHORT nId = rReq.GetSlot();

    if( SID_SIGNATURE == nId || SID_MACRO_SIGNATURE == nId )
    {
        if ( QueryHiddenInformation( WhenSigning, NULL ) == RET_YES )
            ( SID_SIGNATURE == nId ) ? SignDocumentContent() : SignScriptingContent();
        return;
    }

    if ( !GetMedium() && nId != SID_CLOSEDOC )
    {
        rReq.Ignore();
        return;
    }

    sal_Bool bIsPDFExport = sal_False;
    switch(nId)
    {
        case SID_SAVE_VERSION_ON_CLOSE:
        {
            BOOL bSet = GetDocInfo().IsSaveVersionOnClose();
            SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, nId, FALSE);
            if ( pItem )
                bSet = pItem->GetValue();
            GetDocInfo().SetSaveVersionOnClose( bSet );
            SetModified( TRUE );
            if ( !pItem )
                rReq.AppendItem( SfxBoolItem( nId, bSet ) );
            rReq.Done();
            return;
            break;
        }
        case SID_VERSION:
        {
            SfxViewFrame* pFrame = GetFrame();
            if ( !pFrame )
                pFrame = SfxViewFrame::GetFirst( this );
            if ( !pFrame )
                return;

            if ( pFrame->GetFrame()->GetParentFrame() )
            {
                pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                return;
            }

            if ( !IsOwnStorageFormat_Impl( *GetMedium() ) )
                return;

            SfxVersionDialog *pDlg = new SfxVersionDialog( pFrame, NULL );
            pDlg->Execute();
            delete pDlg;
            return;
            break;
        }

//REMOVE            case SID_LOAD_LIBRARY:
//REMOVE            case SID_UNLOAD_LIBRARY:
//REMOVE            case SID_REMOVE_LIBRARY:
//REMOVE            case SID_ADD_LIBRARY:
//REMOVE            {
//REMOVE                // Diese Funktionen sind nur f"ur Aufrufe aus dem Basic gedacht
//REMOVE                SfxApplication *pApp = SFX_APP();
//REMOVE                if ( pApp->IsInBasicCall() )
//REMOVE                    pApp->BasicLibExec_Impl( rReq, GetBasicManager() );
//REMOVE                return;
//REMOVE                break;
//REMOVE            }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCINFO:
        {
            SFX_REQUEST_ARG(rReq, pDocInfItem, SfxDocumentInfoItem, SID_DOCINFO, FALSE);

            // keine Parameter vorhanden?
            if ( !pDocInfItem )
            {
                // Dialog ausf"uhren
                SfxDocumentInfo *pOldInfo = new SfxDocumentInfo;
                if ( pImp->pDocInfo )
                    // r/o-flag korrigieren falls es zu frueh gesetzt wurde
                    pImp->pDocInfo->SetReadOnly( IsReadOnly() );
                *pOldInfo = GetDocInfo();
                DocInfoDlg_Impl( GetDocInfo() );

                // ge"andert?
                if( !(*pOldInfo == GetDocInfo()) )
                {
                    // Dokument gilt als ver"andert
                    FlushDocInfo();

                    // ggf. Recorden
                    if ( !rReq.IsRecording() )
                        rReq.AppendItem( SfxDocumentInfoItem( GetTitle(), GetDocInfo() ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();

                delete pOldInfo;
            }
            else
            {
                // DocInfo aus Parameter anwenden
                GetDocInfo() = (*pDocInfItem)();
                FlushDocInfo();
            }

            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_EXPORTDOCASPDF:
        case SID_DIRECTEXPORTDOCASPDF:
            bIsPDFExport = sal_True;
        case SID_EXPORTDOC:
        case SID_SAVEASDOC:
        case SID_SAVEDOC:
        {
            //!! detaillierte Auswertung eines Fehlercodes
            SfxObjectShellRef xLock( this );

            sal_Bool bDialogUsed = sal_False;
            sal_uInt32 nErrorCode = ERRCODE_NONE;

            // by default versions should be preserved always except in case of an explicit
            // SaveAs via GUI, so the flag must be set accordingly
            pImp->bPreserveVersions = (nId == SID_SAVEDOC);
            try
            {
                SfxErrorContext aEc( ERRCTX_SFX_SAVEASDOC, GetTitle() ); // ???

                // xmlsec05, check with SFX team
                if ( ( GetDocumentSignatureState() == SIGNATURESTATE_SIGNATURES_OK ) || ( GetDocumentSignatureState() == SIGNATURESTATE_SIGNATURES_INVALID ) )
                {
                    if( QueryBox( NULL, SfxResId( RID_XMLSEC_QUERY_LOSINGSIGNATURE ) ).Execute() != RET_YES )
                        return;
                }

                if ( nId == SID_SAVEASDOC )
                {
                    // in case of plugin mode the SaveAs operation means SaveTo
                    SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pViewOnlyItem, SfxBoolItem, SID_VIEWONLY, FALSE );
                    if ( pViewOnlyItem && pViewOnlyItem->GetValue() )
                        rReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );
                }

                // TODO/LATER: do the following GUI related actions in standalown method
                // ========================================================================================================
                // Introduce a status indicator for GUI operation
                SFX_REQUEST_ARG( rReq, pStatusIndicatorItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, FALSE );
                if ( !pStatusIndicatorItem )
                {
                    // get statusindicator
                    uno::Reference< task::XStatusIndicator > xStatusIndicator;
                    SfxViewFrame *pFrame = GetFrame();
                    if ( pFrame && pFrame->GetFrame() )
                    {
                        uno::Reference< task::XStatusIndicatorFactory > xStatFactory(
                                                                    pFrame->GetFrame()->GetFrameInterface(),
                                                                    uno::UNO_QUERY );
                        if( xStatFactory.is() )
                            xStatusIndicator = xStatFactory->createStatusIndicator();
                    }


                    OSL_ENSURE( xStatusIndicator.is(), "Can not retrieve default status indicator!\n" );
                    if ( xStatusIndicator.is() )
                    {
                        SfxUnoAnyItem aStatIndItem( SID_PROGRESS_STATUSBAR_CONTROL, uno::makeAny( xStatusIndicator ) );

                        if ( nId == SID_SAVEDOC )
                        {
                            // in case of saving it is not possible to transport the parameters from here
                            // but it is not clear here whether the saving will be done or saveAs operation
                            GetMedium()->GetItemSet()->Put( aStatIndItem );
                        }

                        rReq.AppendItem( aStatIndItem );
                    }
                }
                else if ( nId == SID_SAVEDOC )
                {
                    // in case of saving it is not possible to transport the parameters from here
                    // but it is not clear here whether the saving will be done or saveAs operation
                    GetMedium()->GetItemSet()->Put( *pStatusIndicatorItem );
                }

                // Introduce an interaction handler for GUI operation
                SFX_REQUEST_ARG( rReq, pInteractionHandlerItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, FALSE );
                if ( !pInteractionHandlerItem )
                {
                    uno::Reference< task::XInteractionHandler > xInteract;
                    uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
                    if( xServiceManager.is() )
                    {
                        xInteract = Reference< XInteractionHandler >(
                            xServiceManager->createInstance( DEFINE_CONST_UNICODE("com.sun.star.task.InteractionHandler") ),
                            UNO_QUERY );
                    }

                    OSL_ENSURE( xInteract.is(), "Can not retrieve default status indicator!\n" );
                    if ( xInteract.is() )
                    {
                        SfxUnoAnyItem aInteractionItem( SID_INTERACTIONHANDLER, uno::makeAny( xInteract ) );
                        if ( nId == SID_SAVEDOC )
                        {
                            // in case of saving it is not possible to transport the parameters from here
                            // but it is not clear here whether the saving will be done or saveAs operation
                            GetMedium()->GetItemSet()->Put( aInteractionItem );
                        }

                        rReq.AppendItem( aInteractionItem );
                    }
                }
                else if ( nId == SID_SAVEDOC )
                {
                    // in case of saving it is not possible to transport the parameters from here
                    // but it is not clear here whether the saving will be done or saveAs operation
                    GetMedium()->GetItemSet()->Put( *pInteractionHandlerItem );
                }
                // ========================================================================================================

                SFX_REQUEST_ARG( rReq, pPasswordItem, SfxStringItem, SID_PASSWORD, FALSE );
                SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pOldPasswordItem, SfxStringItem, SID_PASSWORD, FALSE );
                if ( pOldPasswordItem && !pPasswordItem )
                    rReq.AppendItem( SfxStringItem( SID_PASSWORD, String() ) );

                uno::Sequence< beans::PropertyValue > aDispatchArgs;
                if ( rReq.GetArgs() )
                    TransformItems( nId,
                                    *rReq.GetArgs(),
                                     aDispatchArgs,
                                     NULL );

                const SfxSlot* pSlot = GetModule()->GetSlotPool()->GetSlot( nId );
                if ( !pSlot )
                    throw uno::Exception();

                uno::Reference< lang::XMultiServiceFactory > xEmptyFactory;
                SfxStoringHelper aHelper( xEmptyFactory );

                if ( QueryHiddenInformation( bIsPDFExport ? WhenCreatingPDF : WhenSaving, NULL ) == RET_YES )
                    bDialogUsed = aHelper.GUIStoreModel( GetModel(),
                                                    ::rtl::OUString::createFromAscii( pSlot->GetUnoName() ),
                                                    aDispatchArgs );

                pImp->nDocumentSignatureState = SIGNATURESTATE_NOSIGNATURES;
                pImp->nScriptingSignatureState = SIGNATURESTATE_NOSIGNATURES;
                pImp->bSignatureErrorIsShown = sal_False;

                // merge aDispatchArgs to the request
                SfxAllItemSet aResultParams( GetPool() );
                TransformParameters( nId,
                                     aDispatchArgs,
                                     aResultParams,
                                     NULL );
                rReq.SetArgs( aResultParams );

                SFX_REQUEST_ARG( rReq, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, FALSE );
                ::rtl::OUString aFilterName = pFilterNameItem ? ::rtl::OUString( pFilterNameItem->GetValue() )
                                                              : ::rtl::OUString();
                const SfxFilter* pFilt = GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName );

                OSL_ENSURE( nId == SID_SAVEDOC || pFilt, "The filter can not be zero since it was used for storing!\n" );
                if  (   bDialogUsed && pFilt
                    &&  pFilt->IsOwnFormat()
                    &&  pFilt->UsesStorage()
                    &&  pFilt->GetVersion() >= SOFFICE_FILEFORMAT_60 )
                {
                    SfxViewFrame* pDocViewFrame = SfxViewFrame::GetFirst( this );
                    SfxFrame* pDocFrame = pDocViewFrame ? pDocViewFrame->GetFrame() : NULL;
                    if ( pDocFrame )
                        SfxHelp::OpenHelpAgent( pDocFrame, HID_DID_SAVE_PACKED_XML );
                }

                // the StoreAsURL/StoreToURL method have called this method with false
                // so it has to be restored to true here since it is a call from GUI
                GetMedium()->SetUpdatePickList( sal_True );

                // TODO: in future it must be done in followind way
                // if document is opened from GUI it is immediatelly appeares in the picklist
                // if the document is a new one then it appeares in the picklist immediatelly
                // after SaveAs operation triggered from GUI
            }
            catch( task::ErrorCodeIOException& aErrorEx )
            {
                nErrorCode = (sal_uInt32)aErrorEx.ErrCode;
            }
            catch( Exception& )
            {
                nErrorCode = ERRCODE_IO_GENERAL;
            }

            // by default versions should be preserved always except in case of an explicit
            // SaveAs via GUI, so the flag must be reset to guarantee this
            pImp->bPreserveVersions = sal_True;
            ULONG lErr=GetErrorCode();

            if ( !lErr && nErrorCode )
                lErr = nErrorCode;

            if ( lErr && nErrorCode == ERRCODE_NONE )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE );
                if ( pWarnItem && pWarnItem->GetValue() )
                    nErrorCode = lErr;
            }

            // may be nErrorCode should be shown in future
            if ( lErr != ERRCODE_IO_ABORT )
            {
                SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC,GetTitle());
                ErrorHandler::HandleError( lErr );
            }

            if ( nId == SID_EXPORTDOCASPDF )
            {
                // This function is used by the SendMail function that needs information if a export
                // file was written or not. This could be due to cancellation of the export
                // or due to an error. So IO abort must be handled like an error!
                nErrorCode = ( lErr != ERRCODE_IO_ABORT ) && ( nErrorCode == ERRCODE_NONE ) ? nErrorCode : lErr;
            }

            rReq.SetReturnValue( SfxBoolItem(0, nErrorCode == ERRCODE_NONE ) );

            ResetError();

            Invalidate();
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_CLOSEDOC:
        {
            SfxViewFrame *pFrame = GetFrame();
            if ( pFrame && pFrame->GetFrame()->GetParentFrame() )
            {
                // Wenn SID_CLOSEDOC "uber Menue etc. ausgef"uhrt wird, das
                // aktuelle Dokument aber in einem Frame liegt, soll eigentlich
                // das FrameSetDocument geclosed werden
                pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                rReq.Done();
                return;
            }

            BOOL bInFrameSet = FALSE;
            USHORT nFrames=0;
            pFrame = SfxViewFrame::GetFirst( this );
            while ( pFrame )
            {
                if ( pFrame->GetFrame()->GetParentFrame() )
                {
                    // Auf dieses Dokument existiert noch eine Sicht, die
                    // in einem FrameSet liegt; diese darf nat"urlich nicht
                    // geclosed werden
                    bInFrameSet = TRUE;
                }
                else
                    nFrames++;

                pFrame = SfxViewFrame::GetNext( *pFrame, this );
            }

            if ( bInFrameSet )
            {
                // Alle Sichten, die nicht in einem FrameSet liegen, closen
                pFrame = SfxViewFrame::GetFirst( this );
                while ( pFrame )
                {
                    if ( !pFrame->GetFrame()->GetParentFrame() )
                        pFrame->GetFrame()->DoClose();
                    pFrame = SfxViewFrame::GetNext( *pFrame, this );
                }
            }

            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pSaveItem, SfxBoolItem, SID_CLOSEDOC_SAVE, FALSE);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_CLOSEDOC_FILENAME, FALSE);
            if ( pSaveItem )
            {
                if ( pSaveItem->GetValue() )
                {
                    if ( !pNameItem )
                    {
                        SbxBase::SetError( SbxERR_WRONG_ARGS );
                        rReq.Ignore();
                        return;
                    }
                    SfxAllItemSet aArgs( GetPool() );
                    SfxStringItem aTmpItem( SID_FILE_NAME, pNameItem->GetValue() );
                    aArgs.Put( aTmpItem, aTmpItem.Which() );
                    SfxRequest aSaveAsReq( SID_SAVEASDOC, SFX_CALLMODE_API, aArgs );
                    ExecFile_Impl( aSaveAsReq );
                    if ( !aSaveAsReq.IsDone() )
                    {
                        rReq.Ignore();
                        return;
                    }
                }
                else
                    SetModified(FALSE);
            }

            // Benutzer bricht ab?
            if ( !PrepareClose( 2 ) )
            {
                rReq.SetReturnValue( SfxBoolItem(0, FALSE) );
                rReq.Done();
                return;
            }

            SetModified( FALSE );
            ULONG lErr = GetErrorCode();
            ErrorHandler::HandleError(lErr);

            rReq.SetReturnValue( SfxBoolItem(0, TRUE) );
            rReq.Done();
            rReq.ReleaseArgs(); // da der Pool in Close zerst"ort wird

            if ( SfxApplication::IsPlugin() )
            {
                for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
                {
                    String aName = String::CreateFromAscii("vnd.sun.star.cmd:close");
                    SfxStringItem aNameItem( SID_FILE_NAME, aName );
                    SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private/user" ) );
                    SfxFrameItem aFrame( SID_DOCFRAME, pFrame->GetFrame() );
                    SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SLOT, &aNameItem, &aReferer, 0L );
                    return;
                }
            }
/*
            com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier >
                    xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                    com::sun::star::uno::UNO_QUERY );
            com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess > xList ( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
            sal_Int32 nCount = xList->getCount();
            if ( nCount == nFrames )
            {
                SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
                SfxViewFrame* pLastFrame = SfxViewFrame::Current();
                if ( pLastFrame->GetObjectShell() != this )
                    pLastFrame = pFrame;

                SfxViewFrame* pNextFrame = pFrame;
                while ( pNextFrame )
                {
                    pNextFrame = SfxViewFrame::GetNext( *pFrame, this );
                    if ( pFrame != pLastFrame )
                        pFrame->GetFrame()->DoClose();
                    pFrame = pNextFrame;
                }

                pLastFrame->GetFrame()->CloseDocument_Impl();
            }
            else
 */
                DoClose();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCTEMPLATE:
        {
            // speichern als Dokumentvorlagen
            SfxDocumentTemplateDlg *pDlg = 0;
            SfxErrorContext aEc(ERRCTX_SFX_DOCTEMPLATE,GetTitle());
            SfxDocumentTemplates *pTemplates =  new SfxDocumentTemplates;

            // Find the template filter with the highest version number
            const SfxObjectFactory& rFactory = GetFactory();
            const SfxFilter* pFilter = rFactory.GetTemplateFilter();
            DBG_ASSERT( pFilter, "Template Filter nicht gefunden" );
            if ( !pFilter )
            {
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                return;
            }

            if ( !rReq.GetArgs() )
            {
                pDlg = new SfxDocumentTemplateDlg(0, pTemplates);
                if ( RET_OK == pDlg->Execute() && pDlg->GetTemplateName().Len())
                {
                    String aTargetURL = pTemplates->GetTemplatePath(
                            pDlg->GetRegion(),
                            pDlg->GetTemplateName());

                    if ( aTargetURL.Len() )
                    {
                        INetURLObject aTargetObj( aTargetURL );
                        String aTplExtension( pFilter->GetDefaultExtension().Copy(2) );
                        aTargetObj.setExtension( aTplExtension );
                        aTargetURL = aTargetObj.GetMainURL( INetURLObject::NO_DECODE );
                    }

                    rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aTargetURL ) );

                    rReq.AppendItem(SfxStringItem(
                        SID_TEMPLATE_NAME, pDlg->GetTemplateName()));
                    rReq.AppendItem(SfxUInt16Item(
                        SID_TEMPLATE_REGION, pDlg->GetRegion()));
                }
                else
                {
                    delete pDlg;
                    rReq.Ignore();
                    return;
                }
            }

            // Region und Name aus Parameter holen
            SFX_REQUEST_ARG(rReq, pRegionItem, SfxStringItem, SID_TEMPLATE_REGIONNAME, FALSE);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_TEMPLATE_NAME, FALSE);
            SFX_REQUEST_ARG(rReq, pRegionNrItem, SfxUInt16Item, SID_TEMPLATE_REGION, FALSE);
            if ( (!pRegionItem && !pRegionNrItem ) || !pNameItem )
            {
                DBG_ASSERT( rReq.IsAPI(), "non-API call without Arguments" );
                SbxBase::SetError( SbxERR_WRONG_ARGS );
                rReq.Ignore();
                return;
            }
            String aTemplateName = pNameItem->GetValue();

            // Region-Nr besorgen
            USHORT nRegion = 0;
            if( pRegionItem )
            {
                // Region-Name finden (eigentlich nicht unbedingt eindeutig)
                nRegion = pTemplates->GetRegionNo( pRegionItem->GetValue() );
                if ( nRegion == USHRT_MAX )
                {
                    SbxBase::SetError( ERRCODE_IO_INVALIDPARAMETER );
                    rReq.Ignore();
                    return;
                }
            }
            if ( pRegionNrItem )
                nRegion = pRegionNrItem->GetValue();

            // kein File-Name angegeben?
            if ( SFX_ITEM_SET != rReq.GetArgs()->GetItemState( SID_FILE_NAME ) )
            {
                // TemplatePath nicht angebgeben => aus Region+Name ermitteln
                // Dateiname zusammenbauen lassen
                String aTemplPath = pTemplates->GetTemplatePath( nRegion, aTemplateName );
                INetURLObject aURLObj( aTemplPath );
                String aExtension( pFilter->GetDefaultExtension().Copy(2) );
                aURLObj.setExtension( aExtension, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );

                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aURLObj.GetMainURL( INetURLObject::NO_DECODE ) ) );
            }

            // Dateiname
            SFX_REQUEST_ARG(rReq, pFileItem, SfxStringItem, SID_FILE_NAME, FALSE);
            const String aFileName(((const SfxStringItem *)pFileItem)->GetValue());

            // Medium zusammenbauen
            SfxItemSet* pSet = new SfxAllItemSet( *rReq.GetArgs() );
            SfxMedium aMedium( aFileName, STREAM_STD_READWRITE, FALSE, pFilter, pSet);

            // als Vorlage speichern
            BOOL bModified = IsModified();
//REMOVE                BOOL bHasTemplateConfig = HasTemplateConfig();
//REMOVE                SetTemplateConfig( FALSE );
            BOOL bOK = FALSE;

            //aMedium.CreateTempFileNoCopy();

            // Because we can't save into a storage directly ( only using tempfile ), we must save the DocInfo first, then
            // we can call SaveTo_Impl and Commit
//REMOVE                if ( pFilter->UsesStorage() && ( pFilter->GetVersion() < SOFFICE_FILEFORMAT_60 ) )
//REMOVE                {
//REMOVE                    SfxDocumentInfo *pInfo = new SfxDocumentInfo;
//REMOVE                    pInfo->CopyUserData(GetDocInfo());
//REMOVE                    pInfo->SetTitle( aTemplateName );
//REMOVE                    pInfo->SetChanged( SfxStamp(SvtUserOptions().GetFullName()));
//REMOVE                    SvStorageRef aRef = aMedium.GetStorage();
//REMOVE                    if ( aRef.Is() )
//REMOVE                    {
//REMOVE                        pInfo->SetTime(0L);
//REMOVE                        pInfo->Save(aRef);
//REMOVE                    }
//REMOVE
//REMOVE                    delete pInfo;
//REMOVE                }

            if ( SaveTo_Impl( aMedium, NULL ) )
            {
                bOK = TRUE;
                pTemplates->NewTemplate( nRegion, aTemplateName, aFileName );
            }

            DELETEX(pDlg);

            SetError(aMedium.GetErrorCode());
            ULONG lErr=GetErrorCode();
            if(!lErr && !bOK)
                lErr=ERRCODE_IO_GENERAL;
            ErrorHandler::HandleError(lErr);
            ResetError();
            delete pTemplates;

//REMOVE                if ( IsHandsOff() )
//REMOVE                {
//REMOVE                    if ( !DoSaveCompleted( pMedium ) )
//REMOVE                        DBG_ERROR("Case not handled - no way to get a storage!");
//REMOVE                }
//REMOVE                else
                DoSaveCompleted();

//REMOVE                SetTemplateConfig( bHasTemplateConfig );
            SetModified(bModified);
            rReq.SetReturnValue( SfxBoolItem( 0, bOK ) );
            if ( !bOK )
                return;
            break;
        }
    }

    // Picklisten-Eintrag verhindern
    if ( rReq.IsAPI() )
        GetMedium()->SetUpdatePickList( FALSE );
    else if ( rReq.GetArgs() )
    {
        SFX_ITEMSET_GET( *rReq.GetArgs(), pPicklistItem, SfxBoolItem, SID_PICKLIST, FALSE );
        if ( pPicklistItem )
            GetMedium()->SetUpdatePickList( pPicklistItem->GetValue() );
    }

    // Ignore()-Zweige haben schon returnt
    rReq.Done();
}

//--------------------------------------------------------------------

void SfxObjectShell::GetState_Impl(SfxItemSet &rSet)
{
    DBG_CHKTHIS(SfxObjectShell, 0);
    SfxWhichIter aIter( rSet );

    for ( USHORT nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
            case SID_SAVE_VERSION_ON_CLOSE:
            {
                rSet.Put( SfxBoolItem( nWhich, GetDocInfo().IsSaveVersionOnClose() ) );
                break;
            }

            case SID_DOCTEMPLATE :
            {
                if ( !GetFactory().GetTemplateFilter() )
                    rSet.DisableItem( nWhich );
                break;
            }

            case SID_VERSION:
                {
                    SfxObjectShell *pDoc = this;
                    SfxViewFrame* pFrame = GetFrame();
                    if ( !pFrame )
                        pFrame = SfxViewFrame::GetFirst( this );
                    if ( pFrame  )
                    {
                        if ( pFrame->GetFrame()->GetParentFrame() )
                        {
                            pFrame = pFrame->GetTopViewFrame();
                            pDoc = pFrame->GetObjectShell();
                        }
                    }

                    if ( !pFrame || !pDoc->HasName() ||
                        !IsOwnStorageFormat_Impl( *pDoc->GetMedium() ) )
//REMOVE                            || pDoc->GetMedium()->GetStorage()->GetVersion() < SOFFICE_FILEFORMAT_50 )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_SAVEDOC:
                {
                    BOOL bMediumRO = IsReadOnlyMedium();
                    if ( !bMediumRO && GetMedium() && IsModified() )
                        rSet.Put(SfxStringItem(
                            nWhich, String(SfxResId(STR_SAVEDOC))));
                    else
                        rSet.DisableItem(nWhich);
                }
                break;

            case SID_DOCINFO:
                if ( 0 != ( pImp->eFlags & SFXOBJECTSHELL_NODOCINFO ) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_CLOSEDOC:
            {
                SfxObjectShell *pDoc = this;
                SfxViewFrame *pFrame = GetFrame();
                if ( pFrame && pFrame->GetFrame()->GetParentFrame() )
                {
                    // Wenn SID_CLOSEDOC "uber Menue etc. ausgef"uhrt wird, das
                    // aktuelle Dokument aber in einem Frame liegt, soll eigentlich
                    // das FrameSetDocument geclosed werden
                    pDoc = pFrame->GetTopViewFrame()->GetObjectShell();
                }

                if ( pDoc->GetFlags() & SFXOBJECTSHELL_DONTCLOSE )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxStringItem(nWhich, String(SfxResId(STR_CLOSEDOC))));
                break;
            }

            case SID_SAVEASDOC:
            {
                if( ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) != SFX_LOADED_MAINDOCUMENT )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }
/*
                const SfxFilter* pCombinedFilters = NULL;
                SfxFilterContainer* pFilterContainer = GetFactory().GetFilterContainer();

                if ( pFilterContainer )
                {
                    SfxFilterFlags    nMust    = SFX_FILTER_IMPORT | SFX_FILTER_EXPORT;
                    SfxFilterFlags    nDont    = SFX_FILTER_NOTINSTALLED | SFX_FILTER_INTERNAL;

                    pCombinedFilters = pFilterContainer->GetAnyFilter( nMust, nDont );
                }
*/
                if ( /*!pCombinedFilters ||*/ !GetMedium() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxStringItem( nWhich, String( SfxResId( STR_SAVEASDOC ) ) ) );
                break;
            }

            case SID_EXPORTDOCASPDF:
            case SID_DIRECTEXPORTDOCASPDF:
            {
                /*

                 search for filter cant work correctly ...
                 Because it's not clear, which export filter for which office module
                 must be searched. On the other side it can be very expensive doing so.
                 The best solution would be: on installation time we should know if pdf feature
                 was installed or not!!! (e.g. by writing a bool inside cfg)

                SfxFilterContainer* pFilterContainer = GetFactory().GetFilterContainer();
                if ( pFilterContainer )
                {
                    String aPDFExtension = String::CreateFromAscii( "pdf" );
                    const SfxFilter* pFilter = pFilterContainer->GetFilter4Extension( aPDFExtension, SFX_FILTER_EXPORT );
                    if ( pFilter != NULL )
                        break;
                }

                rSet.DisableItem( nWhich );
                */
                break;
            }

            case SID_DOC_MODIFIED:
            {
                rSet.Put( SfxStringItem( SID_DOC_MODIFIED, IsModified() ? '*' : ' ' ) );
                break;
            }

            case SID_MODIFIED:
            {
                rSet.Put( SfxBoolItem( SID_MODIFIED, IsModified() ) );
                break;
            }

            case SID_DOCINFO_TITLE:
            {
                rSet.Put( SfxStringItem(
                    SID_DOCINFO_TITLE, GetDocInfo().GetTitle() ) );
                break;
            }
            case SID_FILE_NAME:
            {
                if( GetMedium() && HasName() )
                    rSet.Put( SfxStringItem(
                        SID_FILE_NAME, GetMedium()->GetName() ) );
                break;
            }
            case SID_SIGNATURE:
            {
                rSet.Put( SfxUInt16Item( SID_SIGNATURE, GetDocumentSignatureState() ) );
                break;
            }
            case SID_MACRO_SIGNATURE:
            {
                rSet.Put( SfxUInt16Item( SID_MACRO_SIGNATURE, GetScriptingSignatureState() ) );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecProps_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_MODIFIED:
        {
            SetModified( ( (SfxBoolItem&) rReq.GetArgs()->Get(SID_MODIFIED)).GetValue() );
            rReq.Done();
            break;
        }

        case SID_DOCTITLE:
            SetTitle( ( (SfxStringItem&) rReq.GetArgs()->Get(SID_DOCTITLE)).GetValue() );
            rReq.Done();
            break;

        case SID_ON_CREATEDOC:
        case SID_ON_OPENDOC:
        case SID_ON_PREPARECLOSEDOC:
        case SID_ON_CLOSEDOC:
        case SID_ON_SAVEDOC:
        case SID_ON_SAVEASDOC:
        case SID_ON_ACTIVATEDOC:
        case SID_ON_DEACTIVATEDOC:
        case SID_ON_PRINTDOC:
        case SID_ON_SAVEDOCDONE:
        case SID_ON_SAVEASDOCDONE:
            SFX_APP()->EventExec_Impl( rReq, this );
            break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_PLAYMACRO:
        {
            SFX_APP()->PlayMacro_Impl( rReq, GetBasic() );
            break;
        }

        case SID_DOCINFO_AUTHOR :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            SfxStamp aStamp( GetDocInfo().GetCreated() );
            aStamp.SetName( aStr );
            GetDocInfo().SetCreated( aStamp );
            break;
        }

        case SID_DOCINFO_COMMENTS :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            GetDocInfo().SetComment( aStr );
            break;
        }

        case SID_DOCINFO_KEYWORDS :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            GetDocInfo().SetKeywords( aStr );
            break;
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::StateProps_Impl(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    for ( USHORT nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_DOCINFO_AUTHOR :
            {
                String aStr = GetDocInfo().GetCreated().GetName();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCINFO_COMMENTS :
            {
                String aStr = GetDocInfo().GetComment();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCINFO_KEYWORDS :
            {
                String aStr = GetDocInfo().GetKeywords();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCPATH:
            {
                DBG_ERROR( "Not supported anymore!" );
                break;
            }

            case SID_DOCFULLNAME:
            {
                rSet.Put( SfxStringItem( SID_DOCFULLNAME, GetTitle(SFX_TITLE_FULLNAME) ) );
                break;
            }

            case SID_DOCTITLE:
            {
                rSet.Put( SfxStringItem( SID_DOCTITLE, GetTitle() ) );
                break;
            }

            case SID_DOC_READONLY:
            {
                rSet.Put( SfxBoolItem( SID_DOC_READONLY, IsReadOnly() ) );
                break;
            }

            case SID_DOC_SAVED:
            {
                rSet.Put( SfxBoolItem( SID_DOC_SAVED, !IsModified() ) );
                break;
            }

            case SID_CLOSING:
            {
                rSet.Put( SfxBoolItem( SID_CLOSING, Get_Impl()->bInCloseEvent ) );
                break;
            }

            case SID_ON_CREATEDOC:
            case SID_ON_OPENDOC:
            case SID_ON_PREPARECLOSEDOC:
            case SID_ON_CLOSEDOC:
            case SID_ON_SAVEDOC:
            case SID_ON_SAVEASDOC:
            case SID_ON_ACTIVATEDOC:
            case SID_ON_DEACTIVATEDOC:
            case SID_ON_PRINTDOC:
            case SID_ON_SAVEDOCDONE:
            case SID_ON_SAVEASDOCDONE:
                SFX_APP()->EventState_Impl( nSID, rSet, this );
                break;

            case SID_DOC_LOADING:
                rSet.Put( SfxBoolItem( nSID, SFX_LOADED_MAINDOCUMENT !=
                            ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) ) );
                break;

            case SID_IMG_LOADING:
                rSet.Put( SfxBoolItem( nSID, SFX_LOADED_IMAGES !=
                            ( pImp->nLoadedFlags & SFX_LOADED_IMAGES ) ) );
                break;
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecView_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_ACTIVATE:
        {
            SfxViewFrame *pFrame =
                    SfxViewFrame::GetFirst( this, TYPE(SfxTopViewFrame), TRUE );
            if ( pFrame )
                pFrame->GetFrame()->Appear();
            rReq.SetReturnValue( SfxObjectItem( 0, pFrame ) );
            rReq.Done();
            break;
        }
        case SID_NEWWINDOWFOREDIT:
        {
            SfxViewFrame* pFrame = SfxViewFrame::Current();
            if( pFrame->GetObjectShell() == this &&
                ( pFrame->GetFrameType() & SFXFRAME_HASTITLE ) )
                pFrame->ExecuteSlot( rReq );
            else
            {
                String aFileName( GetObjectShell()->GetMedium()->GetName() );
                if ( aFileName.Len() )
                {
                    SfxStringItem aName( SID_FILE_NAME, aFileName );
                    SfxBoolItem aCreateView( SID_OPEN_NEW_VIEW, TRUE );
                    SFX_APP()->GetAppDispatcher_Impl()->Execute(
                        SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aName,
                        &aCreateView, 0L);
                }
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::StateView_Impl(SfxItemSet &rSet)
{
}


sal_uInt16 SfxObjectShell::ImplGetSignatureState( sal_Bool bScriptingContent )
{
    sal_Int16* pState = bScriptingContent ? &pImp->nScriptingSignatureState : &pImp->nDocumentSignatureState;

    if ( *pState == SIGNATURESTATE_UNKNOWN )
    {
        *pState = SIGNATURESTATE_NOSIGNATURES;

        if ( GetMedium() && GetMedium()->GetName().Len() && GetMedium()->GetStorage().is() && IsOwnStorageFormat_Impl( *GetMedium()) )
        {
            try
            {
                uno::Reference< security::XDocumentDigitalSignatures > xD(
                    comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ), uno::UNO_QUERY );

                if ( xD.is() )
                {
                    ::com::sun::star::uno::Sequence< security::DocumentSignaturesInformation > aInfos;
                    if ( bScriptingContent )
                        aInfos = xD->verifyScriptingContentSignatures( GetMedium()->GetLastCommitReadStorage_Impl(),
                                                                        uno::Reference< io::XInputStream >() );
                    else
                        aInfos = xD->verifyDocumentContentSignatures( GetMedium()->GetLastCommitReadStorage_Impl(),
                                                                        uno::Reference< io::XInputStream >() );

                    int nInfos = aInfos.getLength();
                    if( nInfos )
                    {
                        *pState = SIGNATURESTATE_SIGNATURES_OK;
                        for ( int n = 0; n < nInfos; n++ )
                        {
                            if ( !aInfos[n].SignatureIsValid )
                            {
                                *pState = SIGNATURESTATE_SIGNATURES_BROKEN;
                                break; // we know enough
                            }
                        }
                    }
                }
            }
            catch( com::sun::star::uno::Exception& )
            {
            }
        }
    }

    if ( *pState == SIGNATURESTATE_SIGNATURES_OK )
    {
        if ( IsModified() )
            *pState = SIGNATURESTATE_SIGNATURES_INVALID;
    }

    return (sal_uInt16)*pState;
}

void SfxObjectShell::ImplSign( sal_Bool bScriptingContent )
{
    if ( IsModified() || !GetMedium() || !GetMedium()->GetName().Len() )
    {
        if( QueryBox( NULL, SfxResId( RID_XMLSEC_QUERY_SAVEBEFORESIGN ) ).Execute() == RET_YES )
        {
            int nId = SID_SAVEDOC;
            if ( !GetMedium() || !GetMedium()->GetName().Len() )
                nId = SID_SAVEASDOC;
            SfxRequest aSaveRequest( nId, 0, GetPool() );
            ExecFile_Impl( aSaveRequest );
        }
        if ( IsModified() || !GetMedium() || !GetMedium()->GetName().Len() )
            return;

    }

    // Check if it is stored in a OOo format...
    if ( GetMedium() && GetMedium()->GetFilter() && !GetMedium()->GetFilter()->IsOwnFormat() )
    {
        InfoBox( NULL, SfxResId( RID_XMLSEC_INFO_WRONGDOCFORMAT ) ).Execute();
        return;
    }

    // the document is not modified currently, so it can not become modified after signing
    sal_Bool bAllowModifiedBack = sal_False;
    if ( IsEnableSetModified() )
    {
        EnableSetModified( sal_False );
        bAllowModifiedBack = sal_True;
    }

    if ( GetMedium()->SignContents_Impl( bScriptingContent ) )
    {
        if ( bScriptingContent )
            pImp->nScriptingSignatureState = SIGNATURESTATE_UNKNOWN;// Re-Check
        else
            pImp->nDocumentSignatureState = SIGNATURESTATE_UNKNOWN;// Re-Check

        pImp->bSignatureErrorIsShown = sal_False;

        Invalidate( SID_SIGNATURE );
        Invalidate( SID_MACRO_SIGNATURE );
        Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
    }

    if ( bAllowModifiedBack )
        EnableSetModified( sal_True );
}

sal_uInt16 SfxObjectShell::GetDocumentSignatureState()
{
    return ImplGetSignatureState( FALSE );
}

void SfxObjectShell::SignDocumentContent()
{
    ImplSign( FALSE );
}

sal_uInt16 SfxObjectShell::GetScriptingSignatureState()
{
    return ImplGetSignatureState( TRUE );
}

void SfxObjectShell::SignScriptingContent()
{
    ImplSign( TRUE );
}

