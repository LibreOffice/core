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
#include <tools/diagnose_ex.h>
#include <comphelper/base64.hxx>
#include <com/sun/star/frame/XModel.hpp>
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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
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
    OUString sHRef;
    OUString sName;
    OUString sTargetFrameName;
    bool bMap;

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

namespace {

// Implement Title/Description Elements UI (#i73249#)
class XMLTextFrameTitleOrDescContext_Impl : public SvXMLImportContext
{
    OUString&   mrTitleOrDesc;

public:


    XMLTextFrameTitleOrDescContext_Impl( SvXMLImport& rImport,
                                         OUString& rTitleOrDesc );

    virtual void SAL_CALL characters( const OUString& rText ) override;
};

}

XMLTextFrameTitleOrDescContext_Impl::XMLTextFrameTitleOrDescContext_Impl(
        SvXMLImport& rImport,
        OUString& rTitleOrDesc )
    : SvXMLImportContext( rImport )
    , mrTitleOrDesc( rTitleOrDesc )
{
}

void XMLTextFrameTitleOrDescContext_Impl::characters( const OUString& rText )
{
    mrTitleOrDesc += rText;
}

namespace {

class XMLTextFrameParam_Impl : public SvXMLImportContext
{
public:
    XMLTextFrameParam_Impl( SvXMLImport& rImport,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            ParamMap &rParamMap);
};

}

XMLTextFrameParam_Impl::XMLTextFrameParam_Impl(
        SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
        ParamMap &rParamMap):
    SvXMLImportContext( rImport )
{
    OUString sName, sValue;
    bool bFoundValue = false; // to allow empty values
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(DRAW, XML_VALUE):
            {
                sValue = aIter.toString();
                bFoundValue = true;
                break;
            }
            case XML_ELEMENT(DRAW, XML_NAME):
                sName = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    if (!sName.isEmpty() && bFoundValue )
        rParamMap[sName] = sValue;
}

namespace {

class XMLTextFrameContourContext_Impl : public SvXMLImportContext
{
    Reference < XPropertySet > xPropSet;

public:


    XMLTextFrameContourContext_Impl( SvXMLImport& rImport, sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList,
            const Reference < XPropertySet >& rPropSet,
            bool bPath );
};

}

XMLTextFrameContourContext_Impl::XMLTextFrameContourContext_Impl(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< XFastAttributeList > & xAttrList,
        const Reference < XPropertySet >& rPropSet,
        bool bPath ) :
    SvXMLImportContext( rImport ),
    xPropSet( rPropSet )
{
    OUString sD, sPoints, sViewBox;
    bool bPixelWidth = false, bPixelHeight = false;
    bool bAuto = false;
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(SVG, XML_VIEWBOX):
        case XML_ELEMENT(SVG_COMPAT, XML_VIEWBOX):
            sViewBox = aIter.toString();
            break;
        case XML_ELEMENT(SVG, XML_D):
        case XML_ELEMENT(SVG_COMPAT, XML_D):
            if( bPath )
                sD = aIter.toString();
            break;
        case XML_ELEMENT(DRAW,XML_POINTS):
            if( !bPath )
                sPoints = aIter.toString();
            break;
        case XML_ELEMENT(SVG, XML_WIDTH):
        case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            if (::sax::Converter::convertMeasurePx(nWidth, aIter.toView()))
                bPixelWidth = true;
            else
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nWidth, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_HEIGHT):
        case XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
            if (::sax::Converter::convertMeasurePx(nHeight, aIter.toView()))
                bPixelHeight = true;
            else
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nHeight, aIter.toView());
            break;
        case  XML_ELEMENT(DRAW, XML_RECREATE_ON_EDIT):
            bAuto = IsXMLToken(aIter, XML_TRUE);
            break;
        }
    }

    OUString sContourPolyPolygon("ContourPolyPolygon");
    Reference < XPropertySetInfo > xPropSetInfo = rPropSet->getPropertySetInfo();

    if(!xPropSetInfo->hasPropertyByName(sContourPolyPolygon) ||
        nWidth <= 0 || nHeight <= 0 || bPixelWidth != bPixelHeight ||
        !(bPath ? sD : sPoints).getLength())
        return;

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

