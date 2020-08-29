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

#include "ximp3dobject.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xexptran.hxx>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;


SdXML3DObjectContext::SdXML3DObjectContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLShapeContext( rImport, nElement, xAttrList, rShapes, false/*bTemporaryShape*/ ),
    mbSetTransform( false )
{
    for(auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        const OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(DRAW, XML_STYLE_NAME):
            {
                maDrawStyleName = sValue;
                break;
            }
            case XML_ELEMENT(DR3D, XML_TRANSFORM):
            {
                SdXMLImExTransform3D aTransform(sValue, GetImport().GetMM100UnitConverter());
                if(aTransform.NeedsAction())
                    mbSetTransform = aTransform.GetFullHomogenTransform(mxHomMat);
                break;
            }
        }
    }
}

SdXML3DObjectContext::~SdXML3DObjectContext()
{
}

void SdXML3DObjectContext::startFastElement(sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList>& xAttrList)
{
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // set parameters
        if(mbSetTransform)
        {
            xPropSet->setPropertyValue("D3DTransformMatrix", uno::Any(mxHomMat));
        }

        // call parent
        SdXMLShapeContext::startFastElement(nElement, xAttrList);
    }
}

SdXML3DCubeObjectShapeContext::SdXML3DCubeObjectShapeContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXML3DObjectContext( rImport, nElement, xAttrList, rShapes ),
    maMinEdge(-2500.0, -2500.0, -2500.0),
    maMaxEdge(2500.0, 2500.0, 2500.0)
{
    for(auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();

        switch(aIter.getToken())
        {
            case XML_ELEMENT(DR3D, XML_MIN_EDGE):
            {
                ::basegfx::B3DVector aNewVec;
                SvXMLUnitConverter::convertB3DVector(aNewVec, sValue);

                if(aNewVec != maMinEdge)
                    maMinEdge = aNewVec;
                break;
            }
            case XML_ELEMENT(DR3D, XML_MAX_EDGE):
            {
                ::basegfx::B3DVector aNewVec;
                SvXMLUnitConverter::convertB3DVector(aNewVec, sValue);

                if(aNewVec != maMaxEdge)
                    maMaxEdge = aNewVec;
                break;
            }
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

SdXML3DCubeObjectShapeContext::~SdXML3DCubeObjectShapeContext()
{
}

void SdXML3DCubeObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    AddShape( "com.sun.star.drawing.Shape3DCubeObject" );
    if(!mxShape.is())
        return;

    // add, set style and properties from base shape
    SetStyle();
    SdXML3DObjectContext::StartElement(xAttrList);

    // set local parameters on shape
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // set parameters
    drawing::Position3D aPosition3D;
    drawing::Direction3D aDirection3D;

    // convert from min, max to size to be set
    maMaxEdge = maMaxEdge - maMinEdge;

    aPosition3D.PositionX = maMinEdge.getX();
    aPosition3D.PositionY = maMinEdge.getY();
    aPosition3D.PositionZ = maMinEdge.getZ();

    aDirection3D.DirectionX = maMaxEdge.getX();
    aDirection3D.DirectionY = maMaxEdge.getY();
    aDirection3D.DirectionZ = maMaxEdge.getZ();

    xPropSet->setPropertyValue("D3DPosition", uno::Any(aPosition3D));
    xPropSet->setPropertyValue("D3DSize", uno::Any(aDirection3D));
}

SdXML3DSphereObjectShapeContext::SdXML3DSphereObjectShapeContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXML3DObjectContext( rImport, nElement, xAttrList, rShapes ),
    maCenter(0.0, 0.0, 0.0),
    maSphereSize(5000.0, 5000.0, 5000.0)
{
    for(auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();

        switch(aIter.getToken())
        {
            case XML_ELEMENT(DR3D, XML_CENTER):
            {
                ::basegfx::B3DVector aNewVec;
                SvXMLUnitConverter::convertB3DVector(aNewVec, sValue);

                if(aNewVec != maCenter)
                    maCenter = aNewVec;
                break;
            }
            case XML_ELEMENT(DR3D, XML_SIZE):
            {
                ::basegfx::B3DVector aNewVec;
                SvXMLUnitConverter::convertB3DVector(aNewVec, sValue);

                if(aNewVec != maSphereSize)
                    maSphereSize = aNewVec;
                break;
            }
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

SdXML3DSphereObjectShapeContext::~SdXML3DSphereObjectShapeContext()
{
}

void SdXML3DSphereObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    AddShape( "com.sun.star.drawing.Shape3DSphereObject" );
    if(!mxShape.is())
        return;

    // add, set style and properties from base shape
    SetStyle();
    SdXML3DObjectContext::StartElement(xAttrList);

    // set local parameters on shape
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    // set parameters
    drawing::Position3D aPosition3D;
    drawing::Direction3D aDirection3D;

    aPosition3D.PositionX = maCenter.getX();
    aPosition3D.PositionY = maCenter.getY();
    aPosition3D.PositionZ = maCenter.getZ();

    aDirection3D.DirectionX = maSphereSize.getX();
    aDirection3D.DirectionY = maSphereSize.getY();
    aDirection3D.DirectionZ = maSphereSize.getZ();

    xPropSet->setPropertyValue("D3DPosition", uno::Any(aPosition3D));
    xPropSet->setPropertyValue("D3DSize", uno::Any(aDirection3D));
}

SdXML3DPolygonBasedShapeContext::SdXML3DPolygonBasedShapeContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXML3DObjectContext( rImport, nElement, xAttrList, rShapes )
{
    for(auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();

        switch(aIter.getToken())
        {
            case XML_ELEMENT(SVG, XML_VIEWBOX):
            {
                maViewBox = sValue;
                break;
            }
            case XML_ELEMENT(SVG, XML_D):
            {
                maPoints = sValue;
                break;
            }
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

SdXML3DPolygonBasedShapeContext::~SdXML3DPolygonBasedShapeContext()
{
}

void SdXML3DPolygonBasedShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);

    if(!xPropSet.is())
        return;

    // set parameters
    if(!maPoints.isEmpty() && !maViewBox.isEmpty())
    {
        // import 2d tools::PolyPolygon from svg:d
        basegfx::B2DPolyPolygon aPolyPolygon;

        if(basegfx::utils::importFromSvgD(aPolyPolygon, maPoints, GetImport().needFixPositionAfterZ(), nullptr))
        {
            // convert to 3D PolyPolygon
            const basegfx::B3DPolyPolygon aB3DPolyPolygon(
                basegfx::utils::createB3DPolyPolygonFromB2DPolyPolygon(
                    aPolyPolygon));

            // convert to UNO API class PolyPolygonShape3D
            drawing::PolyPolygonShape3D aPolyPolygon3D;
            basegfx::utils::B3DPolyPolygonToUnoPolyPolygonShape3D(
                aB3DPolyPolygon,
                aPolyPolygon3D);

            // set polygon data
            xPropSet->setPropertyValue("D3DPolyPolygon3D", uno::Any(aPolyPolygon3D));
        }
        else
        {
            OSL_ENSURE(false, "Error on importing svg:d for 3D tools::PolyPolygon (!)");
        }
    }

    // call parent
    SdXML3DObjectContext::StartElement(xAttrList);
}


SdXML3DLatheObjectShapeContext::SdXML3DLatheObjectShapeContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXML3DPolygonBasedShapeContext( rImport, nElement, xAttrList, rShapes )
{
}

SdXML3DLatheObjectShapeContext::~SdXML3DLatheObjectShapeContext()
{
}

void SdXML3DLatheObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    AddShape( "com.sun.star.drawing.Shape3DLatheObject" );
    if(mxShape.is())
    {
        // add, set style and properties from base shape
        SetStyle();
        SdXML3DPolygonBasedShapeContext::StartElement(xAttrList);
    }
}

SdXML3DExtrudeObjectShapeContext::SdXML3DExtrudeObjectShapeContext(
    SvXMLImport& rImport,
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXML3DPolygonBasedShapeContext( rImport, nElement, xAttrList, rShapes )
{
}

SdXML3DExtrudeObjectShapeContext::~SdXML3DExtrudeObjectShapeContext()
{
}

void SdXML3DExtrudeObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    AddShape( "com.sun.star.drawing.Shape3DExtrudeObject" );
    if(mxShape.is())
    {
        // add, set style and properties from base shape
        SetStyle();
        SdXML3DPolygonBasedShapeContext::StartElement(xAttrList);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
