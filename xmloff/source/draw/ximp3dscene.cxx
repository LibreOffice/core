/*************************************************************************
 *
 *  $RCSfile: ximp3dscene.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-24 16:36:47 $
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

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// dr3d:3dlight context

SdXML3DLightContext::SdXML3DLightContext(
    SdXMLImport& rImport,
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

TYPEINIT1( SdXML3DSceneShapeContext, SvXMLImportContext );

SdXML3DSceneShapeContext::SdXML3DSceneShapeContext(
    SvXMLImport& rImport,
    USHORT nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SvXMLImportContext( rImport, nPrfx, rLocalName ),
    mxShapes(rShapes),
    mbSetTransform( FALSE ),
    mxPrjMode(drawing::ProjectionMode_PERSPECTIVE),
    mnDistance(1000),
    mnFocalLength(1000),
    mnShadowSlant(0),
    mxShadeMode(drawing::ShadeMode_SMOOTH),
    maAmbientColor(0x00666666),
    mbLightingMode(FALSE)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DSceneShapeContext::~SdXML3DSceneShapeContext()
{

    // release remembered light contexts, they are no longer needed
    while(maList.Count())
        maList.Remove(maList.Count() - 1)->ReleaseRef();
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSceneShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // read attributes for the 3DScene
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DSceneShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_3DSCENESHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_TRANSFORM:
            {
                SdXMLImExTransform3D aTransform(sValue, GetImport().GetMM100UnitConverter());
                if(aTransform.NeedsAction())
                    mbSetTransform = aTransform.GetFullHomogenTransform(mxHomMat);
                break;
            }
            case XML_TOK_3DSCENESHAPE_PROJECTION:
            {
                if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_parallel))))
                    mxPrjMode = drawing::ProjectionMode_PARALLEL;
                else
                    mxPrjMode = drawing::ProjectionMode_PERSPECTIVE;
                break;
            }
            case XML_TOK_3DSCENESHAPE_DISTANCE:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnDistance, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_FOCAL_LENGTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnFocalLength, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_SHADOW_SLANT:
            {
                GetImport().GetMM100UnitConverter().convertNumber(mnShadowSlant, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_SHADE_MODE:
            {
                if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_flat))))
                    mxShadeMode = drawing::ShadeMode_FLAT;
                else if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_phong))))
                    mxShadeMode = drawing::ShadeMode_PHONG;
                else if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_gouraud))))
                    mxShadeMode = drawing::ShadeMode_SMOOTH;
                else
                    mxShadeMode = drawing::ShadeMode_DRAFT;
                break;
            }
            case XML_TOK_3DSCENESHAPE_AMBIENT_COLOR:
            {
                GetImport().GetMM100UnitConverter().convertColor(maAmbientColor, sValue);
                break;
            }
            case XML_TOK_3DSCENESHAPE_LIGHTING_MODE:
            {
                GetImport().GetMM100UnitConverter().convertBool(mbLightingMode, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSceneShapeContext::EndElement()
{
    if(mxShapes.is())
    {
        uno::Reference< drawing::XShape > xShape(mxShapes, uno::UNO_QUERY);
        if(xShape.is())
        {
            // set local parameters on shape
            awt::Point aPoint(mnX, mnY);
            awt::Size aSize(mnWidth, mnHeight);
            xShape->setPosition(aPoint);
            xShape->setSize(aSize);

            uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
            if(xPropSet.is())
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
            }

            // call parent
            SvXMLImportContext::EndElement();
        }
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
        pContext = new SdXML3DLightContext(GetSdImport(), nPrefix, rLocalName, xAttrList);

        // remember SdXMLPresentationPlaceholderContext for later evaluation
        if(pContext)
        {
            pContext->AddRef();
            maList.Insert((SdXML3DLightContext*)pContext, LIST_APPEND);
        }
    }

    // call GroupChildContext function at common ShapeImport
    if(!pContext)
    {
        pContext = GetImport().GetShapeImport()->Create3DSceneChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxShapes);
    }

    // call parent when no own context was created
    if(!pContext)
    {
        pContext = SvXMLImportContext::CreateChildContext(
        nPrefix, rLocalName, xAttrList);
    }

    return pContext;
}


