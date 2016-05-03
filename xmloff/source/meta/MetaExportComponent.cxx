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

#include "MetaExportComponent.hxx"
#include "facreg.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetae.hxx>
#include "PropertySetMerger.hxx"

#include <unotools/docinfohelper.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLMetaExportComponent::XMLMetaExportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
    OUString const & implementationName, SvXMLExportFlags nFlags )
:   SvXMLExport( util::MeasureUnit::CM, xContext, implementationName, XML_TEXT, nFlags )
{
}

XMLMetaExportComponent::~XMLMetaExportComponent()
{
}

void SAL_CALL XMLMetaExportComponent::setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    try
    {
        SvXMLExport::setSourceDocument( xDoc );
    }
    catch( lang::IllegalArgumentException& )
    {
        // allow to use document properties service without model access
        // this is required for document properties exporter
        mxDocProps =
            uno::Reference< document::XDocumentProperties >::query( xDoc );
        if( !mxDocProps.is() )
            throw lang::IllegalArgumentException();
    }
}

sal_uInt32 XMLMetaExportComponent::exportDoc( enum XMLTokenEnum )
{
    uno::Reference< xml::sax::XDocumentHandler > xDocHandler = GetDocHandler();

    if( !(getExportFlags() & SvXMLExportFlags::OASIS) )
    {
        uno::Reference< uno::XComponentContext > xContext = getComponentContext();
        try
        {
            ::comphelper::PropertyMapEntry const aInfoMap[] =
            {
                { OUString("Class"), 0,
                    ::cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::MAYBEVOID, 0},
                { OUString(), 0, css::uno::Type(), 0, 0 }
            };
            uno::Reference< beans::XPropertySet > xConvPropSet(
                ::comphelper::GenericPropertySet_CreateInstance(
                        new ::comphelper::PropertySetInfo( aInfoMap ) ) );

            xConvPropSet->setPropertyValue("Class", uno::Any(GetXMLToken( XML_TEXT )) );

            uno::Reference< beans::XPropertySet > xPropSet =
                getExportInfo().is()
                    ?  PropertySetMerger_CreateInstance( getExportInfo(),
                                                      xConvPropSet )
                    : getExportInfo();

            uno::Sequence< uno::Any > aArgs( 3 );
            aArgs[0] <<= xDocHandler;
            aArgs[1] <<= xPropSet;
            aArgs[2] <<= GetModel();

            // get filter component
            xDocHandler.set(
                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    "com.sun.star.comp.Oasis2OOoTransformer", aArgs, xContext),
                uno::UNO_QUERY_THROW );

            SetDocHandler( xDocHandler );
        }
        catch( com::sun::star::uno::Exception& )
        {
            OSL_FAIL( "Cannot instantiate com.sun.star.comp.Oasis2OOoTransformer!\n");
        }
    }


    xDocHandler->startDocument();

    addChaffWhenEncryptedStorage();

    {

        const SvXMLNamespaceMap& rMap = GetNamespaceMap();
        sal_uInt16 nPos = rMap.GetFirstKey();
        while( USHRT_MAX != nPos )
        {
            GetAttrList().AddAttribute( rMap.GetAttrNameByKey( nPos ), rMap.GetNameByKey( nPos ) );
            nPos = GetNamespaceMap().GetNextKey( nPos );
        }

        const sal_Char* pVersion = nullptr;
        switch( getDefaultVersion() )
        {
        case SvtSaveOptions::ODFVER_LATEST: pVersion = "1.2"; break;
        case SvtSaveOptions::ODFVER_012_EXT_COMPAT: pVersion = "1.2"; break;
        case SvtSaveOptions::ODFVER_012: pVersion = "1.2"; break;
        case SvtSaveOptions::ODFVER_011: pVersion = "1.1"; break;
        case SvtSaveOptions::ODFVER_010: break;

        default:
            OSL_FAIL("xmloff::XMLMetaExportComponent::exportDoc(), unexpected odf default version!");
        }

        if( pVersion )
            AddAttribute( XML_NAMESPACE_OFFICE, XML_VERSION,
                            OUString::createFromAscii(pVersion) );

        SvXMLElementExport aDocElem( *this, XML_NAMESPACE_OFFICE, XML_DOCUMENT_META,
                    true, true );

        // NB: office:meta is now written by _ExportMeta
        _ExportMeta();
    }
    xDocHandler->endDocument();
    return 0;
}

void XMLMetaExportComponent::_ExportMeta()
{
    if (mxDocProps.is()) {
        OUString generator( ::utl::DocInfoHelper::GetGeneratorString() );
        // update generator here
        mxDocProps->setGenerator(generator);
        SvXMLMetaExport * pMeta = new SvXMLMetaExport(*this, mxDocProps);
        uno::Reference<xml::sax::XDocumentHandler> xMeta(pMeta);
        pMeta->Export();
    } else {
        SvXMLExport::_ExportMeta();
    }
}

// methods without content:
void XMLMetaExportComponent::_ExportAutoStyles() {}
void XMLMetaExportComponent::_ExportMasterStyles() {}
void XMLMetaExportComponent::_ExportContent() {}


uno::Sequence< OUString > SAL_CALL XMLMetaExportComponent_getSupportedServiceNames()
    throw()
{
    return uno::Sequence< OUString > { "com.sun.star.document.XMLOasisMetaExporter" };
}

OUString SAL_CALL XMLMetaExportComponent_getImplementationName() throw()
{
    return OUString( "XMLMetaExportComponent" );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaExportComponent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return static_cast<cppu::OWeakObject*>(new XMLMetaExportComponent( comphelper::getComponentContext(rSMgr), XMLMetaExportComponent_getImplementationName(), SvXMLExportFlags::META|SvXMLExportFlags::OASIS));
}

uno::Sequence< OUString > SAL_CALL XMLMetaExportOOO_getSupportedServiceNames()
    throw()
{
    return uno::Sequence< OUString > { "com.sun.star.document.XMLMetaExporter" };
}

OUString SAL_CALL XMLMetaExportOOO_getImplementationName() throw()
{
    return OUString( "XMLMetaExportOOo" );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaExportOOO_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return static_cast<cppu::OWeakObject*>(new XMLMetaExportComponent( comphelper::getComponentContext(rSMgr), XMLMetaExportOOO_getImplementationName(), SvXMLExportFlags::META));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
