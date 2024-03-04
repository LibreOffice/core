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

#include <XMLImageMapContext.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xexptran.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <XMLStringBufferImportContext.hxx>
#include <tools/debug.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::document::XEventsSupplier;

namespace {

class XMLImageMapObjectContext : public SvXMLImportContext
{

protected:

    Reference<XIndexContainer> xImageMap;   /// the image map
    Reference<XPropertySet> xMapEntry;      /// one map-entry (one area)

    OUString sUrl;
    OUString sTargt;
    OUStringBuffer sDescriptionBuffer;
    OUStringBuffer sTitleBuffer;
    OUString sNam;
    bool bIsActive;

    bool bValid;

public:

    XMLImageMapObjectContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::container::XIndexContainer> const & xMap,
        const char* pServiceName);

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

protected:

    virtual void ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter &);

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet);
};

}

XMLImageMapObjectContext::XMLImageMapObjectContext(
    SvXMLImport& rImport,
    Reference<XIndexContainer> const & xMap,
    const char* pServiceName) :
        SvXMLImportContext(rImport),
        xImageMap(xMap),
        bIsActive(true),
        bValid(false)
{
    DBG_ASSERT(nullptr != pServiceName,
               "Please supply the image map object service name");

    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( !xFactory.is() )
        return;

    Reference<XInterface> xIfc = xFactory->createInstance(
        OUString::createFromAscii(pServiceName));
    DBG_ASSERT(xIfc.is(), "can't create image map object!");
    if (xIfc.is())
        xMapEntry.set(xIfc, UNO_QUERY);
    // else: can't create service -> ignore
    // else: can't even get factory -> ignore
}

void XMLImageMapObjectContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        ProcessAttribute(aIter);
}

void XMLImageMapObjectContext::endFastElement(sal_Int32 )
{
    // only create and insert image map object if validity flag is set
    // (and we actually have an image map)
    if ( bValid && xImageMap.is() && xMapEntry.is() )
    {
        // set values
        Prepare( xMapEntry );

        // insert into image map
        xImageMap->insertByIndex( xImageMap->getCount(), Any(xMapEntry) );
    }
    // else: not valid -> don't create and insert
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImageMapObjectContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    switch (nElement)
    {
        case XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS):
        {
            Reference<XEventsSupplier> xEvents( xMapEntry, UNO_QUERY );
            return new XMLEventsImportContext(
                GetImport(), xEvents);
        }
        case XML_ELEMENT(SVG, XML_TITLE):
        case XML_ELEMENT(SVG_COMPAT, XML_TITLE):
            return new XMLStringBufferImportContext(
                GetImport(), sTitleBuffer);
        case XML_ELEMENT(SVG, XML_DESC):
        case XML_ELEMENT(SVG_COMPAT, XML_DESC):
            return new XMLStringBufferImportContext(
                GetImport(), sDescriptionBuffer);
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void XMLImageMapObjectContext::ProcessAttribute(
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(XLINK, XML_HREF):
            sUrl = GetImport().GetAbsoluteReference(aIter.toString());
            break;

        case XML_ELEMENT(OFFICE, XML_TARGET_FRAME_NAME):
            sTargt = aIter.toString();
            break;

        case XML_ELEMENT(DRAW, XML_NOHREF):
            bIsActive = ! IsXMLToken(aIter, XML_NOHREF);
            break;

        case XML_ELEMENT(OFFICE, XML_NAME):
            sNam = aIter.toString();
            break;
        default:
            // do nothing
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
            break;
    }
}

void XMLImageMapObjectContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    rPropertySet->setPropertyValue( "URL", Any( sUrl ) );
    rPropertySet->setPropertyValue( "Title", Any( sTitleBuffer.makeStringAndClear() ) );
    rPropertySet->setPropertyValue( "Description", Any( sDescriptionBuffer.makeStringAndClear() ) );
    rPropertySet->setPropertyValue( "Target", Any( sTargt ) );
    rPropertySet->setPropertyValue( "IsActive", Any( bIsActive ) );
    rPropertySet->setPropertyValue( "Name", Any( sNam ) );
}