namespace {

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
    OUString sCode;
    OUString sMimeType;
    OUString sFrameName;
    OUString sAppletName;
    OUString sFilterService;
    OUString sBase64CharsLeft;
    OUString sTblName;
    OUStringBuffer maUrlBuffer;

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
            sal_Int32 nElement,
            const css::uno::Reference<css::xml::sax::XFastAttributeList > & rAttrList,
            css::text::TextContentAnchorType eAnchorType,
            sal_uInt16 nType,
            const css::uno::Reference<css::xml::sax::XFastAttributeList > & rFrameAttrList,
            bool bMultipleContent = false );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

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

}

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
                OUString sURL = "vnd.sun.star.ServiceName:" + sFilterService;
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
                            XmlStyleFamily::SD_GRAPHICS_ID, sStyleName ) );
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
        xPropSet->setPropertyValue( "GraphicFilter", Any(OUString()) );

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
        catch (lang::IllegalArgumentException const&)
        {
            TOOLS_WARN_EXCEPTION("xmloff.text", "Cannot import part of the text - probably an image in the text frame?");
            return;
        }
    }

    // Make adding the shape to Z-Ordering dependent from if we are
    // inside an inside_deleted_section (redlining). That is necessary
    // since the shape will be removed again later. It would lead to
    // errors if it would stay inside the Z-Ordering. Thus, the
    // easiest way to solve that conflict is to not add it here.
    if(!GetImport().HasTextImport()
        || !GetImport().GetTextImport()->IsInsideDeleteContext())
    {
        Reference < XShape > xShape( xPropSet, UNO_QUERY );

        GetImport().GetShapeImport()->shapeWithZIndexAdded( xShape, nZIndex );
    }

    if( XML_TEXT_FRAME_TEXTBOX != nType )
        return;

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

void XMLTextFrameContext::removeGraphicFromImportContext(const SvXMLImportContext& rContext)
{
    const XMLTextFrameContext_Impl* pXMLTextFrameContext_Impl = dynamic_cast< const XMLTextFrameContext_Impl* >(&rContext);

    if(!pXMLTextFrameContext_Impl)
        return;

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
        sal_Int32 /*nElement*/,
        const Reference< XFastAttributeList > & rAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nNewType,
        const Reference< XFastAttributeList > & rFrameAttrList,
        bool bMultipleContent )
