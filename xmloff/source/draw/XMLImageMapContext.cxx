/*************************************************************************
 *
 *  $RCSfile: XMLImageMapContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-29 14:40:26 $
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

#ifndef _XMLOFF_XMLIMAGEMAPCONTEXT_HXX_
#include "XMLImageMapContext.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX
#include "XMLStringBufferImportContext.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::drawing::PointSequenceSequence;
using ::com::sun::star::document::XEventsSupplier;


enum XMLImageMapToken
{
    XML_TOK_IMAP_URL,
    XML_TOK_IMAP_DESCRIPTION,
    XML_TOK_IMAP_X,
    XML_TOK_IMAP_Y,
    XML_TOK_IMAP_CENTER_X,
    XML_TOK_IMAP_CENTER_Y,
    XML_TOK_IMAP_WIDTH,
    XML_TOK_IMAP_HEIGTH,
    XML_TOK_IMAP_POINTS,
    XML_TOK_IMAP_VIEWBOX,
    XML_TOK_IMAP_NOHREF,
    XML_TOK_IMAP_NAME,
    XML_TOK_IMAP_RADIUS,
    XML_TOK_IMAP_TARGET
};

static __FAR_DATA SvXMLTokenMapEntry aImageMapObjectTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  sXML_href,          XML_TOK_IMAP_URL            },
    { XML_NAMESPACE_OFFICE, sXML_name,          XML_TOK_IMAP_NAME           },
    { XML_NAMESPACE_DRAW,   sXML_nohref,        XML_TOK_IMAP_NOHREF         },
    { XML_NAMESPACE_SVG,    sXML_x,             XML_TOK_IMAP_X              },
    { XML_NAMESPACE_SVG,    sXML_y,             XML_TOK_IMAP_Y              },
    { XML_NAMESPACE_SVG,    sXML_cx,            XML_TOK_IMAP_CENTER_X       },
    { XML_NAMESPACE_SVG,    sXML_cy,            XML_TOK_IMAP_CENTER_Y       },
    { XML_NAMESPACE_SVG,    sXML_width,         XML_TOK_IMAP_WIDTH          },
    { XML_NAMESPACE_SVG,    sXML_height,        XML_TOK_IMAP_HEIGTH         },
    { XML_NAMESPACE_SVG,    sXML_r,             XML_TOK_IMAP_RADIUS         },
    { XML_NAMESPACE_SVG,    sXML_viewBox,       XML_TOK_IMAP_VIEWBOX        },
    { XML_NAMESPACE_DRAW,   sXML_points,        XML_TOK_IMAP_POINTS         },
    { XML_NAMESPACE_OFFICE, sXML_target_frame_name, XML_TOK_IMAP_TARGET     },
    XML_TOKEN_MAP_END
};



class XMLImageMapObjectContext : public SvXMLImportContext
{

protected:

    const ::rtl::OUString sBoundary;
    const ::rtl::OUString sCenter;
    const ::rtl::OUString sDescription;
    const ::rtl::OUString sImageMap;
    const ::rtl::OUString sIsActive;
    const ::rtl::OUString sName;
    const ::rtl::OUString sPolygon;
    const ::rtl::OUString sRadius;
    const ::rtl::OUString sTarget;
    const ::rtl::OUString sURL;

    ::rtl::OUString sServiceName;

    Reference<XIndexContainer> xImageMap;   /// the image map
    Reference<XPropertySet> xMapEntry;      /// one map-entry (one area)

    ::rtl::OUString sUrl;
    ::rtl::OUString sTargt;
    ::rtl::OUStringBuffer sDescriptionBuffer;
    ::rtl::OUString sNam;
    sal_Bool bIsActive;

    sal_Bool bValid;

public:
    TYPEINFO();

    XMLImageMapObjectContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexContainer> xMap,
        const sal_Char* pServiceName);

    void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    void EndElement();

    SvXMLImportContext *CreateChildContext(
        USHORT nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

protected:

    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const ::rtl::OUString& rValue);

    virtual void Prepare(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};


TYPEINIT1( XMLImageMapObjectContext, SvXMLImportContext );

XMLImageMapObjectContext::XMLImageMapObjectContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> xMap,
    const sal_Char* pServiceName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        xImageMap(xMap),
        bIsActive(sal_True),
        bValid(sal_False),
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
    DBG_ASSERT(NULL != pServiceName,
               "Please supply the image map object service name");

    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance(
            OUString::createFromAscii(pServiceName));
        DBG_ASSERT(xIfc.is(), "can't create image map object!");
        if( xIfc.is() )
        {
            Reference<XPropertySet> xPropertySet( xIfc, UNO_QUERY );

            xMapEntry = xPropertySet;
        }
        // else: can't create service -> ignore
    }
    // else: can't even get factory -> ignore
}

void XMLImageMapObjectContext::StartElement(
    const Reference<XAttributeList >& xAttrList )
{
    SvXMLTokenMap aMap(aImageMapObjectTokenMap);

    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sValue = xAttrList->getValueByIndex(nAttr);

        ProcessAttribute(
            (enum XMLImageMapToken)aMap.Get(nPrefix, sLocalName), sValue);
    }
}

void XMLImageMapObjectContext::EndElement()
{
    // only create and insert image map object if validity flag is set
    // (and we actually have an image map)
    if ( bValid && xImageMap.is() && xMapEntry.is() )
    {
        // set values
        Prepare( xMapEntry );

        // insert into image map
        Any aAny;
        aAny <<= xMapEntry;
        xImageMap->insertByIndex( xImageMap->getCount(), aAny );
    }
    // else: not valid -> don't create and insert
}

SvXMLImportContext* XMLImageMapObjectContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         (rLocalName.equalsAsciiL(sXML_events, sizeof(sXML_events)-1)) )
    {
        Reference<XEventsSupplier> xEvents( xMapEntry, UNO_QUERY );
        return new XMLEventsImportContext(
            GetImport(), nPrefix, rLocalName, xEvents);
    }
    else if ( (XML_NAMESPACE_SVG == nPrefix) &&
              (rLocalName.equalsAsciiL(sXML_desc, sizeof(sXML_desc)-1)) )
    {
        return new XMLStringBufferImportContext(
            GetImport(), nPrefix, rLocalName, sDescriptionBuffer);
    }
    else
        return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                      xAttrList);

}

void XMLImageMapObjectContext::ProcessAttribute(
    enum XMLImageMapToken eToken,
    const OUString& rValue)
{
    switch (eToken)
    {
        case XML_TOK_IMAP_URL:
            sUrl = rValue;
            break;

        case XML_TOK_IMAP_TARGET:
            sTargt = rValue;
            break;

        case XML_TOK_IMAP_NOHREF:
            bIsActive = ! rValue.equalsAsciiL(sXML_nohref,
                                              sizeof(sXML_nohref)-1);
            break;

        case XML_TOK_IMAP_NAME:
            sNam = rValue;
            break;
    }
}

void XMLImageMapObjectContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    Any aAny;

    aAny <<= sUrl;
    rPropertySet->setPropertyValue( sURL, aAny );

    aAny <<= sDescriptionBuffer.makeStringAndClear();
    rPropertySet->setPropertyValue( sDescription, aAny );

    aAny <<= sTargt;
    rPropertySet->setPropertyValue( sTarget, aAny );

    aAny.setValue( &bIsActive, ::getBooleanCppuType() );
    rPropertySet->setPropertyValue( sIsActive, aAny );

    aAny <<= sNam;
    rPropertySet->setPropertyValue( sName, aAny );
}



class XMLImageMapRectangleContext : public XMLImageMapObjectContext
{
    awt::Rectangle aRectangle;

    sal_Bool bXOK;
    sal_Bool bYOK;
    sal_Bool bWidthOK;
    sal_Bool bHeightOK;

public:
    TYPEINFO();

    XMLImageMapRectangleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexContainer> xMap);

    virtual ~XMLImageMapRectangleContext();

protected:
    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const ::rtl::OUString& rValue);

    virtual void Prepare(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};



TYPEINIT1(XMLImageMapRectangleContext, XMLImageMapObjectContext);

XMLImageMapRectangleContext::XMLImageMapRectangleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> xMap) :
        XMLImageMapObjectContext(rImport, nPrefix, rLocalName, xMap,
                                 "com.sun.star.image.ImageMapRectangleObject"),
        bXOK(sal_False),
        bYOK(sal_False),
        bWidthOK(sal_False),
        bHeightOK(sal_False)
{
}

XMLImageMapRectangleContext::~XMLImageMapRectangleContext()
{
}

void XMLImageMapRectangleContext::ProcessAttribute(
    enum XMLImageMapToken eToken,
    const OUString& rValue)
{
    sal_Int32 nTmp;
    switch (eToken)
    {
        case XML_TOK_IMAP_X:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                aRectangle.X = nTmp;
                bXOK = sal_True;
            }
            break;
        case XML_TOK_IMAP_Y:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                aRectangle.Y = nTmp;
                bYOK = sal_True;
            }
            break;
        case XML_TOK_IMAP_WIDTH:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                aRectangle.Width = nTmp;
                bWidthOK = sal_True;
            }
            break;
        case XML_TOK_IMAP_HEIGTH:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                aRectangle.Height = nTmp;
                bHeightOK = sal_True;
            }
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(eToken, rValue);
    }

    bValid = bHeightOK && bXOK && bYOK && bWidthOK;
}

void XMLImageMapRectangleContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    Any aAny;
    aAny <<= aRectangle;
    rPropertySet->setPropertyValue( sBoundary, aAny );

    // common properties handled by super class
    XMLImageMapObjectContext::Prepare(rPropertySet);
}


class XMLImageMapPolygonContext : public XMLImageMapObjectContext
{
    ::rtl::OUString sViewBoxString;
    ::rtl::OUString sPointsString;

    sal_Bool bViewBoxOK;
    sal_Bool bPointsOK;

public:
    TYPEINFO();

    XMLImageMapPolygonContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexContainer> xMap);

    virtual ~XMLImageMapPolygonContext();

protected:
    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const ::rtl::OUString& rValue);

    virtual void Prepare(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};



TYPEINIT1(XMLImageMapPolygonContext, XMLImageMapObjectContext);

XMLImageMapPolygonContext::XMLImageMapPolygonContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> xMap) :
        XMLImageMapObjectContext(rImport, nPrefix, rLocalName, xMap,
                                 "com.sun.star.image.ImageMapPolygonObject"),
        bViewBoxOK(sal_False),
        bPointsOK(sal_False)
{
}

XMLImageMapPolygonContext::~XMLImageMapPolygonContext()
{
}

void XMLImageMapPolygonContext::ProcessAttribute(
    enum XMLImageMapToken eToken,
    const OUString& rValue)
{
    switch (eToken)
    {
        case XML_TOK_IMAP_POINTS:
            sPointsString = rValue;
            bPointsOK = sal_True;
            break;
        case XML_TOK_IMAP_VIEWBOX:
            sViewBoxString = rValue;
            bViewBoxOK = sal_True;
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(eToken, rValue);
            break;
    }

    bValid = bViewBoxOK && bPointsOK;
}

void XMLImageMapPolygonContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    // process view box
    SdXMLImExViewBox aViewBox(sViewBoxString,
                              GetImport().GetMM100UnitConverter());

    // get polygon sequence
    awt::Point aPoint(aViewBox.GetX(), aViewBox.GetY());
    awt::Size aSize(aViewBox.GetWidth(), aViewBox.GetHeight());
    SdXMLImExPointsElement aPoints( sPointsString, aViewBox, aPoint, aSize,
                                    GetImport().GetMM100UnitConverter() );
    PointSequenceSequence aPointSeqSeq = aPoints.GetPointSequenceSequence();

    // only use first element of sequence-sequence
    if (aPointSeqSeq.getLength() > 0)
    {
        Any aAny;
        aAny <<= aPointSeqSeq[0];
        rPropertySet->setPropertyValue(sPolygon, aAny);
    }

    // parent properties
    XMLImageMapObjectContext::Prepare(rPropertySet);
}



class XMLImageMapCircleContext : public XMLImageMapObjectContext
{
    awt::Point aCenter;
    sal_Int32 nRadius;

    sal_Bool bXOK;
    sal_Bool bYOK;
    sal_Bool bRadiusOK;

public:
    TYPEINFO();

    XMLImageMapCircleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexContainer> xMap);

    virtual ~XMLImageMapCircleContext();

protected:
    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const ::rtl::OUString& rValue);

    virtual void Prepare(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};

TYPEINIT1(XMLImageMapCircleContext, XMLImageMapObjectContext);

XMLImageMapCircleContext::XMLImageMapCircleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> xMap) :
        XMLImageMapObjectContext(rImport, nPrefix, rLocalName, xMap,
                                 "com.sun.star.image.ImageMapCircleObject"),
        bXOK(sal_False),
        bYOK(sal_False),
        bRadiusOK(sal_False)
{
}

XMLImageMapCircleContext::~XMLImageMapCircleContext()
{
}

void XMLImageMapCircleContext::ProcessAttribute(
    enum XMLImageMapToken eToken,
    const OUString& rValue)
{
    sal_Int32 nTmp;
    switch (eToken)
    {
        case XML_TOK_IMAP_CENTER_X:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                aCenter.X = nTmp;
                bXOK = sal_True;
            }
            break;
        case XML_TOK_IMAP_CENTER_Y:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                aCenter.Y = nTmp;
                bYOK = sal_True;
            }
            break;
        case XML_TOK_IMAP_RADIUS:
            if (GetImport().GetMM100UnitConverter().convertMeasure(nTmp,
                                                                   rValue))
            {
                nRadius = nTmp;
                bRadiusOK = sal_True;
            }
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(eToken, rValue);
    }

    bValid = bRadiusOK && bXOK && bYOK;
}

void XMLImageMapCircleContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    // center (x,y)
    Any aAny;
    aAny <<= aCenter;
    rPropertySet->setPropertyValue( sCenter, aAny );

    // radius
    aAny <<= nRadius;
    rPropertySet->setPropertyValue( sRadius, aAny );

    // common properties handled by super class
    XMLImageMapObjectContext::Prepare(rPropertySet);
}










TYPEINIT1(XMLImageMapContext, SvXMLImportContext);

XMLImageMapContext::XMLImageMapContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropertySet) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        sImageMap(RTL_CONSTASCII_USTRINGPARAM("ImageMap")),
        xPropertySet(rPropertySet)

{
    Any aAny = xPropertySet->getPropertyValue(sImageMap);
    aAny >>= xImageMap;
}

XMLImageMapContext::~XMLImageMapContext()
{
}

SvXMLImportContext *XMLImageMapContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( rLocalName.equalsAsciiL(sXML_area_rectangle,
                                    sizeof(sXML_area_rectangle)-1) )
        {
            pContext = new XMLImageMapRectangleContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
        else if ( rLocalName.equalsAsciiL(sXML_area_polygon,
                                          sizeof(sXML_area_polygon)-1) )
        {
            pContext = new XMLImageMapPolygonContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
        else if ( rLocalName.equalsAsciiL(sXML_area_circle,
                                          sizeof(sXML_area_circle)-1) )
        {
            pContext = new XMLImageMapCircleContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
    }
    else
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);

    return pContext;
}

void XMLImageMapContext::EndElement()
{
    Any aAny;
    aAny <<= xImageMap;
    xPropertySet->setPropertyValue(sImageMap, aAny);
}

