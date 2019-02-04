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

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/base64.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "XMLAnchorTypePropHdl.hxx"
#include <XMLEmbeddedObjectImportContext.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <XMLReplacementImageContext.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/i18nmap.hxx>
#include <xexptran.hxx>
#include <xmloff/shapeimport.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <XMLImageMapContext.hxx>
#include "XMLTextFrameContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "XMLTextListItemContext.hxx"
#include <xmloff/attrlist.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <map>

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

typedef ::std::map < const OUString, OUString > ParamMap;

class XMLTextFrameContextHyperlink_Impl
{
    OUString const sHRef;
    OUString const sName;
    OUString const sTargetFrameName;
    bool const bMap;

public:

    inline XMLTextFrameContextHyperlink_Impl( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap );

    const OUString& GetHRef() const { return sHRef; }
    const OUString& GetName() const { return sName; }
    const OUString& GetTargetFrameName() const { return sTargetFrameName; }
    bool GetMap() const { return bMap; }
};

inline XMLTextFrameContextHyperlink_Impl::XMLTextFrameContextHyperlink_Impl(
    const OUString& rHRef, const OUString& rName,
    const OUString& rTargetFrameName, bool bM ) :
    sHRef( rHRef ),
    sName( rName ),
    sTargetFrameName( rTargetFrameName ),
    bMap( bM )
{
}

// Implement Title/Description Elements UI (#i73249#)
class XMLTextFrameTitleOrDescContext_Impl : public SvXMLImportContext
{
    OUString&   mrTitleOrDesc;

public:


    XMLTextFrameTitleOrDescContext_Impl( SvXMLImport& rImport,
                                         sal_uInt16 nPrfx,
                                         const OUString& rLName,
                                         OUString& rTitleOrDesc );

    virtual void Characters( const OUString& rText ) override;
};


XMLTextFrameTitleOrDescContext_Impl::XMLTextFrameTitleOrDescContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        OUString& rTitleOrDesc )
    : SvXMLImportContext( rImport, nPrfx, rLName )
    , mrTitleOrDesc( rTitleOrDesc )
{
}

void XMLTextFrameTitleOrDescContext_Impl::Characters( const OUString& rText )
{
    mrTitleOrDesc += rText;
}

class XMLTextFrameParam_Impl : public SvXMLImportContext
{
public:


    XMLTextFrameParam_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            ParamMap &rParamMap);
};

XMLTextFrameParam_Impl::XMLTextFrameParam_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
        ParamMap &rParamMap):
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    OUString sName, sValue;
    bool bFoundValue = false; // to allow empty values
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
                bFoundValue=true;
            }
            else if( IsXMLToken(aLocalName, XML_NAME) )
            {
                sName = rValue;
            }
        }
    }
    if (!sName.isEmpty() && bFoundValue )
        rParamMap[sName] = sValue;
}
class XMLTextFrameContourContext_Impl : public SvXMLImportContext
{
    Reference < XPropertySet > xPropSet;

public:


    XMLTextFrameContourContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            const Reference < XPropertySet >& rPropSet,
            bool bPath );
};


XMLTextFrameContourContext_Impl::XMLTextFrameContourContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        const Reference < XPropertySet >& rPropSet,
        bool bPath ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xPropSet( rPropSet )
{
    OUString sD, sPoints, sViewBox;
    bool bPixelWidth = false, bPixelHeight = false;
    bool bAuto = false;
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
            if (::sax::Converter::convertMeasurePx(nWidth, rValue))
                bPixelWidth = true;
            else
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nWidth, rValue);
            break;
        case XML_TOK_TEXT_CONTOUR_HEIGHT:
            if (::sax::Converter::convertMeasurePx(nHeight, rValue))
                bPixelHeight = true;
            else
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nHeight, rValue);
            break;
        case XML_TOK_TEXT_CONTOUR_AUTO:
            bAuto = IsXMLToken(rValue, XML_TRUE);
            break;
        }
    }

    OUString sContourPolyPolygon("ContourPolyPolygon");
    Reference < XPropertySetInfo > xPropSetInfo = rPropSet->getPropertySetInfo();

    if(xPropSetInfo->hasPropertyByName(sContourPolyPolygon) && nWidth > 0 && nHeight > 0 && bPixelWidth == bPixelHeight && (bPath ? sD : sPoints).getLength())
    {
        const SdXMLImExViewBox aViewBox( sViewBox, GetImport().GetMM100UnitConverter());
        basegfx::B2DPolyPolygon aPolyPolygon;

        if( bPath )
        {
            basegfx::utils::importFromSvgD(aPolyPolygon, sD, GetImport().needFixPositionAfterZ(), nullptr);
        }
        else
        {
            basegfx::B2DPolygon aPolygon;

            if(basegfx::utils::importFromSvgPoints(aPolygon, sPoints))
            {
                aPolyPolygon = basegfx::B2DPolyPolygon(aPolygon);
            }
        }

        if(aPolyPolygon.count())
        {
            const basegfx::B2DRange aSourceRange(
                aViewBox.GetX(), aViewBox.GetY(),
                aViewBox.GetX() + aViewBox.GetWidth(), aViewBox.GetY() + aViewBox.GetHeight());
            const basegfx::B2DRange aTargetRange(
                0.0, 0.0,
                nWidth, nHeight);

            if(!aSourceRange.equal(aTargetRange))
            {
                aPolyPolygon.transform(
                    basegfx::utils::createSourceRangeTargetRangeTransform(
                        aSourceRange,
                        aTargetRange));
            }

            css::drawing::PointSequenceSequence aPointSequenceSequence;
            basegfx::utils::B2DPolyPolygonToUnoPointSequenceSequence(aPolyPolygon, aPointSequenceSequence);
            xPropSet->setPropertyValue( sContourPolyPolygon, Any(aPointSequenceSequence) );
        }

        const OUString sIsPixelContour("IsPixelContour");

        if( xPropSetInfo->hasPropertyByName( sIsPixelContour ) )
        {
            xPropSet->setPropertyValue( sIsPixelContour, Any(bPixelWidth) );
        }

        const OUString sIsAutomaticContour("IsAutomaticContour");

        if( xPropSetInfo->hasPropertyByName( sIsAutomaticContour ) )
        {
            xPropSet->setPropertyValue( sIsAutomaticContour, Any(bAuto) );
        }
    }
}

