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

#include <string.h>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/string.hxx>

#include <oox/drawingml/drawingmltypes.hxx>

#include <dmapper/DomainMapper.hxx>
#include <ooxml/resourceids.hxx>
#include <resourcemodel/ResourceModelHelper.hxx>

#include "ConversionHelper.hxx"
#include "GraphicHelpers.hxx"
#include "GraphicImport.hxx"
#include "PropertyMap.hxx"
#include "WrapPolygonHandler.hxx"
#include "dmapperLoggers.hxx"

namespace writerfilter {

using resourcemodel::resolveSprmProps;

namespace dmapper
{
using namespace std;
using namespace css;

class XInputStreamHelper : public cppu::WeakImplHelper1<io::XInputStream>
{
    const sal_uInt8* m_pBuffer;
    const sal_Int32  m_nLength;
    sal_Int32        m_nPosition;
    bool             m_bBmp;

    const sal_uInt8* m_pBMPHeader; //default BMP-header
    sal_Int32        m_nHeaderLength;
public:
    XInputStreamHelper(const sal_uInt8* buf, size_t len, bool bBmp);
    ~XInputStreamHelper();

    virtual ::sal_Int32 SAL_CALL readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception);
    virtual void SAL_CALL closeInput(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception);
};

XInputStreamHelper::XInputStreamHelper(const sal_uInt8* buf, size_t len, bool bBmp) :
        m_pBuffer( buf ),
        m_nLength( len ),
        m_nPosition( 0 ),
        m_bBmp( bBmp )
{
    static const sal_uInt8 aHeader[] =
        {0x42, 0x4d, 0xe6, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
    m_pBMPHeader = aHeader;
    m_nHeaderLength = m_bBmp ? sizeof( aHeader ) / sizeof(sal_uInt8) : 0;
}


XInputStreamHelper::~XInputStreamHelper()
{
}

sal_Int32 XInputStreamHelper::readBytes( uno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception)
{
    return readSomeBytes( aData, nBytesToRead );
}

sal_Int32 XInputStreamHelper::readSomeBytes( uno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
        throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception)
{
    sal_Int32 nRet = 0;
    if( nMaxBytesToRead > 0 )
    {
        if( nMaxBytesToRead > (m_nLength + m_nHeaderLength) - m_nPosition )
            nRet = (m_nLength + m_nHeaderLength) - m_nPosition;
        else
            nRet = nMaxBytesToRead;
        aData.realloc( nRet );
        sal_Int8* pData = aData.getArray();
        sal_Int32 nHeaderRead = 0;
        if( m_nPosition < m_nHeaderLength)
        {
            //copy header content first
            nHeaderRead = m_nHeaderLength - m_nPosition;
            memcpy( pData, m_pBMPHeader + (m_nPosition ), nHeaderRead );
            nRet -= nHeaderRead;
            m_nPosition += nHeaderRead;
        }
        if( nRet )
        {
            memcpy( pData + nHeaderRead, m_pBuffer + (m_nPosition - m_nHeaderLength), nRet );
            m_nPosition += nRet;
        }
    }
    return nRet;
}


void XInputStreamHelper::skipBytes( sal_Int32 nBytesToSkip ) throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception)
{
    if( nBytesToSkip < 0 || m_nPosition + nBytesToSkip > (m_nLength + m_nHeaderLength))
        throw io::BufferSizeExceededException();
    m_nPosition += nBytesToSkip;
}


sal_Int32 XInputStreamHelper::available(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception)
{
    return ( m_nLength + m_nHeaderLength ) - m_nPosition;
}


void XInputStreamHelper::closeInput(  ) throw (io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception)
{
}


struct GraphicBorderLine
{
    sal_Int32   nLineWidth;
    sal_Int32   nLineColor;
    sal_Int32   nLineDistance;
    bool        bHasShadow;

    GraphicBorderLine() :
        nLineWidth(0)
        ,nLineColor(0)
        ,nLineDistance(0)
        ,bHasShadow(false)
        {}

};

class GraphicImport_Impl
{
private:
    sal_Int32 nXSize;
    bool      bXSizeValid;
    sal_Int32 nYSize;
    bool      bYSizeValid;

public:
    GraphicImportType eGraphicImportType;
    DomainMapper&   rDomainMapper;

    sal_Int32 nHoriScaling;
    sal_Int32 nVertScaling;
    sal_Int32 nLeftPosition;
    sal_Int32 nTopPosition;
    sal_Int32 nRightPosition;
    sal_Int32 nBottomPosition;
    sal_Int32 nLeftCrop;
    sal_Int32 nTopCrop;
    sal_Int32 nRightCrop;
    sal_Int32 nBottomCrop;

    bool      bUseSimplePos;
    sal_Int32 zOrder;

    sal_Int16 nHoriOrient;
    sal_Int16 nHoriRelation;
    bool      bPageToggle;
    sal_Int16 nVertOrient;
    sal_Int16 nVertRelation;
    sal_Int32 nWrap;
    bool      bOpaque;
    bool      bContour;
    bool      bContourOutside;
    WrapPolygon::Pointer_t mpWrapPolygon;
    bool      bIgnoreWRK;

    sal_Int32 nLeftMargin;
    sal_Int32 nRightMargin;
    sal_Int32 nTopMargin;
    sal_Int32 nBottomMargin;

    bool bShadow;
    sal_Int32 nShadowXDistance;
    sal_Int32 nShadowYDistance;
    sal_Int32 nShadowColor;
    sal_Int32 nShadowTransparence;

    sal_Int32 nContrast;
    sal_Int32 nBrightness;
    double    fGamma;

    sal_Int32 nFillColor;

    drawing::ColorMode eColorMode;

    GraphicBorderLine   aBorders[4];
    sal_Int32           nCurrentBorderLine;

    sal_Int32       nDffType;
    bool            bIsGraphic;
    bool            bIsBitmap;
    bool            bIsTiff;
    sal_Int32       nBitsPerPixel;

    bool            bHoriFlip;
    bool            bVertFlip;

    bool            bSizeProtected;
    bool            bPositionProtected;

    sal_Int32       nShapeOptionType;

