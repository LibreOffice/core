/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EnhancedShapeDumper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;


// ---------- EnhancedCustomShapeExtrusion.idl ----------


void EnhancedShapeDumper::dumpEnhancedCustomShapeExtrusionService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Extrusion"_ustr);
        bool bExtrusion;
        if(anotherAny >>= bExtrusion)
            dumpExtrusionAsAttribute(bExtrusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Brightness"_ustr);
        double aBrightness = double();
        if(anotherAny >>= aBrightness)
            dumpBrightnessAsAttribute(aBrightness);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Depth"_ustr);
        drawing::EnhancedCustomShapeParameterPair aDepth;
        if(anotherAny >>= aDepth)
            dumpDepthAsElement(aDepth);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Diffusion"_ustr);
        double aDiffusion = double();
        if(anotherAny >>= aDiffusion)
            dumpDiffusionAsAttribute(aDiffusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"NumberOfLineSegments"_ustr);
        sal_Int32 aNumberOfLineSegments = sal_Int32();
        if(anotherAny >>= aNumberOfLineSegments)
            dumpNumberOfLineSegmentsAsAttribute(aNumberOfLineSegments);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"LightFace"_ustr);
        bool bLightFace;
        if(anotherAny >>= bLightFace)
            dumpLightFaceAsAttribute(bLightFace);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"FirstLightHarsh"_ustr);
        bool bFirstLightHarsh;
        if(anotherAny >>= bFirstLightHarsh)
            dumpFirstLightHarshAsAttribute(bFirstLightHarsh);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"SecondLightHarsh"_ustr);
        bool bSecondLightHarsh;
        if(anotherAny >>= bSecondLightHarsh)
            dumpSecondLightHarshAsAttribute(bSecondLightHarsh);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"FirstLightLevel"_ustr);
        double aFirstLightLevel = double();
        if(anotherAny >>= aFirstLightLevel)
            dumpFirstLightLevelAsAttribute(aFirstLightLevel);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"SecondLightLevel"_ustr);
        double aSecondLightLevel = double();
        if(anotherAny >>= aSecondLightLevel)
            dumpSecondLightLevelAsAttribute(aSecondLightLevel);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"FirstLightDirection"_ustr);
        drawing::Direction3D aFirstLightDirection;
        if(anotherAny >>= aFirstLightDirection)
            dumpFirstLightDirectionAsElement(aFirstLightDirection);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"SecondLightDirection"_ustr);
        drawing::Direction3D aSecondLightDirection;
        if(anotherAny >>= aSecondLightDirection)
            dumpSecondLightDirectionAsElement(aSecondLightDirection);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Metal"_ustr);
        bool bMetal;
        if(anotherAny >>= bMetal)
            dumpMetalAsAttribute(bMetal);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ShadeMode"_ustr);
        drawing::ShadeMode eShadeMode;
        if(anotherAny >>= eShadeMode)
            dumpShadeModeAsAttribute(eShadeMode);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RotateAngle"_ustr);
        drawing::EnhancedCustomShapeParameterPair aRotateAngle;
        if(anotherAny >>= aRotateAngle)
            dumpRotateAngleAsElement(aRotateAngle);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RotationCenter"_ustr);
        drawing::Direction3D aRotationCenter;
        if(anotherAny >>= aRotationCenter)
            dumpRotationCenterAsElement(aRotationCenter);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Shininess"_ustr);
        double aShininess = double();
        if(anotherAny >>= aShininess)
            dumpShininessAsAttribute(aShininess);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Skew"_ustr);
        drawing::EnhancedCustomShapeParameterPair aSkew;
        if(anotherAny >>= aSkew)
            dumpSkewAsElement(aSkew);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Specularity"_ustr);
        double aSpecularity = double();
        if(anotherAny >>= aSpecularity)
            dumpSpecularityAsAttribute(aSpecularity);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ProjectionMode"_ustr);
        drawing::ProjectionMode eProjectionMode;
        if(anotherAny >>= eProjectionMode)
            dumpProjectionModeAsAttribute(eProjectionMode);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ViewPoint"_ustr);
        drawing::Position3D aViewPoint;
        if(anotherAny >>= aViewPoint)
            dumpViewPointAsElement(aViewPoint);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Origin"_ustr);
        drawing::EnhancedCustomShapeParameterPair aOrigin;
        if(anotherAny >>= aOrigin)
            dumpOriginAsElement(aOrigin);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ExtrusionColor"_ustr);
        bool bExtrusionColor;
        if(anotherAny >>= bExtrusionColor)
            dumpExtrusionColorAsAttribute(bExtrusionColor);
    }
}
void EnhancedShapeDumper::dumpExtrusionAsAttribute(bool bExtrusion)
{
    if(bExtrusion)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusion"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusion"), "%s", "false");
}

