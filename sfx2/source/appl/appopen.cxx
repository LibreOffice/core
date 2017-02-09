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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <rtl/ustring.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <comphelper/synchronousdispatch.hxx>

#include <vcl/wrkwin.hxx>
#include <svl/intitem.hxx>
#include <vcl/layout.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/doctempl.hxx>
#include <svtools/sfxecode.hxx>
#include <framework/preventduplicateinteraction.hxx>
#include <svtools/ehdl.hxx>
#include <basic/sbxobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/miscopt.hxx>
#include <osl/file.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <vcl/svapp.hxx>

#include <osl/mutex.hxx>


#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/new.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/slstitm.hxx>
#include "appopen.hxx"
#include "objshimp.hxx"
#include "openflag.hxx"
#include <sfx2/passwd.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/viewsh.hxx>
#include "app.hrc"
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/event.hxx>
#include <sfx2/templatedlg.hxx>
#include "openuriexternally.hxx"

#include <officecfg/Office/ProtocolHandler.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::container;
using namespace ::cppu;
using namespace ::sfx2;

void SetTemplate_Impl( const OUString &rFileName,
                        const OUString &rLongName,
                        SfxObjectShell *pDoc)
{
    // write TemplateName to DocumentProperties of document
    // TemplateDate stays as default (=current date)
    pDoc->ResetFromTemplate( rLongName, rFileName );
}


class SfxDocPasswordVerifier : public ::comphelper::IDocPasswordVerifier
{
public:
    inline explicit     SfxDocPasswordVerifier( const Reference< embed::XStorage >& rxStorage ) :
                            mxStorage( rxStorage ) {}

    virtual ::comphelper::DocPasswordVerifierResult
                        verifyPassword( const OUString& rPassword, uno::Sequence< beans::NamedValue >& o_rEncryptionData ) override;
    virtual ::comphelper::DocPasswordVerifierResult
                        verifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData ) override;


private:
    Reference< embed::XStorage > mxStorage;
};


::comphelper::DocPasswordVerifierResult SfxDocPasswordVerifier::verifyPassword( const OUString& rPassword, uno::Sequence< beans::NamedValue >& o_rEncryptionData )
{
    o_rEncryptionData = ::comphelper::OStorageHelper::CreatePackageEncryptionData( rPassword );
    return verifyEncryptionData( o_rEncryptionData );
}


::comphelper::DocPasswordVerifierResult SfxDocPasswordVerifier::verifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData )
{
    ::comphelper::DocPasswordVerifierResult eResult = ::comphelper::DocPasswordVerifierResult::WrongPassword;
    try
    {
        // check the encryption data
        // if the data correct is the stream will be opened successfully
        // and immediately closed
        ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( mxStorage, rEncryptionData );

        mxStorage->openStreamElement(
                "content.xml",
                embed::ElementModes::READ | embed::ElementModes::NOCREATE );

        // no exception -> success
        eResult = ::comphelper::DocPasswordVerifierResult::OK;
    }
    catch( const packages::WrongPasswordException& )
    {
        eResult = ::comphelper::DocPasswordVerifierResult::WrongPassword;
    }
    catch( const uno::Exception& )
    {
        // unknown error, report it as wrong password
        // TODO/LATER: we need an additional way to report unknown problems in this case
        eResult = ::comphelper::DocPasswordVerifierResult::WrongPassword;
    }
    return eResult;
}


sal_uInt32 CheckPasswd_Impl
(
    SfxObjectShell*  pDoc,
    SfxItemPool&     /*rPool*/, // Pool, if a Set has to be created
    SfxMedium*       pFile      // the Medium and its Password shold be obtained
)

