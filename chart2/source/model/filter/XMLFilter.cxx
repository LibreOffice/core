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


#include "XMLFilter.hxx"
#include "macros.hxx"
#include "MediaDescriptorHelper.hxx"
#include "ContainerHelper.hxx"
#include <comphelper/mediadescriptor.hxx>

// for ERRCODE_SFX_GENERAL etc.
// header contains only macros
#include <svtools/sfxecode.hxx>
// header for class SvtSaveOptions
#include <unotools/saveopt.hxx>
#include <comphelper/genericpropertyset.hxx>
// header for struct PropertyMapEntry
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/documentconstants.hxx>

// header for class SotStorage
#include <sot/storage.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::osl::MutexGuard;

// ----------------------------------------
namespace
{
#define LOCAL_CONST_STR(i, x) sal_Char const i[sizeof(x)] = x
#define MAP_LEN(x) x, sizeof(x) - 1

LOCAL_CONST_STR( sXML_metaStreamName,       "meta.xml");
LOCAL_CONST_STR( sXML_styleStreamName,      "styles.xml" );
LOCAL_CONST_STR( sXML_contentStreamName,    "content.xml" );
LOCAL_CONST_STR( sXML_oldContentStreamName, "Content.xml" );

// soffice 6/7
LOCAL_CONST_STR( sXML_export_chart_styles_service,          "com.sun.star.comp.Chart.XMLStylesExporter" );
LOCAL_CONST_STR( sXML_export_chart_content_service,         "com.sun.star.comp.Chart.XMLContentExporter" );

LOCAL_CONST_STR( sXML_import_chart_styles_service,          "com.sun.star.comp.Chart.XMLStylesImporter" );
LOCAL_CONST_STR( sXML_import_chart_content_service,         "com.sun.star.comp.Chart.XMLContentImporter" );
LOCAL_CONST_STR( sXML_import_chart_old_content_service,     "com.sun.star.office.sax.importer.Chart" );

// Oasis
LOCAL_CONST_STR( sXML_export_chart_oasis_styles_service,    "com.sun.star.comp.Chart.XMLOasisStylesExporter" );
LOCAL_CONST_STR( sXML_export_chart_oasis_content_service,   "com.sun.star.comp.Chart.XMLOasisContentExporter" );
LOCAL_CONST_STR( sXML_export_chart_oasis_meta_service,      "com.sun.star.comp.Chart.XMLOasisMetaExporter" );

LOCAL_CONST_STR( sXML_import_chart_oasis_styles_service,    "com.sun.star.comp.Chart.XMLOasisStylesImporter" );
LOCAL_CONST_STR( sXML_import_chart_oasis_content_service,   "com.sun.star.comp.Chart.XMLOasisContentImporter" );
LOCAL_CONST_STR( sXML_import_chart_oasis_meta_service,      "com.sun.star.comp.Chart.XMLOasisMetaImporter" );

uno::Reference< embed::XStorage > lcl_getWriteStorage(
    const Sequence< beans::PropertyValue >& rMediaDescriptor,
    const uno::Reference< uno::XComponentContext >& xContext,const ::rtl::OUString& _sMediaType)
{
    uno::Reference< embed::XStorage > xStorage;
    try
    {
        apphelper::MediaDescriptorHelper aMDHelper( rMediaDescriptor );
        if( aMDHelper.ISSET_Storage )
        {
            xStorage = aMDHelper.Storage;
        }
        else
        {
            Reference< lang::XSingleServiceFactory > xStorageFact(
                xContext->getServiceManager()->createInstanceWithContext(
                    C2U("com.sun.star.embed.StorageFactory"),
                    xContext ), uno::UNO_QUERY_THROW );

            ::std::vector< beans::PropertyValue > aPropertiesForStorage;

            for( sal_Int32 i=rMediaDescriptor.getLength(); i--; )
            {
                // properties understood by storage factory
                // (see package/source/xstor/xfactory.cxx for details)
                if ( rMediaDescriptor[i].Name == "InteractionHandler" || rMediaDescriptor[i].Name == "Password" || rMediaDescriptor[i].Name == "RepairPackage" )
                {
                    aPropertiesForStorage.push_back( rMediaDescriptor[i] );
                }
            }

            if( aMDHelper.ISSET_Storage )
                xStorage.set( aMDHelper.Storage );
            else
            {
                Sequence< uno::Any > aStorageArgs( 3 );
                if( aMDHelper.ISSET_OutputStream  )
                    aStorageArgs[0] <<= aMDHelper.OutputStream;
                else
                    aStorageArgs[0] <<= aMDHelper.URL;
                aStorageArgs[1] <<= (embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE);
                aStorageArgs[2] <<= ::chart::ContainerHelper::ContainerToSequence( aPropertiesForStorage );

                xStorage.set(
                    xStorageFact->createInstanceWithArguments( aStorageArgs ),
                    uno::UNO_QUERY_THROW );
            }
        }

        // set correct media type at storage
        uno::Reference<beans::XPropertySet> xProp(xStorage,uno::UNO_QUERY);
        OUString aMediaType;
        if ( ! xProp.is() ||
             ! ( xProp->getPropertyValue( C2U("MediaType")) >>= aMediaType ) ||
             ( aMediaType.isEmpty() ))
        {
            xProp->setPropertyValue( C2U("MediaType"), uno::makeAny( _sMediaType ));
        }
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }
    return xStorage;
}

uno::Reference< embed::XStorage > lcl_getReadStorage(
    const Sequence< beans::PropertyValue >& rMediaDescriptor,
    const uno::Reference< uno::XComponentContext >& xContext)
{
    uno::Reference< embed::XStorage > xStorage;

    try
    {
        apphelper::MediaDescriptorHelper aMDHelper( rMediaDescriptor );
        if( aMDHelper.ISSET_Storage )
        {
            xStorage = aMDHelper.Storage;
        }
        else
        {
            // get XStream from MediaDescriptor
            uno::Reference< io::XInputStream > xStream;
            ::std::vector< beans::PropertyValue > aPropertiesForStorage;
            for( sal_Int32 i=rMediaDescriptor.getLength(); i--; )
            {
                if( rMediaDescriptor[i].Name == "InputStream" )
                    xStream.set( rMediaDescriptor[i].Value, uno::UNO_QUERY );

                // properties understood by storage factory
                // (see package/source/xstor/xfactory.cxx for details)
                if ( rMediaDescriptor[i].Name == "InteractionHandler" || rMediaDescriptor[i].Name == "Password" || rMediaDescriptor[i].Name == "RepairPackage" )
                {
                    aPropertiesForStorage.push_back( rMediaDescriptor[i] );
                }
            }
            OSL_ENSURE( xStream.is(), "No Stream" );
            if( ! xStream.is())
                return xStorage;

            // convert XInputStream to XStorage via the storage factory
            Reference< lang::XSingleServiceFactory > xStorageFact(
                xContext->getServiceManager()->createInstanceWithContext(
                    C2U("com.sun.star.embed.StorageFactory"),
                    xContext ),
                uno::UNO_QUERY_THROW );
            Sequence< uno::Any > aStorageArgs( 3 );
            aStorageArgs[0] <<= xStream;
            aStorageArgs[1] <<= (embed::ElementModes::READ | embed::ElementModes::NOCREATE);
            aStorageArgs[2] <<= ::chart::ContainerHelper::ContainerToSequence( aPropertiesForStorage );
            xStorage.set(
                xStorageFact->createInstanceWithArguments( aStorageArgs ), uno::UNO_QUERY_THROW );
        }

        OSL_ENSURE( xStorage.is(), "No Storage" );
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    return xStorage;
}


} // anonymous namespace

// ----------------------------------------

namespace chart
{

XMLFilter::XMLFilter( Reference< uno::XComponentContext > const & xContext ) :
        m_xContext( xContext ),
        m_bCancelOperation( false )
{}

XMLFilter::~XMLFilter()
{}

// ____ XFilter ____
sal_Bool SAL_CALL XMLFilter::filter(
    const Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException)
{
    bool bResult = false;

    MutexGuard aGuard( m_aMutex );

    // ignore cancel flag at start of function
    // note: is currently ignored during import/export
    if( m_bCancelOperation )
        m_bCancelOperation = false;

    if( m_xSourceDoc.is())
    {
        OSL_ENSURE( ! m_xTargetDoc.is(), "source doc is set -> target document should not be set" );
        if( impl_Export( m_xSourceDoc,
                         aDescriptor ) == 0 )
        {
            m_xSourceDoc = NULL;
            bResult = true;
        }
    }
    else if( m_xTargetDoc.is())
    {
        if( impl_Import( m_xTargetDoc,
                         aDescriptor ) == 0 )
        {
            m_xTargetDoc = NULL;
            bResult = true;
        }
    }
    else
    {
        OSL_FAIL( "filter() called with no document set" );
    }

    return bResult;
}

void SAL_CALL XMLFilter::cancel()
    throw (uno::RuntimeException)
{
    // if mutex is locked set "cancel state"
    // note: is currently ignored in filter-method
    if( ! m_aMutex.tryToAcquire())
    {
        m_bCancelOperation = true;
    }
}

// ____ XImporter ____
void SAL_CALL XMLFilter::setTargetDocument(
    const Reference< lang::XComponent >& Document )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    OSL_ENSURE( ! m_xSourceDoc.is(), "Setting target doc while source doc is set" );

