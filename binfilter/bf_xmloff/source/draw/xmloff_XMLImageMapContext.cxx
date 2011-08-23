/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLIMAGEMAPCONTEXT_HXX_
#include "XMLImageMapContext.hxx"
#endif


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif



#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif



#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif



#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
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

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
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
namespace binfilter {


using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

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
    { XML_NAMESPACE_XLINK,	XML_HREF,			XML_TOK_IMAP_URL 			},
    { XML_NAMESPACE_OFFICE,	XML_NAME,			XML_TOK_IMAP_NAME			},
    { XML_NAMESPACE_DRAW,	XML_NOHREF, 		XML_TOK_IMAP_NOHREF			},
    { XML_NAMESPACE_SVG,	XML_X,				XML_TOK_IMAP_X				},
    { XML_NAMESPACE_SVG,	XML_Y,				XML_TOK_IMAP_Y				},
    { XML_NAMESPACE_SVG,	XML_CX,			    XML_TOK_IMAP_CENTER_X		},
    { XML_NAMESPACE_SVG,	XML_CY,			    XML_TOK_IMAP_CENTER_Y		},
    { XML_NAMESPACE_SVG,	XML_WIDTH,			XML_TOK_IMAP_WIDTH			},
    { XML_NAMESPACE_SVG,	XML_HEIGHT,		    XML_TOK_IMAP_HEIGTH			},
    { XML_NAMESPACE_SVG,	XML_R,				XML_TOK_IMAP_RADIUS			},
    { XML_NAMESPACE_SVG,	XML_VIEWBOX,		XML_TOK_IMAP_VIEWBOX		},
    { XML_NAMESPACE_SVG,	XML_POINTS,		    XML_TOK_IMAP_POINTS			},
    { XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME, XML_TOK_IMAP_TARGET		},
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

    Reference<XIndexContainer> xImageMap;	/// the image map
    Reference<XPropertySet> xMapEntry;		/// one map-entry (one area)

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
         IsXMLToken(rLocalName, XML_EVENTS) )
    {
        Reference<XEventsSupplier> xEvents( xMapEntry, UNO_QUERY );
        return new XMLEventsImportContext(
            GetImport(), nPrefix, rLocalName, xEvents);
    }
    else if ( (XML_NAMESPACE_SVG == nPrefix) &&
              IsXMLToken(rLocalName, XML_DESC) )
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
    try
    {
        xPropertySet->getPropertyValue(sImageMap) >>= xImageMap;
    }
    catch( ::com::sun::star::uno::Exception e )
    {
        uno::Sequence<OUString> aSeq(0);
        rImport.SetError( XMLERROR_FLAG_WARNING | XMLERROR_API, aSeq, e.Message, NULL );
    }
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
        if ( IsXMLToken(rLocalName, XML_AREA_RECTANGLE) )
        {
            pContext = new XMLImageMapRectangleContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
        else if ( IsXMLToken(rLocalName, XML_AREA_POLYGON) )
        {
            pContext = new XMLImageMapPolygonContext(
                GetImport(), nPrefix, rLocalName, xImageMap);
        }
        else if ( IsXMLToken(rLocalName, XML_AREA_CIRCLE) )
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

}//end of namespace binfilter
