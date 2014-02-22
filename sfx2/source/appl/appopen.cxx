/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <comphelper/synchronousdispatch.hxx>

#include <vcl/wrkwin.hxx>
#include <svl/intitem.hxx>
#include <vcl/msgbox.hxx>
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
#include <svtools/templdlg.hxx>
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
#include "templatedlg.hxx"
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



class SfxOpenDocStatusListener_Impl : public WeakImplHelper1< XDispatchResultListener >
{
public:
    sal_Bool    bFinished;
    sal_Bool    bSuccess;
    virtual void SAL_CALL   dispatchFinished( const DispatchResultEvent& Event ) throw(RuntimeException);
    virtual void SAL_CALL   disposing( const EventObject& Source ) throw(RuntimeException);
                            SfxOpenDocStatusListener_Impl()
                                : bFinished( sal_False )
                                , bSuccess( sal_False )
                            {}
};

void SAL_CALL SfxOpenDocStatusListener_Impl::dispatchFinished( const DispatchResultEvent& aEvent ) throw(RuntimeException)
{
    bSuccess = ( aEvent.State == DispatchResultState::SUCCESS );
    bFinished = sal_True;
}

void SAL_CALL SfxOpenDocStatusListener_Impl::disposing( const EventObject& ) throw(RuntimeException)
{
}



void SetTemplate_Impl( const OUString &rFileName,
                        const OUString &rLongName,
                        SfxObjectShell *pDoc)
{
    
    
    pDoc->ResetFromTemplate( rLongName, rFileName );
}


class SfxDocPasswordVerifier : public ::comphelper::IDocPasswordVerifier
{
public:
    inline explicit     SfxDocPasswordVerifier( const Reference< embed::XStorage >& rxStorage ) :
                            mxStorage( rxStorage ) {}

    virtual ::comphelper::DocPasswordVerifierResult
                        verifyPassword( const OUString& rPassword, uno::Sequence< beans::NamedValue >& o_rEncryptionData );
    virtual ::comphelper::DocPasswordVerifierResult
                        verifyEncryptionData( const uno::Sequence< beans::NamedValue >& rEncryptionData );


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
    ::comphelper::DocPasswordVerifierResult eResult = ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
    try
    {
        
        
        
        ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( mxStorage, rEncryptionData );

        mxStorage->openStreamElement(
                OUString( "content.xml"  ),
                embed::ElementModes::READ | embed::ElementModes::NOCREATE );

        
        eResult = ::comphelper::DocPasswordVerifierResult_OK;
    }
    catch( const packages::WrongPasswordException& )
    {
        eResult = ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
    }
    catch( const uno::Exception& )
    {
        
        
        eResult = ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
    }
    return eResult;
}





