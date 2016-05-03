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

#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/container/XNamed.hpp>
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
#include "XMLEmbeddedObjectImportContext.hxx"
#include <xmloff/XMLBase64ImportContext.hxx>
#include "XMLReplacementImageContext.hxx"
#include <xmloff/prstylei.hxx>
#include <xmloff/i18nmap.hxx>
#include "xexptran.hxx"
#include <xmloff/shapeimport.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include "XMLImageMapContext.hxx"
#include "XMLTextFrameContext.hxx"
#include "XMLTextListBlockContext.hxx"
#include "XMLTextListItemContext.hxx"
#include <xmloff/attrlist.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
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

// Implement Title/Description Elements UI (#i73249#)
class XMLTextFrameTitleOrDescContext_Impl : public SvXMLImportContext
{
    OUString&   mrTitleOrDesc;

public:


    XMLTextFrameTitleOrDescContext_Impl( SvXMLImport& rImport,
                                         sal_uInt16 nPrfx,
                                         const OUString& rLName,
                                         OUString& rTitleOrDesc );
    virtual ~XMLTextFrameTitleOrDescContext_Impl();

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

XMLTextFrameTitleOrDescContext_Impl::~XMLTextFrameTitleOrDescContext_Impl()
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
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_uInt16 nType,
            ParamMap &rParamMap);
    virtual ~XMLTextFrameParam_Impl();
};


XMLTextFrameParam_Impl::~XMLTextFrameParam_Impl()
{
}

XMLTextFrameParam_Impl::XMLTextFrameParam_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
          const OUString& rLName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
        sal_uInt16 /*nType*/,
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
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const Reference < XPropertySet >& rPropSet,
            bool bPath );
    virtual ~XMLTextFrameContourContext_Impl();
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
            basegfx::tools::importFromSvgD(aPolyPolygon, sD, GetImport().needFixPositionAfterZ(), nullptr);
        }
        else
        {
            basegfx::B2DPolygon aPolygon;

            if(basegfx::tools::importFromSvgPoints(aPolygon, sPoints))
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
                    basegfx::tools::createSourceRangeTargetRangeTransform(
                        aSourceRange,
                        aTargetRange));
            }

            com::sun::star::drawing::PointSequenceSequence aPointSequenceSequence;
            basegfx::tools::B2DPolyPolygonToUnoPointSequenceSequence(aPolyPolygon, aPointSequenceSequence);
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

XMLTextFrameContourContext_Impl::~XMLTextFrameContourContext_Impl()
{
}

class XMLTextFrameContext_Impl : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > xPropSet;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::io::XOutputStream > xBase64Stream;

    /// old list item and block (#89891#)
    bool mbListContextPushed;

    const OUString sWidth;
    const OUString sWidthType;
    const OUString sRelativeWidth;
    const OUString sHeight;
    const OUString sRelativeHeight;
    const OUString sSizeType;
    const OUString sIsSyncWidthToHeight;
    const OUString sIsSyncHeightToWidth;
    const OUString sHoriOrient;
    const OUString sHoriOrientPosition;
    const OUString sVertOrient;
    const OUString sVertOrientPosition;
    const OUString sAnchorType;
    const OUString sAnchorPageNo;
    const OUString sGraphicURL;
    const OUString sGraphicFilter;
    const OUString sTitle;
    const OUString sDescription;
    const OUString sFrameStyleName;
    const OUString sGraphicRotation;
    const OUString sTextBoxServiceName;
    const OUString sGraphicServiceName;

    OUString m_sOrigName;
    OUString sName;
    OUString sStyleName;
    OUString sNextName;
    OUString sHRef;
    OUString sFilterName;
    OUString sCode;
    OUString sObject;
    OUString sArchive;
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

    sal_uInt16 nType;
    ::com::sun::star::text::TextContentAnchorType   eAnchorType;

    bool    bMayScript : 1;
    bool    bMinWidth : 1;
    bool    bMinHeight : 1;
    bool    bSyncWidth : 1;
    bool    bSyncHeight : 1;
    bool    bCreateFailed : 1;
    bool    bOwnBase64Stream : 1;

    void Create( bool bHRefOrBase64 );

