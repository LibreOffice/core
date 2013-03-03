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

#include "XMLImageMapExport.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <xmloff/xmlexp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/xmluconv.hxx>
#include "xexptran.hxx"



using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::drawing::PointSequence;


const sal_Char sAPI_ImageMapRectangleObject[] = "com.sun.star.image.ImageMapRectangleObject";
const sal_Char sAPI_ImageMapCircleObject[] = "com.sun.star.image.ImageMapCircleObject";
const sal_Char sAPI_ImageMapPolygonObject[] = "com.sun.star.image.ImageMapPolygonObject";

XMLImageMapExport::XMLImageMapExport(SvXMLExport& rExp) :
    msBoundary("Boundary"),
    msCenter("Center"),
    msDescription("Description"),
    msImageMap("ImageMap"),
    msIsActive("IsActive"),
    msName("Name"),
    msPolygon("Polygon"),
    msRadius("Radius"),
    msTarget("Target"),
    msURL("URL"),
    msTitle("Title"),
    mrExport(rExp),
    mbWhiteSpace(sal_True)
{
}

XMLImageMapExport::~XMLImageMapExport()
{

}

void XMLImageMapExport::Export(
    const Reference<XPropertySet> & rPropertySet)
{
    if (rPropertySet->getPropertySetInfo()->hasPropertyByName(msImageMap))
    {
        Any aAny = rPropertySet->getPropertyValue(msImageMap);
        Reference<XIndexContainer> aContainer;
        aAny >>= aContainer;

        Export(aContainer);
    }
    // else: no ImageMap property -> nothing to do
}

void XMLImageMapExport::Export(
    const Reference<XIndexContainer> & rContainer)
{
    if (rContainer.is())
    {
        if (rContainer->hasElements())
        {
            // image map container element
            SvXMLElementExport aImageMapElement(
                mrExport, XML_NAMESPACE_DRAW, XML_IMAGE_MAP,
                mbWhiteSpace, mbWhiteSpace);

            // iterate over image map elements and call ExportMapEntry(...)
            // for each
            sal_Int32 nLength = rContainer->getCount();
            for(sal_Int32 i = 0; i < nLength; i++)
            {
                Any aAny = rContainer->getByIndex(i);
                Reference<XPropertySet> rElement;
                aAny >>= rElement;

                DBG_ASSERT(rElement.is(), "Image map element is empty!");
                if (rElement.is())
                {
                    ExportMapEntry(rElement);
                }
            }
        }
        // else: container is empty -> nothing to do
    }
    // else: no container -> nothing to do
}


void XMLImageMapExport::ExportMapEntry(
    const Reference<XPropertySet> & rPropertySet)
{
    Reference<XServiceInfo> xServiceInfo(rPropertySet, UNO_QUERY);
    if (xServiceInfo.is())
    {
        enum XMLTokenEnum eType = XML_TOKEN_INVALID;

        // distinguish map entries by their service name
        Sequence<OUString> sServiceNames =
            xServiceInfo->getSupportedServiceNames();
        sal_Int32 nLength = sServiceNames.getLength();
        for( sal_Int32 i=0; i<nLength; i++ )
        {
            OUString& rName = sServiceNames[i];

            if ( rName.equalsAsciiL(sAPI_ImageMapRectangleObject,
                                    sizeof(sAPI_ImageMapRectangleObject)-1) )
            {
                eType = XML_AREA_RECTANGLE;
                break;
            }
            else if ( rName.equalsAsciiL(sAPI_ImageMapCircleObject,
                                         sizeof(sAPI_ImageMapCircleObject)-1) )
            {
                eType = XML_AREA_CIRCLE;
                break;
            }
            else if ( rName.equalsAsciiL(sAPI_ImageMapPolygonObject,
                                         sizeof(sAPI_ImageMapPolygonObject)-1))
            {
                eType = XML_AREA_POLYGON;
                break;
            }
        }

        // return from method if no proper service is found!
        DBG_ASSERT(XML_TOKEN_INVALID != eType,
                   "Image map element doesn't support appropriate service!");
        if (XML_TOKEN_INVALID == eType)
            return;

        // now: handle ImageMapObject properties (those for all types)

        // XLINK (URL property)
        Any aAny = rPropertySet->getPropertyValue(msURL);
        OUString sHref;
        aAny >>= sHref;
        if (!sHref.isEmpty())
        {
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, mrExport.GetRelativeReference(sHref));
        }
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );

        // Target property (and xlink:show)
        aAny = rPropertySet->getPropertyValue(msTarget);
        OUString sTargt;
        aAny >>= sTargt;
        if (!sTargt.isEmpty())
        {
            mrExport.AddAttribute(
                XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME, sTargt);

            mrExport.AddAttribute(
                XML_NAMESPACE_XLINK, XML_SHOW,
                sTargt.equalsAsciiL( "_blank", sizeof("_blank")-1 )
                                        ? XML_NEW : XML_REPLACE );
        }

        // name
        aAny = rPropertySet->getPropertyValue(msName);
        OUString sItemName;
        aAny >>= sItemName;
        if (!sItemName.isEmpty())
        {
            mrExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_NAME, sItemName);
        }

        // is-active
        aAny = rPropertySet->getPropertyValue(msIsActive);
        if (! *(sal_Bool*)aAny.getValue())
        {
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_NOHREF, XML_NOHREF);
        }

        // call specific rectangle/circle/... method
        // also prepare element name
        switch (eType)
        {
            case XML_AREA_RECTANGLE:
                ExportRectangle(rPropertySet);
                break;
            case XML_AREA_CIRCLE:
                ExportCircle(rPropertySet);
                break;
            case XML_AREA_POLYGON:
                ExportPolygon(rPropertySet);
                break;
            default:
                break;
        }

        // write element
        DBG_ASSERT(XML_TOKEN_INVALID != eType,
                   "No name?! How did this happen?");
        SvXMLElementExport aAreaElement(mrExport, XML_NAMESPACE_DRAW, eType,
                                        mbWhiteSpace, mbWhiteSpace);

        // title property (as <svg:title> element)
        OUString sTitle;
        rPropertySet->getPropertyValue(msTitle) >>= sTitle;
        if(!sTitle.isEmpty())
        {
            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SVG, XML_TITLE, mbWhiteSpace, sal_False);
            mrExport.Characters(sTitle);
        }

        // description property (as <svg:desc> element)
        OUString sDescription;
        rPropertySet->getPropertyValue(msDescription) >>= sDescription;
        if (!sDescription.isEmpty())
        {
            SvXMLElementExport aDesc(mrExport, XML_NAMESPACE_SVG, XML_DESC, mbWhiteSpace, sal_False);
            mrExport.Characters(sDescription);
        }

        // export events attached to this
        Reference<XEventsSupplier> xSupplier(rPropertySet, UNO_QUERY);
        mrExport.GetEventExport().Export(xSupplier, mbWhiteSpace);
    }
    // else: no service info -> can't determine type -> ignore entry
}