sal_uInt32 CheckPasswd_Impl
(
    SfxObjectShell*  pDoc,
    SfxItemPool&     /*rPool*/, 
    SfxMedium*       pFile      
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
        uno::Reference< embed::XStorage > xStorage = pFile->GetStorage( sal_True );
        if( xStorage.is() )
        {
            uno::Reference< beans::XPropertySet > xStorageProps( xStorage, uno::UNO_QUERY );
            if ( xStorageProps.is() )
            {
                sal_Bool bIsEncrypted = sal_False;
                try {
                    xStorageProps->getPropertyValue("HasEncryptedEntries")
                        >>= bIsEncrypted;
                } catch( uno::Exception& )
                {
                    
                    
                    
                }

                if ( bIsEncrypted )
                {
                    Window* pWin = pDoc ? pDoc->GetDialogParent( pFile ) : NULL;
                    if ( pWin )
                        pWin->Show();

                    nRet = ERRCODE_SFX_CANTGETPASSWD;

                    SfxItemSet *pSet = pFile->GetItemSet();
                    if( pSet )
                    {
                        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = pFile->GetInteractionHandler();
                        if( xInteractionHandler.is() )
                        {
                            
                            OUString aPassword;
                            SFX_ITEMSET_ARG( pSet, pPasswordItem, SfxStringItem, SID_PASSWORD, false);
                            if ( pPasswordItem )
                                aPassword = pPasswordItem->GetValue();

                            uno::Sequence< beans::NamedValue > aEncryptionData;
                            SFX_ITEMSET_ARG( pSet, pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, false);
                            if ( pEncryptionDataItem )
                                pEncryptionDataItem->GetValue() >>= aEncryptionData;

                            OUString aDocumentName = INetURLObject( pFile->GetOrigURL() ).GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

                            SfxDocPasswordVerifier aVerifier( xStorage );
                            aEncryptionData = ::comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
                                aVerifier, aEncryptionData, aPassword, xInteractionHandler, aDocumentName, comphelper::DocPasswordRequestType_STANDARD );

                            pSet->ClearItem( SID_PASSWORD );
                            pSet->ClearItem( SID_ENCRYPTIONDATA );

                            if ( aEncryptionData.getLength() > 0 )
                            {
                                pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) ) );

                                try
                                {
                                    
                                    pFile->GetVersionList();
                                }
                                catch( uno::Exception& )
                                {
                                    
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




sal_uIntPtr SfxApplication::LoadTemplate( SfxObjectShellLock& xDoc, const OUString &rFileName, sal_Bool bCopy, SfxItemSet* pSet )
{
    const SfxFilter* pFilter = NULL;
    SfxMedium aMedium( rFileName,  ( STREAM_READ | STREAM_SHARE_DENYNONE ) );

    if ( !aMedium.GetStorage( sal_False ).is() )
        aMedium.GetInStream();

    if ( aMedium.GetError() )
    {
        delete pSet;
        return aMedium.GetErrorCode();
    }

    aMedium.UseInteractionHandler( sal_True );
    sal_uIntPtr nErr = GetFilterMatcher().GuessFilter( aMedium,&pFilter,SFX_FILTER_TEMPLATE, 0 );
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

    if ( pFilter->GetFilterFlags() & SFX_FILTER_STARONEFILTER )
    {
        DBG_ASSERT( !xDoc.Is(), "Sorry, not implemented!" );
        delete pSet;
        SfxStringItem aName( SID_FILE_NAME, rFileName );
        SfxStringItem aReferer( SID_REFERER, OUString("private:user") );
        SfxStringItem aFlags( SID_OPTIONS, OUString("T") );
        SfxBoolItem aHidden( SID_HIDDEN, true );
        const SfxPoolItem *pRet = GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, &aName, &aHidden, &aReferer, &aFlags, 0L );
        const SfxObjectItem *pObj = PTR_CAST( SfxObjectItem, pRet );
        if ( pObj )
            xDoc = PTR_CAST( SfxObjectShell, pObj->GetShell() );
        else
        {
            const SfxViewFrameItem *pView = PTR_CAST( SfxViewFrameItem, pRet );
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

        
        SfxMedium *pMedium = new SfxMedium( rFileName, STREAM_STD_READ, pFilter, pSet );
        if(!xDoc->DoLoad(pMedium))
        {
            ErrCode nErrCode = xDoc->GetErrorCode();
            xDoc->DoClose();
            xDoc.Clear();
            return nErrCode;
        }
    }

    if( bCopy )
    {
        try
        {
            

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

            
            return ERRCODE_SFX_GENERAL;
        }

        SetTemplate_Impl( rFileName, OUString(), xDoc );
    }
    else
        SetTemplate_Impl( rFileName, OUString(), xDoc );

    xDoc->SetNoName();
    xDoc->InvalidateName();
    xDoc->SetModified(sal_False);
    xDoc->ResetError();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( xDoc->GetModel(), ::com::sun::star::uno::UNO_QUERY );
    if ( xModel.is() )
    {
        SfxItemSet* pNew = xDoc->GetMedium()->GetItemSet()->Clone();
        pNew->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        pNew->ClearItem( SID_FILTER_NAME );
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
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
    SFX_REQUEST_ARG( rReq, pFactoryItem, SfxStringItem, SID_NEWDOCDIRECT, false);
    OUString aFactName;
    if ( pFactoryItem )
        aFactName = pFactoryItem->GetValue();
   else
        aFactName = SvtModuleOptions().GetDefaultModuleName();


    SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, GetPool() );
    OUString aFact("private:factory/");
    aFact += aFactName;
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aFact ) );
    aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, GetFrame() ) );
    aReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString( "_default" ) ) );

    
    SFX_REQUEST_ARG( rReq, pDefaultPathItem, SfxStringItem, SID_DEFAULTFILEPATH, false);
    if ( pDefaultPathItem )
        aReq.AppendItem( *pDefaultPathItem );
    SFX_REQUEST_ARG( rReq, pDefaultNameItem, SfxStringItem, SID_DEFAULTFILENAME, false);
    if ( pDefaultNameItem )
        aReq.AppendItem( *pDefaultNameItem );

    SFX_APP()->ExecuteSlot( aReq );
    const SfxViewFrameItem* pItem = PTR_CAST( SfxViewFrameItem, aReq.GetReturnValue() );
    if ( pItem )
        rReq.SetReturnValue( SfxFrameItem( 0, pItem->GetFrame() ) );
}



