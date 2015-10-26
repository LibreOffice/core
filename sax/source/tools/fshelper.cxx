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

#include <sax/fshelper.hxx>
#include "fastserializer.hxx"
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sax_fastparser {

FastSerializerHelper::FastSerializerHelper(const Reference< io::XOutputStream >& xOutputStream, bool bWriteHeader ) :
    mpSerializer(new FastSaxSerializer(xOutputStream))
{
    if( bWriteHeader )
        mpSerializer->startDocument();
}

FastSerializerHelper::~FastSerializerHelper()
{
    mpSerializer->endDocument();
    delete mpSerializer;
}

void FastSerializerHelper::startElementInternal(sal_Int32 elementTokenId, ...)
{
    va_list args;
    va_start( args, elementTokenId );
    TokenValueList& rAttrList = mpSerializer->getTokenValueList();

    while (true)
    {
        sal_Int32 nName = va_arg(args, sal_Int32);
        if (nName == FSEND_internal)
            break;
        const char* pValue = va_arg(args, const char*);
        if (pValue)
            rAttrList.push_back(TokenValue(nName, pValue));
    }

    mpSerializer->startFastElement(elementTokenId);
    va_end( args );
}

void FastSerializerHelper::singleElementInternal(sal_Int32 elementTokenId, ...)
{
    va_list args;
    va_start( args, elementTokenId );
    TokenValueList& rAttrList = mpSerializer->getTokenValueList();

    while (true)
    {
        sal_Int32 nName = va_arg(args, sal_Int32);
        if (nName == FSEND_internal)
            break;
        const char* pValue = va_arg(args, const char*);
        if  (pValue)
            rAttrList.push_back(TokenValue(nName, pValue));
    }

    mpSerializer->singleFastElement(elementTokenId);
    va_end( args );
}

void FastSerializerHelper::endElement(sal_Int32 elementTokenId)
{
    mpSerializer->endFastElement(elementTokenId);
}

void FastSerializerHelper::startElement(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
{
    FastAttributeList* pAttrList = dynamic_cast< FastAttributeList* >(xAttrList.get());
    assert(pAttrList);
    mpSerializer->startFastElement(elementTokenId, pAttrList);
}

void FastSerializerHelper::singleElement(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
{
    FastAttributeList* pAttrList = dynamic_cast< FastAttributeList* >(xAttrList.get());
    assert(pAttrList);
    mpSerializer->singleFastElement(elementTokenId, pAttrList);
}

FastSerializerHelper* FastSerializerHelper::write(const char* value)
{
    mpSerializer->write(value, -1);
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(const OUString& value)
{
    mpSerializer->write(value);
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(sal_Int32 value)
{
    mpSerializer->write(OString::number(value));
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(sal_Int64 value)
{
    mpSerializer->write(OString::number(value));
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(double value)
{
    mpSerializer->write(value);
    return this;
}

FastSerializerHelper* FastSerializerHelper::writeEscaped(const char* value)
{
    mpSerializer->write(value, -1, true);
    return this;
}

FastSerializerHelper* FastSerializerHelper::writeEscaped(const OUString& value)
{
    if (!value.isEmpty())
        mpSerializer->write(value, true);
    return this;
}

FastSerializerHelper* FastSerializerHelper::writeId(sal_Int32 tokenId)
{
    mpSerializer->writeId(tokenId);
    return this;
}

css::uno::Reference< css::io::XOutputStream > FastSerializerHelper::getOutputStream()
{
    return mpSerializer->getOutputStream();
}

void FastSerializerHelper::mark(
        sal_Int32 const nTag, const Sequence<sal_Int32>& rOrder)
{
    mpSerializer->mark(nTag, rOrder);
}

void FastSerializerHelper::mergeTopMarks(
        sal_Int32 const nTag, MergeMarks const eMergeType)
{
    mpSerializer->mergeTopMarks(nTag, eMergeType);
}

FastAttributeList * FastSerializerHelper::createAttrList()
{
    return new FastAttributeList( Reference< xml::sax::XFastTokenHandler >() );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