:   SvXMLImportContext( rImport )
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

    auto processAttr = [&](sal_Int32 nElement, const sax_fastparser::FastAttributeList::FastAttributeIter& aIter) -> void
    {
        switch( nElement )
        {
        case XML_ELEMENT(DRAW, XML_STYLE_NAME):
            sStyleName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_NAME):
            m_sOrigName = aIter.toString();
            sName = m_sOrigName;
            break;
        case XML_ELEMENT(DRAW, XML_FRAME_NAME):
            sFrameName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_APPLET_NAME):
            sAppletName = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_ANCHOR_TYPE):
            if( TextContentAnchorType_AT_PARAGRAPH == eAnchorType ||
                TextContentAnchorType_AT_CHARACTER == eAnchorType ||
                TextContentAnchorType_AS_CHARACTER == eAnchorType )
            {

                TextContentAnchorType eNew;
                if( XMLAnchorTypePropHdl::convert( aIter.toView(), eNew ) &&
                    ( TextContentAnchorType_AT_PARAGRAPH == eNew ||
                      TextContentAnchorType_AT_CHARACTER == eNew ||
                      TextContentAnchorType_AS_CHARACTER == eNew ||
                      TextContentAnchorType_AT_PAGE == eNew) )
                    eAnchorType = eNew;
            }
            break;
        case XML_ELEMENT(TEXT, XML_ANCHOR_PAGE_NUMBER):
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(nTmp, aIter.toView(), 1, SHRT_MAX))
                    nPage = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_ELEMENT(SVG, XML_X):
        case XML_ELEMENT(SVG_COMPAT, XML_X):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nX, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_Y):
        case XML_ELEMENT(SVG_COMPAT, XML_Y):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    nY, aIter.toView() );
            break;
        case XML_ELEMENT(SVG, XML_WIDTH):
        case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            // relative widths are obsolete since SRC617. Remove them some day!
            if( aIter.toView().find( '%' ) != std::string_view::npos )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, aIter.toView() );
                nRelWidth = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nWidth, aIter.toView(), 0 );
            }
            break;
        case XML_ELEMENT(STYLE, XML_REL_WIDTH):
            if( IsXMLToken(aIter, XML_SCALE) )
            {
                bSyncWidth = true;
            }
            else
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertPercent( nTmp, aIter.toView() ))
                    nRelWidth = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_ELEMENT(FO, XML_MIN_WIDTH):
        case XML_ELEMENT(FO_COMPAT, XML_MIN_WIDTH):
            if( aIter.toView().find( '%' ) != std::string_view::npos )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, aIter.toView() );
                nRelWidth = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nWidth, aIter.toView(), 0 );
            }
            bMinWidth = true;
            break;
        case XML_ELEMENT(SVG, XML_HEIGHT):
        case XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
            // relative heights are obsolete since SRC617. Remove them some day!
            if( aIter.toView().find( '%' ) != std::string_view::npos )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, aIter.toView() );
                nRelHeight = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nHeight, aIter.toView(), 0 );
            }
            break;
        case XML_ELEMENT(STYLE, XML_REL_HEIGHT):
            if( IsXMLToken( aIter, XML_SCALE ) )
            {
                bSyncHeight = true;
            }
            else if( IsXMLToken( aIter, XML_SCALE_MIN ) )
            {
                bSyncHeight = true;
                bMinHeight = true;
            }
            else
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertPercent( nTmp, aIter.toView() ))
                    nRelHeight = static_cast<sal_Int16>(nTmp);
            }
            break;
        case XML_ELEMENT(FO, XML_MIN_HEIGHT):
        case XML_ELEMENT(FO_COMPAT, XML_MIN_HEIGHT):
            if( aIter.toView().find( '%' ) != std::string_view::npos )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, aIter.toView() );
                nRelHeight = static_cast<sal_Int16>(nTmp);
            }
            else
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        nHeight, aIter.toView(), 0 );
            }
            bMinHeight = true;
            break;
        case XML_ELEMENT(DRAW, XML_ZINDEX):
            ::sax::Converter::convertNumber( nZIndex, aIter.toView(), -1 );
            break;
        case XML_ELEMENT(DRAW, XML_CHAIN_NEXT_NAME):
            sNextName = aIter.toString();
            break;
        case XML_ELEMENT(XLINK, XML_HREF):
            sHRef = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_TRANSFORM):
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
                aSdXMLImExTransform2D.SetString(aIter.toString(), GetImport().GetMM100UnitConverter());
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
        case XML_ELEMENT(DRAW, XML_CODE):
            sCode = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_OBJECT):
            break;
        case XML_ELEMENT(DRAW, XML_ARCHIVE):
            break;
        case XML_ELEMENT(DRAW, XML_MAY_SCRIPT):
            bMayScript = IsXMLToken( aIter, XML_TRUE );
            break;
        case XML_ELEMENT(DRAW, XML_MIME_TYPE):
        case XML_ELEMENT(LO_EXT, XML_MIME_TYPE):
            sMimeType = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES):
        case XML_ELEMENT(DRAW, XML_NOTIFY_ON_UPDATE_OF_TABLE):
            sTblName = aIter.toString();
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    };

    for( auto& aIter : sax_fastparser::castToFastAttributeList(rAttrList) )
            processAttr(aIter.getToken(), aIter);
    for( auto& aIter : sax_fastparser::castToFastAttributeList(rFrameAttrList) )
            processAttr(aIter.getToken(), aIter);

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

void XMLTextFrameContext_Impl::endFastElement(sal_Int32 )
{
    if( ( XML_TEXT_FRAME_OBJECT_OLE == nType ||
          XML_TEXT_FRAME_GRAPHIC == nType) &&
        !xPropSet.is() && !bCreateFailed )
    {
        OUString sTrimmedChars = maUrlBuffer.makeStringAndClear().trim();
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
                    sChars = sBase64CharsLeft + sTrimmedChars;
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

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextFrameContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(DRAW, XML_PARAM) )
    {
        if ( nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN )
            return new XMLTextFrameParam_Impl( GetImport(),
                                               xAttrList, aParamMap );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_BINARY_DATA) )
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
                return new XMLBase64ImportContext( GetImport(), xBase64Stream );
        }
    }
    // Correction of condition which also avoids warnings. (#i100480#)
    if( XML_TEXT_FRAME_OBJECT == nType &&
        ( nElement == XML_ELEMENT(OFFICE, XML_DOCUMENT) ||
          nElement == XML_ELEMENT(MATH, XML_MATH) ) )
    {
        if( !xPropSet.is() && !bCreateFailed )
        {
            XMLEmbeddedObjectImportContext *pEContext =
                new XMLEmbeddedObjectImportContext( GetImport(), nElement, xAttrList );
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
            return pEContext;
        }
    }

    if( xOldTextCursor.is() )  // text-box
    {
        auto p = GetImport().GetTextImport()->CreateTextChildContext(
                            GetImport(), nElement, xAttrList,
                            XMLTextType::TextBox );
        if (p)
            return p;
    }

    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

