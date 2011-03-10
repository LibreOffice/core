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
// includes --------------------------------------------------------------
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/uno/Reference.h>
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
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XExporter.hpp>

#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/msgbox.hxx>

#include <mailmodelapi.hxx>
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include <sfx2/sfxsids.hrc>
#include "dialog.hrc"

#include <unotools/tempfile.hxx>
#include <unotools/configitem.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>
#include <unotools/useroptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

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

void SfxMailModel::MakeValueList( AddressList_Impl* pList, String& rValueList )
{
    rValueList.Erase();
    if ( pList )
    {
        for( size_t i = 0, n = pList->size(); i < n; ++i )
        {
            if ( rValueList.Len() > 0 )
                rValueList += ',';
            rValueList += *pList->at(i);
        }
    }
}

sal_Bool HasDocumentValidSignature( const css::uno::Reference< css::frame::XModel >& xModel )
{
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet( xModel, css::uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            Any a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasValidSignatures" )));
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
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.FilterFactory")) ), uno::UNO_QUERY );
        css::uno::Reference< css::util::XModifiable > xModifiable( xModel, css::uno::UNO_QUERY );

        if ( !xFilterCFG.is() )
            return eRet;

        uno::Any aAny = xFilterCFG->getByName( rFilterName );

        if ( aAny >>= aProps )
        {
            sal_Int32 nPropertyCount = aProps.getLength();
            for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
            {
                if( aProps[nProperty].Name.equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UIComponent"))) )
                {
                    ::rtl::OUString aServiceName;
                    aProps[nProperty].Value >>= aServiceName;
                    if( aServiceName.getLength() )
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
                                String aOkSendText( SfxResId( STR_PDF_EXPORT_SEND ));

                                uno::Sequence< beans::PropertyValue > aFilterDataValue(1);
                                aFilterDataValue[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_OkButtonString" ));
                                aFilterDataValue[0].Value = css::uno::makeAny( ::rtl::OUString( aOkSendText ));

                                //add to the filterdata property, the only one the PDF export filter dialog will care for
                                aPropsForDialog[0].Name =  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterData" ));
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
                                    if( aPropsFromDialog[ nInd ].Name.equals( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FilterData")) ) )
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

sal_Int32 SfxMailModel::GetCount() const
{
    return maAttachedDocuments.size();
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

    const rtl::OUString aModuleManager( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ModuleManager" ));
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
    if (( aModule.getLength() > 0 ) && xModel.is() )
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

            bHasLocation = ( aLocation.getLength() > 0 ) && !bPrivateProtocol;
            OSL_ASSERT( !bPrivateProtocol );
        }
        if ( rType.getLength() > 0 )
            bStoreTo = true;

        if ( xStorable.is() )
        {
            rtl::OUString aFilterName;
            rtl::OUString aTypeName( rType );
            rtl::OUString aFileName;
            rtl::OUString aExtension;

            css::uno::Reference< css::container::XContainerQuery > xContainerQuery(
                xSMGR->createInstance( rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ))),
                    css::uno::UNO_QUERY );

            if ( bStoreTo )
            {
                // Retrieve filter from type
                css::uno::Sequence< css::beans::NamedValue > aQuery( bSendAsPDF ? 3 : 2 );
                aQuery[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Type" ));
                aQuery[0].Value = css::uno::makeAny( aTypeName );
                aQuery[1].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentService" ));
                aQuery[1].Value = css::uno::makeAny( aModule );
                if( bSendAsPDF )
                {
                    // #i91419#
                    // FIXME: we want just an export filter. However currently we need
                    // exact flag value as detailed in the filter configuration to get it
                    // this seems to be a bug
                    // without flags we get an import filter here, which is also unwanted
                    aQuery[2].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Flags" ));
                    aQuery[2].Value = css::uno::makeAny( sal_Int32(0x80042) ); // EXPORT ALIEN 3RDPARTY
                }

                css::uno::Reference< css::container::XEnumeration > xEnumeration =
                    xContainerQuery->createSubSetEnumerationByProperties( aQuery );

                if ( xEnumeration->hasMoreElements() )
                {
                    ::comphelper::SequenceAsHashMap aFilterPropsHM( xEnumeration->nextElement() );
                    aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")),
                                                ::rtl::OUString() );
                }

                if ( bHasLocation )
                {
                    // Retrieve filter from media descriptor
                    ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
                    rtl::OUString aOrgFilterName = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" )),
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
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" )),
                                    ::rtl::OUString() );
                }

                if ( !bHasLocation || ( aFilterName.getLength() == 0 ))
                {
                    // Retrieve the user defined default filter
                    css::uno::Reference< css::container::XNameAccess > xNameAccess( xModuleManager, css::uno::UNO_QUERY );
                    try
                    {
                        ::comphelper::SequenceAsHashMap aFilterPropsHM( xNameAccess->getByName( aModule ) );
                        aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryDefaultFilter")),
                                                    ::rtl::OUString() );
                        css::uno::Reference< css::container::XNameAccess > xNameAccess2(
                            xContainerQuery, css::uno::UNO_QUERY );
                        if ( xNameAccess2.is() )
                        {
                            ::comphelper::SequenceAsHashMap aFilterPropsHM2( xNameAccess2->getByName( aFilterName ) );
                            aTypeName = aFilterPropsHM2.getUnpackedValueOrDefault(
                                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Type")),
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
            if (( aFilterName.getLength() == 0 ) ||
                (( aTypeName.getLength() == 0 ) && !bHasLocation ))
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
                        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ))),
                    css::uno::UNO_QUERY );


                if ( xTypeDetection.is() )
                {
                    try
                    {
                        ::comphelper::SequenceAsHashMap aTypeNamePropsHM( xTypeDetection->getByName( aTypeName ) );
                        uno::Sequence< ::rtl::OUString > aExtensions = aTypeNamePropsHM.getUnpackedValueOrDefault(
                                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Extensions")),
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
            if ( aFileName.getLength() == 0 )
            {
                if ( !bHasLocation )
                {
                    // Create a noname file name with the correct extension
                    const rtl::OUString aNoNameFileName( RTL_CONSTASCII_USTRINGPARAM( "noname" ));
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
            if ( aFileName.getLength() == 0 )
                return eRet;

            OSL_ASSERT( aFilterName.getLength() > 0 );
            OSL_ASSERT( aFileName.getLength() > 0 );

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
            const rtl::OUString aPasswordPropName( RTL_CONSTASCII_USTRINGPARAM( "Password" ));
            css::uno::Sequence< css::beans::PropertyValue > aArgs( ++nNumArgs );
            aArgs[nNumArgs-1].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
            aArgs[nNumArgs-1].Value = css::uno::makeAny( aFilterName );

            ::comphelper::SequenceAsHashMap aMediaDescrPropsHM( xModel->getArgs() );
            rtl::OUString aPassword = aMediaDescrPropsHM.getUnpackedValueOrDefault(
                                            aPasswordPropName,
                                            ::rtl::OUString() );
            if ( aPassword.getLength() > 0 )
            {
                aArgs.realloc( ++nNumArgs );
                aArgs[nNumArgs-1].Name = aPasswordPropName;
                aArgs[nNumArgs-1].Value = css::uno::makeAny( aPassword );
            }

            if ( bModified || !bHasLocation || bStoreTo )
            {
                // Document is modified, is newly created or should be stored in a special format
                try
                {
                    css::uno::Reference< css::util::XURLTransformer > xURLTransformer(
                        xSMGR->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                        css::uno::UNO_QUERY );

                    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( xFrame, css::uno::UNO_QUERY );
                    css::uno::Reference< css::frame::XDispatch > xDispatch;

                    css::util::URL aURL;
                    css::uno::Sequence< css::beans::PropertyValue > aDispatchArgs;

                    if( !bSendAsPDF )
                    {
                        if ( xURLTransformer.is() )
                        {
                            aURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PrepareMailExport" ));
                            xURLTransformer->parseStrict( aURL );
                        }

                        if ( xDispatchProvider.is() )
                        {
                            xDispatch = css::uno::Reference< css::frame::XDispatch >(
                                xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 ));
                            if ( xDispatch.is() )
                            {
                                try
                                {
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
                        // #i30432# notify that export is finished - the Writer may want to restore removed content
                        if ( xURLTransformer.is() )
                        {
                            aURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:MailExportFinished" ));
                            xURLTransformer->parseStrict( aURL );
                        }

                        if ( xDispatchProvider.is() )
                        {
                            xDispatch = css::uno::Reference< css::frame::XDispatch >(
                                xDispatchProvider->queryDispatch( aURL, ::rtl::OUString(), 0 ));
                            if ( xDispatch.is() )
                            {
                                try
                                {
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
                aArgs[nNumArgs-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CopyStreamIfPossible" ) );
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
            DBG_ERRORFILE( "invalid address role" );
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
    if ( eSaveResult == SAVE_SUCCESSFULL && ( sFileName.getLength() > 0 ) )
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
                xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleSystemMail" ))),
                UNO_QUERY );

            if ( ! xSimpleMailClientSupplier.is() )
            {
                xSimpleMailClientSupplier = css::uno::Reference< XSimpleMailClientSupplier >(
                    xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleCommandMail" ))),
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

                    if ( xSimpleMailMessage->getSubject().getLength() == 0 ) {
                        OUString baseName( maAttachedDocuments[0].copy( maAttachedDocuments[0].lastIndexOf( '/' ) + 1 ) );
                        OUString subject( baseName );
                        if ( maAttachedDocuments.size() > 1 )
                            subject += OUString(RTL_CONSTASCII_USTRINGPARAM(", ..."));
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

BOOL CreateFromAddress_Impl( String& rFrom )

/* [Description]

    This function tries to create a From-address with the help of IniManagers.
    For this the fields 'first name', 'Name' and 'Email' are read from the
    application-ini-data. If these fields are not set, FALSE is returned.

    [Return value]

    TRUE:       Address could be created.
    FALSE:      Address could not be created.
*/

{
    SvtUserOptions aUserCFG;
    String aName        = aUserCFG.GetLastName  ();
    String aFirstName   = aUserCFG.GetFirstName ();
    if ( aFirstName.Len() || aName.Len() )
    {
        if ( aFirstName.Len() )
        {
            rFrom = TRIM( aFirstName );

            if ( aName.Len() )
                rFrom += ' ';
        }
        rFrom += TRIM( aName );
        // remove illegal characters
        rFrom.EraseAllChars( '<' );
        rFrom.EraseAllChars( '>' );
        rFrom.EraseAllChars( '@' );
    }
    String aEmailName = aUserCFG.GetEmail();

    // remove illegal characters
    aEmailName.EraseAllChars( '<' );
    aEmailName.EraseAllChars( '>' );

    if ( aEmailName.Len() )
    {
        if ( rFrom.Len() )
            rFrom += ' ';
        ( ( rFrom += '<' ) += TRIM( aEmailName ) ) += '>';
    }
    else
        rFrom.Erase();
    return ( rFrom.Len() > 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