void EnhancedShapeDumper::dumpBrightnessAsAttribute(double aBrightness)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("brightness"), "%f", aBrightness);
}

void EnhancedShapeDumper::dumpEnhancedCustomShapeParameterPair(
    const drawing::EnhancedCustomShapeParameterPair& aParameterPair)
{
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "First" ));
        dumpEnhancedCustomShapeParameter(aParameterPair.First);
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Second" ));
        dumpEnhancedCustomShapeParameter(aParameterPair.Second);
        (void)xmlTextWriterEndElement( xmlWriter );
    }
}

void EnhancedShapeDumper::dumpDepthAsElement(const drawing::EnhancedCustomShapeParameterPair& aDepth)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Depth" ));
    dumpEnhancedCustomShapeParameterPair(aDepth);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpDiffusionAsAttribute(double aDiffusion)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("diffusion"), "%f", aDiffusion);
}

void EnhancedShapeDumper::dumpNumberOfLineSegmentsAsAttribute(sal_Int32 aNumberOfLineSegments)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("numberOfLineSegments"), "%" SAL_PRIdINT32, aNumberOfLineSegments);
}

void EnhancedShapeDumper::dumpLightFaceAsAttribute(bool bLightFace)
{
    if(bLightFace)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lightFace"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lightFace"), "%s", "false");
}

void EnhancedShapeDumper::dumpFirstLightHarshAsAttribute(bool bFirstLightHarsh)
{
    if(bFirstLightHarsh)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightHarsh"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightHarsh"), "%s", "false");
}

void EnhancedShapeDumper::dumpSecondLightHarshAsAttribute(bool bSecondLightHarsh)
{
    if(bSecondLightHarsh)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("secondLightHarsh"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("secondLightHarsh"), "%s", "false");
}

void EnhancedShapeDumper::dumpFirstLightLevelAsAttribute(double aFirstLightLevel)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightLevel"), "%f", aFirstLightLevel);
}

void EnhancedShapeDumper::dumpSecondLightLevelAsAttribute(double aSecondLightLevel)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("secondLightLevel"), "%f", aSecondLightLevel);
}

void EnhancedShapeDumper::dumpDirection3D(drawing::Direction3D aDirection3D)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("directionX"), "%f", aDirection3D.DirectionX);
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("directionY"), "%f", aDirection3D.DirectionY);
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("directionZ"), "%f", aDirection3D.DirectionZ);
}

void EnhancedShapeDumper::dumpFirstLightDirectionAsElement(drawing::Direction3D aFirstLightDirection)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "FirstLightDirection" ));
    dumpDirection3D(aFirstLightDirection);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpSecondLightDirectionAsElement(drawing::Direction3D aSecondLightDirection)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "SecondLightDirection" ));
    dumpDirection3D(aSecondLightDirection);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpMetalAsAttribute(bool bMetal)
{
    if(bMetal)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("metal"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("metal"), "%s", "false");
}

void EnhancedShapeDumper::dumpShadeModeAsAttribute(drawing::ShadeMode eShadeMode)
{
    switch(eShadeMode)
    {
        case drawing::ShadeMode_FLAT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "FLAT");
            break;
        case drawing::ShadeMode_PHONG:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "PHONG");
            break;
        case drawing::ShadeMode_SMOOTH:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "SMOOTH");
            break;
        case drawing::ShadeMode_DRAFT:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shadeMode"), "%s", "DRAFT");
            break;
        default:
            break;
    }
}

void EnhancedShapeDumper::dumpRotateAngleAsElement(const drawing::EnhancedCustomShapeParameterPair& aRotateAngle)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RotateAngle" ));
    dumpEnhancedCustomShapeParameterPair(aRotateAngle);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRotationCenterAsElement(drawing::Direction3D aRotationCenter)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RotationCenter" ));
    dumpDirection3D(aRotationCenter);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpShininessAsAttribute(double aShininess)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shininess"), "%f", aShininess);
}

