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
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>
#include "XmlFilterAdaptor.hxx"
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XImportFilter2.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/fileurl.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/scopeguard.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/pathoptions.hxx>
#include <xmloff/xmlimp.hxx>

#include <strings.hrc>

using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
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
        utl::MediaDescriptor::PROP_STATUSINDICATOR, Reference< XStatusIndicator >()));

    if (xStatusIndicator.is()){
        xStatusIndicator->start(FilterResId(STR_FILTER_DOC_LOADING), 4);
    }

    OUString aBaseURI;
    if (aMediaMap.find(u"URL"_ustr)->second >>= aBaseURI)
    {
        INetURLObject aURLObj(aBaseURI);
        // base URI in this case is the URI of the actual saving location
        // aURLObj.removeSegment();
        aBaseURI = aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    }

    // create an XProperty set to configure the exporter for pretty printing
    static const PropertyMapEntry aImportInfoMap[] =
    {
        { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
        { u"BuildId"_ustr, 0, ::cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { u"DefaultDocumentSettings"_ustr, 0,
          ::cppu::UnoType<Sequence<PropertyValue>>::get(), PropertyAttribute::MAYBEVOID, 0 },
    };

    Reference< XPropertySet > xInfoSet(
            GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
    xInfoSet->setPropertyValue( u"BaseURI"_ustr, Any( aBaseURI ));

    OUString aFilterName;
    auto It = aMediaMap.find(u"FilterName"_ustr);
    if (It != aMediaMap.end() && (It->second >>= aFilterName)
        && aFilterName == "OpenDocument Text Flat XML")
    {
        PropertyValue EmptyDbFieldHidesPara(u"EmptyDbFieldHidesPara"_ustr, 0, Any(false),
                                            PropertyState::PropertyState_DIRECT_VALUE);
        Sequence<PropertyValue> aSettings{ EmptyDbFieldHidesPara };
        xInfoSet->setPropertyValue(u"DefaultDocumentSettings"_ustr, Any(aSettings));
    }
    Sequence< Any > aAnys{ Any(xInfoSet) };


    // the underlying SvXMLImport implements XFastParser, XImporter, XFastDocumentHandler
    // ...except when it's one of the XMLTransformer subclasses
    Reference < XInterface > xFilter = mxContext->getServiceManager()->createInstanceWithArgumentsAndContext( sXMLImportService, aAnys, mxContext );
    assert(xFilter);
    Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    assert(xImporter);
    xImporter->setTargetDocument ( mxDoc );

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }


    // Creating a ConverterBridge instance

    Reference< XInterface > xConvBridge(
        mxContext->getServiceManager()->createInstanceWithContext(udConvertClass, mxContext), UNO_QUERY);
    if (!xConvBridge.is()) {
        SAL_WARN("filter.xmlfa", "XmlFilterAdaptor: unable to create service " << udConvertClass);
        return false;
    }
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    Reference< XImportFilter > xConverter1( xConvBridge, UNO_QUERY );
    Reference< XImportFilter2 > xConverter2( xConvBridge, UNO_QUERY );

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
                msTemplateName = aOptions.SubstituteVariable(u"$(progurl)"_ustr) + "/" + msTemplateName;
            }

            xstyleLoader->loadStylesFromURL(msTemplateName,aValue);
        }
    }

    if (xStatusIndicator.is()){
        xStatusIndicator->setValue(nSteps++);
    }

    // Calling Filtering Component

    try {
        Reference < XFastParser > xFastParser( xFilter, UNO_QUERY ); // SvXMLImport subclasses
        Reference < XDocumentHandler > xDocHandler( xFilter, UNO_QUERY ); // XMLTransformer subclasses
        assert(xFastParser || xDocHandler);
        if (xConverter2 && xFastParser)
        {
            if (!xConverter2->importer(aDescriptor,xFastParser,msUserData)) {
                if (xStatusIndicator.is())
                    xStatusIndicator->end();
                return false;
            }
        }
        else if (xConverter1 && xDocHandler)
        {
            if (!xConverter1->importer(aDescriptor,xDocHandler,msUserData)) {
                if (xStatusIndicator.is())
                    xStatusIndicator->end();
                return false;
            }
        }
        else if (xConverter1 && xFastParser)
        {
            auto pImport = static_cast<SvXMLImport*>(xFastParser.get());
            Reference<XDocumentHandler> xLegacyDocHandler = new SvXMLLegacyToFastDocHandler(pImport);
            if (!xConverter1->importer(aDescriptor,xLegacyDocHandler,msUserData)) {
                if (xStatusIndicator.is())
                    xStatusIndicator->end();
                return false;
            }
        }        
        else
        {
            SAL_WARN("filter.xmlfa", "no working combination found");
            assert(false);
            if (xStatusIndicator.is())
                xStatusIndicator->end();
            return false;
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("filter.xmlfa", "XmlFilterAdaptor");
        if (xStatusIndicator.is())
               xStatusIndicator->end();
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
        utl::MediaDescriptor::PROP_STATUSINDICATOR, Reference< XStatusIndicator >()));

    if (xStatusIndicator.is())
       xStatusIndicator->start(FilterResId(STR_FILTER_DOC_SAVING), 3);

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
        if (aMediaMap.find(u"URL"_ustr)->second >>= aBaseURI)
        {
            INetURLObject aURLObj(aBaseURI);
            // base URI in this case is the URI of the actual saving location
            // aURLObj.removeSegment();
            aBaseURI = aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        }

        // create an XProperty set to configure the exporter for pretty printing
        static const PropertyMapEntry aImportInfoMap[] =
         {
             { u"UsePrettyPrinting"_ustr, 0, cppu::UnoType<sal_Bool>::get(), PropertyAttribute::MAYBEVOID, 0},
             { u"ExportTextNumberElement"_ustr, 0, cppu::UnoType<sal_Bool>::get(), PropertyAttribute::MAYBEVOID, 0},
             { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
         };

        Reference< XPropertySet > xInfoSet(
            GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
        xInfoSet->setPropertyValue(u"UsePrettyPrinting"_ustr, Any( bPrettyPrint ));
        xInfoSet->setPropertyValue(
                        u"ExportTextNumberElement"_ustr,
                        Any( bExportTextNumberElementForListItems ));
        xInfoSet->setPropertyValue(u"BaseURI"_ustr, Any( aBaseURI ));
        Sequence < Any > aAnys{ Any(xConverter), Any(xInfoSet) };

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
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("filter.xmlfa", "XmlFilterAdaptor");
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
    if ( aArguments.hasElements() && ( aArguments[0] >>= aAnySeq ) )
    {
        comphelper::SequenceAsHashMap aMap(aAnySeq);
        msFilterName = aMap.getUnpackedValueOrDefault(
            u"Type"_ustr, OUString());
        msUserData = aMap.getUnpackedValueOrDefault(
            u"UserData"_ustr, Sequence< OUString >());
        msTemplateName = aMap.getUnpackedValueOrDefault(
            u"TemplateName"_ustr, OUString());
    }
}

// XServiceInfo
OUString SAL_CALL XmlFilterAdaptor::getImplementationName(  )
{
    return u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr;
}

sal_Bool SAL_CALL XmlFilterAdaptor::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL XmlFilterAdaptor::getSupportedServiceNames(  )
{
    return { u"com.sun.star.document.ExportFilter"_ustr, u"com.sun.star.document.ImportFilter"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_XmlFilterAdaptor_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XmlFilterAdaptor(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