/*  [Description]

    Ask for the password for a medium, only works if it concerns storage.
    If the password flag is set in the Document Info, then the password is
    requested through a user dialogue and the set at the Set of the medium.
    If the set does not exist the it is created.
*/
{
    sal_uIntPtr nRet = ERRCODE_NONE;

    if( ( !pFile->GetFilter() || pFile->IsStorage() ) )
    {
        uno::Reference< embed::XStorage > xStorage = pFile->GetStorage();
        if( xStorage.is() )
        {
            uno::Reference< beans::XPropertySet > xStorageProps( xStorage, uno::UNO_QUERY );
            if ( xStorageProps.is() )
            {
                bool bIsEncrypted = false;
                try {
                    xStorageProps->getPropertyValue("HasEncryptedEntries")
                        >>= bIsEncrypted;
                } catch( uno::Exception& )
                {
                    // TODO/LATER:
                    // the storage either has no encrypted elements or it's just
                    // does not allow to detect it, probably it should be implemented later
                }

                if ( bIsEncrypted )
                {
                    vcl::Window* pWin = pDoc ? pDoc->GetDialogParent( pFile ) : nullptr;
                    if ( pWin )
                        pWin->Show();

                    nRet = ERRCODE_SFX_CANTGETPASSWD;

                    SfxItemSet *pSet = pFile->GetItemSet();
                    if( pSet )
                    {
                        Reference< css::task::XInteractionHandler > xInteractionHandler = pFile->GetInteractionHandler();
                        if( xInteractionHandler.is() )
                        {
                            // use the comphelper password helper to request a password
                            OUString aPassword;
                            const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_PASSWORD, false);
                            if ( pPasswordItem )
                                aPassword = pPasswordItem->GetValue();

                            uno::Sequence< beans::NamedValue > aEncryptionData;
                            const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_ENCRYPTIONDATA, false);
                            if ( pEncryptionDataItem )
                                pEncryptionDataItem->GetValue() >>= aEncryptionData;

                            SfxDocPasswordVerifier aVerifier( xStorage );
                            aEncryptionData = ::comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
                                aVerifier, aEncryptionData, aPassword, xInteractionHandler, pFile->GetOrigURL(), comphelper::DocPasswordRequestType::Standard );

                            pSet->ClearItem( SID_PASSWORD );
                            pSet->ClearItem( SID_ENCRYPTIONDATA );

                            if ( aEncryptionData.getLength() > 0 )
                            {
                                pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );

                                try
                                {
                                    // update the version list of the medium using the new password
                                    pFile->GetVersionList();
                                }
                                catch( uno::Exception& )
                                {
                                    // TODO/LATER: set the error code
                                }

                                nRet = ERRCODE_NONE;
                            }
                            else
                                nRet = ERRCODE_IO_ABORT;
                        }
                    }
                }
            }
            else
            {
                OSL_FAIL( "A storage must implement XPropertySet interface!" );
                nRet = ERRCODE_SFX_CANTGETPASSWD;
            }
        }
    }

    return nRet;
}


sal_uIntPtr SfxApplication::LoadTemplate( SfxObjectShellLock& xDoc, const OUString &rFileName, SfxItemSet* pSet )
{
    std::shared_ptr<const SfxFilter> pFilter;
    SfxMedium aMedium( rFileName,  ( StreamMode::READ | StreamMode::SHARE_DENYNONE ) );

    if ( !aMedium.GetStorage( false ).is() )
        aMedium.GetInStream();

    if ( aMedium.GetError() )
    {
        delete pSet;
        return aMedium.GetErrorCode();
    }

    aMedium.UseInteractionHandler( true );
    sal_uIntPtr nErr = GetFilterMatcher().GuessFilter( aMedium, pFilter, SfxFilterFlags::TEMPLATE, SfxFilterFlags::NONE );
    if ( 0 != nErr)
    {
        delete pSet;
        return ERRCODE_SFX_NOTATEMPLATE;
    }

    if( !pFilter || !pFilter->IsAllowedAsTemplate() )
    {
        delete pSet;
        return ERRCODE_SFX_NOTATEMPLATE;
    }

    if ( pFilter->GetFilterFlags() & SfxFilterFlags::STARONEFILTER )
    {
        DBG_ASSERT( !xDoc.Is(), "Sorry, not implemented!" );
        delete pSet;
        SfxStringItem aName( SID_FILE_NAME, rFileName );
        SfxStringItem aReferer( SID_REFERER, OUString("private:user") );
        SfxStringItem aFlags( SID_OPTIONS, OUString("T") );
        SfxBoolItem aHidden( SID_HIDDEN, true );
        const SfxPoolItem *pRet = GetDispatcher_Impl()->ExecuteList(
            SID_OPENDOC, SfxCallMode::SYNCHRON,
            { &aName, &aHidden, &aReferer, &aFlags } );
        const SfxObjectItem *pObj = dynamic_cast<const SfxObjectItem*>( pRet  );
        if ( pObj )
            xDoc = dynamic_cast<SfxObjectShell*>( pObj->GetShell()  );
        else
        {
            const SfxViewFrameItem *pView = dynamic_cast<const SfxViewFrameItem*>( pRet  );
            if ( pView )
            {
                SfxViewFrame *pFrame = pView->GetFrame();
                if ( pFrame )
                    xDoc = pFrame->GetObjectShell();
            }
        }

        if ( !xDoc.Is() )
            return ERRCODE_SFX_DOLOADFAILED;
    }
    else
    {
        if ( !xDoc.Is() )
            xDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName() );

        //pMedium takes ownership of pSet
        SfxMedium *pMedium = new SfxMedium( rFileName, StreamMode::STD_READ, pFilter, pSet );
        if(!xDoc->DoLoad(pMedium))
        {
            ErrCode nErrCode = xDoc->GetErrorCode();
            xDoc->DoClose();
            xDoc.Clear();
            return nErrCode;
        }
    }

    try
    {
        // TODO: introduce error handling

        uno::Reference< embed::XStorage > xTempStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
        if( !xTempStorage.is() )
            throw uno::RuntimeException();

        xDoc->GetStorage()->copyToStorage( xTempStorage );

        if ( !xDoc->DoSaveCompleted( new SfxMedium( xTempStorage, OUString() ) ) )
            throw uno::RuntimeException();
    }
    catch( uno::Exception& )
    {
        xDoc->DoClose();
        xDoc.Clear();

        // TODO: transfer correct error outside
        return ERRCODE_SFX_GENERAL;
    }

    SetTemplate_Impl( rFileName, OUString(), xDoc );

    xDoc->SetNoName();
    xDoc->InvalidateName();
    xDoc->SetModified(false);
    xDoc->ResetError();

    css::uno::Reference< css::frame::XModel >  xModel ( xDoc->GetModel(), css::uno::UNO_QUERY );
    if ( xModel.is() )
    {
        SfxItemSet* pNew = xDoc->GetMedium()->GetItemSet()->Clone();
        pNew->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        pNew->ClearItem( SID_FILTER_NAME );
        css::uno::Sequence< css::beans::PropertyValue > aArgs;
        TransformItems( SID_OPENDOC, *pNew, aArgs );
        sal_Int32 nLength = aArgs.getLength();
        aArgs.realloc( nLength + 1 );
        aArgs[nLength].Name = "Title";
        aArgs[nLength].Value <<= OUString( xDoc->GetTitle( SFX_TITLE_DETECT ) );
        xModel->attachResource( OUString(), aArgs );
        delete pNew;
    }

    return xDoc->GetErrorCode();
}


