/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLImageMapExport.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:32:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_XMLIMAGEMAPEXPORT_HXX_
#include "XMLImageMapExport.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif



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
    msBoundary(RTL_CONSTASCII_USTRINGPARAM("Boundary")),
    msCenter(RTL_CONSTASCII_USTRINGPARAM("Center")),
    msDescription(RTL_CONSTASCII_USTRINGPARAM("Description")),
    msImageMap(RTL_CONSTASCII_USTRINGPARAM("ImageMap")),
    msIsActive(RTL_CONSTASCII_USTRINGPARAM("IsActive")),
    msName(RTL_CONSTASCII_USTRINGPARAM("Name")),
    msPolygon(RTL_CONSTASCII_USTRINGPARAM("Polygon")),
    msRadius(RTL_CONSTASCII_USTRINGPARAM("Radius")),
    msTarget(RTL_CONSTASCII_USTRINGPARAM("Target")),
    msURL(RTL_CONSTASCII_USTRINGPARAM("URL")),
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
    // else: no container -> nothign to do
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
        if (sHref.getLength() > 0)
        {
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, mrExport.GetRelativeReference(sHref));
        }
        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );

        // Target property (and xlink:show)
        aAny = rPropertySet->getPropertyValue(msTarget);
        OUString sTargt;
        aAny >>= sTargt;
        if (sTargt.getLength() > 0)
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
        if (sItemName.getLength() > 0)
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

        // description property (as <svg:desc> element)
        aAny = rPropertySet->getPropertyValue(msDescription);
        OUString sDescription;
        aAny >>= sDescription;
        if (sDescription.getLength() > 0)
        {
            SvXMLElementExport aDesc(mrExport, XML_NAMESPACE_SVG, XML_DESC,
                                     mbWhiteSpace, sal_False);
            mrExport.GetDocHandler()->characters(sDescription);
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
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.X);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.Y);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.Width);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.Height);
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
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, aCenter.X);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_CX,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, aCenter.Y);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_CY,
                          aBuffer.makeStringAndClear() );

    // radius
    aAny = rPropertySet->getPropertyValue(msRadius);
    sal_Int32 nRadius = 0;
    aAny >>= nRadius;
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, nRadius);
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
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, 0);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, 0);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, nWidth);
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,
                          aBuffer.makeStringAndClear() );
    mrExport.GetMM100UnitConverter().convertMeasure(aBuffer, nHeight);
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