class XMLTextFrameContext_Impl : public SvXMLImportContext
{
    css::uno::Reference < css::text::XTextCursor > xOldTextCursor;
    css::uno::Reference < css::beans::XPropertySet > xPropSet;
    css::uno::Reference < css::io::XOutputStream > xBase64Stream;

    /// old list item and block (#89891#)
    bool mbListContextPushed;

    OUString m_sOrigName;
    OUString sName;
    OUString sStyleName;
    OUString sNextName;
    OUString sHRef;
    OUString sFilterName;
    OUString sCode;
    OUString sMimeType;
    OUString sFrameName;
    OUString sAppletName;
    OUString sFilterService;
    OUString sBase64CharsLeft;
    OUString sTblName;

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

    sal_uInt16 const nType;
    css::text::TextContentAnchorType   eAnchorType;

    bool    bMayScript : 1;
    bool    bMinWidth : 1;
    bool    bMinHeight : 1;
    bool    bSyncWidth : 1;
    bool    bSyncHeight : 1;
    bool    bCreateFailed : 1;
    bool    bOwnBase64Stream : 1;
    bool    mbMultipleContent : 1; // This context is created based on a multiple content (image)

    void Create();

public:


    bool CreateIfNotThere();
    const OUString& GetHRef() const { return sHRef; }

    XMLTextFrameContext_Impl( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference<css::xml::sax::XAttributeList > & rAttrList,
            css::text::TextContentAnchorType eAnchorType,
            sal_uInt16 nType,
            const css::uno::Reference<css::xml::sax::XAttributeList > & rFrameAttrList,
            bool bMultipleContent = false );

    virtual void EndElement() override;

    virtual void Characters( const OUString& rChars ) override;

    SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                 const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    void SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap );

    // Implement Title/Description Elements UI (#i73249#)
    void SetTitle( const OUString& rTitle );

    void SetDesc( const OUString& rDesc );

    void SetName();

    const OUString& GetOrigName() const { return m_sOrigName; }

    css::text::TextContentAnchorType GetAnchorType() const { return eAnchorType; }

    const css::uno::Reference < css::beans::XPropertySet >& GetPropSet() const { return xPropSet; }
};


