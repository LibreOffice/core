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
#include <com/sun/star/document/XCmisDocument.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <tools/urlobj.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/visitem.hxx>
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
#include "sfx2/sfxresid.hxx"
#include <sfx2/event.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
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
#include "sfx2/sfxhelp.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/checkin.hxx>

#include "../appl/app.hrc"
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include "helpid.hrc"

#include "guisaveas.hxx"
#include "templatedlg.hxx"

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
    OUString&     _rNewNameVar;
    OUString      _aNewName;

public:
                SfxSaveAsContext_Impl( OUString &rNewNameVar,
                                       const OUString &rNewName )
                :   _rNewNameVar( rNewNameVar ),
                    _aNewName( rNewName )
                { rNewNameVar = rNewName; }
                ~SfxSaveAsContext_Impl()
                { _rNewNameVar = ""; }
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
            OSL_FAIL( "Could not register close listener!\n" );
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
    const OUString& aFileName,
    SfxItemSet*   aParams
)
{
    sal_Bool bOk = sal_False;

    {DBG_CHKTHIS(SfxObjectShell, 0);}

    if ( GetMedium() )
    {
        OUString aFilterName;
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
                const SfxFilter* pFilter = SfxFilterMatcher( OUString::createFromAscii(GetFactory().GetShortName()) ).GetFilter4Mime( pContentTypeItem->GetValue(), SFX_FILTER_EXPORT );
                if ( pFilter )
                    aFilterName = pFilter->GetName();
            }
        }

        // in case no filter defined use default one
        if( aFilterName.isEmpty() )
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
    }

    return bOk;
}

void SfxObjectShell::CheckOut( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        xCmisDoc->checkOut( );

        // Remove the info bar
        SfxViewFrame* pViewFrame = GetFrame();
        pViewFrame->RemoveInfoBar( "checkout" );
    }
    catch ( const uno::RuntimeException& e )
    {
        ErrorBox* pErrorBox = new ErrorBox( &GetFrame()->GetWindow(), WB_OK, e.Message );
        pErrorBox->Execute( );
        delete pErrorBox;
    }
}

void SfxObjectShell::CancelCheckOut( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        xCmisDoc->cancelCheckOut( );

        uno::Reference< util::XModifiable > xModifiable( GetModel( ), uno::UNO_QUERY );
        if ( xModifiable.is( ) )
            xModifiable->setModified( sal_False );
    }
    catch ( const uno::RuntimeException& e )
    {
        ErrorBox* pErrorBox = new ErrorBox( &GetFrame()->GetWindow(), WB_OK, e.Message );
        pErrorBox->Execute( );
        delete pErrorBox;
    }
}

