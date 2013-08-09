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
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifiable2.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlerror.hxx"
#include <xmloff/attrlist.hxx>
#include "xmloff/XMLFilterServiceNames.h"
#include "XMLEmbeddedObjectImportContext.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

struct XMLServiceMapEntry_Impl
{
    enum XMLTokenEnum eClass;
    const sal_Char *sFilterService;
    sal_Int32      nFilterServiceLen;
};

#define SERVICE_MAP_ENTRY( cls, app ) \
    { XML_##cls, \
      XML_IMPORT_FILTER_##app, sizeof(XML_IMPORT_FILTER_##app)-1}

const XMLServiceMapEntry_Impl aServiceMap[] =
{
    SERVICE_MAP_ENTRY( TEXT, WRITER ),
    SERVICE_MAP_ENTRY( ONLINE_TEXT, WRITER ),
    SERVICE_MAP_ENTRY( SPREADSHEET, CALC ),
    SERVICE_MAP_ENTRY( DRAWING, DRAW ),
    SERVICE_MAP_ENTRY( GRAPHICS, DRAW ),
    SERVICE_MAP_ENTRY( PRESENTATION, IMPRESS ),
    SERVICE_MAP_ENTRY( CHART, CHART ),
    { XML_TOKEN_INVALID, 0, 0 }
};

class XMLEmbeddedObjectImportContext_Impl : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler > xHandler;

public:
    TYPEINFO();

    XMLEmbeddedObjectImportContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler >& rHandler );

    virtual ~XMLEmbeddedObjectImportContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLEmbeddedObjectImportContext_Impl, SvXMLImportContext );

XMLEmbeddedObjectImportContext_Impl::XMLEmbeddedObjectImportContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< XDocumentHandler >& rHandler ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xHandler( rHandler )
{
}

XMLEmbeddedObjectImportContext_Impl::~XMLEmbeddedObjectImportContext_Impl()
{
}

SvXMLImportContext *XMLEmbeddedObjectImportContext_Impl::CreateChildContext(
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

TYPEINIT1( XMLEmbeddedObjectImportContext, SvXMLImportContext );

sal_Bool XMLEmbeddedObjectImportContext::SetComponent(
        Reference< XComponent >& rComp )
{
    if( !rComp.is() || sFilterService.isEmpty() )
        return sal_False;

    Sequence<Any> aArgs( 0 );

    Reference< XComponentContext > xContext( GetImport().GetComponentContext() );

    xHandler = Reference < XDocumentHandler >(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(sFilterService, aArgs, xContext),
        UNO_QUERY);

    if( !xHandler.is() )
        return sal_False;

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

    return sal_True;
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
        sFilterService = OUString( XML_IMPORT_FILTER_MATH );
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
        static const char * aTmp[] =
        {
            "application/vnd.oasis.openoffice.",
            "application/x-vnd.oasis.openoffice.",
            "application/vnd.oasis.opendocument.",
            "application/x-vnd.oasis.opendocument.",
            NULL
        };
        for (int k=0; aTmp[k]; k++)
        {
            OUString sTmpString = OUString::createFromAscii(aTmp[k]);
            if( sMime.matchAsciiL( aTmp[k], sTmpString.getLength() ) )
            {
                sClass = sMime.copy( sTmpString.getLength() );
                break;
            }
        }

        if( !sClass.isEmpty() )
        {
            const XMLServiceMapEntry_Impl *pEntry = aServiceMap;
            while( pEntry->eClass != XML_TOKEN_INVALID )
            {
                if( IsXMLToken( sClass, pEntry->eClass ) )
                {
                    sFilterService = OUString( pEntry->sFilterService,
                                               pEntry->nFilterServiceLen,
                                               RTL_TEXTENCODING_ASCII_US );

                    switch( pEntry->eClass )
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
                pEntry++;
            }
        }
    }

    sCLSID = aName.GetHexName();
}

XMLEmbeddedObjectImportContext::~XMLEmbeddedObjectImportContext()
{
}

SvXMLImportContext *XMLEmbeddedObjectImportContext::CreateChildContext(
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
        xModifiable2->setModified( sal_True ); // trigger new replacement image generation
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