void EnhancedShapeDumper::dumpSkewAsElement(const drawing::EnhancedCustomShapeParameterPair& aSkew)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Skew" ));
    dumpEnhancedCustomShapeParameterPair(aSkew);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpSpecularityAsAttribute(double aSpecularity)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("specularity"), "%f", aSpecularity);
}

void EnhancedShapeDumper::dumpProjectionModeAsAttribute(drawing::ProjectionMode eProjectionMode)
{
    switch(eProjectionMode)
    {
        case drawing::ProjectionMode_PARALLEL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("projectionMode"), "%s", "PARALLEL");
            break;
        case drawing::ProjectionMode_PERSPECTIVE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("projectionMode"), "%s", "PERSPECTIVE");
            break;
        default:
            break;
    }
}

void EnhancedShapeDumper::dumpViewPointAsElement(drawing::Position3D aViewPoint)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ViewPoint" ));
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("positionX"), "%f", aViewPoint.PositionX);
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("positionY"), "%f", aViewPoint.PositionY);
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("positionZ"), "%f", aViewPoint.PositionZ);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpOriginAsElement(const drawing::EnhancedCustomShapeParameterPair& aOrigin)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Origin" ));
    dumpEnhancedCustomShapeParameterPair(aOrigin);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpExtrusionColorAsAttribute(bool bExtrusionColor)
{
    if(bExtrusionColor)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionColor"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionColor"), "%s", "false");
}


// ---------- EnhancedCustomShapeGeometry.idl -----------


void EnhancedShapeDumper::dumpEnhancedCustomShapeGeometryService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Type"_ustr);
        OUString sType;
        if(anotherAny >>= sType)
            dumpTypeAsAttribute(sType);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ViewBox"_ustr);
        awt::Rectangle aViewBox;
        if(anotherAny >>= aViewBox)
            dumpViewBoxAsElement(aViewBox);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"MirroredX"_ustr);
        bool bMirroredX;
        if(anotherAny >>= bMirroredX)
            dumpMirroredXAsAttribute(bMirroredX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"MirroredY"_ustr);
        bool bMirroredY;
        if(anotherAny >>= bMirroredY)
            dumpMirroredYAsAttribute(bMirroredY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"TextRotateAngle"_ustr);
        double aTextRotateAngle = double();
        if(anotherAny >>= aTextRotateAngle)
            dumpTextRotateAngleAsAttribute(aTextRotateAngle);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"AdjustmentValues"_ustr);
        uno::Sequence< drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
        if(anotherAny >>= aAdjustmentValues)
            dumpAdjustmentValuesAsElement(aAdjustmentValues);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Extrusion"_ustr);
        uno::Sequence< beans::PropertyValue > aExtrusion;
        if(anotherAny >>= aExtrusion)
            dumpExtrusionAsElement(aExtrusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Path"_ustr);
        uno::Sequence< beans::PropertyValue > aPath;
        if(anotherAny >>= aPath)
            dumpPathAsElement(aPath);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"TextPath"_ustr);
        uno::Sequence< beans::PropertyValue > aTextPath;
        if(anotherAny >>= aTextPath)
            dumpTextPathAsElement(aTextPath);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Equations"_ustr);
        uno::Sequence< OUString > aEquations;
        if(anotherAny >>= aEquations)
            dumpEquationsAsElement(aEquations);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Handles"_ustr);
        uno::Sequence< beans::PropertyValues > aHandles;
        if(anotherAny >>= aHandles)
            dumpHandlesAsElement(aHandles);
    }
}
void EnhancedShapeDumper::dumpTypeAsAttribute(std::u16string_view sType)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s",
        OUStringToOString(sType, RTL_TEXTENCODING_UTF8).getStr());
}

void EnhancedShapeDumper::dumpViewBoxAsElement(awt::Rectangle aViewBox)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ViewBox" ));
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("x"), "%" SAL_PRIdINT32, aViewBox.X);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("y"), "%" SAL_PRIdINT32, aViewBox.Y);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, aViewBox.Width);
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, aViewBox.Height);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpMirroredXAsAttribute(bool bMirroredX)
{
    if(bMirroredX)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredX"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredX"), "%s", "false");
}

void EnhancedShapeDumper::dumpMirroredYAsAttribute(bool bMirroredY)
{
    if(bMirroredY)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredY"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredY"), "%s", "false");
}

