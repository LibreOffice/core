/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <EnhancedShapeDumper.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;


// ---------- EnhancedCustomShapeExtrusion.idl ----------


void EnhancedShapeDumper::dumpEnhancedCustomShapeExtrusionService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Extrusion");
        bool bExtrusion;
        if(anotherAny >>= bExtrusion)
            dumpExtrusionAsAttribute(bExtrusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Brightness");
        double aBrightness = double();
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
        double aDiffusion = double();
        if(anotherAny >>= aDiffusion)
            dumpDiffusionAsAttribute(aDiffusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("NumberOfLineSegments");
        sal_Int32 aNumberOfLineSegments = sal_Int32();
        if(anotherAny >>= aNumberOfLineSegments)
            dumpNumberOfLineSegmentsAsAttribute(aNumberOfLineSegments);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("LightFace");
        bool bLightFace;
        if(anotherAny >>= bLightFace)
            dumpLightFaceAsAttribute(bLightFace);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FirstLightHarsh");
        bool bFirstLightHarsh;
        if(anotherAny >>= bFirstLightHarsh)
            dumpFirstLightHarshAsAttribute(bFirstLightHarsh);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SecondLightHarsh");
        bool bSecondLightHarsh;
        if(anotherAny >>= bSecondLightHarsh)
            dumpSecondLightHarshAsAttribute(bSecondLightHarsh);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("FirstLightLevel");
        double aFirstLightLevel = double();
        if(anotherAny >>= aFirstLightLevel)
            dumpFirstLightLevelAsAttribute(aFirstLightLevel);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SecondLightLevel");
        double aSecondLightLevel = double();
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
        bool bMetal;
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
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Shininess");
        double aShininess = double();
        if(anotherAny >>= aShininess)
            dumpShininessAsAttribute(aShininess);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Skew");
        drawing::EnhancedCustomShapeParameterPair aSkew;
        if(anotherAny >>= aSkew)
            dumpSkewAsElement(aSkew);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Specularity");
        double aSpecularity = double();
        if(anotherAny >>= aSpecularity)
            dumpSpecularityAsAttribute(aSpecularity);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ProjectionMode");
        drawing::ProjectionMode eProjectionMode;
        if(anotherAny >>= eProjectionMode)
            dumpProjectionModeAsAttribute(eProjectionMode);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ViewPoint");
        drawing::Position3D aViewPoint;
        if(anotherAny >>= aViewPoint)
            dumpViewPointAsElement(aViewPoint);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Origin");
        drawing::EnhancedCustomShapeParameterPair aOrigin;
        if(anotherAny >>= aOrigin)
            dumpOriginAsElement(aOrigin);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ExtrusionColor");
        bool bExtrusionColor;
        if(anotherAny >>= bExtrusionColor)
            dumpExtrusionColorAsAttribute(bExtrusionColor);
    }
}
void EnhancedShapeDumper::dumpExtrusionAsAttribute(bool bExtrusion)
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
        dumpEnhancedCustomShapeParameter(aParameterPair.First);
        xmlTextWriterEndElement( xmlWriter );
    }
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Second" ));
        dumpEnhancedCustomShapeParameter(aParameterPair.Second);
        xmlTextWriterEndElement( xmlWriter );
    }
}

void EnhancedShapeDumper::dumpDepthAsElement(const drawing::EnhancedCustomShapeParameterPair& aDepth)
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

void EnhancedShapeDumper::dumpLightFaceAsAttribute(bool bLightFace)
{
    if(bLightFace)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lightFace"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("lightFace"), "%s", "false");
}

void EnhancedShapeDumper::dumpFirstLightHarshAsAttribute(bool bFirstLightHarsh)
{
    if(bFirstLightHarsh)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightHarsh"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("firstLightHarsh"), "%s", "false");
}

void EnhancedShapeDumper::dumpSecondLightHarshAsAttribute(bool bSecondLightHarsh)
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

void EnhancedShapeDumper::dumpMetalAsAttribute(bool bMetal)
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

void EnhancedShapeDumper::dumpRotateAngleAsElement(const drawing::EnhancedCustomShapeParameterPair& aRotateAngle)
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

void EnhancedShapeDumper::dumpShininessAsAttribute(double aShininess)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("shininess"), "%f", aShininess);
}

void EnhancedShapeDumper::dumpSkewAsElement(const drawing::EnhancedCustomShapeParameterPair& aSkew)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Skew" ));
    dumpEnhancedCustomShapeParameterPair(aSkew);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpSpecularityAsAttribute(double aSpecularity)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("specularity"), "%f", aSpecularity);
}

