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

SvXMLImportContext *SvXMLImportContext::CreateChildContext( sal_uInt16 nPrefix,
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
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    mrImport.isFastContext = false;
    startUnknownElement( mrImport.getNamespacePrefixFromToken( nElement ),
                         mrImport.getNameFromToken( nElement ), Attribs );
}

void SAL_CALL SvXMLImportContext::startUnknownElement(const OUString & rPrefix, const OUString & rLocalName,
    const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    OUString elementName;
    rtl::Reference < comphelper::AttributeList > rAttrList = new comphelper::AttributeList;
    mrImport.maNamespaceHandler->addNSDeclAttributes( rAttrList );
    if ( !rPrefix.isEmpty() )
        elementName =  rPrefix + ":" + rLocalName;
    else
        elementName = rLocalName;

    if ( Attribs.is() )
    {
        sax_fastparser::FastAttributeList *pAttribList;
        assert( dynamic_cast< sax_fastparser::FastAttributeList *>( Attribs.get() ) != nullptr );
        pAttribList = static_cast< sax_fastparser::FastAttributeList *>( Attribs.get() );

        const std::vector< sal_Int32 >& rAttrTokenList = pAttribList->getFastAttributeTokens();
        for ( size_t i = 0; i < rAttrTokenList.size(); i++ )
        {
            const OUString& rAttrValue = OUString(pAttribList->getFastAttributeValue(i),
                                            pAttribList->AttributeValueLength(i), RTL_TEXTENCODING_UTF8);
            sal_Int32 nToken = rAttrTokenList[ i ];
            const OUString& rAttrNamespacePrefix = mrImport.getNamespacePrefixFromToken( nToken );
            OUString sAttrName = mrImport.getNameFromToken( nToken );
            if ( !rAttrNamespacePrefix.isEmpty() )
                sAttrName = rAttrNamespacePrefix + ":" + sAttrName;

            rAttrList->AddAttribute( sAttrName, "CDATA", rAttrValue );
        }

        uno::Sequence< xml::Attribute > unknownAttribs = Attribs->getUnknownAttributes();
        sal_Int32 len = unknownAttribs.getLength();
        for ( sal_Int32 i = 0; i < len; i++ )
        {
            const OUString& rAttrValue = unknownAttribs[i].Value;
            OUString sAttrName = unknownAttribs[i].Name;
            const OUString& rAttrNamespacePrefix = unknownAttribs[i].NamespaceURL;
            if ( !rAttrNamespacePrefix.isEmpty() )
                sAttrName = rAttrNamespacePrefix + ":" + sAttrName;

            rAttrList->AddAttribute( sAttrName, "CDATA", rAttrValue );
        }
    }

    mrImport.startElement( elementName, rAttrList.get() );
}

void SAL_CALL SvXMLImportContext::endFastElement(sal_Int32 nElement)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    mrImport.isFastContext = false;
    endUnknownElement( mrImport.getNamespacePrefixFromToken( nElement ),
                       mrImport.getNameFromToken( nElement ) );
}

void SAL_CALL SvXMLImportContext::endUnknownElement (const OUString & rPrefix, const OUString & rLocalName)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
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
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    return mrImport.CreateFastContext( Element, Attribs );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLImportContext::createUnknownChildContext
    (const OUString &, const OUString &, const uno::Reference< xml::sax::XFastAttributeList > &)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    return this;
}

void SAL_CALL SvXMLImportContext::characters(const OUString &rChars)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    mrImport.Characters( rChars );
}

void SvXMLImportContext::onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& )
{
}

void SvXMLImportContext::AddFirstRef()
{
    acquire();
}

void SvXMLImportContext::AddNextRef()
{
    acquire();
}

void SvXMLImportContext::ReleaseRef()
{
    release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
