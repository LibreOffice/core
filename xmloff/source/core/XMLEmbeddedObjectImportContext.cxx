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

#include <sal/config.h>

#include <string_view>
#include <tuple>

#include <sal/log.hxx>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifiable2.hpp>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLFilterServiceNames.h>
#include <XMLEmbeddedObjectImportContext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

namespace {

class XMLEmbeddedObjectImportContext_Impl : public SvXMLImportContext
{
    css::uno::Reference< css::xml::sax::XFastDocumentHandler > mxFastHandler;

public:

    XMLEmbeddedObjectImportContext_Impl( SvXMLImport& rImport,
                                    const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& rHandler );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL startFastElement(
                        sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};

}

XMLEmbeddedObjectImportContext_Impl::XMLEmbeddedObjectImportContext_Impl(
        SvXMLImport& rImport,
        const Reference< XFastDocumentHandler >& rHandler ) :
    SvXMLImportContext( rImport ),
    mxFastHandler( rHandler )
{
    assert(mxFastHandler);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLEmbeddedObjectImportContext_Impl::createFastChildContext(
    sal_Int32 ,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    return new XMLEmbeddedObjectImportContext_Impl(GetImport(), mxFastHandler);
}

void XMLEmbeddedObjectImportContext_Impl::startFastElement(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& xAttrList )
{
    mxFastHandler->startFastElement( nElement, xAttrList );
}

void XMLEmbeddedObjectImportContext_Impl::endFastElement(sal_Int32 nElement)
{
    mxFastHandler->endFastElement( nElement );
}

void XMLEmbeddedObjectImportContext_Impl::characters( const OUString& rChars )
{
    mxFastHandler->characters( rChars );
}


void XMLEmbeddedObjectImportContext::SetComponent( Reference< XComponent > const & rComp )
{
    if( !rComp.is() || sFilterService.isEmpty() )
        return;

    Sequence<Any> aArgs( 0 );

    Reference< XComponentContext > xContext( GetImport().GetComponentContext() );

    Reference<XInterface> xFilter =
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(sFilterService, aArgs, xContext);
    SAL_WARN_IF(!xFilter, "xmloff", "could not create filter " << sFilterService);
    if( !xFilter.is() )
        return;

    assert(dynamic_cast<SvXMLImport*>(xFilter.get()));
    SvXMLImport *pFastHandler = dynamic_cast<SvXMLImport*>(xFilter.get());
    mxFastHandler = pFastHandler;

    try
    {
        Reference < XModifiable2 > xModifiable2( rComp, UNO_QUERY_THROW );
        xModifiable2->disableSetModified();
    }
    catch( Exception& )
    {
    }

    Reference < XImporter > xImporter( mxFastHandler, UNO_QUERY );
    xImporter->setTargetDocument( rComp );

    xComp = rComp;  // keep ref to component only if there is a handler

    // #i34042: copy namespace declarations
    // We created a new instance of XMLImport, so we need to propagate the namespace
    // declarations to it.
    pFastHandler->GetNamespaceMap() = GetImport().GetNamespaceMap();
}

XMLEmbeddedObjectImportContext::XMLEmbeddedObjectImportContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const Reference< XFastAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport )
{
    SvGlobalName aName;

    if( nElement == XML_ELEMENT(MATH, XML_MATH) )
    {
        sFilterService = XML_IMPORT_FILTER_MATH;
        aName = SvGlobalName(SO3_SM_CLASSID);
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_DOCUMENT) )
    {
        OUString sMime;

        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(OFFICE, XML_MIMETYPE):
                    sMime = aIter.toString();
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("xmloff", aIter);
            }
        }

        std::u16string_view sClass;
        static std::u16string_view const prefixes[] = {
            u"application/vnd.oasis.openoffice.",
            u"application/x-vnd.oasis.openoffice.",
            u"application/vnd.oasis.opendocument.",
            u"application/x-vnd.oasis.opendocument."};
        for (auto const & p: prefixes)
        {
            if (o3tl::starts_with(sMime, p, &sClass))
            {
                break;
            }
        }

        if( !sClass.empty() )
        {
            static const std::tuple<XMLTokenEnum, OUString, SvGUID> aServiceMap[] = {
                { XML_TEXT, XML_IMPORT_FILTER_WRITER, { SO3_SW_CLASSID } },
                { XML_ONLINE_TEXT, XML_IMPORT_FILTER_WRITER, { SO3_SWWEB_CLASSID } },
                { XML_SPREADSHEET, XML_IMPORT_FILTER_CALC, { SO3_SC_CLASSID } },
                { XML_DRAWING, XML_IMPORT_FILTER_DRAW, { SO3_SDRAW_CLASSID } },
                { XML_GRAPHICS, XML_IMPORT_FILTER_DRAW, { SO3_SDRAW_CLASSID } },
                { XML_PRESENTATION, XML_IMPORT_FILTER_IMPRESS, { SO3_SIMPRESS_CLASSID } },
                { XML_CHART, XML_IMPORT_FILTER_CHART, { SO3_SCH_CLASSID } },
            };
            for (auto const& [eClass, sMatchingFilterService, rCLASSID] : aServiceMap)
            {
                if (IsXMLToken(sClass, eClass))
                {
                    sFilterService = sMatchingFilterService;
                    aName = SvGlobalName(rCLASSID);
                    break;
                }
            }
        }
    }

    sCLSID = aName.GetHexName();
}

XMLEmbeddedObjectImportContext::~XMLEmbeddedObjectImportContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLEmbeddedObjectImportContext::createFastChildContext(
    sal_Int32 ,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& )
{
    if( mxFastHandler.is() )
        return new XMLEmbeddedObjectImportContext_Impl( GetImport(), mxFastHandler );
    return nullptr;
}

void XMLEmbeddedObjectImportContext::startFastElement(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& rAttrList )
{
    if( !mxFastHandler.is() )
        return;

    mxFastHandler->startDocument();
    mxFastHandler->startFastElement( nElement, rAttrList );
}

void XMLEmbeddedObjectImportContext::endFastElement(sal_Int32 nElement)
{
    if( !mxFastHandler.is() )
        return;

    mxFastHandler->endFastElement( nElement );
    mxFastHandler->endDocument();

    try
    {
        Reference < XModifiable2 > xModifiable2( xComp, UNO_QUERY_THROW );
        xModifiable2->enableSetModified();
        xModifiable2->setModified( true ); // trigger new replacement image generation
    }
    catch( Exception& )
    {
    }
}

void XMLEmbeddedObjectImportContext::characters( const OUString& rChars )
{
    if( mxFastHandler.is() )
        mxFastHandler->characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