void EnhancedShapeDumper::dumpTextRotateAngleAsAttribute(double aTextRotateAngle)
{
    (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textRotateAngle"), "%f", aTextRotateAngle);
}

void EnhancedShapeDumper::dumpAdjustmentValuesAsElement(const uno::Sequence< drawing::EnhancedCustomShapeAdjustmentValue>& aAdjustmentValues)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "AdjustmentValues" ));
    for (const auto& i : aAdjustmentValues)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeAdjustmentValue" ));
        uno::Any aAny = i.Value;
        OUString sValue;
        float fValue;
        sal_Int32 nValue;
        bool bValue;
        if(aAny >>= sValue)
        {
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
                OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
        }
        else if(aAny >>= nValue)
        {
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%" SAL_PRIdINT32, nValue);
        }
        else if(aAny >>= fValue)
        {
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%f", fValue);
        }
        else if(aAny >>= bValue)
        {
            (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s", (bValue? "true": "false"));
        }

        switch(i.State)
        {
            case beans::PropertyState_DIRECT_VALUE:
                (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DIRECT_VALUE");
                break;
            case beans::PropertyState_DEFAULT_VALUE:
                (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DEFAULT_VALUE");
                break;
            case beans::PropertyState_AMBIGUOUS_VALUE:
                (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "AMBIGUOUS_VALUE");
                break;
            default:
                break;
        }
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpPropertyValueAsElement(const beans::PropertyValue& aPropertyValue)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PropertyValue" ));

    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
        OUStringToOString(aPropertyValue.Name, RTL_TEXTENCODING_UTF8).getStr());
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("handle"), "%" SAL_PRIdINT32, aPropertyValue.Handle);

    uno::Any aAny = aPropertyValue.Value;
    OUString sValue;
    if(aAny >>= sValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
            OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
    }
    switch(aPropertyValue.State)
    {
        case beans::PropertyState_DIRECT_VALUE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DIRECT_VALUE");
            break;
        case beans::PropertyState_DEFAULT_VALUE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DEFAULT_VALUE");
            break;
        case beans::PropertyState_AMBIGUOUS_VALUE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "AMBIGUOUS_VALUE");
            break;
        default:
            break;
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpExtrusionAsElement(const uno::Sequence< beans::PropertyValue >& aExtrusion)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Extrusion" ));
    for (const auto& i : aExtrusion)
    {
        dumpPropertyValueAsElement(i);
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpPathAsElement(const uno::Sequence< beans::PropertyValue >& aPath)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Path" ));
    for (const auto& i : aPath)
    {
        dumpPropertyValueAsElement(i);
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpTextPathAsElement(const uno::Sequence< beans::PropertyValue >& aTextPath)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "TextPath" ));
    for (const auto& i : aTextPath)
    {
        dumpPropertyValueAsElement(i);
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpEquationsAsElement(const uno::Sequence< OUString >& aEquations)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Equations" ));
    for (const auto& i : aEquations)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
            OUStringToOString(i, RTL_TEXTENCODING_UTF8).getStr());
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

// PropertyValues specifies a sequence of PropertyValue instances.
// so in this case it's a Sequence of a Sequence of a PropertyValue instances.
// Welcome to Sequenception again.
void EnhancedShapeDumper::dumpHandlesAsElement(const uno::Sequence< beans::PropertyValues >& aHandles)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Handles" ));
    for (const auto& i : aHandles)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PropertyValues" ));
        uno::Sequence< beans::PropertyValue > propertyValueSequence = i;
        for (const auto& j : propertyValueSequence)
        {
            dumpPropertyValueAsElement(j);
        }
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}


// ---------- EnhancedCustomShapeHandle.idl -----------


