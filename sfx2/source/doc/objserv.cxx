/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <sot/storage.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <tools/urlobj.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <comphelper/string.hxx>
#include <basic/sbx.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/miscopt.hxx>
#include <comphelper/documentconstants.hxx>

#include <sfx2/app.hxx>
#include <sfx2/signaturestate.hxx>
#include "sfxresid.hxx"
#include <sfx2/event.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/doctdlg.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include "versdlg.hxx"
#include "doc.hrc"
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include "sfxhelp.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/objface.hxx>

#include "../appl/app.hrc"
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "helpid.hrc"

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

//=========================================================================



SFX_IMPL_INTERFACE(SfxObjectShell,SfxShell,SfxResId(0))
{
}

//=========================================================================

class SfxClosePreventer_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
{
    sal_Bool m_bGotOwnership;
    sal_Bool m_bPreventClose;

public:
    SfxClosePreventer_Impl();

    sal_Bool HasOwnership() { return m_bGotOwnership; }

    void SetPreventClose( sal_Bool bPrevent ) { m_bPreventClose = bPrevent; }

    virtual void SAL_CALL queryClosing( const lang::EventObject& aEvent, sal_Bool bDeliverOwnership )
        throw ( uno::RuntimeException, util::CloseVetoException );

    virtual void SAL_CALL notifyClosing( const lang::EventObject& aEvent ) throw ( uno::RuntimeException ) ;

    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw ( uno::RuntimeException ) ;

} ;

SfxClosePreventer_Impl::SfxClosePreventer_Impl()
: m_bGotOwnership( sal_False )
, m_bPreventClose( sal_True )
{
}

void SAL_CALL SfxClosePreventer_Impl::queryClosing( const lang::EventObject&, sal_Bool bDeliverOwnership )
        throw ( uno::RuntimeException, util::CloseVetoException )
{
    if ( m_bPreventClose )
    {
        if ( !m_bGotOwnership )
            m_bGotOwnership = bDeliverOwnership;

        throw util::CloseVetoException();
    }
}

void SAL_CALL SfxClosePreventer_Impl::notifyClosing( const lang::EventObject& ) throw ( uno::RuntimeException )
{}

void SAL_CALL SfxClosePreventer_Impl::disposing( const lang::EventObject& ) throw ( uno::RuntimeException )
{}

//=========================================================================
class SfxInstanceCloseGuard_Impl
{
    SfxClosePreventer_Impl* m_pPreventer;
    uno::Reference< util::XCloseListener > m_xPreventer;
    uno::Reference< util::XCloseable > m_xCloseable;

public:
    SfxInstanceCloseGuard_Impl()
    : m_pPreventer( NULL )
    {}

    ~SfxInstanceCloseGuard_Impl();

    sal_Bool Init_Impl( const uno::Reference< util::XCloseable >& xCloseable );
};

sal_Bool SfxInstanceCloseGuard_Impl::Init_Impl( const uno::Reference< util::XCloseable >& xCloseable )
{
    sal_Bool bResult = sal_False;

    // do not allow reinit after the successful init
    if ( xCloseable.is() && !m_xCloseable.is() )
    {
        try
        {
            m_pPreventer = new SfxClosePreventer_Impl();
            m_xPreventer = uno::Reference< util::XCloseListener >( m_pPreventer );
            xCloseable->addCloseListener( m_xPreventer );
            m_xCloseable = xCloseable;
            bResult = sal_True;
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Could not register close listener!\n" );
        }
    }

    return bResult;
}