    m_xTargetDoc = Document;
}


// ____ XExporter ____
void SAL_CALL XMLFilter::setSourceDocument(
    const Reference< lang::XComponent >& Document )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    OSL_ENSURE( ! m_xTargetDoc.is(), "Setting source doc while target doc is set" );

    m_xSourceDoc = Document;
}


sal_Int32 XMLFilter::impl_Import(
    const Reference< lang::XComponent > & xDocumentComp,
    const Sequence< beans::PropertyValue > & rMediaDescriptor )
{
    sal_Int32 nWarning = 0;

    OSL_ENSURE( xDocumentComp.is(), "Import: No Model" );
    OSL_ENSURE( m_xContext.is(), "Import: No ComponentContext" );

    if( ! (xDocumentComp.is() &&
           m_xContext.is()))
        return nWarning;

    try
    {
        Reference< lang::XServiceInfo > xServInfo( xDocumentComp, uno::UNO_QUERY_THROW );
        if( ! xServInfo->supportsService( C2U( "com.sun.star.chart2.ChartDocument" )))
        {
            OSL_FAIL( "Import: No ChartDocument" );
            return ERRCODE_SFX_GENERAL;
        }

        Reference< lang::XMultiComponentFactory > xFactory( m_xContext->getServiceManager());
        OSL_ENSURE( xFactory.is(), "Import: No Factory" );
        if( ! xFactory.is())
            return ERRCODE_SFX_GENERAL;

        // create a sax parser
        Reference< xml::sax::XParser > xSaxParser = xml::sax::Parser::create(m_xContext);

        bool bOasis = true;
        isOasisFormat( rMediaDescriptor, bOasis );
        Reference< embed::XStorage > xStorage( lcl_getReadStorage( rMediaDescriptor, m_xContext));
        if( ! xStorage.is())
            return ERRCODE_SFX_GENERAL;

        Reference< document::XGraphicObjectResolver > xGraphicObjectResolver;
        uno::Reference< lang::XMultiServiceFactory > xServiceFactory( xFactory, uno::UNO_QUERY);
        if( xServiceFactory.is())
        {
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[0] <<= xStorage;
            xGraphicObjectResolver.set(
                xServiceFactory->createInstanceWithArguments(
                    C2U("com.sun.star.comp.Svx.GraphicImportHelper"), aArgs ), uno::UNO_QUERY );
        }

        // create XPropertySet with extra informatio for the filter
        /** property map for import info set */
        comphelper::PropertyMapEntry aImportInfoMap[] =
        {
            // necessary properties for XML progress bar at load time
            { MAP_LEN( "ProgressRange" ),   0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressMax" ),     0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((const sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "PrivateData" ), 0,
                &::getCppuType( (Reference<XInterface> *)0 ),
                ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "BaseURI" ), 0,
                &::getCppuType( (OUString *)0 ),
                ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamRelPath" ), 0,
                &::getCppuType( (OUString *)0 ),
                ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamName" ), 0,
                &::getCppuType( (OUString *)0 ),
                ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "BuildId" ), 0,
                &::getCppuType( (OUString *)0 ),
                ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };
        uno::Reference< beans::XPropertySet > xImportInfo(
                    comphelper::GenericPropertySet_CreateInstance(
                                new comphelper::PropertySetInfo( aImportInfoMap ) ) );

        // Set base URI and Hierarchical Name
        OUString aHierarchName, aBaseUri;
        uno::Reference< frame::XModel > xModel( m_xSourceDoc, uno::UNO_QUERY );
        if( xModel.is() )
        {
            uno::Sequence< beans::PropertyValue > aModProps = xModel->getArgs();
            for( sal_Int32 nInd = 0; nInd < aModProps.getLength(); nInd++ )
            {
                if( aModProps[nInd].Name.equals( C2U( "HierarchicalDocumentName" ) ) )
                {
                    // Actually this argument only has meaning for embedded documents
                    aModProps[nInd].Value >>= aHierarchName;
                }
                else if( aModProps[nInd].Name.equals( C2U( "DocumentBaseURL" ) ) )
                {
                    aModProps[nInd].Value >>= aBaseUri;
                }
            }
        }

        if( !aBaseUri.isEmpty() )
            xImportInfo->setPropertyValue( C2U("BaseURI"), uno::makeAny( aBaseUri ) );

        if( !aHierarchName.isEmpty() )
            xImportInfo->setPropertyValue( C2U("StreamRelPath"), uno::makeAny( aHierarchName ) );

        // import meta information
        if( bOasis )
            nWarning |= impl_ImportStream(
                C2U( sXML_metaStreamName ),
                C2U( sXML_import_chart_oasis_meta_service ),
                xStorage, xSaxParser, xFactory, xGraphicObjectResolver, xImportInfo );

        // import styles
        nWarning |= impl_ImportStream(
            C2U( sXML_styleStreamName ),
            bOasis
            ? C2U( sXML_import_chart_oasis_styles_service )
            : C2U( sXML_import_chart_styles_service ),
            xStorage, xSaxParser, xFactory, xGraphicObjectResolver, xImportInfo );

        // import content
        sal_Int32 nContentWarning = impl_ImportStream(
            C2U( sXML_contentStreamName ),
            bOasis
            ? C2U( sXML_import_chart_oasis_content_service )
            : C2U( sXML_import_chart_content_service ),
            xStorage, xSaxParser, xFactory, xGraphicObjectResolver, xImportInfo );
        nWarning |= nContentWarning;

        // import of "content.xml" didn't work - try old "Content.xml" stream
        if( nContentWarning != 0 )
        {
            nWarning = impl_ImportStream(
                C2U( sXML_oldContentStreamName ),
                C2U( sXML_import_chart_old_content_service ),
                xStorage, xSaxParser, xFactory, xGraphicObjectResolver, xImportInfo );
        }
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );

        // something went awry
        nWarning = ERRCODE_SFX_GENERAL;
    }

    return nWarning;
}

