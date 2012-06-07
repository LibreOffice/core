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
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "NONE");
                break;
            case drawing::FillStyle_SOLID:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "SOLID");
                break;
            case drawing::FillStyle_GRADIENT:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "GRADIENT");
                break;
            case drawing::FillStyle_HATCH:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "HATCH");
                break;
            case drawing::FillStyle_BITMAP:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillStyle"), "%s", "BITMAP");
                break;
            default:
                break;
        }
    }

    void XShapeDumper::dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillColor"), "%" SAL_PRIdINT32, aColor);
    }

    void XShapeDumper::dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillTransparence"), "%" SAL_PRIdINT32, aTransparence);
    }

    void XShapeDumper::dumpFillTransparenceGradientNameAsAttribute(rtl::OUString sTranspGradName, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillTransparenceGradientName"), "%s",
            rtl::OUStringToOString(sTranspGradName, RTL_TEXTENCODING_UTF8).getStr());
    }

    //because there's more awt::Gradient properties to dump
    void XShapeDumper::dumpGradientProperty(awt::Gradient aGradient, xmlTextWriterPtr xmlWriter)
    {
        switch(aGradient.Style)   //enum GradientStyle
        {
            case awt::GradientStyle_LINEAR:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "LINEAR");
                break;
            case awt::GradientStyle_AXIAL:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "AXIAL");
                break;
            case awt::GradientStyle_RADIAL:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RADIAL");
                break;
            case awt::GradientStyle_ELLIPTICAL:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "ELLIPTICAL");
                break;
            case awt::GradientStyle_SQUARE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "SQUARE");
                break;
            case awt::GradientStyle_RECT:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "RECT");
                break;
            default:
                break;
        }
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("startColor"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.StartColor);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("endColor"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.EndColor);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("angle"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.Angle);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("border"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.Border);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("xOffset"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.XOffset);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("yOffset"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.YOffset);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("startIntensity"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.StartIntensity);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("endIntensity"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.EndIntensity);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stepCount"), "%" SAL_PRIdINT32, (sal_Int32) aGradient.StepCount);
    }

    void XShapeDumper::dumpFillTransparenceGradientAsElement(awt::Gradient aTranspGrad, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillTransparenceGradient" ));
        dumpGradientProperty(aTranspGrad, xmlWriter);
        xmlTextWriterEndElement( xmlWriter );
    }

    void XShapeDumper::dumpFillGradientNameAsAttribute(rtl::OUString sGradName, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillGradientName"), "%s",
            rtl::OUStringToOString(sGradName, RTL_TEXTENCODING_UTF8).getStr());
    }

    void XShapeDumper::dumpFillGradientAsElement(awt::Gradient aGradient, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillGradient" ));
        dumpGradientProperty(aGradient, xmlWriter);
        xmlTextWriterEndElement( xmlWriter );
    }

    void XShapeDumper::dumpFillHatchNameAsAttribute(rtl::OUString sHatchName, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillHatchName"), "%s",
            rtl::OUStringToOString(sHatchName, RTL_TEXTENCODING_UTF8).getStr());
    }

    void XShapeDumper::dumpFillHatchAsElement(drawing::Hatch aHatch, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillHatch" ));
        switch(aHatch.Style)
        {
            case drawing::HatchStyle_SINGLE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "SINGLE");
                break;
            case drawing::HatchStyle_DOUBLE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "DOUBLE");
                break;
            case drawing::HatchStyle_TRIPLE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("style"), "%s", "TRIPLE");
                break;
            default:
                break;
        }
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("color"), "%" SAL_PRIdINT32, (sal_Int32) aHatch.Color);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("distance"), "%" SAL_PRIdINT32, (sal_Int32) aHatch.Distance);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("angle"), "%" SAL_PRIdINT32, (sal_Int32) aHatch.Angle);
        xmlTextWriterEndElement( xmlWriter );
    }

    void XShapeDumper::dumpFillBackgroundAsAttribute(sal_Bool bBackground, xmlTextWriterPtr xmlWriter)
    {
        if(bBackground)
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBackground"), "%s", "true");
        else
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBackground"), "%s", "false");
    }

    void XShapeDumper::dumpFillBitmapNameAsAttribute(rtl::OUString sBitmapName, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillGradientName"), "%s",
            rtl::OUStringToOString(sBitmapName, RTL_TEXTENCODING_UTF8).getStr());
    }

    void XShapeDumper::dumpFillBitmapAsElement(uno::Reference<awt::XBitmap> xBitmap, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FillBitmap" ));
        if (xBitmap.is())
        {
            awt::Size const aSize = xBitmap->getSize();
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, aSize.Width);
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, aSize.Height);
        }
        xmlTextWriterEndElement( xmlWriter );
    }

    void XShapeDumper::dumpFillBitmapURLAsAttribute(rtl::OUString sBitmapURL, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapURL"), "%s",
            rtl::OUStringToOString(sBitmapURL, RTL_TEXTENCODING_UTF8).getStr());
    }

    void XShapeDumper::dumpFillBitmapPositionOffsetXAsAttribute(sal_Int32 aBitmapPositionOffsetX, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapPositionOffsetX"), "%" SAL_PRIdINT32, aBitmapPositionOffsetX);
    }

    void XShapeDumper::dumpFillBitmapPositionOffsetYAsAttribute(sal_Int32 aBitmapPositionOffsetY, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapPositionOffsetY"), "%" SAL_PRIdINT32, aBitmapPositionOffsetY);
    }

    void XShapeDumper::dumpFillBitmapOffsetXAsAttribute(sal_Int32 aBitmapOffsetX, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapOffsetX"), "%" SAL_PRIdINT32, aBitmapOffsetX);
    }

    void XShapeDumper::dumpFillBitmapOffsetYAsAttribute(sal_Int32 aBitmapOffsetY, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapOffsetY"), "%" SAL_PRIdINT32, aBitmapOffsetY);
    }

    void XShapeDumper::dumpFillBitmapRectanglePointAsAttribute(drawing::RectanglePoint eBitmapRectanglePoint, xmlTextWriterPtr xmlWriter)
    {
        switch(eBitmapRectanglePoint)
        {
            case drawing::RectanglePoint_LEFT_TOP:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_TOP");
                break;
            case drawing::RectanglePoint_MIDDLE_TOP:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_TOP");
                break;
            case drawing::RectanglePoint_RIGHT_TOP:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_TOP");
                break;
            case drawing::RectanglePoint_LEFT_MIDDLE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_MIDDLE");
                break;
            case drawing::RectanglePoint_MIDDLE_MIDDLE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_MIDDLE");
                break;
            case drawing::RectanglePoint_RIGHT_MIDDLE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_MIDDLE");
                break;
            case drawing::RectanglePoint_LEFT_BOTTOM:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "LEFT_BOTTOM");
                break;
            case drawing::RectanglePoint_MIDDLE_BOTTOM:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "MIDDLE_BOTTOM");
                break;
            case drawing::RectanglePoint_RIGHT_BOTTOM:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapRectanglePoint"), "%s", "RIGHT_BOTTOM");
                break;
            default:
                break;
        }
    }

    void XShapeDumper::dumpFillBitmapLogicalSizeAsAttribute(sal_Bool bBitmapLogicalSize, xmlTextWriterPtr xmlWriter)
    {
        if(bBitmapLogicalSize)
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapLogicalSize"), "%s", "true");
        else
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapLogicalSize"), "%s", "false");
    }

    void XShapeDumper::dumpFillBitmapSizeXAsAttribute(sal_Int32 aBitmapSizeX, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapSizeX"), "%" SAL_PRIdINT32, aBitmapSizeX);
    }

    void XShapeDumper::dumpFillBitmapSizeYAsAttribute(sal_Int32 aBitmapSizeY, xmlTextWriterPtr xmlWriter)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("fillBitmapSizeY"), "%" SAL_PRIdINT32, aBitmapSizeY);
    }

    void XShapeDumper::dumpFillBitmapModeAsAttribute(drawing::BitmapMode eBitmapMode, xmlTextWriterPtr xmlWriter)
    {
        switch(eBitmapMode)
        {
            case drawing::BitmapMode_REPEAT:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "REPEAT");
                break;
            case drawing::BitmapMode_STRETCH:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "STRETCH");
                break;
            case drawing::BitmapMode_NO_REPEAT:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("fillBitmapMode"), "%s", "NO_REPEAT");
                break;
            default:
                break;
        }
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
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s",
            rtl::OUStringToOString(xDescr->getShapeType(), RTL_TEXTENCODING_UTF8).getStr());
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
        else if(xServiceInfo->supportsService("com.sun.star.drawing.FillProperties"))
        {
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillStyle");
                drawing::FillStyle eFillStyle;
                if(anotherAny >>= eFillStyle)
                    dumpFillStyleAsAttribute(eFillStyle, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillColor");
                sal_Int32 aColor;
                if(anotherAny >>= aColor)
                    dumpFillColorAsAttribute(aColor, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillTransparence");
                sal_Int32 aTransparence;
                if(anotherAny >>= aTransparence)
                    dumpFillTransparenceAsAttribute(aTransparence, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillTransparenceGradientName");
                rtl::OUString sTranspGradName;
                if(anotherAny >>= sTranspGradName)
                    dumpFillTransparenceGradientNameAsAttribute(sTranspGradName, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillTransparenceGradient");
                awt::Gradient aTranspGrad;
                if(anotherAny >>= aTranspGrad)
                    dumpFillTransparenceGradientAsElement(aTranspGrad, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillGradientName");
                rtl::OUString sGradName;
                if(anotherAny >>= sGradName)
                    dumpFillGradientNameAsAttribute(sGradName, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillGradient");
                awt::Gradient aGradient;
                if(anotherAny >>= aGradient)
                    dumpFillGradientAsElement(aGradient, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillHatchName");
                rtl::OUString sHatchName;
                if(anotherAny >>= sHatchName)
                    dumpFillGradientNameAsAttribute(sHatchName, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillHatch");
                drawing::Hatch aHatch;
                if(anotherAny >>= aHatch)
                    dumpFillHatchAsElement(aHatch, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBackground");
                sal_Bool bFillBackground;
                if(anotherAny >>= bFillBackground)
                    dumpFillBackgroundAsAttribute(bFillBackground, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapName");
                rtl::OUString sBitmapName;
                if(anotherAny >>= sBitmapName)
                    dumpFillGradientNameAsAttribute(sBitmapName, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmap");
                uno::Reference<awt::XBitmap> xBitmap(xShape, uno::UNO_QUERY);
                if(anotherAny >>= xBitmap)
                    dumpFillBitmapAsElement(xBitmap, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapURL");
                rtl::OUString sBitmapURL;
                if(anotherAny >>= sBitmapURL)
                    dumpFillBitmapURLAsAttribute(sBitmapURL, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapPositionOffsetX");
                sal_Int32 aBitmapPositionOffsetX;
                if(anotherAny >>= aBitmapPositionOffsetX)
                    dumpFillBitmapPositionOffsetXAsAttribute(aBitmapPositionOffsetX, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapPositionOffsetY");
                sal_Int32 aBitmapPositionOffsetY;
                if(anotherAny >>= aBitmapPositionOffsetY)
                    dumpFillBitmapPositionOffsetYAsAttribute(aBitmapPositionOffsetY, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapOffsetX");
                sal_Int32 aBitmapOffsetX;
                if(anotherAny >>= aBitmapOffsetX)
                    dumpFillBitmapOffsetXAsAttribute(aBitmapOffsetX, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapOffsetY");
                sal_Int32 aBitmapOffsetY;
                if(anotherAny >>= aBitmapOffsetY)
                    dumpFillBitmapOffsetYAsAttribute(aBitmapOffsetY, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapRectanglePoint");
                drawing::RectanglePoint eBitmapRectanglePoint;
                if(anotherAny >>= eBitmapRectanglePoint)
                    dumpFillBitmapRectanglePointAsAttribute(eBitmapRectanglePoint, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapLogicalSize");
                sal_Bool bBitmapLogicalSize;
                if(anotherAny >>= bBitmapLogicalSize)
                    dumpFillBitmapLogicalSizeAsAttribute(bBitmapLogicalSize, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapSizeX");
                sal_Int32 aBitmapSizeX;
                if(anotherAny >>= aBitmapSizeX)
                    dumpFillBitmapSizeXAsAttribute(aBitmapSizeX, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapSizeY");
                sal_Int32 aBitmapSizeY;
                if(anotherAny >>= aBitmapSizeY)
                    dumpFillBitmapSizeYAsAttribute(aBitmapSizeY, xmlWriter);
            }
            {
                uno::Any anotherAny = xPropSet->getPropertyValue("FillBitmapMode");
                drawing::BitmapMode eBitmapMode;
                if(anotherAny >>= eBitmapMode)
                    dumpFillBitmapModeAsAttribute(eBitmapMode, xmlWriter);
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