void XMLTextFrameContext_Impl::Create()
{
    rtl::Reference < XMLTextImportHelper > xTextImportHelper =
        GetImport().GetTextImport();

    switch ( nType)
    {
        case XML_TEXT_FRAME_OBJECT:
        case XML_TEXT_FRAME_OBJECT_OLE:
            if( xBase64Stream.is() )
            {
                OUString sURL( GetImport().ResolveEmbeddedObjectURLFromBase64() );
                if( !sURL.isEmpty() )
                    xPropSet = GetImport().GetTextImport()
                            ->createAndInsertOLEObject( GetImport(), sURL,
                                                        sStyleName,
                                                        sTblName,
                                                        nWidth, nHeight );
            }
            else if( !sHRef.isEmpty() )
            {
                OUString sURL( GetImport().ResolveEmbeddedObjectURL( sHRef,
                                                                OUString() ) );

                if( GetImport().IsPackageURL( sHRef ) )
                {
                    xPropSet = GetImport().GetTextImport()
                            ->createAndInsertOLEObject( GetImport(), sURL,
                                                        sStyleName,
                                                        sTblName,
                                                        nWidth, nHeight );
                }
                else
                {
                    // it should be an own OOo link that has no storage persistence
                    xPropSet = GetImport().GetTextImport()
                            ->createAndInsertOOoLink( GetImport(),
                                                        sURL,
                                                        sStyleName,
                                                        sTblName,
                                                        nWidth, nHeight );
                }
            }
            else
            {
                OUString sURL( "vnd.sun.star.ServiceName:" );
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
            if(!sHRef.isEmpty())
                GetImport().GetAbsoluteReference(sHRef);
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
                    case XML_TEXT_FRAME_TEXTBOX: sServiceName = "com.sun.star.text.TextFrame"; break;
                    case XML_TEXT_FRAME_GRAPHIC: sServiceName = "com.sun.star.text.GraphicObject"; break;
                }
                Reference<XInterface> xIfc = xFactory->createInstance( sServiceName );
                SAL_WARN_IF( !xIfc.is(), "xmloff.text", "couldn't create frame" );
                if( xIfc.is() )
                    xPropSet.set( xIfc, UNO_QUERY );
            }
        }
    }

    if( !xPropSet.is() )
    {
        bCreateFailed = true;
        return;
    }

    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // Skip duplicated frames
    if(!mbMultipleContent && // It's allowed to have multiple image for the same frame
       !sName.isEmpty() &&
       xTextImportHelper->IsDuplicateFrame(sName, nX, nY, nWidth, nHeight))
    {
        bCreateFailed = true;
        return;
    }

    // set name
    Reference < XNamed > xNamed( xPropSet, UNO_QUERY );
    if( xNamed.is() )
    {
        OUString sOrigName( xNamed->getName() );
        if( sOrigName.isEmpty() ||
            (!sName.isEmpty() && sOrigName != sName) )
        {
            OUString sOldName( sName );

            sal_Int32 i = 0;
            while( xTextImportHelper->HasFrameByName( sName ) )
            {
                sName = sOldName + OUString::number( ++i );
            }
            xNamed->setName( sName );
            if( sName != sOldName )
            {
                xTextImportHelper->GetRenameMap().Add( XML_TEXT_RENAME_TYPE_FRAME,
                                             sOldName, sName );

            }
        }
    }

    // frame style
    XMLPropStyleContext *pStyle = nullptr;
    if( !sStyleName.isEmpty() )
    {
        pStyle = xTextImportHelper->FindAutoFrameStyle( sStyleName );
        if( pStyle )
            sStyleName = pStyle->GetParentName();
    }

    Any aAny;
    if( !sStyleName.isEmpty() )
    {
        OUString sDisplayStyleName( GetImport().GetStyleDisplayName(
                            XML_STYLE_FAMILY_SD_GRAPHICS_ID, sStyleName ) );
        const Reference < XNameContainer > & rStyles =
            xTextImportHelper->GetFrameStyles();
        if( rStyles.is() &&
            rStyles->hasByName( sDisplayStyleName ) )
        {
            xPropSet->setPropertyValue( "FrameStyleName", Any(sDisplayStyleName) );
        }
    }

    // anchor type (must be set before any other properties, because
    // otherwise some orientations cannot be set or will be changed
    // afterwards)
    xPropSet->setPropertyValue( "AnchorType", Any(eAnchorType) );

    // hard properties
    if( pStyle )
        pStyle->FillPropertySet( xPropSet );

    // x and y
    sal_Int16 nHoriOrient =  HoriOrientation::NONE;
    aAny = xPropSet->getPropertyValue( "HoriOrient" );
    aAny >>= nHoriOrient;
    if( HoriOrientation::NONE == nHoriOrient )
    {
        xPropSet->setPropertyValue( "HoriOrientPosition", Any(nX) );
    }

    sal_Int16 nVertOrient =  VertOrientation::NONE;
    aAny = xPropSet->getPropertyValue( "VertOrient" );
    aAny >>= nVertOrient;
    if( VertOrientation::NONE == nVertOrient )
    {
        xPropSet->setPropertyValue( "VertOrientPosition", Any(nY) );
    }

    // width
    if( nWidth > 0 )
    {
        xPropSet->setPropertyValue( "Width", Any(nWidth) );
    }
    if( nRelWidth > 0 || nWidth > 0 )
    {
        xPropSet->setPropertyValue( "RelativeWidth", Any(nRelWidth) );
    }
    if( bSyncWidth || nWidth > 0 )
    {
        xPropSet->setPropertyValue( "IsSyncWidthToHeight", Any(bSyncWidth) );
    }
    if( xPropSetInfo->hasPropertyByName( "WidthType" ) &&
        (bMinWidth || nWidth > 0 || nRelWidth > 0 ) )
    {
        sal_Int16 nSizeType =
            (bMinWidth && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                           : SizeType::FIX;
        xPropSet->setPropertyValue( "WidthType", Any(nSizeType) );
    }

    if( nHeight > 0 )
    {
        xPropSet->setPropertyValue( "Height", Any(nHeight) );
    }
    if( nRelHeight > 0 || nHeight > 0 )
    {
        xPropSet->setPropertyValue( "RelativeHeight", Any(nRelHeight) );
    }
    if( bSyncHeight || nHeight > 0 )
    {
        xPropSet->setPropertyValue( "IsSyncHeightToWidth", Any(bSyncHeight) );
    }
    if( xPropSetInfo->hasPropertyByName( "SizeType" ) &&
        (bMinHeight || nHeight > 0 || nRelHeight > 0 ) )
    {
        sal_Int16 nSizeType =
            (bMinHeight && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                            : SizeType::FIX;
        xPropSet->setPropertyValue( "SizeType", Any(nSizeType) );
    }

    if( XML_TEXT_FRAME_GRAPHIC == nType )
    {
        // URL
        OSL_ENSURE( !sHRef.isEmpty() || xBase64Stream.is(),
                    "neither URL nor base64 image data given" );
        rtl::Reference < XMLTextImportHelper > xTxtImport =
            GetImport().GetTextImport();
        uno::Reference<graphic::XGraphic> xGraphic;
        if (!sHRef.isEmpty())
        {
            xGraphic = GetImport().loadGraphicByURL(sHRef);
        }
        else if (xBase64Stream.is())
        {
            xGraphic = GetImport().loadGraphicFromBase64(xBase64Stream);
            xBase64Stream = nullptr;
        }

        if (xGraphic.is())
            xPropSet->setPropertyValue("Graphic", Any(xGraphic));

        // filter name
        xPropSet->setPropertyValue( "GraphicFilter", Any(sFilterName) );

        // rotation
        xPropSet->setPropertyValue( "GraphicRotation", Any(nRotation) );
    }

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    if( TextContentAnchorType_AT_PAGE == eAnchorType && nPage > 0 )
    {
        xPropSet->setPropertyValue( "AnchorPageNo", Any(nPage) );
    }

    if( XML_TEXT_FRAME_OBJECT != nType  &&
        XML_TEXT_FRAME_OBJECT_OLE != nType  &&
        XML_TEXT_FRAME_APPLET != nType &&
        XML_TEXT_FRAME_PLUGIN!= nType &&
        XML_TEXT_FRAME_FLOATING_FRAME != nType)
    {
        Reference < XTextContent > xTxtCntnt( xPropSet, UNO_QUERY );
        try
        {
            xTextImportHelper->InsertTextContent(xTxtCntnt);
        }
        catch (lang::IllegalArgumentException const& e)
        {
            SAL_WARN("xmloff.text", "Cannot import part of the text - probably an image in the text frame? " << e);
            return;
        }
    }

    // Make adding the shape to Z-Ordering dependent from if we are
    // inside a inside_deleted_section (redlining). That is necessary
    // since the shape will be removed again later. It would lead to
    // errors if it would stay inside the Z-Ordering. Thus, the
    // easiest way to solve that conflict is to not add it here.
    if(!GetImport().HasTextImport()
        || !GetImport().GetTextImport()->IsInsideDeleteContext())
    {
        Reference < XShape > xShape( xPropSet, UNO_QUERY );

        GetImport().GetShapeImport()->shapeWithZIndexAdded( xShape, nZIndex );
    }

    if( XML_TEXT_FRAME_TEXTBOX == nType )
    {
        xTextImportHelper->ConnectFrameChains( sName, sNextName, xPropSet );
        Reference < XTextFrame > xTxtFrame( xPropSet, UNO_QUERY );
        Reference < XText > xTxt = xTxtFrame->getText();
        xOldTextCursor = xTextImportHelper->GetCursor();
        xTextImportHelper->SetCursor( xTxt->createTextCursor() );

        // remember old list item and block (#89892#) and reset them
        // for the text frame
        xTextImportHelper->PushListContext();
        mbListContextPushed = true;
    }
}

void XMLTextFrameContext::removeGraphicFromImportContext(const SvXMLImportContext& rContext)
{
    const XMLTextFrameContext_Impl* pXMLTextFrameContext_Impl = dynamic_cast< const XMLTextFrameContext_Impl* >(&rContext);

    if(pXMLTextFrameContext_Impl)
    {
        try
        {
            // just dispose to delete
            uno::Reference< lang::XComponent > xComp(pXMLTextFrameContext_Impl->GetPropSet(), UNO_QUERY);

            // Inform shape importer about the removal so it can adjust
            // z-indexes.
            uno::Reference<drawing::XShape> xShape(xComp, uno::UNO_QUERY);
            GetImport().GetShapeImport()->shapeRemoved(xShape);

            if(xComp.is())
            {
                xComp->dispose();
            }
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Error in cleanup of multiple graphic object import (!)" );
        }
    }
}

OUString XMLTextFrameContext::getGraphicPackageURLFromImportContext(const SvXMLImportContext& rContext) const
{
    const XMLTextFrameContext_Impl* pXMLTextFrameContext_Impl = dynamic_cast< const XMLTextFrameContext_Impl* >(&rContext);

    if(pXMLTextFrameContext_Impl)
    {
        return "vnd.sun.star.Package:" + pXMLTextFrameContext_Impl->GetHRef();
    }

    return OUString();
}

css::uno::Reference<css::graphic::XGraphic> XMLTextFrameContext::getGraphicFromImportContext(const SvXMLImportContext& rContext) const
{
    uno::Reference<graphic::XGraphic> xGraphic;

    const XMLTextFrameContext_Impl* pXMLTextFrameContext_Impl = dynamic_cast<const XMLTextFrameContext_Impl*>(&rContext);

    if (pXMLTextFrameContext_Impl)
    {
        try
        {
            const uno::Reference<beans::XPropertySet>& xPropertySet = pXMLTextFrameContext_Impl->GetPropSet();

            if (xPropertySet.is())
            {
                xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
            }
        }
        catch (uno::Exception&)
        {}
    }
    return xGraphic;
}

bool XMLTextFrameContext_Impl::CreateIfNotThere()
{
    if( !xPropSet.is() &&
        ( XML_TEXT_FRAME_OBJECT_OLE == nType ||
          XML_TEXT_FRAME_GRAPHIC == nType ) &&
        xBase64Stream.is() && !bCreateFailed )
    {
        if( bOwnBase64Stream )
            xBase64Stream->closeOutput();
        Create();
    }

    return xPropSet.is();
}

XMLTextFrameContext_Impl::XMLTextFrameContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & rAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nNewType,
        const Reference< XAttributeList > & rFrameAttrList,
        bool bMultipleContent )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   mbListContextPushed( false )