void SfxObjectShell::CheckIn( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        // Pop up dialog to ask for comment and major
        SfxCheckinDialog checkinDlg( &GetFrame( )->GetWindow( ) );
        if ( checkinDlg.Execute( ) == RET_OK )
        {
            OUString sComment = checkinDlg.GetComment( );
            sal_Bool bMajor = checkinDlg.IsMajor( );
            xCmisDoc->checkIn( bMajor, sComment );
            uno::Reference< util::XModifiable > xModifiable( GetModel( ), uno::UNO_QUERY );
            if ( xModifiable.is( ) )
                xModifiable->setModified( sal_False );
        }
    }
    catch ( const uno::RuntimeException& e )
    {
        ErrorBox* pErrorBox = new ErrorBox( &GetFrame()->GetWindow(), WB_OK, e.Message );
        pErrorBox->Execute( );
        delete pErrorBox;
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecFile_Impl(SfxRequest &rReq)
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}

    sal_uInt16 nId = rReq.GetSlot();

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
            SFX_REQUEST_ARG(rReq, pDocInfItem, SfxDocumentInfoItem, SID_DOCINFO, sal_False);
            if ( pDocInfItem )
            {
                // parameter, e.g. from replayed macro
                pDocInfItem->UpdateDocumentInfo(getDocProperties(), true);
                SetUseUserData( pDocInfItem->IsUseUserData() );
            }
            else
            {
                // no argument containing DocInfo; check optional arguments
                sal_Bool bReadOnly = IsReadOnly();
                SFX_REQUEST_ARG(rReq, pROItem, SfxBoolItem, SID_DOC_READONLY, sal_False);
                if ( pROItem )
                    // override readonly attribute of document
                    // e.g. if a readonly document is saved elsewhere and user asks for editing DocInfo before
                    bReadOnly = pROItem->GetValue();

                // collect data for dialog
                OUString aURL, aTitle;
                if ( HasName() )
                {
                    aURL = GetMedium()->GetName();
                    aTitle = GetTitle();
                }
                else
                {
                    aURL = "private:factory/" + OUString::createFromAscii( GetFactory().GetShortName() );

                    aTitle = GetTitle();
                }

                Reference< XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY );
                uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties();

                SfxDocumentInfoItem aDocInfoItem( aURL, getDocProperties(), aCmisProperties,
                    IsUseUserData() );
                if ( !GetSlotState( SID_DOCTEMPLATE ) )
                    // templates not supported
                    aDocInfoItem.SetTemplate(sal_False);

                SfxItemSet aSet(GetPool(), SID_DOCINFO, SID_DOCINFO, SID_DOC_READONLY, SID_DOC_READONLY,
                                SID_EXPLORER_PROPS_START, SID_EXPLORER_PROPS_START, SID_BASEURL, SID_BASEURL,
                                0L );
                aSet.Put( aDocInfoItem );
                aSet.Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                aSet.Put( SfxStringItem( SID_EXPLORER_PROPS_START, aTitle ) );
                aSet.Put( SfxStringItem( SID_BASEURL, GetMedium()->GetBaseURL() ) );

                // creating dialog is done via virtual method; application will
                // add its own statistics page
                SfxDocumentInfoDialog *pDlg = CreateDocumentInfoDialog(0, aSet);
                if ( RET_OK == pDlg->Execute() )
                {
                    SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pDocInfoItem, SfxDocumentInfoItem, SID_DOCINFO, sal_False);
                    if ( pDocInfoItem )
                    {
                        // user has done some changes to DocumentInfo
                        pDocInfoItem->UpdateDocumentInfo(getDocProperties());
                        uno::Sequence< document::CmisProperty > aNewCmisProperties =
                            pDocInfoItem->GetCmisProperties( );
                        if ( aNewCmisProperties.getLength( ) > 0 )
                            xCmisDoc->updateCmisProperties( aNewCmisProperties );
                        SetUseUserData( ((const SfxDocumentInfoItem *)pDocInfoItem)->IsUseUserData() );
                        // add data from dialog for possible recording purpose
                        rReq.AppendItem( SfxDocumentInfoItem( GetTitle(),
                            getDocProperties(), aNewCmisProperties, IsUseUserData() ) );
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
                rReq.SetReturnValue( SfxBoolItem( 0, sal_False ) );
                return;
            }

            //!! detailed analysis of an error code
            SfxObjectShellRef xLock( this );

            // the model can not be closed till the end of this method
            // if somebody tries to close it during this time the model will be closed
            // at the end of the method
            aModelGuard.Init_Impl( uno::Reference< util::XCloseable >( GetModel(), uno::UNO_QUERY ) );

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
                    SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pViewOnlyItem, SfxBoolItem, SID_VIEWONLY, sal_False );
                    if ( pViewOnlyItem && pViewOnlyItem->GetValue() )
                        rReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );
                }

                // TODO/LATER: do the following GUI related actions in standalown method
                // ========================================================================================================
                // Introduce a status indicator for GUI operation
                SFX_REQUEST_ARG( rReq, pStatusIndicatorItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, sal_False );
                if ( !pStatusIndicatorItem )
                {
                    // get statusindicator
                    uno::Reference< task::XStatusIndicator > xStatusIndicator;
                    uno::Reference < frame::XController > xCtrl( GetModel()->getCurrentController() );
                    if ( xCtrl.is() )
                    {
                        uno::Reference< task::XStatusIndicatorFactory > xStatFactory( xCtrl->getFrame(), uno::UNO_QUERY );
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
                SFX_REQUEST_ARG( rReq, pInteractionHandlerItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False );
                if ( !pInteractionHandlerItem )
                {
                    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                    uno::Reference< task::XInteractionHandler2 > xInteract(
                        task::InteractionHandler::createWithParent(xContext, 0) );

                    SfxUnoAnyItem aInteractionItem( SID_INTERACTIONHANDLER, uno::makeAny( xInteract ) );
                    if ( nId == SID_SAVEDOC )
                    {
                        // in case of saving it is not possible to transport the parameters from here
                        // but it is not clear here whether the saving will be done or saveAs operation
                        GetMedium()->GetItemSet()->Put( aInteractionItem );
                    }

                    rReq.AppendItem( aInteractionItem );
                }
                else if ( nId == SID_SAVEDOC )
                {
                    // in case of saving it is not possible to transport the parameters from here
                    // but it is not clear here whether the saving will be done or saveAs operation
                    GetMedium()->GetItemSet()->Put( *pInteractionHandlerItem );
                }
                // ========================================================================================================

                sal_Bool bPreselectPassword = sal_False;
                SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pOldEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False );
                SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pOldPasswordItem, SfxStringItem, SID_PASSWORD, sal_False );
                if ( pOldEncryptionDataItem || pOldPasswordItem )
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

                SfxStoringHelper aHelper;

                if ( QueryHiddenInformation( bIsPDFExport ? WhenCreatingPDF : WhenSaving, NULL ) == RET_YES )
                {
                    aHelper.GUIStoreModel( GetModel(),
                                                         OUString::createFromAscii( pSlot->GetUnoName() ),
                                                         aDispatchArgs,
                                                         bPreselectPassword,
                                                         GetSharedFileURL(),
                                                         GetDocumentSignatureState() );
                }
                else
                {
                    // the user has decided not to store the document
                    throw task::ErrorCodeIOException( OUString(),
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

                // the StoreAsURL/StoreToURL method have called this method with false
                // so it has to be restored to true here since it is a call from GUI
                GetMedium()->SetUpdatePickList( sal_True );

                // TODO: in future it must be done in followind way
                // if document is opened from GUI it is immediatelly appears in the picklist
                // if the document is a new one then it appears in the picklist immediatelly
                // after SaveAs operation triggered from GUI
            }
            catch( const task::ErrorCodeIOException& aErrorEx )
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
            sal_uIntPtr lErr=GetErrorCode();

            if ( !lErr && nErrorCode )
                lErr = nErrorCode;

            if ( lErr && nErrorCode == ERRCODE_NONE )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, sal_False );
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

            if ( nId == SID_SAVEASDOC  && nErrorCode == ERRCODE_NONE )
            {
                SetReadOnlyUI(false);
            }

            rReq.SetReturnValue( SfxBoolItem(0, nErrorCode == ERRCODE_NONE ) );

            ResetError();

            Invalidate();
            break;
        }

        case SID_SAVEACOPY:
        {
            SfxAllItemSet aArgs( GetPool() );
            aArgs.Put( SfxBoolItem( SID_SAVEACOPYITEM, sal_True ) );
            SfxRequest aSaveACopyReq( SID_EXPORTDOC, SFX_CALLMODE_API, aArgs );
            ExecFile_Impl( aSaveACopyReq );
            if ( !aSaveACopyReq.IsDone() )
            {
                rReq.Ignore();
                return;
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_CLOSEDOC:
        {
            SfxViewFrame *pFrame = GetFrame();
            if ( pFrame && pFrame->GetFrame().GetParentFrame() )
            {
                // If SID_CLOSEDOC is excecuted through menu and so on, but
                // the current document is in a frame, then the
                // FrameSetDocument should actually be closed.
                pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                rReq.Done();
                return;
            }

            sal_Bool bInFrameSet = sal_False;
            sal_uInt16 nFrames=0;
            pFrame = SfxViewFrame::GetFirst( this );
            while ( pFrame )
            {
                if ( pFrame->GetFrame().GetParentFrame() )
                {
                    // In this document there still exists a view that is
                    // in a FrameSet , which of course may not be closed
                    // geclosed werden
                    bInFrameSet = sal_True;
                }
                else
                    nFrames++;

                pFrame = SfxViewFrame::GetNext( *pFrame, this );
            }

            if ( bInFrameSet )
            {
                // Close all views that are not in a FrameSet.
                pFrame = SfxViewFrame::GetFirst( this );
                while ( pFrame )
                {
                    if ( !pFrame->GetFrame().GetParentFrame() )
                        pFrame->GetFrame().DoClose();
                    pFrame = SfxViewFrame::GetNext( *pFrame, this );
                }
            }

            // Evaluate Parameter
            SFX_REQUEST_ARG(rReq, pSaveItem, SfxBoolItem, SID_CLOSEDOC_SAVE, sal_False);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_CLOSEDOC_FILENAME, sal_False);
            if ( pSaveItem )
            {
                if ( pSaveItem->GetValue() )
                {
                    if ( !pNameItem )
                    {
#ifndef DISABLE_SCRIPTING
                        SbxBase::SetError( SbxERR_WRONG_ARGS );
#endif
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
                    SetModified(sal_False);
            }

            // Cancelled by the user?
            if ( !PrepareClose( 2 ) )
            {
                rReq.SetReturnValue( SfxBoolItem(0, sal_False) );
                rReq.Done();
                return;
            }

            SetModified( sal_False );
            sal_uIntPtr lErr = GetErrorCode();
            ErrorHandler::HandleError(lErr);

            rReq.SetReturnValue( SfxBoolItem(0, sal_True) );
            rReq.Done();
            rReq.ReleaseArgs(); // because the pool is destroyed in Close
            DoClose();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCTEMPLATE:
        {
            // save as document templates
            SfxTemplateManagerDlg aDlg;
            aDlg.setDocumentModel(GetModel());
            aDlg.setSaveMode(true);
            aDlg.Execute();

            break;
        }

        case SID_CHECKOUT:
        {
            CheckOut( );
            break;
        }
        case SID_CANCELCHECKOUT:
        {
            if (  QueryBox( NULL, SfxResId( RID_QUERY_CANCELCHECKOUT ) ).Execute( ) == RET_YES )
            {
                CancelCheckOut( );
            }
            break;
        }
        case SID_CHECKIN:
        {
            CheckIn( );
            break;
        }
    }

    // Prevent entry in the Pick-lists
    if ( rReq.IsAPI() )
        GetMedium()->SetUpdatePickList( sal_False );
    else if ( rReq.GetArgs() )
    {
        SFX_ITEMSET_GET( *rReq.GetArgs(), pPicklistItem, SfxBoolItem, SID_PICKLIST, sal_False );
        if ( pPicklistItem )
            GetMedium()->SetUpdatePickList( pPicklistItem->GetValue() );
    }

    // Ignore()-branches have already returned
    rReq.Done();
}

//-------------------------------------------------------------------------

void SfxObjectShell::GetState_Impl(SfxItemSet &rSet)
{
    DBG_CHKTHIS(SfxObjectShell, 0);
    SfxWhichIter aIter( rSet );

    for ( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
            case SID_DOCTEMPLATE :
            {
                if ( !GetFactory().GetTemplateFilter() )
                    rSet.DisableItem( nWhich );
                break;
            }

            case SID_CHECKOUT:
                {
                    bool bShow = false;
                    Reference< XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY );
                    uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties();

                    if ( xCmisDoc->isVersionable( ) && aCmisProperties.hasElements( ) )
                    {
                        // Loop over the CMIS Properties to find cmis:isVersionSeriesCheckedOut
                        bool bFoundCheckedout = false;
                        sal_Bool bCheckedOut = sal_False;
                        for ( sal_Int32 i = 0; i < aCmisProperties.getLength() && !bFoundCheckedout; ++i )
                        {
                            if ( aCmisProperties[i].Name == "cmis:isVersionSeriesCheckedOut" )
                            {
                                bFoundCheckedout = true;
                                aCmisProperties[i].Value >>= bCheckedOut;
                            }
                        }
                        bShow = !bCheckedOut;
                    }

                    if ( !bShow )
                    {
                        rSet.DisableItem( nWhich );
                        rSet.Put( SfxVisibilityItem( nWhich, sal_False ) );
                    }
                }
                break;

            case SID_CANCELCHECKOUT:
            case SID_CHECKIN:
                {
                    bool bShow = false;
                    Reference< XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY );
                    uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties( );

                    if ( xCmisDoc->isVersionable( ) && aCmisProperties.hasElements( ) )
                    {
                        // Loop over the CMIS Properties to find cmis:isVersionSeriesCheckedOut
                        bool bFoundCheckedout = false;
                        sal_Bool bCheckedOut = sal_False;
                        for ( sal_Int32 i = 0; i < aCmisProperties.getLength() && !bFoundCheckedout; ++i )
                        {
                            if ( aCmisProperties[i].Name == "cmis:isVersionSeriesCheckedOut" )
                            {
                                bFoundCheckedout = true;
                                aCmisProperties[i].Value >>= bCheckedOut;
                            }
                        }
                        bShow = bCheckedOut;
                    }

                    if ( !bShow )
                    {
                        rSet.DisableItem( nWhich );
                        rSet.Put( SfxVisibilityItem( nWhich, sal_False ) );
                    }
                }
                break;

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
                            nWhich, SfxResId(STR_SAVEDOC).toString()));
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

                    // If SID_CLOSEDOC is excecuted through menu and so on, but
                    // the current document is in a frame, then the
                    // FrameSetDocument should actually be closed.
                    pDoc = pFrame->GetTopViewFrame()->GetObjectShell();
                }

                if ( pDoc->GetFlags() & SFXOBJECTSHELL_DONTCLOSE )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxStringItem(nWhich, SfxResId(STR_CLOSEDOC).toString()));
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
                    rSet.Put( SfxStringItem( nWhich, SfxResId(STR_SAVEASDOC).toString() ) );
                break;
            }

            case SID_SAVEACOPY:
            {
                if( ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) != SFX_LOADED_MAINDOCUMENT )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }
                if ( /*!pCombinedFilters ||*/ !GetMedium() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxStringItem( nWhich, SfxResId(STR_SAVEACOPY).toString() ) );
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

        case SID_DOCINFO_AUTHOR :
        {
            OUString aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            getDocProperties()->setAuthor( aStr );
            break;
        }

        case SID_DOCINFO_COMMENTS :
        {
            OUString aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            getDocProperties()->setDescription( aStr );
            break;
        }

        case SID_DOCINFO_KEYWORDS :
        {
            OUString aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
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
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
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
                OSL_FAIL( "Not supported anymore!" );
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
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this, sal_True );
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
                OUString aFileName( GetObjectShell()->GetMedium()->GetName() );
                if ( !aFileName.isEmpty() )
                {
                    SfxStringItem aName( SID_FILE_NAME, aFileName );
                    SfxBoolItem aCreateView( SID_OPEN_NEW_VIEW, sal_True );
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

    if ( GetMedium() && !GetMedium()->GetName().isEmpty() && IsOwnStorageFormat_Impl( *GetMedium())  && GetMedium()->GetStorage().is() )
    {
        try
        {
            if ( !xLocSigner.is() )
            {
                OUString aVersion;
                try
                {
                    uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
                    xPropSet->getPropertyValue("Version") >>= aVersion;
                }
                catch( uno::Exception& )
                {
                }

                xLocSigner.set( security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), aVersion) );

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
        &&  !GetMedium()->GetName().isEmpty()
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
    OUString aODFVersion;
    try
    {
        // check the version of the document
        uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
        xPropSet->getPropertyValue("Version") >>= aODFVersion;
    }
    catch( uno::Exception& )
    {}

    bool bNoSig = false;

    if ( IsModified() || !GetMedium() || GetMedium()->GetName().isEmpty()
      || (!aODFVersion.equals( ODFVER_012_TEXT ) && !bHasSign) )
    {
        // the document might need saving ( new, modified or in ODF1.1 format without signature )

        if ( nVersion >= SvtSaveOptions::ODFVER_012 )
        {

            if ( (bHasSign && QueryBox( NULL, SfxResId( MSG_XMLSEC_QUERY_SAVESIGNEDBEFORESIGN ) ).Execute() == RET_YES)
              || (!bHasSign && QueryBox( NULL, SfxResId( RID_XMLSEC_QUERY_SAVEBEFORESIGN ) ).Execute() == RET_YES) )
            {
                sal_uInt16 nId = SID_SAVEDOC;
                if ( !GetMedium() || GetMedium()->GetName().isEmpty() )
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
                // When the document is modified then we must not show the
                // digital signatures dialog
                // If we have come here then the user denied to save.
                if (!bHasSign)
                    bNoSig = true;
            }
        }
        else
        {
            ErrorBox( NULL, WB_OK, SfxResId(STR_XMLSEC_ODF12_EXPECTED).toString() ).Execute();
            return;
        }

        if ( IsModified() || !GetMedium() || GetMedium()->GetName().isEmpty() )
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
    return ImplGetSignatureState( sal_False );
}

void SfxObjectShell::SignDocumentContent()
{
    ImplSign( sal_False );
}

sal_uInt16 SfxObjectShell::GetScriptingSignatureState()
{
    return ImplGetSignatureState( sal_True );
}

void SfxObjectShell::SignScriptingContent()
{
    ImplSign( sal_True );
}

namespace
{
    class theSfxObjectShellUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSfxObjectShellUnoTunnelId > {};
}

const uno::Sequence<sal_Int8>& SfxObjectShell::getUnoTunnelId()
{
    return theSfxObjectShellUnoTunnelId::get().getSeq();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