void XMLImageMapExport::ExportRectangle(
    const Reference<XPropertySet> & rPropertySet)
{
    // get boundary rectangle
    Any aAny = rPropertySet->getPropertyValue(msBoundary);
    awt::Rectangle aRectangle;
    aAny >>= aRectangle;

    // parameters svg:x, svg:y, svg:width, svg:height
    OUStringBuffer aBuffer;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, aRectangle.X);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, aRectangle.Y);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer,
            aRectangle.Width);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer,
            aRectangle.Height);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT,
                          aBuffer.makeStringAndClear() );
}

void XMLImageMapExport::ExportCircle(
    const Reference<XPropertySet> & rPropertySet)
{
    // get boundary rectangle
    Any aAny = rPropertySet->getPropertyValue(msCenter);
    awt::Point aCenter;
    aAny >>= aCenter;

    // parameters svg:cx, svg:cy
    OUStringBuffer aBuffer;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, aCenter.X);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_CX,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, aCenter.Y);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_CY,
                          aBuffer.makeStringAndClear() );

    // radius
    aAny = rPropertySet->getPropertyValue(msRadius);
    sal_Int32 nRadius = 0;
    aAny >>= nRadius;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, nRadius);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_R,
                          aBuffer.makeStringAndClear() );
}

void XMLImageMapExport::ExportPolygon(
    const Reference<XPropertySet> & rPropertySet)
{
    // polygons get exported as bounding box, viewbox, and coordinate
    // pair sequence. The bounding box is always the entire image.

    // get polygon point sequence
    Any aAny = rPropertySet->getPropertyValue(msPolygon);
    PointSequence aPoly;
    aAny >>= aPoly;

    // get bounding box (assume top-left to be 0,0)
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;
    sal_Int32 nLength = aPoly.getLength();
    const struct awt::Point* pPointPtr = aPoly.getConstArray();
    for ( sal_Int32 i = 0; i < nLength; i++ )
    {
        sal_Int32 nPolyX = pPointPtr->X;
        sal_Int32 nPolyY = pPointPtr->Y;

        if ( nPolyX > nWidth )
            nWidth = nPolyX;
        if ( nPolyY > nHeight )
            nHeight = nPolyY;

        pPointPtr++;
    }
    DBG_ASSERT(nWidth > 0, "impossible Polygon found");
    DBG_ASSERT(nHeight > 0, "impossible Polygon found");

    // parameters svg:x, svg:y, svg:width, svg:height
    OUStringBuffer aBuffer;
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, 0);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, 0);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, nWidth);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasureToXML(aBuffer, nHeight);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT,
                          aBuffer.makeStringAndClear() );

    // svg:viewbox
    SdXMLImExViewBox aViewBox(0, 0, nWidth, nHeight);
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX,
                aViewBox.GetExportString());

    // export point sequence
    awt::Point aPoint(0, 0);
    awt::Size aSize(nWidth, nHeight);
    SdXMLImExPointsElement aPoints( &aPoly, aViewBox, aPoint, aSize );
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_POINTS,
                          aPoints.GetExportString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
