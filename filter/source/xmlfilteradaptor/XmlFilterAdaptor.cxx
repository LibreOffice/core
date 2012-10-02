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
#include <ctype.h>
#include <stdio.h>
#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>
#include "XmlFilterAdaptor.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>

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
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;

using ::rtl::OUString;

#define MAP_LEN(x) x, sizeof(x) - 1

Reference< com::sun::star::frame::XModel > xModel;

sal_Bool SAL_CALL XmlFilterAdaptor::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    OUString udConvertClass=msUserData[0];
    OUString udImport =msUserData[2];
    sal_Int32 nSteps= 0;
    sal_Int32 nProgressRange = 4;

    comphelper::MediaDescriptor aMediaMap(aDescriptor);
    Reference< XStatusIndicator > xStatusIndicator(aMediaMap.getUnpackedValueOrDefault(
        comphelper::MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >()));

    if (xStatusIndicator.is()){
        xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "Loading :" )),nProgressRange);
    }

    OUString sXMLImportService (  udImport  );
    const OUString sSaxParser ( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser") );
    Reference < XParser > xSaxParser( mxMSF->createInstance( sSaxParser ), UNO_QUERY );

    Sequence< Any > aAnys(1);
    OUString aBaseURI;
    if (aMediaMap.find(OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" )))->second >>= aBaseURI)
    {
        INetURLObject aURLObj(aBaseURI);
        // base URI in this case is the URI of the actual saving location
        // aURLObj.removeSegment();
        aBaseURI = aURLObj.GetMainURL(INetURLObject::NO_DECODE);
    }

    // create an XProperty set to configure the exporter for pretty printing
    PropertyMapEntry aImportInfoMap[] =
     {
        { MAP_LEN( "BaseURI" ), 0, &::getCppuType((const OUString*)0), PropertyAttribute::MAYBEVOID, 0},
         { NULL, 0, 0, NULL, 0, 0 }
     };

     Reference< XPropertySet > xInfoSet(
        GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
     xInfoSet->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseURI" )), makeAny( aBaseURI ));
    aAnys[0] <<= xInfoSet;


    Reference < XDocumentHandler > xHandler( mxMSF->createInstanceWithArguments( sXMLImportService, aAnys ), UNO_QUERY );
    if(! xHandler.is()) {
        OSL_FAIL("XMLReader::Read: %s Unable to create service instance xHandler\n" );
        return sal_False;
    }
    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument ( mxDoc );

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }

    //*********************
    // Creating a ConverterBridge instance
    //*********************
    Reference< XInterface > xConvBridge(mxMSF->createInstance( udConvertClass ), UNO_QUERY);
    if(! xConvBridge.is()){
        OSL_FAIL( "XMLReader::Read: %s service missing\n" );
        return sal_False;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    Reference< XImportFilter > xConverter( xConvBridge, UNO_QUERY );

     //********************
    //Template Loading if Required
    //********************
    if (!msTemplateName.isEmpty()){
        Reference< XStyleFamiliesSupplier > xstylefamiliessupplier(mxDoc, UNO_QUERY);

        Reference< XNameAccess >xName;
        if(xstylefamiliessupplier.is()){
            xName=xstylefamiliessupplier->getStyleFamilies();
        }
        Reference< XStyleLoader > xstyleLoader (xstylefamiliessupplier->getStyleFamilies(), UNO_QUERY);


        if(xstyleLoader.is()){
            xName=xstylefamiliessupplier->getStyleFamilies();
        }

        Sequence < OUString > elementNames = xName->getElementNames();
        if(xstyleLoader.is()){
            Sequence<com::sun::star::beans::PropertyValue> pValue=xstyleLoader->getStyleLoaderOptions();

            //Load the Styles from the Template URL Supplied in the TypeDetection file
            if(msTemplateName.indexOf(OUString( RTL_CONSTASCII_USTRINGPARAM( "file:" )))==-1)
            {
                Reference< XConfigManager >xCfgMgr ( mxMSF->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" )) ), UNO_QUERY );
                OUString PathString(xCfgMgr->substituteVariables(OUString(RTL_CONSTASCII_USTRINGPARAM("$(progurl)"))));
                PathString = PathString.concat(OUString( RTL_CONSTASCII_USTRINGPARAM( "/" )));
                msTemplateName=PathString.concat(msTemplateName);
            }

            xstyleLoader->loadStylesFromURL(msTemplateName,pValue);
        }
    }

//    sal_Bool xconv_ret = sal_True;

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }
    //*********************
    // Calling Filtering Component
    //*********************
    try {
        if (!xConverter->importer(aDescriptor,xHandler,msUserData)) {
            if (xStatusIndicator.is())
                   xStatusIndicator->end();
            return sal_False;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    catch( const Exception& e )
#else
    catch( const Exception& )
#endif
    {
        if (xStatusIndicator.is())
               xStatusIndicator->end();

        OSL_FAIL( ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        return sal_False;
    }
    if (xStatusIndicator.is()) {
        xStatusIndicator->setValue(nSteps++);
        xStatusIndicator->end();
    }
    return sal_True;
}

sal_Bool SAL_CALL XmlFilterAdaptor::exportImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{

    OUString udConvertClass = msUserData[0];
    OUString udExport = msUserData[3];

    // Status Bar
    sal_Int32 nSteps= 1;
    sal_Int32 nProgressRange(3);
    comphelper::MediaDescriptor aMediaMap(aDescriptor);
    Reference< XStatusIndicator > xStatusIndicator(aMediaMap.getUnpackedValueOrDefault(
        comphelper::MediaDescriptor::PROP_STATUSINDICATOR(), Reference< XStatusIndicator >()));

    if (xStatusIndicator.is())
       xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "Saving :" )),nProgressRange);

    // Set up converter bridge.
    Reference< com::sun::star::xml::XExportFilter > xConverter(mxMSF->createInstance(udConvertClass ), UNO_QUERY);
    if(! xConverter.is()){
      OSL_FAIL( "xml export sub service missing" );
      return sal_False;
    }

    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    //put filter component into exporting state
    if (!xConverter->exporter(aDescriptor, msUserData)) {
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return sal_False;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    try{
        // create the xml exporter service and supply the converter component
        // which implements the document handler
        Sequence < Any > aAnys (2);
        aAnys[0] <<= xConverter;


        // pretty printing is confusing for some filters so it is disabled by default
        sal_Bool bPrettyPrint =
            (msUserData.getLength() > 6 && msUserData[6].equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("true")));

        // export of <text:number> element for <text:list-item> elements are
        // needed for certain filters.
        sal_Bool bExportTextNumberElementForListItems =
                            ( msUserData.getLength() > 7 &&
                              msUserData[7].equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("true")) );

        // get the base URI, so we can use relative links
        OUString aBaseURI;
        if (aMediaMap.find(OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" )))->second >>= aBaseURI)
        {
            INetURLObject aURLObj(aBaseURI);
            // base URI in this case is the URI of the actual saving location
            // aURLObj.removeSegment();
            aBaseURI = aURLObj.GetMainURL(INetURLObject::NO_DECODE);
        }

        // create an XProperty set to configure the exporter for pretty printing
         PropertyMapEntry aImportInfoMap[] =
         {
             { MAP_LEN( "UsePrettyPrinting" ), 0, &::getCppuType((const sal_Bool*)0), PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ExportTextNumberElement" ), 0, &::getCppuType((const sal_Bool*)0), PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "BaseURI" ), 0, &::getCppuType((const OUString*)0), PropertyAttribute::MAYBEVOID, 0},
             { NULL, 0, 0, NULL, 0, 0 }
         };

         Reference< XPropertySet > xInfoSet(
            GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
         xInfoSet->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "UsePrettyPrinting" )), makeAny( bPrettyPrint ));
        xInfoSet->setPropertyValue(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportTextNumberElement" )),
                        makeAny( bExportTextNumberElementForListItems ));
         xInfoSet->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseURI" )), makeAny( aBaseURI ));
        aAnys[1] <<= xInfoSet;

        Reference< XExporter > xExporter( mxMSF->createInstanceWithArguments (
                       udExport, aAnys ), UNO_QUERY_THROW );

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
            return sal_False;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    catch( const Exception& exE )
#else
    catch( const Exception& )
#endif
    {
        OSL_FAIL( ::rtl::OUStringToOString( exE.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return sal_False;
    }

    // done
    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return sal_True;
}

sal_Bool SAL_CALL XmlFilterAdaptor::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
  throw (RuntimeException)
{
    return meType == FILTER_EXPORT ? exportImpl ( aDescriptor ) : importImpl ( aDescriptor );
}
void SAL_CALL XmlFilterAdaptor::cancel(  )
    throw (RuntimeException)
{
}
// XExporter
void SAL_CALL XmlFilterAdaptor::setSourceDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    meType = FILTER_EXPORT;
    mxDoc = xDoc;
        com::sun::star::uno::Reference< com::sun::star::frame::XModel >rModel ( com::sun::star::uno::Reference< com::sun::star::frame::XModel >::query( xDoc ) );
    xModel=rModel;

}

// XImporter
void SAL_CALL XmlFilterAdaptor::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    meType = FILTER_IMPORT;
    mxDoc = xDoc;
    //xModel = uno::Reference< frame::XModel >::query( xDoc );
}
// XInitialization
void SAL_CALL XmlFilterAdaptor::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        comphelper::SequenceAsHashMap aMap(aAnySeq);
        msFilterName = aMap.getUnpackedValueOrDefault(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Type" )), OUString());
        msUserData = aMap.getUnpackedValueOrDefault(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "UserData" )), Sequence< OUString >());
        msTemplateName = aMap.getUnpackedValueOrDefault(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "TemplateName" )), OUString());
    }
}
OUString XmlFilterAdaptor_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.XmlFilterAdaptor" ) );
}
#define SERVICE_NAME1 "com.sun.star.document.ExportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ImportFilter"
sal_Bool SAL_CALL XmlFilterAdaptor_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName == SERVICE_NAME1 || ServiceName == SERVICE_NAME2;
}
Sequence< OUString > SAL_CALL XmlFilterAdaptor_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
    pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
    return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

Reference< XInterface > SAL_CALL XmlFilterAdaptor_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new XmlFilterAdaptor( rSMgr );
}

// XServiceInfo
OUString SAL_CALL XmlFilterAdaptor::getImplementationName(  )
    throw (RuntimeException)
{
    return XmlFilterAdaptor_getImplementationName();
}
sal_Bool SAL_CALL XmlFilterAdaptor::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return XmlFilterAdaptor_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL XmlFilterAdaptor::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return XmlFilterAdaptor_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
