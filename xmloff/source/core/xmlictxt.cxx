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

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <sax/fastattribs.hxx>
#include <comphelper/attributelist.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp, sal_uInt16 nPrfx,
                              const OUString& rLName )
    : mrImport(rImp)
    , maLocalName(rLName)
    , m_nRefCount(0)
    , mnPrefix(nPrfx)
    , mbPrefixAndLocalNameFilledIn(true)
{
}

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp )
    : mrImport(rImp)
    , m_nRefCount(0)
    , mnPrefix(0)
    , mbPrefixAndLocalNameFilledIn(false)
{
}

SvXMLImportContext::~SvXMLImportContext()
{
}

SvXMLImportContextRef SvXMLImportContext::CreateChildContext( sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const uno::Reference<xml::sax::XAttributeList>& )
{
    return nullptr;
}

void SvXMLImportContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& )
{
}

void SvXMLImportContext::endFastElement(sal_Int32 )
{
}

// css::xml::sax::XFastContextHandler:
void SAL_CALL SvXMLImportContext::startFastElement(sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    const OUString& rPrefix = SvXMLImport::getNamespacePrefixFromToken(nElement, &GetImport().GetNamespaceMap());
    const OUString& rLocalName = SvXMLImport::getNameFromToken( nElement );
    startUnknownElement( SvXMLImport::aDefaultNamespace, (rPrefix.isEmpty())? rLocalName : rPrefix + SvXMLImport::aNamespaceSeparator + rLocalName, Attribs );
}

void SAL_CALL SvXMLImportContext::startUnknownElement(const OUString & /*rNamespace*/, const OUString & /*rElementName*/,
    const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
{
    mrImport.maAttrList->Clear();
    mrImport.maNamespaceHandler->addNSDeclAttributes( mrImport.maAttrList );

    if ( Attribs.is() )
    {
        for( auto &it : sax_fastparser::castToFastAttributeList( Attribs ) )
        {
            sal_Int32 nToken = it.getToken();
            const OUString& rAttrNamespacePrefix = SvXMLImport::getNamespacePrefixFromToken(nToken, &GetImport().GetNamespaceMap());
            OUString sAttrName = SvXMLImport::getNameFromToken( nToken );
            if ( !rAttrNamespacePrefix.isEmpty() )
                sAttrName = rAttrNamespacePrefix + SvXMLImport::aNamespaceSeparator + sAttrName;

            mrImport.maAttrList->AddAttribute( sAttrName, "CDATA", it.toString() );
        }

        const uno::Sequence< xml::Attribute > unknownAttribs = Attribs->getUnknownAttributes();
        for ( const auto& rUnknownAttrib : unknownAttribs )
        {
            const OUString& rAttrValue = rUnknownAttrib.Value;
            const OUString& rAttrName = rUnknownAttrib.Name;
            // note: rAttrName is expected to be namespace-prefixed here
            mrImport.maAttrList->AddAttribute( rAttrName, "CDATA", rAttrValue );
        }
    }
    StartElement( mrImport.maAttrList.get() );
}

void SAL_CALL SvXMLImportContext::endUnknownElement (const OUString & /*rNamespace*/, const OUString & /*rElementName*/)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLImportContext::createFastChildContext
    (sal_Int32 /*Element*/, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    return nullptr;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLImportContext::createUnknownChildContext
    (const OUString & /*rNamespace*/, const OUString & /*rName*/, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
    return nullptr;
}

void SAL_CALL SvXMLImportContext::characters(const OUString &/*rChars*/)
{
}

// XInterface
css::uno::Any SAL_CALL SvXMLImportContext::queryInterface( const css::uno::Type& aType )
{
    css::uno::Any a = ::cppu::queryInterface(
                aType,
                static_cast< XFastContextHandler* >(this),
                static_cast< XTypeProvider* >(this),
                static_cast< css::uno::XInterface* >(static_cast< XFastContextHandler* >(this)));

    return a;
}

// XTypeProvider
css::uno::Sequence< css::uno::Type > SAL_CALL SvXMLImportContext::getTypes()
{
    return { cppu::UnoType<XFastContextHandler>::get(),
             cppu::UnoType<XTypeProvider>::get() };
}

css::uno::Sequence< sal_Int8 > SAL_CALL SvXMLImportContext::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