void SfxApplication::NewDocDirectExec_Impl( SfxRequest& rReq )
{
    const SfxStringItem* pFactoryItem = rReq.GetArg<SfxStringItem>(SID_NEWDOCDIRECT);
    OUString aFactName;
    if ( pFactoryItem )
        aFactName = pFactoryItem->GetValue();
   else
        aFactName = SvtModuleOptions().GetDefaultModuleName();


    SfxRequest aReq( SID_OPENDOC, SfxCallMode::SYNCHRON, GetPool() );
    OUString aFact("private:factory/");
    aFact += aFactName;
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aFact ) );
    aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, GetFrame() ) );
    aReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString( "_default" ) ) );

    // TODO/LATER: Should the other arguments be transferred as well?
    const SfxStringItem* pDefaultPathItem = rReq.GetArg<SfxStringItem>(SID_DEFAULTFILEPATH);
    if ( pDefaultPathItem )
        aReq.AppendItem( *pDefaultPathItem );
    const SfxStringItem* pDefaultNameItem = rReq.GetArg<SfxStringItem>(SID_DEFAULTFILENAME);
    if ( pDefaultNameItem )
        aReq.AppendItem( *pDefaultNameItem );

    SfxGetpApp()->ExecuteSlot( aReq );
    const SfxViewFrameItem* pItem = dynamic_cast<const SfxViewFrameItem*>( aReq.GetReturnValue()  );
    if ( pItem )
        rReq.SetReturnValue( SfxFrameItem( 0, pItem->GetFrame() ) );
}


void SfxApplication::NewDocExec_Impl( SfxRequest& rReq )
{
    // No Parameter from BASIC only Factory given?
    const SfxStringItem* pTemplNameItem = rReq.GetArg<SfxStringItem>(SID_TEMPLATE_NAME);
    const SfxStringItem* pTemplFileNameItem = rReq.GetArg<SfxStringItem>(SID_FILE_NAME);
    const SfxStringItem* pTemplRegionNameItem = rReq.GetArg<SfxStringItem>(SID_TEMPLATE_REGIONNAME);

    SfxObjectShellLock xDoc;

    OUString  aTemplateRegion, aTemplateName, aTemplateFileName;
    bool    bDirect = false; // through FileName instead of Region/Template
    SfxErrorContext aEc(ERRCTX_SFX_NEWDOC);
    if ( !pTemplNameItem && !pTemplFileNameItem )
    {
        bool bNewWin = false;
        vcl::Window* pTopWin = GetTopWindow();

        SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
        Reference<XModel> xModel;
        if(pCurrentShell)
            xModel = pCurrentShell->GetModel();

        ScopedVclPtrInstance< SfxTemplateManagerDlg > aTemplDlg;

        if (xModel.is())
            aTemplDlg->setDocumentModel(xModel);

        int nRet = aTemplDlg->Execute();
        if ( nRet == RET_OK )
        {
            rReq.Done();
            if ( pTopWin != GetTopWindow() )
            {
                // the dialogue opens a document -> a new TopWindow appears
                pTopWin = GetTopWindow();
                bNewWin = true;
            }
        }

        if ( bNewWin && pTopWin )
            // after the destruction of the dialogue its parent comes to top,
            // but we want that the new document is on top
            pTopWin->ToTop();

        return;
    }
    else
    {
        // Template-Name
        if ( pTemplNameItem )
            aTemplateName = pTemplNameItem->GetValue();

        // Template-Region
        if ( pTemplRegionNameItem )
            aTemplateRegion = pTemplRegionNameItem->GetValue();

        // Template-File-Name
        if ( pTemplFileNameItem )
        {
            aTemplateFileName = pTemplFileNameItem->GetValue();
            bDirect = true;
        }
    }

    sal_uIntPtr lErr = 0;
    SfxItemSet* pSet = new SfxAllItemSet( GetPool() );
    pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );
    if ( !bDirect )
    {
        SfxDocumentTemplates aTmpFac;
        if( aTemplateFileName.isEmpty() )
            aTmpFac.GetFull( aTemplateRegion, aTemplateName, aTemplateFileName );

        if( aTemplateFileName.isEmpty() )
            lErr = ERRCODE_SFX_TEMPLATENOTFOUND;
    }

    INetURLObject aObj( aTemplateFileName );
    SfxErrorContext aEC( ERRCTX_SFX_LOADTEMPLATE, aObj.PathToFileName() );

    if ( lErr != ERRCODE_NONE )
    {
        sal_uIntPtr lFatalErr = IgnoreWarning(lErr);
        if ( lFatalErr )
            ErrorHandler::HandleError(lErr);
    }
    else
    {
        SfxCallMode eMode = SfxCallMode::SYNCHRON;

        const SfxPoolItem *pRet=nullptr;
        SfxStringItem aReferer( SID_REFERER, "private:user" );
        SfxStringItem aTarget( SID_TARGETNAME, "_default" );
        if ( !aTemplateFileName.isEmpty() )
        {
            DBG_ASSERT( aObj.GetProtocol() != INetProtocol::NotValid, "Illegal URL!" );

            SfxStringItem aName( SID_FILE_NAME, aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            SfxStringItem aTemplName( SID_TEMPLATE_NAME, aTemplateName );
            SfxStringItem aTemplRegionName( SID_TEMPLATE_REGIONNAME, aTemplateRegion );
            pRet = GetDispatcher_Impl()->ExecuteList(SID_OPENDOC, eMode,
                {&aName, &aTarget, &aReferer, &aTemplName, &aTemplRegionName});
        }
        else
        {
            SfxStringItem aName( SID_FILE_NAME, "private:factory" );
            pRet = GetDispatcher_Impl()->ExecuteList(SID_OPENDOC, eMode,
                    { &aName, &aTarget, &aReferer } );
        }

        if ( pRet )
            rReq.SetReturnValue( *pRet );
    }
}