void EnhancedShapeDumper::dumpEnhancedCustomShapeHandleService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"MirroredX"_ustr);
        bool bMirroredX;
        if(anotherAny >>= bMirroredX)
            dumpMirroredXAsAttribute(bMirroredX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"MirroredY"_ustr);
        bool bMirroredY;
        if(anotherAny >>= bMirroredY)
            dumpMirroredYAsAttribute(bMirroredY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Switched"_ustr);
        bool bSwitched;
        if(anotherAny >>= bSwitched)
            dumpSwitchedAsAttribute(bSwitched);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Position"_ustr);
        drawing::EnhancedCustomShapeParameterPair aPosition;
        if(anotherAny >>= aPosition)
            dumpPositionAsElement(aPosition);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Polar"_ustr);
        drawing::EnhancedCustomShapeParameterPair aPolar;
        if(anotherAny >>= aPolar)
            dumpPolarAsElement(aPolar);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RefX"_ustr);
        sal_Int32 aRefX = sal_Int32();
        if(anotherAny >>= aRefX)
            dumpRefXAsAttribute(aRefX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RefY"_ustr);
        sal_Int32 aRefY = sal_Int32();
        if(anotherAny >>= aRefY)
            dumpRefYAsAttribute(aRefY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RefAngle"_ustr);
        sal_Int32 aRefAngle = sal_Int32();
        if(anotherAny >>= aRefAngle)
            dumpRefAngleAsAttribute(aRefAngle);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RefR"_ustr);
        sal_Int32 aRefR = sal_Int32();
        if(anotherAny >>= aRefR)
            dumpRefRAsAttribute(aRefR);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RangeXMinimum"_ustr);
        drawing::EnhancedCustomShapeParameter aRangeXMinimum;
        if(anotherAny >>= aRangeXMinimum)
            dumpRangeXMinimumAsElement(aRangeXMinimum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RangeXMaximum"_ustr);
        drawing::EnhancedCustomShapeParameter aRangeXMaximum;
        if(anotherAny >>= aRangeXMaximum)
            dumpRangeXMaximumAsElement(aRangeXMaximum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RangeYMinimum"_ustr);
        drawing::EnhancedCustomShapeParameter aRangeYMinimum;
        if(anotherAny >>= aRangeYMinimum)
            dumpRangeYMinimumAsElement(aRangeYMinimum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RangeYMaximum"_ustr);
        drawing::EnhancedCustomShapeParameter aRangeYMaximum;
        if(anotherAny >>= aRangeYMaximum)
            dumpRangeYMaximumAsElement(aRangeYMaximum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RadiusRangeMinimum"_ustr);
        drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
        if(anotherAny >>= aRadiusRangeMinimum)
            dumpRadiusRangeMinimumAsElement(aRadiusRangeMinimum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"RadiusRangeMaximum"_ustr);
        drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
        if(anotherAny >>= aRadiusRangeMaximum)
            dumpRadiusRangeMaximumAsElement(aRadiusRangeMaximum);
    }
}

void EnhancedShapeDumper::dumpSwitchedAsAttribute(bool bSwitched)
{
    if(bSwitched)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("switched"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("switched"), "%s", "false");
}

void EnhancedShapeDumper::dumpPositionAsElement(const drawing::EnhancedCustomShapeParameterPair& aPosition)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Position" ));
    dumpEnhancedCustomShapeParameterPair(aPosition);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpPolarAsElement(const drawing::EnhancedCustomShapeParameterPair& aPolar)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Polar" ));
    dumpEnhancedCustomShapeParameterPair(aPolar);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRefXAsAttribute(sal_Int32 aRefX)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refX"), "%" SAL_PRIdINT32, aRefX);
}

void EnhancedShapeDumper::dumpRefYAsAttribute(sal_Int32 aRefY)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refY"), "%" SAL_PRIdINT32, aRefY);
}

void EnhancedShapeDumper::dumpRefAngleAsAttribute(sal_Int32 aRefAngle)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refAngle"), "%" SAL_PRIdINT32, aRefAngle);
}

void EnhancedShapeDumper::dumpRefRAsAttribute(sal_Int32 aRefR)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refR"), "%" SAL_PRIdINT32, aRefR);
}

void EnhancedShapeDumper::dumpEnhancedCustomShapeParameter(
    const drawing::EnhancedCustomShapeParameter& aParameter)
{
    uno::Any aAny = aParameter.Value;
    OUString sValue;
    float fValue;
    sal_Int32 nValue;
    bool bValue;
    if(aAny >>= sValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
            OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
    }
    else if(aAny >>= nValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%" SAL_PRIdINT32, nValue);
    }
    else if(aAny >>= fValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%f", fValue);
    }
    else if(aAny >>= bValue)
    {
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s", (bValue? "true": "false"));
    }
    sal_Int32 aType = aParameter.Type;
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%" SAL_PRIdINT32, aType);
}

void EnhancedShapeDumper::dumpRangeXMinimumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeXMinimum)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeXMinimum" ));
    dumpEnhancedCustomShapeParameter(aRangeXMinimum);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRangeXMaximumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeXMaximum)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeXMaximum" ));
    dumpEnhancedCustomShapeParameter(aRangeXMaximum);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRangeYMinimumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeYMinimum)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeYMinimum" ));
    dumpEnhancedCustomShapeParameter(aRangeYMinimum);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRangeYMaximumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeYMaximum)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeYMaximum" ));
    dumpEnhancedCustomShapeParameter(aRangeYMaximum);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRadiusRangeMinimumAsElement(const drawing::EnhancedCustomShapeParameter& aRadiusRangeMinimum)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RadiusRangeMinimum" ));
    dumpEnhancedCustomShapeParameter(aRadiusRangeMinimum);
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRadiusRangeMaximumAsElement(const drawing::EnhancedCustomShapeParameter& aRadiusRangeMaximum)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RadiusRangeMaximum" ));
    dumpEnhancedCustomShapeParameter(aRadiusRangeMaximum);
    (void)xmlTextWriterEndElement( xmlWriter );
}


