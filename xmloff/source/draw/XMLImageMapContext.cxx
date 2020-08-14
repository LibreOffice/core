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
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xexptran.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <XMLStringBufferImportContext.hxx>
#include <tools/debug.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::document::XEventsSupplier;

namespace {

enum XMLImageMapToken: decltype(XML_TOK_UNKNOWN)
{
    XML_TOK_IMAP_URL,
    XML_TOK_IMAP_X,
    XML_TOK_IMAP_Y,
    XML_TOK_IMAP_CENTER_X,
    XML_TOK_IMAP_CENTER_Y,
    XML_TOK_IMAP_WIDTH,
    XML_TOK_IMAP_HEIGHT,
    XML_TOK_IMAP_POINTS,
    XML_TOK_IMAP_VIEWBOX,
    XML_TOK_IMAP_NOHREF,
    XML_TOK_IMAP_NAME,
    XML_TOK_IMAP_RADIUS,
    XML_TOK_IMAP_TARGET
};

}

const SvXMLTokenMapEntry aImageMapObjectTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  XML_HREF,           XML_TOK_IMAP_URL            },
    { XML_NAMESPACE_OFFICE, XML_NAME,           XML_TOK_IMAP_NAME           },
    { XML_NAMESPACE_DRAW,   XML_NOHREF,         XML_TOK_IMAP_NOHREF         },
    { XML_NAMESPACE_SVG,    XML_X,              XML_TOK_IMAP_X              },
    { XML_NAMESPACE_SVG,    XML_Y,              XML_TOK_IMAP_Y              },
    { XML_NAMESPACE_SVG,    XML_CX,             XML_TOK_IMAP_CENTER_X       },
    { XML_NAMESPACE_SVG,    XML_CY,             XML_TOK_IMAP_CENTER_Y       },
    { XML_NAMESPACE_SVG,    XML_WIDTH,          XML_TOK_IMAP_WIDTH          },
    { XML_NAMESPACE_SVG,    XML_HEIGHT,         XML_TOK_IMAP_HEIGHT         },
    { XML_NAMESPACE_SVG,    XML_R,              XML_TOK_IMAP_RADIUS         },
    { XML_NAMESPACE_SVG,    XML_VIEWBOX,        XML_TOK_IMAP_VIEWBOX        },
    { XML_NAMESPACE_DRAW,   XML_POINTS,         XML_TOK_IMAP_POINTS         },
    { XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME, XML_TOK_IMAP_TARGET      },
    XML_TOKEN_MAP_END
};

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
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        css::uno::Reference<css::container::XIndexContainer> const & xMap,
        const char* pServiceName);

    void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList >& xAttrList ) override;

    void EndElement() override;

    SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;

protected:

    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const OUString& rValue);

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet);
};

}

XMLImageMapObjectContext::XMLImageMapObjectContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> const & xMap,
    const char* pServiceName) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
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
    if( xIfc.is() )
    {
        Reference<XPropertySet> xPropertySet( xIfc, UNO_QUERY );

        xMapEntry = xPropertySet;
    }
    // else: can't create service -> ignore
    // else: can't even get factory -> ignore
}

void XMLImageMapObjectContext::StartElement(
    const Reference<XAttributeList >& xAttrList )
{
    static const SvXMLTokenMap aMap(aImageMapObjectTokenMap);

    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sValue = xAttrList->getValueByIndex(nAttr);

        ProcessAttribute(
            static_cast<enum XMLImageMapToken>(aMap.Get(nPrefix, sLocalName)), sValue);
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
        xImageMap->insertByIndex( xImageMap->getCount(), Any(xMapEntry) );
    }
    // else: not valid -> don't create and insert
}

SvXMLImportContextRef XMLImageMapObjectContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & /*xAttrList*/ )
{
    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_EVENT_LISTENERS) )
    {
        Reference<XEventsSupplier> xEvents( xMapEntry, UNO_QUERY );
        return new XMLEventsImportContext(
            GetImport(), nPrefix, rLocalName, xEvents);
    }
    else if ( (XML_NAMESPACE_SVG == nPrefix) &&
              IsXMLToken(rLocalName, XML_TITLE) )
    {
        return new XMLStringBufferImportContext(
            GetImport(), nPrefix, rLocalName, sTitleBuffer);
    }
    else if ( (XML_NAMESPACE_SVG == nPrefix) &&
              IsXMLToken(rLocalName, XML_DESC) )
    {
        return new XMLStringBufferImportContext(
            GetImport(), nPrefix, rLocalName, sDescriptionBuffer);
    }
    return nullptr;
}

