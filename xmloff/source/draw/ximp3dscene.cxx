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

#include <sax/tools/converter.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>

#include "ximp3dscene.hxx"
#include <xmloff/xmluconv.hxx>
#include <xexptran.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include "eventimp.hxx"
#include "descriptionimp.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// dr3d:3dlight context

SdXML3DLightContext::SdXML3DLightContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList)
:   SvXMLImportContext( rImport, nPrfx, rLName),
    maDiffuseColor(0x00000000),
    maDirection(0.0, 0.0, 1.0),
    mbEnabled(false),
    mbSpecular(false)
{
    // read attributes for the 3DScene
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DLightAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_3DLIGHT_DIFFUSE_COLOR:
            {
                ::sax::Converter::convertColor(maDiffuseColor, sValue);
                break;
            }
            case XML_TOK_3DLIGHT_DIRECTION:
            {
                ::basegfx::B3DVector aVal;
                SvXMLUnitConverter::convertB3DVector(aVal, sValue);
                if (!rtl::math::isNan(aVal.getX()) && !rtl::math::isNan(aVal.getY()) && !rtl::math::isNan(aVal.getZ()))
                {
                    maDirection = aVal;
                }
                else
                {
                    SAL_WARN("xmloff", "NaNs found in light direction: " << sValue);
                }
                break;
            }
            case XML_TOK_3DLIGHT_ENABLED:
            {
                (void)::sax::Converter::convertBool(mbEnabled, sValue);
                break;
            }
            case XML_TOK_3DLIGHT_SPECULAR:
            {
                (void)::sax::Converter::convertBool(mbSpecular, sValue);
                break;
            }
        }
    }
}

SdXML3DLightContext::~SdXML3DLightContext()
{
}


SdXML3DSceneShapeContext::SdXML3DSceneShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShapes ), SdXML3DSceneAttributesHelper( rImport )
{
}

SdXML3DSceneShapeContext::~SdXML3DSceneShapeContext()
{
}

void SdXML3DSceneShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create new 3DScene shape and add it to rShapes, use it
    // as base for the new 3DScene import
    AddShape( "com.sun.star.drawing.Shape3DSceneObject" );
    if( mxShape.is() )
    {
        SetStyle();

        mxChildren.set( mxShape, uno::UNO_QUERY );
        if( mxChildren.is() )
            GetImport().GetShapeImport()->pushGroupForSorting( mxChildren );

        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();
    }

    // read attributes for the 3DScene
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        processSceneAttribute( nPrefix, aLocalName, sValue );
    }

    // #91047# call parent function is missing here, added it
    if(mxShape.is())
    {
        // call parent
        SdXMLShapeContext::StartElement(xAttrList);
    }
}

void SdXML3DSceneShapeContext::EndElement()
{
    if(mxShape.is())
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            setSceneAttributes( xPropSet );
        }

        if( mxChildren.is() )
            GetImport().GetShapeImport()->popGroupAndSort();

        // call parent
        SdXMLShapeContext::EndElement();
    }
}

SvXMLImportContextRef SdXML3DSceneShapeContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;

    // #i68101#
    if( nPrefix == XML_NAMESPACE_SVG &&
        (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) )
    {
        xContext = new SdXMLDescriptionContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        xContext = new SdXMLEventsContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    // look for local light context first
    else if(nPrefix == XML_NAMESPACE_DR3D && IsXMLToken( rLocalName, XML_LIGHT ) )
    {
        // dr3d:light inside dr3d:scene context
        xContext = create3DLightContext( nPrefix, rLocalName, xAttrList );
    }

    // call GroupChildContext function at common ShapeImport
    if (!xContext)
    {
        xContext = GetImport().GetShapeImport()->Create3DSceneChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxChildren);
    }

    // call parent when no own context was created
    if (!xContext)
    {
        xContext = SvXMLImportContext::CreateChildContext(
        nPrefix, rLocalName, xAttrList);
    }

    return xContext;
}

