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

#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_ 
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_ 
#include <com/sun/star/text/SizeType.hpp>
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
namespace binfilter {




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
using namespace ::binfilter::xmloff::token;
using ::com::sun::star::document::XEventsSupplier;

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
    OUString&	rDesc;

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

TYPEINIT1( XMLTextFrameContext, SvXMLImportContext );

void XMLTextFrameContext::Create( sal_Bool bHRefOrBase64 )
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
        const Reference < XNameContainer > & rStyles =
            xTxtImport->GetFrameStyles();
        if( rStyles.is() &&
            rStyles->hasByName( sStyleName ) )
        {
            aAny <<= sStyleName;
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

    if( pHyperlink )
    {
        SetHyperlink( pHyperlink->GetHRef(), pHyperlink->GetName(),
                      pHyperlink->GetTargetFrameName(), pHyperlink->GetMap() );
        delete pHyperlink;
        pHyperlink = 0;
    }

}

sal_Bool XMLTextFrameContext::CreateIfNotThere()
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

XMLTextFrameContext::XMLTextFrameContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nNewType ) :
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
    pHyperlink( 0 )
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
    bSyncWidth = sal_False;
    bSyncHeight = sal_False;
    bCreateFailed = sal_False;
    bOwnBase64Stream = sal_False;

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
        return;	// no URL: no image or OLE object

    Create( sal_True );
}

XMLTextFrameContext::~XMLTextFrameContext()
{
    delete pHyperlink;	// should be 0, but might exist not only because of
                        // a filter bug but also because of an improper XML
                        // file, so an OSL_ENSURE is not correct either.
}

void XMLTextFrameContext::EndElement()
{
    CreateIfNotThere();

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

SvXMLImportContext *XMLTextFrameContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_SVG == nPrefix &&
        IsXMLToken( rLocalName, XML_DESC ) )
    {
        pContext = new XMLTextFrameDescContext_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, sDesc );
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( (nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN) &&
              IsXMLToken( rLocalName, XML_PARAM ) )
        {
            pContext = new XMLTextFrameParam_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, nType, aParamMap );
        }
        else
        {
            if( IsXMLToken( rLocalName, XML_CONTOUR_POLYGON ) )
            {
                if( CreateIfNotThere() )
                    pContext = new XMLTextFrameContourContext_Impl( GetImport(),
                                                  nPrefix, rLocalName,
                                                  xAttrList, xPropSet, sal_False );
            }
            else if( IsXMLToken( rLocalName, XML_CONTOUR_PATH ) )
            {
                if( CreateIfNotThere() )
                    pContext = new XMLTextFrameContourContext_Impl( GetImport(),
                                                  nPrefix, rLocalName,
                                                  xAttrList, xPropSet, sal_True );
            }
            else if ( IsXMLToken( rLocalName, XML_IMAGE_MAP ) &&
                      ( nType == XML_TEXT_FRAME_TEXTBOX ||
                        nType == XML_TEXT_FRAME_GRAPHIC ||
                        nType == XML_TEXT_FRAME_OBJECT_OLE ) )
            {
                if( CreateIfNotThere() )
                    pContext = new XMLImageMapContext( GetImport(), nPrefix,
                                                       rLocalName, xPropSet );
            }
        }
    }
    else if( (XML_NAMESPACE_OFFICE == nPrefix) )
    {
        if( IsXMLToken( rLocalName, XML_EVENTS ) )
        {
            // do we still have the frame object?
            if( CreateIfNotThere() )
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
        else if( xmloff::token::IsXMLToken( rLocalName,
                                            xmloff::token::XML_BINARY_DATA ) )
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
                    Reference< ::com::sun::star::lang::XComponent> aXComponent(xEOS->getEmbeddedObject());
                    pEContext->SetComponent( aXComponent );
                }
            }
            pContext = pEContext;
        }
    }
    if( !pContext && xOldTextCursor.is() )	// text-box
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
                            GetImport(), nPrefix, rLocalName, xAttrList,
                            XML_TEXT_TYPE_TEXTBOX );

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLTextFrameContext::Characters( const OUString& rChars )
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

void XMLTextFrameContext::SetHyperlink( const OUString& rHRef,
                       const OUString& rName, 
                       const OUString& rTargetFrameName,
                       sal_Bool bMap )
{
    if( !xPropSet.is() )
    {
        OSL_ENSURE( !pHyperlink, "recursive SetHyperlink call" );
        delete pHyperlink;
        pHyperlink = new XMLTextFrameContextHyperlink_Impl(
                rHRef, rName, rTargetFrameName, bMap );
        return;
    }

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

Reference < XTextContent > XMLTextFrameContext::GetTextContent() const
{
    return Reference < XTextContent >( xPropSet, UNO_QUERY );
}
}//end of namespace binfilter