// ---------- EnhancedCustomShapePath.idl ---------------


void EnhancedShapeDumper::dumpEnhancedCustomShapePathService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Coordinates"_ustr);
        uno::Sequence< drawing::EnhancedCustomShapeParameterPair > aCoordinates;
        if(anotherAny >>= aCoordinates)
            dumpCoordinatesAsElement(aCoordinates);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"Segments"_ustr);
        uno::Sequence< drawing::EnhancedCustomShapeSegment > aSegments;
        if(anotherAny >>= aSegments)
            dumpSegmentsAsElement(aSegments);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"StretchX"_ustr);
        sal_Int32 aStretchX = sal_Int32();
        if(anotherAny >>= aStretchX)
            dumpStretchXAsAttribute(aStretchX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"StretchY"_ustr);
        sal_Int32 aStretchY = sal_Int32();
        if(anotherAny >>= aStretchY)
            dumpStretchYAsAttribute(aStretchY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"TextFrames"_ustr);
        uno::Sequence< drawing::EnhancedCustomShapeTextFrame > aTextFrames;
        if(anotherAny >>= aTextFrames)
            dumpTextFramesAsElement(aTextFrames);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"GluePoints"_ustr);
        uno::Sequence< drawing::EnhancedCustomShapeParameterPair > aGluePoints;
        if(anotherAny >>= aGluePoints)
            dumpGluePointsAsElement(aGluePoints);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"GluePointLeavingDirections"_ustr);
        uno::Sequence< double > aGluePointLeavingDirections;
        if(anotherAny >>= aGluePointLeavingDirections)
            dumpGluePointLeavingDirectionsAsElement(aGluePointLeavingDirections);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"GluePointType"_ustr);
        sal_Int32 aGluePointType = sal_Int32();
        if(anotherAny >>= aGluePointType)
            dumpGluePointTypeAsAttribute(aGluePointType);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ExtrusionAllowed"_ustr);
        bool bExtrusionAllowed;
        if(anotherAny >>= bExtrusionAllowed)
            dumpExtrusionAllowedAsAttribute(bExtrusionAllowed);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ConcentricGradientFillAllowed"_ustr);
        bool bConcentricGradientFillAllowed;
        if(anotherAny >>= bConcentricGradientFillAllowed)
            dumpConcentricGradientFillAllowedAsAttribute(bConcentricGradientFillAllowed);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"TextPathAllowed"_ustr);
        bool bTextPathAllowed;
        if(anotherAny >>= bTextPathAllowed)
            dumpTextPathAllowedAsAttribute(bTextPathAllowed);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"SubViewSize"_ustr);
        uno::Sequence< awt::Size > aSubViewSize;
        if(anotherAny >>= aSubViewSize)
            dumpSubViewSizeAsElement(aSubViewSize);
    }
}

void EnhancedShapeDumper::dumpCoordinatesAsElement(const uno::Sequence< drawing::EnhancedCustomShapeParameterPair >& aCoordinates)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Coordinates" ));
    for (const auto& i : aCoordinates)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeParameterPair" ));
        dumpEnhancedCustomShapeParameterPair(i);
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpSegmentsAsElement(const uno::Sequence< drawing::EnhancedCustomShapeSegment >& aSegments)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Segments" ));
    for (const auto& i : aSegments)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeSegment" ));
        sal_Int32 aCommand = i.Command;
        sal_Int32 aCount = i.Count;
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("command"), "%" SAL_PRIdINT32, aCommand);
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("count"), "%" SAL_PRIdINT32, aCount);
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpStretchXAsAttribute(sal_Int32 aStretchX)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stretchX"), "%" SAL_PRIdINT32, aStretchX);
}