SdXML3DSceneAttributesHelper::SdXML3DSceneAttributesHelper( SvXMLImport& rImporter )
:   mrImport( rImporter ),
    mbSetTransform( false ),
    mxPrjMode(drawing::ProjectionMode_PERSPECTIVE),
    mnDistance(1000),
    mnFocalLength(1000),
    mnShadowSlant(0),
    mxShadeMode(drawing::ShadeMode_SMOOTH),
    maAmbientColor(0x00666666),
    mbLightingMode(false),
    maVRP(0.0, 0.0, 1.0),
    maVPN(0.0, 0.0, 1.0),
    maVUP(0.0, 1.0, 0.0),
    mbVRPUsed(false),
    mbVPNUsed(false),
    mbVUPUsed(false)
{
}

/** creates a 3d light context and adds it to the internal list for later processing */
SvXMLImportContext * SdXML3DSceneAttributesHelper::create3DLightContext( sal_uInt16 nPrfx, const OUString& rLName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList)
{
    const rtl::Reference<SdXML3DLightContext> xContext{new SdXML3DLightContext(mrImport, nPrfx, rLName, xAttrList)};

    // remember SdXML3DLightContext for later evaluation
    maList.push_back(xContext);

    return xContext.get();
}

/** this should be called for each scene attribute */
void SdXML3DSceneAttributesHelper::processSceneAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DR3D == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_TRANSFORM ) )
        {
            SdXMLImExTransform3D aTransform(rValue, mrImport.GetMM100UnitConverter());
            if(aTransform.NeedsAction())
                mbSetTransform = aTransform.GetFullHomogenTransform(mxHomMat);
            return;
        }
        else if( IsXMLToken( rLocalName, XML_VRP ) )
        {
            ::basegfx::B3DVector aNewVec;
            SvXMLUnitConverter::convertB3DVector(aNewVec, rValue);

            if(aNewVec != maVRP)
            {
                maVRP = aNewVec;
                mbVRPUsed = true;
            }
            return;
        }
        else if( IsXMLToken( rLocalName, XML_VPN ) )
        {
            ::basegfx::B3DVector aNewVec;
            SvXMLUnitConverter::convertB3DVector(aNewVec, rValue);

            if(aNewVec != maVPN)
            {
                maVPN = aNewVec;
                mbVPNUsed = true;
            }
            return;
        }
        else if( IsXMLToken( rLocalName, XML_VUP ) )
        {
            ::basegfx::B3DVector aNewVec;
            SvXMLUnitConverter::convertB3DVector(aNewVec, rValue);

            if(aNewVec != maVUP)
            {
                maVUP = aNewVec;
                mbVUPUsed = true;
            }
            return;
        }
        else if( IsXMLToken( rLocalName, XML_PROJECTION ) )
        {
            if( IsXMLToken( rValue, XML_PARALLEL ) )
                mxPrjMode = drawing::ProjectionMode_PARALLEL;
            else
                mxPrjMode = drawing::ProjectionMode_PERSPECTIVE;
            return;
        }
        else if( IsXMLToken( rLocalName, XML_DISTANCE ) )
        {
            mrImport.GetMM100UnitConverter().convertMeasureToCore(mnDistance,
                    rValue);
            return;
        }
        else if( IsXMLToken( rLocalName, XML_FOCAL_LENGTH ) )
        {
            mrImport.GetMM100UnitConverter().convertMeasureToCore(mnFocalLength,
                    rValue);
            return;
        }
        else if( IsXMLToken( rLocalName, XML_SHADOW_SLANT ) )
        {
            ::sax::Converter::convertNumber(mnShadowSlant, rValue);
            return;
        }
        else if( IsXMLToken( rLocalName, XML_SHADE_MODE ) )
        {
            if( IsXMLToken( rValue, XML_FLAT ) )
                mxShadeMode = drawing::ShadeMode_FLAT;
            else if( IsXMLToken( rValue, XML_PHONG ) )
                mxShadeMode = drawing::ShadeMode_PHONG;
            else if( IsXMLToken( rValue, XML_GOURAUD ) )
                mxShadeMode = drawing::ShadeMode_SMOOTH;
            else
                mxShadeMode = drawing::ShadeMode_DRAFT;
            return;
        }
        else if( IsXMLToken( rLocalName, XML_AMBIENT_COLOR ) )
        {
            ::sax::Converter::convertColor(maAmbientColor, rValue);
            return;
        }
        else if( IsXMLToken( rLocalName, XML_LIGHTING_MODE ) )
        {
            (void)::sax::Converter::convertBool(mbLightingMode, rValue);
            return;
        }
    }
}