public:


    bool CreateIfNotThere();
    const OUString& GetHRef() const { return sHRef; }

    XMLTextFrameContext_Impl( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rAttrList,
            ::com::sun::star::text::TextContentAnchorType eAnchorType,
            sal_uInt16 nType,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rFrameAttrList );
    virtual ~XMLTextFrameContext_Impl();

    virtual void EndElement() override;

    virtual void Characters( const OUString& rChars ) override;

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

    void SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap );

    // Implement Title/Description Elements UI (#i73249#)
    void SetTitle( const OUString& rTitle );

    void SetDesc( const OUString& rDesc );

    void SetName();

    ::com::sun::star::text::TextContentAnchorType GetAnchorType() const { return eAnchorType; }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet >& GetPropSet() const { return xPropSet; }
};


void XMLTextFrameContext_Impl::Create( bool /*bHRefOrBase64*/ )
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
                    case XML_TEXT_FRAME_TEXTBOX: sServiceName = sTextBoxServiceName; break;
                    case XML_TEXT_FRAME_GRAPHIC: sServiceName = sGraphicServiceName; break;
                }
                Reference<XInterface> xIfc = xFactory->createInstance( sServiceName );
                DBG_ASSERT( xIfc.is(), "couldn't create frame" );
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
                sName = sOldName;
                sName += OUString::number( ++i );
            }
            xNamed->setName( sName );
            if( sName != sOldName )
            {
                bool bSuccess = xTextImportHelper->GetRenameMap().Add( XML_TEXT_RENAME_TYPE_FRAME,
                                             sOldName, sName );

                if (!bSuccess && !sOldName.isEmpty())
                {
                    bCreateFailed = true;
                    return;
                }
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
            xPropSet->setPropertyValue( sFrameStyleName, Any(sDisplayStyleName) );
        }
    }

    // anchor type (must be set before any other properties, because
    // otherwise some orientations cannot be set or will be changed
    // afterwards)
    xPropSet->setPropertyValue( sAnchorType, Any(eAnchorType) );

    // hard properties
    if( pStyle )
        pStyle->FillPropertySet( xPropSet );

    // x and y
    sal_Int16 nHoriOrient =  HoriOrientation::NONE;
    aAny = xPropSet->getPropertyValue( sHoriOrient );
    aAny >>= nHoriOrient;
    if( HoriOrientation::NONE == nHoriOrient )
    {
        xPropSet->setPropertyValue( sHoriOrientPosition, Any(nX) );
    }

    sal_Int16 nVertOrient =  VertOrientation::NONE;
    aAny = xPropSet->getPropertyValue( sVertOrient );
    aAny >>= nVertOrient;
    if( VertOrientation::NONE == nVertOrient )
    {
        xPropSet->setPropertyValue( sVertOrientPosition, Any(nY) );
    }

    // width
    if( nWidth > 0 )
    {
        xPropSet->setPropertyValue( sWidth, Any(nWidth) );
    }
    if( nRelWidth > 0 || nWidth > 0 )
    {
        xPropSet->setPropertyValue( sRelativeWidth, Any(nRelWidth) );
    }
    if( bSyncWidth || nWidth > 0 )
    {
        xPropSet->setPropertyValue( sIsSyncWidthToHeight, Any(bSyncWidth) );
    }
    if( xPropSetInfo->hasPropertyByName( sWidthType ) &&
        (bMinWidth || nWidth > 0 || nRelWidth > 0 ) )
    {
        sal_Int16 nSizeType =
            (bMinWidth && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                           : SizeType::FIX;
        xPropSet->setPropertyValue( sWidthType, Any(nSizeType) );
    }

    if( nHeight > 0 )
    {
        xPropSet->setPropertyValue( sHeight, Any(nHeight) );
    }
    if( nRelHeight > 0 || nHeight > 0 )
    {
        xPropSet->setPropertyValue( sRelativeHeight, Any(nRelHeight) );
    }
    if( bSyncHeight || nHeight > 0 )
    {
        xPropSet->setPropertyValue( sIsSyncHeightToWidth, Any(bSyncHeight) );
    }
    if( xPropSetInfo->hasPropertyByName( sSizeType ) &&
        (bMinHeight || nHeight > 0 || nRelHeight > 0 ) )
    {
        sal_Int16 nSizeType =
            (bMinHeight && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN
                                                            : SizeType::FIX;
        xPropSet->setPropertyValue( sSizeType, Any(nSizeType) );
    }

    if( XML_TEXT_FRAME_GRAPHIC == nType )
    {
        // URL
        OSL_ENSURE( !sHRef.isEmpty() || xBase64Stream.is(),
                    "neither URL nor base64 image data given" );
        rtl::Reference < XMLTextImportHelper > xTxtImport =
            GetImport().GetTextImport();
        if( !sHRef.isEmpty() )
        {
            bool bForceLoad = xTxtImport->IsInsertMode() ||
                                  xTxtImport->IsBlockMode() ||
                                  xTxtImport->IsStylesOnlyMode() ||
                                  xTxtImport->IsOrganizerMode();
            sHRef = GetImport().ResolveGraphicObjectURL( sHRef, !bForceLoad );
        }
        else if( xBase64Stream.is() )
        {
            sHRef = GetImport().ResolveGraphicObjectURLFromBase64( xBase64Stream );
            xBase64Stream = nullptr;
        }
        xPropSet->setPropertyValue( sGraphicURL, Any(sHRef) );

        // filter name
        xPropSet->setPropertyValue( sGraphicFilter, Any(sFilterName) );

        // rotation
        xPropSet->setPropertyValue( sGraphicRotation, Any(nRotation) );
    }

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    if( TextContentAnchorType_AT_PAGE == eAnchorType && nPage > 0 )
    {
        xPropSet->setPropertyValue( sAnchorPageNo, Any(nPage) );
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
            SAL_WARN("xmloff.text", "Cannot import part of the text - probably an image in the text frame? " << e.Message);
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

void XMLTextFrameContext::removeGraphicFromImportContext(const SvXMLImportContext& rContext) const
{
    const XMLTextFrameContext_Impl* pXMLTextFrameContext_Impl = dynamic_cast< const XMLTextFrameContext_Impl* >(&rContext);

    if(pXMLTextFrameContext_Impl)
    {
        try
        {
            // just dispose to delete
            uno::Reference< lang::XComponent > xComp(pXMLTextFrameContext_Impl->GetPropSet(), UNO_QUERY);

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

OUString XMLTextFrameContext::getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const
{
    OUString aRetval;
    const XMLTextFrameContext_Impl* pXMLTextFrameContext_Impl = dynamic_cast< const XMLTextFrameContext_Impl* >(&rContext);

    if(pXMLTextFrameContext_Impl)
    {
        return pXMLTextFrameContext_Impl->GetHRef();
    }

    return aRetval;
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
        Create( true );
    }

    return xPropSet.is();
}

XMLTextFrameContext_Impl::XMLTextFrameContext_Impl(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & rAttrList,
        TextContentAnchorType eATyp,
        sal_uInt16 nNewType,
        const Reference< XAttributeList > & rFrameAttrList )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   mbListContextPushed( false )
,   sWidth("Width")
,   sWidthType("WidthType")
,   sRelativeWidth("RelativeWidth")
,   sHeight("Height")
,   sRelativeHeight("RelativeHeight")
,   sSizeType("SizeType")
,   sIsSyncWidthToHeight("IsSyncWidthToHeight")
,   sIsSyncHeightToWidth("IsSyncHeightToWidth")
,   sHoriOrient("HoriOrient")
,   sHoriOrientPosition("HoriOrientPosition")
,   sVertOrient("VertOrient")
,   sVertOrientPosition("VertOrientPosition")
,   sAnchorType("AnchorType")
,   sAnchorPageNo("AnchorPageNo")
,   sGraphicURL("GraphicURL")
,   sGraphicFilter("GraphicFilter")
,   sTitle("Title")
,   sDescription("Description")
,   sFrameStyleName("FrameStyleName")
,   sGraphicRotation("GraphicRotation")
,   sTextBoxServiceName("com.sun.star.text.TextFrame")
,   sGraphicServiceName("com.sun.star.text.GraphicObject")
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
                    nPage = (sal_Int16)nTmp;
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
                nRelWidth = (sal_Int16)nTmp;
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
                    nRelWidth = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_WIDTH:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, rValue );
                nRelWidth = (sal_Int16)nTmp;
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
                nRelHeight = (sal_Int16)nTmp;
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
                    nRelHeight = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_MIN_HEIGHT:
            if( rValue.indexOf( '%' ) != -1 )
            {
                sal_Int32 nTmp;
                ::sax::Converter::convertPercent( nTmp, rValue );
                nRelHeight = (sal_Int16)nTmp;
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
                OUString sValue( rValue );
                sValue = sValue.trim();
                const OUString aRotate(GetXMLToken(XML_ROTATE));
                const sal_Int32 nRotateLen(aRotate.getLength());
                sal_Int32 nLen = sValue.getLength();
                if( nLen >= nRotateLen+3 &&
                    0 == sValue.compareTo( aRotate, nRotateLen ) &&
                    '(' == sValue[nRotateLen] &&
                    ')' == sValue[nLen-1] )
                {
                    sValue = sValue.copy( nRotateLen+1, nLen-(nRotateLen+2) );
                    sValue = sValue.trim();
                    sal_Int32 nVal;
                    if (::sax::Converter::convertNumber( nVal, sValue ))
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
          sHRef.isEmpty() ) ||
        ( XML_TEXT_FRAME_APPLET  == nType && sCode.isEmpty() ) ||
        ( XML_TEXT_FRAME_PLUGIN == nType &&
          sHRef.isEmpty() && sMimeType.isEmpty() ) )
        return; // no URL: no image or OLE object

    Create( true );
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
    if (mbListContextPushed) {
        GetImport().GetTextImport()->PopListContext();
    }

    if (( nType == XML_TEXT_FRAME_APPLET || nType == XML_TEXT_FRAME_PLUGIN ) && xPropSet.is())
        GetImport().GetTextImport()->endAppletOrPlugin( xPropSet, aParamMap);
}

SvXMLImportContext *XMLTextFrameContext_Impl::CreateChildContext(
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
                Create( false );
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
                    ::sax::Converter::decodeBase64SomeChars( aBuffer, sChars );
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
                        << m_sOrigName << "\": " << e.Message);
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
        if( xPropSetInfo->hasPropertyByName( sTitle ) )
        {
            xPropSet->setPropertyValue( sTitle, makeAny( rTitle ) );
        }
    }
}

void XMLTextFrameContext_Impl::SetDesc( const OUString& rDesc )
{
    if ( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sDescription ) )
        {
            xPropSet->setPropertyValue( sDescription, makeAny( rDesc ) );
        }
    }
}