void EnhancedShapeDumper::dumpProjectionModeAsAttribute(drawing::ProjectionMode eProjectionMode)
{
    switch(eProjectionMode)
    {
        case drawing::ProjectionMode_PARALLEL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("projectionMode"), "%s", "PARALLEL");
            break;
        case drawing::ProjectionMode_PERSPECTIVE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("projectionMode"), "%s", "PERSPECTIVE");
            break;
        default:
            break;
    }
}

void EnhancedShapeDumper::dumpViewPointAsElement(drawing::Position3D aViewPoint)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ViewPoint" ));
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("positionX"), "%f", aViewPoint.PositionX);
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("positionY"), "%f", aViewPoint.PositionY);
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("positionZ"), "%f", aViewPoint.PositionZ);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpOriginAsElement(const drawing::EnhancedCustomShapeParameterPair& aOrigin)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Origin" ));
    dumpEnhancedCustomShapeParameterPair(aOrigin);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpExtrusionColorAsAttribute(bool bExtrusionColor)
{
    if(bExtrusionColor)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionColor"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionColor"), "%s", "false");
}


// ---------- EnhancedCustomShapeGeometry.idl -----------


void EnhancedShapeDumper::dumpEnhancedCustomShapeGeometryService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Type");
        OUString sType;
        if(anotherAny >>= sType)
            dumpTypeAsAttribute(sType);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ViewBox");
        awt::Rectangle aViewBox;
        if(anotherAny >>= aViewBox)
            dumpViewBoxAsElement(aViewBox);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("MirroredX");
        bool bMirroredX;
        if(anotherAny >>= bMirroredX)
            dumpMirroredXAsAttribute(bMirroredX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("MirroredY");
        bool bMirroredY;
        if(anotherAny >>= bMirroredY)
            dumpMirroredYAsAttribute(bMirroredY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextRotateAngle");
        double aTextRotateAngle = double();
        if(anotherAny >>= aTextRotateAngle)
            dumpTextRotateAngleAsAttribute(aTextRotateAngle);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("AdjustmentValues");
        uno::Sequence< drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentValues;
        if(anotherAny >>= aAdjustmentValues)
            dumpAdjustmentValuesAsElement(aAdjustmentValues);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Extrusion");
        uno::Sequence< beans::PropertyValue > aExtrusion;
        if(anotherAny >>= aExtrusion)
            dumpExtrusionAsElement(aExtrusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Path");
        uno::Sequence< beans::PropertyValue > aPath;
        if(anotherAny >>= aPath)
            dumpPathAsElement(aPath);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextPath");
        uno::Sequence< beans::PropertyValue > aTextPath;
        if(anotherAny >>= aTextPath)
            dumpTextPathAsElement(aTextPath);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Equations");
        uno::Sequence< OUString > aEquations;
        if(anotherAny >>= aEquations)
            dumpEquationsAsElement(aEquations);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Handles");
        uno::Sequence< beans::PropertyValues > aHandles;
        if(anotherAny >>= aHandles)
            dumpHandlesAsElement(aHandles);
    }
}
void EnhancedShapeDumper::dumpTypeAsAttribute(const OUString& sType)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s",
        OUStringToOString(sType, RTL_TEXTENCODING_UTF8).getStr());
}

void EnhancedShapeDumper::dumpViewBoxAsElement(awt::Rectangle aViewBox)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ViewBox" ));
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("x"), "%" SAL_PRIdINT32, aViewBox.X);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("y"), "%" SAL_PRIdINT32, aViewBox.Y);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, aViewBox.Width);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, aViewBox.Height);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpMirroredXAsAttribute(bool bMirroredX)
{
    if(bMirroredX)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredX"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredX"), "%s", "false");
}

void EnhancedShapeDumper::dumpMirroredYAsAttribute(bool bMirroredY)
{
    if(bMirroredY)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredY"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("mirroredY"), "%s", "false");
}

void EnhancedShapeDumper::dumpTextRotateAngleAsAttribute(double aTextRotateAngle)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textRotateAngle"), "%f", aTextRotateAngle);
}