/** this sets the scene attributes at this propertyset */
void SdXML3DSceneAttributesHelper::setSceneAttributes( const css::uno::Reference< css::beans::XPropertySet >& xPropSet )
{
    uno::Any aAny;

    // world transformation
    if(mbSetTransform)
    {
        xPropSet->setPropertyValue("D3DTransformMatrix", uno::Any(mxHomMat));
    }

    // distance
    xPropSet->setPropertyValue("D3DSceneDistance", uno::Any(mnDistance));
    // focalLength
    xPropSet->setPropertyValue("D3DSceneFocalLength", uno::Any(mnFocalLength));
    // shadowSlant
    xPropSet->setPropertyValue("D3DSceneShadowSlant", uno::Any(static_cast<sal_Int16>(mnShadowSlant)));
    // shadeMode
    xPropSet->setPropertyValue("D3DSceneShadeMode", uno::Any(mxShadeMode));
    // ambientColor
    xPropSet->setPropertyValue("D3DSceneAmbientColor", uno::Any(maAmbientColor));
    // lightingMode
    xPropSet->setPropertyValue("D3DSceneTwoSidedLighting", uno::Any(mbLightingMode));

    if( !maList.empty() )
    {
        uno::Any aAny2;
        uno::Any aAny3;

        // set lights
        for( size_t a = 0; a < maList.size(); a++)
        {
            SdXML3DLightContext* pCtx = maList[ a ].get();

            // set anys
            aAny <<= pCtx->GetDiffuseColor();
            drawing::Direction3D aLightDir;
            aLightDir.DirectionX = pCtx->GetDirection().getX();
            aLightDir.DirectionY = pCtx->GetDirection().getY();
            aLightDir.DirectionZ = pCtx->GetDirection().getZ();
            aAny2 <<= aLightDir;
            aAny3 <<= pCtx->GetEnabled();

            switch(a)
            {
                case 0:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor1", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection1", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn1", aAny3);
                    break;
                }
                case 1:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor2", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection2", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn2", aAny3);
                    break;
                }
                case 2:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor3", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection3", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn3", aAny3);
                    break;
                }
                case 3:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor4", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection4", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn4", aAny3);
                    break;
                }
                case 4:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor5", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection5", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn5", aAny3);
                    break;
                }
                case 5:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor6", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection6", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn6", aAny3);
                    break;
                }
                case 6:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor7", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection7", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn7", aAny3);
                    break;
                }
                case 7:
                {
                    xPropSet->setPropertyValue("D3DSceneLightColor8", aAny);
                    xPropSet->setPropertyValue("D3DSceneLightDirection8", aAny2);
                    xPropSet->setPropertyValue("D3DSceneLightOn8", aAny3);
                    break;
                }
            }
        }
    }

    // CameraGeometry and camera settings
    drawing::CameraGeometry aCamGeo;
    aCamGeo.vrp.PositionX = maVRP.getX();
    aCamGeo.vrp.PositionY = maVRP.getY();
    aCamGeo.vrp.PositionZ = maVRP.getZ();
    aCamGeo.vpn.DirectionX = maVPN.getX();
    aCamGeo.vpn.DirectionY = maVPN.getY();
    aCamGeo.vpn.DirectionZ = maVPN.getZ();
    aCamGeo.vup.DirectionX = maVUP.getX();
    aCamGeo.vup.DirectionY = maVUP.getY();
    aCamGeo.vup.DirectionZ = maVUP.getZ();
    xPropSet->setPropertyValue("D3DCameraGeometry", uno::Any(aCamGeo));

    // #91047# set drawing::ProjectionMode AFTER camera geometry is set
    // projection "D3DScenePerspective" drawing::ProjectionMode
    xPropSet->setPropertyValue("D3DScenePerspective", uno::Any(mxPrjMode));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