SfxInstanceCloseGuard_Impl::~SfxInstanceCloseGuard_Impl()
{
    if ( m_xCloseable.is() && m_xPreventer.is() )
    {
        try
        {
            m_xCloseable->removeCloseListener( m_xPreventer );
        }
        catch( uno::Exception& )
        {
        }

        try
        {
            if ( m_pPreventer )
            {
                m_pPreventer->SetPreventClose( sal_False );

                if ( m_pPreventer->HasOwnership() )
                    m_xCloseable->close( sal_True ); // TODO: do it asynchronously
            }
        }
        catch( uno::Exception& )
        {
        }
    }
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
    bool bPrinting = false;
    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
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

            // use the title that is provided in the media descriptor
            SFX_ITEMSET_ARG( aParams, pDocTitleItem, SfxStringItem, SID_DOCINFO_TITLE, sal_False );
            if ( pDocTitleItem )
                getDocProperties()->setTitle( pDocTitleItem->GetValue() );

            bOk = CommonSaveAs_Impl( INetURLObject(aFileName), aFilterName,
                aParams );

        }

        // prevent picklist-entry
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

    // this guard is created here to have it destruction at the end of the method
    SfxInstanceCloseGuard_Impl aModelGuard;

    sal_Bool bIsPDFExport = sal_False;
    switch(nId)
    {
        case SID_VERSION:
        {
            SfxViewFrame* pFrame = GetFrame();
            if ( !pFrame )
                pFrame = SfxViewFrame::GetFirst( this );
            if ( !pFrame )
                return;

            if ( pFrame->GetFrame().GetParentFrame() )
            {
                pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                return;
            }

            if ( !IsOwnStorageFormat_Impl( *GetMedium() ) )
                return;

            SfxVersionDialog *pDlg = new SfxVersionDialog( pFrame, IsSaveVersionOnClose() );
            pDlg->Execute();
            SetSaveVersionOnClose( pDlg->IsSaveVersionOnClose() );
            delete pDlg;
            rReq.Done();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCINFO:
        {
            SFX_REQUEST_ARG(rReq, pDocInfItem, SfxDocumentInfoItem, SID_DOCINFO, FALSE);
            if ( pDocInfItem )
            {
                // parameter, e.g. from replayed macro
                pDocInfItem->UpdateDocumentInfo(getDocProperties(), true);
                SetUseUserData( pDocInfItem->IsUseUserData() );
            }
            else
            {
                // no argument containing DocInfo; check optional arguments
                BOOL bReadOnly = IsReadOnly();
                SFX_REQUEST_ARG(rReq, pROItem, SfxBoolItem, SID_DOC_READONLY, FALSE);
                if ( pROItem )
                    // override readonly attribute of document
                    // e.g. if a readonly document is saved elsewhere and user asks for editing DocInfo before
                    bReadOnly = pROItem->GetValue();

                // collect data for dialog
                String aURL, aTitle;
                if ( HasName() )
                {
                    aURL = GetMedium()->GetName();
                    aTitle = GetTitle();
                }
                else
                {
                    aURL = DEFINE_CONST_UNICODE( "private:factory/" );
                    aURL += String::CreateFromAscii( GetFactory().GetShortName() );

                    aTitle = GetTitle();
                }

                SfxDocumentInfoItem aDocInfoItem( aURL, getDocProperties(),
                    IsUseUserData() );
                if ( !GetSlotState( SID_DOCTEMPLATE ) )
                    // templates not supported
                    aDocInfoItem.SetTemplate(FALSE);

                SfxItemSet aSet(GetPool(), SID_DOCINFO, SID_DOCINFO, SID_DOC_READONLY, SID_DOC_READONLY,
                                SID_EXPLORER_PROPS_START, SID_EXPLORER_PROPS_START, SID_BASEURL, SID_BASEURL,
                                0L );
                aSet.Put( aDocInfoItem );
                aSet.Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                aSet.Put( SfxStringItem( SID_EXPLORER_PROPS_START, aTitle ) );
                aSet.Put( SfxStringItem( SID_BASEURL, GetMedium()->GetBaseURL() ) );

                // creating dialog is done via virtual method; application will add its own statistics page
                SfxDocumentInfoDialog *pDlg = CreateDocumentInfoDialog(0, aSet);
                if ( RET_OK == pDlg->Execute() )
                {
                    SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pDocInfoItem, SfxDocumentInfoItem, SID_DOCINFO, FALSE);
                    if ( pDocInfoItem )
                    {
                        // user has done some changes to DocumentInfo
                        pDocInfoItem->UpdateDocumentInfo(getDocProperties());
                        SetUseUserData( ((const SfxDocumentInfoItem *)pDocInfoItem)->IsUseUserData() );

                        // add data from dialog for possible recording purposes
                        rReq.AppendItem( SfxDocumentInfoItem( GetTitle(),
                            getDocProperties(), IsUseUserData() ) );
                    }

                    rReq.Done();
                }
                else
                    // nothing done; no recording
                    rReq.Ignore();

                delete pDlg;
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
            // derived class may decide to abort this
            if( !QuerySlotExecutable( nId ) )
            {
                rReq.SetReturnValue( SfxBoolItem( 0, FALSE ) );
                return;
            }

            //!! detaillierte Auswertung eines Fehlercodes
            SfxObjectShellRef xLock( this );

            // the model can not be closed till the end of this method
            // if somebody tries to close it during this time the model will be closed
            // at the end of the method
            aModelGuard.Init_Impl( uno::Reference< util::XCloseable >( GetModel(), uno::UNO_QUERY ) );

            sal_Bool bDialogUsed = sal_False;
            sal_uInt32 nErrorCode = ERRCODE_NONE;

            // by default versions should be preserved always except in case of an explicit
            // SaveAs via GUI, so the flag must be set accordingly
            pImp->bPreserveVersions = (nId == SID_SAVEDOC);
            try
            {
                SfxErrorContext aEc( ERRCTX_SFX_SAVEASDOC, GetTitle() ); // ???

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
                    if ( pFrame )
                    {
                        uno::Reference< task::XStatusIndicatorFactory > xStatFactory(
                                                                    pFrame->GetFrame().GetFrameInterface(),
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

                sal_Bool bPreselectPassword = sal_False;
                SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pOldPasswordItem, SfxStringItem, SID_PASSWORD, FALSE );
                if ( pOldPasswordItem )
                    bPreselectPassword = sal_True;

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
                {
                    bDialogUsed = aHelper.GUIStoreModel( GetModel(),
                                                         ::rtl::OUString::createFromAscii( pSlot->GetUnoName() ),
                                                         aDispatchArgs,
                                                         bPreselectPassword,
                                                         GetSharedFileURL(),
                                                         GetDocumentSignatureState() );
                }
                else
                {
                    // the user has decided not to store the document
                    throw task::ErrorCodeIOException( ::rtl::OUString(),
                                                      uno::Reference< uno::XInterface >(),
                                                      ERRCODE_IO_ABORT );
                }

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
                    if ( pDocViewFrame )
                        SfxHelp::OpenHelpAgent( &pDocViewFrame->GetFrame(), HID_DID_SAVE_PACKED_XML );
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
            if ( pFrame && pFrame->GetFrame().GetParentFrame() )
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
                if ( pFrame->GetFrame().GetParentFrame() )
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
                    if ( !pFrame->GetFrame().GetParentFrame() )
                        pFrame->GetFrame().DoClose();
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

            if ( !rReq.GetArgs() )
            {
                pDlg = new SfxDocumentTemplateDlg(0, pTemplates);
                if ( RET_OK == pDlg->Execute() && pDlg->GetTemplateName().Len())
                {
                    rReq.AppendItem(SfxStringItem(
                        SID_TEMPLATE_NAME, pDlg->GetTemplateName()));
                    rReq.AppendItem(SfxStringItem(
                        SID_TEMPLATE_REGIONNAME, pDlg->GetRegionName()));
                }
                else
                {
                    delete pDlg;
                    rReq.Ignore();
                    return;
                }
            }

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

            ::rtl::OUString aTemplateName = pNameItem->GetValue();
            ::rtl::OUString aTemplateGroup;
            if ( pRegionItem )
                aTemplateGroup = pRegionItem->GetValue();
            else
                // pRegionNrItem must not be NULL, it was just checked
                aTemplateGroup = pTemplates->GetFullRegionName( pRegionNrItem->GetValue() );
            // check Group and Name
            delete pTemplates;

            sal_Bool bOk = sal_False;
            try
            {
                uno::Reference< frame::XStorable > xStorable( GetModel(), uno::UNO_QUERY_THROW );
                ::rtl::OUString aService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DocumentTemplates" ) );
                uno::Reference< frame::XDocumentTemplates > xTemplates(
                                comphelper::getProcessServiceFactory()->createInstance( aService ),
                                uno::UNO_QUERY_THROW );

                bOk = xTemplates->storeTemplate( aTemplateGroup, aTemplateName, xStorable );
            }
            catch( uno::Exception& )
            {
            }

            DELETEX(pDlg);

            rReq.SetReturnValue( SfxBoolItem( 0, bOk ) );
            if ( bOk )
            {
                // update the Organizer runtime cache from the template component if the cache has already been created
                // TODO/LATER: get rid of this cache duplication
                SfxDocumentTemplates aTemplates;
                aTemplates.ReInitFromComponent();
            }
            else
            {
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                return;
            }

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

//-------------------------------------------------------------------------

void SfxObjectShell::GetState_Impl(SfxItemSet &rSet)
{
    DBG_CHKTHIS(SfxObjectShell, 0);
    SfxWhichIter aIter( rSet );

    for ( USHORT nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
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
                        if ( pFrame->GetFrame().GetParentFrame() )
                        {
                            pFrame = pFrame->GetTopViewFrame();
                            pDoc = pFrame->GetObjectShell();
                        }
                    }

                    if ( !pFrame || !pDoc->HasName() ||
                        !IsOwnStorageFormat_Impl( *pDoc->GetMedium() ) )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_SAVEDOC:
                {
                    SvtMiscOptions aMiscOptions;
                    bool bAlwaysAllowSave = aMiscOptions.IsSaveAlwaysAllowed();
                    bool bAllowSave = (bAlwaysAllowSave || IsModified());
                    bool bMediumRO = IsReadOnlyMedium();
                    if ( !bMediumRO && GetMedium() && bAllowSave )
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
                if ( pFrame && pFrame->GetFrame().GetParentFrame() )
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
                if ( /*!pCombinedFilters ||*/ !GetMedium() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxStringItem( nWhich, String( SfxResId( STR_SAVEASDOC ) ) ) );
                break;
            }

            case SID_EXPORTDOCASPDF:
            case SID_DIRECTEXPORTDOCASPDF:
            {
                break;
            }

            case SID_DOC_MODIFIED:
            {
                rSet.Put( SfxBoolItem( SID_DOC_MODIFIED, IsModified() ) );
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
                    SID_DOCINFO_TITLE, getDocProperties()->getTitle() ) );
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
                // the slot makes sense only if there is a macro in the document
                if ( pImp->documentStorageHasMacros() || pImp->aMacroMode.hasMacroLibrary() )
                    rSet.Put( SfxUInt16Item( SID_MACRO_SIGNATURE, GetScriptingSignatureState() ) );
                else
                    rSet.DisableItem( nWhich );
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

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_PLAYMACRO:
        {
            SFX_APP()->PlayMacro_Impl( rReq, GetBasic() );
            break;
        }

        case SID_DOCINFO_AUTHOR :
        {
            ::rtl::OUString aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            getDocProperties()->setAuthor( aStr );
            break;
        }

        case SID_DOCINFO_COMMENTS :
        {
            ::rtl::OUString aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            getDocProperties()->setDescription( aStr );
            break;
        }

        case SID_DOCINFO_KEYWORDS :
        {
            ::rtl::OUString aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            getDocProperties()->setKeywords(
                ::comphelper::string::convertCommaSeparated(aStr) );
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
                rSet.Put( SfxStringItem( nSID,
                            getDocProperties()->getAuthor() ) );
                break;
            }

            case SID_DOCINFO_COMMENTS :
            {
                rSet.Put( SfxStringItem( nSID,
                            getDocProperties()->getDescription()) );
                break;
            }

            case SID_DOCINFO_KEYWORDS :
            {
                rSet.Put( SfxStringItem( nSID, ::comphelper::string::
                    convertCommaSeparated(getDocProperties()->getKeywords())) );
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
                rSet.Put( SfxBoolItem( SID_CLOSING, false ) );
                break;
            }

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
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this, TRUE );
            if ( pFrame )
                pFrame->GetFrame().Appear();
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

void SfxObjectShell::StateView_Impl(SfxItemSet& /*rSet*/)
{
}

sal_uInt16 SfxObjectShell::ImplCheckSignaturesInformation( const uno::Sequence< security::DocumentSignatureInformation >& aInfos )
{
    sal_Bool bCertValid = sal_True;
    sal_uInt16 nResult = SIGNATURESTATE_NOSIGNATURES;
    int nInfos = aInfos.getLength();
    bool bCompleteSignature = true;
    if( nInfos )
    {
        nResult = SIGNATURESTATE_SIGNATURES_OK;
        for ( int n = 0; n < nInfos; n++ )
        {
            if ( bCertValid )
            {
                sal_Int32 nCertStat = aInfos[n].CertificateStatus;
                bCertValid = nCertStat == security::CertificateValidity::VALID ? sal_True : sal_False;
            }

            if ( !aInfos[n].SignatureIsValid )
            {
                nResult = SIGNATURESTATE_SIGNATURES_BROKEN;
                break; // we know enough
            }
            bCompleteSignature &= !aInfos[n].PartialDocumentSignature;
        }
    }

    if ( nResult == SIGNATURESTATE_SIGNATURES_OK && !bCertValid )
        nResult = SIGNATURESTATE_SIGNATURES_NOTVALIDATED;
    else if ( nResult == SIGNATURESTATE_SIGNATURES_OK && bCertValid && !bCompleteSignature)
        nResult = SIGNATURESTATE_SIGNATURES_PARTIAL_OK;

    // this code must not check whether the document is modified
    // it should only check the provided info

    return nResult;
}

uno::Sequence< security::DocumentSignatureInformation > SfxObjectShell::ImplAnalyzeSignature( sal_Bool bScriptingContent, const uno::Reference< security::XDocumentDigitalSignatures >& xSigner )
{
    uno::Sequence< security::DocumentSignatureInformation > aResult;
    uno::Reference< security::XDocumentDigitalSignatures > xLocSigner = xSigner;

    if ( GetMedium() && GetMedium()->GetName().Len() && IsOwnStorageFormat_Impl( *GetMedium())  && GetMedium()->GetStorage().is() )
    {
        try
        {
            if ( !xLocSigner.is() )
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[0] <<= ::rtl::OUString();
                try
                {
                    uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
                    aArgs[0] = xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ) );
                }
                catch( uno::Exception& )
                {
                }

                xLocSigner.set( comphelper::getProcessServiceFactory()->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ), aArgs ), uno::UNO_QUERY_THROW );

            }

            if ( bScriptingContent )
                aResult = xLocSigner->verifyScriptingContentSignatures( GetMedium()->GetZipStorageToSign_Impl(),
                                                                uno::Reference< io::XInputStream >() );
            else
                aResult = xLocSigner->verifyDocumentContentSignatures( GetMedium()->GetZipStorageToSign_Impl(),
                                                                uno::Reference< io::XInputStream >() );
        }
        catch( com::sun::star::uno::Exception& )
        {
        }
    }

    return aResult;
}

