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
#include <fastserializer.hxx>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sax_fastparser {

FastSerializerHelper::FastSerializerHelper(const Reference< io::XOutputStream >& xOutputStream, bool bWriteHeader ) :
    mpSerializer(new FastSaxSerializer())
{
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext(), UNO_SET_THROW );
    Reference< lang::XMultiComponentFactory > xFactory( xContext->getServiceManager(), UNO_SET_THROW );
    mxTokenHandler.set( xFactory->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.FastTokenHandler") ), xContext ), UNO_QUERY_THROW );

    mpSerializer->setFastTokenHandler( mxTokenHandler );
    mpSerializer->setOutputStream( xOutputStream );
    if( bWriteHeader )
        mpSerializer->startDocument();
}

FastSerializerHelper::~FastSerializerHelper()
{
    mpSerializer->endDocument();
    delete mpSerializer;
}

void FastSerializerHelper::startElementV(sal_Int32 elementTokenId, va_list args)
{
    FastAttributeList* pAttrList = new FastAttributeList( mxTokenHandler );

    while (true)
    {
        sal_Int32 nName = va_arg(args, sal_Int32);
        if (nName == FSEND)
            break;
        const char* pValue = va_arg(args, const char*);
        if (pValue)
            pAttrList->add(nName, pValue);
    }

    const com::sun::star::uno::Reference<com::sun::star::xml::sax::XFastAttributeList> xAttrList(pAttrList);
    mpSerializer->startFastElement(elementTokenId, xAttrList);
}

void FastSerializerHelper::singleElementV(sal_Int32 elementTokenId, va_list args)
{
    FastAttributeList* pAttrList = new FastAttributeList( mxTokenHandler );

    while (true)
    {
        sal_Int32 nName = va_arg(args, sal_Int32);
        if (nName == FSEND)
            break;
        const char* pValue = va_arg(args, const char*);
        if  (pValue)
            pAttrList->add(nName, pValue);
    }

    const com::sun::star::uno::Reference<com::sun::star::xml::sax::XFastAttributeList> xAttrList(pAttrList);
    mpSerializer->singleFastElement(elementTokenId, xAttrList);
}

void FastSerializerHelper::endElement(sal_Int32 elementTokenId)
{
    mpSerializer->endFastElement(elementTokenId);
}

void FastSerializerHelper::startElementV(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
{
    mpSerializer->startFastElement(elementTokenId, xAttrList);
}


void FastSerializerHelper::singleElementV(sal_Int32 elementTokenId, XFastAttributeListRef xAttrList)
{
    mpSerializer->singleFastElement(elementTokenId, xAttrList);
}

FastSerializerHelper* FastSerializerHelper::write(const char* value)
{
    return write(rtl::OUString::createFromAscii(value));
}

FastSerializerHelper* FastSerializerHelper::write(const rtl::OUString& value)
{
    mpSerializer->characters(value);
    return this;
}

FastSerializerHelper* FastSerializerHelper::write(sal_Int32 value)
{
    return write(::rtl::OUString::valueOf(value));
}

FastSerializerHelper* FastSerializerHelper::write(sal_Int64 value)
{
    return write(::rtl::OUString::valueOf(value));
}

FastSerializerHelper* FastSerializerHelper::write(double value)
{
    return write(::rtl::OUString::valueOf(value));
}

FastSerializerHelper* FastSerializerHelper::writeEscaped(const char* value)
{
    return writeEscaped(::rtl::OUString::createFromAscii(value));
}

FastSerializerHelper* FastSerializerHelper::writeEscaped(const ::rtl::OUString& value)
{
    return write(FastSaxSerializer::escapeXml(value));
}

FastSerializerHelper* FastSerializerHelper::writeId(sal_Int32 tokenId)
{
    mpSerializer->writeId(tokenId);
    return this;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > FastSerializerHelper::getOutputStream()
{
    return mpSerializer->getOutputStream();
}

void FastSerializerHelper::mark( Sequence< sal_Int32 > aOrder )
{
    mpSerializer->mark( aOrder );
}

void FastSerializerHelper::mergeTopMarks( MergeMarksEnum eMergeType )
{
    mpSerializer->mergeTopMarks( eMergeType );
}

FastAttributeList * FastSerializerHelper::createAttrList()
{
    return new FastAttributeList( mxTokenHandler );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
