/*************************************************************************
 *
 *  $RCSfile: XMLImageMapExport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-30 13:53:39 $
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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
    rExport(rExp),
    bWhiteSpace(sal_True),
    sBoundary(RTL_CONSTASCII_USTRINGPARAM("Boundary")),
    sCenter(RTL_CONSTASCII_USTRINGPARAM("Center")),
    sDescription(RTL_CONSTASCII_USTRINGPARAM("Description")),
    sImageMap(RTL_CONSTASCII_USTRINGPARAM("ImageMap")),
    sIsActive(RTL_CONSTASCII_USTRINGPARAM("IsActive")),
    sName(RTL_CONSTASCII_USTRINGPARAM("Name")),
    sPolygon(RTL_CONSTASCII_USTRINGPARAM("Polygon")),
    sRadius(RTL_CONSTASCII_USTRINGPARAM("Radius")),
    sTarget(RTL_CONSTASCII_USTRINGPARAM("Target")),
    sURL(RTL_CONSTASCII_USTRINGPARAM("URL"))
{
}

XMLImageMapExport::~XMLImageMapExport()
{

}

void XMLImageMapExport::Export(
    const Reference<XPropertySet> & rPropertySet)
{
    if (rPropertySet->getPropertySetInfo()->hasPropertyByName(sImageMap))
    {
        Any aAny = rPropertySet->getPropertyValue(sImageMap);
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
                rExport,
                XML_NAMESPACE_DRAW, sXML_image_map,
                bWhiteSpace, bWhiteSpace);

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


enum MapEntryType
{
    RECTANGLE,
    CIRCLE,
    POLYGON,
    INVALID
};

void XMLImageMapExport::ExportMapEntry(
    const Reference<XPropertySet> & rPropertySet)
{
    OUString sAreaRectangle(RTL_CONSTASCII_USTRINGPARAM(sXML_area_rectangle));
    OUString sAreaCircle(RTL_CONSTASCII_USTRINGPARAM(sXML_area_circle));
    OUString sAreaPolygon(RTL_CONSTASCII_USTRINGPARAM(sXML_area_polygon));


    Reference<XServiceInfo> xServiceInfo(rPropertySet, UNO_QUERY);
    if (xServiceInfo.is())
    {
        enum MapEntryType eType = INVALID;

        // distinguish map entries by their service name
        Sequence<OUString> sServiceNames =
            xServiceInfo->getSupportedServiceNames();
        sal_Int32 nLength = sServiceNames.getLength();
        sal_Bool bFound = sal_False;
        for( sal_Int32 i=0; i<nLength; i++ )
        {
            OUString& rName = sServiceNames[i];

            if ( rName.equalsAsciiL(sAPI_ImageMapRectangleObject,
                                    sizeof(sAPI_ImageMapRectangleObject)-1) )
            {
                eType = RECTANGLE;
                break;
            }
            else if ( rName.equalsAsciiL(sAPI_ImageMapCircleObject,
                                         sizeof(sAPI_ImageMapCircleObject)-1) )
            {
                eType = CIRCLE;
                break;
            }
            else if ( rName.equalsAsciiL(sAPI_ImageMapPolygonObject,
                                         sizeof(sAPI_ImageMapPolygonObject)-1))
            {
                eType = POLYGON;
                break;
            }
        }

        // return from method if no proper service is found!
        DBG_ASSERT(INVALID != eType,
                   "Image map element doesn't support appropriate service!");
        if (INVALID == eType)
            return;

        // now: handle ImageMapObject properties (those for all types)

        // XLINK (URL property)
        Any aAny = rPropertySet->getPropertyValue(sURL);
        OUString sHref;
        aAny >>= sHref;
        if (sHref.getLength() > 0)
        {
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, sHref);
        }
        rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type,
                                   sXML_simple );

        // Target property (and xlink:show)
        aAny = rPropertySet->getPropertyValue(sTarget);
        OUString sTargt;
        aAny >>= sTargt;
        if (sTargt.getLength() > 0)
        {
            rExport.AddAttribute(
                XML_NAMESPACE_OFFICE, sXML_target_frame_name, sTargt);

            rExport.AddAttributeASCII(
                XML_NAMESPACE_XLINK, sXML_show,
                sTargt.equalsAsciiL( "_blank", sizeof("_blank")-1 )
                                        ? sXML_new : sXML_replace );
        }

        // name
        aAny = rPropertySet->getPropertyValue(sName);
        OUString sItemName;
        aAny >>= sItemName;
        if (sItemName.getLength() > 0)
        {
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_name, sItemName);
        }

        // is-active
        aAny = rPropertySet->getPropertyValue(sIsActive);
        if (! *(sal_Bool*)aAny.getValue())
        {
            rExport.AddAttributeASCII(XML_NAMESPACE_DRAW, sXML_nohref,
                                      sXML_nohref);
        }

        // call specific rectangle/circle/... method
        // also prepare element name
        OUString* pElementName = NULL;
        switch (eType)
        {
            case RECTANGLE:
                ExportRectangle(rPropertySet);
                pElementName = &sAreaRectangle;
                break;
            case CIRCLE:
                ExportCircle(rPropertySet);
                pElementName = &sAreaCircle;
                break;
            case POLYGON:
                ExportPolygon(rPropertySet);
                pElementName = &sAreaPolygon;
                break;
        }

        // write element
        DBG_ASSERT(NULL != pElementName, "No name?! How did this happen?");
        SvXMLElementExport aAreaElement(rExport,
                                        XML_NAMESPACE_DRAW, *pElementName,
                                        bWhiteSpace, bWhiteSpace);

        // description property (as <svg:desc> element)
        aAny = rPropertySet->getPropertyValue(sDescription);
        OUString sDescription;
        aAny >>= sDescription;
        if (sDescription.getLength() > 0)
        {
            SvXMLElementExport aDesc(rExport, XML_NAMESPACE_SVG, sXML_desc,
                                     bWhiteSpace, sal_False);
            rExport.GetDocHandler()->characters(sDescription);
        }

        // export events attached to this
        Reference<XEventsSupplier> xSupplier(rPropertySet, UNO_QUERY);
        rExport.GetEventExport().Export(xSupplier, bWhiteSpace);
    }
    // else: no service info -> can't determine type -> ignore entry
}

void XMLImageMapExport::ExportRectangle(
    const Reference<XPropertySet> & rPropertySet)
{
    // get boundary rectangle
    Any aAny = rPropertySet->getPropertyValue(sBoundary);
    awt::Rectangle aRectangle;
    aAny >>= aRectangle;

    // parameters svg:x, svg:y, svg:width, svg:height
    OUStringBuffer aBuffer;
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.X);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_x,
                          aBuffer.makeStringAndClear() );
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.Y);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_y,
                          aBuffer.makeStringAndClear() );
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.Width);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_width,
                          aBuffer.makeStringAndClear() );
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, aRectangle.Height);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_height,
                          aBuffer.makeStringAndClear() );
}

void XMLImageMapExport::ExportCircle(
    const Reference<XPropertySet> & rPropertySet)
{
    // get boundary rectangle
    Any aAny = rPropertySet->getPropertyValue(sCenter);
    awt::Point aCenter;
    aAny >>= aCenter;

    // parameters svg:cx, svg:cy
    OUStringBuffer aBuffer;
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, aCenter.X);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_cx,
                          aBuffer.makeStringAndClear() );
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, aCenter.Y);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_cy,
                          aBuffer.makeStringAndClear() );

    // radius
    aAny = rPropertySet->getPropertyValue(sRadius);
    sal_Int32 nRadius;
    aAny >>= nRadius;
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, nRadius);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_r,
                          aBuffer.makeStringAndClear() );
}

void XMLImageMapExport::ExportPolygon(
    const Reference<XPropertySet> & rPropertySet)
{
    // polygons get exported as bounding box, viewbox, and coordinate
    // pair sequence. The bounding box is always the entire image.

    // get polygon point sequence
    Any aAny = rPropertySet->getPropertyValue(sPolygon);
    PointSequence aPoly;
    aAny >>= aPoly;

    // get bounding box (assume top-left to be 0,0)
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;
    sal_Int32 nLength = aPoly.getLength();
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        sal_Int32 nPolyX = aPoly[i].X;
        sal_Int32 nPolyY = aPoly[i].Y;

        nWidth = (nPolyX > nWidth ) ? nPolyX : nWidth;
        nHeight = (nPolyY > nHeight ) ? nPolyY : nHeight;
    }
    DBG_ASSERT(nWidth > 0, "impossible Polygon found");
    DBG_ASSERT(nHeight > 0, "impossible Polygon found");

    // parameters svg:x, svg:y, svg:width, svg:height
    OUStringBuffer aBuffer;
    rExport.AddAttributeASCII( XML_NAMESPACE_SVG, sXML_x, "0" );
    rExport.AddAttributeASCII( XML_NAMESPACE_SVG, sXML_y, "0" );
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, nWidth);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_width,
                          aBuffer.makeStringAndClear() );
    rExport.GetMM100UnitConverter().convertMeasure(aBuffer, nHeight);
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_height,
                          aBuffer.makeStringAndClear() );

    // svg:viewbox
    SdXMLImExViewBox aViewBox(0, 0, nWidth, nHeight);
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_viewBox,
                aViewBox.GetExportString(rExport.GetMM100UnitConverter()));

    // export point sequence
    awt::Point aPoint(0, 0);
    awt::Size aSize(nWidth, nHeight);
    SdXMLImExPointsElement aPoints( &aPoly, aViewBox, aPoint, aSize,
                                    rExport.GetMM100UnitConverter() );
    rExport.AddAttribute( XML_NAMESPACE_SVG, sXML_points,
                          aPoints.GetExportString());
}