,   nType( nNewType )
,   eAnchorType( eATyp )
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
    bMayScript = false;

    bMinHeight = false;
    bMinWidth = false;
    bSyncWidth = false;
    bSyncHeight = false;
    bCreateFailed = false;
    bOwnBase64Stream = false;
    mbMultipleContent = bMultipleContent;

    rtl::Reference < XMLTextImportHelper > xTxtImport =
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
            m_sOrigName = rValue;
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
                if( XMLAnchorTypePropHdl::convert( rValue, eNew ) &&
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
                if (::sax::Converter::convertNumber(nTmp, rValue, 1, SHRT_MAX))
                    nPage = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_TOK_TEXT_FRAME_X:
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nX, rValue);
            break;
        case XML_TOK_TEXT_FRAME_Y:
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nY, rValue );
            break;
        case XML_TOK_TEXT_FRAME_WIDTH:
            // relative widths are obsolete since SRC617. Remove them some day!
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, rValue );
                nRelWidth = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nWidth, rValue, 0 );
            }
            break;
        case XML_TOK_TEXT_FRAME_REL_WIDTH:
            if( IsXMLToken(rValue, XML_SCALE) )
            {
                bSyncWidth = true;
            }
            else
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertPercent( nTmp, rValue ))
                    nRelWidth = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_WIDTH:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, rValue );
                nRelWidth = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nWidth, rValue, 0 );
            }
            bMinWidth = true;
            break;
        case XML_TOK_TEXT_FRAME_HEIGHT:
            // relative heights are obsolete since SRC617. Remove them some day!
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, rValue );
                nRelHeight = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nHeight, rValue, 0 );
            }
            break;
        case XML_TOK_TEXT_FRAME_REL_HEIGHT:
            if( IsXMLToken( rValue, XML_SCALE ) )
            {
                bSyncHeight = true;
            }
            else if( IsXMLToken( rValue, XML_SCALE_MIN ) )
            {
                bSyncHeight = true;
                bMinHeight = true;
            }
            else
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertPercent( nTmp, rValue ))
                    nRelHeight = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_HEIGHT:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, rValue );
                nRelHeight = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nHeight, rValue, 0 );
            }
            bMinHeight = true;
            break;
        case XML_TOK_TEXT_FRAME_Z_INDEX:
            ::sax::Converter::convertNumber( nZIndex, rValue, -1 );
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
                // RotateFlyFrameFix: im/export full 'draw:transform' using existing tooling
                // Currently only rotation is used, but combinations with 'draw:transform'
                // may be necessary in the future, so that svg:x/svg:y/svg:width/svg:height
                // may be extended/replaced with 'draw:transform' (see draw objects)
                SdXMLImExTransform2D aSdXMLImExTransform2D;
                basegfx::B2DHomMatrix aFullTransform;

                // Use SdXMLImExTransform2D to convert to transformation
                // Note: using GetTwipUnitConverter instead of GetMM100UnitConverter may be needed,
                // but is not generally available (as it should be, a 'current' UnitConverter should
                // be available at GetExport() - and maybe was once). May have to be addressed as soon
                // as translate transformations are used here.
                aSdXMLImExTransform2D.SetString(rValue, GetImport().GetMM100UnitConverter());
                aSdXMLImExTransform2D.GetFullTransform(aFullTransform);

                if(!aFullTransform.isIdentity())
                {
                    const basegfx::utils::B2DHomMatrixBufferedDecompose aDecomposedTransform(aFullTransform);

                    // currently we *only* use rotation (and translation indirectly), so warn if *any*
                    // of the other transform parts is used
                    SAL_WARN_IF(!basegfx::fTools::equal(1.0, aDecomposedTransform.getScale().getX()), "xmloff.text", "draw:transform uses scaleX" );
                    SAL_WARN_IF(!basegfx::fTools::equal(1.0, aDecomposedTransform.getScale().getY()), "xmloff.text", "draw:transform uses scaleY" );
                    SAL_WARN_IF(!basegfx::fTools::equalZero(aDecomposedTransform.getShearX()), "xmloff.text", "draw:transform uses shearX" );

                    // Translation comes from the translate to RotCenter, rot and BackTranslate.
                    // This means that it represents the translation between unrotated TopLeft
                    // and rotated TopLeft. This may be checked here now, but currently we only
                    // use rotation around center and assume that this *was* a rotation around
                    // center. The check would compare the object's center with the RotCenter
                    // that can be extracted from the transformation in aFullTransform.
                    // The definition contains implicitly the RotationCenter absolute
                    // to the scaled and translated object, so this may be used if needed (see
                    // _exportTextGraphic how the -trans/rot/trans is composed)

                    if(!basegfx::fTools::equalZero(aDecomposedTransform.getRotate()))
                    {
                        // rotation is used, set it. Convert from deg to 10th degree integer
                        // CAUTION: due to #i78696# (rotation mirrored using API) the rotate
                        // value is already mirrored, so do not do it again here (to be in sync
                        // with XMLTextParagraphExport::_exportTextGraphic normally it would need
                        // to me mirrored using * -1.0, see conversion there)
                        // CAUTION-II: due to tdf#115782 it is better for current ODF to indeed use it
                        // with the wrong orientation as in all other cases - ARGH! We will need to
                        // correct this in future ODF ASAP! For now, mirror the rotation here AGAIN
                        const double fRotate(aDecomposedTransform.getRotate() * (-1800.0/M_PI));
                        nRotation = static_cast< sal_Int16 >(basegfx::fround(fRotate) % 3600);

                        // tdf#115529 may be negative, with the above modulo maximal -3599, so
                        // no loop needed here. nRotation is used in setPropertyValue("GraphicRotation")
                        // and *has* to be in the range [0 .. 3600[
                        if(nRotation < 0)
                        {
                            nRotation += 3600;
                        }
                    }
                }
            }
            break;
        case XML_TOK_TEXT_FRAME_CODE:
            sCode = rValue;
            break;
        case XML_TOK_TEXT_FRAME_OBJECT:
            break;
        case XML_TOK_TEXT_FRAME_ARCHIVE:
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
          sHRef.isEmpty() ) ||
        ( XML_TEXT_FRAME_APPLET  == nType && sCode.isEmpty() ) ||
        ( XML_TEXT_FRAME_PLUGIN == nType &&
          sHRef.isEmpty() && sMimeType.isEmpty() ) )
        return; // no URL: no image or OLE object

    Create();
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
    if (mbListContextPushed) {
        GetImport().GetTextImport()->PopListContext();
    }

    if (( nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN ) && xPropSet.is())
        GetImport().GetTextImport()->endAppletOrPlugin( xPropSet, aParamMap);
}