void XMLTextFrameContext_Impl::characters( const OUString& rChars )
{
    maUrlBuffer.append(rChars);
}

void XMLTextFrameContext_Impl::SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap )
{
    static const OUStringLiteral s_HyperLinkURL = u"HyperLinkURL";
    static const OUStringLiteral s_HyperLinkName = u"HyperLinkName";
    static const OUStringLiteral s_HyperLinkTarget = u"HyperLinkTarget";
    static const OUStringLiteral s_ServerMap = u"ServerMap";
    if( !xPropSet.is() )
        return;

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
    if (m_sOrigName.isEmpty() || !xNamed.is())
        return;

    OUString const name(xNamed->getName());
    if (name != m_sOrigName)
    {
        try
        {
            xNamed->setName(m_sOrigName);
        }
        catch (uno::Exception const&)
        {   // fdo#71698 document contains 2 frames with same draw:name
            TOOLS_INFO_EXCEPTION("xmloff.text", "SetName(): exception setting \""
                    << m_sOrigName << "\"");
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
        const Reference< XFastAttributeList > & xAttrList,
        TextContentAnchorType eATyp )
:   SvXMLImportContext( rImport )
,   MultiImageImportHelper()
,   m_xAttrList( new sax_fastparser::FastAttributeList( xAttrList ) )
    // Implement Title/Description Elements UI (#i73249#)
,   m_sTitle()
,   m_sDesc()
,   m_eDefaultAnchorType( eATyp )
    // Shapes in Writer cannot be named via context menu (#i51726#)
,   m_HasAutomaticStyleWithoutParentStyle( false )
,   m_bSupportsReplacement( false )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        // New distinguish attribute between Writer objects and Draw objects is:
        // Draw objects have an automatic style without a parent style (#i51726#)
        switch (aIter.getToken())
        {
            case XML_ELEMENT(DRAW, XML_STYLE_NAME):
            {
                OUString aStyleName = aIter.toString();
                if( !aStyleName.isEmpty() )
                {
                    rtl::Reference < XMLTextImportHelper > xTxtImport =
                                                        GetImport().GetTextImport();
                    XMLPropStyleContext* pStyle = xTxtImport->FindAutoFrameStyle( aStyleName );
                    if ( pStyle && pStyle->GetParentName().isEmpty() )
                    {
                        m_HasAutomaticStyleWithoutParentStyle = true;
                    }
                }
                break;
            }
            case XML_ELEMENT(TEXT, XML_ANCHOR_TYPE):
            {
                TextContentAnchorType eNew;
                if( XMLAnchorTypePropHdl::convert( aIter.toView(), eNew ) &&
                    ( TextContentAnchorType_AT_PARAGRAPH == eNew ||
                      TextContentAnchorType_AT_CHARACTER == eNew ||
                      TextContentAnchorType_AS_CHARACTER == eNew ||
                      TextContentAnchorType_AT_PAGE == eNew) )
                    m_eDefaultAnchorType = eNew;
                break;
            }
        }
    }
}

