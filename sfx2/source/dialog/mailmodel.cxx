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
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/msgbox.hxx>

#include <sfx2/mailmodelapi.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxsids.hrc>
#include "dialog.hrc"

#include <unotools/tempfile.hxx>
#include <unotools/configitem.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>
#include <unotools/useroptions.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/implbase1.hxx>

// --------------------------------------------------------------
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::rtl;

namespace css = ::com::sun::star;
// - class PrepareListener_Impl ------------------------------------------
class PrepareListener_Impl : public ::cppu::WeakImplHelper1< css::frame::XStatusListener >
{
    bool m_bState;
public:
        PrepareListener_Impl();
        virtual ~PrepareListener_Impl();

        // css.frame.XStatusListener
        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& aEvent)
          throw(css::uno::RuntimeException);

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
          throw(css::uno::RuntimeException);

        bool IsSet() const {return m_bState;}
};

PrepareListener_Impl::PrepareListener_Impl() :
    m_bState( false )
{
}

PrepareListener_Impl::~PrepareListener_Impl()
{
}

void PrepareListener_Impl::statusChanged(const css::frame::FeatureStateEvent& rEvent) throw(css::uno::RuntimeException)
{
    if( rEvent.IsEnabled )
        rEvent.State >>= m_bState;
    else
        m_bState = sal_False;
}

void PrepareListener_Impl::disposing(const css::lang::EventObject& /*rEvent*/) throw(css::uno::RuntimeException)
{
}

// class SfxMailModel -----------------------------------------------

static const char       PDF_DOCUMENT_TYPE[]   = "pdf_Portable_Document_Format";
static const sal_uInt32 PDF_DOCUMENT_TYPE_LEN = 28;

void SfxMailModel::ClearList( AddressList_Impl* pList )
{
    if ( pList )
    {
        for( size_t i = 0, n = pList->size(); i < n; ++i )
            delete pList->at(i);
        pList->clear();
    }
}

sal_Bool HasDocumentValidSignature( const css::uno::Reference< css::frame::XModel >& xModel )
{
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet( xModel, css::uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            Any a = xPropSet->getPropertyValue( rtl::OUString( "HasValidSignatures" ));
            sal_Bool bReturn = sal_Bool();
            if ( a >>= bReturn )
                return bReturn;
        }
    }
    catch ( css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }

    return sal_False;
}