SvXMLImportContextRef XMLTextFrameContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( (nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN) &&
              IsXMLToken( rLocalName, XML_PARAM ) )
        {
            pContext = new XMLTextFrameParam_Impl( GetImport(),
                                              nPrefix, rLocalName,
                                               xAttrList, aParamMap );
        }
    }
    else if( XML_NAMESPACE_OFFICE == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_BINARY_DATA ) )
        {
            if( !xPropSet.is() && !xBase64Stream.is() && !bCreateFailed )
            {
                switch( nType )
                {
                case XML_TEXT_FRAME_GRAPHIC:
                    xBase64Stream =
                        GetImport().GetStreamForGraphicObjectURLFromBase64();
                    break;
                case XML_TEXT_FRAME_OBJECT_OLE:
                    xBase64Stream =
                        GetImport().GetStreamForEmbeddedObjectURLFromBase64();
                    break;
                }
                if( xBase64Stream.is() )
                    pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    xBase64Stream );
            }
        }
    }
    // Correction of condition which also avoids warnings. (#i100480#)
    if( !pContext &&
        ( XML_TEXT_FRAME_OBJECT == nType &&
          ( ( XML_NAMESPACE_OFFICE == nPrefix &&
              IsXMLToken( rLocalName, XML_DOCUMENT ) ) ||
            ( XML_NAMESPACE_MATH == nPrefix &&
              IsXMLToken( rLocalName, XML_MATH ) ) ) ) )
    {
        if( !xPropSet.is() && !bCreateFailed )
        {
            XMLEmbeddedObjectImportContext *pEContext =
                new XMLEmbeddedObjectImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList );
            sFilterService = pEContext->GetFilterServiceName();
            if( !sFilterService.isEmpty() )
            {
                Create();
                if( xPropSet.is() )
                {
                    Reference < XEmbeddedObjectSupplier > xEOS( xPropSet,
                                                                UNO_QUERY );
                    OSL_ENSURE( xEOS.is(),
                            "no embedded object supplier for own object" );
                    Reference<css::lang::XComponent> aXComponent(xEOS->getEmbeddedObject());
                    pEContext->SetComponent( aXComponent );
                }
            }
            pContext = pEContext;
        }
    }
    if( !pContext && xOldTextCursor.is() )  // text-box
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
                            GetImport(), nPrefix, rLocalName, xAttrList,
                            XMLTextType::TextBox );

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
        if( !sTrimmedChars.isEmpty() )
        {
            if( !xBase64Stream.is() )
            {
                if( XML_TEXT_FRAME_GRAPHIC == nType )
                {
                    xBase64Stream =
                        GetImport().GetStreamForGraphicObjectURLFromBase64();
                }
                else
                {
                    xBase64Stream =
                        GetImport().GetStreamForEmbeddedObjectURLFromBase64();
                }
                if( xBase64Stream.is() )
                    bOwnBase64Stream = true;
            }
            if( bOwnBase64Stream && xBase64Stream.is() )
            {
                OUString sChars;
                if( !sBase64CharsLeft.isEmpty() )
                {
                    sChars = sBase64CharsLeft;
                    sChars += sTrimmedChars;
                    sBase64CharsLeft.clear();
                }
                else
                {
                    sChars = sTrimmedChars;
                }
                Sequence< sal_Int8 > aBuffer( (sChars.getLength() / 4) * 3 );
                sal_Int32 nCharsDecoded =
                    ::comphelper::Base64::decodeSomeChars( aBuffer, sChars );
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
                       bool bMap )
{
    static const char s_HyperLinkURL[] = "HyperLinkURL";
    static const char s_HyperLinkName[] = "HyperLinkName";
    static const char s_HyperLinkTarget[] = "HyperLinkTarget";
    static const char s_ServerMap[] = "ServerMap";
    if( !xPropSet.is() )
        return;

    rtl::Reference< XMLTextImportHelper > xTxtImp = GetImport().GetTextImport();
    Reference < XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if( !xPropSetInfo.is() ||
        !xPropSetInfo->hasPropertyByName(s_HyperLinkURL))
        return;

    xPropSet->setPropertyValue( s_HyperLinkURL, Any(rHRef) );

    if (xPropSetInfo->hasPropertyByName(s_HyperLinkName))
    {
        xPropSet->setPropertyValue(s_HyperLinkName, Any(rName));
    }

    if (xPropSetInfo->hasPropertyByName(s_HyperLinkTarget))
    {
        xPropSet->setPropertyValue( s_HyperLinkTarget, Any(rTargetFrameName) );
    }

    if (xPropSetInfo->hasPropertyByName(s_ServerMap))
    {
        xPropSet->setPropertyValue(s_ServerMap, Any(bMap));
    }
}

void XMLTextFrameContext_Impl::SetName()
{
    Reference<XNamed> xNamed(xPropSet, UNO_QUERY);
    if (!m_sOrigName.isEmpty() && xNamed.is())
    {
        OUString const name(xNamed->getName());
        if (name != m_sOrigName)
        {
            try
            {
                xNamed->setName(m_sOrigName);
            }
            catch (uno::Exception const& e)
            {   // fdo#71698 document contains 2 frames with same draw:name
                SAL_INFO("xmloff.text", "SetName(): exception setting \""
                        << m_sOrigName << "\": " << e);
            }
        }
    }
}

// Implement Title/Description Elements UI (#i73249#)
void XMLTextFrameContext_Impl::SetTitle( const OUString& rTitle )
{
    if ( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( "Title" ) )
        {
            xPropSet->setPropertyValue( "Title", makeAny( rTitle ) );
        }
    }
}