void EnhancedShapeDumper::dumpAdjustmentValuesAsElement(const uno::Sequence< drawing::EnhancedCustomShapeAdjustmentValue>& aAdjustmentValues)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "AdjustmentValues" ));
    sal_Int32 nLength = aAdjustmentValues.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeAdjustmentValue" ));
        uno::Any aAny = aAdjustmentValues[i].Value;
        OUString sValue;
        float fValue;
        sal_Int32 nValue;
        bool bValue;
        if(aAny >>= sValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
                OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
        }
        else if(aAny >>= nValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%" SAL_PRIdINT32, nValue);
        }
        else if(aAny >>= fValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%f", fValue);
        }
        else if(aAny >>= bValue)
        {
            xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s", (bValue? "true": "false"));
        }

        switch(aAdjustmentValues[i].State)
        {
            case beans::PropertyState_DIRECT_VALUE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DIRECT_VALUE");
                break;
            case beans::PropertyState_DEFAULT_VALUE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DEFAULT_VALUE");
                break;
            case beans::PropertyState_AMBIGUOUS_VALUE:
                xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "AMBIGUOUS_VALUE");
                break;
            default:
                break;
        }
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpPropertyValueAsElement(beans::PropertyValue aPropertyValue)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PropertyValue" ));

    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
        OUStringToOString(aPropertyValue.Name, RTL_TEXTENCODING_UTF8).getStr());
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("handle"), "%" SAL_PRIdINT32, aPropertyValue.Handle);

    uno::Any aAny = aPropertyValue.Value;
    OUString sValue;
    if(aAny >>= sValue)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
            OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
    }
    switch(aPropertyValue.State)
    {
        case beans::PropertyState_DIRECT_VALUE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DIRECT_VALUE");
            break;
        case beans::PropertyState_DEFAULT_VALUE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "DEFAULT_VALUE");
            break;
        case beans::PropertyState_AMBIGUOUS_VALUE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("propertyState"), "%s", "AMBIGUOUS_VALUE");
            break;
        default:
            break;
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpExtrusionAsElement(const uno::Sequence< beans::PropertyValue >& aExtrusion)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Extrusion" ));
    sal_Int32 nLength = aExtrusion.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        dumpPropertyValueAsElement(aExtrusion[i]);
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpPathAsElement(const uno::Sequence< beans::PropertyValue >& aPath)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Path" ));
    sal_Int32 nLength = aPath.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        dumpPropertyValueAsElement(aPath[i]);
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpTextPathAsElement(const uno::Sequence< beans::PropertyValue >& aTextPath)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "TextPath" ));
    sal_Int32 nLength = aTextPath.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        dumpPropertyValueAsElement(aTextPath[i]);
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpEquationsAsElement(const uno::Sequence< OUString >& aEquations)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Equations" ));
    sal_Int32 nLength = aEquations.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("name"), "%s",
            OUStringToOString(aEquations[i], RTL_TEXTENCODING_UTF8).getStr());
    }
    xmlTextWriterEndElement( xmlWriter );
}

// PropertyValues specifies a sequence of PropertyValue instances.
// so in this case it's a Sequence of a Sequence of a PropertyValue instances.
// Welcome to Sequenception again.
void EnhancedShapeDumper::dumpHandlesAsElement(const uno::Sequence< beans::PropertyValues >& aHandles)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Handles" ));
    sal_Int32 nSequenceLength = aHandles.getLength();
    for (sal_Int32 i = 0; i < nSequenceLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "PropertyValues" ));
        uno::Sequence< beans::PropertyValue > propertyValueSequence = aHandles[i];
        sal_Int32 nLength = propertyValueSequence.getLength();
        for (sal_Int32 j = 0; j < nLength; ++j)
        {
            dumpPropertyValueAsElement(propertyValueSequence[j]);
        }
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}


// ---------- EnhancedCustomShapeHandle.idl -----------


