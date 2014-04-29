/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <tools/gen.hxx>
#include <xmloff/shapeexport.hxx>
#include "sdpropls.hxx"
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include "xexptran.hxx"
#include <xmloff/xmltoken.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;


//////////////////////////////////////////////////////////////////////////////

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

            // #89764# if export of position is suppressed for group shape,
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
    XmlShapeType eShapeType, sal_Int32 /* nFeatures = SEF_DEFAULT */, awt::Point* /*pRefPoint = NULL */)
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
            mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));

        switch(eShapeType)
        {
            case XmlShapeTypeDraw3DCubeObject:
            {
                // minEdge
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")));
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                ::basegfx::B3DVector aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // maxEdge
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")));
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

                // write 3DCube shape
                // #123542# Do this *after* the attributes are added, else these will be lost since opening
                // the scope will clear the global attribute list at the exporter
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_CUBE, sal_True, sal_True);

                break;
            }
            case XmlShapeTypeDraw3DSphereObject:
            {
                // Center
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")));
                drawing::Position3D aPosition3D;
                aAny >>= aPosition3D;
                ::basegfx::B3DVector aPos3D(aPosition3D.PositionX, aPosition3D.PositionY, aPosition3D.PositionZ);

                // Size
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")));
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

                // write 3DSphere shape
                // #123542# Do this *after* the attributes are added, else these will be lost since opening
                // the scope will clear the global attribute list at the exporter
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_SPHERE, sal_True, sal_True);

                break;
            }
            case XmlShapeTypeDraw3DLatheObject:
            case XmlShapeTypeDraw3DExtrudeObject:
            {
                // write special 3DLathe/3DExtrude attributes, get 3D PolyPolygon as drawing::PolyPolygonShape3D
                aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPolyPolygon3D")));
                drawing::PolyPolygonShape3D xPolyPolygon3D;
                aAny >>= xPolyPolygon3D;

                // convert to 3D PolyPolygon
                const basegfx::B3DPolyPolygon aPolyPolygon3D(
                    basegfx::tools::UnoPolyPolygonShape3DToB3DPolyPolygon(
                        xPolyPolygon3D));

                // convert to 2D PolyPolygon using identity 3D transformation (just grep X and Y)
                const basegfx::B3DHomMatrix aB3DHomMatrixFor2DConversion;
                const basegfx::B2DPolyPolygon aPolyPolygon(
                    basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(
                        aPolyPolygon3D,
                        aB3DHomMatrixFor2DConversion));

                // get 2D range of it
                const basegfx::B2DRange aPolyPolygonRange(aPolyPolygon.getB2DRange());

                // export ViewBox
                SdXMLImExViewBox aViewBox(
                    aPolyPolygonRange.getMinX(),
                    aPolyPolygonRange.getMinY(),
                    aPolyPolygonRange.getWidth(),
                    aPolyPolygonRange.getHeight());

                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());

                // prepare svg:d string
                const ::rtl::OUString aPolygonString(
                    basegfx::tools::exportToSvgD(
                        aPolyPolygon,
                        true,           // bUseRelativeCoordinates
                        false,          // bDetectQuadraticBeziers TTTT: not used in old, but maybe activated now
                        true));         // bHandleRelativeNextPointCompatible

                // write point array
                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aPolygonString);

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
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));

    // VRP, VPN, VUP
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DCameraGeometry")));
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
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DScenePerspective")));
    drawing::ProjectionMode xPrjMode;
    aAny >>= xPrjMode;
    if(xPrjMode == drawing::ProjectionMode_PARALLEL)
        aStr = GetXMLToken(XML_PARALLEL);
    else
        aStr = GetXMLToken(XML_PERSPECTIVE);
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_PROJECTION, aStr);

    // distance
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneDistance")));
    sal_Int32 nDistance = 0;
    aAny >>= nDistance;
    mrExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDistance);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_DISTANCE, aStr);

    // focalLength
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneFocalLength")));
    sal_Int32 nFocalLength = 0;
    aAny >>= nFocalLength;
    mrExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nFocalLength);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_FOCAL_LENGTH, aStr);

    // shadowSlant
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadowSlant")));
    sal_Int16 nShadowSlant = 0;
    aAny >>= nShadowSlant;
    mrExport.GetMM100UnitConverter().convertNumber(sStringBuffer, (sal_Int32)nShadowSlant);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADOW_SLANT, aStr);

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
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SHADE_MODE, aStr);

    // ambientColor
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneAmbientColor")));
    sal_Int32 aColTemp = 0;
    Color aAmbientColor;
    aAny >>= aColTemp; aAmbientColor.SetColor(aColTemp);
    mrExport.GetMM100UnitConverter().convertColor(sStringBuffer, aAmbientColor);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_AMBIENT_COLOR, aStr);

    // lightingMode
    aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneTwoSidedLighting")));
    sal_Bool bTwoSidedLighting = false;
    aAny >>= bTwoSidedLighting;
    mrExport.GetMM100UnitConverter().convertBool(sStringBuffer, bTwoSidedLighting);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_LIGHTING_MODE, aStr);
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
    sal_Int32 aColTemp = 0;
    Color aLightColor;
    ::basegfx::B3DVector aLightDirection;
    drawing::Direction3D xLightDir;
    sal_Bool bLightOnOff = false;
    for(sal_Int32 nLamp = 1; nLamp <= 8; nLamp++)
    {
        aIndexStr = OUString::valueOf( nLamp );

        // lightcolor
        aPropName = aColorPropName;
        aPropName += aIndexStr;
        xPropSet->getPropertyValue( aPropName ) >>= aColTemp;
        aLightColor.SetColor(aColTemp);
        mrExport.GetMM100UnitConverter().convertColor(sStringBuffer, aLightColor);
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
        mrExport.GetMM100UnitConverter().convertBool(sStringBuffer, bLightOnOff);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_ENABLED, aStr);

        // specular
        mrExport.AddAttribute(XML_NAMESPACE_DR3D, XML_SPECULAR,
            nLamp == 1 ? XML_TRUE : XML_FALSE);

        // write light entry
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DR3D, XML_LIGHT, sal_True, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////
