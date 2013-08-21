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

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <tools/helpers.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/shapeexport.hxx>
#include "sdpropls.hxx"
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include "xexptran.hxx"
#include <xmloff/xmltoken.hxx>
#include <basegfx/vector/b3dvector.hxx>

#include "xmloff/xmlnmspe.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

void XMLShapeExport::ImpExport3DSceneShape( const uno::Reference< drawing::XShape >& xShape, XmlShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY);
    if(xShapes.is() && xShapes->getCount())
    {
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
        DBG_ASSERT( xPropSet.is(), "XMLShapeExport::ImpExport3DSceneShape can't export a scene without a propertyset" );
        if( xPropSet.is() )
        {
            // Transformation
            ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

            // 3d attributes
            export3DSceneAttributes( xPropSet );

            // write 3DScene shape
            sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
            SvXMLElementExport aOBJ( mrExport, XML_NAMESPACE_DR3D, XML_SCENE, bCreateNewline, sal_True);

            ImpExportDescription( xShape ); // #i68101#
            ImpExportEvents( xShape );

            // write 3DSceneLights
            export3DLamps( xPropSet );

            // #89764# if export of position is supressed for group shape,
            // positions of contained objects should be written relative to
            // the upper left edge of the group.
            awt::Point aUpperLeft;

            if(!(nFeatures & SEF_EXPORT_POSITION))
            {
                nFeatures |= SEF_EXPORT_POSITION;
                aUpperLeft = xShape->getPosition();
                pRefPoint = &aUpperLeft;
            }

            // write members
            exportShapes( xShapes, nFeatures, pRefPoint );
        }
    }
}