void XMLTextFrameContext_Impl::SetDesc( const OUString& rDesc )
{
    if ( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( "Description" ) )
        {
            xPropSet->setPropertyValue( "Description", makeAny( rDesc ) );
        }
    }
}


bool XMLTextFrameContext::CreateIfNotThere( css::uno::Reference < css::beans::XPropertySet >& rPropSet )
{
    SvXMLImportContext *pContext = m_xImplContext.get();
    XMLTextFrameContext_Impl *pImpl = dynamic_cast< XMLTextFrameContext_Impl*>( pContext );
    if( pImpl && pImpl->CreateIfNotThere() )
        rPropSet = pImpl->GetPropSet();

    return rPropSet.is();
}

XMLTextFrameContext::XMLTextFrameContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        TextContentAnchorType eATyp )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   MultiImageImportHelper()
,   m_xAttrList( new SvXMLAttributeList( xAttrList ) )
    // Implement Title/Description Elements UI (#i73249#)
,   m_sTitle()
,   m_sDesc()
,   m_eDefaultAnchorType( eATyp )
    // Shapes in Writer cannot be named via context menu (#i51726#)
,   m_HasAutomaticStyleWithoutParentStyle( false )
,   m_bSupportsReplacement( false )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
        // New distinguish attribute between Writer objects and Draw objects is:
        // Draw objects have an automatic style without a parent style (#i51726#)
        if ( XML_NAMESPACE_DRAW == nPrefix &&
             IsXMLToken( aLocalName, XML_STYLE_NAME ) )
        {
            OUString aStyleName = xAttrList->getValueByIndex( i );
            if( !aStyleName.isEmpty() )
            {
                rtl::Reference < XMLTextImportHelper > xTxtImport =
                                                    GetImport().GetTextImport();
                XMLPropStyleContext* pStyle( nullptr );
                pStyle = xTxtImport->FindAutoFrameStyle( aStyleName );
                if ( pStyle && pStyle->GetParentName().isEmpty() )
                {
                    m_HasAutomaticStyleWithoutParentStyle = true;
                }
            }
        }
        else if ( XML_NAMESPACE_TEXT == nPrefix &&
                  IsXMLToken( aLocalName, XML_ANCHOR_TYPE ) )
        {
            TextContentAnchorType eNew;
            if( XMLAnchorTypePropHdl::convert( xAttrList->getValueByIndex(i),
                        eNew ) &&
                ( TextContentAnchorType_AT_PARAGRAPH == eNew ||
                  TextContentAnchorType_AT_CHARACTER == eNew ||
                  TextContentAnchorType_AS_CHARACTER == eNew ||
                  TextContentAnchorType_AT_PAGE == eNew) )
                m_eDefaultAnchorType = eNew;
        }
    }
}