namespace {

/**
 * Check if a given filter type should open the hyperlinked document
 * natively.
 *
 * @param rFilter filter object
 */
bool lcl_isFilterNativelySupported(const SfxFilter& rFilter)
{
    if (rFilter.IsOwnFormat())
        return true;

    const OUString& aName = rFilter.GetFilterName();
    if (aName.startsWith("MS Excel"))
        // We can handle all Excel variants natively.
        return true;

    return false;
}

}

void SfxApplication::OpenDocExec_Impl( SfxRequest& rReq )
{
    OUString aDocService;
    const SfxStringItem* pDocSrvItem = rReq.GetArg<SfxStringItem>(SID_DOC_SERVICE);
    if (pDocSrvItem)
        aDocService = pDocSrvItem->GetValue();

    sal_uInt16 nSID = rReq.GetSlot();
    const SfxStringItem* pFileNameItem = rReq.GetArg<SfxStringItem>(SID_FILE_NAME);
    if ( pFileNameItem )
    {
        OUString aCommand( pFileNameItem->GetValue() );
        const SfxSlot* pSlot = GetInterface()->GetSlot( aCommand );
        if ( pSlot )
        {
            pFileNameItem = nullptr;
        }
        else
        {
            if ( aCommand.startsWith("slot:") )
            {
                sal_uInt16 nSlotId = (sal_uInt16) aCommand.copy(5).toInt32();
                if ( nSlotId == SID_OPENDOC )
                    pFileNameItem = nullptr;
            }
        }
    }

    if ( !pFileNameItem )
    {
        // get FileName from dialog
        std::vector<OUString> aURLList;
        OUString aFilter;
        SfxItemSet* pSet = nullptr;
        OUString aPath;
        const SfxStringItem* pFolderNameItem = rReq.GetArg<SfxStringItem>(SID_PATH);
        if ( pFolderNameItem )
            aPath = pFolderNameItem->GetValue();
        else if ( nSID == SID_OPENTEMPLATE )
        {
            aPath = SvtPathOptions().GetTemplatePath();
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(aPath, ';');
            aPath = aPath.getToken( nTokenCount ? ( nTokenCount - 1 ) : 0 , ';' );
        }

        sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG;
        const SfxBoolItem* pSystemDialogItem = rReq.GetArg<SfxBoolItem>(SID_FILE_DIALOG);
        if ( pSystemDialogItem )
            nDialog = pSystemDialogItem->GetValue() ? SFX2_IMPL_DIALOG_SYSTEM : SFX2_IMPL_DIALOG_OOO;

        const SfxBoolItem* pRemoteDialogItem = rReq.GetArg<SfxBoolItem>(SID_REMOTE_DIALOG);
        if ( pRemoteDialogItem && pRemoteDialogItem->GetValue())
            nDialog = SFX2_IMPL_DIALOG_REMOTE;

        sal_Int16 nDialogType = ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION;
        FileDialogFlags eDialogFlags = FileDialogFlags::MultiSelection;
        const SfxBoolItem* pSignPDFItem = rReq.GetArg<SfxBoolItem>(SID_SIGNPDF);
        if (pSignPDFItem && pSignPDFItem->GetValue())
        {
            eDialogFlags |= FileDialogFlags::SignPDF;
            nDialogType = ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
        }

        OUString sStandardDir;

        const SfxStringItem* pStandardDirItem = rReq.GetArg<SfxStringItem>(SID_STANDARD_DIR);
        if ( pStandardDirItem )
            sStandardDir = pStandardDirItem->GetValue();

        css::uno::Sequence< OUString >  aBlackList;

        const SfxStringListItem* pBlackListItem = rReq.GetArg<SfxStringListItem>(SID_BLACK_LIST);
        if ( pBlackListItem )
            pBlackListItem->GetStringList( aBlackList );


        sal_uIntPtr nErr = sfx2::FileOpenDialog_Impl(
                nDialogType,
                eDialogFlags, OUString(), aURLList,
                aFilter, pSet, &aPath, nDialog, sStandardDir, aBlackList );

        if ( nErr == ERRCODE_ABORT )
        {
            aURLList.clear();
            return;
        }

        rReq.SetArgs( *static_cast<SfxAllItemSet*>(pSet) );
        if ( !aFilter.isEmpty() )
            rReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilter ) );
        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_default") ) );
        rReq.AppendItem( SfxStringItem( SID_REFERER, "private:user" ) );
        delete pSet;

        if(!aURLList.empty())
        {
            if ( nSID == SID_OPENTEMPLATE )
                rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, false ) );

            // This helper wraps an existing (or may new created InteractionHandler)
            // intercept all incoming interactions and provide useful information
            // later if the following transaction was finished.

            ::framework::PreventDuplicateInteraction*                 pHandler       = new ::framework::PreventDuplicateInteraction(::comphelper::getProcessComponentContext());
            css::uno::Reference< css::task::XInteractionHandler >     xHandler       (static_cast< css::task::XInteractionHandler* >(pHandler), css::uno::UNO_QUERY);
            css::uno::Reference< css::task::XInteractionHandler >     xWrappedHandler;

            // wrap existing handler or create new UUI handler
            const SfxUnoAnyItem* pInteractionItem = rReq.GetArg<SfxUnoAnyItem>(SID_INTERACTIONHANDLER);
            if (pInteractionItem)
            {
                pInteractionItem->GetValue() >>= xWrappedHandler;
                rReq.RemoveItem( SID_INTERACTIONHANDLER );
            }
            if (xWrappedHandler.is())
                pHandler->setHandler(xWrappedHandler);
            else
                pHandler->useDefaultUUIHandler();
            rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,css::uno::makeAny(xHandler)) );

            // define rules for this handler
            css::uno::Type                                            aInteraction = ::cppu::UnoType<css::task::ErrorCodeRequest>::get();
            ::framework::PreventDuplicateInteraction::InteractionInfo aRule        (aInteraction, 1);
            pHandler->addInteractionRule(aRule);

            if (!aDocService.isEmpty())
            {
                rReq.RemoveItem(SID_DOC_SERVICE);
                rReq.AppendItem(SfxStringItem(SID_DOC_SERVICE, aDocService));
            }

            for(std::vector<OUString>::const_iterator i = aURLList.begin(); i != aURLList.end(); ++i)
            {
                rReq.RemoveItem( SID_FILE_NAME );
                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, *i ) );

                // Run synchronous, so that not the next document is loaded
                // when rescheduling
                // TODO/LATER: use URLList argument and always remove one document after another, each step in asychronous execution, until finished
                // but only if reschedule is a problem
                GetDispatcher_Impl()->Execute( SID_OPENDOC, SfxCallMode::SYNCHRON, *rReq.GetArgs() );

                // check for special interaction "NO MORE DOCUMENTS ALLOWED" and
                // break loop then. Otherwise we risk showing the same interaction more than once.
                if ( pHandler->getInteractionInfo(aInteraction, &aRule) )
                {
                    if (aRule.m_nCallCount > 0)
                    {
                        if (aRule.m_xRequest.is())
                        {
                            css::task::ErrorCodeRequest aRequest;
                            if (aRule.m_xRequest->getRequest() >>= aRequest)
                            {
                                if (aRequest.ErrCode ==
                                    sal::static_int_cast< sal_Int32 >(
                                        ERRCODE_SFX_NOMOREDOCUMENTSALLOWED))
                                    break;
                            }
                        }
                    }
                }
            }

            aURLList.clear();
            return;
        }
        aURLList.clear();
    }

    bool bHyperlinkUsed = false;

    if ( SID_OPENURL == nSID )
    {
        // SID_OPENURL does the same as SID_OPENDOC!
        rReq.SetSlot( SID_OPENDOC );
    }
    else if ( nSID == SID_OPENTEMPLATE )
    {
        rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, false ) );
    }
    // pass URL to OS by using ShellExecuter or open it internal
    // if it seems to be an own format.
    /* Attention!
            There exist two possibilities to open hyperlinks:
            a) using SID_OPENHYPERLINK (new)
            b) using SID_BROWSE        (old)
     */
    else if ( nSID == SID_OPENHYPERLINK )
    {
        rReq.SetSlot( SID_OPENDOC );
        bHyperlinkUsed = true;
    }

    // no else here! It's optional ...
    if (!bHyperlinkUsed)
    {
        const SfxBoolItem* pHyperLinkUsedItem = rReq.GetArg<SfxBoolItem>(SID_BROWSE);
        if ( pHyperLinkUsedItem )
            bHyperlinkUsed = pHyperLinkUsedItem->GetValue();
        // no "official" item, so remove it from ItemSet before using UNO-API
        rReq.RemoveItem( SID_BROWSE );
    }

    const SfxStringItem* pFileName = rReq.GetArg<SfxStringItem>(SID_FILE_NAME);
    OUString aFileName = pFileName->GetValue();

    OUString aReferer;
    const SfxStringItem* pRefererItem = rReq.GetArg<SfxStringItem>(SID_REFERER);
    if ( pRefererItem )
        aReferer = pRefererItem->GetValue();

    const SfxStringItem* pFileFlagsItem = rReq.GetArg<SfxStringItem>(SID_OPTIONS);
    if ( pFileFlagsItem )
    {
        OUString aFileFlags = pFileFlagsItem->GetValue();
        aFileFlags = aFileFlags.toAsciiUpperCase();
        if ( -1 != aFileFlags.indexOf( 0x0054 ) )               // T = 54h
        {
            rReq.RemoveItem( SID_TEMPLATE );
            rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, true ) );
        }

        if ( -1 != aFileFlags.indexOf( 0x0048 ) )               // H = 48h
        {
            rReq.RemoveItem( SID_HIDDEN );
            rReq.AppendItem( SfxBoolItem( SID_HIDDEN, true ) );
        }

        if ( -1 != aFileFlags.indexOf( 0x0052 ) )               // R = 52h
        {
            rReq.RemoveItem( SID_DOC_READONLY );
            rReq.AppendItem( SfxBoolItem( SID_DOC_READONLY, true ) );
        }

        if ( -1 != aFileFlags.indexOf( 0x0042 ) )               // B = 42h
        {
            rReq.RemoveItem( SID_PREVIEW );
            rReq.AppendItem( SfxBoolItem( SID_PREVIEW, true ) );
        }

        rReq.RemoveItem( SID_OPTIONS );
    }

    // Mark without URL cannot be handled by hyperlink code
    if ( bHyperlinkUsed && !aFileName.isEmpty() && aFileName[0] != '#' )
    {
        Reference< css::document::XTypeDetection > xTypeDetection( ::comphelper::getProcessServiceFactory()->createInstance(
                                                                       "com.sun.star.document.TypeDetection"),
                                                                   UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            URL             aURL;
            OUString aTypeName;

            aURL.Complete = aFileName;
            Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict( aURL );

            INetProtocol aINetProtocol = INetURLObject( aURL.Complete ).GetProtocol();
            SvtExtendedSecurityOptions aExtendedSecurityOptions;
            SvtExtendedSecurityOptions::OpenHyperlinkMode eMode = aExtendedSecurityOptions.GetOpenHyperlinkMode();

            if ( eMode == SvtExtendedSecurityOptions::OPEN_NEVER && aINetProtocol != INetProtocol::VndSunStarHelp )
            {
                SolarMutexGuard aGuard;
                vcl::Window *pWindow = SfxGetpApp()->GetTopWindow();

                ScopedVclPtrInstance<MessageDialog> aSecurityWarningBox(pWindow,
                                                  SfxResId(STR_SECURITY_WARNING_NO_HYPERLINKS),
                                                  VclMessageType::Warning);
                aSecurityWarningBox->SetText( SfxResId(RID_SECURITY_WARNING_TITLE).toString() );
                aSecurityWarningBox->Execute();
                return;
            }

            aTypeName = xTypeDetection->queryTypeByURL( aURL.Main );
            SfxFilterMatcher& rMatcher = SfxGetpApp()->GetFilterMatcher();
            std::shared_ptr<const SfxFilter> pFilter = rMatcher.GetFilter4EA( aTypeName );
            if (!pFilter || !lcl_isFilterNativelySupported(*pFilter))
            {
                // hyperlink does not link to own type => special handling (http, ftp) browser and (other external protocols) OS
                if ( aINetProtocol == INetProtocol::Mailto )
                {
                    // don't dispatch mailto hyperlink to desktop dispatcher
                    rReq.RemoveItem( SID_TARGETNAME );
                    rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_self") ) );
                }
                else if ( aINetProtocol == INetProtocol::Ftp ||
                     aINetProtocol == INetProtocol::Http ||
                     aINetProtocol == INetProtocol::Https )
                {
                    sfx2::openUriExternally(aURL.Complete, true);
                    return;
                }
                else
                {
                    // check for "internal" protocols that should not be forwarded to the system
                    std::vector < OUString > aProtocols(2);

                    // add special protocols that always should be treated as internal
                    aProtocols[0] = "private:*";
                    aProtocols[1] = "vnd.sun.star.*";

                    // get registered protocol handlers from configuration
                    Reference < XNameAccess > xAccess(officecfg::Office::ProtocolHandler::HandlerSet::get());
                    Sequence < OUString > aNames = xAccess->getElementNames();
                    for ( sal_Int32 nName = 0; nName < aNames.getLength(); nName ++)
                    {
                        Reference < XPropertySet > xSet;
                        Any aRet = xAccess->getByName( aNames[nName] );
                        aRet >>= xSet;
                        if ( xSet.is() )
                        {
                            // copy protocols
                            aRet = xSet->getPropertyValue("Protocols");
                            Sequence < OUString > aTmp;
                            aRet >>= aTmp;

                            aProtocols.insert(aProtocols.end(),aTmp.begin(),aTmp.end());
                        }
                    }

                    bool bFound = false;
                    for (const OUString & rProtocol : aProtocols)
                    {
                        WildCard aPattern(rProtocol);
                        if ( aPattern.Matches( aURL.Complete ) )
                        {
                            bFound = true;
                            break;
                        }
                    }

                    if ( !bFound )
                    {
                        bool bLoadInternal = false;
                        try
                        {
                            sfx2::openUriExternally(
                                aURL.Complete, pFilter == nullptr);
                        }
                        catch ( css::system::SystemShellExecuteException& )
                        {
                            rReq.RemoveItem( SID_TARGETNAME );
                            rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_default") ) );
                            bLoadInternal = true;
                        }
                        if ( !bLoadInternal )
                            return;
                    }
                }
            }
            else
            {
                // hyperlink document must be loaded into a new frame
                rReq.RemoveItem( SID_TARGETNAME );
                rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_default") ) );
            }
        }
    }

    if (!SvtSecurityOptions().isSecureMacroUri(aFileName, aReferer))
    {
        SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, aFileName );
        ErrorHandler::HandleError( ERRCODE_IO_ACCESSDENIED );
        return;
    }

    SfxFrame* pTargetFrame = nullptr;
    Reference< XFrame > xTargetFrame;

    const SfxFrameItem* pFrameItem = rReq.GetArg<SfxFrameItem>(SID_DOCFRAME);
    if ( pFrameItem )
        pTargetFrame = pFrameItem->GetFrame();

    if ( !pTargetFrame )
    {
        const SfxUnoFrameItem* pUnoFrameItem = rReq.GetArg<SfxUnoFrameItem>(SID_FILLFRAME);
        if ( pUnoFrameItem )
            xTargetFrame = pUnoFrameItem->GetFrame();
    }

    if ( !pTargetFrame && !xTargetFrame.is() && SfxViewFrame::Current() )
        pTargetFrame = &SfxViewFrame::Current()->GetFrame();

    // check if caller has set a callback
    const SfxLinkItem* pLinkItem = rReq.GetArg<SfxLinkItem>(SID_DONELINK);

    // remove from Itemset, because it confuses the parameter transformation
    if ( pLinkItem )
        pLinkItem = static_cast<SfxLinkItem*>( pLinkItem->Clone() );

    rReq.RemoveItem( SID_DONELINK );

    // check if the view must be hidden
    bool bHidden = false;
    const SfxBoolItem* pHidItem = rReq.GetArg<SfxBoolItem>(SID_HIDDEN);
    if ( pHidItem )
        bHidden = pHidItem->GetValue();

    // This request is a UI call. We have to set the right values inside the MediaDescriptor
    // for: InteractionHandler, StatusIndicator, MacroExecutionMode and DocTemplate.
    // But we have to look for already existing values or for real hidden requests.
    const SfxBoolItem* pPreviewItem = rReq.GetArg<SfxBoolItem>(SID_PREVIEW);
    if (!bHidden && ( !pPreviewItem || !pPreviewItem->GetValue() ) )
    {
        const SfxUnoAnyItem* pInteractionItem = rReq.GetArg<SfxUnoAnyItem>(SID_INTERACTIONHANDLER);
        const SfxUInt16Item* pMacroExecItem = rReq.GetArg<SfxUInt16Item>(SID_MACROEXECMODE);
        const SfxUInt16Item* pDocTemplateItem = rReq.GetArg<SfxUInt16Item>(SID_UPDATEDOCMODE);

        if (!pInteractionItem)
        {
            Reference < task::XInteractionHandler2 > xHdl = task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr );
            rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,css::uno::makeAny(xHdl)) );
        }
        if (!pMacroExecItem)
            rReq.AppendItem( SfxUInt16Item(SID_MACROEXECMODE,css::document::MacroExecMode::USE_CONFIG) );
        if (!pDocTemplateItem)
            rReq.AppendItem( SfxUInt16Item(SID_UPDATEDOCMODE,css::document::UpdateDocMode::ACCORDING_TO_CONFIG) );
    }

    // extract target name
    OUString aTarget;
    const SfxStringItem* pTargetItem = rReq.GetArg<SfxStringItem>(SID_TARGETNAME);
    if ( pTargetItem )
        aTarget = pTargetItem->GetValue();
    else
    {
        const SfxBoolItem* pNewViewItem = rReq.GetArg<SfxBoolItem>(SID_OPEN_NEW_VIEW);
        if ( pNewViewItem && pNewViewItem->GetValue() )
            aTarget = "_blank" ;
    }

    if ( bHidden )
    {
        aTarget = "_blank";
        DBG_ASSERT( rReq.IsSynchronCall() || pLinkItem, "Hidden load process must be done synchronously!" );
    }

    Reference < XController > xController;
        // if a frame is given, it must be used for the starting point of the targeting mechanism
        // this code is also used if asynchronous loading is possible, because loadComponent always is synchron
        if ( !xTargetFrame.is() )
        {
            if ( pTargetFrame )
            {
                xTargetFrame = pTargetFrame->GetFrameInterface();
            }
            else
            {
                xTargetFrame.set( Desktop::create(::comphelper::getProcessComponentContext()), UNO_QUERY );
            }
        }

        // make URL ready
        const SfxStringItem* pURLItem = rReq.GetArg<SfxStringItem>(SID_FILE_NAME);
        aFileName = pURLItem->GetValue();
        if( aFileName.startsWith("#") ) // Mark without URL
        {
            SfxViewFrame *pView = pTargetFrame ? pTargetFrame->GetCurrentViewFrame() : nullptr;
            if ( !pView )
                pView = SfxViewFrame::Current();
            pView->GetViewShell()->JumpToMark( aFileName.copy(1) );
            rReq.SetReturnValue( SfxViewFrameItem( pView ) );
            return;
        }

        // convert items to properties for framework API calls
        Sequence < PropertyValue > aArgs;
        TransformItems( SID_OPENDOC, *rReq.GetArgs(), aArgs );
        // Any Referer (that was relevant in the above call to
        // SvtSecurityOptions::isSecureMacroUri) is no longer relevant, assuming
        // this "open" request is initiated directly by the user:
        for (sal_Int32 i = 0; i != aArgs.getLength(); ++i) {
            if (aArgs[i].Name == "Referer") {
                ++i;
                for (; i != aArgs.getLength(); ++i) {
                    aArgs[i - 1] = aArgs[i];
                }
                aArgs.realloc(aArgs.getLength()-1);
                break;
            }
        }

        // TODO/LATER: either remove LinkItem or create an asynchronous process for it
        if( bHidden || pLinkItem || rReq.IsSynchronCall() )
        {
            // if loading must be done synchron, we must wait for completion to get a return value
            // find frame by myself; I must know the exact frame to get the controller for the return value from it
            Reference < XComponent > xComp;

            try
            {
                xComp = ::comphelper::SynchronousDispatch::dispatch( xTargetFrame, aFileName, aTarget, 0, aArgs );
            }
            catch(const RuntimeException&)
            {
                throw;
            }
            catch(const css::uno::Exception&)
            {
            }

            Reference < XModel > xModel( xComp, UNO_QUERY );
            if ( xModel.is() )
                xController = xModel->getCurrentController();
            else
                xController.set( xComp, UNO_QUERY );

        }
        else
        {
            URL aURL;
            aURL.Complete = aFileName;
            Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict( aURL );

            Reference < XDispatchProvider > xProv( xTargetFrame, UNO_QUERY );
            Reference < XDispatch > xDisp = xProv.is() ? xProv->queryDispatch( aURL, aTarget, FrameSearchFlag::ALL ) : Reference < XDispatch >();
            if ( xDisp.is() )
                xDisp->dispatch( aURL, aArgs );
        }

    if ( xController.is() )
    {
        // try to find the SfxFrame for the controller
        SfxFrame* pCntrFrame = nullptr;
        for ( SfxViewShell* pShell = SfxViewShell::GetFirst( false ); pShell; pShell = SfxViewShell::GetNext( *pShell, false ) )
        {
            if ( pShell->GetController() == xController )
            {
                pCntrFrame = &pShell->GetViewFrame()->GetFrame();
                break;
            }
        }

        if ( pCntrFrame )
        {
            SfxObjectShell* pSh = pCntrFrame->GetCurrentDocument();
            DBG_ASSERT( pSh, "Controller without ObjectShell ?!" );

            rReq.SetReturnValue( SfxViewFrameItem( pCntrFrame->GetCurrentViewFrame() ) );

            if ( bHidden )
                pSh->RestoreNoDelete();
        }
    }

    if (pLinkItem)
    {
        const SfxPoolItem* pRetValue = rReq.GetReturnValue();
        if (pRetValue)
        {
            SfxPoolItem* pClone = pRetValue->Clone();
            pLinkItem->GetValue().Call(pClone);
        }
        delete pLinkItem;
    }
}

void SfxApplication::OpenRemoteExec_Impl( SfxRequest& rReq )
{
    rReq.AppendItem( SfxBoolItem( SID_REMOTE_DIALOG, true ) );
    GetDispatcher_Impl()->Execute( SID_OPENDOC, SfxCallMode::SYNCHRON, *rReq.GetArgs() );
}

void SfxApplication::SignPDFExec_Impl(SfxRequest& rReq)
{
    rReq.AppendItem(SfxBoolItem(SID_SIGNPDF, true));
    GetDispatcher_Impl()->Execute(SID_OPENDOC, SfxCallMode::SYNCHRON, *rReq.GetArgs());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