void XMLShapeExport::ImpExport3DShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 /* nFeatures = SEF_DEFAULT */, awt::Point* /*pRefPoint = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUString aStr;
        OUStringBuffer sStringBuffer;

        // transformation (UNO_NAME_3D_TRANSFORM_MATRIX == "D3DTransformMatrix")
        uno::Any aAny = xPropSet->getPropertyValue("D3DTransformMatrix");
        drawing::HomogenMatrix xHomMat;
        aAny >>= xHomMat;
        SdXMLImExTransform3D aTransform;
        aTransform.AddHomogenMatrix(xHomMat);
        if(aTransform.NeedsAction())
            mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));

        switch(eShapeType)
        {
            case XmlShapeTypeDraw3DCubeObject:
            {
                // write 3DCube shape
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_CUBE, sal_True, sal_True);

                // minEdge
                aAny = xPropSet->getPropertyValue("D3DPosition");
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                ::basegfx::B3DVector aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // maxEdge
                aAny = xPropSet->getPropertyValue("D3DSize");
                drawing::Direction3D aDirection3D;
                aAny >>= aDirection3D;
                ::basegfx::B3DVector aDir3D(aDirection3D.DirectionX, aDirection3D.DirectionY, aDirection3D.DirectionZ);

                // transform maxEdge from distance to pos
                aDir3D = aPos3D + aDir3D;

                // write minEdge
                if(aPos3D != ::basegfx::B3DVector(-2500.0, -2500.0, -2500.0)) // write only when not default
                {
                    mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aPos3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_MIN_EDGE, aStr);
                }

                // write maxEdge
                if(aDir3D != ::basegfx::B3DVector(2500.0, 2500.0, 2500.0)) // write only when not default
                {
                    mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_MAX_EDGE, aStr);
                }

                break;
            }
            case XmlShapeTypeDraw3DSphereObject:
            {
                // write 3DSphere shape
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_SPHERE, sal_True, sal_True);

                // Center
                aAny = xPropSet->getPropertyValue("D3DPosition");
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                ::basegfx::B3DVector aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // Size
                aAny = xPropSet->getPropertyValue("D3DSize");
                drawing::Direction3D aDirection3D;
                aAny >>= aDirection3D;
                ::basegfx::B3DVector aDir3D(aDirection3D.DirectionX, aDirection3D.DirectionY, aDirection3D.DirectionZ);

                // write Center
                if(aPos3D != ::basegfx::B3DVector(0.0, 0.0, 0.0)) // write only when not default
                {
                    mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aPos3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_CENTER, aStr);
                }

                // write Size
                if(aDir3D != ::basegfx::B3DVector(5000.0, 5000.0, 5000.0)) // write only when not default
                {
                    mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SIZE, aStr);
                }

                break;
            }
            case XmlShapeTypeDraw3DLatheObject:
            case XmlShapeTypeDraw3DExtrudeObject:
            {
                // write special 3DLathe/3DExtrude attributes
                aAny = xPropSet->getPropertyValue("D3DPolyPolygon3D");
                drawing::PolyPolygonShape3D xPolyPolygon3D;
                aAny >>= xPolyPolygon3D;

                // look for maximal values
                double fXMin = 0;
                double fXMax = 0;
                double fYMin = 0;
                double fYMax = 0;
                sal_Bool bInit(sal_False);
                sal_Int32 nOuterSequenceCount(xPolyPolygon3D.SequenceX.getLength());
                drawing::DoubleSequence* pInnerSequenceX = xPolyPolygon3D.SequenceX.getArray();
                drawing::DoubleSequence* pInnerSequenceY = xPolyPolygon3D.SequenceY.getArray();

                sal_Int32 a;
                for (a = 0; a < nOuterSequenceCount; a++)
                {
                    sal_Int32 nInnerSequenceCount(pInnerSequenceX->getLength());
                    double* pArrayX = pInnerSequenceX->getArray();
                    double* pArrayY = pInnerSequenceY->getArray();

                    for(sal_Int32 b(0L); b < nInnerSequenceCount; b++)
                    {
                        double fX = *pArrayX++;
                        double fY = *pArrayY++;

                        if(bInit)
                        {
                            if(fX > fXMax)
                                fXMax = fX;

                            if(fX < fXMin)
                                fXMin = fX;

                            if(fY > fYMax)
                                fYMax = fY;

                            if(fY < fYMin)
                                fYMin = fY;
                        }
                        else
                        {
                            fXMin = fXMax = fX;
                            fYMin = fYMax = fY;
                            bInit = sal_True;
                        }
                    }

                    pInnerSequenceX++;
                    pInnerSequenceY++;
                }

                // export ViewBox
                awt::Point aMinPoint(FRound(fXMin), FRound(fYMin));
                awt::Size aMaxSize(FRound(fXMax) - aMinPoint.X, FRound(fYMax) - aMinPoint.Y);
                SdXMLImExViewBox aViewBox(
                    aMinPoint.X, aMinPoint.Y, aMaxSize.Width, aMaxSize.Height);
                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX,
                    aViewBox.GetExportString());

                // prepare svx:d element export
                SdXMLImExSvgDElement aSvgDElement(aViewBox, GetExport());
                pInnerSequenceX = xPolyPolygon3D.SequenceX.getArray();
                pInnerSequenceY = xPolyPolygon3D.SequenceY.getArray();

                for (a = 0; a < nOuterSequenceCount; a++)
                {
                    sal_Int32 nInnerSequenceCount(pInnerSequenceX->getLength());
                    double* pArrayX = pInnerSequenceX->getArray();
                    double* pArrayY = pInnerSequenceY->getArray();
                    drawing::PointSequence aPoly(nInnerSequenceCount);
                    awt::Point* pInnerSequence = aPoly.getArray();

                    for(sal_Int32 b(0L); b < nInnerSequenceCount; b++)
                    {
                        double fX = *pArrayX++;
                        double fY = *pArrayY++;

                        *pInnerSequence = awt::Point(FRound(fX), FRound(fY));
                        pInnerSequence++;
                    }

                    // calculate closed flag
                    awt::Point* pFirst = aPoly.getArray();
                    awt::Point* pLast = pFirst + (nInnerSequenceCount - 1);
                    sal_Bool bClosed = (pFirst->X == pLast->X && pFirst->Y == pLast->Y);

                    aSvgDElement.AddPolygon(&aPoly, 0L, aMinPoint,
                        aMaxSize, bClosed);

                    // #80594# corrected error in PolyPolygon3D export for 3D XML
                    pInnerSequenceX++;
                    pInnerSequenceY++;
                }

                // write point array
                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());

                if(eShapeType == XmlShapeTypeDraw3DLatheObject)
                {
                    // write 3DLathe shape
                    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_ROTATE, sal_True, sal_True);
                }
                else
                {
                    // write 3DExtrude shape
                    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_EXTRUDE, sal_True, sal_True);
                }
                break;
            }
            default:
                break;
        }
    }
}

/** helper for chart that adds all attributes of a 3d scene element to the export */
void XMLShapeExport::export3DSceneAttributes( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet )
{
    OUString aStr;
    OUStringBuffer sStringBuffer;

    // world transformation (UNO_NAME_3D_TRANSFORM_MATRIX == "D3DTransformMatrix")
    uno::Any aAny = xPropSet->getPropertyValue("D3DTransformMatrix");
    drawing::HomogenMatrix xHomMat;
    aAny >>= xHomMat;
    SdXMLImExTransform3D aTransform;
    aTransform.AddHomogenMatrix(xHomMat);
    if(aTransform.NeedsAction())
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));

    // VRP, VPN, VUP
    aAny = xPropSet->getPropertyValue("D3DCameraGeometry");
    drawing::CameraGeometry aCamGeo;
    aAny >>= aCamGeo;

    ::basegfx::B3DVector aVRP(aCamGeo.vrp.PositionX, aCamGeo.vrp.PositionY, aCamGeo.vrp.PositionZ);
    if(aVRP != ::basegfx::B3DVector(0.0, 0.0, 1.0)) // write only when not default
    {
        mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aVRP);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VRP, aStr);
    }

    ::basegfx::B3DVector aVPN(aCamGeo.vpn.DirectionX, aCamGeo.vpn.DirectionY, aCamGeo.vpn.DirectionZ);
    if(aVPN != ::basegfx::B3DVector(0.0, 0.0, 1.0)) // write only when not default
    {
        mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aVPN);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VPN, aStr);
    }

    ::basegfx::B3DVector aVUP(aCamGeo.vup.DirectionX, aCamGeo.vup.DirectionY, aCamGeo.vup.DirectionZ);
    if(aVUP != ::basegfx::B3DVector(0.0, 1.0, 0.0)) // write only when not default
    {
        mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aVUP);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VUP, aStr);
    }

    // projection "D3DScenePerspective" drawing::ProjectionMode
    aAny = xPropSet->getPropertyValue("D3DScenePerspective");
    drawing::ProjectionMode xPrjMode;
    aAny >>= xPrjMode;
    if(xPrjMode == drawing::ProjectionMode_PARALLEL)
        aStr = GetXMLToken(XML_PARALLEL);
    else
        aStr = GetXMLToken(XML_PERSPECTIVE);
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_PROJECTION, aStr);

    // distance
    aAny = xPropSet->getPropertyValue("D3DSceneDistance");
    sal_Int32 nDistance = 0;
    aAny >>= nDistance;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            nDistance);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DISTANCE, aStr);

    // focalLength
    aAny = xPropSet->getPropertyValue("D3DSceneFocalLength");
    sal_Int32 nFocalLength = 0;
    aAny >>= nFocalLength;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            nFocalLength);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_FOCAL_LENGTH, aStr);

    // shadowSlant
    aAny = xPropSet->getPropertyValue("D3DSceneShadowSlant");
    sal_Int16 nShadowSlant = 0;
    aAny >>= nShadowSlant;
    ::sax::Converter::convertNumber(sStringBuffer, (sal_Int32)nShadowSlant);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADOW_SLANT, aStr);

    // shadeMode
    aAny = xPropSet->getPropertyValue("D3DSceneShadeMode");
    drawing::ShadeMode xShadeMode;
    if(aAny >>= xShadeMode)
    {
        if(xShadeMode == drawing::ShadeMode_FLAT)
            aStr = GetXMLToken(XML_FLAT);
        else if(xShadeMode == drawing::ShadeMode_PHONG)
            aStr = GetXMLToken(XML_PHONG);
        else if(xShadeMode == drawing::ShadeMode_SMOOTH)
            aStr = GetXMLToken(XML_GOURAUD);
        else
            aStr = GetXMLToken(XML_DRAFT);
    }
    else
    {
        // ShadeMode enum not there, write default
        aStr = GetXMLToken(XML_GOURAUD);
    }
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADE_MODE, aStr);

    // ambientColor
    aAny = xPropSet->getPropertyValue("D3DSceneAmbientColor");
    sal_Int32 nAmbientColor = 0;
    aAny >>= nAmbientColor;
    ::sax::Converter::convertColor(sStringBuffer, nAmbientColor);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_AMBIENT_COLOR, aStr);

    // lightingMode
    aAny = xPropSet->getPropertyValue("D3DSceneTwoSidedLighting");
    sal_Bool bTwoSidedLighting = false;
    aAny >>= bTwoSidedLighting;
    ::sax::Converter::convertBool(sStringBuffer, bTwoSidedLighting);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_LIGHTING_MODE, aStr);
}