sal_Int32 XMLFilter::impl_ImportStream(
    const OUString & rStreamName,
    const OUString & rServiceName,
    const Reference< embed::XStorage > & xStorage,
    const Reference< xml::sax::XParser > & xParser,
    const Reference< lang::XMultiComponentFactory > & xFactory,
    const Reference< document::XGraphicObjectResolver > & xGraphicObjectResolver,
    uno::Reference< beans::XPropertySet >& xImportInfo )
{
    sal_Int32 nWarning = ERRCODE_SFX_GENERAL;

    Reference< container::XNameAccess > xNameAcc( xStorage, uno::UNO_QUERY );
    if( ! (xNameAcc.is() &&
           xNameAcc->hasByName( rStreamName )))
        return 0;

    if( xImportInfo.is() )
        xImportInfo->setPropertyValue( C2U("StreamName"), uno::makeAny( rStreamName ) );

    if( xStorage.is() &&
        xStorage->isStreamElement( rStreamName ) )
    {
        try
        {
            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream.set(
                xStorage->openStreamElement(
                    rStreamName,
                    embed::ElementModes::READ | embed::ElementModes::NOCREATE ),
                uno::UNO_QUERY );

            // todo: encryption

            if( aParserInput.aInputStream.is())
            {
                sal_Int32 nArgs = 0;
                if( xGraphicObjectResolver.is())
                    nArgs++;
                if( xImportInfo.is())
                    nArgs++;

                uno::Sequence< uno::Any > aFilterCompArgs( nArgs );

                nArgs = 0;
                if( xGraphicObjectResolver.is())
                    aFilterCompArgs[nArgs++] <<= xGraphicObjectResolver;
                if( xImportInfo.is())
                    aFilterCompArgs[ nArgs++ ] <<= xImportInfo;

                Reference< xml::sax::XDocumentHandler > xDocHandler(
                    xFactory->createInstanceWithArgumentsAndContext( rServiceName, aFilterCompArgs, m_xContext ),
                    uno::UNO_QUERY_THROW );


                Reference< document::XImporter > xImporter( xDocHandler, uno::UNO_QUERY_THROW );
                xImporter->setTargetDocument( Reference< lang::XComponent >( m_xTargetDoc, uno::UNO_QUERY_THROW ));

                if ( !m_sDocumentHandler.isEmpty() )
                {
                    try
                    {
                        uno::Sequence< uno::Any > aArgs(2);
                        beans::NamedValue aValue;
                        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentHandler"));
                        aValue.Value <<= xDocHandler;
                        aArgs[0] <<= aValue;
                        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Model"));
                        aValue.Value <<= m_xTargetDoc;
                        aArgs[1] <<= aValue;

                        xDocHandler.set(xFactory->createInstanceWithArgumentsAndContext(m_sDocumentHandler,aArgs,m_xContext), uno::UNO_QUERY );
                        xImporter.set(xDocHandler,uno::UNO_QUERY);
                    }
                    catch (const uno::Exception&)
                    {
                        OSL_FAIL("Exception caught!");
                    }
                }
                xParser->setDocumentHandler( xDocHandler );
                xParser->parseStream( aParserInput );
            }

            // load was successful
            nWarning = 0;
        }
        catch (const xml::sax::SAXParseException&)
        {
            // todo: if encrypted: ERRCODE_SFX_WRONGPASSWORD
        }
        catch (const xml::sax::SAXException&)
        {
            // todo: if encrypted: ERRCODE_SFX_WRONGPASSWORD
        }
        catch (const packages::zip::ZipIOException&)
        {
            nWarning = ERRCODE_IO_BROKENPACKAGE;
        }
        catch (const io::IOException&)
        {
        }
        catch (const uno::Exception& rEx)
        {
            ASSERT_EXCEPTION(rEx);
        }
    }

    return nWarning;
}