bool XMLTextFrameContext::CreateIfNotThere( ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet >& rPropSet )
{
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = dynamic_cast< XMLTextFrameContext_Impl*>( pContext );
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
        TextContentAnchorType eATyp )
:   SvXMLImportContext( rImport, nPrfx, rLName )
,   MultiImageImportHelper()
,   m_xAttrList( new SvXMLAttributeList( xAttrList ) )
,   m_pHyperlink( nullptr )
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

XMLTextFrameContext::~XMLTextFrameContext()
{
    delete m_pHyperlink;
}

void XMLTextFrameContext::EndElement()
{
    /// solve if multiple image child contexts were imported
    SvXMLImportContextRef const pMultiContext(solveMultipleImages());

    SvXMLImportContext const*const pContext =
        (pMultiContext) ? &pMultiContext : &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = const_cast<XMLTextFrameContext_Impl*>(dynamic_cast< const XMLTextFrameContext_Impl*>( pContext ));
    assert(!pMultiContext || pImpl);
    if( pImpl )
    {
        pImpl->CreateIfNotThere();

        // fdo#68839: in case the surviving image was not the first one,
        // it will have a counter added to its name - set the original name
        if (pMultiContext) // do this only when necessary; esp. not for text
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
            delete m_pHyperlink;
            m_pHyperlink = nullptr;
        }

    }
}