void XMLImageMapObjectContext::ProcessAttribute(
    enum XMLImageMapToken eToken,
    const OUString& rValue)
{
    switch (eToken)
    {
        case XML_TOK_IMAP_URL:
            sUrl = GetImport().GetAbsoluteReference(rValue);
            break;

        case XML_TOK_IMAP_TARGET:
            sTargt = rValue;
            break;

        case XML_TOK_IMAP_NOHREF:
            bIsActive = ! IsXMLToken(rValue, XML_NOHREF);
            break;

        case XML_TOK_IMAP_NAME:
            sNam = rValue;
            break;
        default:
            // do nothing
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
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        css::uno::Reference<css::container::XIndexContainer> const & xMap);

protected:
    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const OUString& rValue) override;

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet) override;
};

}

XMLImageMapRectangleContext::XMLImageMapRectangleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> const & xMap) :
        XMLImageMapObjectContext(rImport, nPrefix, rLocalName, xMap,
                                 "com.sun.star.image.ImageMapRectangleObject"),
        bXOK(false),
        bYOK(false),
        bWidthOK(false),
        bHeightOK(false)
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
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                aRectangle.X = nTmp;
                bXOK = true;
            }
            break;
        case XML_TOK_IMAP_Y:
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                aRectangle.Y = nTmp;
                bYOK = true;
            }
            break;
        case XML_TOK_IMAP_WIDTH:
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                aRectangle.Width = nTmp;
                bWidthOK = true;
            }
            break;
        case XML_TOK_IMAP_HEIGHT:
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                aRectangle.Height = nTmp;
                bHeightOK = true;
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
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        css::uno::Reference<css::container::XIndexContainer> const & xMap);

protected:
    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const OUString& rValue) override;

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet) override;
};

}

XMLImageMapPolygonContext::XMLImageMapPolygonContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> const & xMap) :
        XMLImageMapObjectContext(rImport, nPrefix, rLocalName, xMap,
                                 "com.sun.star.image.ImageMapPolygonObject"),
        bViewBoxOK(false),
        bPointsOK(false)
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
            bPointsOK = true;
            break;
        case XML_TOK_IMAP_VIEWBOX:
            sViewBoxString = rValue;
            bViewBoxOK = true;
            break;
        default:
            XMLImageMapObjectContext::ProcessAttribute(eToken, rValue);
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
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        css::uno::Reference<css::container::XIndexContainer> const & xMap);

protected:
    virtual void ProcessAttribute(
        enum XMLImageMapToken eToken,
        const OUString& rValue) override;

    virtual void Prepare(
        css::uno::Reference<css::beans::XPropertySet> & rPropertySet) override;
};

}

XMLImageMapCircleContext::XMLImageMapCircleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XIndexContainer> const & xMap)
    : XMLImageMapObjectContext(rImport, nPrefix, rLocalName, xMap,
          "com.sun.star.image.ImageMapCircleObject")
    , nRadius(0)
    , bXOK(false)
    , bYOK(false)
    , bRadiusOK(false)
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
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                aCenter.X = nTmp;
                bXOK = true;
            }
            break;
        case XML_TOK_IMAP_CENTER_Y:
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                aCenter.Y = nTmp;
                bYOK = true;
            }
            break;
        case XML_TOK_IMAP_RADIUS:
            if (GetImport().GetMM100UnitConverter().convertMeasureToCore(nTmp,
                                                                   rValue))
            {
                nRadius = nTmp;
                bRadiusOK = true;
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
    rPropertySet->setPropertyValue( "Center", uno::Any(aCenter) );
    // radius
    rPropertySet->setPropertyValue( "Radius", uno::Any(nRadius) );

    // common properties handled by super class
    XMLImageMapObjectContext::Prepare(rPropertySet);
}


const OUStringLiteral gsImageMap("ImageMap");

XMLImageMapContext::XMLImageMapContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    Reference<XPropertySet> const & rPropertySet) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
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
        uno::Sequence<OUString> aSeq(0);
        rImport.SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, nullptr );
    }
}

XMLImageMapContext::~XMLImageMapContext()
{
}

SvXMLImportContextRef XMLImageMapContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & /*xAttrList*/ )
{
    SvXMLImportContextRef xContext;

    if ( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( IsXMLToken(rLocalName, XML_AREA_RECTANGLE) )
        {
            xContext = new XMLImageMapRectangleContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
        else if ( IsXMLToken(rLocalName, XML_AREA_POLYGON) )
        {
            xContext = new XMLImageMapPolygonContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
        else if ( IsXMLToken(rLocalName, XML_AREA_CIRCLE) )
        {
            xContext = new XMLImageMapCircleContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
    }

    return xContext;
}

void XMLImageMapContext::EndElement()
{
    Reference < XPropertySetInfo > xInfo =
        xPropertySet->getPropertySetInfo();
    if( xInfo.is() && xInfo->hasPropertyByName( gsImageMap ) )
        xPropertySet->setPropertyValue(gsImageMap, uno::makeAny( xImageMap ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