namespace {

class XMLImageMapRectangleContext : public XMLImageMapObjectContext
{
    awt::Rectangle aRectangle;

    bool bXOK;
    bool bYOK;
    bool bWidthOK;
    bool bHeightOK;

public:

    XMLImageMapRectangleContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::container::XIndexContainer> const & xMap);

protected:
    virtual void ProcessAttribute(
        const sax_fastparser::FastAttributeList::FastAttributeIter &) override;

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet) override;
};

}

XMLImageMapRectangleContext::XMLImageMapRectangleContext(
    SvXMLImport& rImport,
    Reference<XIndexContainer> const & xMap) :
        XMLImageMapObjectContext(rImport, xMap,
                                 "com.sun.star.image.ImageMapRectangleObject"),
        bXOK(false),
        bYOK(false),
        bWidthOK(false),
        bHeightOK(false)
{
}

void XMLImageMapRectangleContext::ProcessAttribute(
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    sal_Int32 nTmp;
    switch (aIter.getToken())
    {
        case XML_ELEMENT(SVG, XML_X):
        case XML_ELEMENT(SVG_COMPAT, XML_X):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                aRectangle.X = nTmp;
                bXOK = true;
            }
            break;
        case  XML_ELEMENT(SVG, XML_Y):
        case  XML_ELEMENT(SVG_COMPAT, XML_Y):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                aRectangle.Y = nTmp;
                bYOK = true;
            }
            break;
        case XML_ELEMENT(SVG, XML_WIDTH):
        case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                aRectangle.Width = nTmp;
                bWidthOK = true;
            }
            break;
        case  XML_ELEMENT(SVG, XML_HEIGHT):
        case  XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                aRectangle.Height = nTmp;
                bHeightOK = true;
            }
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(aIter);
    }

    bValid = bHeightOK && bXOK && bYOK && bWidthOK;
}

void XMLImageMapRectangleContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    rPropertySet->setPropertyValue( "Boundary", uno::Any(aRectangle) );

    // common properties handled by super class
    XMLImageMapObjectContext::Prepare(rPropertySet);
}

namespace {

class XMLImageMapPolygonContext : public XMLImageMapObjectContext
{
    OUString sViewBoxString;
    OUString sPointsString;

    bool bViewBoxOK;
    bool bPointsOK;

public:

    XMLImageMapPolygonContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::container::XIndexContainer> const & xMap);

protected:
    virtual void ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter &) override;

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet) override;
};

}

XMLImageMapPolygonContext::XMLImageMapPolygonContext(
    SvXMLImport& rImport,
    Reference<XIndexContainer> const & xMap) :
        XMLImageMapObjectContext(rImport, xMap,
                                 "com.sun.star.image.ImageMapPolygonObject"),
        bViewBoxOK(false),
        bPointsOK(false)
{
}

void XMLImageMapPolygonContext::ProcessAttribute(
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(DRAW, XML_POINTS):
            sPointsString = aIter.toString();
            bPointsOK = true;
            break;
        case XML_ELEMENT(SVG, XML_VIEWBOX):
        case XML_ELEMENT(SVG_COMPAT, XML_VIEWBOX):
            sViewBoxString = aIter.toString();
            bViewBoxOK = true;
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(aIter);
            break;
    }

    bValid = bViewBoxOK && bPointsOK;
}

void XMLImageMapPolygonContext::Prepare(Reference<XPropertySet> & rPropertySet)
{
    // process view box
    SdXMLImExViewBox aViewBox(sViewBoxString, GetImport().GetMM100UnitConverter());

    // get polygon sequence
    basegfx::B2DPolygon aPolygon;

    if(basegfx::utils::importFromSvgPoints(aPolygon, sPointsString))
    {
        if(aPolygon.count())
        {
            css::drawing::PointSequence aPointSequence;
            basegfx::utils::B2DPolygonToUnoPointSequence(aPolygon, aPointSequence);
            rPropertySet->setPropertyValue("Polygon", Any(aPointSequence));
        }
    }

    // parent properties
    XMLImageMapObjectContext::Prepare(rPropertySet);
}

