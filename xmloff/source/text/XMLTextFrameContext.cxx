/*************************************************************************
 *
 *  $RCSfile: XMLTextFrameContext.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-21 13:40:51 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
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
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLANCHORTYPEPROPHDL_HXX
#include "XMLAnchorTypePropHdl.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif
#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif
#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include "shapeimport.hxx"
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif

#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using ::com::sun::star::document::XEventsSupplier;

class XMLTextFrameDescContext_Impl : public SvXMLImportContext
{
    OUString&   rDesc;

public:

    TYPEINFO();

    XMLTextFrameDescContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                  const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            OUString& rD );
    virtual ~XMLTextFrameDescContext_Impl();

    virtual void Characters( const OUString& rText );
};

TYPEINIT1( XMLTextFrameDescContext_Impl, SvXMLImportContext );

XMLTextFrameDescContext_Impl::XMLTextFrameDescContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        OUString& rD  ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rDesc( rD )
{
}

XMLTextFrameDescContext_Impl::~XMLTextFrameDescContext_Impl()
{
}

void XMLTextFrameDescContext_Impl::Characters( const OUString& rText )
{
    rDesc += rText;
}

// ------------------------------------------------------------------------

class XMLTextFrameParam_Impl : public SvXMLImportContext
{
public:

    TYPEINFO();

    XMLTextFrameParam_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                  const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nType,
            ParamMap &rParamMap);
    virtual ~XMLTextFrameParam_Impl();
};
TYPEINIT1( XMLTextFrameParam_Impl, SvXMLImportContext );
XMLTextFrameParam_Impl::~XMLTextFrameParam_Impl()
{
}
XMLTextFrameParam_Impl::XMLTextFrameParam_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                  const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nType,
            ParamMap &rParamMap):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString sName, sValue;
    sal_Bool bFoundValue = sal_False; // to allow empty values
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
        if ( XML_NAMESPACE_DRAW == nPrefix && aLocalName.equalsAsciiL( sXML_value, sizeof(sXML_value) -1 ) )
        {
            sValue = rValue;
            bFoundValue=sal_True;
        }
        else if ( XML_NAMESPACE_OFFICE == nPrefix && aLocalName.equalsAsciiL( sXML_name, sizeof( sXML_name) -1 ) )
            sName = rValue;
    }
    if (sName.getLength() && bFoundValue )
        rParamMap[sName] = sValue;
}
class XMLTextFrameContourContext_Impl : public SvXMLImportContext
{
    Reference < XPropertySet > xPropSet;

public:

    TYPEINFO();

    XMLTextFrameContourContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                  const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const Reference < XPropertySet >& rPropSet,
            sal_Bool bPath );
    virtual ~XMLTextFrameContourContext_Impl();
};

TYPEINIT1( XMLTextFrameContourContext_Impl, SvXMLImportContext );

XMLTextFrameContourContext_Impl::XMLTextFrameContourContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        const Reference < XPropertySet >& rPropSet,
        sal_Bool bPath ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xPropSet( rPropSet )
{
    OUString sD, sPoints, sViewBox;
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;

    const SvXMLTokenMap& rTokenMap =
        bPath ? GetImport().GetShapeImport()->GetPathShapeAttrTokenMap()
              : GetImport().GetShapeImport()->GetPolygonShapeAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( bPath )
        {
            switch( rTokenMap.Get( nPrefix, aLocalName ) )
            {
            case XML_TOK_PATHSHAPE_VIEWBOX:
                sViewBox = rValue;
                break;
            case XML_TOK_PATHSHAPE_D:
                sD = rValue;
                break;
//          case XML_TOK_PATHSHAPE_WIDTH:
//              GetImport().GetMM100UnitConverter().convertMeasure( nWidth,
//                                                                  rValue);
//              break;
//          case XML_TOK_PATHSHAPE_HEIGHT:
//              GetImport().GetMM100UnitConverter().convertMeasure( nHeight,
//                                                                  rValue);
//              break;
            }
        }
        else
        {
            switch( rTokenMap.Get( nPrefix, aLocalName ) )
            {
            case XML_TOK_POLYGONSHAPE_VIEWBOX:
                sViewBox = rValue;
                break;
            case XML_TOK_POLYGONSHAPE_POINTS:
                sPoints = rValue;
                break;
//          case XML_TOK_POLYGONSHAPE_WIDTH:
//              GetImport().GetMM100UnitConverter().convertMeasure( nWidth,
//                                                                  rValue);
//              break;
//          case XML_TOK_POLYGONSHAPE_HEIGHT:
//              GetImport().GetMM100UnitConverter().convertMeasure( nHeight,
//                                                                  rValue);
//              break;
            }
        }
    }

    OUString sContourPolyPolygon(
            RTL_CONSTASCII_USTRINGPARAM("ContourPolyPolygon") );
    if( rPropSet->getPropertySetInfo()->hasPropertyByName(
                                                    sContourPolyPolygon ) &&
        nWidth > 0 && nHeight > 0 && (bPath ? sD : sPoints).getLength() )
    {
        awt::Point aPoint( 0,  0 );
        awt::Size aSize( nWidth, nHeight );
        SdXMLImExViewBox aViewBox( sViewBox,
                                   GetImport().GetMM100UnitConverter());
        Any aAny;
        if( bPath )
        {
            SdXMLImExSvgDElement aPoints( sD, aViewBox, aPoint, aSize,
                                          GetImport().GetMM100UnitConverter() );
            aAny <<= aPoints.GetPointSequenceSequence();
        }
        else
        {
            SdXMLImExPointsElement aPoints( sPoints, aViewBox, aPoint, aSize,
                                        GetImport().GetMM100UnitConverter() );
            aAny <<= aPoints.GetPointSequenceSequence();
        }

        xPropSet->setPropertyValue( sContourPolyPolygon, aAny );
    }
}

XMLTextFrameContourContext_Impl::~XMLTextFrameContourContext_Impl()
{
}


// ------------------------------------------------------------------------

TYPEINIT1( XMLTextFrameContext, SvXMLImportContext );

XMLTextFrameContext::XMLTextFrameContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nNewType ) :
    nType( nNewType ),
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sWidth(RTL_CONSTASCII_USTRINGPARAM("Width")),
    sRelativeWidth(RTL_CONSTASCII_USTRINGPARAM("RelativeWidth")),
    sHeight(RTL_CONSTASCII_USTRINGPARAM("Height")),
    sRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("RelativeHeight")),
    sSizeType(RTL_CONSTASCII_USTRINGPARAM("SizeType")),
    sIsSyncWidthToHeight(RTL_CONSTASCII_USTRINGPARAM("IsSyncWidthToHeight")),
    sIsSyncHeightToWidth(RTL_CONSTASCII_USTRINGPARAM("IsSyncHeightToWidth")),
    sHoriOrientPosition(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")),
    sVertOrientPosition(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")),
    sChainNextName(RTL_CONSTASCII_USTRINGPARAM("ChainNextName")),
    sAnchorType(RTL_CONSTASCII_USTRINGPARAM("AnchorType")),
    sAnchorPageNo(RTL_CONSTASCII_USTRINGPARAM("AnchorPageNo")),
    sGraphicURL(RTL_CONSTASCII_USTRINGPARAM("GraphicURL")),
    sGraphicFilter(RTL_CONSTASCII_USTRINGPARAM("GraphicFilter")),
    sAlternativeText(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")),
    sFrameStyleName(RTL_CONSTASCII_USTRINGPARAM("FrameStyleName")),
    sGraphicRotation(RTL_CONSTASCII_USTRINGPARAM("GraphicRotation")),
    sTextBoxServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame")),
    sGraphicServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GraphicObject"))
{
    OUString    sName;
    OUString    sStyleName;
    OUString    sChainNextName;
    OUString    sHRef;
    OUString    sFilterName;
    OUString    sCode;
    OUString    sObject;
    OUString    sArchive;
    OUString    sMimeType;
    OUString    sFrameName;
    OUString    sAppletName;

    sal_Int32   nX = 0;
    sal_Int32   nY = 0;
    sal_Int32   nWidth = 0;
    sal_Int32   nHeight = 0;
    sal_Int32   nZIndex = -1;
    sal_Int16   nPage = 0;
    sal_Int16   nRotation = 0;
    sal_Int8    nRelWidth = 0;
    sal_Int8    nRelHeight = 0;
    sal_Bool    bMayScript = sal_False;

    TextContentAnchorType   eAnchorType = eATyp;

    sal_Bool    bMinHeight = sal_False;
    sal_Bool    bSyncWidth = sal_False;
    sal_Bool    bSyncHeight = sal_False;
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    const SvXMLTokenMap& rTokenMap =
        xTxtImport->GetTextFrameAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_FRAME_STYLE_NAME:
            sStyleName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_NAME:
            sName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_FRAME_NAME:
            sFrameName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_APPLET_NAME:
            sAppletName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_ANCHOR_TYPE:
            if( TextContentAnchorType_AT_PARAGRAPH == eAnchorType ||
                TextContentAnchorType_AT_CHARACTER == eAnchorType ||
                TextContentAnchorType_AS_CHARACTER == eAnchorType )
            {

                TextContentAnchorType eNew;
                if( XMLAnchorTypePropHdl::convert( rValue,
                            GetImport().GetMM100UnitConverter(), eNew ) &&
                    ( TextContentAnchorType_AT_PARAGRAPH == eNew ||
                      TextContentAnchorType_AT_CHARACTER == eNew ||
                      TextContentAnchorType_AS_CHARACTER == eNew ||
                      TextContentAnchorType_AT_PAGE == eNew) )
                    eAnchorType = eNew;
            }
            break;
        case XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER:
            {
                sal_Int32 nTmp;
                   if( GetImport().GetMM100UnitConverter().
                                convertNumber( nTmp, rValue, 1, SHRT_MAX ) )
                    nPage = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_X:
            GetImport().GetMM100UnitConverter().convertMeasure( nX, rValue );
            break;
        case XML_TOK_TEXT_FRAME_Y:
            GetImport().GetMM100UnitConverter().convertMeasure( nY, rValue );
            break;
        case XML_TOK_TEXT_FRAME_WIDTH:
            // relative widths are obsolete since SRC617. Remove them some day!
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                GetImport().GetMM100UnitConverter().convertPercent( nTmp,
                                                                    rValue );
                nRelWidth = (sal_Int8)nTmp;
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasure( nWidth,
                                                                    rValue, 0 );
            }
            break;
        case XML_TOK_TEXT_FRAME_REL_WIDTH:
            if( rValue.equalsAsciiL( sXML_scale, sizeof(sXML_scale)-1 ) )
            {
                bSyncWidth = sal_True;
            }
            else
            {
                sal_Int32 nTmp;
                if( GetImport().GetMM100UnitConverter().
                        convertPercent( nTmp, rValue ) )
                    nRelWidth = (sal_Int8)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_HEIGHT:
            // relative heights are obsolete since SRC617. Remove them some day!
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                GetImport().GetMM100UnitConverter().convertPercent( nTmp,
                                                                    rValue );
                nRelHeight = (sal_Int8)nTmp;
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasure( nHeight,
                                                                    rValue, 0 );
            }
            break;
        case XML_TOK_TEXT_FRAME_REL_HEIGHT:
            if( rValue.equalsAsciiL( sXML_scale, sizeof(sXML_scale)-1 ) )
            {
                bSyncHeight = sal_True;
            }
            else if( rValue.equalsAsciiL( sXML_scale_min,
                                          sizeof(sXML_scale_min)-1 ) )
            {
                bSyncHeight = sal_True;
                bMinHeight = sal_True;
            }
            else
            {
                sal_Int32 nTmp;
                if( GetImport().GetMM100UnitConverter().
                        convertPercent( nTmp, rValue ) )
                    nRelHeight = (sal_Int8)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_HEIGHT:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                GetImport().GetMM100UnitConverter().convertPercent( nTmp,
                                                                    rValue );
                nRelHeight = (sal_Int8)nTmp;
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasure( nHeight,
                                                                    rValue, 0 );
            }
            bMinHeight = sal_True;
            break;
        case XML_TOK_TEXT_FRAME_Z_INDEX:
            GetImport().GetMM100UnitConverter().convertNumber( nZIndex, rValue, -1 );
            break;
        case XML_TOK_TEXT_FRAME_NEXT_CHAIN_NAME:
            sChainNextName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_HREF:
            sHRef = rValue;
            break;
        case XML_TOK_TEXT_FRAME_FILTER_NAME:
            sFilterName = rValue;
            break;
        case XML_TOK_TEXT_FRAME_TRANSFORM:
            {
                OUString sValue( rValue );
                sValue.trim();
                const sal_Int32 nRotateLen = sizeof(sXML_rotate)-1;
                sal_Int32 nLen = sValue.getLength();
                if( nLen >= nRotateLen+3 &&
                    0 == sValue.compareToAscii( sXML_rotate, nRotateLen ) &&
                    '(' == sValue[nRotateLen] &&
                    ')' == sValue[nLen-1] )
                {
                    sValue = sValue.copy( nRotateLen+1, nLen-(nRotateLen+2) );
                    sValue.trim();
                    sal_Int32 nVal;
                    if( GetImport().GetMM100UnitConverter().convertNumber( nVal, sValue ) )
                        nRotation = (sal_Int16)(nVal % 360 );
                }
            }
            break;
        case XML_TOK_TEXT_FRAME_CODE:
            sCode = rValue;
            break;
        case XML_TOK_TEXT_FRAME_OBJECT:
            sObject = rValue;
            break;
        case XML_TOK_TEXT_FRAME_ARCHIVE:
            sArchive = rValue;
            break;
        case XML_TOK_TEXT_FRAME_MAY_SCRIPT:
            if ( rValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( sXML_true ) ) )
                bMayScript = sal_True;
            else
                bMayScript = sal_False;
            break;
        case XML_TOK_TEXT_FRAME_MIME_TYPE:
            sMimeType = rValue;
            break;
        }
    }

    if( (XML_TEXT_FRAME_APPLET  == nType || XML_TEXT_FRAME_PLUGIN == nType ||
         XML_TEXT_FRAME_GRAPHIC == nType || XML_TEXT_FRAME_OBJECT == nType ||
         XML_TEXT_FRAME_OBJECT_OLE    == nType)
        && !sHRef.getLength() )
        return; // no URL: no image or OLE object

    switch ( nType)
    {
        case XML_TEXT_FRAME_OBJECT:
        case XML_TEXT_FRAME_OBJECT_OLE:
        {
            OUString sURL( GetImport().ResolveEmbeddedObjectURL( sHRef, OUString() ) );

            if( sURL.getLength() )
                xPropSet = GetImport().GetTextImport()
                        ->createAndInsertOLEObject( GetImport(), sURL,
                                                    sStyleName,
                                                    nWidth, nHeight );
            break;
        }
        case XML_TEXT_FRAME_APPLET:
        {
            xPropSet = GetImport().GetTextImport()
                            ->createAndInsertApplet( sAppletName, sCode,
                                                     bMayScript, sHRef,
                                                     nWidth, nHeight);
            break;
        }
        case XML_TEXT_FRAME_PLUGIN:
        {
            xPropSet = GetImport().GetTextImport()
                            ->createAndInsertPlugin( sMimeType, sHRef,
                                                         nWidth, nHeight);

            break;
        }
        case XML_TEXT_FRAME_FLOATING_FRAME:
        {
            xPropSet = GetImport().GetTextImport()
                            ->createAndInsertFloatingFrame( sFrameName, sHRef,
                                                            sStyleName,
                                                            nWidth, nHeight);
            break;
        }
        default:
        {
            Reference<XMultiServiceFactory> xFactory( GetImport().GetModel(),
                                                      UNO_QUERY );
            if( xFactory.is() )
            {
                OUString sServiceName;
                switch( nType )
                {
                    case XML_TEXT_FRAME_TEXTBOX: sServiceName = sTextBoxServiceName; break;
                    case XML_TEXT_FRAME_GRAPHIC: sServiceName = sGraphicServiceName; break;
                }
                Reference<XInterface> xIfc = xFactory->createInstance( sServiceName );
                DBG_ASSERT( xIfc.is(), "couldn't create frame" );
                if( xIfc.is() )
                    xPropSet = Reference < XPropertySet >( xIfc, UNO_QUERY );
            }
        }
    }

    if( !xPropSet.is() )
        return;

    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // set name
    Reference < XNamed > xNamed( xPropSet, UNO_QUERY );
    if( xNamed.is() )
    {
        OUString sOrigName( xNamed->getName() );
        if( !sOrigName.getLength() ||
            (sName.getLength() && sOrigName != sName) )
        {
            OUString sOldName( sName );
            sal_Int32 i = 0;
            while( xTxtImport->HasFrameByName( sName ) )
            {
                sName = sOldName;
                sName += OUString::valueOf( ++i );
            }
            xNamed->setName( sName );
            if( sName != sOldName )
                xTxtImport->GetRenameMap().Add( XML_TEXT_RENAME_TYPE_FRAME,
                                             sOldName, sName );
        }
    }

    // frame style
    XMLPropStyleContext *pStyle = 0;
    if( sStyleName.getLength() )
    {
        pStyle = xTxtImport->FindAutoFrameStyle( sStyleName );
        if( pStyle )
            sStyleName = pStyle->GetParent();
    }

    Any aAny;
    if( sStyleName.getLength() )
    {
        const Reference < XNameContainer > & rStyles =
            xTxtImport->GetFrameStyles();
        if( rStyles.is() &&
            rStyles->hasByName( sStyleName ) )
        {
            aAny <<= sStyleName;
            xPropSet->setPropertyValue( sFrameStyleName, aAny );
        }
    }

    // hard properties
    if( pStyle )
        pStyle->FillPropertySet( xPropSet );

    // anchor type
    aAny <<= eAnchorType;
    xPropSet->setPropertyValue( sAnchorType, aAny );


    // x and y
    aAny <<= nX;
    xPropSet->setPropertyValue( sHoriOrientPosition, aAny );
    aAny <<= nY;
    xPropSet->setPropertyValue( sVertOrientPosition, aAny );

    // width
    if( nWidth > 0 )
    {
        aAny <<= nWidth;
        xPropSet->setPropertyValue( sWidth, aAny );
    }
    if( nRelWidth > 0 || nWidth > 0 )
    {
        aAny <<= nRelWidth;
        xPropSet->setPropertyValue( sRelativeWidth, aAny );
    }
    if( bSyncWidth || nWidth > 0 )
    {
        aAny.setValue( &bSyncWidth, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsSyncWidthToHeight, aAny );
    }

    if( nHeight > 0 )
    {
        aAny <<= nHeight;
        xPropSet->setPropertyValue( sHeight, aAny );
    }
    if( nRelHeight > 0 || nHeight > 0 )
    {
        aAny <<= nRelHeight;
        xPropSet->setPropertyValue( sRelativeHeight, aAny );
    }
    if( bSyncHeight || nHeight > 0 )
    {
        aAny.setValue( &bSyncHeight, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsSyncHeightToWidth, aAny );
    }
    if( xPropSetInfo->hasPropertyByName( sSizeType ) &&
        (bMinHeight || nHeight > 0 || nRelHeight > 0 ) )
    {
        sal_Int16 nSizeType =
            (bMinHeight && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                            : SizeType::FIX;
        aAny <<= nSizeType;
        xPropSet->setPropertyValue( sSizeType, aAny );
    }

    if( XML_TEXT_FRAME_GRAPHIC == nType )
    {
        // URL
        UniReference < XMLTextImportHelper > xTxtImport =
            GetImport().GetTextImport();
        sal_Bool bForceLoad = xTxtImport->IsInsertMode() ||
                              xTxtImport->IsBlockMode() ||
                              xTxtImport->IsStylesOnlyMode() ||
                              xTxtImport->IsOrganizerMode();
        aAny <<= GetImport().ResolveGraphicObjectURL( sHRef, !bForceLoad );
        xPropSet->setPropertyValue( sGraphicURL, aAny );

        // filter name
        aAny <<=sFilterName;
        xPropSet->setPropertyValue( sGraphicFilter, aAny );

        // rotation
        aAny <<= nRotation;
        xPropSet->setPropertyValue( sGraphicRotation, aAny );
    }

    if( XML_TEXT_FRAME_OBJECT != nType  &&
        XML_TEXT_FRAME_OBJECT_OLE != nType  &&
        XML_TEXT_FRAME_APPLET != nType &&
        XML_TEXT_FRAME_PLUGIN!= nType &&
        XML_TEXT_FRAME_FLOATING_FRAME != nType)
    {
        Reference < XTextContent > xTxtCntnt( xPropSet, UNO_QUERY );
        xTxtImport->InsertTextContent( xTxtCntnt );

        Reference < XShape > xShape( xPropSet, UNO_QUERY );
        GetImport().GetShapeImport()->shapeWithZIndexAdded( xShape, nZIndex );
    }

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    if( TextContentAnchorType_AT_PAGE == eAnchorType && nPage > 0 )
    {
        aAny <<= nPage;
        xPropSet->setPropertyValue( sAnchorPageNo, aAny );
    }

    if( XML_TEXT_FRAME_TEXTBOX == nType )
    {
        xTxtImport->ConnectFrameChains( sName, sChainNextName, xPropSet );
        Reference < XTextFrame > xTxtFrame( xPropSet, UNO_QUERY );
        Reference < XText > xTxt = xTxtFrame->getText();
        xOldTextCursor = xTxtImport->GetCursor();
        xTxtImport->SetCursor( xTxt->createTextCursor() );
    }

}

XMLTextFrameContext::~XMLTextFrameContext()
{
}

void XMLTextFrameContext::EndElement()
{
    // alternative text
    if( sDesc.getLength() )
    {
        if ( xPropSet.is() )
        {
            Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
            if( xPropSetInfo->hasPropertyByName( sAlternativeText ) )
            {
                Any aAny;
                aAny <<= sDesc;
                xPropSet->setPropertyValue( sAlternativeText, aAny );
            }
        }
    }

    if( xOldTextCursor.is() )
    {
        GetImport().GetTextImport()->DeleteParagraph();
        GetImport().GetTextImport()->SetCursor( xOldTextCursor );
    }
    if (( nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN ) && xPropSet.is())
        GetImport().GetTextImport()->endAppletOrPlugin( xPropSet, aParamMap);
}

SvXMLImportContext *XMLTextFrameContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_SVG == nPrefix &&
        rLocalName.equalsAsciiL( sXML_desc, sizeof(sXML_desc)-1 ) )
    {
        pContext = new XMLTextFrameDescContext_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, sDesc );
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( (nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN) &&
              rLocalName.equalsAsciiL (  sXML_param, sizeof (sXML_param) -1 ) )
        {
            pContext = new XMLTextFrameParam_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, nType, aParamMap );
        }
        else if( xPropSet.is() )
        {
            if( rLocalName.equalsAsciiL( sXML_contour_polygon,
                                         sizeof(sXML_contour_polygon)-1 ) )
                pContext = new XMLTextFrameContourContext_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, xPropSet, sal_False );
            else if( rLocalName.equalsAsciiL( sXML_contour_path,
                                         sizeof(sXML_contour_path)-1 ) )
                pContext = new XMLTextFrameContourContext_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, xPropSet, sal_True );
        }
    }
    else if( (XML_NAMESPACE_OFFICE == nPrefix) &&
             rLocalName.equalsAsciiL(sXML_events, sizeof(sXML_events)-1) )
    {
        // do we still have the frame object?
        if (xPropSet.is())
        {
            // is it an event supplier?
            Reference<XEventsSupplier> xEventsSupplier(xPropSet, UNO_QUERY);
            if (xEventsSupplier.is())
            {
                // OK, we have the events, so create the context
                pContext = new XMLEventsImportContext(GetImport(), nPrefix,
                                                  rLocalName, xEventsSupplier);
            }
            // else: no events, no event import
        }
        // else: no object, no event import
    }
    if( !pContext && xOldTextCursor.is() )  // text-box
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
                            GetImport(), nPrefix, rLocalName, xAttrList,
                            XML_TEXT_TYPE_TEXTBOX );

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}


void XMLTextFrameContext::SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       sal_Bool bMap )
{
    if( !xPropSet.is() )
        return;

    UniReference< XMLTextImportHelper > xTxtImp = GetImport().GetTextImport();
    Reference < XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if( !xPropSetInfo.is() ||
        !xPropSetInfo->hasPropertyByName( xTxtImp->sHyperLinkURL ) )
        return;

    Any aAny;
    aAny <<= rHRef;
    xPropSet->setPropertyValue( xTxtImp->sHyperLinkURL, aAny );

    if( xPropSetInfo->hasPropertyByName( xTxtImp->sHyperLinkName ) )
    {
        aAny <<= rName;
        xPropSet->setPropertyValue( xTxtImp->sHyperLinkName, aAny );
    }

    if( xPropSetInfo->hasPropertyByName( xTxtImp->sHyperLinkTarget ) )
    {
        aAny <<= rTargetFrameName;
        xPropSet->setPropertyValue( xTxtImp->sHyperLinkTarget, aAny );
    }

    if( xPropSetInfo->hasPropertyByName( xTxtImp->sServerMap ) )
    {
        aAny.setValue( &bMap, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( xTxtImp->sServerMap, aAny );
    }
}