void XMLTextFrameContext::EndElement()
{
    /// solve if multiple image child contexts were imported
    SvXMLImportContextRef const pMultiContext(solveMultipleImages());

    SvXMLImportContext const*const pContext =
        (pMultiContext.is()) ? pMultiContext.get() : m_xImplContext.get();
    XMLTextFrameContext_Impl *pImpl = const_cast<XMLTextFrameContext_Impl*>(dynamic_cast< const XMLTextFrameContext_Impl*>( pContext ));
    assert(!pMultiContext.is() || pImpl);
    if( pImpl )
    {
        pImpl->CreateIfNotThere();

        // fdo#68839: in case the surviving image was not the first one,
        // it will have a counter added to its name - set the original name
        if (pMultiContext.is()) // do this only when necessary; esp. not for text
        {                  // frames that may have entries in GetRenameMap()!
            pImpl->SetName();
        }

        if( !m_sTitle.isEmpty() )
        {
            pImpl->SetTitle( m_sTitle );
        }
        if( !m_sDesc.isEmpty() )
        {
            pImpl->SetDesc( m_sDesc );
        }

        if( m_pHyperlink )
        {
            pImpl->SetHyperlink( m_pHyperlink->GetHRef(), m_pHyperlink->GetName(),
                          m_pHyperlink->GetTargetFrameName(), m_pHyperlink->GetMap() );
            m_pHyperlink.reset();
        }

        GetImport().GetTextImport()->StoreLastImportedFrameName(pImpl->GetOrigName());

    }
}