namespace {

class XMLImageMapCircleContext : public XMLImageMapObjectContext
{
    awt::Point aCenter;
    sal_Int32 nRadius;

    bool bXOK;
    bool bYOK;
    bool bRadiusOK;

public:

    XMLImageMapCircleContext(
        SvXMLImport& rImport,
        css::uno::Reference<css::container::XIndexContainer> const & xMap);

protected:
    virtual void ProcessAttribute(
        const sax_fastparser::FastAttributeList::FastAttributeIter &) override;

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet) override;
};

}

XMLImageMapCircleContext::XMLImageMapCircleContext(
    SvXMLImport& rImport,
    Reference<XIndexContainer> const & xMap)
    : XMLImageMapObjectContext(rImport, xMap,
          "com.sun.star.image.ImageMapCircleObject")
    , nRadius(0)
    , bXOK(false)
    , bYOK(false)
    , bRadiusOK(false)
{
}

void XMLImageMapCircleContext::ProcessAttribute(
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    sal_Int32 nTmp;
    switch (aIter.getToken())
    {
        case XML_ELEMENT(SVG, XML_CX):
        case XML_ELEMENT(SVG_COMPAT, XML_CX):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                aCenter.X = nTmp;
                bXOK = true;
            }
            break;
        case XML_ELEMENT(SVG, XML_CY):
        case XML_ELEMENT(SVG_COMPAT, XML_CY):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                aCenter.Y = nTmp;
                bYOK = true;
            }
            break;
        case XML_ELEMENT(SVG, XML_R):
        case XML_ELEMENT(SVG_COMPAT, XML_R):
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   aIter.toView()))
            {
                nRadius = nTmp;
                bRadiusOK = true;
            }
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(aIter);
    }

    bValid = bRadiusOK && bXOK && bYOK;
}

void XMLImageMapCircleContext::Prepare(
    Reference<XPropertySet> & rPropertySet)
{
    // center (x,y)
    rPropertySet->setPropertyValue( "Center", uno::Any(aCenter) );
    // radius
    rPropertySet->setPropertyValue( "Radius", uno::Any(nRadius) );

    // common properties handled by super class
    XMLImageMapObjectContext::Prepare(rPropertySet);
}


constexpr OUString gsImageMap(u"ImageMap"_ustr);

XMLImageMapContext::XMLImageMapContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> const & rPropertySet) :
        SvXMLImportContext(rImport),
        xPropertySet(rPropertySet)
{
    try
    {
        Reference < XPropertySetInfo > xInfo =
            xPropertySet->getPropertySetInfo();
        if( xInfo.is() && xInfo->hasPropertyByName( gsImageMap ) )
            xPropertySet->getPropertyValue(gsImageMap) >>= xImageMap;
    }
    catch(const css::uno::Exception& e)
    {
        rImport.SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, {}, e.Message, nullptr );
    }
}

XMLImageMapContext::~XMLImageMapContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImageMapContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    switch (nElement)
    {
        case XML_ELEMENT(DRAW, XML_AREA_RECTANGLE):
            return new XMLImageMapRectangleContext(
                GetImport(), xImageMap);
        case XML_ELEMENT(DRAW, XML_AREA_POLYGON):
            return new XMLImageMapPolygonContext(
                GetImport(), xImageMap);
        case XML_ELEMENT(DRAW, XML_AREA_CIRCLE):
            return new XMLImageMapCircleContext(
                GetImport(), xImageMap);
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return nullptr;
}

void XMLImageMapContext::endFastElement(sal_Int32 )
{
    Reference < XPropertySetInfo > xInfo =
        xPropertySet->getPropertySetInfo();
    if( xInfo.is() && xInfo->hasPropertyByName( gsImageMap ) )
        xPropertySet->setPropertyValue(gsImageMap, uno::Any( xImageMap ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
