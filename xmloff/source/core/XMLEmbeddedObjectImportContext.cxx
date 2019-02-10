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

#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifiable2.hpp>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/XMLFilterServiceNames.h>
#include <XMLEmbeddedObjectImportContext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLEmbeddedObjectImportContext_Impl : public SvXMLImportContext
{
    css::uno::Reference< css::xml::sax::XDocumentHandler > xHandler;

public:

    XMLEmbeddedObjectImportContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
    const css::uno::Reference< css::xml::sax::XDocumentHandler >& rHandler );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void EndElement() override;

    virtual void Characters( const OUString& rChars ) override;
};


XMLEmbeddedObjectImportContext_Impl::XMLEmbeddedObjectImportContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< XDocumentHandler >& rHandler ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xHandler( rHandler )
{
}

SvXMLImportContextRef XMLEmbeddedObjectImportContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList >& )
{
    return new XMLEmbeddedObjectImportContext_Impl( GetImport(),
                                                    nPrefix, rLocalName,
                                                    xHandler );
}

void XMLEmbeddedObjectImportContext_Impl::StartElement(
        const Reference< XAttributeList >& xAttrList )
{
    xHandler->startElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                GetPrefix(), GetLocalName() ),
                            xAttrList );
}

void XMLEmbeddedObjectImportContext_Impl::EndElement()
{
    xHandler->endElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                GetPrefix(), GetLocalName() ) );
}

void XMLEmbeddedObjectImportContext_Impl::Characters( const OUString& rChars )
{
    xHandler->characters( rChars );
}


void XMLEmbeddedObjectImportContext::SetComponent( Reference< XComponent > const & rComp )
{
    if( !rComp.is() || sFilterService.isEmpty() )
        return;

    Sequence<Any> aArgs( 0 );

    Reference< XComponentContext > xContext( GetImport().GetComponentContext() );

    xHandler.set(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(sFilterService, aArgs, xContext),
        UNO_QUERY);

    if( !xHandler.is() )
        return;

    if (SvXMLImport *pFastHandler = dynamic_cast<SvXMLImport*>(xHandler.get()))
        xHandler.set( new SvXMLLegacyToFastDocHandler( pFastHandler ) );

    try
    {
        Reference < XModifiable2 > xModifiable2( rComp, UNO_QUERY_THROW );
        xModifiable2->disableSetModified();
    }
    catch( Exception& )
    {
    }

    Reference < XImporter > xImporter( xHandler, UNO_QUERY );
    xImporter->setTargetDocument( rComp );

    xComp = rComp;  // keep ref to component only if there is a handler
}

XMLEmbeddedObjectImportContext::XMLEmbeddedObjectImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    SvGlobalName aName;

    if( nPrfx == XML_NAMESPACE_MATH &&
        IsXMLToken( rLName, XML_MATH ) )
    {
        sFilterService = XML_IMPORT_FILTER_MATH;
        aName = SvGlobalName(SO3_SM_CLASSID);
    }
    else if( nPrfx == XML_NAMESPACE_OFFICE &&
        IsXMLToken( rLName, XML_DOCUMENT ) )
    {
        OUString sMime;

        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
            if( nPrefix == XML_NAMESPACE_OFFICE &&
                IsXMLToken( aLocalName, XML_MIMETYPE ) )
            {
                sMime = xAttrList->getValueByIndex( i );
                break;
            }
        }

        OUString sClass;
        static OUStringLiteral const prefixes[] = {
            "application/vnd.oasis.openoffice.",
            "application/x-vnd.oasis.openoffice.",
            "application/vnd.oasis.opendocument.",
            "application/x-vnd.oasis.opendocument."};
        for (auto const & p: prefixes)
        {
            if (sMime.startsWith(p, &sClass))
            {
                break;
            }
        }

        if( !sClass.isEmpty() )
        {
            static struct { XMLTokenEnum eClass; OUStringLiteral sFilterService;
            } const aServiceMap[] = {
                { XML_TEXT,         OUStringLiteral(XML_IMPORT_FILTER_WRITER) },
                { XML_ONLINE_TEXT,  OUStringLiteral(XML_IMPORT_FILTER_WRITER) },
                { XML_SPREADSHEET,  OUStringLiteral(XML_IMPORT_FILTER_CALC) },
                { XML_DRAWING,      OUStringLiteral(XML_IMPORT_FILTER_DRAW) },
                { XML_GRAPHICS,     OUStringLiteral(XML_IMPORT_FILTER_DRAW) },
                { XML_PRESENTATION, OUStringLiteral(XML_IMPORT_FILTER_IMPRESS) },
                { XML_CHART,        OUStringLiteral(XML_IMPORT_FILTER_CHART) }};
            for (auto const & entry: aServiceMap)
            {
                if( IsXMLToken( sClass, entry.eClass ) )
                {
                    sFilterService = entry.sFilterService;

                    switch( entry.eClass )
                    {
                    case XML_TEXT:          aName = SvGlobalName(SO3_SW_CLASSID); break;
                    case XML_ONLINE_TEXT:   aName = SvGlobalName(SO3_SWWEB_CLASSID); break;
                    case XML_SPREADSHEET:   aName = SvGlobalName(SO3_SC_CLASSID); break;
                    case XML_DRAWING:
                    case XML_GRAPHICS:
                    case XML_IMAGE:     aName = SvGlobalName(SO3_SDRAW_CLASSID); break;
                    case XML_PRESENTATION:  aName = SvGlobalName(SO3_SIMPRESS_CLASSID); break;
                    case XML_CHART:         aName = SvGlobalName(SO3_SCH_CLASSID); break;
                    default:
                        break;
                    }

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

SvXMLImportContextRef XMLEmbeddedObjectImportContext::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< XAttributeList >& )
{
    if( xHandler.is() )
        return new XMLEmbeddedObjectImportContext_Impl( GetImport(),
                                                        nPrefix, rLocalName,
                                                        xHandler );
    else
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void XMLEmbeddedObjectImportContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    if( xHandler.is() )
    {
        xHandler->startDocument();
        // #i34042: copy namepspace declarations
        SvXMLAttributeList *pAttrList = new SvXMLAttributeList( rAttrList );
        Reference< XAttributeList > xAttrList( pAttrList );
        const SvXMLNamespaceMap& rNamespaceMap = GetImport().GetNamespaceMap();
        sal_uInt16 nPos = rNamespaceMap.GetFirstKey();
        while( USHRT_MAX != nPos )
        {
            OUString aAttrName( rNamespaceMap.GetAttrNameByKey( nPos ) );
            if( xAttrList->getValueByName( aAttrName ).isEmpty() )
            {
                pAttrList->AddAttribute( aAttrName,
                                          rNamespaceMap.GetNameByKey( nPos ) );
            }
            nPos = rNamespaceMap.GetNextKey( nPos );
        }
        xHandler->startElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                    GetPrefix(), GetLocalName() ),
                                xAttrList );
    }
}

void XMLEmbeddedObjectImportContext::EndElement()
{
    if( xHandler.is() )
    {
        xHandler->endElement( GetImport().GetNamespaceMap().GetQNameByKey(
                                    GetPrefix(), GetLocalName() ) );
        xHandler->endDocument();

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
}

void XMLEmbeddedObjectImportContext::Characters( const OUString& rChars )
{
    if( xHandler.is() )
        xHandler->characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