SvXMLImportContext *XMLTextFrameContext::CreateChildContext(
        sal_uInt16 p_nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if( !m_xImplContext.Is() )
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
                    pContext = GetImport().GetShapeImport()->CreateFrameChildContext(
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
                        pContext = GetImport().GetShapeImport()->CreateFrameChildContext(
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

                if( !pContext )
                {

                    pContext = new XMLTextFrameContext_Impl( GetImport(), p_nPrefix,
                                                        rLocalName, xAttrList,
                                                        m_eDefaultAnchorType,
                                                        nFrameType,
                                                        m_xAttrList );
                }

                m_xImplContext = pContext;

                if(getSupportsMultipleContents() && XML_TEXT_FRAME_GRAPHIC == nFrameType)
                {
                    addContent(*m_xImplContext);
                }
            }
        }
    }
    else if(getSupportsMultipleContents() && XML_NAMESPACE_DRAW == p_nPrefix && IsXMLToken(rLocalName, XML_IMAGE))
    {
        // read another image
        pContext = new XMLTextFrameContext_Impl(
            GetImport(), p_nPrefix, rLocalName, xAttrList,
            m_eDefaultAnchorType, XML_TEXT_FRAME_GRAPHIC, m_xAttrList);

        m_xImplContext = pContext;
        addContent(*m_xImplContext);
    }
    else if( m_bSupportsReplacement && !m_xReplImplContext &&
             XML_NAMESPACE_DRAW == p_nPrefix &&
             IsXMLToken( rLocalName, XML_IMAGE ) )
    {
        // read replacement image
        Reference < XPropertySet > xPropSet;
        if( CreateIfNotThere( xPropSet ) )
        {
            pContext = new XMLReplacementImageContext( GetImport(),
                                p_nPrefix, rLocalName, xAttrList, xPropSet );
            m_xReplImplContext = pContext;
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
                    pContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        p_nPrefix,
                                                                        rLocalName,
                                                                        m_sTitle );
                }
            }
            else
            {
                if( IsXMLToken( rLocalName, XML_TITLE ) )
                {
                    pContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
                                                                        p_nPrefix,
                                                                        rLocalName,
                                                                        m_sTitle );
                }
                else if ( IsXMLToken( rLocalName, XML_DESC ) )
                {
                    pContext = new XMLTextFrameTitleOrDescContext_Impl( GetImport(),
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
                if( CreateIfNotThere( xPropSet ) )
                    pContext = new XMLTextFrameContourContext_Impl( GetImport(), p_nPrefix, rLocalName,
                                                  xAttrList, xPropSet, false );
            }
            else if( IsXMLToken( rLocalName, XML_CONTOUR_PATH ) )
            {
                if( CreateIfNotThere( xPropSet ) )
                    pContext = new XMLTextFrameContourContext_Impl( GetImport(), p_nPrefix, rLocalName,
                                                  xAttrList, xPropSet, true );
            }
            else if( IsXMLToken( rLocalName, XML_IMAGE_MAP ) )
            {
                if( CreateIfNotThere( xPropSet ) )
                    pContext = new XMLImageMapContext( GetImport(), p_nPrefix, rLocalName, xPropSet );
            }
        }
        else if( (XML_NAMESPACE_OFFICE == p_nPrefix) && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
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
                    pContext = new XMLEventsImportContext(GetImport(), p_nPrefix,
                                                      rLocalName, xEventsSupplier);
                }
            }
        }
    }
    else if( p_nPrefix == XML_NAMESPACE_SVG &&  // #i68101#
                (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) )
    {
        pContext = m_xImplContext->CreateChildContext( p_nPrefix, rLocalName, xAttrList );
    }
    else
    {
        // the child is a drawing shape
        pContext = XMLShapeImportHelper::CreateFrameChildContext(
                                    &m_xImplContext, p_nPrefix, rLocalName, xAttrList );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );

    return pContext;
}

