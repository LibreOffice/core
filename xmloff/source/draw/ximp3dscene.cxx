/*************************************************************************
 *
 *  $RCSfile: ximp3dscene.cxx,v $
 *
 *  $Revision: 1.7 $
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

#pragma hdrstop

#ifndef _XIMP3DSCENE_HXX
#include "ximp3dscene.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dlight context

SdXML3DLightContext::SdXML3DLightContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const rtl::OUString& rLName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList)
:   SvXMLImportContext( rImport, nPrfx, rLName),
    maDiffuseColor(0x00000000),
    maDirection(0.0, 0.0, 1.0),
    mbEnabled(FALSE),
    mbSpecular(FALSE)
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
                GetImport().GetMM100UnitConverter().convertColor(maDiffuseColor, sValue);
                break;
            }
            case XML_TOK_3DLIGHT_DIRECTION:
            {
                GetImport().GetMM100UnitConverter().convertVector3D(maDirection, sValue);
                break;
            }
            case XML_TOK_3DLIGHT_ENABLED:
            {
                GetImport().GetMM100UnitConverter().convertBool(mbEnabled, sValue);
                break;
            }
            case XML_TOK_3DLIGHT_SPECULAR:
            {
                GetImport().GetMM100UnitConverter().convertBool(mbSpecular, sValue);
                break;
            }
        }
    }
}

SdXML3DLightContext::~SdXML3DLightContext()
{
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DSceneShapeContext, SdXMLShapeContext );

SdXML3DSceneShapeContext::SdXML3DSceneShapeContext(
    SvXMLImport& rImport,
    USHORT nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ), SdXML3DSceneAttributesHelper( rImport )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DSceneShapeContext::~SdXML3DSceneShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSceneShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create new 3DScene shape and add it to rShapes, use it
    // as base for the new 3DScene import
    AddShape( "com.sun.star.drawing.Shape3DSceneObject" );
    if( mxShape.is() )
    {
        mxChilds = uno::Reference< drawing::XShapes >::query( mxShape );

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
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSceneShapeContext::EndElement()
{
    if(mxShape.is())
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            setSceneAttributes( xPropSet );
        }

        // call parent
        SdXMLShapeContext::EndElement();
    }
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXML3DSceneShapeContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0L;

    // look for local light context first
    if(nPrefix == XML_NAMESPACE_DR3D
        && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_light))))
    {
        // dr3d:light inside dr3d:scene context
        pContext = create3DLightContext( nPrefix, rLocalName, xAttrList );
    }

    // call GroupChildContext function at common ShapeImport
    if(!pContext)
    {
        pContext = GetImport().GetShapeImport()->Create3DSceneChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxChilds);
    }

    // call parent when no own context was created
    if(!pContext)
    {
        pContext = SvXMLImportContext::CreateChildContext(
        nPrefix, rLocalName, xAttrList);
    }

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DSceneAttributesHelper::SdXML3DSceneAttributesHelper( SvXMLImport& rImporter )
:   mrImport( rImporter ),
    mbSetTransform( FALSE ),
    mxPrjMode(drawing::ProjectionMode_PERSPECTIVE),
    mnDistance(1000),
    mnFocalLength(1000),
    mnShadowSlant(0),
    mxShadeMode(drawing::ShadeMode_SMOOTH),
    maAmbientColor(0x00666666),
    mbLightingMode(FALSE),
    maVRP(0.0, 0.0, 1.0),
    maVPN(0.0, 0.0, 1.0),
    maVUP(0.0, 1.0, 0.0),
    mbVRPUsed(FALSE),
    mbVPNUsed(FALSE),
    mbVUPUsed(FALSE)
{
}

SdXML3DSceneAttributesHelper::~SdXML3DSceneAttributesHelper()
{
    // release remembered light contexts, they are no longer needed
    while(maList.Count())
        maList.Remove(maList.Count() - 1)->ReleaseRef();
}

/** creates a 3d ligth context and adds it to the internal list for later processing */
SvXMLImportContext * SdXML3DSceneAttributesHelper::create3DLightContext( sal_uInt16 nPrfx, const rtl::OUString& rLName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    SvXMLImportContext* pContext = new SdXML3DLightContext(mrImport, nPrfx, rLName, xAttrList);

    // remember SdXML3DLightContext for later evaluation
    if(pContext)
    {
        pContext->AddRef();
        maList.Insert((SdXML3DLightContext*)pContext, LIST_APPEND);
    }

    return pContext;
}