void EnhancedShapeDumper::dumpEnhancedCustomShapeHandleService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("MirroredX");
        bool bMirroredX;
        if(anotherAny >>= bMirroredX)
            dumpMirroredXAsAttribute(bMirroredX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("MirroredY");
        bool bMirroredY;
        if(anotherAny >>= bMirroredY)
            dumpMirroredYAsAttribute(bMirroredY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Switched");
        bool bSwitched;
        if(anotherAny >>= bSwitched)
            dumpSwitchedAsAttribute(bSwitched);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Position");
        drawing::EnhancedCustomShapeParameterPair aPosition;
        if(anotherAny >>= aPosition)
            dumpPositionAsElement(aPosition);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Polar");
        drawing::EnhancedCustomShapeParameterPair aPolar;
        if(anotherAny >>= aPolar)
            dumpPolarAsElement(aPolar);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RefX");
        sal_Int32 aRefX = sal_Int32();
        if(anotherAny >>= aRefX)
            dumpRefXAsAttribute(aRefX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RefY");
        sal_Int32 aRefY = sal_Int32();
        if(anotherAny >>= aRefY)
            dumpRefYAsAttribute(aRefY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RefAngle");
        sal_Int32 aRefAngle = sal_Int32();
        if(anotherAny >>= aRefAngle)
            dumpRefAngleAsAttribute(aRefAngle);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RefR");
        sal_Int32 aRefR = sal_Int32();
        if(anotherAny >>= aRefR)
            dumpRefRAsAttribute(aRefR);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RangeXMinimum");
        drawing::EnhancedCustomShapeParameter aRangeXMinimum;
        if(anotherAny >>= aRangeXMinimum)
            dumpRangeXMinimumAsElement(aRangeXMinimum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RangeXMaximum");
        drawing::EnhancedCustomShapeParameter aRangeXMaximum;
        if(anotherAny >>= aRangeXMaximum)
            dumpRangeXMaximumAsElement(aRangeXMaximum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RangeYMinimum");
        drawing::EnhancedCustomShapeParameter aRangeYMinimum;
        if(anotherAny >>= aRangeYMinimum)
            dumpRangeYMinimumAsElement(aRangeYMinimum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RangeYMaximum");
        drawing::EnhancedCustomShapeParameter aRangeYMaximum;
        if(anotherAny >>= aRangeYMaximum)
            dumpRangeYMaximumAsElement(aRangeYMaximum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RadiusRangeMinimum");
        drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
        if(anotherAny >>= aRadiusRangeMinimum)
            dumpRadiusRangeMinimumAsElement(aRadiusRangeMinimum);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("RadiusRangeMaximum");
        drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
        if(anotherAny >>= aRadiusRangeMaximum)
            dumpRadiusRangeMaximumAsElement(aRadiusRangeMaximum);
    }
}

void EnhancedShapeDumper::dumpSwitchedAsAttribute(bool bSwitched)
{
    if(bSwitched)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("switched"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("switched"), "%s", "false");
}

void EnhancedShapeDumper::dumpPositionAsElement(const drawing::EnhancedCustomShapeParameterPair& aPosition)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Position" ));
    dumpEnhancedCustomShapeParameterPair(aPosition);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpPolarAsElement(const drawing::EnhancedCustomShapeParameterPair& aPolar)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Polar" ));
    dumpEnhancedCustomShapeParameterPair(aPolar);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRefXAsAttribute(sal_Int32 aRefX)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refX"), "%" SAL_PRIdINT32, aRefX);
}

void EnhancedShapeDumper::dumpRefYAsAttribute(sal_Int32 aRefY)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refY"), "%" SAL_PRIdINT32, aRefY);
}

void EnhancedShapeDumper::dumpRefAngleAsAttribute(sal_Int32 aRefAngle)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refAngle"), "%" SAL_PRIdINT32, aRefAngle);
}

void EnhancedShapeDumper::dumpRefRAsAttribute(sal_Int32 aRefR)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("refR"), "%" SAL_PRIdINT32, aRefR);
}

void EnhancedShapeDumper::dumpEnhancedCustomShapeParameter(drawing::EnhancedCustomShapeParameter aParameter)
{
    uno::Any aAny = aParameter.Value;
    OUString sValue;
    float fValue;
    sal_Int32 nValue;
    bool bValue;
    if(aAny >>= sValue)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s",
            OUStringToOString(sValue, RTL_TEXTENCODING_UTF8).getStr());
    }
    else if(aAny >>= nValue)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%" SAL_PRIdINT32, nValue);
    }
    else if(aAny >>= fValue)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%f", fValue);
    }
    else if(aAny >>= bValue)
    {
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("value"), "%s", (bValue? "true": "false"));
    }
    sal_Int32 aType = aParameter.Type;
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%" SAL_PRIdINT32, aType);
}