sal_Int32 XMLFilter::impl_Export(
    const Reference< lang::XComponent > & xDocumentComp,
    const Sequence< beans::PropertyValue > & rMediaDescriptor )
{
    //save

    sal_Int32 nWarning = 0;

    OSL_ENSURE( xDocumentComp.is(), "Export: No Model" );
    OSL_ENSURE( m_xContext.is(), "Export: No ComponentContext" );

    if( !xDocumentComp.is() || !m_xContext.is() )
        return nWarning;

    try
    {
        Reference< lang::XServiceInfo > xServInfo( xDocumentComp, uno::UNO_QUERY_THROW );
        if( ! xServInfo->supportsService( C2U( "com.sun.star.chart2.ChartDocument" )))
        {
            OSL_FAIL( "Export: No ChartDocument" );
            return ERRCODE_SFX_GENERAL;
        }

        Reference< lang::XMultiComponentFactory > xFactory( m_xContext->getServiceManager());
        OSL_ENSURE( xFactory.is(), "Export: No Factory" );
        if( ! xFactory.is())
            return ERRCODE_SFX_GENERAL;
        uno::Reference< lang::XMultiServiceFactory > xServiceFactory( m_xContext->getServiceManager(), uno::UNO_QUERY);
        if( ! xServiceFactory.is())
            return ERRCODE_SFX_GENERAL;

        uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(m_xContext);

        bool bOasis = true;
        isOasisFormat( rMediaDescriptor, bOasis );

        uno::Reference< embed::XStorage > xStorage( lcl_getWriteStorage( rMediaDescriptor, m_xContext, getMediaType(bOasis) ) );
        OSL_ENSURE( xStorage.is(), "No Storage" );
        if( ! xStorage.is())
            return ERRCODE_SFX_GENERAL;

        uno::Reference< xml::sax::XDocumentHandler> xDocHandler( xSaxWriter, uno::UNO_QUERY );

        if ( !m_sDocumentHandler.isEmpty() )
        {
            try
            {
                uno::Sequence< uno::Any > aArgs(2);
                beans::NamedValue aValue;
                aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentHandler"));
                aValue.Value <<= xDocHandler;
                aArgs[0] <<= aValue;
                aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Model"));
                aValue.Value <<= xDocumentComp;
                aArgs[1] <<= aValue;

                xDocHandler.set(xServiceFactory->createInstanceWithArguments(m_sDocumentHandler,aArgs), uno::UNO_QUERY );
                xSaxWriter.set(xDocHandler,uno::UNO_QUERY);
            }
            catch (const uno::Exception&)
            {
                OSL_FAIL("Exception caught!");
            }
        }

        uno::Sequence< uno::Any > aGraphicResolverArgs(1);
        aGraphicResolverArgs[0] <<= xStorage;
        Reference< document::XGraphicObjectResolver > xGraphicObjectResolver(
            xServiceFactory->createInstanceWithArguments(
                C2U("com.sun.star.comp.Svx.GraphicExportHelper"), aGraphicResolverArgs ), uno::UNO_QUERY );

        uno::Reference< beans::XPropertySet > xInfoSet;
        {
            // property map for export info set
            comphelper::PropertyMapEntry aExportInfoMap[] =
            {
                { MAP_LEN("UsePrettyPrinting"), 0, &::getBooleanCppuType(), beans::PropertyAttribute::MAYBEVOID, 0},
                { MAP_LEN("BaseURI"), 0, &::getCppuType( (OUString *)0 ), beans::PropertyAttribute::MAYBEVOID, 0 },
                { MAP_LEN("StreamRelPath"), 0, &::getCppuType( (OUString *)0 ), beans::PropertyAttribute::MAYBEVOID, 0 },
                { MAP_LEN("StreamName"), 0, &::getCppuType( (OUString *)0 ), beans::PropertyAttribute::MAYBEVOID, 0 },
                { MAP_LEN("ExportTableNumberList"), 0, &::getBooleanCppuType(), beans::PropertyAttribute::MAYBEVOID, 0 },
                { NULL, 0, 0, NULL, 0, 0 }
            };

            xInfoSet = comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) );

            SvtSaveOptions aSaveOpt;
            OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
            sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
            xInfoSet->setPropertyValue( sUsePrettyPrinting, uno::makeAny( bUsePrettyPrinting ) );
            if( ! bOasis )
                xInfoSet->setPropertyValue( C2U("ExportTableNumberList"), uno::makeAny( true ));
        }

        sal_Int32 nArgs = 2;
        if( xGraphicObjectResolver.is())
            nArgs++;

        uno::Sequence< uno::Any > aFilterProperties( nArgs );
        {
            nArgs = 0;
            aFilterProperties[ nArgs++ ] <<= xInfoSet;
            aFilterProperties[ nArgs++ ] <<= xDocHandler;
            if( xGraphicObjectResolver.is())
                aFilterProperties[ nArgs++ ] <<= xGraphicObjectResolver;
        }

        // export meta information
        if( bOasis )
            nWarning |= impl_ExportStream(
                C2U( sXML_metaStreamName ),
                C2U( sXML_export_chart_oasis_meta_service ),
                xStorage, xSaxWriter, xServiceFactory, aFilterProperties );

        // export styles
        nWarning |= impl_ExportStream(
            C2U( sXML_styleStreamName ),
            bOasis
            ? C2U( sXML_export_chart_oasis_styles_service )
            : C2U( sXML_export_chart_styles_service ),
            xStorage, xSaxWriter, xServiceFactory, aFilterProperties );

        // export content
        sal_Int32 nContentWarning = impl_ExportStream(
            C2U( sXML_contentStreamName ),
            bOasis
            ? C2U( sXML_export_chart_oasis_content_service )
            : C2U( sXML_export_chart_content_service ),
            xStorage, xSaxWriter, xServiceFactory, aFilterProperties );
        nWarning |= nContentWarning;

        Reference< lang::XComponent > xComp( xGraphicObjectResolver, uno::UNO_QUERY );
        if( xComp.is())
            xComp->dispose();

        uno::Reference<embed::XTransactedObject> xTransact( xStorage ,uno::UNO_QUERY);
        if ( xTransact.is() )
            xTransact->commit();
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );

        // something went awry
        nWarning = ERRCODE_SFX_GENERAL;
    }

    return nWarning;
}