/** this should be called for each scene attribute */
void SdXML3DSceneAttributesHelper::processSceneAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DR3D == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_transform)) )
        {
            SdXMLImExTransform3D aTransform(rValue, mrImport.GetMM100UnitConverter());
            if(aTransform.NeedsAction())
                mbSetTransform = aTransform.GetFullHomogenTransform(mxHomMat);
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_vrp)) )
        {
            Vector3D aNewVec;
            mrImport.GetMM100UnitConverter().convertVector3D(aNewVec, rValue);

            if(aNewVec != maVRP)
            {
                maVRP = aNewVec;
                mbVRPUsed = TRUE;
            }
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_vpn)) )
        {
            Vector3D aNewVec;
            mrImport.GetMM100UnitConverter().convertVector3D(aNewVec, rValue);

            if(aNewVec != maVPN)
            {
                maVPN = aNewVec;
                mbVPNUsed = TRUE;
            }
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_vup)) )
        {
            Vector3D aNewVec;
            mrImport.GetMM100UnitConverter().convertVector3D(aNewVec, rValue);

            if(aNewVec != maVUP)
            {
                maVUP = aNewVec;
                mbVUPUsed = TRUE;
            }
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_projection)) )
        {
            if(rValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_parallel))))
                mxPrjMode = drawing::ProjectionMode_PARALLEL;
            else
                mxPrjMode = drawing::ProjectionMode_PERSPECTIVE;
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_distance)) )
        {
            mrImport.GetMM100UnitConverter().convertMeasure(mnDistance, rValue);
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_focal_length)) )
        {
            mrImport.GetMM100UnitConverter().convertMeasure(mnFocalLength, rValue);
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_shadow_slant)) )
        {
            mrImport.GetMM100UnitConverter().convertNumber(mnShadowSlant, rValue);
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_shade_mode)) )
        {
            if(rValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_flat))))
                mxShadeMode = drawing::ShadeMode_FLAT;
            else if(rValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_phong))))
                mxShadeMode = drawing::ShadeMode_PHONG;
            else if(rValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_gouraud))))
                mxShadeMode = drawing::ShadeMode_SMOOTH;
            else
                mxShadeMode = drawing::ShadeMode_DRAFT;
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_ambient_color)) )
        {
            mrImport.GetMM100UnitConverter().convertColor(maAmbientColor, rValue);
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_lighting_mode)) )
        {
            mrImport.GetMM100UnitConverter().convertBool(mbLightingMode, rValue);
            return;
        }
    }
}

/** this sets the scene attributes at this propertyset */
void SdXML3DSceneAttributesHelper::setSceneAttributes( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xPropSet )
{
    uno::Any aAny;

    // world transformation
    if(mbSetTransform)
    {
        aAny <<= mxHomMat;
        xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DTransformMatrix")), aAny);
    }

    // projection "D3DScenePerspective" drawing::ProjectionMode
    aAny <<= mxPrjMode;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DScenePerspective")), aAny);

    // distance
    aAny <<= mnDistance;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneDistance")), aAny);

    // focalLength
    aAny <<= mnFocalLength;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneFocalLength")), aAny);

    // shadowSlant
    aAny <<= (sal_Int16)mnShadowSlant;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadowSlant")), aAny);

    // shadeMode
    aAny <<= mxShadeMode;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneShadeMode")), aAny);

    // ambientColor
    aAny <<= maAmbientColor.GetColor();
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneAmbientColor")), aAny);

    // lightingMode
    aAny <<= mbLightingMode;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneTwoSidedLighting")), aAny);

    if(maList.Count())
    {
        uno::Any aAny2;
        uno::Any aAny3;

        // set lights
        for(sal_uInt32 a(0L); a < maList.Count(); a++)
        {
            SdXML3DLightContext* pCtx = (SdXML3DLightContext*)maList.GetObject(a);

            // set anys
            aAny <<= pCtx->GetDiffuseColor().GetColor();
            drawing::Direction3D xLightDir;
            xLightDir.DirectionX = pCtx->GetDirection().X();
            xLightDir.DirectionY = pCtx->GetDirection().Y();
            xLightDir.DirectionZ = pCtx->GetDirection().Z();
            aAny2 <<= xLightDir;
            aAny3 <<= pCtx->GetEnabled();

            switch(a)
            {
                case 0:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor1")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection1")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn1")), aAny3);
                    break;
                }
                case 1:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor2")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection2")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn2")), aAny3);
                    break;
                }
                case 2:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor3")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection3")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn3")), aAny3);
                    break;
                }
                case 3:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor4")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection4")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn4")), aAny3);
                    break;
                }
                case 4:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor5")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection5")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn5")), aAny3);
                    break;
                }
                case 5:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor6")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection6")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn6")), aAny3);
                    break;
                }
                case 6:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor7")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection7")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn7")), aAny3);
                    break;
                }
                case 7:
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightColor8")), aAny);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightDirection8")), aAny2);
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSceneLightOn8")), aAny3);
                    break;
                }
            }
        }
    }

    // CameraGeometry and camera settings
    drawing::CameraGeometry aCamGeo;
    aCamGeo.vrp.PositionX = maVRP.X();
    aCamGeo.vrp.PositionY = maVRP.Y();
    aCamGeo.vrp.PositionZ = maVRP.Z();
    aCamGeo.vpn.DirectionX = maVPN.X();
    aCamGeo.vpn.DirectionY = maVPN.Y();
    aCamGeo.vpn.DirectionZ = maVPN.Z();
    aCamGeo.vup.DirectionX = maVUP.X();
    aCamGeo.vup.DirectionY = maVUP.Y();
    aCamGeo.vup.DirectionZ = maVUP.Z();
    aAny <<= aCamGeo;
    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DCameraGeometry")), aAny);
}