void EnhancedShapeDumper::dumpRangeXMinimumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeXMinimum)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeXMinimum" ));
    dumpEnhancedCustomShapeParameter(aRangeXMinimum);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRangeXMaximumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeXMaximum)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeXMaximum" ));
    dumpEnhancedCustomShapeParameter(aRangeXMaximum);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRangeYMinimumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeYMinimum)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeYMinimum" ));
    dumpEnhancedCustomShapeParameter(aRangeYMinimum);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRangeYMaximumAsElement(const drawing::EnhancedCustomShapeParameter& aRangeYMaximum)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RangeYMaximum" ));
    dumpEnhancedCustomShapeParameter(aRangeYMaximum);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRadiusRangeMinimumAsElement(const drawing::EnhancedCustomShapeParameter& aRadiusRangeMinimum)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RadiusRangeMinimum" ));
    dumpEnhancedCustomShapeParameter(aRadiusRangeMinimum);
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpRadiusRangeMaximumAsElement(const drawing::EnhancedCustomShapeParameter& aRadiusRangeMaximum)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "RadiusRangeMaximum" ));
    dumpEnhancedCustomShapeParameter(aRadiusRangeMaximum);
    xmlTextWriterEndElement( xmlWriter );
}


// ---------- EnhancedCustomShapePath.idl ---------------


void EnhancedShapeDumper::dumpEnhancedCustomShapePathService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Coordinates");
        uno::Sequence< drawing::EnhancedCustomShapeParameterPair > aCoordinates;
        if(anotherAny >>= aCoordinates)
            dumpCoordinatesAsElement(aCoordinates);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Segments");
        uno::Sequence< drawing::EnhancedCustomShapeSegment > aSegments;
        if(anotherAny >>= aSegments)
            dumpSegmentsAsElement(aSegments);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("StretchX");
        sal_Int32 aStretchX = sal_Int32();
        if(anotherAny >>= aStretchX)
            dumpStretchXAsAttribute(aStretchX);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("StretchY");
        sal_Int32 aStretchY = sal_Int32();
        if(anotherAny >>= aStretchY)
            dumpStretchYAsAttribute(aStretchY);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextFrames");
        uno::Sequence< drawing::EnhancedCustomShapeTextFrame > aTextFrames;
        if(anotherAny >>= aTextFrames)
            dumpTextFramesAsElement(aTextFrames);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("GluePoints");
        uno::Sequence< drawing::EnhancedCustomShapeParameterPair > aGluePoints;
        if(anotherAny >>= aGluePoints)
            dumpGluePointsAsElement(aGluePoints);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("GluePointLeavingDirections");
        uno::Sequence< double > aGluePointLeavingDirections;
        if(anotherAny >>= aGluePointLeavingDirections)
            dumpGluePointLeavingDirectionsAsElement(aGluePointLeavingDirections);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("GluePointType");
        sal_Int32 aGluePointType = sal_Int32();
        if(anotherAny >>= aGluePointType)
            dumpGluePointTypeAsAttribute(aGluePointType);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ExtrusionAllowed");
        bool bExtrusionAllowed;
        if(anotherAny >>= bExtrusionAllowed)
            dumpExtrusionAllowedAsAttribute(bExtrusionAllowed);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ConcentricGradientFillAllowed");
        bool bConcentricGradientFillAllowed;
        if(anotherAny >>= bConcentricGradientFillAllowed)
            dumpConcentricGradientFillAllowedAsAttribute(bConcentricGradientFillAllowed);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextPathAllowed");
        bool bTextPathAllowed;
        if(anotherAny >>= bTextPathAllowed)
            dumpTextPathAllowedAsAttribute(bTextPathAllowed);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("SubViewSize");
        uno::Sequence< awt::Size > aSubViewSize;
        if(anotherAny >>= aSubViewSize)
            dumpSubViewSizeAsElement(aSubViewSize);
    }
}

void EnhancedShapeDumper::dumpCoordinatesAsElement(const uno::Sequence< drawing::EnhancedCustomShapeParameterPair >& aCoordinates)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Coordinates" ));
    sal_Int32 nLength = aCoordinates.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeParameterPair" ));
        dumpEnhancedCustomShapeParameterPair(aCoordinates[i]);
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpSegmentsAsElement(const uno::Sequence< drawing::EnhancedCustomShapeSegment >& aSegments)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Segments" ));
    sal_Int32 nLength = aSegments.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeSegment" ));
        sal_Int32 aCommand = aSegments[i].Command;
        sal_Int32 aCount = aSegments[i].Count;
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("command"), "%" SAL_PRIdINT32, aCommand);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("count"), "%" SAL_PRIdINT32, aCount);
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpStretchXAsAttribute(sal_Int32 aStretchX)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stretchX"), "%" SAL_PRIdINT32, aStretchX);
}

void EnhancedShapeDumper::dumpStretchYAsAttribute(sal_Int32 aStretchY)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("stretchY"), "%" SAL_PRIdINT32, aStretchY);
}