    OUString sName;
    OUString sAlternativeText;
    OUString title;
    std::queue<OUString>& m_rPositivePercentages;
    OUString sAnchorId;

    GraphicImport_Impl(GraphicImportType eImportType, DomainMapper&   rDMapper, std::queue<OUString>& rPositivePercentages) :
        nXSize(0)
        ,bXSizeValid(false)
        ,nYSize(0)
        ,bYSizeValid(false)
        ,eGraphicImportType( eImportType )
        ,rDomainMapper( rDMapper )
        ,nHoriScaling(0)
        ,nVertScaling(0)
        ,nLeftPosition(0)
        ,nTopPosition(0)
        ,nRightPosition(0)
        ,nBottomPosition(0)
        ,nLeftCrop(0)
        ,nTopCrop (0)
        ,nRightCrop (0)
        ,nBottomCrop(0)
        ,bUseSimplePos(false)
        ,zOrder(-1)
        ,nHoriOrient(   text::HoriOrientation::NONE )
        ,nHoriRelation( text::RelOrientation::FRAME )
        ,bPageToggle( false )
        ,nVertOrient(  text::VertOrientation::NONE )
        ,nVertRelation( text::RelOrientation::FRAME )
        ,nWrap(0)
        ,bOpaque( true )
        ,bContour(false)
        ,bContourOutside(true)
        ,bIgnoreWRK(true)
        ,nLeftMargin(319)
        ,nRightMargin(319)
        ,nTopMargin(0)
        ,nBottomMargin(0)
        ,bShadow(false)
        ,nShadowXDistance(0)
        ,nShadowYDistance(0)
        ,nShadowColor(0)
        ,nShadowTransparence(0)
        ,nContrast(0)
        ,nBrightness(0)
        ,fGamma( -1.0 )
        ,nFillColor( 0xffffffff )
        ,eColorMode( drawing::ColorMode_STANDARD )
        ,nCurrentBorderLine(BORDER_TOP)
        ,nDffType( 0 )
        ,bIsGraphic(false)
        ,bIsBitmap(false)
        ,bIsTiff(false)
        ,nBitsPerPixel(0)
        ,bHoriFlip(false)
        ,bVertFlip(false)
        ,bSizeProtected(false)
        ,bPositionProtected(false)
        ,nShapeOptionType(0)
        ,m_rPositivePercentages(rPositivePercentages)
    {}

    void setXSize(sal_Int32 _nXSize)
    {
        nXSize = _nXSize;
        bXSizeValid = true;
    }

    sal_uInt32 getXSize() const
    {
        return nXSize;
    }

    bool isXSizeValid() const
    {
        return bXSizeValid;
    }

    void setYSize(sal_Int32 _nYSize)
    {
        nYSize = _nYSize;
        bYSizeValid = true;
    }

    sal_uInt32 getYSize() const
    {
        return nYSize;
    }

    bool isYSizeValis () const
    {
        return bYSizeValid;
    }

    void applyMargins(uno::Reference< beans::XPropertySet > xGraphicObjectProperties) const
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_LEFT_MARGIN ), uno::makeAny(nLeftMargin));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_RIGHT_MARGIN ), uno::makeAny(nRightMargin));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_TOP_MARGIN ), uno::makeAny(nTopMargin));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_BOTTOM_MARGIN ), uno::makeAny(nBottomMargin));
    }

    void applyPosition(uno::Reference< beans::XPropertySet > xGraphicObjectProperties) const
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT          ),
                uno::makeAny(nHoriOrient));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT          ),
                uno::makeAny(nVertOrient));
    }

    void applyRelativePosition(uno::Reference< beans::XPropertySet > xGraphicObjectProperties) const
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT_POSITION),
                uno::makeAny(nLeftPosition));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_ORIENT_RELATION ),
                uno::makeAny(nHoriRelation));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_PAGE_TOGGLE ),
                uno::makeAny(bPageToggle));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT_POSITION),
                uno::makeAny(nTopPosition));
        xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_ORIENT_RELATION ),
                uno::makeAny(nVertRelation));
    }

    void applyZOrder(uno::Reference<beans::XPropertySet>& xGraphicObjectProperties) const
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        if (zOrder >= 0)
        {
            GraphicZOrderHelper* pZOrderHelper = rDomainMapper.graphicZOrderHelper();
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_Z_ORDER), uno::makeAny(pZOrderHelper->findZOrder(zOrder)));
            pZOrderHelper->addItem(xGraphicObjectProperties, zOrder);
        }
    }

    void applyName(uno::Reference<beans::XPropertySet>& xGraphicObjectProperties) const
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        try
        {
            if( !sName.isEmpty() )
            {
                uno::Reference< container::XNamed > xNamed( xGraphicObjectProperties, uno::UNO_QUERY_THROW );
                xNamed->setName( sName );
            }
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_DESCRIPTION ),
                uno::makeAny( sAlternativeText ));
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_TITLE ),
                uno::makeAny( title ));
        }
        catch( const uno::Exception& e )
        {
            SAL_WARN("writerfilter", "failed. Message :" << e.Message);
        }
    }
};

GraphicImport::GraphicImport(uno::Reference<uno::XComponentContext> xComponentContext,
                             uno::Reference<lang::XMultiServiceFactory> xTextFactory,
                             DomainMapper& rDMapper,
                             GraphicImportType eImportType,
                             std::queue<OUString>& rPositivePercentages)
: LoggedProperties(dmapper_logger, "GraphicImport")
, LoggedTable(dmapper_logger, "GraphicImport")
, LoggedStream(dmapper_logger, "GraphicImport")
, m_pImpl(new GraphicImport_Impl(eImportType, rDMapper, rPositivePercentages))
, m_xComponentContext(xComponentContext)
, m_xTextFactory(xTextFactory)
{
}

GraphicImport::~GraphicImport()
{
}

void GraphicImport::handleWrapTextValue(sal_uInt32 nVal)
{
    switch (nVal)
    {
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_bothSides: // 90920;
        m_pImpl->nWrap = text::WrapTextMode_PARALLEL;
        break;
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_left: // 90921;
        m_pImpl->nWrap = text::WrapTextMode_LEFT;
        break;
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_right: // 90922;
        m_pImpl->nWrap = text::WrapTextMode_RIGHT;
        break;
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_largest: // 90923;
        m_pImpl->nWrap = text::WrapTextMode_DYNAMIC;
        break;
    default:;
    }
}

