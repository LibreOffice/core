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

void FastSerializerHelper::startElement(sal_Int32 elementTokenId)
{
    mpSerializer->startFastElement(elementTokenId);
}
void FastSerializerHelper::pushAttributeValue(sal_Int32 attribute, const char* value)
{
    mpSerializer->getTokenValueList().emplace_back(attribute, value);
}
void FastSerializerHelper::pushAttributeValue(sal_Int32 attribute, const OString& value)
{
    mpSerializer->getTokenValueList().emplace_back(attribute, value.getStr());
}
void FastSerializerHelper::singleElement(sal_Int32 elementTokenId)
{
    mpSerializer->singleFastElement(elementTokenId);
}

void FastSerializerHelper::endElement(sal_Int32 elementTokenId)
{
    mpSerializer->endFastElement(elementTokenId);
}

void FastSerializerHelper::startElement(sal_Int32 elementTokenId, const rtl::Reference<FastAttributeList>& xAttrList)
{
    assert(xAttrList);
    mpSerializer->startFastElement(elementTokenId, xAttrList.get());
}

void FastSerializerHelper::singleElement(sal_Int32 elementTokenId, const rtl::Reference<FastAttributeList>& xAttrList)
{
    assert(xAttrList);
    mpSerializer->singleFastElement(elementTokenId, xAttrList.get());
}

FastSerializerHelper* FastSerializerHelper::write(const char* value)
{
    mpSerializer->write(value, -1);
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(const OString& value)
{
    mpSerializer->write(value);
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(std::u16string_view value)
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

FastSerializerHelper* FastSerializerHelper::writeEscaped(std::u16string_view value)
{
    if (!value.empty())
        mpSerializer->write(value, true);
    return this;
}

FastSerializerHelper* FastSerializerHelper::writeId(sal_Int32 tokenId)
{
    mpSerializer->writeId(tokenId);
    return this;
}

css::uno::Reference< css::io::XOutputStream > const & FastSerializerHelper::getOutputStream() const
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

rtl::Reference<FastAttributeList> FastSerializerHelper::createAttrList()
{
    return new FastAttributeList( nullptr );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
