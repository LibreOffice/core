/*************************************************************************
 *
 *  $RCSfile: XMLTextFrameContext.cxx,v $
 *
 *  $Revision: 1.58 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:36:31 $
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
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDOBJECTSUPPLIER_HPP_
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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
#ifndef _XMLOFF_XMLEMBEDDEDOBJECTIMPORTCONTEXT_HXX
#include "XMLEmbeddedObjectImportContext.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include "XMLBase64ImportContext.hxx"
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
#ifndef _XMLOFF_XMLIMAGEMAPCONTEXT_HXX_
#include "XMLImageMapContext.hxx"
#endif

#ifndef _XMLTEXTFRAMECONTEXT_HXX
#include "XMLTextFrameContext.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef __SGI_STL_MAP
#include <map>
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
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;
using ::com::sun::star::document::XEventsSupplier;

#define XML_TEXT_FRAME_TEXTBOX 1
#define XML_TEXT_FRAME_GRAPHIC 2
#define XML_TEXT_FRAME_OBJECT 3
#define XML_TEXT_FRAME_OBJECT_OLE 4
#define XML_TEXT_FRAME_APPLET 5
#define XML_TEXT_FRAME_PLUGIN 6
#define XML_TEXT_FRAME_FLOATING_FRAME 7

typedef ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess> ParamMap;

class XMLTextFrameContextHyperlink_Impl
{
    OUString sHRef;
    OUString sName;
    OUString sTargetFrameName;
    sal_Bool bMap;

public:

    inline XMLTextFrameContextHyperlink_Impl( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       sal_Bool bMap );

    const OUString& GetHRef() const { return sHRef; }
    const OUString& GetName() const { return sName; }
    const OUString& GetTargetFrameName() const { return sTargetFrameName; }
    sal_Bool GetMap() const { return bMap; }
};

inline XMLTextFrameContextHyperlink_Impl::XMLTextFrameContextHyperlink_Impl(
    const OUString& rHRef, const OUString& rName,
    const OUString& rTargetFrameName, sal_Bool bM ) :
    sHRef( rHRef ),
    sName( rName ),
    sTargetFrameName( rTargetFrameName ),
    bMap( bM )
{
}

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

XMLTextFrameParam_Impl::XMLTextFrameParam_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
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
        if ( XML_NAMESPACE_DRAW == nPrefix )
        {
               if( IsXMLToken(aLocalName, XML_VALUE) )
            {
                sValue = rValue;
                bFoundValue=sal_True;
            }
            else if( IsXMLToken(aLocalName, XML_NAME) )
            {
                sName = rValue;
            }
        }
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
    sal_Bool bPixelWidth = sal_False, bPixelHeight = sal_False;
    sal_Bool bAuto = sal_False;
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;

    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextContourAttrTokenMap();

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
        case XML_TOK_TEXT_CONTOUR_VIEWBOX:
            sViewBox = rValue;
            break;
        case XML_TOK_TEXT_CONTOUR_D:
            if( bPath )
                sD = rValue;
            break;
        case XML_TOK_TEXT_CONTOUR_POINTS:
            if( !bPath )
                sPoints = rValue;
            break;
        case XML_TOK_TEXT_CONTOUR_WIDTH:
            if( GetImport().GetMM100UnitConverter().convertMeasurePx( nWidth,
                                                                      rValue) )
                bPixelWidth = sal_True;
            else
                GetImport().GetMM100UnitConverter().convertMeasure( nWidth,
                                                                rValue);
            break;
        case XML_TOK_TEXT_CONTOUR_HEIGHT:
            if( GetImport().GetMM100UnitConverter().convertMeasurePx( nHeight,
                                                                rValue) )
                bPixelHeight = sal_True;
            else
                GetImport().GetMM100UnitConverter().convertMeasure( nHeight,
                                                                    rValue);
            break;
        case XML_TOK_TEXT_CONTOUR_AUTO:
            bAuto = IsXMLToken(rValue, XML_TRUE);
            break;
        }
    }

    OUString sContourPolyPolygon(
            RTL_CONSTASCII_USTRINGPARAM("ContourPolyPolygon") );
    Reference < XPropertySetInfo > xPropSetInfo =
        rPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName(
                                                    sContourPolyPolygon ) &&
        nWidth > 0 && nHeight > 0 && bPixelWidth == bPixelHeight &&
        (bPath ? sD : sPoints).getLength() )
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

        OUString sIsPixelContour(
                RTL_CONSTASCII_USTRINGPARAM("IsPixelContour") );
        xPropSet->setPropertyValue( sContourPolyPolygon, aAny );

        if( xPropSetInfo->hasPropertyByName( sIsPixelContour ) )
        {
            aAny.setValue( &bPixelWidth, ::getBooleanCppuType() );
            xPropSet->setPropertyValue( sIsPixelContour, aAny );
        }

        OUString sIsAutomaticContour(
                RTL_CONSTASCII_USTRINGPARAM("IsAutomaticContour") );
        if( xPropSetInfo->hasPropertyByName( sIsAutomaticContour ) )
        {
            aAny.setValue( &bAuto, ::getBooleanCppuType() );
            xPropSet->setPropertyValue( sIsAutomaticContour, aAny );
        }
    }
}

XMLTextFrameContourContext_Impl::~XMLTextFrameContourContext_Impl()
{
}


// ------------------------------------------------------------------------

class XMLTextFrameContext_Impl : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > xPropSet;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::io::XOutputStream > xBase64Stream;

    /// old list item and block (#89891#)
    SvXMLImportContextRef xListBlock;
    SvXMLImportContextRef xListItem;

    const ::rtl::OUString sWidth;
    const ::rtl::OUString sWidthType;
    const ::rtl::OUString sRelativeWidth;
    const ::rtl::OUString sHeight;
    const ::rtl::OUString sRelativeHeight;
    const ::rtl::OUString sSizeType;
    const ::rtl::OUString sIsSyncWidthToHeight;
    const ::rtl::OUString sIsSyncHeightToWidth;
    const ::rtl::OUString sHoriOrient;
    const ::rtl::OUString sHoriOrientPosition;
    const ::rtl::OUString sVertOrient;
    const ::rtl::OUString sVertOrientPosition;
    const ::rtl::OUString sChainNextName;
    const ::rtl::OUString sAnchorType;
    const ::rtl::OUString sAnchorPageNo;
    const ::rtl::OUString sGraphicURL;
    const ::rtl::OUString sGraphicFilter;
    const ::rtl::OUString sAlternativeText;
    const ::rtl::OUString sFrameStyleName;
    const ::rtl::OUString sGraphicRotation;
    const ::rtl::OUString sTextBoxServiceName;
    const ::rtl::OUString sGraphicServiceName;

    ::rtl::OUString sName;
    ::rtl::OUString sStyleName;
    ::rtl::OUString sNextName;
    ::rtl::OUString sHRef;
    ::rtl::OUString sFilterName;
    ::rtl::OUString sCode;
    ::rtl::OUString sObject;
    ::rtl::OUString sArchive;
    ::rtl::OUString sMimeType;
    ::rtl::OUString sFrameName;
    ::rtl::OUString sAppletName;
    ::rtl::OUString sFilterService;
    ::rtl::OUString sBase64CharsLeft;
    ::rtl::OUString sTblName;

    ParamMap aParamMap;

    sal_Int32   nX;
    sal_Int32   nY;
    sal_Int32   nWidth;
    sal_Int32   nHeight;
    sal_Int32   nZIndex;
    sal_Int16   nPage;
    sal_Int16   nRotation;
    sal_Int16   nRelWidth;
    sal_Int16   nRelHeight;

    sal_uInt16 nType;
    ::com::sun::star::text::TextContentAnchorType   eAnchorType;

    sal_Bool    bMayScript : 1;
    sal_Bool    bMinWidth : 1;
    sal_Bool    bMinHeight : 1;
    sal_Bool    bSyncWidth : 1;
    sal_Bool    bSyncHeight : 1;
    sal_Bool    bCreateFailed : 1;
    sal_Bool    bOwnBase64Stream : 1;

    void Create( sal_Bool bHRefOrBase64 );

public:

    TYPEINFO();

    sal_Bool CreateIfNotThere();

    XMLTextFrameContext_Impl( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rAttrList,
            ::com::sun::star::text::TextContentAnchorType eAnchorType,
            sal_uInt16 nType,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rFrameAttrList );
    virtual ~XMLTextFrameContext_Impl();

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    void SetHyperlink( const ::rtl::OUString& rHRef,
                       const ::rtl::OUString& rName,
                       const ::rtl::OUString& rTargetFrameName,
                       sal_Bool bMap );
    void SetDesc( const ::rtl::OUString& rDesc );

    ::com::sun::star::text::TextContentAnchorType GetAnchorType() const { return eAnchorType; }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet >& GetPropSet() { return xPropSet; }
};

TYPEINIT1( XMLTextFrameContext_Impl, SvXMLImportContext );

void XMLTextFrameContext_Impl::Create( sal_Bool bHRefOrBase64 )
{
    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();

    switch ( nType)
    {
        case XML_TEXT_FRAME_OBJECT:
        case XML_TEXT_FRAME_OBJECT_OLE:
            if( bHRefOrBase64 )
            {
                OUString sURL( GetImport().ResolveEmbeddedObjectURL( sHRef,
                                                                OUString() ) );

                if( sURL.getLength() )
                    xPropSet = GetImport().GetTextImport()
                            ->createAndInsertOLEObject( GetImport(), sURL,
                                                        sStyleName,
                                                        sTblName,
                                                        nWidth, nHeight );
            }
            else
            {
                OUString sURL( RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.ServiceName:") );
                sURL += sFilterService;
                xPropSet = GetImport().GetTextImport()
                            ->createAndInsertOLEObject( GetImport(), sURL,
                                                        sStyleName,
                                                        sTblName,
                                                        nWidth, nHeight );

            }
            break;
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
    {
        bCreateFailed = sal_True;
        return;
    }

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
        OUString sDisplayStyleName( GetImport().GetStyleDisplayName(
                            XML_STYLE_FAMILY_SD_GRAPHICS_ID, sStyleName ) );
        const Reference < XNameContainer > & rStyles =
            xTxtImport->GetFrameStyles();
        if( rStyles.is() &&
            rStyles->hasByName( sDisplayStyleName ) )
        {
            aAny <<= sDisplayStyleName;
            xPropSet->setPropertyValue( sFrameStyleName, aAny );
        }
    }

    // anchor type (must be set before any other properties, because
    // otherwise some orientations cannot be set or will be changed
    // afterwards)
    aAny <<= eAnchorType;
    xPropSet->setPropertyValue( sAnchorType, aAny );

    // hard properties
    if( pStyle )
        pStyle->FillPropertySet( xPropSet );


    // x and y
    sal_Int16 nHoriOrient =  HoriOrientation::NONE;
    aAny = xPropSet->getPropertyValue( sHoriOrient );
    aAny >>= nHoriOrient;
    if( HoriOrientation::NONE == nHoriOrient )
    {
        aAny <<= nX;
        xPropSet->setPropertyValue( sHoriOrientPosition, aAny );
    }

    sal_Int16 nVertOrient =  VertOrientation::NONE;
    aAny = xPropSet->getPropertyValue( sVertOrient );
    aAny >>= nVertOrient;
    if( VertOrientation::NONE == nVertOrient )
    {
        aAny <<= nY;
        xPropSet->setPropertyValue( sVertOrientPosition, aAny );
    }

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
        sal_Bool bTmp = bSyncWidth;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsSyncWidthToHeight, aAny );
    }
    if( xPropSetInfo->hasPropertyByName( sWidthType ) &&
        (bMinWidth || nWidth > 0 || nRelWidth > 0 ) )
    {
        sal_Int16 nSizeType =
            (bMinWidth && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                           : SizeType::FIX;
        aAny <<= nSizeType;
        xPropSet->setPropertyValue( sWidthType, aAny );
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
        sal_Bool bTmp = bSyncHeight;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
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
        OSL_ENSURE( sHRef.getLength() > 0 || xBase64Stream.is(),
                    "neither URL nor base64 image data given" );
        UniReference < XMLTextImportHelper > xTxtImport =
            GetImport().GetTextImport();
        if( sHRef.getLength() )
        {
            sal_Bool bForceLoad = xTxtImport->IsInsertMode() ||
                                  xTxtImport->IsBlockMode() ||
                                  xTxtImport->IsStylesOnlyMode() ||
                                  xTxtImport->IsOrganizerMode();
            sHRef = GetImport().ResolveGraphicObjectURL( sHRef, !bForceLoad );
        }
        else if( xBase64Stream.is() )
        {
            sHRef = GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream );
            xBase64Stream = 0;
        }
        aAny <<= sHRef;
        xPropSet->setPropertyValue( sGraphicURL, aAny );

        // filter name
        aAny <<=sFilterName;
        xPropSet->setPropertyValue( sGraphicFilter, aAny );

        // rotation
        aAny <<= nRotation;
        xPropSet->setPropertyValue( sGraphicRotation, aAny );
    }

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    if( TextContentAnchorType_AT_PAGE == eAnchorType && nPage > 0 )
    {
        aAny <<= nPage;
        xPropSet->setPropertyValue( sAnchorPageNo, aAny );
    }

    if( XML_TEXT_FRAME_OBJECT != nType  &&
        XML_TEXT_FRAME_OBJECT_OLE != nType  &&
        XML_TEXT_FRAME_APPLET != nType &&
        XML_TEXT_FRAME_PLUGIN!= nType &&
        XML_TEXT_FRAME_FLOATING_FRAME != nType)
    {
        Reference < XTextContent > xTxtCntnt( xPropSet, UNO_QUERY );
        xTxtImport->InsertTextContent( xTxtCntnt );
    }

    Reference < XShape > xShape( xPropSet, UNO_QUERY );

    // #107848#
    // Make adding the shepe to Z-Ordering dependent from if we are
    // inside a inside_deleted_section (redlining). That is necessary
    // since the shape will be removed again later. It would lead to
    // errors if it would stay inside the Z-Ordering. Thus, the
    // easiest way to solve that conflict is to not add it here.
    if(!GetImport().HasTextImport()
        || !GetImport().GetTextImport()->IsInsideDeleteContext())
    {
        GetImport().GetShapeImport()->shapeWithZIndexAdded( xShape, nZIndex );
    }

    if( XML_TEXT_FRAME_TEXTBOX == nType )
    {
        xTxtImport->ConnectFrameChains( sName, sNextName, xPropSet );
        Reference < XTextFrame > xTxtFrame( xPropSet, UNO_QUERY );
        Reference < XText > xTxt = xTxtFrame->getText();
        xOldTextCursor = xTxtImport->GetCursor();
        xTxtImport->SetCursor( xTxt->createTextCursor() );

        // remember old list item and block (#89892#) and reset them
        // for the text frame
        xListBlock = xTxtImport->GetListBlock();
        xListItem = xTxtImport->GetListItem();
        xTxtImport->SetListBlock( NULL );
        xTxtImport->SetListItem( NULL );
    }
}

sal_Bool XMLTextFrameContext_Impl::CreateIfNotThere()
{
    if( !xPropSet.is() &&
        ( XML_TEXT_FRAME_OBJECT_OLE == nType ||
          XML_TEXT_FRAME_GRAPHIC == nType ) &&
        xBase64Stream.is() && !bCreateFailed )
    {
        if( bOwnBase64Stream )
            xBase64Stream->closeOutput();
        Create( sal_True );
    }

    return xPropSet.is();
}

XMLTextFrameContext_Impl::XMLTextFrameContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & rAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nNewType,
        const Reference< XAttributeList > & rFrameAttrList ) :
    nType( nNewType ),
    eAnchorType( eATyp ),
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sWidth(RTL_CONSTASCII_USTRINGPARAM("Width")),
    sRelativeWidth(RTL_CONSTASCII_USTRINGPARAM("RelativeWidth")),
    sHeight(RTL_CONSTASCII_USTRINGPARAM("Height")),
    sRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("RelativeHeight")),
    sSizeType(RTL_CONSTASCII_USTRINGPARAM("SizeType")),
    sIsSyncWidthToHeight(RTL_CONSTASCII_USTRINGPARAM("IsSyncWidthToHeight")),
    sIsSyncHeightToWidth(RTL_CONSTASCII_USTRINGPARAM("IsSyncHeightToWidth")),
    sHoriOrient(RTL_CONSTASCII_USTRINGPARAM("HoriOrient")),
    sHoriOrientPosition(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")),
    sVertOrient(RTL_CONSTASCII_USTRINGPARAM("VertOrient")),
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
    sGraphicServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GraphicObject")),
    sWidthType(RTL_CONSTASCII_USTRINGPARAM("WidthType"))
{
    nX = 0;
    nY = 0;
    nWidth = 0;
    nHeight = 0;
    nZIndex = -1;
    nPage = 0;
    nRotation = 0;
    nRelWidth = 0;
    nRelHeight = 0;
    bMayScript = sal_False;

    bMinHeight = sal_False;
    bMinWidth = sal_False;
    bSyncWidth = sal_False;
    bSyncHeight = sal_False;
    bCreateFailed = sal_False;
    bOwnBase64Stream = sal_False;

    UniReference < XMLTextImportHelper > xTxtImport =
        GetImport().GetTextImport();
    const SvXMLTokenMap& rTokenMap =
        xTxtImport->GetTextFrameAttrTokenMap();

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    sal_Int16 nTotalAttrCount = nAttrCount + (rFrameAttrList.is() ? rFrameAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nTotalAttrCount; i++ )
    {
        const OUString& rAttrName =
            i < nAttrCount ? rAttrList->getNameByIndex( i ) : rFrameAttrList->getNameByIndex( i-nAttrCount );
        const OUString& rValue =
            i < nAttrCount ? rAttrList->getValueByIndex( i ): rFrameAttrList->getValueByIndex( i-nAttrCount );

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
                nRelWidth = (sal_Int16)nTmp;
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasure( nWidth,
                                                                    rValue, 0 );
            }
            break;
        case XML_TOK_TEXT_FRAME_REL_WIDTH:
            if( IsXMLToken(rValue, XML_SCALE) )
            {
                bSyncWidth = sal_True;
            }
            else
            {
                sal_Int32 nTmp;
                if( GetImport().GetMM100UnitConverter().
                        convertPercent( nTmp, rValue ) )
                    nRelWidth = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_WIDTH:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                GetImport().GetMM100UnitConverter().convertPercent( nTmp,
                                                                    rValue );
                nRelWidth = (sal_Int16)nTmp;
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasure( nWidth,
                                                                    rValue, 0 );
            }
            bMinWidth = sal_True;
            break;
        case XML_TOK_TEXT_FRAME_HEIGHT:
            // relative heights are obsolete since SRC617. Remove them some day!
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                GetImport().GetMM100UnitConverter().convertPercent( nTmp,
                                                                    rValue );
                nRelHeight = (sal_Int16)nTmp;
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasure( nHeight,
                                                                    rValue, 0 );
            }
            break;
        case XML_TOK_TEXT_FRAME_REL_HEIGHT:
            if( IsXMLToken( rValue, XML_SCALE ) )
            {
                bSyncHeight = sal_True;
            }
            else if( IsXMLToken( rValue, XML_SCALE_MIN ) )
            {
                bSyncHeight = sal_True;
                bMinHeight = sal_True;
            }
            else
            {
                sal_Int32 nTmp;
                if( GetImport().GetMM100UnitConverter().
                        convertPercent( nTmp, rValue ) )
                    nRelHeight = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_HEIGHT:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                GetImport().GetMM100UnitConverter().convertPercent( nTmp,
                                                                    rValue );
                nRelHeight = (sal_Int16)nTmp;
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
            sNextName = rValue;
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
            bMayScript = IsXMLToken( rValue, XML_TRUE );
            break;
        case XML_TOK_TEXT_FRAME_MIME_TYPE:
            sMimeType = rValue;
            break;
        case XML_TOK_TEXT_FRAME_NOTIFY_ON_UPDATE:
            sTblName = rValue;
            break;
        }
    }

    if( ( (XML_TEXT_FRAME_GRAPHIC == nType ||
           XML_TEXT_FRAME_OBJECT == nType ||
           XML_TEXT_FRAME_OBJECT_OLE == nType) &&
          !sHRef.getLength() ) ||
        ( XML_TEXT_FRAME_APPLET  == nType && !sCode.getLength() ) ||
        ( XML_TEXT_FRAME_PLUGIN == nType &&
          sHRef.getLength() == 0 && sMimeType.getLength() == 0 ) )
        return; // no URL: no image or OLE object

    Create( sal_True );
}

XMLTextFrameContext_Impl::~XMLTextFrameContext_Impl()
{
}

void XMLTextFrameContext_Impl::EndElement()
{
    CreateIfNotThere();

    if( xOldTextCursor.is() )
    {
        GetImport().GetTextImport()->DeleteParagraph();
        GetImport().GetTextImport()->SetCursor( xOldTextCursor );
    }

    // reinstall old list item (if necessary) #89892#
    if ( xListBlock.Is() )
    {
        GetImport().GetTextImport()->SetListBlock(
            (XMLTextListBlockContext*)&xListBlock );
        GetImport().GetTextImport()->SetListItem(
            (XMLTextListItemContext*)&xListItem );
    }

    if (( nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN ) && xPropSet.is())
        GetImport().GetTextImport()->endAppletOrPlugin( xPropSet, aParamMap);
}

SvXMLImportContext *XMLTextFrameContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( (nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN) &&
              IsXMLToken( rLocalName, XML_PARAM ) )
        {
            pContext = new XMLTextFrameParam_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, nType, aParamMap );
        }
    }
    else if( (XML_NAMESPACE_OFFICE == nPrefix) )
    {
        if( IsXMLToken( rLocalName, XML_BINARY_DATA ) )
        {
            if( !xPropSet.is() && !xBase64Stream.is() && !bCreateFailed )
            {
                switch( nType )
                {
                case XML_TEXT_FRAME_GRAPHIC:
                    xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
                    break;
                case XML_TEXT_FRAME_OBJECT_OLE:
                    sHRef = OUString( RTL_CONSTASCII_USTRINGPARAM( "#Obj12345678" ) );
                    xBase64Stream =
                        GetImport().ResolveEmbeddedObjectURLFromBase64( sHRef );
                    break;
                }
                if( xBase64Stream.is() )
                    pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    xBase64Stream );
            }
        }
    }
    if( !pContext &&
            ( XML_TEXT_FRAME_OBJECT == nType &&
              (XML_NAMESPACE_OFFICE == nPrefix &&
               IsXMLToken( rLocalName, XML_DOCUMENT )) ||
              (XML_NAMESPACE_MATH == nPrefix &&
               IsXMLToken(rLocalName, XML_MATH) ) ) )
    {
        if( !xPropSet.is() && !bCreateFailed )
        {
            XMLEmbeddedObjectImportContext *pEContext =
                new XMLEmbeddedObjectImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList );
            sFilterService = pEContext->GetFilterServiceName();
            if( sFilterService.getLength() != 0 )
            {
                Create( sal_False );
                if( xPropSet.is() )
                {
                    Reference < XEmbeddedObjectSupplier > xEOS( xPropSet,
                                                                UNO_QUERY );
                    OSL_ENSURE( xEOS.is(),
                            "no embedded object supplier for own object" );
                    Reference<com::sun::star::lang::XComponent> aXComponent(xEOS->getEmbeddedObject());
                    pEContext->SetComponent( aXComponent );
                }
            }
            pContext = pEContext;
        }
    }
    if( !pContext && xOldTextCursor.is() )  // text-box
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
                            GetImport(), nPrefix, rLocalName, xAttrList,
                            XML_TEXT_TYPE_TEXTBOX );

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLTextFrameContext_Impl::Characters( const OUString& rChars )
{
    if( ( XML_TEXT_FRAME_OBJECT_OLE == nType ||
          XML_TEXT_FRAME_GRAPHIC == nType) &&
        !xPropSet.is() && !bCreateFailed )
    {
        OUString sTrimmedChars( rChars. trim() );
        if( sTrimmedChars.getLength() )
        {
            if( !xBase64Stream.is() )
            {
                if( XML_TEXT_FRAME_GRAPHIC == nType )
                {
                    xBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
                }
                else
                {
                    sHRef = OUString( RTL_CONSTASCII_USTRINGPARAM( "#Obj12345678" ) );
                    xBase64Stream =
                        GetImport().ResolveEmbeddedObjectURLFromBase64( sHRef );
                }
                if( xBase64Stream.is() )
                    bOwnBase64Stream = sal_True;
            }
            if( bOwnBase64Stream && xBase64Stream.is() )
            {
                OUString sChars;
                if( sBase64CharsLeft )
                {
                    sChars = sBase64CharsLeft;
                    sChars += sTrimmedChars;
                    sBase64CharsLeft = OUString();
                }
                else
                {
                    sChars = sTrimmedChars;
                }
                Sequence< sal_Int8 > aBuffer( (sChars.getLength() / 4) * 3 );
                sal_Int32 nCharsDecoded =
                    GetImport().GetMM100UnitConverter().
                        decodeBase64SomeChars( aBuffer, sChars );
                xBase64Stream->writeBytes( aBuffer );
                if( nCharsDecoded != sChars.getLength() )
                    sBase64CharsLeft = sChars.copy( nCharsDecoded );
            }
        }
    }
}

void XMLTextFrameContext_Impl::SetHyperlink( const OUString& rHRef,
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

void XMLTextFrameContext_Impl::SetDesc( const OUString& rDesc )
{
    if ( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sAlternativeText ) )
        {
            xPropSet->setPropertyValue( sAlternativeText, makeAny( rDesc ) );
        }
    }
}

//-----------------------------------------------------------------------------------------------------

TYPEINIT1( XMLTextFrameContext, SvXMLImportContext );

sal_Bool XMLTextFrameContext::CreateIfNotThere()
{
    sal_Bool bRet = sal_False;
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = PTR_CAST( XMLTextFrameContext_Impl, pContext );
    if( pImpl )
        bRet = pImpl->CreateIfNotThere();

    return bRet;
}

sal_Bool XMLTextFrameContext::CreateIfNotThere( ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet >& rPropSet )
{
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = PTR_CAST( XMLTextFrameContext_Impl, pContext );
    if( pImpl )
    {
        if( pImpl->CreateIfNotThere() )
            rPropSet = pImpl->GetPropSet();
    }

    return rPropSet.is();
}

XMLTextFrameContext::XMLTextFrameContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    m_xAttrList( new SvXMLAttributeList( xAttrList ) ),
    m_eDefaultAnchorType( eATyp ),
    m_pHyperlink( 0 ),
    m_bHasName( sal_False )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
        if( XML_NAMESPACE_DRAW == nPrefix &&
            IsXMLToken( aLocalName, XML_NAME ) )
        {
            m_bHasName = xAttrList->getValueByIndex(i).getLength() != 0;
        }
        else if( XML_NAMESPACE_TEXT == nPrefix &&
            IsXMLToken( aLocalName, XML_ANCHOR_TYPE ) )
        {
            TextContentAnchorType eNew;
            if( XMLAnchorTypePropHdl::convert( xAttrList->getValueByIndex(i),
                        GetImport().GetMM100UnitConverter(), eNew ) &&
                ( TextContentAnchorType_AT_PARAGRAPH == eNew ||
                  TextContentAnchorType_AT_CHARACTER == eNew ||
                  TextContentAnchorType_AS_CHARACTER == eNew ||
                  TextContentAnchorType_AT_PAGE == eNew) )
                m_eDefaultAnchorType = eNew;
        }
    }
}

XMLTextFrameContext::~XMLTextFrameContext()
{
    delete m_pHyperlink;
}

void XMLTextFrameContext::EndElement()
{
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = PTR_CAST( XMLTextFrameContext_Impl, pContext );
    if( pImpl )
    {
        pImpl->CreateIfNotThere();

        // alternative text
        if( m_sDesc.getLength() )
            pImpl->SetDesc( m_sDesc );

        if( m_pHyperlink )
        {
            pImpl->SetHyperlink( m_pHyperlink->GetHRef(), m_pHyperlink->GetName(),
                          m_pHyperlink->GetTargetFrameName(), m_pHyperlink->GetMap() );
            delete m_pHyperlink;
            m_pHyperlink = 0;
        }

    }
}

SvXMLImportContext *XMLTextFrameContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !m_xImplContext.Is() )
    {
        // no child exists
        if( XML_NAMESPACE_DRAW == nPrefix )
        {
            sal_uInt16 nFrameType = USHRT_MAX;
            if( IsXMLToken( rLocalName, XML_TEXT_BOX ) )
                nFrameType = XML_TEXT_FRAME_TEXTBOX;
            else if( IsXMLToken( rLocalName, XML_IMAGE ) )
                nFrameType = XML_TEXT_FRAME_GRAPHIC;
            else if( IsXMLToken( rLocalName, XML_OBJECT ) )
                nFrameType = XML_TEXT_FRAME_OBJECT;
            else if( IsXMLToken( rLocalName, XML_OBJECT_OLE ) )
                nFrameType = XML_TEXT_FRAME_OBJECT_OLE;
            else if( IsXMLToken( rLocalName, XML_APPLET) )
                nFrameType = XML_TEXT_FRAME_APPLET;
            else if( IsXMLToken( rLocalName, XML_PLUGIN ) )
                nFrameType = XML_TEXT_FRAME_PLUGIN;
            else if( IsXMLToken( rLocalName, XML_FLOATING_FRAME ) )
                nFrameType = XML_TEXT_FRAME_FLOATING_FRAME;

            if( USHRT_MAX != nFrameType )
            {
                if( (XML_TEXT_FRAME_TEXTBOX == nFrameType || XML_TEXT_FRAME_GRAPHIC == nFrameType )
                    && !m_bHasName )
                {
                    Reference < XShapes > xShapes;
                    pContext = GetImport().GetShapeImport()->CreateFrameChildContext(
                                    GetImport(), nPrefix, rLocalName, xAttrList, xShapes, m_xAttrList );
                }
                else
                {

                    pContext = new XMLTextFrameContext_Impl( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        m_eDefaultAnchorType,
                                                        nFrameType,
                                                        m_xAttrList );
                }
                m_xImplContext = pContext;
            }
        }
    }
    else if( m_xImplContext->ISA( XMLTextFrameContext_Impl ) )
    {
        // the child is a writer frame
        if( XML_NAMESPACE_SVG == nPrefix && IsXMLToken( rLocalName, XML_DESC ) )
        {
            pContext = new XMLTextFrameDescContext_Impl( GetImport(), nPrefix, rLocalName,
                                                            xAttrList, m_sDesc );
        }
        else if( XML_NAMESPACE_DRAW == nPrefix )
        {
            Reference < XPropertySet > xPropSet;
            if( IsXMLToken( rLocalName, XML_CONTOUR_POLYGON ) )
            {
                if( CreateIfNotThere( xPropSet ) )
                    pContext = new XMLTextFrameContourContext_Impl( GetImport(), nPrefix, rLocalName,
                                                  xAttrList, xPropSet, sal_False );
            }
            else if( IsXMLToken( rLocalName, XML_CONTOUR_PATH ) )
            {
                if( CreateIfNotThere( xPropSet ) )
                    pContext = new XMLTextFrameContourContext_Impl( GetImport(), nPrefix, rLocalName,
                                                  xAttrList, xPropSet, sal_True );
            }
            else if( IsXMLToken( rLocalName, XML_IMAGE_MAP ) )
            {
                if( CreateIfNotThere( xPropSet ) )
                    pContext = new XMLImageMapContext( GetImport(), nPrefix, rLocalName, xPropSet );
            }
        }
        else if( (XML_NAMESPACE_OFFICE == nPrefix) && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
        {
            // do we still have the frame object?
            Reference < XPropertySet > xPropSet;
            if( CreateIfNotThere( xPropSet ) )
            {
                // is it an event supplier?
                Reference<XEventsSupplier> xEventsSupplier(xPropSet, UNO_QUERY);
                if (xEventsSupplier.is())
                {
                    // OK, we have the events, so create the context
                    pContext = new XMLEventsImportContext(GetImport(), nPrefix,
                                                      rLocalName, xEventsSupplier);
                }
            }
        }
    }
    else
    {
        // the child is a drawinh´g shape
        pContext = GetImport().GetShapeImport()->CreateFrameChildContext(
                                    &m_xImplContext, nPrefix, rLocalName, xAttrList );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLTextFrameContext::SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       sal_Bool bMap )
{
    OSL_ENSURE( !m_pHyperlink, "recursive SetHyperlink call" );
    delete m_pHyperlink;
    m_pHyperlink = new XMLTextFrameContextHyperlink_Impl(
                rHRef, rName, rTargetFrameName, bMap );
}

TextContentAnchorType XMLTextFrameContext::GetAnchorType() const
{
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = PTR_CAST( XMLTextFrameContext_Impl, pContext );
    if( pImpl )
        return pImpl->GetAnchorType();
    else
        return m_eDefaultAnchorType;
}

Reference < XTextContent > XMLTextFrameContext::GetTextContent() const
{
    Reference < XTextContent > xTxtCntnt;
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = PTR_CAST( XMLTextFrameContext_Impl, pContext );
    if( pImpl )
        xTxtCntnt.set( pImpl->GetPropSet(), UNO_QUERY );

    return xTxtCntnt;
}
