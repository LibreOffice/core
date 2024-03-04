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

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace ::com::sun::star;

SvXMLImportContext::SvXMLImportContext( SvXMLImport& rImp )
    : mrImport(rImp)
    , m_nRefCount(0)
{
}

SvXMLImportContext::~SvXMLImportContext()
{
}

void SvXMLImportContext::endFastElement(sal_Int32 )
{
}

// css::xml::sax::XFastContextHandler:
void SAL_CALL SvXMLImportContext::startFastElement(sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList > & )
{
}

void SAL_CALL SvXMLImportContext::startUnknownElement(const OUString & /*rNamespace*/, const OUString & /*rElementName*/,
    const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/)
{
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