void SfxApplication::NewDocExec_Impl( SfxRequest& rReq )
{
    
    SFX_REQUEST_ARG(rReq, pTemplNameItem, SfxStringItem, SID_TEMPLATE_NAME, false);
    SFX_REQUEST_ARG(rReq, pTemplFileNameItem, SfxStringItem, SID_FILE_NAME, false);
    SFX_REQUEST_ARG(rReq, pTemplRegionNameItem, SfxStringItem, SID_TEMPLATE_REGIONNAME, false);

    SfxObjectShellLock xDoc;

    OUString  aTemplateRegion, aTemplateName, aTemplateFileName;
    sal_Bool    bDirect = sal_False; 
    SfxErrorContext aEc(ERRCTX_SFX_NEWDOC);
    if ( !pTemplNameItem && !pTemplFileNameItem )
    {
        sal_Bool bNewWin = sal_False;
        Window* pTopWin = GetTopWindow();

        SfxTemplateManagerDlg aTemplDlg;
        int nRet = aTemplDlg.Execute();
        if ( nRet == RET_OK )
        {
            rReq.Done();
            if ( pTopWin != GetTopWindow() )
            {
                
                pTopWin = GetTopWindow();
                bNewWin = sal_True;
            }
        }

        if ( bNewWin && pTopWin )
            
            
            pTopWin->ToTop();

        return;
    }
    else
    {
        
        if ( pTemplNameItem )
            aTemplateName = pTemplNameItem->GetValue();

        
        if ( pTemplRegionNameItem )
            aTemplateRegion = pTemplRegionNameItem->GetValue();

        
        if ( pTemplFileNameItem )
        {
            aTemplateFileName = pTemplFileNameItem->GetValue();
            bDirect = sal_True;
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
        sal_uIntPtr lFatalErr = ERRCODE_TOERROR(lErr);
        if ( lFatalErr )
            ErrorHandler::HandleError(lErr);
    }
    else
    {
        SfxCallMode eMode = SFX_CALLMODE_SYNCHRON;

        const SfxPoolItem *pRet=0;
        SfxStringItem aReferer( SID_REFERER, "private:user" );
        SfxStringItem aTarget( SID_TARGETNAME, "_default" );
        if ( !aTemplateFileName.isEmpty() )
        {
            DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );

            SfxStringItem aName( SID_FILE_NAME, aObj.GetMainURL( INetURLObject::NO_DECODE ) );
            SfxStringItem aTemplName( SID_TEMPLATE_NAME, aTemplateName );
            SfxStringItem aTemplRegionName( SID_TEMPLATE_REGIONNAME, aTemplateRegion );
            pRet = GetDispatcher_Impl()->Execute( SID_OPENDOC, eMode, &aName, &aTarget, &aReferer, &aTemplName, &aTemplRegionName, 0L );
        }
        else
        {
            SfxStringItem aName( SID_FILE_NAME, "private:factory" );
            pRet = GetDispatcher_Impl()->Execute( SID_OPENDOC, eMode, &aName, &aTarget, &aReferer, 0L );
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

    OUString aName = rFilter.GetFilterName();
    if (aName.startsWith("MS Excel"))
        
        return true;

    return false;
}

}

void SfxApplication::OpenDocExec_Impl( SfxRequest& rReq )
{
    OUString aDocService;
    SFX_REQUEST_ARG(rReq, pDocSrvItem, SfxStringItem, SID_DOC_SERVICE, false);
    if (pDocSrvItem)
        aDocService = pDocSrvItem->GetValue();

    sal_uInt16 nSID = rReq.GetSlot();
    SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, false );
    if ( pFileNameItem )
    {
        OUString aCommand( pFileNameItem->GetValue() );
        const SfxSlot* pSlot = GetInterface()->GetSlot( aCommand );
        if ( pSlot )
        {
            pFileNameItem = NULL;
        }
        else
        {
            if ( aCommand.startsWith("slot:") )
            {
                sal_uInt16 nSlotId = (sal_uInt16) aCommand.copy(5).toInt32();
                if ( nSlotId == SID_OPENDOC )
                    pFileNameItem = NULL;
            }
        }
    }

    if ( !pFileNameItem )
    {
        
        std::vector<OUString> pURLList;
        OUString aFilter;
        SfxItemSet* pSet = NULL;
        OUString aPath;
        SFX_REQUEST_ARG( rReq, pFolderNameItem, SfxStringItem, SID_PATH, false );
        if ( pFolderNameItem )
            aPath = pFolderNameItem->GetValue();
        else if ( nSID == SID_OPENTEMPLATE )
        {
            aPath = SvtPathOptions().GetTemplatePath();
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(aPath, ';');
            aPath = aPath.getToken( nTokenCount ? ( nTokenCount - 1 ) : 0 , ';' );
        }

        sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG;
        SFX_REQUEST_ARG( rReq, pSystemDialogItem, SfxBoolItem, SID_FILE_DIALOG, false );
        if ( pSystemDialogItem )
            nDialog = pSystemDialogItem->GetValue() ? SFX2_IMPL_DIALOG_SYSTEM : SFX2_IMPL_DIALOG_OOO;

        OUString sStandardDir;

        SFX_REQUEST_ARG( rReq, pStandardDirItem, SfxStringItem, SID_STANDARD_DIR, false );
        if ( pStandardDirItem )
            sStandardDir = pStandardDirItem->GetValue();

        ::com::sun::star::uno::Sequence< OUString >  aBlackList;

        SFX_REQUEST_ARG( rReq, pBlackListItem, SfxStringListItem, SID_BLACK_LIST, false );
        if ( pBlackListItem )
            pBlackListItem->GetStringList( aBlackList );


        sal_uIntPtr nErr = sfx2::FileOpenDialog_Impl(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                SFXWB_MULTISELECTION, OUString(), pURLList,
                aFilter, pSet, &aPath, nDialog, sStandardDir, aBlackList );

        if ( nErr == ERRCODE_ABORT )
        {
            pURLList.clear();
            return;
        }

        rReq.SetArgs( *(SfxAllItemSet*)pSet );
        if ( !aFilter.isEmpty() )
            rReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilter ) );
        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_default") ) );
        rReq.AppendItem( SfxStringItem( SID_REFERER, "private:user" ) );
        delete pSet;

        if(!pURLList.empty())
        {
            if ( nSID == SID_OPENTEMPLATE )
                rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, false ) );

            
            
            

            ::framework::PreventDuplicateInteraction*                 pHandler       = new ::framework::PreventDuplicateInteraction(::comphelper::getProcessComponentContext());
            css::uno::Reference< css::task::XInteractionHandler >     xHandler       (static_cast< css::task::XInteractionHandler* >(pHandler), css::uno::UNO_QUERY);
            css::uno::Reference< css::task::XInteractionHandler >     xWrappedHandler;

            
            SFX_REQUEST_ARG(rReq, pInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, false);
            if (pInteractionItem)
            {
                pInteractionItem->GetValue() >>= xWrappedHandler;
                rReq.RemoveItem( SID_INTERACTIONHANDLER );
            }
            if (xWrappedHandler.is())
                pHandler->setHandler(xWrappedHandler);
            else
                pHandler->useDefaultUUIHandler();
            rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHandler)) );

            
            css::uno::Type                                            aInteraction = ::getCppuType(static_cast< css::task::ErrorCodeRequest* >(0));
            ::framework::PreventDuplicateInteraction::InteractionInfo aRule        (aInteraction, 1);
            pHandler->addInteractionRule(aRule);

            if (!aDocService.isEmpty())
            {
                rReq.RemoveItem(SID_DOC_SERVICE);
                rReq.AppendItem(SfxStringItem(SID_DOC_SERVICE, aDocService));
            }

            for(std::vector<OUString>::const_iterator i = pURLList.begin(); i != pURLList.end(); ++i)
            {
                rReq.RemoveItem( SID_FILE_NAME );
                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, *i ) );

                
                
                
                
                GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, *rReq.GetArgs() );

                
                
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

            pURLList.clear();
            return;
        }
        pURLList.clear();
    }

    sal_Bool bHyperlinkUsed = sal_False;

    if ( SID_OPENURL == nSID )
    {
        
        rReq.SetSlot( SID_OPENDOC );
        nSID = SID_OPENDOC;
    }
    else if ( nSID == SID_OPENTEMPLATE )
    {
        rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, false ) );
    }
    
    
    /* Attention!
            There exist two possibilities to open hyperlinks:
            a) using SID_OPENHYPERLINK (new)
            b) using SID_BROWSE        (old)
     */
    else if ( nSID == SID_OPENHYPERLINK )
    {
        rReq.SetSlot( SID_OPENDOC );
        nSID = SID_OPENDOC;
        bHyperlinkUsed = sal_True;
    }

    
    if (!bHyperlinkUsed)
    {
        SFX_REQUEST_ARG(rReq, pHyperLinkUsedItem, SfxBoolItem, SID_BROWSE, false);
        if ( pHyperLinkUsedItem )
            bHyperlinkUsed = pHyperLinkUsedItem->GetValue();
        
        rReq.RemoveItem( SID_BROWSE );
    }

    SFX_REQUEST_ARG( rReq, pFileName, SfxStringItem, SID_FILE_NAME, false );
    OUString aFileName = pFileName->GetValue();

    OUString aReferer;
    SFX_REQUEST_ARG( rReq, pRefererItem, SfxStringItem, SID_REFERER, false );
    if ( pRefererItem )
        aReferer = pRefererItem->GetValue();

    SFX_REQUEST_ARG( rReq, pFileFlagsItem, SfxStringItem, SID_OPTIONS, false);
    if ( pFileFlagsItem )
    {
        OUString aFileFlags = pFileFlagsItem->GetValue();
        aFileFlags = aFileFlags.toAsciiUpperCase();
        if ( -1 != aFileFlags.indexOf( 0x0054 ) )               
        {
            rReq.RemoveItem( SID_TEMPLATE );
            rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, true ) );
        }

        if ( -1 != aFileFlags.indexOf( 0x0048 ) )               
        {
            rReq.RemoveItem( SID_HIDDEN );
            rReq.AppendItem( SfxBoolItem( SID_HIDDEN, true ) );
        }

        if ( -1 != aFileFlags.indexOf( 0x0052 ) )               
        {
            rReq.RemoveItem( SID_DOC_READONLY );
            rReq.AppendItem( SfxBoolItem( SID_DOC_READONLY, true ) );
        }

        if ( -1 != aFileFlags.indexOf( 0x0042 ) )               
        {
            rReq.RemoveItem( SID_PREVIEW );
            rReq.AppendItem( SfxBoolItem( SID_PREVIEW, true ) );
        }

        rReq.RemoveItem( SID_OPTIONS );
    }

    
    if ( bHyperlinkUsed && !aFileName.isEmpty() && aFileName[0] != '#' )
    {
        Reference< ::com::sun::star::document::XTypeDetection > xTypeDetection(
                                                                    ::comphelper::getProcessServiceFactory()->createInstance(
                                                                    OUString("com.sun.star.document.TypeDetection")),
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

            if ( eMode == SvtExtendedSecurityOptions::OPEN_NEVER && aINetProtocol != INET_PROT_VND_SUN_STAR_HELP )
            {
                SolarMutexGuard aGuard;
                Window *pWindow = SFX_APP()->GetTopWindow();

                WarningBox  aSecurityWarningBox( pWindow, SfxResId( RID_SECURITY_WARNING_NO_HYPERLINKS ));
                aSecurityWarningBox.SetText( SfxResId(RID_SECURITY_WARNING_TITLE).toString() );
                aSecurityWarningBox.Execute();
                return;
            }

            aTypeName = xTypeDetection->queryTypeByURL( aURL.Main );
            SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
            const SfxFilter* pFilter = rMatcher.GetFilter4EA( aTypeName );
            if (!pFilter || !lcl_isFilterNativelySupported(*pFilter))
            {
                
                if ( aINetProtocol == INET_PROT_MAILTO )
                {
                    
                    rReq.RemoveItem( SID_TARGETNAME );
                    rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_self") ) );
                }
                else if ( aINetProtocol == INET_PROT_FTP ||
                     aINetProtocol == INET_PROT_HTTP ||
                     aINetProtocol == INET_PROT_HTTPS )
                {
                    sfx2::openUriExternally(aURL.Complete, true);
                    return;
                }
                else
                {
                    
                    Sequence < OUString > aProtocols(2);

                    
                    aProtocols[0] = "private:*";
                    aProtocols[1] = "vnd.sun.star.*";

                    
                    Reference < XNameAccess > xAccess(officecfg::Office::ProtocolHandler::HandlerSet::get());
                    Sequence < OUString > aNames = xAccess->getElementNames();
                    for ( sal_Int32 nName = 0; nName < aNames.getLength(); nName ++)
                    {
                        Reference < XPropertySet > xSet;
                        Any aRet = xAccess->getByName( aNames[nName] );
                        aRet >>= xSet;
                        if ( xSet.is() )
                        {
                            
                            aRet = xSet->getPropertyValue("Protocols");
                            Sequence < OUString > aTmp;
                            aRet >>= aTmp;

                            
                            sal_Int32 nLength = aProtocols.getLength();
                            aProtocols.realloc( nLength+aTmp.getLength() );
                            for ( sal_Int32 n=0; n<aTmp.getLength(); n++ )
                                aProtocols[(++nLength)-1] = aTmp[n];
                        }
                    }

                    sal_Bool bFound = sal_False;
                    for ( sal_Int32 nProt=0; nProt<aProtocols.getLength(); nProt++ )
                    {
                        WildCard aPattern(aProtocols[nProt]);
                        if ( aPattern.Matches( aURL.Complete ) )
                        {
                            bFound = sal_True;
                            break;
                        }
                    }

                    if ( !bFound )
                    {
                        sal_Bool bLoadInternal = sal_False;
                        try
                        {
                            sfx2::openUriExternally(
                                aURL.Complete, pFilter == 0);
                        }
                        catch ( ::com::sun::star::system::SystemShellExecuteException& )
                        {
                            rReq.RemoveItem( SID_TARGETNAME );
                            rReq.AppendItem( SfxStringItem( SID_TARGETNAME, OUString("_default") ) );
                            bLoadInternal = sal_True;
                        }
                        if ( !bLoadInternal )
                            return;
                    }
                }
            }
            else
            {
                
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

    SfxFrame* pTargetFrame = NULL;
    Reference< XFrame > xTargetFrame;

    SFX_REQUEST_ARG(rReq, pFrameItem, SfxFrameItem, SID_DOCFRAME, false);
    if ( pFrameItem )
        pTargetFrame = pFrameItem->GetFrame();

    if ( !pTargetFrame )
    {
        SFX_REQUEST_ARG(rReq, pUnoFrameItem, SfxUnoFrameItem, SID_FILLFRAME, false);
        if ( pUnoFrameItem )
            xTargetFrame = pUnoFrameItem->GetFrame();
    }

    if ( !pTargetFrame && !xTargetFrame.is() && SfxViewFrame::Current() )
        pTargetFrame = &SfxViewFrame::Current()->GetFrame();

    
    SFX_REQUEST_ARG(rReq, pLinkItem, SfxLinkItem, SID_DONELINK, false );

    
    if ( pLinkItem )
        pLinkItem = (SfxLinkItem*) pLinkItem->Clone();

    rReq.RemoveItem( SID_DONELINK );

    
    sal_Bool bHidden = sal_False;
    SFX_REQUEST_ARG(rReq, pHidItem, SfxBoolItem, SID_HIDDEN, false);
    if ( pHidItem )
        bHidden = pHidItem->GetValue();

    
    
    
    SFX_REQUEST_ARG(rReq, pPreviewItem, SfxBoolItem, SID_PREVIEW, false);
    if (!bHidden && ( !pPreviewItem || !pPreviewItem->GetValue() ) )
    {
        SFX_REQUEST_ARG(rReq, pInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, false);
        SFX_REQUEST_ARG(rReq, pMacroExecItem  , SfxUInt16Item, SID_MACROEXECMODE     , false);
        SFX_REQUEST_ARG(rReq, pDocTemplateItem, SfxUInt16Item, SID_UPDATEDOCMODE     , false);

        if (!pInteractionItem)
        {
            Reference < task::XInteractionHandler2 > xHdl = task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), 0 );
            rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHdl)) );
        }
        if (!pMacroExecItem)
            rReq.AppendItem( SfxUInt16Item(SID_MACROEXECMODE,::com::sun::star::document::MacroExecMode::USE_CONFIG) );
        if (!pDocTemplateItem)
            rReq.AppendItem( SfxUInt16Item(SID_UPDATEDOCMODE,::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG) );
    }

    
    OUString aTarget;
    SFX_REQUEST_ARG(rReq, pTargetItem, SfxStringItem, SID_TARGETNAME, false);
    if ( pTargetItem )
        aTarget = pTargetItem->GetValue();
    else
    {
        SFX_REQUEST_ARG( rReq, pNewViewItem, SfxBoolItem, SID_OPEN_NEW_VIEW, false );
        if ( pNewViewItem && pNewViewItem->GetValue() )
            aTarget = "_blank" ;
    }

    if ( bHidden )
    {
        aTarget = "_blank";
        DBG_ASSERT( rReq.IsSynchronCall() || pLinkItem, "Hidden load process must be done synchronously!" );
    }

    Reference < XController > xController;
        
        
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

        
        SFX_REQUEST_ARG( rReq, pURLItem, SfxStringItem, SID_FILE_NAME, false );
        aFileName = pURLItem->GetValue();
        if( aFileName.startsWith("#") ) 
        {
            SfxViewFrame *pView = pTargetFrame ? pTargetFrame->GetCurrentViewFrame() : 0;
            if ( !pView )
                pView = SfxViewFrame::Current();
            pView->GetViewShell()->JumpToMark( aFileName.copy(1) );
            rReq.SetReturnValue( SfxViewFrameItem( 0, pView ) );
            return;
        }

        
        Sequence < PropertyValue > aArgs;
        TransformItems( SID_OPENDOC, *rReq.GetArgs(), aArgs );
        
        
        
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

        
        if( bHidden || pLinkItem || rReq.IsSynchronCall() )
        {
            
            
            Reference < XComponent > xComp;

            try
            {
                xComp = ::comphelper::SynchronousDispatch::dispatch( xTargetFrame, aFileName, aTarget, 0, aArgs );
            }
            catch(const RuntimeException&)
            {
                throw;
            }
            catch(const ::com::sun::star::uno::Exception&)
            {
            }

            Reference < XModel > xModel( xComp, UNO_QUERY );
            if ( xModel.is() )
                xController = xModel->getCurrentController();
            else
                xController = Reference < XController >( xComp, UNO_QUERY );

        }
        else
        {
            URL aURL;
            aURL.Complete = aFileName;
            Reference< util::XURLTransformer > xTrans( util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
            xTrans->parseStrict( aURL );

            Reference < XDispatchProvider > xProv( xTargetFrame, UNO_QUERY );
            Reference < XDispatch > xDisp = xProv.is() ? xProv->queryDispatch( aURL, aTarget, FrameSearchFlag::ALL ) : Reference < XDispatch >();;
            SAL_INFO( "sfx.appl", "PERFORMANCE - SfxApplication::OpenDocExec_Impl" );
            if ( xDisp.is() )
                xDisp->dispatch( aURL, aArgs );
        }

    if ( xController.is() )
    {
        
        SfxFrame* pCntrFrame = NULL;
        for ( SfxViewShell* pShell = SfxViewShell::GetFirst( 0, sal_False ); pShell; pShell = SfxViewShell::GetNext( *pShell, 0, sal_False ) )
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

            rReq.SetReturnValue( SfxViewFrameItem( 0, pCntrFrame->GetCurrentViewFrame() ) );

            if ( bHidden )
                pSh->RestoreNoDelete();
        }
    }

    if ( pLinkItem )
    {
        SfxPoolItem* pRet = rReq.GetReturnValue()->Clone();
        pLinkItem->GetValue().Call(pRet);
        delete pLinkItem;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