SfxMailModel::SaveResult SfxMailModel::ShowFilterOptionsDialog(
    uno::Reference< lang::XMultiServiceFactory > xSMGR,
    uno::Reference< frame::XModel > xModel,
    const rtl::OUString& rFilterName,
    const rtl::OUString& rType,
    bool bModified,
    sal_Int32& rNumArgs,
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs )
{
    SaveResult eRet( SAVE_ERROR );

    try
    {
        uno::Sequence < beans::PropertyValue > aProps;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > xFilterCFG =
                uno::Reference< container::XNameAccess >(
                    xSMGR->createInstance(
                        ::rtl::OUString("com.sun.star.document.FilterFactory") ), uno::UNO_QUERY );
        css::uno::Reference< css::util::XModifiable > xModifiable( xModel, css::uno::UNO_QUERY );

        if ( !xFilterCFG.is() )
            return eRet;

        uno::Any aAny = xFilterCFG->getByName( rFilterName );

        if ( aAny >>= aProps )
        {
            sal_Int32 nPropertyCount = aProps.getLength();
            for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
            {
                if( aProps[nProperty].Name == "UIComponent" )
                {
                    ::rtl::OUString aServiceName;
                    aProps[nProperty].Value >>= aServiceName;
                    if( !aServiceName.isEmpty() )
                    {
                        uno::Reference< ui::dialogs::XExecutableDialog > xFilterDialog(
                            xSMGR->createInstance( aServiceName ), uno::UNO_QUERY );
                        uno::Reference< beans::XPropertyAccess > xFilterProperties(
                            xFilterDialog, uno::UNO_QUERY );

                        if( xFilterDialog.is() && xFilterProperties.is() )
                        {
                            uno::Sequence< beans::PropertyValue > aPropsForDialog(1);
                            uno::Reference< document::XExporter > xExporter( xFilterDialog, uno::UNO_QUERY );

                            if ( rType.equalsAsciiL( PDF_DOCUMENT_TYPE, PDF_DOCUMENT_TYPE_LEN ))
                            {
                                //add an internal property, used to tell the dialog we want to set a different
                                //string for the ok button
                                //used in filter/source/pdf/impdialog.cxx
                                uno::Sequence< beans::PropertyValue > aFilterDataValue(1);
                                aFilterDataValue[0].Name = ::rtl::OUString( "_OkButtonString" );
                                aFilterDataValue[0].Value = css::uno::makeAny(SfxResId(STR_PDF_EXPORT_SEND ).toString());

                                //add to the filterdata property, the only one the PDF export filter dialog will care for
                                aPropsForDialog[0].Name =  ::rtl::OUString( "FilterData" );
                                aPropsForDialog[0].Value = css::uno::makeAny( aFilterDataValue );

                                //when executing the dialog will merge the persistent FilterData properties
                                xFilterProperties->setPropertyValues( aPropsForDialog );
                            }

                            if( xExporter.is() )
                                xExporter->setSourceDocument(
                                    uno::Reference< lang::XComponent >( xModel, uno::UNO_QUERY ) );

                            if( xFilterDialog->execute() )
                            {
                                //get the filter data
                                uno::Sequence< beans::PropertyValue > aPropsFromDialog = xFilterProperties->getPropertyValues();

                                //add them to the args
                                for ( sal_Int32 nInd = 0; nInd < aPropsFromDialog.getLength(); nInd++ )
                                {
                                    if( aPropsFromDialog[ nInd ].Name == "FilterData" )
                                    {
                                        //found the filterdata, add to the storing argument
                                        rArgs.realloc( ++rNumArgs );
                                        rArgs[rNumArgs-1].Name = aPropsFromDialog[ nInd ].Name;
                                        rArgs[rNumArgs-1].Value = aPropsFromDialog[ nInd ].Value;
                                        break;
                                    }
                                }
                                eRet = SAVE_SUCCESSFULL;
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
                                        xModifiable->setModified( sal_False );
                                    }
                                    catch( com::sun::star::beans::PropertyVetoException& )
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

sal_Bool SfxMailModel::IsEmpty() const
{
    return maAttachedDocuments.empty();
}

SfxMailModel::SaveResult SfxMailModel::SaveDocumentAsFormat(
    const rtl::OUString& aSaveFileName,
    const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
    const rtl::OUString& rType,
    rtl::OUString& rFileNamePath )
{
    SaveResult  eRet( SAVE_ERROR );
    bool        bSendAsPDF = (rType.equalsAsciiL( PDF_DOCUMENT_TYPE, PDF_DOCUMENT_TYPE_LEN ));

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = ::comphelper::getProcessServiceFactory();
    if (!xSMGR.is())
        return eRet;

    const rtl::OUString aModuleManager( "com.sun.star.frame.ModuleManager" );
    css::uno::Reference< css::frame::XModuleManager > xModuleManager( xSMGR->createInstance( aModuleManager ), css::uno::UNO_QUERY_THROW );
    if ( !xModuleManager.is() )
        return eRet;

    rtl::OUString aModule;
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
            rtl::OUString aLocation = xStorable->getLocation();
            INetURLObject aFileObj( aLocation );

            bool bPrivateProtocol = ( aFileObj.GetProtocol() == INET_PROT_PRIV_SOFFICE );

            bHasLocation =  !aLocation.isEmpty() && !bPrivateProtocol;
            OSL_ASSERT( !bPrivateProtocol );
        }
        if ( !rType.isEmpty() )
            bStoreTo = true;

        if ( xStorable.is() )
        {
            rtl::OUString aFilterName;
            rtl::OUString aTypeName( rType );
            rtl::OUString aFileName;
            rtl::OUString aExtension;

            css::uno::Reference< css::container::XContainerQuery > xContainerQuery(
                xSMGR->createInstance( rtl::OUString(
                    "com.sun.star.document.FilterFactory" )),
                    css::uno::UNO_QUERY );

            if ( bStoreTo )
            {
                // Retrieve filter from type
                css::uno::Sequence< css::beans::NamedValue > aQuery( bSendAsPDF ? 3 : 2 );
                aQuery[0].Name  = rtl::OUString( "Type" );
                aQuery[0].Value = css::uno::makeAny( aTypeName );
                aQuery[1].Name  = rtl::OUString( "DocumentService" );
                aQuery[1].Value = css::uno::makeAny( aModule );
                if( bSendAsPDF )
                {
                    // #i91419#
                    // FIXME: we want just an export filter. However currently we need
                    // exact flag value as detailed in the filter configuration to get it
                    // this seems to be a bug
                    // without flags we get an import filter here, which is also unwanted
                    aQuery[2].Name  = rtl::OUString( "Flags" );
                    aQuery[2].Value = css::uno::makeAny( sal_Int32(0x80042) ); // EXPORT ALIEN 3RDPARTY
                }

                css::uno::Reference< css::container::XEnumeration > xEnumeration =
                    xContainerQuery->createSubSetEnumerationByProperties( aQuery );

                if ( xEnumeration->hasMoreElements() )
                {
                    ::comphelper::SequenceAsHashMap aFilterPropsHM( xEnumeration->nextElement() );
                    aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                ::rtl::OUString("Name"),
                                                ::rtl::OUString() );
                }

                if ( bHasLocation )
                {
                    // Retrieve filter from media descriptor
                    ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
                    rtl::OUString aOrgFilterName = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                    ::rtl::OUString( "FilterName" ),
                                    ::rtl::OUString() );
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
                                    ::rtl::OUString( "FilterName" ),
                                    ::rtl::OUString() );
                }

                if ( !bHasLocation ||  aFilterName.isEmpty())
                {
                    // Retrieve the user defined default filter
                    css::uno::Reference< css::container::XNameAccess > xNameAccess( xModuleManager, css::uno::UNO_QUERY );
                    try
                    {
                        ::comphelper::SequenceAsHashMap aFilterPropsHM( xNameAccess->getByName( aModule ) );
                        aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                    ::rtl::OUString("ooSetupFactoryDefaultFilter"),
                                                    ::rtl::OUString() );
                        css::uno::Reference< css::container::XNameAccess > xNameAccess2(
                            xContainerQuery, css::uno::UNO_QUERY );
                        if ( xNameAccess2.is() )
                        {
                            ::comphelper::SequenceAsHashMap aFilterPropsHM2( xNameAccess2->getByName( aFilterName ) );
                            aTypeName = aFilterPropsHM2.getUnpackedValueOrDefault(
                                                        ::rtl::OUString("Type"),
                                                        ::rtl::OUString() );
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

            // Determine filen name and extension
            if ( bHasLocation && !bStoreTo )
            {
                INetURLObject aFileObj( xStorable->getLocation() );
                aExtension = (rtl::OUString)aFileObj.getExtension();
            }
            else
            {
                css::uno::Reference< container::XNameAccess > xTypeDetection(
                    xSMGR->createInstance( ::rtl::OUString(
                        "com.sun.star.document.TypeDetection" )),
                    css::uno::UNO_QUERY );


                if ( xTypeDetection.is() )
                {
                    try
                    {
                        ::comphelper::SequenceAsHashMap aTypeNamePropsHM( xTypeDetection->getByName( aTypeName ) );
                        uno::Sequence< ::rtl::OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                                        ::rtl::OUString("Extensions"),
                                                        ::uno::Sequence< ::rtl::OUString >() );
                        if ( aExtensions.getLength() )
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
                    const rtl::OUString aNoNameFileName( "noname" );
                    aFileName = aNoNameFileName;
                }
                else
                {
                    // Determine file name from model
                    INetURLObject aFileObj( xStorable->getLocation() );
                    aFileName = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::NO_DECODE );
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
            ::utl::TempFile aTempDir( NULL, sal_True );

            INetURLObject aFilePathObj( aTempDir.GetURL() );
            aFilePathObj.insertName( aFileName );
            aFilePathObj.setExtension( aExtension );

            rtl::OUString aFileURL = aFilePathObj.GetMainURL( INetURLObject::NO_DECODE );

            sal_Int32 nNumArgs(0);
            const rtl::OUString aPasswordPropName( "Password" );
            css::uno::Sequence< css::beans::PropertyValue > aArgs( ++nNumArgs );
            aArgs[nNumArgs-1].Name  = rtl::OUString( "FilterName" );
            aArgs[nNumArgs-1].Value = css::uno::makeAny( aFilterName );

            ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
            rtl::OUString aPassword = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                            aPasswordPropName,
                                            ::rtl::OUString() );
            if ( !aPassword.isEmpty() )
            {
                aArgs.realloc( ++nNumArgs );
                aArgs[nNumArgs-1].Name = aPasswordPropName;
                aArgs[nNumArgs-1].Value = css::uno::makeAny( aPassword );
            }

            bool bNeedsPreparation = false;
            css::util::URL aPrepareURL;
            css::uno::Reference< css::frame::XDispatch > xPrepareDispatch;
            css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( xFrame, css::uno::UNO_QUERY );
            css::uno::Reference< css::util::XURLTransformer > xURLTransformer( css::util::URLTransformer::create( ::comphelper::ComponentContext(xSMGR).getUNOContext() ) );
            if( !bSendAsPDF )
            {
                try
                {
                    // check if the document needs to be prepared for sending as mail (embedding of links, removal of invisible content)

                    aPrepareURL.Complete = rtl::OUString( ".uno:PrepareMailExport" );
                    xURLTransformer->parseStrict( aPrepareURL );

                    if ( xDispatchProvider.is() )
                    {
                        xPrepareDispatch = css::uno::Reference< css::frame::XDispatch >(
                            xDispatchProvider->queryDispatch( aPrepareURL, ::rtl::OUString(), 0 ));
                        if ( xPrepareDispatch.is() )
                        {
                                PrepareListener_Impl* pPrepareListener;
                                uno::Reference< css::frame::XStatusListener > xStatusListener = pPrepareListener = new PrepareListener_Impl;
                                xPrepareDispatch->addStatusListener( xStatusListener, aPrepareURL );
                                bNeedsPreparation = pPrepareListener->IsSet();
                                xPrepareDispatch->removeStatusListener( xStatusListener, aPrepareURL );
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
                        if ( xPrepareDispatch.is() )
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
                    }

                    //check if this is the pdf otput filter (i#64555)
                    if( bSendAsPDF )
                    {
                        SaveResult eShowPDFFilterDialog = ShowFilterOptionsDialog(
                                                            xSMGR, xModel, aFilterName, rType, bModified, nNumArgs, aArgs );

                        // don't continue on dialog cancel or error
                        if ( eShowPDFFilterDialog != SAVE_SUCCESSFULL )
                            return eShowPDFFilterDialog;
                    }

                    xStorable->storeToURL( aFileURL, aArgs );
                    rFileNamePath = aFileURL;
                    eRet = SAVE_SUCCESSFULL;

                    if( !bSendAsPDF )
                    {
                        css::util::URL aURL;
                        // #i30432# notify that export is finished - the Writer may want to restore removed content
                        aURL.Complete = rtl::OUString( ".uno:MailExportFinished" );
                        xURLTransformer->parseStrict( aURL );

                        if ( xDispatchProvider.is() )
                        {
                            css::uno::Reference< css::frame::XDispatch > xDispatch = css::uno::Reference< css::frame::XDispatch >(
                                xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 ));
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
                            xModifiable->setModified( sal_False );
                        }
                        catch( com::sun::star::beans::PropertyVetoException& )
                        {
                        }
                    }
                }
                catch ( com::sun::star::io::IOException& )
                {
                    eRet = SAVE_ERROR;
                }
            }
            else
            {
                // We need 1:1 copy of the document to preserve an added signature.
                aArgs.realloc( ++nNumArgs );
                aArgs[nNumArgs-1].Name = ::rtl::OUString( "CopyStreamIfPossible"  );
                aArgs[nNumArgs-1].Value = css::uno::makeAny( (sal_Bool)sal_True );

                try
                {
                    xStorable->storeToURL( aFileURL, aArgs );
                    rFileNamePath = aFileURL;
                    eRet = SAVE_SUCCESSFULL;
                }
                catch ( com::sun::star::io::IOException& )
                {
                    eRet = SAVE_ERROR;
                }
            }
        }
    }

    return eRet;
}

SfxMailModel::SfxMailModel() :
    mpToList    ( NULL ),
    mpCcList    ( NULL ),
    mpBccList   ( NULL ),
    mePriority  ( PRIO_NORMAL ),
    mbLoadDone  ( sal_True )
{
}

SfxMailModel::~SfxMailModel()
{
    ClearList( mpToList );
    delete mpToList;
    ClearList( mpCcList );
    delete mpCcList;
    ClearList( mpBccList );
    delete mpBccList;
}

void SfxMailModel::AddAddress( const String& rAddress, AddressRole eRole )
{
    // don't add a empty address
    if ( rAddress.Len() > 0 )
    {
        AddressList_Impl* pList = NULL;
        if ( ROLE_TO == eRole )
        {
            if ( !mpToList )
                // create the list
                mpToList = new AddressList_Impl();
            pList = mpToList;
        }
        else if ( ROLE_CC == eRole )
        {
            if ( !mpCcList )
                // create the list
                mpCcList = new AddressList_Impl();
            pList = mpCcList;
        }
        else if ( ROLE_BCC == eRole )
        {
            if ( !mpBccList )
                // create the list
                mpBccList = new AddressList_Impl();
            pList = mpBccList;
        }
        else
        {
            SAL_WARN( "sfx2.dialog", "invalid address role" );
        }

        if ( pList )
        {
            // add address to list
            AddressItemPtr_Impl pAddress = new String( rAddress );
            pList->push_back( pAddress );
        }
    }
}

SfxMailModel::SendMailResult SfxMailModel::AttachDocument(
    const ::rtl::OUString& sDocumentType,
    const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
    const ::rtl::OUString& sAttachmentTitle )
{
    rtl::OUString sFileName;

    SaveResult eSaveResult = SaveDocumentAsFormat( sAttachmentTitle, xFrameOrModel, sDocumentType, sFileName );
    if ( eSaveResult == SAVE_SUCCESSFULL &&  !sFileName.isEmpty() )
        maAttachedDocuments.push_back(sFileName);
    return eSaveResult == SAVE_SUCCESSFULL ? SEND_MAIL_OK : SEND_MAIL_ERROR;
}

SfxMailModel::SendMailResult SfxMailModel::Send( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    OSL_ENSURE(!maAttachedDocuments.empty(),"No document added!");
    SendMailResult  eResult = SEND_MAIL_ERROR;
    if ( !maAttachedDocuments.empty() )
    {
        css::uno::Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
        if ( xMgr.is() )
        {
            css::uno::Reference< XSimpleMailClientSupplier >    xSimpleMailClientSupplier;

            // Prefer the SimpleSystemMail service if available
            xSimpleMailClientSupplier = css::uno::Reference< XSimpleMailClientSupplier >(
                xMgr->createInstance( OUString( "com.sun.star.system.SimpleSystemMail" )),
                UNO_QUERY );

            if ( ! xSimpleMailClientSupplier.is() )
            {
                xSimpleMailClientSupplier = css::uno::Reference< XSimpleMailClientSupplier >(
                    xMgr->createInstance( OUString( "com.sun.star.system.SimpleCommandMail" )),
                    UNO_QUERY );
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
                    if ( maFromAddress.Len() == 0 )
                    {
                        // from address not set, try figure out users e-mail address
                        CreateFromAddress_Impl( maFromAddress );
                    }
                    xSimpleMailMessage->setOriginator( maFromAddress );

                    size_t nToCount     = mpToList ? mpToList->size() : 0;
                    size_t nCcCount     = mpCcList ? mpCcList->size() : 0;
                    size_t nCcSeqCount  = nCcCount;

                    // set recipient (only one) for this simple mail server!!
                    if ( nToCount > 1 )
                    {
                        nCcSeqCount = nToCount - 1 + nCcCount;
                        xSimpleMailMessage->setRecipient( *mpToList->at( 0 ) );
                        nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                    }
                    else if ( nToCount == 1 )
                    {
                        xSimpleMailMessage->setRecipient( *mpToList->at( 0 ) );
                        nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                    }

                    // all other recipient must be handled with CC recipients!
                    if ( nCcSeqCount > 0 )
                    {
                        size_t                  nIndex = 0;
                        Sequence< OUString >    aCcRecipientSeq;

                        aCcRecipientSeq.realloc( nCcSeqCount );
                        if ( nCcSeqCount > nCcCount )
                        {
                            for ( size_t i = 1; i < nToCount; ++i )
                            {
                                aCcRecipientSeq[nIndex++] = *mpToList->at(i);
                            }
                        }

                        for ( size_t i = 0; i < nCcCount; i++ )
                        {
                            aCcRecipientSeq[nIndex++] = *mpCcList->at(i);
                        }
                        xSimpleMailMessage->setCcRecipient( aCcRecipientSeq );
                    }

                    size_t nBccCount = mpBccList ? mpBccList->size() : 0;
                    if ( nBccCount > 0 )
                    {
                        Sequence< OUString > aBccRecipientSeq( nBccCount );
                        for ( size_t i = 0; i < nBccCount; ++i )
                        {
                            aBccRecipientSeq[i] = *mpBccList->at(i);
                        }
                        xSimpleMailMessage->setBccRecipient( aBccRecipientSeq );
                    }

                    Sequence< OUString > aAttachmentSeq(&(maAttachedDocuments[0]),maAttachedDocuments.size());

                    if ( xSimpleMailMessage->getSubject().isEmpty() ) {
                        OUString baseName( maAttachedDocuments[0].copy( maAttachedDocuments[0].lastIndexOf( '/' ) + 1 ) );
                        OUString subject( baseName );
                        if ( maAttachedDocuments.size() > 1 )
                            subject += OUString(", ...");
                        xSimpleMailMessage->setSubject( subject );
                    }
                    xSimpleMailMessage->setAttachement( aAttachmentSeq );

                    sal_Bool bSend( sal_False );
                    try
                    {
                        xSimpleMailClient->sendSimpleMailMessage( xSimpleMailMessage, nSendFlags );
                        bSend = sal_True;
                    }
                    catch ( IllegalArgumentException& )
                    {
                    }
                    catch ( Exception& )
                    {
                    }

                    if ( bSend == sal_False )
                    {
                        css::uno::Reference< css::awt::XWindow > xParentWindow = xFrame->getContainerWindow();

                        SolarMutexGuard aGuard;
                        Window* pParentWindow = VCLUnoHelper::GetWindow( xParentWindow );

                        ErrorBox aBox( pParentWindow, SfxResId( RID_ERRBOX_MAIL_CONFIG ));
                        aBox.Execute();
                        eResult = SEND_MAIL_CANCELLED;
                    }
                    else
                        eResult = SEND_MAIL_OK;
                }
            }
        }
    }
    else
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

SfxMailModel::SendMailResult SfxMailModel::SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame, const rtl::OUString& rTypeName )
{
    SaveResult      eSaveResult;
    SendMailResult  eResult = SEND_MAIL_ERROR;
    rtl::OUString   aFileName;

    eSaveResult = SaveDocumentAsFormat( rtl::OUString(), xFrame, rTypeName, aFileName );

    if ( eSaveResult == SAVE_SUCCESSFULL )
    {
        maAttachedDocuments.push_back( aFileName );
        return Send( xFrame );
    }
    else if ( eSaveResult == SAVE_CANCELLED )
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

// functions -------------------------------------------------------------

sal_Bool CreateFromAddress_Impl( String& rFrom )

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
    String aName        = aUserCFG.GetLastName  ();
    String aFirstName   = aUserCFG.GetFirstName ();
    if ( aFirstName.Len() || aName.Len() )
    {
        if ( aFirstName.Len() )
        {
            rFrom = comphelper::string::strip(aFirstName, ' ');

            if ( aName.Len() )
                rFrom += ' ';
        }
        rFrom += comphelper::string::strip(aName, ' ');
        // remove illegal characters
        rFrom = comphelper::string::remove(rFrom, '<');
        rFrom = comphelper::string::remove(rFrom, '>');
        rFrom = comphelper::string::remove(rFrom, '@');
    }
    String aEmailName = aUserCFG.GetEmail();

    // remove illegal characters
    aEmailName = comphelper::string::remove(aEmailName, '<');
    aEmailName = comphelper::string::remove(aEmailName, '>');

    if ( aEmailName.Len() )
    {
        if ( rFrom.Len() )
            rFrom += ' ';
        ( ( rFrom += '<' ) += comphelper::string::strip(aEmailName, ' ') ) += '>';
    }
    else
        rFrom.Erase();
    return ( rFrom.Len() > 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