void EnhancedShapeDumper::dumpStretchYAsAttribute(sal_Int32 aStretchY)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stretchY"), "%" SAL_PRIdINT32, aStretchY);
}

void EnhancedShapeDumper::dumpTextFramesAsElement(const uno::Sequence< drawing::EnhancedCustomShapeTextFrame >& aTextFrames)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "TextFrames" ));
    for (const auto& i : aTextFrames)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeTextFrame" ));
        {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "TopLeft" ));
        dumpEnhancedCustomShapeParameterPair(i.TopLeft);
        (void)xmlTextWriterEndElement( xmlWriter );

        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "BottomRight" ));
        dumpEnhancedCustomShapeParameterPair(i.BottomRight);
        (void)xmlTextWriterEndElement( xmlWriter );
        }
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpGluePointsAsElement(const uno::Sequence< drawing::EnhancedCustomShapeParameterPair >& aGluePoints)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "GluePoints" ));
    for (const auto& i : aGluePoints)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeParameterPair" ));
        dumpEnhancedCustomShapeParameterPair(i);
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpGluePointLeavingDirectionsAsElement(const uno::Sequence< double >& aGluePointLeavingDirections)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "GluePointLeavingDirections" ));
    for (const auto& i : aGluePointLeavingDirections)
    {
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("value"), "%f", i);
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpGluePointTypeAsAttribute(sal_Int32 aGluePointType)
{
    (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("gluePointType"), "%" SAL_PRIdINT32, aGluePointType);
}

void EnhancedShapeDumper::dumpExtrusionAllowedAsAttribute(bool bExtrusionAllowed)
{
    if(bExtrusionAllowed)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionAllowed"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionAllowed"), "%s", "false");
}

void EnhancedShapeDumper::dumpConcentricGradientFillAllowedAsAttribute(bool bConcentricGradientFillAllowed)
{
    if(bConcentricGradientFillAllowed)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("concentricGradientFillAllowed"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("concentricGradientFillAllowed"), "%s", "false");
}

void EnhancedShapeDumper::dumpTextPathAllowedAsAttribute(bool bTextPathAllowed)
{
    if(bTextPathAllowed)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathAllowed"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathAllowed"), "%s", "false");
}

void EnhancedShapeDumper::dumpSubViewSizeAsElement(const uno::Sequence< awt::Size >& aSubViewSize)
{
    (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "SubViewSize" ));
    for (const auto& i : aSubViewSize)
    {
        (void)xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Size" ));
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, i.Width);
        (void)xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, i.Height);
        (void)xmlTextWriterEndElement( xmlWriter );
    }
    (void)xmlTextWriterEndElement( xmlWriter );
}


// ---------- EnhancedCustomShapeTextPath.idl ---------------


void EnhancedShapeDumper::dumpEnhancedCustomShapeTextPathService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"TextPath"_ustr);
        bool bTextPath;
        if(anotherAny >>= bTextPath)
            dumpTextPathAsAttribute(bTextPath);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"TextPathMode"_ustr);
        drawing::EnhancedCustomShapeTextPathMode eTextPathMode;
        if(anotherAny >>= eTextPathMode)
            dumpTextPathModeAsAttribute(eTextPathMode);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue(u"ScaleX"_ustr);
        bool bScaleX;
        if(anotherAny >>= bScaleX)
            dumpScaleXAsAttribute(bScaleX);
    }
}

void EnhancedShapeDumper::dumpTextPathAsAttribute(bool bTextPath)
{
    if(bTextPath)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPath"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPath"), "%s", "false");
}

void EnhancedShapeDumper::dumpTextPathModeAsAttribute(drawing::EnhancedCustomShapeTextPathMode eTextPathMode)
{
    switch(eTextPathMode)
    {
        case drawing::EnhancedCustomShapeTextPathMode_NORMAL:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathMode"), "%s", "NORMAL");
            break;
        case drawing::EnhancedCustomShapeTextPathMode_PATH:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathMode"), "%s", "PATH");
            break;
        case drawing::EnhancedCustomShapeTextPathMode_SHAPE:
            (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathMode"), "%s", "SHAPE");
            break;
        default:
            break;
    }
}

void EnhancedShapeDumper::dumpScaleXAsAttribute(bool bScaleX)
{
    if(bScaleX)
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("scaleX"), "%s", "true");
    else
        (void)xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("scaleX"), "%s", "false");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
