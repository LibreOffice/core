/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "test/xshape_dumper.hxx"
#include <rtl/strbuf.hxx>
#include <rtl/oustringostreaminserter.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>

using namespace com::sun::star;

namespace {

#define DEBUG_DUMPER 0

int writeCallback(void* pContext, const char* sBuffer, int nLen)
{
    rtl::OStringBuffer* pBuffer = static_cast<rtl::OStringBuffer*>(pContext);
    pBuffer->append(sBuffer);
    return nLen;
}

int closeCallback(void* )
{
    return 0;
}

}

void XShapeDumper::dumpPositionAsAttribute(const awt::Point& rPoint)
{
    xmlTextWriterWriteFormatAttribute(mxmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, rPoint.X);
    xmlTextWriterWriteFormatAttribute(mxmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, rPoint.Y);
}

void XShapeDumper::dumpSizeAsAttribute(const awt::Size& rSize)
{
    xmlTextWriterWriteFormatAttribute(mxmlWriter, BAD_CAST("sizeX"), "%" SAL_PRIdINT32, rSize.Width);
    xmlTextWriterWriteFormatAttribute(mxmlWriter, BAD_CAST("sizeY"), "%" SAL_PRIdINT32, rSize.Height);
}

void XShapeDumper::dumpShapeDescriptorAsAttribute( uno::Reference< drawing::XShapeDescriptor > xDescr )
{
    xmlTextWriterWriteFormatAttribute(mxmlWriter, BAD_CAST("type"), "%s", rtl::OUStringToOString(xDescr->getShapeType(), RTL_TEXTENCODING_UTF8).getStr());
}

void XShapeDumper::dumpXShape( uno::Reference< drawing::XShape > xShape )
{
    xmlTextWriterStartElement( mxmlWriter, BAD_CAST( "XShape" ) );

    dumpPositionAsAttribute(xShape->getPosition());
    dumpSizeAsAttribute(xShape->getSize());
    uno::Reference< drawing::XShapeDescriptor > xDescr(xShape, uno::UNO_QUERY_THROW);
    dumpShapeDescriptorAsAttribute(xDescr);

    uno::Reference< lang::XServiceInfo > xServiceInfo( xShape, uno::UNO_QUERY_THROW );
    uno::Sequence< rtl::OUString > aServiceNames = xServiceInfo->getSupportedServiceNames();

    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("Name");
    rtl::OUString aName;
    if (aAny >>= aName)
    {
        if (!aName.isEmpty())
            xmlTextWriterWriteFormatAttribute( mxmlWriter, BAD_CAST("name"), "%s", rtl::OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (xServiceInfo->supportsService("com.sun.star.drawing.Text"))
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY_THROW);
        rtl::OUString aText = xText->getString();
        if(!aText.isEmpty())
            xmlTextWriterWriteFormatAttribute( mxmlWriter, BAD_CAST("text"), "%s", rtl::OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
    }
    else if(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
    {
        uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY_THROW);
        dumpXShapes(xShapes);
    }
#if DEBUG_DUMPER
    sal_Int32 nServices = aServiceNames.getLength();
    for (sal_Int32 i = 0; i < nServices; ++i)
    {
        xmlTextWriterStartElement(mxmlWriter, BAD_CAST( "ServiceName" ));
        xmlTextWriterWriteFormatAttribute(mxmlWriter, BAD_CAST( "name" ), "%s", rtl::OUStringToOString(aServiceNames[i], RTL_TEXTENCODING_UTF8).getStr());
        xmlTextWriterEndElement( mxmlWriter );
    }
#endif

    xmlTextWriterEndElement( mxmlWriter );
}

void XShapeDumper::dumpXShapes( uno::Reference< drawing::XShapes > xShapes )
{
    xmlTextWriterStartElement( mxmlWriter, BAD_CAST( "XShapes" ) );
    uno::Reference< container::XIndexAccess > xIA( xShapes, uno::UNO_QUERY_THROW);
    sal_Int32 nLength = xIA->getCount();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference< drawing::XShape > xShape( xIA->getByIndex( i ), uno::UNO_QUERY_THROW );
        dumpXShape( xShape );
    }

    xmlTextWriterEndElement( mxmlWriter );
}

rtl::OUString XShapeDumper::dump()
{
    rtl::OStringBuffer aString;
    xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO( writeCallback, closeCallback, &aString, NULL );
    mxmlWriter = xmlNewTextWriter( xmlOutBuffer );
    xmlTextWriterSetIndent( mxmlWriter, 1 );

    xmlTextWriterStartDocument( mxmlWriter, NULL, NULL, NULL );

    dumpXShapes( mxShapes );

    xmlTextWriterEndDocument( mxmlWriter );
    xmlFreeTextWriter( mxmlWriter );

    return OStringToOUString(aString.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
