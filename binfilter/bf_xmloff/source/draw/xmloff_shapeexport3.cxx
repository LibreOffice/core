/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <bf_goodies/hmatrix.hxx>


#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>

#include <com/sun/star/drawing/ProjectionMode.hpp>

#include <com/sun/star/drawing/ShadeMode.hpp>



#include <com/sun/star/drawing/CameraGeometry.hpp>

#include <com/sun/star/drawing/DoubleSequence.hpp>



#include <tools/debug.hxx>


#include "xmlexp.hxx"

#include "xmluconv.hxx"

#include "xexptran.hxx"


#include "xmlnmspe.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

//////////////////////////////////////////////////////////////////////////////

inline sal_Int32 FRound( double fVal )
{
    return( fVal > 0.0 ? (sal_Int32) ( fVal + 0.5 ) : -(sal_Int32) ( -fVal + 0.5 ) );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExport3DSceneShape( const uno::Reference< drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
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
            SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DR3D, XML_SCENE, bCreateNewline, sal_True);

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

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExport3DShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUString aStr;
        OUStringBuffer sStringBuffer;

        // transformation (UNO_NAME_3D_TRANSFORM_MATRIX == "D3DTransformMatrix")
        uno::Any aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DTransformMatrix")));
        drawing::HomogenMatrix xHomMat;
        aAny >>= xHomMat;
        SdXMLImExTransform3D aTransform;
        aTransform.AddHomogenMatrix(xHomMat);
        if(aTransform.NeedsAction())
            rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(rExport.GetMM100UnitConverter()));
        
        switch(eShapeType)
        {
            case XmlShapeTypeDraw3DCubeObject:
            {
                // write 3DCube shape
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, XML_CUBE, sal_True, sal_True);

                // minEdge
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")));
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                Vector3D aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // maxEdge
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")));
                drawing::Direction3D aDirection3D;
                aAny >>= aDirection3D;
                Vector3D aDir3D(aDirection3D.DirectionX, aDirection3D.DirectionY, aDirection3D.DirectionZ);

                // transform maxEdge from distance to pos
                aDir3D = aPos3D + aDir3D;

                // write minEdge
                if(aPos3D != Vector3D(-2500.0, -2500.0, -2500.0)) // write only when not default
                {
                    rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aPos3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_MIN_EDGE, aStr);
                }

                // write maxEdge
                if(aDir3D != Vector3D(2500.0, 2500.0, 2500.0)) // write only when not default
                {
                    rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_MAX_EDGE, aStr);
                }

                break;
            }
            case XmlShapeTypeDraw3DSphereObject:
            {
                // write 3DSphere shape
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, XML_SPHERE, sal_True, sal_True);

                // Center
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")));
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                Vector3D aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // Size
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")));
                drawing::Direction3D aDirection3D;
                aAny >>= aDirection3D;
                Vector3D aDir3D(aDirection3D.DirectionX, aDirection3D.DirectionY, aDirection3D.DirectionZ);

                // write Center
                if(aPos3D != Vector3D(0.0, 0.0, 0.0)) // write only when not default
                {
                    rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aPos3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_CENTER, aStr);
                }

                // write Size
                if(aDir3D != Vector3D(5000.0, 5000.0, 5000.0)) // write only when not default
                {
                    rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SIZE, aStr);
                }

                break;
            }
            case XmlShapeTypeDraw3DLatheObject:
            case XmlShapeTypeDraw3DExtrudeObject:
            {
                // write special 3DLathe/3DExtrude attributes
                uno::Any aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPolyPolygon3D")));
                drawing::PolyPolygonShape3D xPolyPolygon3D;
                aAny >>= xPolyPolygon3D;

                // look for maximal values
                double fXMin, fXMax, fYMin, fYMax;
                BOOL bInit(FALSE);
                sal_Int32 nOuterSequenceCount(xPolyPolygon3D.SequenceX.getLength());
                drawing::DoubleSequence* pInnerSequenceX = xPolyPolygon3D.SequenceX.getArray();
                drawing::DoubleSequence* pInnerSequenceY = xPolyPolygon3D.SequenceY.getArray();
                sal_Int32 a;

                for(a = 0L; a < nOuterSequenceCount; a++)
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
                            bInit = TRUE;
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
                rExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, 
                    aViewBox.GetExportString(rExport.GetMM100UnitConverter()));

                // prepare svx:d element export
                SdXMLImExSvgDElement aSvgDElement(aViewBox);
                pInnerSequenceX = xPolyPolygon3D.SequenceX.getArray();
                pInnerSequenceY = xPolyPolygon3D.SequenceY.getArray();

                for(a = 0L; a < nOuterSequenceCount; a++)
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
                    BOOL bClosed = (pFirst->X == pLast->X && pFirst->Y == pLast->Y);

                    aSvgDElement.AddPolygon(&aPoly, 0L, aMinPoint, 
                        aMaxSize, rExport.GetMM100UnitConverter(), bClosed);

                    // #80594# corrected error in PolyPolygon3D export for 3D XML
                    pInnerSequenceX++;
                    pInnerSequenceY++;
                }

                // write point array
                rExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());

                if(eShapeType == XmlShapeTypeDraw3DLatheObject)
                {
                    // write 3DLathe shape
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, XML_ROTATE, sal_True, sal_True);
                }
                else
                {
                    // write 3DExtrude shape
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, XML_EXTRUDE, sal_True, sal_True);
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