sal_Int32 XMLFilter::impl_ExportStream(
    const OUString & rStreamName,
    const OUString & rServiceName,
    const Reference< embed::XStorage > & xStorage,
    const uno::Reference< xml::sax::XWriter >& xActiveDataSource,
    const Reference< lang::XMultiServiceFactory >& xServiceFactory,
    const Sequence< uno::Any > & rFilterProperties )
{
    sal_Int32 nWarning = 0;

    try
    {
        if( !xServiceFactory.is() )
            return ERRCODE_SFX_GENERAL;
        if( !xStorage.is() )
            return ERRCODE_SFX_GENERAL;
        if ( !xActiveDataSource.is() )
            return ERRCODE_SFX_GENERAL;

        uno::Reference< io::XStream > xStream( xStorage->openStreamElement(
            rStreamName, embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE ) );
        if ( !xStream.is() )
            return ERRCODE_SFX_GENERAL;
        uno::Reference< io::XOutputStream > xOutputStream( xStream->getOutputStream() );
        if ( !xOutputStream.is() )
            return ERRCODE_SFX_GENERAL;

        uno::Reference< beans::XPropertySet > xStreamProp( xOutputStream, uno::UNO_QUERY );
        if(xStreamProp.is()) try
        {
            xStreamProp->setPropertyValue( C2U("MediaType"), uno::makeAny( C2U("text/xml") ) );
            xStreamProp->setPropertyValue( C2U("Compressed"), uno::makeAny( sal_True ) );//@todo?
            xStreamProp->setPropertyValue( C2U("UseCommonStoragePasswordEncryption"), uno::makeAny( sal_True ) );
        }
        catch (const uno::Exception& rEx)
        {
            ASSERT_EXCEPTION( rEx );
        }

        xActiveDataSource->setOutputStream(xOutputStream);

        // set Base URL
        {
            uno::Reference< beans::XPropertySet > xInfoSet;
            if( rFilterProperties.getLength() > 0 )
                rFilterProperties.getConstArray()[0] >>= xInfoSet;
            OSL_ENSURE( xInfoSet.is(), "missing infoset for export" );
            if( xInfoSet.is() )
                xInfoSet->setPropertyValue( C2U("StreamName"), uno::makeAny( rStreamName ) );
        }

        Reference< XExporter > xExporter( xServiceFactory->createInstanceWithArguments(
            rServiceName, rFilterProperties ), uno::UNO_QUERY);
        if ( !xExporter.is() )
            return ERRCODE_SFX_GENERAL;

        xExporter->setSourceDocument( m_xSourceDoc );

        uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY );
        if ( !xFilter.is() )
            return ERRCODE_SFX_GENERAL;

        uno::Sequence < beans::PropertyValue > aMediaDesc(0);
        //@todo? filter properties? ... url? ...
        xFilter->filter( aMediaDesc );
    }
    catch (const uno::Exception& rEx)
    {
        ASSERT_EXCEPTION( rEx );
    }
    return nWarning;
}