/** helper for chart that exports all lamps from the propertyset */
void XMLShapeExport::export3DLamps( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet )
{
    // write lamps 1..8 as content
    OUString aStr;
    OUStringBuffer sStringBuffer;

    const OUString aColorPropName("D3DSceneLightColor");
    const OUString aDirectionPropName("D3DSceneLightDirection");
    const OUString aLightOnPropName("D3DSceneLightOn");

    OUString aPropName;
    OUString aIndexStr;
    ::basegfx::B3DVector aLightDirection;
    drawing::Direction3D xLightDir;
    sal_Bool bLightOnOff = false;
    for(sal_Int32 nLamp = 1; nLamp <= 8; nLamp++)
    {
        aIndexStr = OUString::number( nLamp );

        // lightcolor
        aPropName = aColorPropName;
        aPropName += aIndexStr;
        sal_Int32 nLightColor = 0;
        xPropSet->getPropertyValue( aPropName ) >>= nLightColor;
        ::sax::Converter::convertColor(sStringBuffer, nLightColor);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR, aStr);

        // lightdirection
        aPropName = aDirectionPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= xLightDir;
        aLightDirection = ::basegfx::B3DVector(xLightDir.DirectionX, xLightDir.DirectionY, xLightDir.DirectionZ);
        mrExport.GetMM100UnitConverter().convertB3DVector(sStringBuffer, aLightDirection);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DIRECTION, aStr);

        // lighton
        aPropName = aLightOnPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= bLightOnOff;
        ::sax::Converter::convertBool(sStringBuffer, bLightOnOff);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_ENABLED, aStr);

        // specular
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SPECULAR,
            nLamp == 1 ? XML_TRUE : XML_FALSE);

        // write light entry
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_LIGHT, sal_True, sal_True);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
