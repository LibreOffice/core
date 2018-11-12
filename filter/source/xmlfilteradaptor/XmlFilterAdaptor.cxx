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

#include <iostream>
#include <stdlib.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>
#include "XmlFilterAdaptor.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/fileurl.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/scopeguard.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/pathoptions.hxx>

using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::style;
using namespace com::sun::star::xml;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::frame;
using namespace ::com::sun::star::task;

bool XmlFilterAdaptor::importImpl( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    OUString udConvertClass    = msUserData[0];
    const OUString sXMLImportService = msUserData[2];
    sal_Int32 nSteps= 0;

    utl::MediaDescriptor aMediaMap(aDescriptor);
    Reference< XStatusIndicator > xStatusIndicator(aMediaMap.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >()));

    if (xStatusIndicator.is()){
        xStatusIndicator->start( "Loading :", 4);
    }

    Sequence< Any > aAnys(1);
    OUString aBaseURI;
    if (aMediaMap.find(OUString( "URL" ))->second >>= aBaseURI)
    {
        INetURLObject aURLObj(aBaseURI);
        // base URI in this case is the URI of the actual saving location
        // aURLObj.removeSegment();
        aBaseURI = aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    }

    // create an XProperty set to configure the exporter for pretty printing
    PropertyMapEntry aImportInfoMap[] =
    {
        { OUString("BaseURI"), 0, ::cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
        { OUString("DefaultDocumentSettings"), 0,
          ::cppu::UnoType<Sequence<PropertyValue>>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    Reference< XPropertySet > xInfoSet(
            GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
    xInfoSet->setPropertyValue( "BaseURI", makeAny( aBaseURI ));

    OUString aFilterName;
    auto It = aMediaMap.find(OUString("FilterName"));
    if (It != aMediaMap.end() && (It->second >>= aFilterName)
        && aFilterName == "OpenDocument Text Flat XML")
    {
        PropertyValue EmptyDbFieldHidesPara("EmptyDbFieldHidesPara", 0, Any(false),
                                            PropertyState::PropertyState_DIRECT_VALUE);
        Sequence<PropertyValue> aSettings{ EmptyDbFieldHidesPara };
        xInfoSet->setPropertyValue("DefaultDocumentSettings", makeAny(aSettings));
    }
    aAnys[0] <<= xInfoSet;


    Reference < XDocumentHandler > xHandler( mxContext->getServiceManager()->createInstanceWithArgumentsAndContext( sXMLImportService, aAnys, mxContext ), UNO_QUERY );
    if (!xHandler.is()) {
        SAL_WARN("filter.xmlfa", "XmlFilterAdaptor: unable to create service " << sXMLImportService);
        return false;
    }
    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument ( mxDoc );

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }


    // Creating a ConverterBridge instance

    Reference< XInterface > xConvBridge(mxContext->getServiceManager()->createInstanceWithContext(udConvertClass, mxContext), UNO_QUERY);
    if (!xConvBridge.is()) {
        SAL_WARN("filter.xmlfa", "XmlFilterAdaptor: unable to create service " << udConvertClass);
        return false;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    Reference< XImportFilter > xConverter( xConvBridge, UNO_QUERY );

    // prevent unnecessary broadcasting when loading
    Reference< XModel > xModel( mxDoc, UNO_QUERY );
    if( xModel.is() )
        xModel->lockControllers();
    comphelper::ScopeGuard guard([&]() {
        // cleanup when leaving
        if( xModel.is() )
            xModel->unlockControllers();
    });

    //Template Loading if Required

    if (!msTemplateName.isEmpty()){
        Reference< XStyleFamiliesSupplier > xstylefamiliessupplier(mxDoc, UNO_QUERY);
        Reference< XStyleLoader > xstyleLoader (xstylefamiliessupplier->getStyleFamilies(), UNO_QUERY);
        if(xstyleLoader.is()){
            Sequence<css::beans::PropertyValue> aValue = xstyleLoader->getStyleLoaderOptions();

            //Load the Styles from the Template URL Supplied in the TypeDetection file
            if(!comphelper::isFileUrl(msTemplateName))
            {
                SvtPathOptions aOptions;
                msTemplateName = aOptions.SubstituteVariable("$(progurl)") + "/" + msTemplateName;
            }

            xstyleLoader->loadStylesFromURL(msTemplateName,aValue);
        }
    }

//    sal_Bool xconv_ret = sal_True;

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }

    // Calling Filtering Component

    try {
        if (!xConverter->importer(aDescriptor,xHandler,msUserData)) {
            if (xStatusIndicator.is())
                   xStatusIndicator->end();
            return false;
        }
    }
    catch( const Exception& e )
    {
        if (xStatusIndicator.is())
               xStatusIndicator->end();

        SAL_WARN("filter.xmlfa", "XmlFilterAdaptor: " << e);
        return false;
    }
    if (xStatusIndicator.is()) {
        xStatusIndicator->setValue(nSteps++);
        xStatusIndicator->end();
    }
    return true;
}

bool XmlFilterAdaptor::exportImpl( const Sequence< css::beans::PropertyValue >& aDescriptor )
{

    OUString udConvertClass = msUserData[0];
    OUString udExport = msUserData[3];

    // Status Bar
    sal_Int32 nSteps= 1;
    utl::MediaDescriptor aMediaMap(aDescriptor);
    Reference< XStatusIndicator > xStatusIndicator(aMediaMap.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >()));

    if (xStatusIndicator.is())
       xStatusIndicator->start( "Saving :", 3);

    // Set up converter bridge.
    Reference< css::xml::XExportFilter > xConverter(mxContext->getServiceManager()->createInstanceWithContext( udConvertClass, mxContext ), UNO_QUERY);
    if (!xConverter.is()) {
        SAL_WARN("filter.xmlfa", "XmlFilterAdaptor: unable to create service " << udConvertClass);
        return false;
    }

    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    //put filter component into exporting state
    if (!xConverter->exporter(aDescriptor, msUserData)) {
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return false;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    try{
        // create the xml exporter service and supply the converter component
        // which implements the document handler
        Sequence < Any > aAnys (2);
        aAnys[0] <<= xConverter;


        // pretty printing is confusing for some filters so it is disabled by default
        bool bPrettyPrint =
            (msUserData.getLength() > 6 && msUserData[6].equalsIgnoreAsciiCase("true"));

        // export of <text:number> element for <text:list-item> elements are
        // needed for certain filters.
        bool bExportTextNumberElementForListItems =
                            ( msUserData.getLength() > 7 &&
                              msUserData[7].equalsIgnoreAsciiCase("true") );

        // get the base URI, so we can use relative links
        OUString aBaseURI;
        if (aMediaMap.find(OUString( "URL" ))->second >>= aBaseURI)
        {
            INetURLObject aURLObj(aBaseURI);
            // base URI in this case is the URI of the actual saving location
            // aURLObj.removeSegment();
            aBaseURI = aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        }

        // create an XProperty set to configure the exporter for pretty printing
        PropertyMapEntry aImportInfoMap[] =
         {
             { OUString("UsePrettyPrinting"), 0, cppu::UnoType<sal_Bool>::get(), PropertyAttribute::MAYBEVOID, 0},
             { OUString("ExportTextNumberElement"), 0, cppu::UnoType<sal_Bool>::get(), PropertyAttribute::MAYBEVOID, 0},
             { OUString("BaseURI"), 0, ::cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
             { OUString(), 0, css::uno::Type(), 0, 0 }
         };

        Reference< XPropertySet > xInfoSet(
            GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
        xInfoSet->setPropertyValue("UsePrettyPrinting", makeAny( bPrettyPrint ));
        xInfoSet->setPropertyValue(
                        "ExportTextNumberElement",
                        makeAny( bExportTextNumberElementForListItems ));
        xInfoSet->setPropertyValue("BaseURI", makeAny( aBaseURI ));
        aAnys[1] <<= xInfoSet;

        Reference< XExporter > xExporter( mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                       udExport, aAnys, mxContext ), UNO_QUERY_THROW );

        // attach to source document
        xExporter->setSourceDocument( mxDoc );

        // get XFilter interface
        Reference< XFilter > xFilter( xExporter, UNO_QUERY_THROW );

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        // call the actual filtering component
        if (!xFilter->filter(aDescriptor))
        {
            if (xStatusIndicator.is())
                   xStatusIndicator->end();
            return false;
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN("filter.xmlfa", "XmlFilterAdaptor: " << e);
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return false;
    }

    // done
    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return true;
}

sal_Bool SAL_CALL XmlFilterAdaptor::filter( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    return meType == FILTER_EXPORT ? exportImpl ( aDescriptor ) : importImpl ( aDescriptor );
}
void SAL_CALL XmlFilterAdaptor::cancel(  )
{
}
// XExporter
void SAL_CALL XmlFilterAdaptor::setSourceDocument( const Reference< css::lang::XComponent >& xDoc )
{
    meType = FILTER_EXPORT;
    mxDoc = xDoc;
}

// XImporter
void SAL_CALL XmlFilterAdaptor::setTargetDocument( const Reference< css::lang::XComponent >& xDoc )
{
    meType = FILTER_IMPORT;
    mxDoc = xDoc;
}
// XInitialization
void SAL_CALL XmlFilterAdaptor::initialize( const Sequence< Any >& aArguments )
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        comphelper::SequenceAsHashMap aMap(aAnySeq);
        msFilterName = aMap.getUnpackedValueOrDefault(
            "Type", OUString());
        msUserData = aMap.getUnpackedValueOrDefault(
            "UserData", Sequence< OUString >());
        msTemplateName = aMap.getUnpackedValueOrDefault(
            "TemplateName", OUString());
    }
}
OUString XmlFilterAdaptor_getImplementationName ()
{
    return OUString( "com.sun.star.comp.Writer.XmlFilterAdaptor" );
}

Sequence< OUString > XmlFilterAdaptor_getSupportedServiceNames(  )
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ExportFilter";
    pArray[1] =  "com.sun.star.document.ImportFilter";
    return aRet;
}

Reference< XInterface > XmlFilterAdaptor_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new XmlFilterAdaptor( comphelper::getComponentContext(rSMgr) ));
}

// XServiceInfo
OUString SAL_CALL XmlFilterAdaptor::getImplementationName(  )
{
    return XmlFilterAdaptor_getImplementationName();
}

sal_Bool SAL_CALL XmlFilterAdaptor::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL XmlFilterAdaptor::getSupportedServiceNames(  )
{
    return XmlFilterAdaptor_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
