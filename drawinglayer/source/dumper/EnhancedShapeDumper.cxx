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

#include <drawinglayer/EnhancedShapeDumper.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

// ------------------------------------------------------
// ---------- EnhancedCustomShapeExtrusion.idl ----------
// ------------------------------------------------------

void EnhancedShapeDumper::dumpEnhancedCustomShapeExtrusionService(uno::Reference< beans::XPropertySet > xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Extrusion");
        sal_Bool bExtrusion;
        if(anotherAny >>= bExtrusion)
            dumpExtrusionAsAttribute(bExtrusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Brightness");
        double aBrightness;
        if(anotherAny >>= aBrightness)
            dumpBrightnessAsAttribute(aBrightness);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Depth");
        drawing::EnhancedCustomShapeParameterPair aDepth;
        if(anotherAny >>= aDepth)
            dumpDepthAsElement(aDepth);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Diffusion");
        double aDiffusion;
        if(anotherAny >>= aDiffusion)
            dumpDiffusionAsAttribute(aDiffusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("NumberOfLineSegments");
        sal_Int32 aNumberOfLineSegments;
        if(anotherAny >>= aNumberOfLineSegments)
            dumpNumberOfLineSegmentsAsAttribute(aNumberOfLineSegments);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LightFace");
        sal_Bool bLightFace;
        if(anotherAny >>= bLightFace)
            dumpLightFaceAsAttribute(bLightFace);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FirstLightHarsh");
        sal_Bool bFirstLightHarsh;
        if(anotherAny >>= bFirstLightHarsh)
            dumpFirstLightHarshAsAttribute(bFirstLightHarsh);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SecondLightHarsh");
        sal_Bool bSecondLightHarsh;
        if(anotherAny >>= bSecondLightHarsh)
            dumpSecondLightHarshAsAttribute(bSecondLightHarsh);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FirstLightLevel");
        double aFirstLightLevel;
        if(anotherAny >>= aFirstLightLevel)
            dumpFirstLightLevelAsAttribute(aFirstLightLevel);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SecondLightLevel");
        double aSecondLightLevel;
        if(anotherAny >>= aSecondLightLevel)
            dumpSecondLightLevelAsAttribute(aSecondLightLevel);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FirstLightDirection");
        drawing::Direction3D aFirstLightDirection;
        if(anotherAny >>= aFirstLightDirection)
            dumpFirstLightDirectionAsElement(aFirstLightDirection);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SecondLightDirection");
        drawing::Direction3D aSecondLightDirection;
        if(anotherAny >>= aSecondLightDirection)
            dumpSecondLightDirectionAsElement(aSecondLightDirection);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Metal");
        sal_Bool bMetal;
        if(anotherAny >>= bMetal)
            dumpMetalAsAttribute(bMetal);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ShadeMode");
        drawing::ShadeMode eShadeMode;
        if(anotherAny >>= eShadeMode)
            dumpShadeModeAsAttribute(eShadeMode);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RotateAngle");
        drawing::EnhancedCustomShapeParameterPair aRotateAngle;
        if(anotherAny >>= aRotateAngle)
            dumpRotateAngleAsElement(aRotateAngle);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RotationCenter");
        drawing::Direction3D aRotationCenter;
        if(anotherAny >>= aRotationCenter)
            dumpRotationCenterAsElement(aRotationCenter);
    }
}
void EnhancedShapeDumper::dumpExtrusionAsAttribute(sal_Bool bExtrusion)
{
    if(bExtrusion)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusion"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusion"), "%s", "false");
}

void EnhancedShapeDumper::dumpBrightnessAsAttribute(double aBrightness)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("brightness"), "%f", aBrightness);
}