void XMLTextFrameContext::endFastElement(sal_Int32 )
{
    /// solve if multiple image child contexts were imported
    SvXMLImportContextRef const pMultiContext(solveMultipleImages());

    SvXMLImportContext const*const pContext =
        (pMultiContext.is()) ? pMultiContext.get() : m_xImplContext.get();
    XMLTextFrameContext_Impl *pImpl = const_cast<XMLTextFrameContext_Impl*>(dynamic_cast< const XMLTextFrameContext_Impl*>( pContext ));
    assert(!pMultiContext.is() || pImpl);
    if( !pImpl )
        return;

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

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextFrameContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;

    if( !m_xImplContext.is() )
    {
        // no child exists
        if( IsTokenInNamespace(nElement, XML_NAMESPACE_DRAW) )
        {
            sal_uInt16 nFrameType = USHRT_MAX;
            switch (nElement & TOKEN_MASK)
            {
                case XML_TEXT_BOX:
                    nFrameType = XML_TEXT_FRAME_TEXTBOX;
                    break;
                case XML_IMAGE:
                    nFrameType = XML_TEXT_FRAME_GRAPHIC;
                    break;
                case XML_OBJECT:
                    nFrameType = XML_TEXT_FRAME_OBJECT;
                    break;
                case XML_OBJECT_OLE:
                    nFrameType = XML_TEXT_FRAME_OBJECT_OLE;
                    break;
                case XML_APPLET:
                    nFrameType = XML_TEXT_FRAME_APPLET;
                    break;
                case XML_PLUGIN:
                    nFrameType = XML_TEXT_FRAME_PLUGIN;
                    break;
                case XML_FLOATING_FRAME:
                    nFrameType = XML_TEXT_FRAME_FLOATING_FRAME;
                    break;
            }

            if( USHRT_MAX != nFrameType )
            {
                // Shapes in Writer cannot be named via context menu (#i51726#)
                if ( ( XML_TEXT_FRAME_TEXTBOX == nFrameType ||
                       XML_TEXT_FRAME_GRAPHIC == nFrameType ) &&
                     m_HasAutomaticStyleWithoutParentStyle )
                {
                    Reference < XShapes > xShapes;
                    xContext = XMLShapeImportHelper::CreateFrameChildContext(
                                    GetImport(), nElement, xAttrList, xShapes, m_xAttrList.get() );
                }
                else if( XML_TEXT_FRAME_PLUGIN == nFrameType )
                {
                    bool bMedia = false;

                    // check, if we have a media object
                    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
                    {
                        if( aIter.getToken() == XML_ELEMENT(DRAW, XML_MIME_TYPE) )
                        {
                            if( aIter.toString() == "application/vnd.sun.star.media" )
                                bMedia = true;

                            // leave this loop
                            break;
                        }
                    }

                    if( bMedia )
                    {
                        Reference < XShapes > xShapes;
                        xContext = XMLShapeImportHelper::CreateFrameChildContext(
                                        GetImport(), nElement, xAttrList, xShapes, m_xAttrList.get() );
                    }
                }
                else if( XML_TEXT_FRAME_OBJECT == nFrameType ||
                         XML_TEXT_FRAME_OBJECT_OLE == nFrameType )
                {
                    m_bSupportsReplacement = true;
                }
                else if(XML_TEXT_FRAME_GRAPHIC == nFrameType)
                {
                    setSupportsMultipleContents( (nElement & TOKEN_MASK) == XML_IMAGE );
                }

                if (!xContext)
                {
                    xContext = new XMLTextFrameContext_Impl( GetImport(), nElement,
                                                        xAttrList,
                                                        m_eDefaultAnchorType,
                                                        nFrameType,
                                                        m_xAttrList.get() );
                }

                m_xImplContext = xContext;

                if(getSupportsMultipleContents() && XML_TEXT_FRAME_GRAPHIC == nFrameType)
                {
                    addContent(*m_xImplContext);
                }
            }
        }
    }
    else if(getSupportsMultipleContents() && nElement == XML_ELEMENT(DRAW, XML_IMAGE))
    {
        // read another image
        xContext = new XMLTextFrameContext_Impl(
            GetImport(), nElement, xAttrList,
            m_eDefaultAnchorType, XML_TEXT_FRAME_GRAPHIC, m_xAttrList.get(), true);

        m_xImplContext = xContext;
        addContent(*m_xImplContext);
    }
    else if( m_bSupportsReplacement && !m_xReplImplContext.is() &&
             nElement == XML_ELEMENT(DRAW, XML_IMAGE) )
    {
        // read replacement image
        Reference < XPropertySet > xPropSet;
        if( CreateIfNotThere( xPropSet ) )
        {
            xContext = new XMLReplacementImageContext( GetImport(),
                                nElement, xAttrList, xPropSet );
            m_xReplImplContext = xContext;
        }
    }
    else if( nullptr != dynamic_cast< const XMLTextFrameContext_Impl*>( m_xImplContext.get() ))
    {
        // the child is a writer frame
        if( IsTokenInNamespace(nElement, XML_NAMESPACE_SVG) ||
            IsTokenInNamespace(nElement, XML_NAMESPACE_SVG_COMPAT) )
        {
            // Implement Title/Description Elements UI (#i73249#)
            const bool bOld = SvXMLImport::OOo_2x >= GetImport().getGeneratorVersion();
            if ( bOld )
            {
                if ( (nElement & TOKEN_MASK) == XML_DESC )
                {
                    xContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        m_sTitle );
                }
            }
            else
            {
                if( (nElement & TOKEN_MASK) == XML_TITLE )
                {
                    if (getSupportsMultipleContents())
                    {   // tdf#103567 ensure props are set on surviving shape
                        m_xImplContext = solveMultipleImages();
                    }
                    xContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        m_sTitle );
                }
                else if ( (nElement & TOKEN_MASK) == XML_DESC )
                {
                    if (getSupportsMultipleContents())
                    {   // tdf#103567 ensure props are set on surviving shape
                        m_xImplContext = solveMultipleImages();
                    }
                    xContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        m_sDesc );
                }
            }
        }
        else if( IsTokenInNamespace(nElement, XML_NAMESPACE_DRAW) )
        {
            Reference < XPropertySet > xPropSet;
            if( (nElement & TOKEN_MASK) == XML_CONTOUR_POLYGON )
            {
                if (getSupportsMultipleContents())
                {   // tdf#103567 ensure props are set on surviving shape
                    m_xImplContext = solveMultipleImages();
                }
                if( CreateIfNotThere( xPropSet ) )
                    xContext = new XMLTextFrameContourContext_Impl( GetImport(), nElement,
                                                  xAttrList, xPropSet, false );
            }
            else if( (nElement & TOKEN_MASK) == XML_CONTOUR_PATH )
            {
                if (getSupportsMultipleContents())
                {   // tdf#103567 ensure props are set on surviving shape
                    m_xImplContext = solveMultipleImages();
                }
                if( CreateIfNotThere( xPropSet ) )
                    xContext = new XMLTextFrameContourContext_Impl( GetImport(), nElement,
                                                  xAttrList, xPropSet, true );
            }
            else if( (nElement & TOKEN_MASK) == XML_IMAGE_MAP )
            {
                if (getSupportsMultipleContents())
                {   // tdf#103567 ensure props are set on surviving shape
                    m_xImplContext = solveMultipleImages();
                }
                if( CreateIfNotThere( xPropSet ) )
                    xContext = new XMLImageMapContext( GetImport(), xPropSet );
            }
        }
        else if( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
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
                    xContext = new XMLEventsImportContext(GetImport(), xEventsSupplier);
                }
            }
        }
    }
    // #i68101#
    else if( nElement == XML_ELEMENT(SVG, XML_TITLE) || nElement == XML_ELEMENT(SVG, XML_DESC ) ||
             nElement == XML_ELEMENT(SVG_COMPAT, XML_TITLE) || nElement == XML_ELEMENT(SVG_COMPAT, XML_DESC ) )
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            m_xImplContext = solveMultipleImages();
        }
        xContext = &dynamic_cast<SvXMLImportContext&>(*m_xImplContext->createFastChildContext( nElement, xAttrList ));
    }
    else if (nElement == XML_ELEMENT(LO_EXT, XML_SIGNATURELINE))
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            m_xImplContext = solveMultipleImages();
        }
        xContext = &dynamic_cast<SvXMLImportContext&>(*m_xImplContext->createFastChildContext(nElement, xAttrList));
    }
    else if (nElement == XML_ELEMENT(LO_EXT, XML_QRCODE))
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            m_xImplContext = solveMultipleImages();
        }
        xContext = &dynamic_cast<SvXMLImportContext&>(*m_xImplContext->createFastChildContext(nElement, xAttrList));
    }
    else
    {
        // the child is a drawing shape
        return XMLShapeImportHelper::CreateFrameChildContext(
                                    m_xImplContext.get(), nElement, xAttrList );
    }

    return xContext.get();
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