void GraphicImport::putPropertyToFrameGrabBag( const OUString& sPropertyName, const uno::Any& aPropertyValue )
{
    beans::PropertyValue pProperty;
    pProperty.Name = sPropertyName;
    pProperty.Value = aPropertyValue;

    if (!m_xShape.is())
        return;

    uno::Reference< beans::XPropertySet > xSet(m_xShape, uno::UNO_QUERY_THROW);
    if (!xSet.is())
        return;

    uno::Reference< beans::XPropertySetInfo > xSetInfo(xSet->getPropertySetInfo());
    if (!xSetInfo.is())
        return;

    const OUString aGrabBagPropName("FrameInteropGrabBag");

    if (xSetInfo->hasPropertyByName(aGrabBagPropName))
    {
        uno::Sequence< beans::PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

        sal_Int32 nLength = aGrabBag.getLength();
        aGrabBag.realloc(nLength + 1);
        aGrabBag[nLength] = pProperty;

        xSet->setPropertyValue(aGrabBagPropName, uno::makeAny(aGrabBag));
    }
}

void GraphicImport::lcl_attribute(Id nName, Value& rValue)
{
    sal_Int32 nIntValue = rValue.getInt();
    switch( nName )
    {
        case NS_ooxml::LN_blip: //the binary graphic data in a shape
            {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rValue.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_payload :
        {
            writerfilter::Reference<BinaryObj>::Pointer_t pPictureData = rValue.getBinary();
            if( pPictureData.get())
                pPictureData->resolve(*this);
        }
        break;

        //border properties
        case NS_ooxml::LN_CT_Border_sz:
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineWidth = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_val:
            //graphic borders don't support different line types
        break;
        case NS_ooxml::LN_CT_Border_space:
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].nLineDistance = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            m_pImpl->aBorders[m_pImpl->nCurrentBorderLine].bHasShadow = nIntValue ? true : false;
        break;
        case NS_ooxml::LN_CT_Border_frame: // ignored
            break;
        case NS_ooxml::LN_CT_PositiveSize2D_cx:// 90407;
        case NS_ooxml::LN_CT_PositiveSize2D_cy:// 90408;
        {
            sal_Int32 nDim = ConversionHelper::convertEMUToMM100( nIntValue );
            if( nName == NS_ooxml::LN_CT_PositiveSize2D_cx )
                m_pImpl->setXSize(nDim);
            else
                m_pImpl->setYSize(nDim);
        }
        break;
        case NS_ooxml::LN_CT_EffectExtent_l:// 90907;
        case NS_ooxml::LN_CT_EffectExtent_t:// 90908;
        case NS_ooxml::LN_CT_EffectExtent_r:// 90909;
        case NS_ooxml::LN_CT_EffectExtent_b:// 90910;
            //todo: extends the wrapping size of the object, e.g. if shadow is added
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_id:// 90650;
            //id of the object - ignored
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_name:// 90651;
            //name of the object
            m_pImpl->sName = rValue.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_descr:// 90652;
            //alternative text
            m_pImpl->sAlternativeText = rValue.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_title:
            //alternative text
            m_pImpl->title = rValue.getString();
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noChangeAspect://90644;
            //disallow aspect ratio change - ignored
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noMove:// 90645;
            m_pImpl->bPositionProtected = true;
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noResize: // 90646;
            m_pImpl->bSizeProtected = true;
        break;
        case NS_ooxml::LN_CT_Anchor_distT: // 90983;
        case NS_ooxml::LN_CT_Anchor_distB: // 90984;
        case NS_ooxml::LN_CT_Anchor_distL: // 90985;
        case NS_ooxml::LN_CT_Anchor_distR: // 90986;
        {
            m_pImpl->nShapeOptionType = nName;
            ProcessShapeOptions(rValue);
        }
        break;
        case NS_ooxml::LN_CT_Anchor_simplePos_attr: // 90987;
            m_pImpl->bUseSimplePos = nIntValue > 0;
        break;
        case NS_ooxml::LN_CT_Anchor_relativeHeight: // 90988;
            m_pImpl->zOrder = nIntValue;
        break;
        case NS_ooxml::LN_CT_Anchor_behindDoc: // 90989; - in background
            if( nIntValue > 0 )
                m_pImpl->bOpaque = false;
        break;
        case NS_ooxml::LN_CT_Anchor_locked: // 90990; - ignored
        case NS_ooxml::LN_CT_Anchor_layoutInCell: // 90991; - ignored
        case NS_ooxml::LN_CT_Anchor_hidden: // 90992; - ignored
        break;
        case NS_ooxml::LN_CT_Anchor_allowOverlap: // 90993;
            //enable overlapping - ignored
        break;
        case NS_ooxml::LN_CT_Anchor_wp14_anchorId:
        case NS_ooxml::LN_CT_Inline_wp14_anchorId:
        {
            OUStringBuffer aBuffer = OUString::number(nIntValue, 16);
            OUStringBuffer aString;
            comphelper::string::padToLength(aString, 8 - aBuffer.getLength(), '0');
            aString.append(aBuffer.getStr());
            m_pImpl->sAnchorId = aString.makeStringAndClear().toAsciiUpperCase();
        }
        break;
        case NS_ooxml::LN_CT_Point2D_x: // 90405;
            m_pImpl->nLeftPosition = ConversionHelper::convertTwipToMM100(nIntValue);
            m_pImpl->nHoriRelation = text::RelOrientation::PAGE_FRAME;
            m_pImpl->nHoriOrient = text::HoriOrientation::NONE;
        break;
        case NS_ooxml::LN_CT_Point2D_y: // 90406;
            m_pImpl->nTopPosition = ConversionHelper::convertTwipToMM100(nIntValue);
            m_pImpl->nVertRelation = text::RelOrientation::PAGE_FRAME;
            m_pImpl->nVertOrient = text::VertOrientation::NONE;
        break;
        case NS_ooxml::LN_CT_WrapTight_wrapText: // 90934;
            m_pImpl->bContour = true;
            m_pImpl->bContourOutside = true;

            handleWrapTextValue(rValue.getInt());

            break;
        case NS_ooxml::LN_CT_WrapThrough_wrapText:
            m_pImpl->bContour = true;
            m_pImpl->bContourOutside = false;

            handleWrapTextValue(rValue.getInt());

            break;
        case NS_ooxml::LN_CT_WrapSquare_wrapText: //90928;
            handleWrapTextValue(rValue.getInt());
            break;
        case NS_ooxml::LN_shape:
            {
                uno::Reference< drawing::XShape> xShape;
                rValue.getAny( ) >>= xShape;

                if ( xShape.is( ) )
                {
                    // Is it a graphic image
                    bool bUseShape = true;
                    try
                    {
                        uno::Reference< beans::XPropertySet > xShapeProps
                            ( xShape, uno::UNO_QUERY_THROW );

                        OUString sUrl;
                        xShapeProps->getPropertyValue("GraphicURL") >>= sUrl;

                        sal_Int32 nRotation = 0;
                        xShapeProps->getPropertyValue("RotateAngle") >>= nRotation;

                        ::com::sun::star::beans::PropertyValues aMediaProperties( 1 );
                        aMediaProperties[0].Name = "URL";
                        aMediaProperties[0].Value <<= sUrl;

                        xShapeProps->getPropertyValue("Shadow") >>= m_pImpl->bShadow;
                        if (m_pImpl->bShadow)
                        {
                            xShapeProps->getPropertyValue("ShadowXDistance") >>= m_pImpl->nShadowXDistance;
                            xShapeProps->getPropertyValue("ShadowYDistance") >>= m_pImpl->nShadowYDistance;
                            xShapeProps->getPropertyValue("ShadowColor") >>= m_pImpl->nShadowColor;
                            xShapeProps->getPropertyValue("ShadowTransparence") >>= m_pImpl->nShadowTransparence;
                        }

                        // fdo#70457: transform XShape into a SwXTextGraphicObject only if there's no rotation
                        if ( nRotation == 0 )
                            m_xGraphicObject = createGraphicObject( aMediaProperties );

                        bUseShape = !m_xGraphicObject.is( );

                        if ( !bUseShape )
                        {
                            // Define the object size
                            uno::Reference< beans::XPropertySet > xGraphProps( m_xGraphicObject,
                                    uno::UNO_QUERY );
                            awt::Size aSize = xShape->getSize( );
                            xGraphProps->setPropertyValue("Height",
                                   uno::makeAny( aSize.Height ) );
                            xGraphProps->setPropertyValue("Width",
                                   uno::makeAny( aSize.Width ) );

                            text::GraphicCrop aGraphicCrop( 0, 0, 0, 0 );
                            uno::Reference< beans::XPropertySet > xSourceGraphProps( xShape, uno::UNO_QUERY );
                            uno::Any aAny = xSourceGraphProps->getPropertyValue("GraphicCrop");
                            if(aAny >>= aGraphicCrop) {
                                xGraphProps->setPropertyValue("GraphicCrop",
                                    uno::makeAny( aGraphicCrop ) );
                            }

                            // We need to drop the shape here somehow
                            uno::Reference< lang::XComponent > xShapeComponent( xShape, uno::UNO_QUERY );
                            xShapeComponent->dispose( );
                        }
                    }
                    catch( const beans::UnknownPropertyException & )
                    {
                        // It isn't a graphic image
                    }

                    if ( bUseShape )
                        m_xShape = xShape;


                    if ( m_xShape.is( ) )
                    {
                        uno::Reference< beans::XPropertySet > xShapeProps
                            (m_xShape, uno::UNO_QUERY_THROW);


                        PropertyNameSupplier& rPropNameSupplier =
                            PropertyNameSupplier::GetPropertyNameSupplier();
                        xShapeProps->setPropertyValue
                            (rPropNameSupplier.GetName(PROP_ANCHOR_TYPE),
                             uno::makeAny
                             (text::TextContentAnchorType_AS_CHARACTER));

                        uno::Reference<lang::XServiceInfo> xServiceInfo(m_xShape, uno::UNO_QUERY_THROW);

                        // TextFrames can't be rotated. But for anything else,
                        // make sure that setting size doesn't affect rotation,
                        // that would not match Word's definition of rotation.
                        bool bKeepRotation = false;
                        if (!xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
                        {
                            bKeepRotation = true;
                            xShapeProps->setPropertyValue
                                (rPropNameSupplier.GetName(PROP_TEXT_RANGE),
                                 uno::makeAny
                                 (m_pImpl->rDomainMapper.GetCurrentTextRange()));
                        }

                        awt::Size aSize(m_xShape->getSize());

                        if (m_pImpl->isXSizeValid())
                            aSize.Width = m_pImpl->getXSize();
                        if (m_pImpl->isYSizeValis())
                            aSize.Height = m_pImpl->getYSize();

                        uno::Any aRotation;
                        if (bKeepRotation)
                            aRotation = xShapeProps->getPropertyValue("RotateAngle");
                        m_xShape->setSize(aSize);
                        if (bKeepRotation)
                            xShapeProps->setPropertyValue("RotateAngle", aRotation);

                        m_pImpl->bIsGraphic = true;

                        if (!m_pImpl->sAnchorId.isEmpty())
                        {
                            putPropertyToFrameGrabBag("AnchorId", uno::makeAny(m_pImpl->sAnchorId));
                        }
                    }

                    if (bUseShape && m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
                    {
                        // If we are here, this is a drawingML shape. For those, only dmapper (and not oox) knows the anchoring infos (just like for Writer pictures).
                        // But they aren't Writer pictures, either (which are already handled above).
                        uno::Reference< beans::XPropertySet > xShapeProps(m_xShape, uno::UNO_QUERY_THROW);
                        // This needs to be AT_PARAGRAPH and not AT_CHARACTER, otherwise shape will move when the user inserts a new paragraph.
                        xShapeProps->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_PARAGRAPH));

                        //only the position orientation is handled in applyPosition()
                        m_pImpl->applyPosition(xShapeProps);

                        uno::Reference<lang::XServiceInfo> xServiceInfo(m_xShape, uno::UNO_QUERY_THROW);
                        if (xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
                        {
                            // Position of the groupshape should be set after children have been added.
                            m_xShape->setPosition(awt::Point(m_pImpl->nLeftPosition, m_pImpl->nTopPosition));
                        }
                        m_pImpl->applyRelativePosition(xShapeProps);

                        m_pImpl->applyMargins(xShapeProps);
                        bool bOpaque = m_pImpl->bOpaque && !m_pImpl->rDomainMapper.IsInHeaderFooter();
                        xShapeProps->setPropertyValue("Opaque", uno::makeAny(bOpaque));
                        xShapeProps->setPropertyValue("Surround", uno::makeAny(m_pImpl->nWrap));
                        m_pImpl->applyZOrder(xShapeProps);
                        m_pImpl->applyName(xShapeProps);
                    }
                }
            }
        break;
        case NS_ooxml::LN_CT_Inline_distT:
            m_pImpl->nTopMargin = ConversionHelper::convertEMUToMM100(nIntValue);
        break;
        case NS_ooxml::LN_CT_Inline_distB:
            m_pImpl->nBottomMargin = ConversionHelper::convertEMUToMM100(nIntValue);
        break;
        case NS_ooxml::LN_CT_Inline_distL:
            m_pImpl->nLeftMargin = ConversionHelper::convertEMUToMM100(nIntValue);
        break;
        case NS_ooxml::LN_CT_Inline_distR:
            m_pImpl->nRightMargin = ConversionHelper::convertEMUToMM100(nIntValue);
        break;
        case NS_ooxml::LN_CT_GraphicalObjectData_uri:
            rValue.getString();
            //TODO: does it need to be handled?
        break;
        case NS_ooxml::LN_CT_SizeRelH_relativeFrom:
            {
                switch (nIntValue)
                {
                case NS_ooxml::LN_ST_SizeRelFromH_margin:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::makeAny(text::RelOrientation::FRAME));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromH_page:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::makeAny(text::RelOrientation::PAGE_FRAME));
                    }
                    break;
                default:
                    SAL_WARN("writerfilter", "GraphicImport::lcl_attribute: unhandled NS_ooxml::LN_CT_SizeRelH_relativeFrom value: " << nIntValue);
                    break;
                }
            }
            break;
        case NS_ooxml::LN_CT_SizeRelV_relativeFrom:
            {
                switch (nIntValue)
                {
                case NS_ooxml::LN_ST_SizeRelFromV_margin:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::makeAny(text::RelOrientation::FRAME));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromV_page:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::makeAny(text::RelOrientation::PAGE_FRAME));
                    }
                    break;
                default:
                    SAL_WARN("writerfilter", "GraphicImport::lcl_attribute: unhandled NS_ooxml::LN_CT_SizeRelV_relativeFrom value: " << nIntValue);
                    break;
                }
            }
            break;
        default:
#ifdef DEBUG_DMAPPER_GRAPHIC_IMPORT
            dmapper_logger->element("unhandled");
#endif
            break;
    }
}

uno::Reference<text::XTextContent> GraphicImport::GetGraphicObject()
{
    uno::Reference<text::XTextContent> xResult;

    if (m_xGraphicObject.is())
        xResult = m_xGraphicObject;
    else if (m_xShape.is())
    {
        xResult.set(m_xShape, uno::UNO_QUERY_THROW);
    }

    return xResult;
}

uno::Reference<drawing::XShape> GraphicImport::GetXShapeObject(){
    return m_xShape;
}

void GraphicImport::ProcessShapeOptions(Value& rValue)
{
    sal_Int32 nIntValue = rValue.getInt();
    switch( m_pImpl->nShapeOptionType )
    {
        case NS_ooxml::LN_CT_Anchor_distL:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->nLeftMargin = nIntValue / 360;
        break;
        case NS_ooxml::LN_CT_Anchor_distT:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->nTopMargin = nIntValue / 360;
        break;
        case NS_ooxml::LN_CT_Anchor_distR:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->nRightMargin = nIntValue / 360;
        break;
        case NS_ooxml::LN_CT_Anchor_distB:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->nBottomMargin = nIntValue / 360;
        break;
        default:
            OSL_FAIL( "shape option unsupported?");
    }
}