SvXMLImportContextRef XMLTextFrameContext::CreateChildContext(
        sal_uInt16 p_nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    if( !m_xImplContext.is() )
    {
        // no child exists
        if( XML_NAMESPACE_DRAW == p_nPrefix )
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
                // Shapes in Writer cannot be named via context menu (#i51726#)
                if ( ( XML_TEXT_FRAME_TEXTBOX == nFrameType ||
                       XML_TEXT_FRAME_GRAPHIC == nFrameType ) &&
                     m_HasAutomaticStyleWithoutParentStyle )
                {
                    Reference < XShapes > xShapes;
                    xContext = GetImport().GetShapeImport()->CreateFrameChildContext(
                                    GetImport(), p_nPrefix, rLocalName, xAttrList, xShapes, m_xAttrList );
                }
                else if( XML_TEXT_FRAME_PLUGIN == nFrameType )
                {
                    bool bMedia = false;

                    // check, if we have a media object
                    for( sal_Int16 n = 0, nAttrCount = ( xAttrList.is() ? xAttrList->getLength() : 0 ); n < nAttrCount; ++n )
                    {
                        OUString    aLocalName;
                        sal_uInt16  nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( n ), &aLocalName );

                        if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( aLocalName, XML_MIME_TYPE ) )
                        {
                            if( xAttrList->getValueByIndex( n ) == "application/vnd.sun.star.media" )
                                bMedia = true;

                            // leave this loop
                            n = nAttrCount - 1;
                        }
                    }

                    if( bMedia )
                    {
                        Reference < XShapes > xShapes;
                        xContext = GetImport().GetShapeImport()->CreateFrameChildContext(
                                        GetImport(), p_nPrefix, rLocalName, xAttrList, xShapes, m_xAttrList );
                    }
                }
                else if( XML_TEXT_FRAME_OBJECT == nFrameType ||
                         XML_TEXT_FRAME_OBJECT_OLE == nFrameType )
                {
                    m_bSupportsReplacement = true;
                }
                else if(XML_TEXT_FRAME_GRAPHIC == nFrameType)
                {
                    setSupportsMultipleContents(IsXMLToken(rLocalName, XML_IMAGE));
                }

                if (!xContext)
                {
                    xContext = new XMLTextFrameContext_Impl( GetImport(), p_nPrefix,
                                                        rLocalName, xAttrList,
                                                        m_eDefaultAnchorType,
                                                        nFrameType,
                                                        m_xAttrList );
                }

                m_xImplContext = xContext;

                if(getSupportsMultipleContents() && XML_TEXT_FRAME_GRAPHIC == nFrameType)
                {
                    addContent(*m_xImplContext.get());
                }
            }
        }
    }
    else if(getSupportsMultipleContents() && XML_NAMESPACE_DRAW == p_nPrefix && IsXMLToken(rLocalName, XML_IMAGE))
    {
        // read another image
        xContext = new XMLTextFrameContext_Impl(
            GetImport(), p_nPrefix, rLocalName, xAttrList,
            m_eDefaultAnchorType, XML_TEXT_FRAME_GRAPHIC, m_xAttrList, true);

        m_xImplContext = xContext;
        addContent(*m_xImplContext.get());
    }
    else if( m_bSupportsReplacement && !m_xReplImplContext.is() &&
             XML_NAMESPACE_DRAW == p_nPrefix &&
             IsXMLToken( rLocalName, XML_IMAGE ) )
    {
        // read replacement image
        Reference < XPropertySet > xPropSet;
        if( CreateIfNotThere( xPropSet ) )
        {
            xContext = new XMLReplacementImageContext( GetImport(),
                                p_nPrefix, rLocalName, xAttrList, xPropSet );
            m_xReplImplContext = xContext;
        }
    }
    else if( nullptr != dynamic_cast< const XMLTextFrameContext_Impl*>( m_xImplContext.get() ))
    {
        // the child is a writer frame
        if( XML_NAMESPACE_SVG == p_nPrefix )
        {
            // Implement Title/Description Elements UI (#i73249#)
            const bool bOld = SvXMLImport::OOo_2x >= GetImport().getGeneratorVersion();
            if ( bOld )
            {
                if ( IsXMLToken( rLocalName, XML_DESC ) )
                {
                    xContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        p_nPrefix,
                                                                        rLocalName,
                                                                        m_sTitle );
                }
            }
            else
            {
                if( IsXMLToken( rLocalName, XML_TITLE ) )
                {
                    if (getSupportsMultipleContents())
                    {   // tdf#103567 ensure props are set on surviving shape
                        m_xImplContext = solveMultipleImages();
                    }
                    xContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        p_nPrefix,
                                                                        rLocalName,
                                                                        m_sTitle );
                }
                else if ( IsXMLToken( rLocalName, XML_DESC ) )
                {
                    if (getSupportsMultipleContents())
                    {   // tdf#103567 ensure props are set on surviving shape
                        m_xImplContext = solveMultipleImages();
                    }
                    xContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        p_nPrefix,
                                                                        rLocalName,
                                                                        m_sDesc );
                }
            }
        }
        else if( XML_NAMESPACE_DRAW == p_nPrefix )
        {
            Reference < XPropertySet > xPropSet;
            if( IsXMLToken( rLocalName, XML_CONTOUR_POLYGON ) )
            {
                if (getSupportsMultipleContents())
                {   // tdf#103567 ensure props are set on surviving shape
                    m_xImplContext = solveMultipleImages();
                }
                if( CreateIfNotThere( xPropSet ) )
                    xContext = new XMLTextFrameContourContext_Impl( GetImport(), p_nPrefix, rLocalName,
                                                  xAttrList, xPropSet, false );
            }
            else if( IsXMLToken( rLocalName, XML_CONTOUR_PATH ) )
            {
                if (getSupportsMultipleContents())
                {   // tdf#103567 ensure props are set on surviving shape
                    m_xImplContext = solveMultipleImages();
                }
                if( CreateIfNotThere( xPropSet ) )
                    xContext = new XMLTextFrameContourContext_Impl( GetImport(), p_nPrefix, rLocalName,
                                                  xAttrList, xPropSet, true );
            }
            else if( IsXMLToken( rLocalName, XML_IMAGE_MAP ) )
            {
                if (getSupportsMultipleContents())
                {   // tdf#103567 ensure props are set on surviving shape
                    m_xImplContext = solveMultipleImages();
                }
                if( CreateIfNotThere( xPropSet ) )
                    xContext = new XMLImageMapContext( GetImport(), p_nPrefix, rLocalName, xPropSet );
            }
        }
        else if( (XML_NAMESPACE_OFFICE == p_nPrefix) && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
        {
            if (getSupportsMultipleContents())
            {   // tdf#103567 ensure props are set on surviving shape
                m_xImplContext = solveMultipleImages();
            }
            // do we still have the frame object?
            Reference < XPropertySet > xPropSet;
            if( CreateIfNotThere( xPropSet ) )
            {
                // is it an event supplier?
                Reference<XEventsSupplier> xEventsSupplier(xPropSet, UNO_QUERY);
                if (xEventsSupplier.is())
                {
                    // OK, we have the events, so create the context
                    xContext = new XMLEventsImportContext(GetImport(), p_nPrefix,
                                                      rLocalName, xEventsSupplier);
                }
            }
        }
    }
    else if( p_nPrefix == XML_NAMESPACE_SVG &&  // #i68101#
                (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) )
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            m_xImplContext = solveMultipleImages();
        }
        xContext = m_xImplContext->CreateChildContext( p_nPrefix, rLocalName, xAttrList );
    }
    else if (p_nPrefix == XML_NAMESPACE_LO_EXT && (IsXMLToken(rLocalName, XML_SIGNATURELINE)))
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            m_xImplContext = solveMultipleImages();
        }
        xContext = m_xImplContext->CreateChildContext(p_nPrefix, rLocalName, xAttrList);
    }
    else
    {
        // the child is a drawing shape
        xContext = XMLShapeImportHelper::CreateFrameChildContext(
                                    m_xImplContext.get(), p_nPrefix, rLocalName, xAttrList );
    }

    if (!xContext)
        xContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );

    return xContext;
}

void XMLTextFrameContext::SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap )
{
    OSL_ENSURE( !m_pHyperlink, "recursive SetHyperlink call" );
    m_pHyperlink = std::make_unique<XMLTextFrameContextHyperlink_Impl>(
                rHRef, rName, rTargetFrameName, bMap );
}

TextContentAnchorType XMLTextFrameContext::GetAnchorType() const
{
    SvXMLImportContext *pContext = m_xImplContext.get();
    XMLTextFrameContext_Impl *pImpl = dynamic_cast< XMLTextFrameContext_Impl*>( pContext );
    if( pImpl )
        return pImpl->GetAnchorType();
    else
        return m_eDefaultAnchorType;
}

Reference < XTextContent > XMLTextFrameContext::GetTextContent() const
{
    Reference < XTextContent > xTxtCntnt;
    SvXMLImportContext *pContext = m_xImplContext.get();
    XMLTextFrameContext_Impl *pImpl = dynamic_cast< XMLTextFrameContext_Impl* >( pContext );
    if( pImpl )
        xTxtCntnt.set( pImpl->GetPropSet(), UNO_QUERY );

    return xTxtCntnt;
}

Reference < XShape > XMLTextFrameContext::GetShape() const
{
    Reference < XShape > xShape;
    SvXMLImportContext* pContext = m_xImplContext.get();
    SvXMLShapeContext* pImpl = dynamic_cast<SvXMLShapeContext*>( pContext  );
    if ( pImpl )
    {
        xShape = pImpl->getShape();
    }

    return xShape;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
