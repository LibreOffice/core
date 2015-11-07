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
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>

using namespace ::com::sun::star;

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp, sal_uInt16 nPrfx,
                              const OUString& rLName ) :
    mrImport( rImp ),
    mnPrefix( nPrfx ),
    maLocalName( rLName ),
    mpRewindMap( 0 )
{
}

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp ) :
    mrImport( rImp ),
    mnPrefix ( 0 ),
    mpRewindMap( 0 )
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

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL SvXMLImportContext::startFastElement(sal_Int32, const uno::Reference< xml::sax::XFastAttributeList > &)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SAL_CALL SvXMLImportContext::startUnknownElement(const OUString &, const OUString &,
    const uno::Reference< xml::sax::XFastAttributeList > &)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SAL_CALL SvXMLImportContext::endFastElement(sal_Int32)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SAL_CALL SvXMLImportContext::endUnknownElement (const OUString & , const OUString & )
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
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
    return 0;
}

void SAL_CALL SvXMLImportContext::characters(const OUString &)
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
}

void SvXMLImportContext::onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