void XMLTextFrameContext::SetHyperlink( const OUString& rHRef,
                       const OUString& rName,
                       const OUString& rTargetFrameName,
                       bool bMap )
{
    OSL_ENSURE( !m_pHyperlink, "recursive SetHyperlink call" );
    delete m_pHyperlink;
    m_pHyperlink = new XMLTextFrameContextHyperlink_Impl(
                rHRef, rName, rTargetFrameName, bMap );
}

TextContentAnchorType XMLTextFrameContext::GetAnchorType() const
{
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = dynamic_cast< XMLTextFrameContext_Impl*>( pContext );
    if( pImpl )
        return pImpl->GetAnchorType();
    else
        return m_eDefaultAnchorType;
}

Reference < XTextContent > XMLTextFrameContext::GetTextContent() const
{
    Reference < XTextContent > xTxtCntnt;
    SvXMLImportContext *pContext = &m_xImplContext;
    XMLTextFrameContext_Impl *pImpl = dynamic_cast< XMLTextFrameContext_Impl* >( pContext );
    if( pImpl )
        xTxtCntnt.set( pImpl->GetPropSet(), UNO_QUERY );

    return xTxtCntnt;
}

Reference < XShape > XMLTextFrameContext::GetShape() const
{
    Reference < XShape > xShape;
    SvXMLImportContext* pContext = &m_xImplContext;
    SvXMLShapeContext* pImpl = dynamic_cast<SvXMLShapeContext*>( pContext  );
    if ( pImpl )
    {
        xShape = pImpl->getShape();
    }

    return xShape;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
