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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/SimpleSystemMail.hpp>
#include <com/sun/star/system/SimpleCommandMail.hpp>
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

#include <sfx2/mailmodelapi.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <unotools/useroptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/implbase.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;

namespace {

// - class PrepareListener_Impl ------------------------------------------
class PrepareListener_Impl : public ::cppu::WeakImplHelper< css::frame::XStatusListener >
{
    bool m_bState;
public:
        PrepareListener_Impl();

        // css.frame.XStatusListener
        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& aEvent) override;

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

        bool IsSet() const {return m_bState;}
};

}

PrepareListener_Impl::PrepareListener_Impl() :
    m_bState( false )
{
}

void PrepareListener_Impl::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    if( rEvent.IsEnabled )
        rEvent.State >>= m_bState;
    else
        m_bState = false;
}

void PrepareListener_Impl::disposing(const css::lang::EventObject& /*rEvent*/)
{
}

// class SfxMailModel -----------------------------------------------

const char16_t   PDF_DOCUMENT_TYPE[]   = u"pdf_Portable_Document_Format";

SfxMailModel::SaveResult SfxMailModel::ShowFilterOptionsDialog(
    const uno::Reference< lang::XMultiServiceFactory >& xSMGR,
    const uno::Reference< frame::XModel >& xModel,
    const OUString& rFilterName,
    std::u16string_view rType,
    bool bModified,
    sal_Int32& rNumArgs,
    css::uno::Sequence< css::beans::PropertyValue >& rArgs )
{
    SaveResult eRet( SAVE_ERROR );

    try
    {
        uno::Sequence < beans::PropertyValue > aProps;
        css::uno::Reference< css::container::XNameAccess > xFilterCFG(
                    xSMGR->createInstance( "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );
        css::uno::Reference< css::util::XModifiable > xModifiable( xModel, css::uno::UNO_QUERY );

        if ( !xFilterCFG.is() )
            return eRet;

        uno::Any aAny = xFilterCFG->getByName( rFilterName );

        if ( aAny >>= aProps )
        {
            for (const auto& rProp : aProps)
            {
                if( rProp.Name == "UIComponent" )
                {
                    OUString aServiceName;
                    rProp.Value >>= aServiceName;
                    if( !aServiceName.isEmpty() )
                    {
                        uno::Reference< ui::dialogs::XExecutableDialog > xFilterDialog(
                            xSMGR->createInstance( aServiceName ), uno::UNO_QUERY );
                        uno::Reference< beans::XPropertyAccess > xFilterProperties(
                            xFilterDialog, uno::UNO_QUERY );

                        if( xFilterDialog.is() && xFilterProperties.is() )
                        {
                            uno::Reference< document::XExporter > xExporter( xFilterDialog, uno::UNO_QUERY );

                            if ( rType == PDF_DOCUMENT_TYPE )
                            {
                                //add an internal property, used to tell the dialog we want to set a different
                                //string for the ok button
                                //used in filter/source/pdf/impdialog.cxx
                                uno::Sequence< beans::PropertyValue > aFilterDataValue{
                                    comphelper::makePropertyValue("_OkButtonString",
                                                                  SfxResId(STR_PDF_EXPORT_SEND ))
                                };

                                //add to the filterdata property, the only one the PDF export filter dialog will care for
                                uno::Sequence< beans::PropertyValue > aPropsForDialog{
                                    comphelper::makePropertyValue("FilterData", aFilterDataValue)
                                };

                                //when executing the dialog will merge the persistent FilterData properties
                                xFilterProperties->setPropertyValues( aPropsForDialog );
                            }

                            if( xExporter.is() )
                                xExporter->setSourceDocument( xModel );

                            if( xFilterDialog->execute() )
                            {
                                //get the filter data
                                const uno::Sequence< beans::PropertyValue > aPropsFromDialog = xFilterProperties->getPropertyValues();

                                //add them to the args
                                auto pProp = std::find_if(aPropsFromDialog.begin(), aPropsFromDialog.end(),
                                    [](const beans::PropertyValue& rDialogProp) { return rDialogProp.Name == "FilterData"; });
                                if (pProp != aPropsFromDialog.end())
                                {
                                    //found the filterdata, add to the storing argument
                                    rArgs.realloc( ++rNumArgs );
                                    auto pArgs = rArgs.getArray();
                                    pArgs[rNumArgs-1].Name = pProp->Name;
                                    pArgs[rNumArgs-1].Value = pProp->Value;
                                }
                                eRet = SAVE_SUCCESSFUL;
                            }
                            else
                            {
                                // cancel from dialog, then do not send
                                // If the model is not modified, it could be modified by the dispatch calls.
                                // Therefore set back to modified = false. This should not hurt if we call
                                // on a non-modified model.
                                if ( !bModified )
                                {
                                    try
                                    {
                                        xModifiable->setModified( false );
                                    }
                                    catch( css::beans::PropertyVetoException& )
                                    {
                                    }
                                }
                                eRet = SAVE_CANCELLED;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    catch( css::uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
    }

    return eRet;
}

bool SfxMailModel::IsEmpty() const
{
    return maAttachedDocuments.empty();
}

SfxMailModel::SaveResult SfxMailModel::SaveDocumentAsFormat(
    const OUString& aSaveFileName,
    const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
    const OUString& rType,
    OUString& rFileNamePath )
{
    SaveResult  eRet( SAVE_ERROR );
    bool        bSendAsPDF = ( rType == PDF_DOCUMENT_TYPE );

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = ::comphelper::getProcessServiceFactory();
    css::uno::Reference< css::uno::XComponentContext > xContext  = ::comphelper::getProcessComponentContext();
    if (!xContext.is())
        return eRet;

    css::uno::Reference< css::frame::XModuleManager2 > xModuleManager( css::frame::ModuleManager::create(xContext) );

    OUString aModule;
    try
    {
         aModule = xModuleManager->identify( xFrameOrModel );
    }
    catch ( css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }

    css::uno::Reference< css::frame::XFrame > xFrame( xFrameOrModel, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XModel > xModel( xFrameOrModel, css::uno::UNO_QUERY );
    if ( xFrame.is() )
    {
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }

    // We need at least a valid module name and model reference
    if ( !aModule.isEmpty()  && xModel.is() )
    {
        bool bModified( false );
        bool bHasLocation( false );
        bool bStoreTo( false );

        css::uno::Reference< css::util::XModifiable > xModifiable( xModel, css::uno::UNO_QUERY );
        css::uno::Reference< css::frame::XStorable > xStorable( xModel, css::uno::UNO_QUERY );

        if ( xModifiable.is() )
            bModified = xModifiable->isModified();
        if ( xStorable.is() )
        {
            OUString aLocation = xStorable->getLocation();
            INetURLObject aFileObj( aLocation );

            bool bPrivateProtocol = ( aFileObj.GetProtocol() == INetProtocol::PrivSoffice );

            bHasLocation =  !aLocation.isEmpty() && !bPrivateProtocol;
            OSL_ASSERT( !bPrivateProtocol );
        }
        if ( !rType.isEmpty() )
            bStoreTo = true;

        if ( xStorable.is() )
        {
            OUString aFilterName;
            OUString aTypeName( rType );
            OUString aFileName;
            OUString aExtension;

            css::uno::Reference< css::container::XContainerQuery > xContainerQuery(
                xSMGR->createInstance( "com.sun.star.document.FilterFactory" ),
                css::uno::UNO_QUERY );

            if ( bStoreTo )
            {
                // Retrieve filter from type
                css::uno::Sequence< css::beans::NamedValue > aQuery( bSendAsPDF ? 3 : 2 );
                auto pQuery = aQuery.getArray();
                pQuery[0].Name  = "Type";
                pQuery[0].Value <<= aTypeName;
                pQuery[1].Name  = "DocumentService";
                pQuery[1].Value <<= aModule;
                if( bSendAsPDF )
                {
                    // #i91419#
                    // FIXME: we want just an export filter. However currently we need
                    // exact flag value as detailed in the filter configuration to get it
                    // this seems to be a bug
                    // without flags we get an import filter here, which is also unwanted
                    pQuery[2].Name  = "Flags";
                    pQuery[2].Value <<= sal_Int32(0x80042); // SfxFilterFlags: EXPORT ALIEN 3RDPARTY
                }

                css::uno::Reference< css::container::XEnumeration > xEnumeration =
                    xContainerQuery->createSubSetEnumerationByProperties( aQuery );

                if ( xEnumeration->hasMoreElements() )
                {
                    ::comphelper::SequenceAsHashMap aFilterPropsHM( xEnumeration->nextElement() );
                    aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                "Name",
                                                OUString() );
                }

                if ( bHasLocation )
                {
                    // Retrieve filter from media descriptor
                    ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
                    OUString aOrgFilterName = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                    "FilterName",
                                    OUString() );
                    if ( aOrgFilterName == aFilterName )
                    {
                        // We should save the document in the original format. Therefore this
                        // is not a storeTo operation. To support signing in this case, reset
                        // bStoreTo flag.
                        bStoreTo = false;
                    }
                }
            }
            else
            {
                if ( bHasLocation )
                {
                    // Retrieve filter from media descriptor
                    ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
                    aFilterName = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                    "FilterName",
                                    OUString() );
                }

                if ( !bHasLocation ||  aFilterName.isEmpty())
                {
                    // Retrieve the user defined default filter
                    try
                    {
                        ::comphelper::SequenceAsHashMap aFilterPropsHM( xModuleManager->getByName( aModule ) );
                        aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                    "ooSetupFactoryDefaultFilter",
                                                    OUString() );
                        css::uno::Reference< css::container::XNameAccess > xNameAccess(
                            xContainerQuery, css::uno::UNO_QUERY );
                        if ( xNameAccess.is() )
                        {
                            ::comphelper::SequenceAsHashMap aFilterPropsHM2( xNameAccess->getByName( aFilterName ) );
                            aTypeName = aFilterPropsHM2.getUnpackedValueOrDefault(
                                                        "Type",
                                                        OUString() );
                        }
                    }
                    catch ( css::container::NoSuchElementException& )
                    {
                    }
                    catch ( css::beans::UnknownPropertyException& )
                    {
                    }
                }
            }

            // No filter found => error
            // No type and no location => error
            if (( aFilterName.isEmpty() ) ||
                ( aTypeName.isEmpty()  && !bHasLocation ))
                return eRet;

            // Determine file name and extension
            if ( bHasLocation && !bStoreTo )
            {
                INetURLObject aFileObj( xStorable->getLocation() );
                aExtension = aFileObj.getExtension();
            }
            else
            {
                css::uno::Reference< container::XNameAccess > xTypeDetection(
                    xSMGR->createInstance( "com.sun.star.document.TypeDetection" ),
                    css::uno::UNO_QUERY );


                if ( xTypeDetection.is() )
                {
                    try
                    {
                        ::comphelper::SequenceAsHashMap aTypeNamePropsHM( xTypeDetection->getByName( aTypeName ) );
                        uno::Sequence< OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                                        "Extensions",
                                                        ::uno::Sequence< OUString >() );
                        if ( aExtensions.hasElements() )
                            aExtension = aExtensions[0];
                    }
                    catch ( css::container::NoSuchElementException& )
                    {
                    }
                }
            }

            // Use provided save file name. If empty determine file name
            aFileName = aSaveFileName;
            if ( aFileName.isEmpty() )
            {
                if ( !bHasLocation )
                {
                    // Create a noname file name with the correct extension
                    aFileName = "noname";
                }
                else
                {
                    // Determine file name from model
                    INetURLObject aFileObj( xStorable->getLocation() );
                    aFileName = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::NONE );
                }
            }

            // No file name => error
            if ( aFileName.isEmpty() )
                return eRet;

            OSL_ASSERT( !aFilterName.isEmpty() );
            OSL_ASSERT( !aFileName.isEmpty() );

            // Creates a temporary directory to store a predefined file into it.
            // This makes it possible to store the file for "send document as e-mail"
            // with the original file name. We cannot use the original file as
            // some mail programs need exclusive access.
            INetURLObject aFilePathObj( ::utl::CreateTempURL(nullptr, true) );
            aFilePathObj.insertName( aFileName );
            aFilePathObj.setExtension( aExtension );

            OUString aFileURL = aFilePathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            sal_Int32 nNumArgs(1);
            static constexpr OUString aPasswordPropName( u"Password"_ustr );
            css::uno::Sequence< css::beans::PropertyValue > aArgs{ comphelper::makePropertyValue(
                "FilterName", aFilterName) };

            ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
            OUString aPassword = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                            aPasswordPropName,
                                            OUString() );
            if ( !aPassword.isEmpty() )
            {
                aArgs.realloc( ++nNumArgs );
                auto pArgs = aArgs.getArray();
                pArgs[nNumArgs-1].Name = aPasswordPropName;
                pArgs[nNumArgs-1].Value <<= aPassword;
            }

            bool bNeedsPreparation = false;
            css::util::URL aPrepareURL;
            css::uno::Reference< css::frame::XDispatch > xPrepareDispatch;
            css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( xFrame, css::uno::UNO_QUERY );
            css::uno::Reference< css::util::XURLTransformer > xURLTransformer( css::util::URLTransformer::create( xContext ) );
            if( !bSendAsPDF )
            {
                try
                {
                    // check if the document needs to be prepared for sending as mail (embedding of links, removal of invisible content)

                    aPrepareURL.Complete = ".uno:PrepareMailExport";
                    xURLTransformer->parseStrict( aPrepareURL );

                    if ( xDispatchProvider.is() )
                    {
                        xPrepareDispatch.set( xDispatchProvider->queryDispatch( aPrepareURL, OUString(), 0 ));
                        if ( xPrepareDispatch.is() )
                        {
                                rtl::Reference<PrepareListener_Impl> pPrepareListener = new PrepareListener_Impl;
                                xPrepareDispatch->addStatusListener( pPrepareListener, aPrepareURL );
                                bNeedsPreparation = pPrepareListener->IsSet();
                                xPrepareDispatch->removeStatusListener( pPrepareListener, aPrepareURL );
                        }
                    }
                }
                catch ( css::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( css::uno::Exception& )
                {
                }
            }

            if ( bModified || !bHasLocation || bStoreTo || bNeedsPreparation )
            {
                // Document is modified, is newly created or should be stored in a special format
                try
                {
                    if( bNeedsPreparation && xPrepareDispatch.is() )
                    {
                        try
                        {
                            css::uno::Sequence< css::beans::PropertyValue > aDispatchArgs;
                            xPrepareDispatch->dispatch( aPrepareURL, aDispatchArgs );
                        }
                        catch ( css::uno::RuntimeException& )
                        {
                            throw;
                        }
                        catch ( css::uno::Exception& )
                        {
                        }
                    }

                    //check if this is the pdf output filter (i#64555)
                    if( bSendAsPDF )
                    {
                        SaveResult eShowPDFFilterDialog = ShowFilterOptionsDialog(
                                                            xSMGR, xModel, aFilterName, rType, bModified, nNumArgs, aArgs );

                        // don't continue on dialog cancel or error
                        if ( eShowPDFFilterDialog != SAVE_SUCCESSFUL )
                            return eShowPDFFilterDialog;
                    }

                    xStorable->storeToURL( aFileURL, aArgs );
                    rFileNamePath = aFileURL;
                    eRet = SAVE_SUCCESSFUL;

                    if( !bSendAsPDF )
                    {
                        css::util::URL aURL;
                        // #i30432# notify that export is finished - the Writer may want to restore removed content
                        aURL.Complete = ".uno:MailExportFinished";
                        xURLTransformer->parseStrict( aURL );

                        if ( xDispatchProvider.is() )
                        {
                            css::uno::Reference< css::frame::XDispatch > xDispatch(
                                xDispatchProvider->queryDispatch( aURL, OUString(), 0 ));
                            if ( xDispatch.is() )
                            {
                                try
                                {
                                    css::uno::Sequence< css::beans::PropertyValue > aDispatchArgs;
                                    xDispatch->dispatch( aURL, aDispatchArgs );
                                }
                                catch ( css::uno::RuntimeException& )
                                {
                                    throw;
                                }
                                catch ( css::uno::Exception& )
                                {
                                }
                            }
                        }
                    }
                    // If the model is not modified, it could be modified by the dispatch calls.
                    // Therefore set back to modified = false. This should not hurt if we call
                    // on a non-modified model.
                    if ( !bModified )
                    {
                        try
                        {
                            xModifiable->setModified( false );
                        }
                        catch( css::beans::PropertyVetoException& )
                        {
                        }
                    }
                }
                catch ( css::io::IOException& )
                {
                    eRet = SAVE_ERROR;
                }
            }
            else
            {
                // We need 1:1 copy of the document to preserve an added signature.
                aArgs.realloc( ++nNumArgs );
                auto pArgs = aArgs.getArray();
                pArgs[nNumArgs-1].Name = "CopyStreamIfPossible";
                pArgs[nNumArgs-1].Value <<= true;

                try
                {
                    xStorable->storeToURL( aFileURL, aArgs );
                    rFileNamePath = aFileURL;
                    eRet = SAVE_SUCCESSFUL;
                }
                catch ( css::io::IOException& )
                {
                    eRet = SAVE_ERROR;
                }
            }
        }
    }

    return eRet;
}

SfxMailModel::SfxMailModel()
{
}

SfxMailModel::~SfxMailModel()
{
}

void SfxMailModel::AddToAddress( const OUString& rAddress )
{
    // don't add an empty address
    if ( !rAddress.isEmpty() )
    {
        if ( !mpToList )
            // create the list
            mpToList.reset(new AddressList_Impl);

        // add address to list
        mpToList->push_back( rAddress );
    }
}

SfxMailModel::SendMailResult SfxMailModel::AttachDocument(
    const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
    const OUString& sAttachmentTitle )
{
    OUString sFileName;

    SaveResult eSaveResult = SaveDocumentAsFormat( sAttachmentTitle, xFrameOrModel, OUString()/*sDocumentType*/, sFileName );
    if ( eSaveResult == SAVE_SUCCESSFUL &&  !sFileName.isEmpty() )
        maAttachedDocuments.push_back(sFileName);
    return eSaveResult == SAVE_SUCCESSFUL ? SEND_MAIL_OK : SEND_MAIL_ERROR;
}

SfxMailModel::SendMailResult SfxMailModel::Send( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    OSL_ENSURE(!maAttachedDocuments.empty(),"No document added!");
    SendMailResult  eResult = SEND_MAIL_ERROR;
    if ( !maAttachedDocuments.empty() )
    {
        css::uno::Reference < XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        css::uno::Reference< XSimpleMailClientSupplier >    xSimpleMailClientSupplier;

        // Prefer the SimpleSystemMail service if available
        try {
            xSimpleMailClientSupplier = SimpleSystemMail::create( xContext );
        }
        catch ( const uno::Exception & )
        {}

        if ( ! xSimpleMailClientSupplier.is() )
        {
            try {
                xSimpleMailClientSupplier = SimpleCommandMail::create( xContext );
            }
            catch ( const uno::Exception & )
            {}
        }

        if ( xSimpleMailClientSupplier.is() )
        {
            css::uno::Reference< XSimpleMailClient > xSimpleMailClient = xSimpleMailClientSupplier->querySimpleMailClient();

            if ( !xSimpleMailClient.is() )
            {
                // no mail client support => message box!
                return SEND_MAIL_ERROR;
            }

            // we have a simple mail client
            css::uno::Reference< XSimpleMailMessage > xSimpleMailMessage = xSimpleMailClient->createSimpleMailMessage();
            if ( xSimpleMailMessage.is() )
            {
                sal_Int32 nSendFlags = SimpleMailClientFlags::DEFAULTS;
                if ( maFromAddress.isEmpty() )
                {
                    // from address not set, try figure out users e-mail address
                    CreateFromAddress_Impl( maFromAddress );
                }
                xSimpleMailMessage->setOriginator( maFromAddress );

                size_t nToCount     = mpToList ? mpToList->size() : 0;

                // set recipient (only one) for this simple mail server!!
                if ( nToCount >= 1 )
                {
                    xSimpleMailMessage->setRecipient( mpToList->at( 0 ) );
                    nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                }

                // all other recipient must be handled with CC recipients!
                if ( nToCount > 1 )
                {
                    Sequence< OUString >    aCcRecipientSeq( nToCount - 1 );
                    std::copy_n(std::next(mpToList->begin()), aCcRecipientSeq.getLength(),
                                aCcRecipientSeq.getArray());
                    xSimpleMailMessage->setCcRecipient( aCcRecipientSeq );
                }

                Sequence< OUString > aAttachmentSeq(maAttachedDocuments.data(),maAttachedDocuments.size());

                if ( xSimpleMailMessage->getSubject().isEmpty() ) {
                    INetURLObject url(
                        maAttachedDocuments[0], INetURLObject::EncodeMechanism::WasEncoded);
                    OUString subject(
                        url.getBase(
                            INetURLObject::LAST_SEGMENT, false,
                            INetURLObject::DecodeMechanism::WithCharset));
                    if (subject.isEmpty()) {
                        subject = maAttachedDocuments[0];
                    }
                    if ( maAttachedDocuments.size() > 1 )
                        subject += ", ...";
                    xSimpleMailMessage->setSubject( subject );
                }
                xSimpleMailMessage->setAttachement( aAttachmentSeq );

                bool bSend( false );
                try
                {
                    xSimpleMailClient->sendSimpleMailMessage( xSimpleMailMessage, nSendFlags );
                    bSend = true;
                }
                catch ( IllegalArgumentException& )
                {
                }
                catch ( Exception& )
                {
                }

                if ( !bSend )
                {
                    css::uno::Reference< css::awt::XWindow > xParentWindow = xFrame->getContainerWindow();

                    SolarMutexGuard aGuard;

                    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(Application::GetFrameWeld(xParentWindow), "sfx/ui/errorfindemaildialog.ui"));
                    std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("ErrorFindEmailDialog"));
                    xBox->run();
                    eResult = SEND_MAIL_CANCELLED;
                }
                else
                    eResult = SEND_MAIL_OK;
            }
        }
    }
    else
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

SfxMailModel::SendMailResult SfxMailModel::SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& rTypeName )
{
    SaveResult      eSaveResult;
    SendMailResult  eResult = SEND_MAIL_ERROR;
    OUString   aFileName;

    eSaveResult = SaveDocumentAsFormat( OUString(), xFrame, rTypeName, aFileName );

    if ( eSaveResult == SAVE_SUCCESSFUL )
    {
        maAttachedDocuments.push_back( aFileName );
        return Send( xFrame );
    }
    else if ( eSaveResult == SAVE_CANCELLED )
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

// functions -------------------------------------------------------------

bool CreateFromAddress_Impl( OUString& rFrom )

/* [Description]

    This function tries to create a From-address with the help of IniManagers.
    For this the fields 'first name', 'Name' and 'Email' are read from the
    application-ini-data. If these fields are not set, FALSE is returned.

    [Return value]

    sal_True:       Address could be created.
    sal_False:      Address could not be created.
*/

{
    SvtUserOptions aUserCFG;
    OUString aName        = aUserCFG.GetLastName  ();
    OUString aFirstName   = aUserCFG.GetFirstName ();
    if ( !aFirstName.isEmpty() || !aName.isEmpty() )
    {
        if ( !aFirstName.isEmpty() )
        {
            rFrom = comphelper::string::strip(aFirstName, ' ');

            if ( !aName.isEmpty() )
                rFrom += " ";
        }
        rFrom += comphelper::string::strip(aName, ' ');
        // remove illegal characters
        rFrom = rFrom.replaceAll("<", "").replaceAll(">", "").replaceAll("@", "");
    }
    OUString aEmailName = aUserCFG.GetEmail();

    // remove illegal characters
    aEmailName = aEmailName.replaceAll("<", "").replaceAll(">", "");

    if ( !aEmailName.isEmpty() )
    {
        if ( !rFrom.isEmpty() )
            rFrom += " ";
        rFrom += "<" + comphelper::string::strip(aEmailName, ' ') + ">";
    }
    else
        rFrom.clear();
    return !rFrom.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