void GraphicImport::lcl_sprm(Sprm& rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();
    Value::Pointer_t pValue = rSprm.getValue();

    switch(nSprmId)
    {
        case 0xf004: //dff record
        case 0xf00a: //part of 0xf004 - shape properties
        case 0xf00b: //part of 0xf004
        case 0xf007:
        case 0xf122: //udefprop
        case NS_ooxml::LN_CT_Inline_extent: // 90911;
        case NS_ooxml::LN_CT_Inline_effectExtent: // 90912;
        case NS_ooxml::LN_CT_Inline_docPr: // 90913;
        case NS_ooxml::LN_CT_Inline_cNvGraphicFramePr: // 90914;
        case NS_ooxml::LN_CT_NonVisualGraphicFrameProperties_graphicFrameLocks:// 90657
        case NS_ooxml::LN_CT_Inline_a_graphic:// 90915
        case NS_ooxml::LN_CT_Anchor_simplePos_elem: // 90975;
        case NS_ooxml::LN_CT_Anchor_extent: // 90978;
        case NS_ooxml::LN_CT_Anchor_effectExtent: // 90979;
        case NS_ooxml::LN_EG_WrapType_wrapSquare: // 90945;
        case NS_ooxml::LN_EG_WrapType_wrapTight: // 90946;
        case NS_ooxml::LN_EG_WrapType_wrapThrough:
        case NS_ooxml::LN_CT_Anchor_docPr: // 90980;
        case NS_ooxml::LN_CT_Anchor_cNvGraphicFramePr: // 90981;
        case NS_ooxml::LN_CT_Anchor_a_graphic: // 90982;
        case NS_ooxml::LN_CT_WrapPath_start: // 90924;
        case NS_ooxml::LN_CT_WrapPath_lineTo: // 90925;
        case NS_ooxml::LN_graphic_graphic:
        case NS_ooxml::LN_pic_pic:
        case NS_ooxml::LN_dgm_relIds:
        case NS_ooxml::LN_lc_lockedCanvas:
        case NS_ooxml::LN_c_chart:
        case NS_ooxml::LN_wps_wsp:
        case NS_ooxml::LN_wpg_wgp:
        case NS_ooxml::LN_sizeRelH_sizeRelH:
        case NS_ooxml::LN_sizeRelV_sizeRelV:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_CT_WrapTight_wrapPolygon:
        case NS_ooxml::LN_CT_WrapThrough_wrapPolygon:
            {
                WrapPolygonHandler aHandler;

                resolveSprmProps(aHandler, rSprm);

                m_pImpl->mpWrapPolygon = aHandler.getPolygon();
            }
            break;
        case NS_ooxml::LN_CT_Anchor_positionH: // 90976;
        {
            // Use a special handler for the positionning
            PositionHandlerPtr pHandler( new PositionHandler( false ));
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get( ) )
            {
                pProperties->resolve( *pHandler );
                if( !m_pImpl->bUseSimplePos )
                {
                    m_pImpl->nHoriRelation = pHandler->relation();
                    m_pImpl->nHoriOrient = pHandler->orientation();
                    m_pImpl->nLeftPosition = pHandler->position();
                    if (m_pImpl->nHoriRelation == text::RelOrientation::PAGE_FRAME && m_pImpl->nHoriOrient == text::HoriOrientation::RIGHT)
                    {
                        // If the shape is relative from page and aligned to
                        // right, then set the relation to right and clear the
                        // orientation, that provides the same visual result as
                        // Word.
                        m_pImpl->nHoriRelation = text::RelOrientation::PAGE_RIGHT;
                        m_pImpl->nHoriOrient = text::HoriOrientation::NONE;
                    }
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_Anchor_positionV: // 90977;
        {
            // Use a special handler for the positionning
            PositionHandlerPtr pHandler( new PositionHandler( true ));
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get( ) )
            {
                pProperties->resolve( *pHandler );
                if( !m_pImpl->bUseSimplePos )
                {
                    m_pImpl->nVertRelation = pHandler->relation();
                    m_pImpl->nVertOrient = pHandler->orientation();
                    m_pImpl->nTopPosition = pHandler->position();
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_SizeRelH_pctWidth:
        case NS_ooxml::LN_CT_SizeRelV_pctHeight:
            if (m_xShape.is() && !m_pImpl->m_rPositivePercentages.empty())
            {
                sal_Int16 nPositivePercentage = rtl::math::round(m_pImpl->m_rPositivePercentages.front().toDouble() / oox::drawingml::PER_PERCENT);
                m_pImpl->m_rPositivePercentages.pop();

                if (nPositivePercentage)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                    OUString aProperty = nSprmId == NS_ooxml::LN_CT_SizeRelH_pctWidth ? OUString("RelativeWidth") : OUString("RelativeHeight");
                    xPropertySet->setPropertyValue(aProperty, uno::makeAny(nPositivePercentage));
                }
            }
            break;
        case 0x271b:
        case 0x271c:
        {
            if( nSprmId != 0x271c || m_pImpl->nDffType == 0xf01f || m_pImpl->nDffType == 0xf01e )
            {
                writerfilter::Reference<BinaryObj>::Pointer_t pPictureData = rSprm.getBinary();
                if( pPictureData.get())
                    pPictureData->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_EG_WrapType_wrapNone: // 90944; - doesn't contain attributes
            //depending on the behindDoc attribute text wraps through behind or in fron of the object
            m_pImpl->nWrap = text::WrapTextMode_THROUGHT;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapTopAndBottom: // 90948;
            m_pImpl->nWrap = text::WrapTextMode_NONE;
        break;
        case 0xf010:
        case 0xf011:
            //ignore - doesn't contain useful members
        break;
        case NS_ooxml::LN_CT_GraphicalObject_graphicData:// 90660;
            {
                m_pImpl->bIsGraphic = true;

                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                    pProperties->resolve(*this);
            }
        break;
        default:
#if OSL_DEBUG_LEVEL > 0
            OString sMessage( "GraphicImport::sprm() - Id: ");
            sMessage += OString::number( nSprmId, 10 );
            sMessage += " / 0x";
            sMessage += OString::number( nSprmId, 16 );
            SAL_WARN("writerfilter", sMessage.getStr());
#endif
        break;
    }
}

void GraphicImport::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}
/*-------------------------------------------------------------------------
    crop is stored as "fixed float" as 16.16 fraction value
    related to width/or height
  -----------------------------------------------------------------------*/
void lcl_CalcCrop( sal_Int32& nCrop, sal_Int32 nRef )
{
    nCrop = ((nCrop >> 16   ) * nRef )
       + (((nCrop & 0xffff) * nRef ) >> 16);
}

uno::Reference< text::XTextContent > GraphicImport::createGraphicObject( const beans::PropertyValues& aMediaProperties )
{
    uno::Reference< text::XTextContent > xGraphicObject;
    try
    {
        uno::Reference< graphic::XGraphicProvider > xGraphicProvider( graphic::GraphicProvider::create(m_xComponentContext) );
        uno::Reference< graphic::XGraphic > xGraphic = xGraphicProvider->queryGraphic( aMediaProperties );

        if(xGraphic.is())
        {
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

            uno::Reference< beans::XPropertySet > xGraphicObjectProperties(
            m_xTextFactory->createInstance("com.sun.star.text.TextGraphicObject"),
                uno::UNO_QUERY_THROW);
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_GRAPHIC), uno::makeAny( xGraphic ));
            xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_ANCHOR_TYPE),
                uno::makeAny( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR ?
                                    text::TextContentAnchorType_AT_CHARACTER :
                                    text::TextContentAnchorType_AS_CHARACTER ));
            xGraphicObject = uno::Reference< text::XTextContent >( xGraphicObjectProperties, uno::UNO_QUERY_THROW );

            //shapes have only one border, PICF might have four
            table::BorderLine2 aBorderLine;
            for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
            {
                if( m_pImpl->eGraphicImportType == IMPORT_AS_GRAPHIC || !nBorder )
                {
                    aBorderLine.Color = m_pImpl->aBorders[m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE ? BORDER_TOP : static_cast<BorderPosition>(nBorder) ].nLineColor;
                    aBorderLine.InnerLineWidth = 0;
                    aBorderLine.OuterLineWidth = (sal_Int16)m_pImpl->aBorders[m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE ? BORDER_TOP : static_cast<BorderPosition>(nBorder) ].nLineWidth;
                    aBorderLine.LineDistance = 0;
                }
                PropertyIds aBorderProps[4] =
                {
                    PROP_LEFT_BORDER,
                    PROP_RIGHT_BORDER,
                    PROP_TOP_BORDER,
                    PROP_BOTTOM_BORDER
                };
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( aBorderProps[nBorder]), uno::makeAny(aBorderLine));
            }

            // setting graphic object shadow proerties
            if (m_pImpl->bShadow)
            {
                // Shadow width is approximated by average of X and Y
                table::ShadowFormat aShadow;
                sal_uInt32 nShadowColor = m_pImpl->nShadowColor & 0x00FFFFFF; // The shadow color we get is RGB only.
                sal_Int32 nShadowWidth = (abs(m_pImpl->nShadowXDistance)
                                          + abs(m_pImpl->nShadowYDistance)) / 2;

                aShadow.ShadowWidth = nShadowWidth;
                sal_uInt8 nShadowTransparence = float(m_pImpl->nShadowTransparence) * 2.55;
                nShadowColor |= (nShadowTransparence << 24); // Add transparence to the color.
                aShadow.Color = nShadowColor;
                // Distances -ve for top and right, +ve for bottom and left
                if (m_pImpl->nShadowXDistance > 0)
                {
                    if (m_pImpl->nShadowYDistance > 0)
                        aShadow.Location = com::sun::star::table::ShadowLocation_BOTTOM_RIGHT;
                    else
                        aShadow.Location = com::sun::star::table::ShadowLocation_TOP_RIGHT;
                }
                else
                {
                    if (m_pImpl->nShadowYDistance > 0)
                        aShadow.Location = com::sun::star::table::ShadowLocation_BOTTOM_LEFT;
                    else
                        aShadow.Location = com::sun::star::table::ShadowLocation_TOP_LEFT;
                }

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SHADOW_FORMAT), uno::makeAny(aShadow));
            }

            // setting properties for all types
            if( m_pImpl->bPositionProtected )
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_POSITION_PROTECTED ),
                    uno::makeAny(true));
            if( m_pImpl->bSizeProtected )
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SIZE_PROTECTED ),
                    uno::makeAny(true));

            if( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR )
            {
                sal_Int32 nWidth = m_pImpl->nRightPosition - m_pImpl->nLeftPosition;
                if( m_pImpl->eGraphicImportType == IMPORT_AS_SHAPE )
                {
                    sal_Int32 nHeight = m_pImpl->nBottomPosition - m_pImpl->nTopPosition;
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SIZE),
                        uno::makeAny( awt::Size( nWidth, nHeight )));
                }
                //adjust margins
                if( (m_pImpl->nHoriOrient == text::HoriOrientation::LEFT &&
                    (m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                        m_pImpl->nHoriRelation == text::RelOrientation::FRAME) ) ||
                     (m_pImpl->nHoriOrient == text::HoriOrientation::INSIDE &&
                       m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ))
                    m_pImpl->nLeftMargin = 0;
                if((m_pImpl->nHoriOrient == text::HoriOrientation::RIGHT &&
                        (m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                            m_pImpl->nHoriRelation == text::RelOrientation::FRAME) ) ||
                    (m_pImpl->nHoriOrient == text::HoriOrientation::INSIDE &&
                        m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ))
                    m_pImpl->nRightMargin = 0;
                // adjust top/bottom margins
                if( m_pImpl->nVertOrient == text::VertOrientation::TOP &&
                        ( m_pImpl->nVertRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                            m_pImpl->nVertRelation == text::RelOrientation::PAGE_FRAME))
                    m_pImpl->nTopMargin = 0;
                if( m_pImpl->nVertOrient == text::VertOrientation::BOTTOM &&
                        ( m_pImpl->nVertRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                            m_pImpl->nVertRelation == text::RelOrientation::PAGE_FRAME))
                    m_pImpl->nBottomMargin = 0;
                if( m_pImpl->nVertOrient == text::VertOrientation::BOTTOM &&
                        m_pImpl->nVertRelation == text::RelOrientation::PAGE_PRINT_AREA )
                    m_pImpl->nBottomMargin = 0;

                //adjust alignment
                if( m_pImpl->nHoriOrient == text::HoriOrientation::INSIDE &&
                        m_pImpl->nHoriRelation == text::RelOrientation::PAGE_FRAME )
                {
                    // convert 'left to page' to 'from left -<width> to page text area'
                    m_pImpl->nHoriOrient = text::HoriOrientation::NONE;
                    m_pImpl->nHoriRelation = text::RelOrientation::PAGE_PRINT_AREA;
                    m_pImpl->nLeftPosition = - nWidth;
                }
                else if( m_pImpl->nHoriOrient == text::HoriOrientation::OUTSIDE &&
                        m_pImpl->nHoriRelation == text::RelOrientation::PAGE_FRAME )
                {
                    // convert 'right to page' to 'from left 0 to right page border'
                    m_pImpl->nHoriOrient = text::HoriOrientation::NONE;
                    m_pImpl->nHoriRelation = text::RelOrientation::PAGE_RIGHT;
                    m_pImpl->nLeftPosition = 0;
                }

                m_pImpl->applyPosition(xGraphicObjectProperties);
                m_pImpl->applyRelativePosition(xGraphicObjectProperties);
                bool bOpaque = m_pImpl->bOpaque && !m_pImpl->rDomainMapper.IsInHeaderFooter( );
                if( !bOpaque )
                {
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_OPAQUE ),
                        uno::makeAny(bOpaque));
                }
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SURROUND ),
                        uno::makeAny(m_pImpl->nWrap));

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_SURROUND_CONTOUR ),
                    uno::makeAny(m_pImpl->bContour));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_CONTOUR_OUTSIDE ),
                    uno::makeAny(m_pImpl->bContourOutside));
                m_pImpl->applyMargins(xGraphicObjectProperties);

                if( m_pImpl->eColorMode == drawing::ColorMode_STANDARD &&
                    m_pImpl->nContrast == -70 &&
                    m_pImpl->nBrightness == 70 )
                {
                    // strange definition of WATERMARK!
                    m_pImpl->nContrast = 0;
                    m_pImpl->nBrightness = 0;
                    m_pImpl->eColorMode = drawing::ColorMode_WATERMARK;
                }

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_ADJUST_CONTRAST ),
                    uno::makeAny((sal_Int16)m_pImpl->nContrast));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_ADJUST_LUMINANCE ),
                    uno::makeAny((sal_Int16)m_pImpl->nBrightness));
                if(m_pImpl->eColorMode != drawing::ColorMode_STANDARD)
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_GRAPHIC_COLOR_MODE ),
                        uno::makeAny(m_pImpl->eColorMode));
                if(m_pImpl->fGamma > 0. )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_GAMMA ),
                        uno::makeAny(m_pImpl->fGamma ));
                if(m_pImpl->bHoriFlip)
                {
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_MIRRORED_ON_EVEN_PAGES ),
                        uno::makeAny( m_pImpl->bHoriFlip ));
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_HORI_MIRRORED_ON_ODD_PAGES ),
                        uno::makeAny( m_pImpl->bHoriFlip ));
                }

                if( m_pImpl->bVertFlip )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_VERT_MIRRORED ),
                        uno::makeAny( m_pImpl->bVertFlip ));
                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_BACK_COLOR ),
                    uno::makeAny( m_pImpl->nFillColor ));

                m_pImpl->applyZOrder(xGraphicObjectProperties);

                //there seems to be no way to detect the original size via _real_ API
                uno::Reference< beans::XPropertySet > xGraphicProperties( xGraphic, uno::UNO_QUERY_THROW );
                awt::Size aGraphicSize, aGraphicSizePixel;
                xGraphicProperties->getPropertyValue(rPropNameSupplier.GetName( PROP_SIZE100th_M_M )) >>= aGraphicSize;
                xGraphicProperties->getPropertyValue(rPropNameSupplier.GetName( PROP_SIZE_PIXEL )) >>= aGraphicSizePixel;

                uno::Any aContourPolyPolygon;
                if( aGraphicSize.Width && aGraphicSize.Height &&
                    m_pImpl->mpWrapPolygon.get() != NULL)
                {
                    WrapPolygon::Pointer_t pCorrected = m_pImpl->mpWrapPolygon->correctWordWrapPolygon(aGraphicSize);
                    aContourPolyPolygon <<= pCorrected->getPointSequenceSequence();
                }

                xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_CONTOUR_POLY_POLYGON),
                                                           aContourPolyPolygon);

                if( aGraphicSize.Width && aGraphicSize.Height )
                {
                    //todo: i71651 graphic size is not provided by the GraphicDescriptor
                    lcl_CalcCrop( m_pImpl->nTopCrop, aGraphicSize.Height );
                    lcl_CalcCrop( m_pImpl->nBottomCrop, aGraphicSize.Height );
                    lcl_CalcCrop( m_pImpl->nLeftCrop, aGraphicSize.Width );
                    lcl_CalcCrop( m_pImpl->nRightCrop, aGraphicSize.Width );


                    // We need a separate try-catch here, otherwise a bad crop setting will also nuke the size settings as well.
                    try
                    {
                        xGraphicProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_GRAPHIC_CROP ),
                                uno::makeAny(text::GraphicCrop(m_pImpl->nTopCrop, m_pImpl->nBottomCrop, m_pImpl->nLeftCrop, m_pImpl->nRightCrop)));
                    }
                    catch (const uno::Exception& e)
                    {
                        SAL_WARN("writerfilter", "failed. Message :" << e.Message);
                    }
                }

            }

            if(m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_INLINE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
            {
                if( m_pImpl->getXSize() && m_pImpl->getYSize() )
                    xGraphicObjectProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SIZE),
                        uno::makeAny( awt::Size( m_pImpl->getXSize(), m_pImpl->getYSize() )));
                m_pImpl->applyMargins(xGraphicObjectProperties);
                m_pImpl->applyName(xGraphicObjectProperties);
            }
        }
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("writerfilter", "failed. Message :" << e.Message);
    }
    return xGraphicObject;
}



