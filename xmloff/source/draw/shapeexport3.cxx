/*************************************************************************
 *
 *  $RCSfile: shapeexport3.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-28 11:19:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _B3D_HMATRIX_HXX
#include <goodies/hmatrix.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX
#include "shapeexport.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#include "xmlkywd.hxx"
#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

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
            SvXMLElementExport aOBJ( rExport, XML_NAMESPACE_DR3D, sXML_scene, sal_True, sal_True);

            ImpExportEvents( xShape );

            // write 3DSceneLights
            export3DLamps( xPropSet );

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
            rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_transform, aTransform.GetExportString(rExport.GetMM100UnitConverter()));

        switch(eShapeType)
        {
            case XmlShapeTypeDraw3DCubeObject:
            {
                // write 3DCube shape
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, sXML_cube, sal_True, sal_True);

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
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_min_edge, aStr);
                }

                // write maxEdge
                if(aDir3D != Vector3D(2500.0, 2500.0, 2500.0)) // write only when not default
                {
                    rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_max_edge, aStr);
                }

                break;
            }
            case XmlShapeTypeDraw3DSphereObject:
            {
                // write 3DSphere shape
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, sXML_sphere, sal_True, sal_True);

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
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_center, aStr);
                }

                // write Size
                if(aDir3D != Vector3D(5000.0, 5000.0, 5000.0)) // write only when not default
                {
                    rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aDir3D);
                    aStr = sStringBuffer.makeStringAndClear();
                    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_size, aStr);
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

                for(sal_Int32 a(0L); a < nOuterSequenceCount; a++)
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
                rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_viewBox,
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
                rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_d, aSvgDElement.GetExportString());

                if(eShapeType == XmlShapeTypeDraw3DLatheObject)
                {
                    // write 3DLathe shape
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, sXML_rotate, sal_True, sal_True);
                }
                else
                {
                    // write 3DExtrude shape
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, sXML_extrude, sal_True, sal_True);
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

/** helper for chart that adds all attributes of a 3d scene element to the export */
void XMLShapeExport::export3DSceneAttributes( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet )
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
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_transform, aTransform.GetExportString(rExport.GetMM100UnitConverter()));

    // VRP, VPN, VUP
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DCameraGeometry")));
    drawing::CameraGeometry aCamGeo;
    aAny >>= aCamGeo;

    Vector3D aVRP(aCamGeo.vrp.PositionX, aCamGeo.vrp.PositionY, aCamGeo.vrp.PositionZ);
    if(aVRP != Vector3D(0.0, 0.0, 1.0)) // write only when not default
    {
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aVRP);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_vrp, aStr);
    }

    Vector3D aVPN(aCamGeo.vpn.DirectionX, aCamGeo.vpn.DirectionY, aCamGeo.vpn.DirectionZ);
    if(aVPN != Vector3D(0.0, 0.0, 1.0)) // write only when not default
    {
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aVPN);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_vpn, aStr);
    }

    Vector3D aVUP(aCamGeo.vup.DirectionX, aCamGeo.vup.DirectionY, aCamGeo.vup.DirectionZ);
    if(aVUP != Vector3D(0.0, 1.0, 0.0)) // write only when not default
    {
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aVUP);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_vup, aStr);
    }

    // projection "D3DScenePerspective" drawing::ProjectionMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DScenePerspective")));
    drawing::ProjectionMode xPrjMode;
    aAny >>= xPrjMode;
    if(xPrjMode == drawing::ProjectionMode_PARALLEL)
        aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_parallel));
    else
        aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_perspective));
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_projection, aStr);

    // distance
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneDistance")));
    sal_Int32 nDistance;
    aAny >>= nDistance;
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDistance);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_distance, aStr);

    // focalLength
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneFocalLength")));
    sal_Int32 nFocalLength;
    aAny >>= nFocalLength;
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nFocalLength);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_focal_length, aStr);

    // shadowSlant
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadowSlant")));
    sal_Int16 nShadowSlant;
    aAny >>= nShadowSlant;
    rExport.GetMM100UnitConverter().convertNumber(sStringBuffer, (sal_Int32)nShadowSlant);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_shadow_slant, aStr);

    // shadeMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadeMode")));
    drawing::ShadeMode xShadeMode;
    if(aAny >>= xShadeMode)
    {
        if(xShadeMode == drawing::ShadeMode_FLAT)
            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_flat));
        else if(xShadeMode == drawing::ShadeMode_PHONG)
            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_phong));
        else if(xShadeMode == drawing::ShadeMode_SMOOTH)
            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_gouraud));
        else
            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_draft));
    }
    else
    {
        // ShadeMode enum not there, write default
        aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_gouraud));
    }
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_shade_mode, aStr);

    // ambientColor
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneAmbientColor")));
    sal_Int32 aColTemp;
    Color aAmbientColor;
    aAny >>= aColTemp; aAmbientColor.SetColor(aColTemp);
    rExport.GetMM100UnitConverter().convertColor(sStringBuffer, aAmbientColor);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_ambient_color, aStr);

    // lightingMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneTwoSidedLighting")));
    sal_Bool bTwoSidedLighting;
    aAny >>= bTwoSidedLighting;
    rExport.GetMM100UnitConverter().convertBool(sStringBuffer, bTwoSidedLighting);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_lighting_mode, aStr);
}

/** helper for chart that exports all lamps from the propertyset */
void XMLShapeExport::export3DLamps( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet )
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
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_diffuse_color, aStr);

        // lightdirection
        aPropName = aDirectionPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= xLightDir;
        aLightDirection = Vector3D(xLightDir.DirectionX, xLightDir.DirectionY, xLightDir.DirectionZ);
        rExport.GetMM100UnitConverter().convertVector3D(sStringBuffer, aLightDirection);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_direction, aStr);

        // lighton
        aPropName = aLightOnPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue(aPropName) >>= bLightOnOff;
        rExport.GetMM100UnitConverter().convertBool(sStringBuffer, bLightOnOff);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_enabled, aStr);

        // specular
        rExport.AddAttribute(XML_NAMESPACE_DR3D, sXML_specular,
            nLamp == 1 ? OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_true ) ) :
                            OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_false ) ) );

        // write light entry
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DR3D, sXML_light, sal_True, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////
