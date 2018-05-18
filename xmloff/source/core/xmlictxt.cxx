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

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <sax/fastattribs.hxx>

using namespace ::com::sun::star;

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp, sal_uInt16 nPrfx,
                              const OUString& rLName )
    : mrImport(rImp)
    , mnPrefix(nPrfx)
    , maLocalName(rLName)
{
}

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp )
    : mrImport(rImp)
    , mnPrefix(0)
{
}

SvXMLImportContext::~SvXMLImportContext()
{
}

SvXMLImportContextRef SvXMLImportContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLocalName,
                                            const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return mrImport.CreateContext( nPrefix, rLocalName, xAttrList );
}

void SvXMLImportContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& )
{
}

void SvXMLImportContext::EndElement()
{
}

void SvXMLImportContext::Characters( const OUString& )
{
}

// css::xml::sax::XFastContextHandler:
void SAL_CALL SvXMLImportContext::startFastElement(sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    mrImport.isFastContext = false;
    startUnknownElement( SvXMLImport::getNamespacePrefixFromToken(nElement, &GetImport().GetNamespaceMap()),
                         SvXMLImport::getNameFromToken( nElement ), Attribs );
}

void SAL_CALL SvXMLImportContext::startUnknownElement(const OUString & rPrefix, const OUString & rLocalName,
    const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    OUString elementName;

    if ( mrImport.maAttrList.is() )
        mrImport.maAttrList->Clear();
    else
        mrImport.maAttrList = new comphelper::AttributeList;

    mrImport.maNamespaceHandler->addNSDeclAttributes( mrImport.maAttrList );

    if ( !rPrefix.isEmpty() )
        elementName =  rPrefix + ":" + rLocalName;
    else
        elementName = rLocalName;

    if ( Attribs.is() )
    {
        sax_fastparser::FastAttributeList *pAttribList =
            sax_fastparser::FastAttributeList::castToFastAttributeList( Attribs );

        for( auto &it : *pAttribList )
        {
            sal_Int32 nToken = it.getToken();
            const OUString& rAttrNamespacePrefix = SvXMLImport::getNamespacePrefixFromToken(nToken, &GetImport().GetNamespaceMap());
            OUString sAttrName = SvXMLImport::getNameFromToken( nToken );
            if ( !rAttrNamespacePrefix.isEmpty() )
                sAttrName = rAttrNamespacePrefix + ":" + sAttrName;

            mrImport.maAttrList->AddAttribute( sAttrName, "CDATA", it.toString() );
        }

        uno::Sequence< xml::Attribute > unknownAttribs = Attribs->getUnknownAttributes();
        sal_Int32 len = unknownAttribs.getLength();
        for ( sal_Int32 i = 0; i < len; i++ )
        {
            const OUString& rAttrValue = unknownAttribs[i].Value;
            OUString sAttrName = unknownAttribs[i].Name;
            // NOTE: in *this* release branch, the NamespaceURL is *not*
            // the URL, but the *prefix*!
            const OUString& rAttrNamespacePrefix = unknownAttribs[i].NamespaceURL;
            if ( !rAttrNamespacePrefix.isEmpty() )
                sAttrName = rAttrNamespacePrefix + ":" + sAttrName;

            mrImport.maAttrList->AddAttribute( sAttrName, "CDATA", rAttrValue );
        }
    }

    mrImport.startElement( elementName, mrImport.maAttrList.get() );
}

void SAL_CALL SvXMLImportContext::endFastElement(sal_Int32 nElement)
{
    mrImport.isFastContext = false;
    endUnknownElement( SvXMLImport::getNamespacePrefixFromToken(nElement, &GetImport().GetNamespaceMap()),
                       SvXMLImport::getNameFromToken( nElement ) );
}

void SAL_CALL SvXMLImportContext::endUnknownElement (const OUString & rPrefix, const OUString & rLocalName)
{
    OUString elementName;
    if ( !rPrefix.isEmpty() )
        elementName = rPrefix + ":" + rLocalName;
    else
        elementName = rLocalName;
    mrImport.endElement( elementName );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLImportContext::createFastChildContext
    (sal_Int32 Element, const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    // Call CreateFastContext only if it's the first element of the document
    if ( !mrImport.maFastContexts.size() )
        return mrImport.CreateFastContext( Element, Attribs );
    else
        return new SvXMLImportContext( GetImport() );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLImportContext::createUnknownChildContext
    (const OUString &, const OUString &, const uno::Reference< xml::sax::XFastAttributeList > &)
{
    return this;
}

void SAL_CALL SvXMLImportContext::characters(const OUString &rChars)
{
    mrImport.Characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