void GraphicImport::data(const sal_uInt8* buf, size_t len, writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

        ::com::sun::star::beans::PropertyValues aMediaProperties( 1 );
        aMediaProperties[0].Name = rPropNameSupplier.GetName(PROP_INPUT_STREAM);

        uno::Reference< io::XInputStream > xIStream = new XInputStreamHelper( buf, len, m_pImpl->bIsBitmap );
        aMediaProperties[0].Value <<= xIStream;

        m_xGraphicObject = createGraphicObject( aMediaProperties );
}


void GraphicImport::lcl_startSectionGroup()
{
}


void GraphicImport::lcl_endSectionGroup()
{
}


void GraphicImport::lcl_startParagraphGroup()
{
}


void GraphicImport::lcl_endParagraphGroup()
{
}


void GraphicImport::lcl_startCharacterGroup()
{
}


void GraphicImport::lcl_endCharacterGroup()
{
}


void GraphicImport::lcl_text(const sal_uInt8 * /*_data*/, size_t /*len*/)
{
}


void GraphicImport::lcl_utext(const sal_uInt8 * /*_data*/, size_t /*len*/)
{
}


void GraphicImport::lcl_props(writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}


void GraphicImport::lcl_table(Id /*name*/, writerfilter::Reference<Table>::Pointer_t /*ref*/)
{
}


void GraphicImport::lcl_substream(Id /*name*/, ::writerfilter::Reference<Stream>::Pointer_t /*ref*/)
{
}


void GraphicImport::lcl_info(const string & /*info*/)
{
}

void GraphicImport::lcl_startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > /*xShape*/ )
{
}

void GraphicImport::lcl_endShape( )
{
}

bool GraphicImport::IsGraphic() const
{
    return m_pImpl->bIsGraphic;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