void EnhancedShapeDumper::dumpTextFramesAsElement(const uno::Sequence< drawing::EnhancedCustomShapeTextFrame >& aTextFrames)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "TextFrames" ));
    sal_Int32 nLength = aTextFrames.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeTextFrame" ));
        {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "TopLeft" ));
        dumpEnhancedCustomShapeParameterPair(aTextFrames[i].TopLeft);
        xmlTextWriterEndElement( xmlWriter );

        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "BottomRight" ));
        dumpEnhancedCustomShapeParameterPair(aTextFrames[i].BottomRight);
        xmlTextWriterEndElement( xmlWriter );
        }
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpGluePointsAsElement(const uno::Sequence< drawing::EnhancedCustomShapeParameterPair >& aGluePoints)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "GluePoints" ));
    sal_Int32 nLength = aGluePoints.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "EnhancedCustomShapeParameterPair" ));
        dumpEnhancedCustomShapeParameterPair(aGluePoints[i]);
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpGluePointLeavingDirectionsAsElement(const uno::Sequence< double >& aGluePointLeavingDirections)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "GluePointLeavingDirections" ));
    sal_Int32 nLength = aGluePointLeavingDirections.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("value"), "%f", aGluePointLeavingDirections[i]);
    }
    xmlTextWriterEndElement( xmlWriter );
}

void EnhancedShapeDumper::dumpGluePointTypeAsAttribute(sal_Int32 aGluePointType)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("gluePointType"), "%" SAL_PRIdINT32, aGluePointType);
}

void EnhancedShapeDumper::dumpExtrusionAllowedAsAttribute(bool bExtrusionAllowed)
{
    if(bExtrusionAllowed)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionAllowed"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusionAllowed"), "%s", "false");
}

void EnhancedShapeDumper::dumpConcentricGradientFillAllowedAsAttribute(bool bConcentricGradientFillAllowed)
{
    if(bConcentricGradientFillAllowed)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("concentricGradientFillAllowed"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("concentricGradientFillAllowed"), "%s", "false");
}

void EnhancedShapeDumper::dumpTextPathAllowedAsAttribute(bool bTextPathAllowed)
{
    if(bTextPathAllowed)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathAllowed"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathAllowed"), "%s", "false");
}

void EnhancedShapeDumper::dumpSubViewSizeAsElement(const uno::Sequence< awt::Size >& aSubViewSize)
{
    xmlTextWriterStartElement(xmlWriter, BAD_CAST( "SubViewSize" ));
    sal_Int32 nLength = aSubViewSize.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "Size" ));
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("width"), "%" SAL_PRIdINT32, aSubViewSize[i].Width);
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("height"), "%" SAL_PRIdINT32, aSubViewSize[i].Height);
        xmlTextWriterEndElement( xmlWriter );
    }
    xmlTextWriterEndElement( xmlWriter );
}


// ---------- EnhancedCustomShapeTextPath.idl ---------------


void EnhancedShapeDumper::dumpEnhancedCustomShapeTextPathService(const uno::Reference< beans::XPropertySet >& xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextPath");
        bool bTextPath;
        if(anotherAny >>= bTextPath)
            dumpTextPathAsAttribute(bTextPath);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("TextPathMode");
        drawing::EnhancedCustomShapeTextPathMode eTextPathMode;
        if(anotherAny >>= eTextPathMode)
            dumpTextPathModeAsAttribute(eTextPathMode);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("ScaleX");
        bool bScaleX;
        if(anotherAny >>= bScaleX)
            dumpScaleXAsAttribute(bScaleX);
    }
}

void EnhancedShapeDumper::dumpTextPathAsAttribute(bool bTextPath)
{
    if(bTextPath)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPath"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPath"), "%s", "false");
}

void EnhancedShapeDumper::dumpTextPathModeAsAttribute(drawing::EnhancedCustomShapeTextPathMode eTextPathMode)
{
    switch(eTextPathMode)
    {
        case drawing::EnhancedCustomShapeTextPathMode_NORMAL:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathMode"), "%s", "NORMAL");
            break;
        case drawing::EnhancedCustomShapeTextPathMode_PATH:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathMode"), "%s", "PATH");
            break;
        case drawing::EnhancedCustomShapeTextPathMode_SHAPE:
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("textPathMode"), "%s", "SHAPE");
            break;
        default:
            break;
    }
}

void EnhancedShapeDumper::dumpScaleXAsAttribute(bool bScaleX)
{
    if(bScaleX)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("scaleX"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("scaleX"), "%s", "false");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