sal_uInt16 SfxObjectShell::ImplGetSignatureState( sal_Bool bScriptingContent )
{
    sal_Int16* pState = bScriptingContent ? &pImp->nScriptingSignatureState : &pImp->nDocumentSignatureState;

    if ( *pState == SIGNATURESTATE_UNKNOWN )
    {
        *pState = SIGNATURESTATE_NOSIGNATURES;

        uno::Sequence< security::DocumentSignatureInformation > aInfos = ImplAnalyzeSignature( bScriptingContent );
        *pState = ImplCheckSignaturesInformation( aInfos );
    }

    if ( *pState == SIGNATURESTATE_SIGNATURES_OK || *pState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED
        || *pState == SIGNATURESTATE_SIGNATURES_PARTIAL_OK)
    {
        if ( IsModified() )
            *pState = SIGNATURESTATE_SIGNATURES_INVALID;
    }

    return (sal_uInt16)*pState;
}

void SfxObjectShell::ImplSign( sal_Bool bScriptingContent )
{
    // Check if it is stored in OASIS format...
    if  (   GetMedium()
        &&  GetMedium()->GetFilter()
        &&  GetMedium()->GetName().Len()
        &&  (   !GetMedium()->GetFilter()->IsOwnFormat()
            ||  !GetMedium()->HasStorage_Impl()
            )
        )
    {
        // Only OASIS and OOo6.x formats will be handled further
        InfoBox( NULL, SfxResId( RID_XMLSEC_INFO_WRONGDOCFORMAT ) ).Execute();
        return;
    }

    // check whether the document is signed
    ImplGetSignatureState( sal_False ); // document signature
    ImplGetSignatureState( sal_True ); // script signature
    sal_Bool bHasSign = ( pImp->nScriptingSignatureState != SIGNATURESTATE_NOSIGNATURES || pImp->nDocumentSignatureState != SIGNATURESTATE_NOSIGNATURES );

    // the target ODF version on saving
    SvtSaveOptions aSaveOpt;
    SvtSaveOptions::ODFDefaultVersion nVersion = aSaveOpt.GetODFDefaultVersion();

    // the document is not new and is not modified
    ::rtl::OUString aODFVersion;
    try
    {
        // check the version of the document
        uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
        xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ) ) >>= aODFVersion;
    }
    catch( uno::Exception& )
    {}

    bool bNoSig = false;

    if ( IsModified() || !GetMedium() || !GetMedium()->GetName().Len()
      || (!aODFVersion.equals( ODFVER_012_TEXT ) && !bHasSign) )
    {
        // the document might need saving ( new, modified or in ODF1.1 format without signature )

        if ( nVersion >= SvtSaveOptions::ODFVER_012 )
        {

            if ( (bHasSign && QueryBox( NULL, SfxResId( MSG_XMLSEC_QUERY_SAVESIGNEDBEFORESIGN ) ).Execute() == RET_YES)
              || (!bHasSign && QueryBox( NULL, SfxResId( RID_XMLSEC_QUERY_SAVEBEFORESIGN ) ).Execute() == RET_YES) )
            {
                USHORT nId = SID_SAVEDOC;
                if ( !GetMedium() || !GetMedium()->GetName().Len() )
                    nId = SID_SAVEASDOC;
                SfxRequest aSaveRequest( nId, 0, GetPool() );
                //ToDo: Review. We needed to call SetModified, otherwise the document would not be saved.
                SetModified(sal_True);
                ExecFile_Impl( aSaveRequest );

                // Check if it is stored in OASIS format...
                if ( GetMedium() && GetMedium()->GetFilter()
                  && ( !GetMedium()->GetFilter()->IsOwnFormat() || !GetMedium()->HasStorage_Impl()
                    || SotStorage::GetVersion( GetMedium()->GetStorage() ) <= SOFFICE_FILEFORMAT_60 ) )
                {
                    // Only OASIS format will be handled further
                    InfoBox( NULL, SfxResId( RID_XMLSEC_INFO_WRONGDOCFORMAT ) ).Execute();
                    return;
                }
            }
            else
            {
                //When the document is modified then we must not show the digital signatures dialog
                //If we have come here then the user denied to save.
                if (!bHasSign)
                    bNoSig = true;
            }
        }
        else
        {
            ErrorBox( NULL, WB_OK, SfxResId( STR_XMLSEC_ODF12_EXPECTED ) ).Execute();
            return;
        }

        if ( IsModified() || !GetMedium() || !GetMedium()->GetName().Len() )
            return;
    }

    // the document is not modified currently, so it can not become modified after signing
    sal_Bool bAllowModifiedBack = sal_False;
    if ( IsEnableSetModified() )
    {
        EnableSetModified( sal_False );
        bAllowModifiedBack = sal_True;
    }

    // we have to store to the original document, the original medium should be closed for this time
    if ( !bNoSig
      && ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
    {
        GetMedium()->CloseAndRelease();

        // We sign only ODF1.2, that means that if this point has been reached,
        // the ODF1.2 signing process should be used.
        // This code still might be called to show the signature of ODF1.1 document.
        sal_Bool bSigned = GetMedium()->SignContents_Impl(
            bScriptingContent,
            aODFVersion,
            pImp->nDocumentSignatureState == SIGNATURESTATE_SIGNATURES_OK
            || pImp->nDocumentSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED
            || pImp->nDocumentSignatureState == SIGNATURESTATE_SIGNATURES_PARTIAL_OK);

        DoSaveCompleted( GetMedium() );

        if ( bSigned )
        {
            if ( bScriptingContent )
            {
                pImp->nScriptingSignatureState = SIGNATURESTATE_UNKNOWN;// Re-Check

                // adding of scripting signature removes existing document signature
                pImp->nDocumentSignatureState = SIGNATURESTATE_UNKNOWN;// Re-Check
            }
            else
                pImp->nDocumentSignatureState = SIGNATURESTATE_UNKNOWN;// Re-Check

            pImp->bSignatureErrorIsShown = sal_False;

            Invalidate( SID_SIGNATURE );
            Invalidate( SID_MACRO_SIGNATURE );
            Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
        }
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

// static
const uno::Sequence<sal_Int8>& SfxObjectShell::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