void EnhancedShapeDumper::dumpEnhancedCustomShapeParameterPair(drawing::EnhancedCustomShapeParameterPair aParameterPair)
{
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "First" ));
        uno::Any aAny = aParameterPair.First.Value;
        rtl::OUString sValue;
        if(aAny >>= sValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
                rtl::OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
        }
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%" SAL_PRIdINT32, aParameterPair.First.Type);
        xmlTextWriterEndElement( xmlWriter );
    }
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Second" ));
        uno::Any aAny = aParameterPair.Second.Value;
        rtl::OUString sValue;
        if(aAny >>= sValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
                rtl::OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
        }
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%" SAL_PRIdINT32, aParameterPair.Second.Type);
        xmlTextWriterEndElement( xmlWriter );
    }
}

void EnhancedShapeDumper::dumpDepthAsElement(drawing::EnhancedCustomShapeParameterPair aDepth)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Depth" ));
    dumpEnhancedCustomShapeParameterPair(aDepth);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpDiffusionAsAttribute(double aDiffusion)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("diffusion"), "%f", aDiffusion);
}

void EnhancedShapeDumper::dumpNumberOfLineSegmentsAsAttribute(sal_Int32 aNumberOfLineSegments)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("numberOfLineSegments"), "%" SAL_PRIdINT32, aNumberOfLineSegments);
}

void EnhancedShapeDumper::dumpLightFaceAsAttribute(sal_Bool bLightFace)
{
    if(bLightFace)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lightFace"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lightFace"), "%s", "false");
}

void EnhancedShapeDumper::dumpFirstLightHarshAsAttribute(sal_Bool bFirstLightHarsh)
{
    if(bFirstLightHarsh)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightHarsh"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightHarsh"), "%s", "false");
}

void EnhancedShapeDumper::dumpSecondLightHarshAsAttribute(sal_Bool bSecondLightHarsh)
{
    if(bSecondLightHarsh)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("secondLightHarsh"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("secondLightHarsh"), "%s", "false");
}

void EnhancedShapeDumper::dumpFirstLightLevelAsAttribute(double aFirstLightLevel)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightLevel"), "%f", aFirstLightLevel);
}

void EnhancedShapeDumper::dumpSecondLightLevelAsAttribute(double aSecondLightLevel)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("secondLightLevel"), "%f", aSecondLightLevel);
}

void EnhancedShapeDumper::dumpDirection3D(drawing::Direction3D aDirection3D)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("directionX"), "%f", aDirection3D.DirectionX);
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("directionY"), "%f", aDirection3D.DirectionY);
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("directionZ"), "%f", aDirection3D.DirectionZ);
}

void EnhancedShapeDumper::dumpFirstLightDirectionAsElement(drawing::Direction3D aFirstLightDirection)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FirstLightDirection" ));
    dumpDirection3D(aFirstLightDirection);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpSecondLightDirectionAsElement(drawing::Direction3D aSecondLightDirection)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "SecondLightDirection" ));
    dumpDirection3D(aSecondLightDirection);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpMetalAsAttribute(sal_Bool bMetal)
{
    if(bMetal)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("metal"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("metal"), "%s", "false");
}

void EnhancedShapeDumper::dumpShadeModeAsAttribute(drawing::ShadeMode eShadeMode)
{
    switch(eShadeMode)
    {
        case drawing::ShadeMode_FLAT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "FLAT");
            break;
        case drawing::ShadeMode_PHONG:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "PHONG");
            break;
        case drawing::ShadeMode_SMOOTH:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "SMOOTH");
            break;
        case drawing::ShadeMode_DRAFT:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "DRAFT");
            break;
        default:
            break;
    }
}

void EnhancedShapeDumper::dumpRotateAngleAsElement(drawing::EnhancedCustomShapeParameterPair aRotateAngle)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RotateAngle" ));
    dumpEnhancedCustomShapeParameterPair(aRotateAngle);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRotationCenterAsElement(drawing::Direction3D aRotationCenter)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RotationCenter" ));
    dumpDirection3D(aRotationCenter);
    xmlTextWriterEndElement( xmlWriter );
}


