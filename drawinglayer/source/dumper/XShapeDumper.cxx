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
 * Copyright (C) 2012 Artur Dorda <artur.dorda+libo@gmail.com> (initial developer)
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

#include <drawinglayer/XShapeDumper.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <rtl/strbuf.hxx>


#define DEBUG_DUMPER 0

using namespace com::sun::star;
//class XShapeDumper

namespace {

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
} //end of namespace

    XShapeDumper::XShapeDumper()
    {

    }
    // ----------------------------------------
    // ---------- FillProperties.idl ----------
    // ----------------------------------------
    void XShapeDumper::dumpFillStyleAsAttribute(drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter)
    {
        switch(eFillStyle)
        {
            case drawing::FillStyle_NONE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("FillStyle"), "%s", "NONE");
                break;
            case drawing::FillStyle_SOLID:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("FillStyle"), "%s", "SOLID");
                break;
            case drawing::FillStyle_GRADIENT:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("FillStyle"), "%s", "GRADIENT");
                break;
            case drawing::FillStyle_HATCH:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("FillStyle"), "%s", "HATCH");
                break;
            case drawing::FillStyle_BITMAP:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("FillStyle"), "%s", "BITMAP");
                break;
            default:
                break;
        }
    }

    void XShapeDumper::dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, aColor);
    }

    void XShapeDumper::dumpPositionAsAttribute(const awt::Point& rPoint, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, rPoint.X);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, rPoint.Y);
    }

    void XShapeDumper::dumpSizeAsAttribute(const awt::Size& rSize, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeX"), "%" SAL_PRIdINT32, rSize.Width);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeY"), "%" SAL_PRIdINT32, rSize.Height);
    }

    void XShapeDumper::dumpShapeDescriptorAsAttribute( uno::Reference< drawing::XShapeDescriptor > xDescr, xmlTextWriterPtr xmlWriter )
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s", rtl::OUStringToOString(xDescr->getShapeType(), RTL_TEXTENCODING_UTF8).getStr());
    }

    void XShapeDumper::dumpXShape(uno::Reference< drawing::XShape > xShape, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShape" ) );
        uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySetInfo> xPropSetInfo = xPropSet->getPropertySetInfo();
        rtl::OUString aName;

        dumpPositionAsAttribute(xShape->getPosition(), xmlWriter);
        dumpSizeAsAttribute(xShape->getSize(), xmlWriter);
        uno::Reference< drawing::XShapeDescriptor > xDescr(xShape, uno::UNO_QUERY_THROW);
        dumpShapeDescriptorAsAttribute(xDescr, xmlWriter);

        // uno::Sequence<beans::Property> aProperties = xPropSetInfo->getProperties();

        uno::Reference< lang::XServiceInfo > xServiceInfo( xShape, uno::UNO_QUERY_THROW );
        uno::Sequence< rtl::OUString > aServiceNames = xServiceInfo->getSupportedServiceNames();

        uno::Any aAny = xPropSet->getPropertyValue("Name");
        if (aAny >>= aName)
        {
            if (!aName.isEmpty())
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("name"), "%s", rtl::OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr());
        }
        if (xServiceInfo->supportsService("com.sun.star.drawing.Text"))
        {
            uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY_THROW);
            rtl::OUString aText = xText->getString();
            if(!aText.isEmpty())
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("text"), "%s", rtl::OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
        }
        else if(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
        {
            uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY_THROW);
            dumpXShapes(xShapes, xmlWriter);
        }
        else if(xServiceInfo->supportsService("com.sun.star.drawing.FillStyle"))
        {
            uno::Any anotherAny = xPropSet->getPropertyValue("FillStyle");
            drawing::FillStyle eFillStyle;
            if( anotherAny >>= eFillStyle)
                    dumpFillStyleAsAttribute(eFillStyle, xmlWriter);
        }
        else if(xServiceInfo->supportsService("com.sun.star.util.Color"))
        {
            uno::Any anotherAny = xPropSet->getPropertyValue("FillColor");
            sal_Int32 aColor;
            if(anotherAny >>= aColor)
            {
                dumpFillColorAsAttribute(aColor, xmlWriter);
            }
        }

        #if DEBUG_DUMPER
            sal_Int32 nServices = aServiceNames.getLength();
            for (sal_Int32 i = 0; i < nServices; ++i)
            {
                xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ServiceName" ));
                xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST( "name" ), "%s", rtl::OUStringToOString(aServiceNames[i], RTL_TEXTENCODING_UTF8).getStr());
                xmlTextWriterEndElement( xmlWriter );
            }
        #endif

        xmlTextWriterEndElement( xmlWriter );
    }

    void XShapeDumper::dumpXShapes( uno::Reference< drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter )
    {
        xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShapes" ) );
        uno::Reference< container::XIndexAccess > xIA( xShapes, uno::UNO_QUERY_THROW);
        sal_Int32 nLength = xIA->getCount();
        for (sal_Int32 i = 0; i < nLength; ++i)
        {
            uno::Reference< drawing::XShape > xShape( xIA->getByIndex( i ), uno::UNO_QUERY_THROW );
            dumpXShape( xShape, xmlWriter );
        }

        xmlTextWriterEndElement( xmlWriter );
    }

    rtl::OUString XShapeDumper::dump(uno::Reference<drawing::XShapes> xPageShapes)
    {

        rtl::OStringBuffer aString;
        xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO( writeCallback, closeCallback, &aString, NULL );
        xmlTextWriterPtr xmlWriter = xmlNewTextWriter( xmlOutBuffer );
        xmlTextWriterSetIndent( xmlWriter, 1 );

        xmlTextWriterStartDocument( xmlWriter, NULL, NULL, NULL );

        dumpXShapes( xPageShapes, xmlWriter );

        xmlTextWriterEndDocument( xmlWriter );
        xmlFreeTextWriter( xmlWriter );

        return OStringToOUString(aString.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
    }