/** helper for chart that adds all attributes of a 3d scene element to the export */
void XMLShapeExport::export3DSceneAttributes( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet )
{
    OUString aStr;
    OUStringBuffer sStringBuffer;

    // world transformation (UNO_NAME_3D_TRANSFORM_MATRIX == "D3DTransformMatrix")
    uno::Any aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DTransformMatrix")));
    drawing::HomogenMatrix xHomMat;
    aAny >>= xHomMat;
    SdXMLImExTransform3D aTransform;
    aTransform.AddHomogenMatrix(xHomMat);
    if(aTransform.NeedsAction())
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(rExport.GetMM100UnitConverter()));

    // VRP, VPN, VUP
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DCameraGeometry")));
    drawing::CameraGeometry aCamGeo;
    aAny >>= aCamGeo;
    
    Vector3D aVRP(aCamGeo.vrp.PositionX, aCamGeo.vrp.PositionY, aCamGeo.vrp.PositionZ);
    if(aVRP != Vector3D(0.0, 0.0, 1.0)) // write only when not default
    {
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aVRP);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VRP, aStr);
    }

    Vector3D aVPN(aCamGeo.vpn.DirectionX, aCamGeo.vpn.DirectionY, aCamGeo.vpn.DirectionZ);
    if(aVPN != Vector3D(0.0, 0.0, 1.0)) // write only when not default
    {
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aVPN);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VPN, aStr);
    }

    Vector3D aVUP(aCamGeo.vup.DirectionX, aCamGeo.vup.DirectionY, aCamGeo.vup.DirectionZ);
    if(aVUP != Vector3D(0.0, 1.0, 0.0)) // write only when not default
    {
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aVUP);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_VUP, aStr);
    }

    // projection "D3DScenePerspective" drawing::ProjectionMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DScenePerspective")));
    drawing::ProjectionMode xPrjMode;
    aAny >>= xPrjMode;
    if(xPrjMode == drawing::ProjectionMode_PARALLEL)
        aStr = GetXMLToken(XML_PARALLEL);
    else
        aStr = GetXMLToken(XML_PERSPECTIVE);
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_PROJECTION, aStr);

    // distance
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneDistance")));
    sal_Int32 nDistance;
    aAny >>= nDistance;
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDistance);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DISTANCE, aStr);

    // focalLength
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneFocalLength")));
    sal_Int32 nFocalLength;
    aAny >>= nFocalLength;
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nFocalLength);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_FOCAL_LENGTH, aStr);

    // shadowSlant
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadowSlant")));
    sal_Int16 nShadowSlant;
    aAny >>= nShadowSlant;
    rExport.GetMM100UnitConverter().convertNumber(sStringBuffer, (sal_Int32)nShadowSlant);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADOW_SLANT, aStr);

    // shadeMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadeMode")));
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
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADE_MODE, aStr);

    // ambientColor
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneAmbientColor")));
    sal_Int32 aColTemp;
    Color aAmbientColor;
    aAny >>= aColTemp; aAmbientColor.SetColor(aColTemp);
    rExport.GetMM100UnitConverter().convertColor(sStringBuffer, aAmbientColor);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_AMBIENT_COLOR, aStr);

    // lightingMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneTwoSidedLighting")));
    sal_Bool bTwoSidedLighting;
    aAny >>= bTwoSidedLighting;
    rExport.GetMM100UnitConverter().convertBool(sStringBuffer, bTwoSidedLighting);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_LIGHTING_MODE, aStr);
}

/** helper for chart that exports all lamps from the propertyset */
void XMLShapeExport::export3DLamps( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet )
{
    // write lamps 1..8 as content
    OUString aStr;
    OUStringBuffer sStringBuffer;

    const OUString aColorPropName(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor") );
    const OUString aDirectionPropName(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection") );
    const OUString aLightOnPropName(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn") );

    OUString aPropName;
    OUString aIndexStr;
    sal_Int32 aColTemp;
    Color aLightColor;
    Vector3D aLightDirection;
    drawing::Direction3D xLightDir;
    sal_Bool bLightOnOff;
    for(sal_Int32 nLamp = 1; nLamp <= 8; nLamp++)
    {
        aIndexStr = OUString::valueOf( nLamp );

        // lightcolor
        aPropName = aColorPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue( aPropName ) >>= aColTemp;
        aLightColor.SetColor(aColTemp);
        rExport.GetMM100UnitConverter().convertColor(sStringBuffer, aLightColor);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR, aStr);

        // lightdirection
        aPropName = aDirectionPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= xLightDir;
        aLightDirection = Vector3D(xLightDir.DirectionX, xLightDir.DirectionY, xLightDir.DirectionZ);
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aLightDirection);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DIRECTION, aStr);

        // lighton
        aPropName = aLightOnPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= bLightOnOff;
        rExport.GetMM100UnitConverter().convertBool(sStringBuffer, bLightOnOff);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_ENABLED, aStr);
        
        // specular
        rExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SPECULAR,
            nLamp == 1 ? XML_TRUE : XML_FALSE);
        
        // write light entry
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, XML_LIGHT, sal_True, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