// --------------------------------------------------------------------------------

Sequence< OUString > XMLFilter::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.document.ImportFilter" );
    aServices[ 1 ] = C2U( "com.sun.star.document.ExportFilter" );

    // todo: services are incomplete.  Missing:
    // XInitialization, XNamed
    return aServices;
}
// -----------------------------------------------------------------------------

void XMLFilter::isOasisFormat(const Sequence< beans::PropertyValue >& _rMediaDescriptor, bool & rOutOASIS )
{
    apphelper::MediaDescriptorHelper aMDHelper( _rMediaDescriptor );
    if( aMDHelper.ISSET_FilterName )
        rOutOASIS = aMDHelper.FilterName == "chart8";
}
// -----------------------------------------------------------------------------
::rtl::OUString XMLFilter::getMediaType(bool _bOasis)
{
    return _bOasis ? MIMETYPE_OASIS_OPENDOCUMENT_CHART : MIMETYPE_VND_SUN_XML_CHART;
}
// -----------------------------------------------------------------------------

APPHELPER_XSERVICEINFO_IMPL( XMLFilter, C2U( "com.sun.star.comp.chart2.XMLFilter" ) );
// -----------------------------------------------------------------------------

void XMLReportFilterHelper::isOasisFormat(const Sequence< beans::PropertyValue >& _rMediaDescriptor, bool & rOutOASIS )
{
    apphelper::MediaDescriptorHelper aMDHelper( _rMediaDescriptor );
    if( aMDHelper.ISSET_FilterName )
        rOutOASIS = aMDHelper.FilterName == "StarOffice XML (Base) Report Chart";
}
// -----------------------------------------------------------------------------
::rtl::OUString XMLReportFilterHelper::getMediaType(bool )
{
    return MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
