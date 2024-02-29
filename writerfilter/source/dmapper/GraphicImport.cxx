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
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/BufferSizeExceededException.hpp>
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

#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdtrans.hxx>
#include <svx/unoapi.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <rtl/math.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

#include "DomainMapper.hxx"
#include <dmapper/GraphicZOrderHelper.hxx>
#include <ooxml/resourceids.hxx>

#include "ConversionHelper.hxx"
#include "GraphicHelpers.hxx"
#include "GraphicImport.hxx"
#include "PropertyMap.hxx"
#include "TagLogger.hxx"
#include "WrapPolygonHandler.hxx"
#include "util.hxx"

#include <comphelper/propertysequence.hxx>
#include <algorithm>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <o3tl/unit_conversion.hxx>
#include <oox/export/drawingml.hxx>
#include <utility>

using namespace css;

namespace
{
bool isTopGroupObj(const uno::Reference<drawing::XShape>& xShape)
{
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
    if (!pObject)
        return false;

    if (pObject->getParentSdrObjectFromSdrObject())
        return false;

    return pObject->IsGroupObject();
}
}

namespace writerfilter::dmapper
{

namespace {

class XInputStreamHelper : public cppu::WeakImplHelper<io::XInputStream>
{
    const sal_uInt8* m_pBuffer;
    const sal_Int32  m_nLength;
    sal_Int32        m_nPosition;
public:
    XInputStreamHelper(const sal_uInt8* buf, size_t len);

    virtual ::sal_Int32 SAL_CALL readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) override;
    virtual ::sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) override;
    virtual ::sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;
};

}

XInputStreamHelper::XInputStreamHelper(const sal_uInt8* buf, size_t len) :
        m_pBuffer( buf ),
        m_nLength( len ),
        m_nPosition( 0 )
{
}

sal_Int32 XInputStreamHelper::readBytes( uno::Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
{
    return readSomeBytes( aData, nBytesToRead );
}

sal_Int32 XInputStreamHelper::readSomeBytes( uno::Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
{
    sal_Int32 nRet = 0;
    if( nMaxBytesToRead > 0 )
    {
        if( nMaxBytesToRead > m_nLength - m_nPosition )
            nRet = m_nLength - m_nPosition;
        else
            nRet = nMaxBytesToRead;
        aData.realloc( nRet );
        sal_Int8* pData = aData.getArray();
        if( nRet )
        {
            memcpy( pData, m_pBuffer + m_nPosition, nRet );
            m_nPosition += nRet;
        }
    }
    return nRet;
}


void XInputStreamHelper::skipBytes( sal_Int32 nBytesToSkip )
{
    if( nBytesToSkip < 0 || m_nPosition + nBytesToSkip > m_nLength)
        throw io::BufferSizeExceededException();
    m_nPosition += nBytesToSkip;
}


sal_Int32 XInputStreamHelper::available(  )
{
    return m_nLength - m_nPosition;
}


void XInputStreamHelper::closeInput(  )
{
}

namespace {

struct GraphicBorderLine
{
    sal_Int32   nLineWidth;
    bool        bHasShadow;

    GraphicBorderLine() :
        nLineWidth(0)
        ,bHasShadow(false)
        {}

    bool isEmpty() const
    {
        return nLineWidth == 0 && !bHasShadow;
    }

};

}

class GraphicImport_Impl
{
private:
    sal_Int32 m_nXSize;
    bool      m_bXSizeValid;
    sal_Int32 m_nYSize;
    bool      m_bYSizeValid;

public:
    GraphicImportType & m_rGraphicImportType;
    DomainMapper&   m_rDomainMapper;

    sal_Int32 m_nLeftPosition;
    sal_Int32 m_nTopPosition;

    bool      m_bUseSimplePos;
    std::optional<sal_Int32> m_oZOrder;

    sal_Int16 m_nHoriOrient;
    sal_Int16 m_nHoriRelation;
    bool m_bPageToggle = false;
    sal_Int16 m_nVertOrient;
    sal_Int16 m_nVertRelation;
    text::WrapTextMode m_nWrap;
    bool      m_bLayoutInCell;
    bool      m_bCompatForcedLayoutInCell;
    bool m_bAllowOverlap = true;

    // Opaque means not in the background (but instead, the graphic will be over top of the text)
    // This flag holds where LO will ACTUALLY put the graphic
    bool      m_bOpaque;
    // BehindDoc means in the background. This flag says the graphic REQUESTED to be behind the text
    bool      m_bBehindDoc;

    bool      m_bContour;
    bool      m_bContourOutside;
    WrapPolygon::Pointer_t mpWrapPolygon;

    sal_Int32 m_nLeftMargin;
    sal_Int32 m_nLeftMarginOrig = 0;
    sal_Int32 m_nRightMargin;
    sal_Int32 m_nTopMargin;
    sal_Int32 m_nBottomMargin;

    bool m_bShadow;
    sal_Int32 m_nShadowXDistance;
    sal_Int32 m_nShadowYDistance;
    sal_Int32 m_nShadowColor;
    sal_Int32 m_nShadowTransparence;

    sal_Int32 m_nContrast;
    sal_Int32 m_nBrightness;

    static constexpr sal_Int32 nFillColor = 0xffffffff;

    drawing::ColorMode m_eColorMode;

    GraphicBorderLine   m_aBorders[4];

    bool            m_bIsGraphic;

    bool            m_bSizeProtected;
    bool            m_bPositionProtected;
    bool            m_bHidden;
    bool            m_bDecorative = false;

    sal_Int32       m_nShapeOptionType;

    OUString m_sName;
    OUString m_sAlternativeText;
    OUString m_title;
    OUString m_sHyperlinkURL;
    std::pair<OUString, OUString>& m_rPositionOffsets;
    std::pair<OUString, OUString>& m_rAligns;
    std::queue<OUString>& m_rPositivePercentages;
    OUString m_sAnchorId;
    comphelper::SequenceAsHashMap m_aInteropGrabBag;
    std::optional<sal_Int32> m_oEffectExtentLeft;
    std::optional<sal_Int32> m_oEffectExtentTop;
    std::optional<sal_Int32> m_oEffectExtentRight;
    std::optional<sal_Int32> m_oEffectExtentBottom;
    std::optional<text::GraphicCrop> m_oCrop;

    GraphicImport_Impl(GraphicImportType & rImportType, DomainMapper& rDMapper,
            std::pair<OUString, OUString>& rPositionOffsets,
            std::pair<OUString, OUString>& rAligns,
            std::queue<OUString>& rPositivePercentages)
        : m_nXSize(0)
        ,m_bXSizeValid(false)
        ,m_nYSize(0)
        ,m_bYSizeValid(false)
        ,m_rGraphicImportType(rImportType)
        ,m_rDomainMapper( rDMapper )
        ,m_nLeftPosition(0)
        ,m_nTopPosition(0)
        ,m_bUseSimplePos(false)
        ,m_nHoriOrient(   text::HoriOrientation::NONE )
        ,m_nHoriRelation( text::RelOrientation::FRAME )
        ,m_nVertOrient(  text::VertOrientation::NONE )
        ,m_nVertRelation( text::RelOrientation::FRAME )
        ,m_nWrap(text::WrapTextMode_NONE)
        ,m_bLayoutInCell(true)
        ,m_bCompatForcedLayoutInCell(false)
        ,m_bOpaque( !rDMapper.IsInHeaderFooter() )
        ,m_bBehindDoc(false)
        ,m_bContour(false)
        ,m_bContourOutside(true)
        ,m_nLeftMargin(319)
        ,m_nRightMargin(319)
        ,m_nTopMargin(0)
        ,m_nBottomMargin(0)
        ,m_bShadow(false)
        ,m_nShadowXDistance(0)
        ,m_nShadowYDistance(0)
        ,m_nShadowColor(0)
        ,m_nShadowTransparence(0)
        ,m_nContrast(0)
        ,m_nBrightness(0)
        ,m_eColorMode( drawing::ColorMode_STANDARD )
        ,m_bIsGraphic(false)
        ,m_bSizeProtected(false)
        ,m_bPositionProtected(false)
        ,m_bHidden(false)
        ,m_nShapeOptionType(0)
        ,m_rPositionOffsets(rPositionOffsets)
        ,m_rAligns(rAligns)
        ,m_rPositivePercentages(rPositivePercentages)
    {
    }

    void setXSize(sal_Int32 _nXSize)
    {
        m_nXSize = _nXSize;
        m_bXSizeValid = true;
    }

    sal_uInt32 getXSize() const
    {
        return m_nXSize;
    }

    bool isXSizeValid() const
    {
        return m_bXSizeValid;
    }

    void setYSize(sal_Int32 _nYSize)
    {
        m_nYSize = _nYSize;
        m_bYSizeValid = true;
    }

    sal_uInt32 getYSize() const
    {
        return m_nYSize;
    }

    bool isYSizeValid() const
    {
        return m_bYSizeValid;
    }

    void applyMargins(const uno::Reference< beans::XPropertySet >& xGraphicObjectProperties) const
    {
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_LEFT_MARGIN ), uno::Any(m_nLeftMargin));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_RIGHT_MARGIN ), uno::Any(m_nRightMargin));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_TOP_MARGIN ), uno::Any(m_nTopMargin));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_BOTTOM_MARGIN ), uno::Any(m_nBottomMargin));
    }

    void applyPosition(const uno::Reference< beans::XPropertySet >& xGraphicObjectProperties) const
    {
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HORI_ORIENT          ),
                uno::Any(m_nHoriOrient));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_VERT_ORIENT          ),
                uno::Any(m_nVertOrient));
    }

    void applyRelativePosition(const uno::Reference< beans::XPropertySet >& xGraphicObjectProperties, bool bRelativeOnly = false) const
    {
        if (!bRelativeOnly)
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HORI_ORIENT_POSITION),
                                                       uno::Any(m_nLeftPosition));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HORI_ORIENT_RELATION ),
                uno::Any(m_nHoriRelation));
        xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_PAGE_TOGGLE),
                                                   uno::Any(m_bPageToggle));
        if (!bRelativeOnly)
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_VERT_ORIENT_POSITION),
                                                       uno::Any(m_nTopPosition));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_VERT_ORIENT_RELATION ),
                uno::Any(m_nVertRelation));
    }

    void applyZOrder(uno::Reference<beans::XPropertySet> const & xGraphicObjectProperties) const
    {
        std::optional<sal_Int32> oZOrder = m_oZOrder;
        if (m_rGraphicImportType == GraphicImportType::IMPORT_AS_DETECTED_INLINE
            && !m_rDomainMapper.IsInShape())
        {
            oZOrder = SAL_MIN_INT32;
        }
        else if (!oZOrder)
            return;
        else
        {
            // tdf#120760 Send objects with behinddoc=true to the back.

            // zOrder can be defined either by z-index or by relativeHeight.
            // z-index indicates background with a negative value,
            // while relativeHeight indicates background with BehindDoc = true.
            //
            // In general, all z-index-defined shapes appear on top of relativeHeight graphics
            // regardless of the value.
            // So we have to try to put all relativeHeights as far back as possible,
            // and this has already partially happened because they were already made to be negative
            // but now the behindDoc relativeHeights need to be forced to the very back.
            //
            // Subtract even more so behindDoc relativeHeights will be behind
            // foreground relativeHeights and also behind all of the negative z-indexes
            // (especially needed for IsInHeaderFooter, as EVERYTHING is forced to the background).
            //
            // relativeHeight already removed 0x1E00 0000, so can subtract another 0x6200 0000
            const bool bBehindText = m_bBehindDoc && !m_bOpaque;
            if (bBehindText)
                oZOrder = *oZOrder - 0x62000000;
        }

        // TODO: it is possible that RTF has been wrong all along as well. Always true here?
        const bool bLastDuplicateWins(!m_rDomainMapper.IsRTFImport()
            || m_rGraphicImportType == GraphicImportType::IMPORT_AS_DETECTED_INLINE);

        GraphicZOrderHelper& rZOrderHelper = m_rDomainMapper.graphicZOrderHelper();
        xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_Z_ORDER),
            uno::Any(rZOrderHelper.findZOrder(*oZOrder, bLastDuplicateWins)));
        rZOrderHelper.addItem(xGraphicObjectProperties, *oZOrder);
    }

    void applyName(uno::Reference<beans::XPropertySet> const & xGraphicObjectProperties) const
    {
        try
        {
            if (!m_sName.isEmpty())
            {
                uno::Reference<container::XNamed> const xNamed(xGraphicObjectProperties, uno::UNO_QUERY_THROW);
                xNamed->setName(m_sName);
            }
            // else: name is automatically generated by SwDoc::MakeFlySection_()

            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_DESCRIPTION ),
                uno::Any( m_sAlternativeText ));
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_TITLE ),
                uno::Any( m_title ));
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("writerfilter", "failed");
        }
    }

    void applyHyperlink(uno::Reference<beans::XPropertySet> const & xShapeProps, bool bIsShape)
    {
        // Graphic objects have a different hyperlink prop than shapes
        auto aHyperlinkProp = bIsShape ? PROP_HYPERLINK : PROP_HYPER_LINK_U_R_L;
        if (!m_sHyperlinkURL.isEmpty())
        {
            xShapeProps->setPropertyValue(
                getPropertyName(aHyperlinkProp), uno::Any(m_sHyperlinkURL));
        }
    }

    /// Getter for m_aInteropGrabBag, but also merges in the values from other members if they are set.
    comphelper::SequenceAsHashMap const & getInteropGrabBag()
    {
        comphelper::SequenceAsHashMap aEffectExtent;
        if (m_oEffectExtentLeft)
            aEffectExtent["l"] <<= *m_oEffectExtentLeft;
        if (m_oEffectExtentTop)
            aEffectExtent["t"] <<= *m_oEffectExtentTop;
        if (m_oEffectExtentRight)
            aEffectExtent["r"] <<= *m_oEffectExtentRight;
        if (m_oEffectExtentBottom)
            aEffectExtent["b"] <<= *m_oEffectExtentBottom;
        if (!aEffectExtent.empty())
            m_aInteropGrabBag["CT_EffectExtent"] <<= aEffectExtent.getAsConstPropertyValueList();
        return m_aInteropGrabBag;
    }
};

GraphicImport::GraphicImport(uno::Reference<uno::XComponentContext> xComponentContext,
                             uno::Reference<lang::XMultiServiceFactory> xTextFactory,
                             DomainMapper& rDMapper,
                             GraphicImportType & rImportType,
                             std::pair<OUString, OUString>& rPositionOffsets,
                             std::pair<OUString, OUString>& rAligns,
                             std::queue<OUString>& rPositivePercentages)
: LoggedProperties("GraphicImport")
, LoggedTable("GraphicImport")
, LoggedStream("GraphicImport")
, m_pImpl(new GraphicImport_Impl(rImportType, rDMapper, rPositionOffsets, rAligns, rPositivePercentages))
, m_xComponentContext(std::move(xComponentContext))
, m_xTextFactory(std::move(xTextFactory))
{
}

GraphicImport::~GraphicImport()
{
}

com::sun::star::awt::Point GraphicImport::GetGraphicObjectPosition() const
{
    return (com::sun::star::awt::Point(m_pImpl->m_nLeftPosition, m_pImpl->m_nTopPosition));
}

bool GraphicImport::GetLayoutInCell() const
{
    return m_pImpl->m_bLayoutInCell;
}

void GraphicImport::handleWrapTextValue(sal_uInt32 nVal)
{
    switch (nVal)
    {
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_bothSides:
        m_pImpl->m_nWrap = text::WrapTextMode_PARALLEL;
        break;
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_left:
        m_pImpl->m_nWrap = text::WrapTextMode_LEFT;
        break;
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_right:
        m_pImpl->m_nWrap = text::WrapTextMode_RIGHT;
        break;
    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_WrapText_largest:
        m_pImpl->m_nWrap = text::WrapTextMode_DYNAMIC;
        break;
    default:;
    }
}

void GraphicImport::putPropertyToFrameGrabBag( const OUString& sPropertyName, const uno::Any& aPropertyValue )
{
    beans::PropertyValue aProperty;
    aProperty.Name = sPropertyName;
    aProperty.Value = aPropertyValue;

    if (!m_xShape.is())
        return;

    uno::Reference< beans::XPropertySet > xSet(m_xShape, uno::UNO_QUERY_THROW);

    uno::Reference< beans::XPropertySetInfo > xSetInfo(xSet->getPropertySetInfo());
    if (!xSetInfo.is())
        return;

    OUString aGrabBagPropName;
    uno::Reference<lang::XServiceInfo> xServiceInfo(m_xShape, uno::UNO_QUERY_THROW);
    if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        aGrabBagPropName = "FrameInteropGrabBag";
    else
        aGrabBagPropName = "InteropGrabBag";

    if (xSetInfo->hasPropertyByName(aGrabBagPropName))
    {
        //Add pProperty to the end of the Sequence for aGrabBagPropName
        uno::Sequence<beans::PropertyValue> aTmp;
        xSet->getPropertyValue(aGrabBagPropName) >>= aTmp;
        std::vector<beans::PropertyValue> aGrabBag(comphelper::sequenceToContainer<std::vector<beans::PropertyValue> >(aTmp));
        aGrabBag.push_back(aProperty);

        xSet->setPropertyValue(aGrabBagPropName, uno::Any(comphelper::containerToSequence(aGrabBag)));
    }
}

static bool lcl_bHasGroupSlantedChild(const SdrObject* pObj)
{
    // Returns true, if a child object differs more than 0.02deg from horizontal or vertical.
    // Because lines sometimes are imported as customshapes, a horizontal or vertical line
    // might not have exactly 0, 90, 180, or 270 degree as rotate angle.
    if (!pObj)
        return false;
    if (!pObj->IsGroupObject())
        return false;
    SdrObjList* pSubList = pObj->GetSubList();
    if (!pSubList)
        return false;
    SdrObjListIter aIterator(pSubList, SdrIterMode::DeepNoGroups);
    while (aIterator.IsMore())
    {
        const SdrObject* pSubObj = aIterator.Next();
        const Degree100 nRotateAngle = NormAngle36000(pSubObj->GetRotateAngle());
        const sal_uInt16 nRot = nRotateAngle.get();
        if ((3 < nRot && nRot < 8997) || (9003 < nRot && nRot < 17997)
            || (18003 < nRot && nRot < 26997) || (27003 < nRot && nRot < 35997))
            return true;
    }
    return false;
}

void GraphicImport::lcl_correctWord2007EffectExtent(const sal_Int32 nMSOAngle)
{
    // Word versions older than 14 do not swap width and height (see lcl_doMSOWidthHeightSwap)
    // and therefore generate different effectExtent. We correct them here.
    sal_Int16 nAngleDeg = (nMSOAngle / 60000) % 180;
    if (nAngleDeg < 45 || nAngleDeg >= 135)
        return;

    sal_Int32 nDiff = o3tl::convert(
        (double(m_pImpl->getXSize()) - double(m_pImpl->getYSize())) / 2.0,
        o3tl::Length::mm100, o3tl::Length::emu);
    if (m_pImpl->m_oEffectExtentLeft)
        *m_pImpl->m_oEffectExtentLeft += nDiff;
    if (m_pImpl->m_oEffectExtentRight)
        *m_pImpl->m_oEffectExtentRight += nDiff;
    if (m_pImpl->m_oEffectExtentTop)
        *m_pImpl->m_oEffectExtentTop -= nDiff;
    if (m_pImpl->m_oEffectExtentBottom)
        *m_pImpl->m_oEffectExtentBottom -= nDiff;
}

static void lcl_doMSOWidthHeightSwap(awt::Point& rLeftTop, awt::Size& rSize,
                                       const sal_Int32 nMSOAngle)
{
    if (nMSOAngle == 0)
        return;
    // convert nMSOAngle to degree in [0°,180°[
    sal_Int16 nAngleDeg = (nMSOAngle / 60000) % 180;
    if (nAngleDeg >= 45 && nAngleDeg < 135)
    {
        // keep center of rectangle given in rLeftTop and rSize
        sal_Int32 aTemp = rSize.Width - rSize.Height;
        rLeftTop.X += aTemp / 2;
        rLeftTop.Y -= aTemp / 2;
        std::swap(rSize.Width, rSize.Height);
    }
    return;
}

void GraphicImport::lcl_expandRectangleByEffectExtent(awt::Point& rLeftTop, awt::Size& rSize)
{
    sal_Int32 nEffectExtent = (m_pImpl->m_oEffectExtentLeft)
                                  ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentLeft)
                                  : 0;
    rLeftTop.X -= nEffectExtent;
    rSize.Width += nEffectExtent;
    nEffectExtent = (m_pImpl->m_oEffectExtentRight)
                        ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentRight)
                        : 0;
    rSize.Width += nEffectExtent;
    nEffectExtent = (m_pImpl->m_oEffectExtentTop)
                        ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentTop)
                        : 0;
    rLeftTop.Y -= nEffectExtent;
    rSize.Height += nEffectExtent;
    nEffectExtent = (m_pImpl->m_oEffectExtentBottom)
                        ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentBottom)
                        : 0;
    rSize.Height += nEffectExtent;
}

void GraphicImport::lcl_attribute(Id nName, Value& rValue)
{
    sal_Int32 nIntValue = rValue.getInt();
    switch( nName )
    {
        case NS_ooxml::LN_OfficeArtExtension_Decorative_val:
            m_pImpl->m_bDecorative = true;
        break;
        case NS_ooxml::LN_CT_Hyperlink_URL:
            m_pImpl->m_sHyperlinkURL = rValue.getString();
        break;
        case NS_ooxml::LN_blip: //the binary graphic data in a shape
            {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rValue.getProperties();
            if( pProperties )
            {
                pProperties->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_payload :
        {
            writerfilter::Reference<BinaryObj>::Pointer_t pPictureData = rValue.getBinary();
            if( pPictureData )
                pPictureData->resolve(*this);
        }
        break;

        //border properties
        case NS_ooxml::LN_CT_Border_sz:
            m_pImpl->m_aBorders[BORDER_TOP].nLineWidth = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_val:
            //graphic borders don't support different line types
        break;
        case NS_ooxml::LN_CT_Border_space:
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            m_pImpl->m_aBorders[BORDER_TOP].bHasShadow = nIntValue != 0;
        break;
        case NS_ooxml::LN_CT_Border_frame:
            break;
        case NS_ooxml::LN_CT_PositiveSize2D_cx:
        case NS_ooxml::LN_CT_PositiveSize2D_cy:
        {
            sal_Int32 nDim = oox::drawingml::convertEmuToHmm(nIntValue);
            // drawingML equivalent of oox::vml::ShapeType::getAbsRectangle():
            // make sure a shape isn't hidden implicitly just because it has
            // zero height or width.
            if (nDim == 0)
                nDim = 1;

            if( nName == NS_ooxml::LN_CT_PositiveSize2D_cx )
                m_pImpl->setXSize(nDim);
            else
                m_pImpl->setYSize(nDim);
        }
        break;
        case NS_ooxml::LN_CT_EffectExtent_l:
            m_pImpl->m_oEffectExtentLeft = nIntValue;
            break;
        case NS_ooxml::LN_CT_EffectExtent_t:
            m_pImpl->m_oEffectExtentTop = nIntValue;
            break;
        case NS_ooxml::LN_CT_EffectExtent_r:
            m_pImpl->m_oEffectExtentRight = nIntValue;
            break;
        case NS_ooxml::LN_CT_EffectExtent_b:
            m_pImpl->m_oEffectExtentBottom = nIntValue;
            break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_id:
            //id of the object - ignored
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_name:
            //name of the object
            m_pImpl->m_sName = rValue.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_descr:
            //alternative text
            m_pImpl->m_sAlternativeText = rValue.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_title:
            //alternative text
            m_pImpl->m_title = rValue.getString();
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_hidden:
            m_pImpl->m_bHidden = (nIntValue == 1);
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noChangeAspect:
            //disallow aspect ratio change - ignored
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noMove:
            m_pImpl->m_bPositionProtected = true;
        break;
        case NS_ooxml::LN_CT_GraphicalObjectFrameLocking_noResize:
            m_pImpl->m_bSizeProtected = true;
        break;
        case NS_ooxml::LN_CT_Anchor_distT:
        case NS_ooxml::LN_CT_Anchor_distB:
        case NS_ooxml::LN_CT_Anchor_distL:
        case NS_ooxml::LN_CT_Anchor_distR:
        {
            m_pImpl->m_nShapeOptionType = nName;
            ProcessShapeOptions(rValue);
        }
        break;
        case NS_ooxml::LN_CT_Anchor_simplePos_attr:
            m_pImpl->m_bUseSimplePos = nIntValue > 0;
        break;
        case NS_ooxml::LN_CT_Anchor_relativeHeight: // unsigned content
        {
            // undocumented - based on testing: both 0 and 1 are equivalent to the maximum 503316479
            const sal_Int32 nMaxAllowed = 0x1DFFFFFF;
            if (nIntValue < 2 || nIntValue > nMaxAllowed)
                m_pImpl->m_oZOrder = nMaxAllowed;
            else
                m_pImpl->m_oZOrder = nIntValue;

            // all relativeHeight objects (i.e. DOCX graphics that use GraphicImport),
            // no matter how high their value, are below the lowest z-index shape (in same layer)
            // so emulate that by pretending that they are below text (in the hell-layer).
            // Please be assured that this does not actually place it in the hell-layer.
            m_pImpl->m_oZOrder = *m_pImpl->m_oZOrder - (nMaxAllowed + 1);
        }
        break;
        case NS_ooxml::LN_CT_Anchor_behindDoc:
            if (nIntValue > 0)
            {
                m_pImpl->m_bOpaque = false;
                m_pImpl->m_bBehindDoc = true;
            }
        break;
        case NS_ooxml::LN_CT_Anchor_locked:
        break;
        case NS_ooxml::LN_CT_Anchor_layoutInCell:
            // Starting in MSO 2013, anchors are ALWAYS considered to be laid out in table cell.
            m_pImpl->m_bCompatForcedLayoutInCell = !nIntValue
                && m_pImpl->m_rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() > 14
                && m_pImpl->m_rDomainMapper.IsInTable();
            m_pImpl->m_bLayoutInCell = m_pImpl->m_bCompatForcedLayoutInCell || nIntValue;
        break;
        case NS_ooxml::LN_CT_Anchor_hidden:
        break;
        case NS_ooxml::LN_CT_Anchor_allowOverlap:
            m_pImpl->m_bAllowOverlap = nIntValue != 0;
            break;
        case NS_ooxml::LN_CT_Anchor_wp14_anchorId:
        case NS_ooxml::LN_CT_Inline_wp14_anchorId:
        {
            OUStringBuffer aBuffer = OUString::number(nIntValue, 16);
            OUStringBuffer aString;
            comphelper::string::padToLength(aString, 8 - aBuffer.getLength(), '0');
            aString.append(aBuffer.getStr());
            m_pImpl->m_sAnchorId = aString.makeStringAndClear().toAsciiUpperCase();
        }
        break;
        case NS_ooxml::LN_CT_Point2D_x:
            m_pImpl->m_nLeftPosition = ConversionHelper::convertTwipToMM100(nIntValue);
            m_pImpl->m_nHoriRelation = text::RelOrientation::PAGE_FRAME;
            m_pImpl->m_nHoriOrient = text::HoriOrientation::NONE;
        break;
        case NS_ooxml::LN_CT_Point2D_y:
            m_pImpl->m_nTopPosition = ConversionHelper::convertTwipToMM100(nIntValue);
            m_pImpl->m_nVertRelation = text::RelOrientation::PAGE_FRAME;
            m_pImpl->m_nVertOrient = text::VertOrientation::NONE;
        break;
        case NS_ooxml::LN_CT_WrapTight_wrapText:
            m_pImpl->m_bContour = true;
            m_pImpl->m_bContourOutside = true;

            handleWrapTextValue(rValue.getInt());

            break;
        case NS_ooxml::LN_CT_WrapThrough_wrapText:
            m_pImpl->m_bContour = true;
            m_pImpl->m_bContourOutside = false;

            handleWrapTextValue(rValue.getInt());

            break;
        case NS_ooxml::LN_CT_WrapSquare_wrapText:
            handleWrapTextValue(rValue.getInt());
            break;
        case NS_ooxml::LN_CT_BlipFillProperties_srcRect:
            m_pImpl->m_oCrop.emplace(rValue.getAny().get<text::GraphicCrop>());
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

                        uno::Reference<graphic::XGraphic> xGraphic;
                        xShapeProps->getPropertyValue("Graphic") >>= xGraphic;

                        sal_Int32 nRotation = 0;
                        xShapeProps->getPropertyValue("RotateAngle") >>= nRotation;

                        css::beans::PropertyValues aGrabBag;
                        xShapeProps->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                        // if the shape contains effects in the grab bag, we should not transform it
                        // in a XTextContent so those effects can be preserved
                        bool bContainsEffects = std::any_of(std::cbegin(aGrabBag), std::cend(aGrabBag), [](const auto& rProp) {
                            return rProp.Name == "EffectProperties"
                                || rProp.Name == "3DEffectProperties"
                                || rProp.Name == "ArtisticEffectProperties";
                        });

                        xShapeProps->getPropertyValue("Shadow") >>= m_pImpl->m_bShadow;
                        if (m_pImpl->m_bShadow)
                        {
                            xShapeProps->getPropertyValue("ShadowXDistance") >>= m_pImpl->m_nShadowXDistance;
                            xShapeProps->getPropertyValue("ShadowYDistance") >>= m_pImpl->m_nShadowYDistance;
                            xShapeProps->getPropertyValue("ShadowColor") >>= m_pImpl->m_nShadowColor;
                            xShapeProps->getPropertyValue("ShadowTransparence") >>= m_pImpl->m_nShadowTransparence;
                        }

                        xShapeProps->getPropertyValue("GraphicColorMode") >>= m_pImpl->m_eColorMode;
                        xShapeProps->getPropertyValue("AdjustLuminance") >>= m_pImpl->m_nBrightness;
                        xShapeProps->getPropertyValue("AdjustContrast") >>= m_pImpl->m_nContrast;

                        // fdo#70457: transform XShape into a SwXTextGraphicObject only if there's no rotation
                        if ( nRotation == 0 && !bContainsEffects )
                            m_xGraphicObject = createGraphicObject( xGraphic, xShapeProps );

                        bUseShape = !m_xGraphicObject.is( );

                        if ( !bUseShape )
                        {
                            // Define the object size
                            uno::Reference< beans::XPropertySet > xGraphProps( m_xGraphicObject,
                                    uno::UNO_QUERY );
                            awt::Size aSize = xShape->getSize( );
                            xGraphProps->setPropertyValue("Height",
                                   uno::Any( aSize.Height ) );
                            xGraphProps->setPropertyValue("Width",
                                   uno::Any( aSize.Width ) );

                            text::GraphicCrop aGraphicCrop( 0, 0, 0, 0 );
                            uno::Reference< beans::XPropertySet > xSourceGraphProps( xShape, uno::UNO_QUERY );
                            uno::Any aAny = xSourceGraphProps->getPropertyValue("GraphicCrop");
                            if (m_pImpl->m_oCrop)
                            {   // RTF: RTFValue from resolvePict()
                                xGraphProps->setPropertyValue("GraphicCrop",
                                        uno::Any(*m_pImpl->m_oCrop));
                            }
                            else if (aAny >>= aGraphicCrop)
                            {   // DOCX: imported in oox BlipFillContext
                                xGraphProps->setPropertyValue("GraphicCrop",
                                    uno::Any( aGraphicCrop ) );
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


                        xShapeProps->setPropertyValue
                            (getPropertyName(PROP_ANCHOR_TYPE),
                             uno::Any
                             (text::TextContentAnchorType_AS_CHARACTER));

                        // In Word, if a shape is anchored inline, that
                        // excludes being in the background.
                        xShapeProps->setPropertyValue("Opaque", uno::Any(true));

                        uno::Reference<lang::XServiceInfo> xServiceInfo(m_xShape, uno::UNO_QUERY_THROW);

                        // TextFrames can't be rotated. But for anything else,
                        // make sure that setting size doesn't affect rotation,
                        // that would not match Word's definition of rotation.
                        bool bKeepRotation = false;
                        if (!xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
                        {
                            bKeepRotation = true;
                            xShapeProps->setPropertyValue
                                (getPropertyName(PROP_TEXT_RANGE),
                                 uno::Any
                                 (m_pImpl->m_rDomainMapper.GetCurrentTextRange()));
                        }

                        awt::Size aSize(m_xShape->getSize());

                        // One purpose of the next part is, to set the logic rectangle of the SdrObject
                        // to nXSize and nYSize from import. That doesn't work for groups or lines,
                        // because they do not have a logic rectangle and m_xShape->getSize and
                        // m_xShape->setSize would work on the snap rectangle. In case a shape is
                        // rotated, non-uniform scaling the snap rectangle will introduce shearing on
                        // the shape. In case group or line is rotated, nXSize and nYSize contain the
                        // unrotated size from oox. The rotation is already incorporated into group
                        // children and line points. We must not scale them to unrotated size. Exclude
                        // those shapes here.

                        // Get MSO rotation angle. GetRotateAngle from SdrObject is not suitable
                        // here, because it returns the rotate angle of the first child for groups
                        // and slope angle for lines, even if line or group had not been rotated.
                        // Import in oox has put the rotation from oox file into InteropGrabBag.
                        comphelper::SequenceAsHashMap aInteropGrabBag(xShapeProps->getPropertyValue("InteropGrabBag"));
                        sal_Int32 nOOXAngle(0);
                        aInteropGrabBag.getValue("mso-rotation-angle") >>= nOOXAngle; // 1/60000 deg
                        // tdf#143455: A diagram is imported as group, but has no valid object list
                        // and contour wrap is different to Word. As workaround diagrams are excluded
                        // here in various places.
                        const SdrObject* pDiagramCandidate(SdrObject::getSdrObjectFromXShape(m_xShape));
                        const bool bIsDiagram(nullptr != pDiagramCandidate && pDiagramCandidate->isDiagram());
                        // tdf#143476: A lockedCanvas (Word2007) is imported as group, but has not
                        // got size and position. Values from m_Impl has to be used.
                        bool bIsLockedCanvas(false);
                        aInteropGrabBag.getValue("LockedCanvas") >>= bIsLockedCanvas;
                        bool bIsWordprocessingCanvas(false);
                        aInteropGrabBag.getValue("WordprocessingCanvas") >>= bIsWordprocessingCanvas;
                        const bool bIsGroupOrLine = (xServiceInfo->supportsService("com.sun.star.drawing.GroupShape")
                            && !bIsDiagram && !bIsLockedCanvas && !bIsWordprocessingCanvas)
                            || xServiceInfo->supportsService("com.sun.star.drawing.LineShape");
                        SdrObject* pShape = SdrObject::getSdrObjectFromXShape(m_xShape);
                        if ((bIsGroupOrLine && !lcl_bHasGroupSlantedChild(pShape) && nOOXAngle == 0)
                            || !bIsGroupOrLine)
                        {
                            if (m_pImpl->isXSizeValid())
                                aSize.Width = m_pImpl->getXSize();
                            if (m_pImpl->isYSizeValid())
                                aSize.Height = m_pImpl->getYSize();
                        }

                        Degree100 nRotation;
                        if (bKeepRotation)
                        {
                            // Use internal API, getPropertyValue("RotateAngle")
                            // would use GetObjectRotation(), which is not what
                            // we want.
                            if (pShape)
                                nRotation = pShape->GetRotateAngle();
                        }

                        // tdf#157960: SdrEdgeObj::NbcResize would reset the adjustment values of
                        // connectors to default zero. Thus we do not resize in case of a group that
                        // represents a Word drawing canvas.
                        if (!bIsWordprocessingCanvas)
                            m_xShape->setSize(aSize);

                        if (bKeepRotation)
                        {
                            xShapeProps->setPropertyValue("RotateAngle", uno::Any(nRotation.get()));
                        }

                        m_pImpl->m_bIsGraphic = true;

                        if (!m_pImpl->m_sAnchorId.isEmpty())
                        {
                            putPropertyToFrameGrabBag("AnchorId", uno::Any(m_pImpl->m_sAnchorId));
                        }

                        // Calculate mso unrotated rectangle and its center, needed below
                        awt::Size aImportSize(m_xShape->getSize()); // here only fallback
                        if (m_pImpl->isXSizeValid())
                            aImportSize.Width = m_pImpl->getXSize(); // Hmm
                        if (m_pImpl->isYSizeValid())
                            aImportSize.Height = m_pImpl->getYSize(); // Hmm
                        const awt::Point aImportPosition(GetGraphicObjectPosition()); // Hmm
                        double fCentrumX = aImportPosition.X + aImportSize.Width / 2.0;
                        double fCentrumY = aImportPosition.Y + aImportSize.Height / 2.0;

                        // In case of group and lines, transformations are incorporated in the child
                        // shapes or points respectively in LO. MSO has rotation as separate property.
                        // The position refers to the unrotated rectangle of MSO. We need to adapt it
                        // to the left-top of the transformed shape.
                        awt::Size aLOSize(m_xShape->getSize()); // LO snap rectangle size in Hmm
                        if (bIsGroupOrLine  && !(m_pImpl->mpWrapPolygon))
                        {
                            // Set LO position. MSO rotation is done on shape center.
                            if(pShape && pShape->IsGroupObject())
                            {
                                tools::Rectangle aSnapRect = pShape->GetSnapRect(); // Twips
                                m_pImpl->m_nLeftPosition = ConversionHelper::convertTwipToMM100(aSnapRect.Left());
                                m_pImpl->m_nTopPosition = ConversionHelper::convertTwipToMM100(aSnapRect.Top());
                                aLOSize.Width = ConversionHelper::convertTwipToMM100(aSnapRect.getOpenWidth());
                                aLOSize.Height = ConversionHelper::convertTwipToMM100(aSnapRect.getOpenHeight());
                            }
                            else
                            {
                                m_pImpl->m_nLeftPosition = fCentrumX - aLOSize.Width / 2.0;
                                m_pImpl->m_nTopPosition = fCentrumY - aLOSize.Height / 2.0;
                            }
                            m_xShape->setPosition(GetGraphicObjectPosition());
                        }
                        // ToDo: Rotated shapes with position type "Alignment" (UI of Word) have
                        // wrong position. Word aligns the unrotated logic rectangle, LO the rotated
                        // snap rectangle.

                        // Margin correction

                        // tdf#143475: Word 2007 (vers 12) calculates effectExtent for rotated images
                        // based on the unrotated image without width-height-swap. We correct this to
                        // those values, which would be calculated if width-height-swap was used.
                        if (m_pImpl->m_rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() < 14
                            && xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape")
                            && nOOXAngle != 0)
                        {
                            lcl_correctWord2007EffectExtent(nOOXAngle);
                        }

                        if (m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_INLINE)
                        {
                            if (nOOXAngle == 0)
                            {
                                // EffectExtent contains all needed additional space, including fat
                                // stroke and shadow. Simple add it to the margins.
                                sal_Int32 nEffectExtent = (m_pImpl->m_oEffectExtentLeft)
                                    ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentLeft)
                                    : 0;
                                m_pImpl->m_nLeftMargin += nEffectExtent;
                                nEffectExtent = (m_pImpl->m_oEffectExtentRight)
                                    ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentRight) : 0;
                                m_pImpl->m_nRightMargin += nEffectExtent;
                                nEffectExtent = (m_pImpl->m_oEffectExtentTop)
                                    ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentTop) : 0;
                                m_pImpl->m_nTopMargin += nEffectExtent;
                                nEffectExtent = (m_pImpl->m_oEffectExtentBottom)
                                    ? oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentBottom) : 0;
                                m_pImpl->m_nBottomMargin += nEffectExtent;
                            }
                            else
                            {
                                // As of June 2021 LibreOffice uses an area, which is large enough to
                                // contain the rotated snap rectangle. MSO uses a smaller area, so
                                // that the rotated snap rectangle covers text.
                                awt::Point aMSOBaseLeftTop = aImportPosition;
                                awt::Size aMSOBaseSize = aImportSize;
                                lcl_doMSOWidthHeightSwap(aMSOBaseLeftTop, aMSOBaseSize, nOOXAngle);
                                lcl_expandRectangleByEffectExtent(aMSOBaseLeftTop, aMSOBaseSize);

                                // Get LO SnapRect from SdrObject if possible
                                awt::Rectangle aLOSnapRect;
                                // For case we have no SdrObject, initialize with values from m_pImpl
                                aLOSnapRect.X = m_pImpl->m_nLeftPosition;
                                aLOSnapRect.Y = m_pImpl->m_nTopPosition;
                                aLOSnapRect.Width = aLOSize.Width;
                                aLOSnapRect.Height = aLOSize.Height;
                                if (pShape)
                                {
                                    tools::Rectangle aSnapRect = pShape->GetSnapRect(); // Twip
                                    aLOSnapRect.X = ConversionHelper::convertTwipToMM100(aSnapRect.Left());
                                    aLOSnapRect.Y = ConversionHelper::convertTwipToMM100(aSnapRect.Top());
                                    aLOSnapRect.Width = ConversionHelper::convertTwipToMM100(aSnapRect.getOpenWidth());
                                    aLOSnapRect.Height = ConversionHelper::convertTwipToMM100(aSnapRect.getOpenHeight());
                                }

                                m_pImpl->m_nLeftMargin  += aLOSnapRect.X - aMSOBaseLeftTop.X;
                                m_pImpl->m_nRightMargin += aMSOBaseLeftTop.X + aMSOBaseSize.Width
                                                         - (aLOSnapRect.X + aLOSnapRect.Width);
                                m_pImpl->m_nTopMargin  += aLOSnapRect.Y - aMSOBaseLeftTop.Y;
                                m_pImpl->m_nBottomMargin += aMSOBaseLeftTop.Y + aMSOBaseSize.Height
                                                          - (aLOSnapRect.Y + aLOSnapRect.Height);
                                // tdf#141880 LibreOffice cannot handle negative vertical margins.
                                // Those cases are caught below at common place.
                            }
                        } // end IMPORT_AS_DETECTED_INLINE
                        else if ((m_pImpl->m_nWrap == text::WrapTextMode_PARALLEL
                                  || m_pImpl->m_nWrap == text::WrapTextMode_DYNAMIC
                                  || m_pImpl->m_nWrap == text::WrapTextMode_LEFT
                                  || m_pImpl->m_nWrap == text::WrapTextMode_RIGHT
                                  || m_pImpl->m_nWrap == text::WrapTextMode_NONE)
                                  && !(m_pImpl->mpWrapPolygon) && !bIsDiagram && !bIsWordprocessingCanvas)
                        {
                            // For wrap "Square" an area is defined around which the text wraps. MSO
                            // describes the area by a base rectangle and effectExtent. LO uses the
                            // shape bounding box and margins. We adapt the margins to get the same
                            // area as MSO.
                            awt::Point aMSOBaseLeftTop = aImportPosition;
                            awt::Size aMSOBaseSize = aImportSize;
                            lcl_doMSOWidthHeightSwap(aMSOBaseLeftTop, aMSOBaseSize, nOOXAngle);
                            lcl_expandRectangleByEffectExtent(aMSOBaseLeftTop, aMSOBaseSize);

                            // Get LO bound rectangle from SdrObject if possible
                            awt::Rectangle aLOBoundRect;
                            // For case we have no SdrObject, initialize with values from m_pImpl
                            aLOBoundRect.X = m_pImpl->m_nLeftPosition;
                            aLOBoundRect.Y = m_pImpl->m_nTopPosition;
                            aLOBoundRect.Width = aLOSize.Width;
                            aLOBoundRect.Height = aLOSize.Height;
                            if (pShape)
                            {
                                tools::Rectangle aBoundRect = pShape->GetCurrentBoundRect(); // Twip
                                aLOBoundRect.X = ConversionHelper::convertTwipToMM100(aBoundRect.Left());
                                aLOBoundRect.Y = ConversionHelper::convertTwipToMM100(aBoundRect.Top());
                                aLOBoundRect.Width = ConversionHelper::convertTwipToMM100(aBoundRect.getOpenWidth());
                                aLOBoundRect.Height = ConversionHelper::convertTwipToMM100(aBoundRect.getOpenHeight());
                            }

                            m_pImpl->m_nLeftMargin += aLOBoundRect.X - aMSOBaseLeftTop.X;
                            m_pImpl->m_nRightMargin += aMSOBaseLeftTop.X + aMSOBaseSize.Width
                                                     - (aLOBoundRect.X + aLOBoundRect.Width);
                            m_pImpl->m_nTopMargin += aLOBoundRect.Y - aMSOBaseLeftTop.Y;
                            m_pImpl->m_nBottomMargin += aMSOBaseLeftTop.Y + aMSOBaseSize.Height
                                                      - (aLOBoundRect.Y + aLOBoundRect.Height);
                        }
                        else if (m_pImpl->mpWrapPolygon && !bIsDiagram && !bIsWordprocessingCanvas)
                        {
                            // Word uses a wrap polygon, LibreOffice has no explicit wrap polygon
                            // but creates the wrap contour based on the shape geometry, without
                            // stroke width and shadow, but with rotation and flip. The concepts
                            // are not compatible. We approximate Word's rendering by setting
                            // wrap margins.

                            // Build a range from the wrap polygon from Word.
                            const drawing::PointSequenceSequence aWrapPolygon
                                = m_pImpl->mpWrapPolygon->getPointSequenceSequence();
                            basegfx::B2DPolyPolygon aB2DWrapPolyPolygon
                                = basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(
                                    aWrapPolygon);
                            // Wrap polygon values are relative to 0..21600|0..21600.
                            // Scale to shape size (in Hmm).
                            basegfx::B2DHomMatrix aMatrix = basegfx::utils::createScaleB2DHomMatrix(
                                aImportSize.Width / 21600.0, aImportSize.Height / 21600.0);
                            aB2DWrapPolyPolygon.transform(aMatrix);

                            // Shape geometry will be rotated, rotate wrap polygon too.
                            if (nOOXAngle != 0)
                            {
                                aMatrix = basegfx::utils::createRotateAroundPoint(
                                    aImportSize.Width / 2.0, aImportSize.Height / 2.0,
                                    basegfx::deg2rad<60000>(nOOXAngle));
                                aB2DWrapPolyPolygon.transform(aMatrix);
                            }
                            basegfx::B2DRange aB2DWrapRange = aB2DWrapPolyPolygon.getB2DRange();

                            // Build a range from shape geometry
                            basegfx::B2DRange aShapeRange;
                            if (pShape)
                            {
                                basegfx::B2DPolyPolygon aShapePolygon = pShape->TakeXorPoly(); // Twips
                                aMatrix = basegfx::utils::createScaleB2DHomMatrix(
                                    o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100),
                                    o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100));
                                aShapePolygon.transform(aMatrix);
                                // Wrap polygon treats left/top of shape as origin, shift shape polygon accordingly
                                aMatrix = basegfx::utils::createTranslateB2DHomMatrix(
                                    -aImportPosition.X, -aImportPosition.Y);
                                aShapePolygon.transform(aMatrix);
                                aShapeRange = aShapePolygon.getB2DRange();
                            }
                            else // can this happen?
                            {
                                aShapeRange
                                    = basegfx::B2DRange(0, 0, aImportSize.Width, aImportSize.Height);
                                if (nOOXAngle != 0)
                                {
                                    aMatrix = basegfx::utils::createRotateB2DHomMatrix(
                                        basegfx::deg2rad<60000>(nOOXAngle));
                                    aShapeRange.transform(aMatrix);
                                }
                            }

                            // Add difference between shape and wrap range to margin and remember
                            // difference in Twips for export.
                            comphelper::SequenceAsHashMap aAnchorDistDiff;

                            const double fTopDiff = aShapeRange.getMinY() - aB2DWrapRange.getMinY();
                            m_pImpl->m_nTopMargin += basegfx::fround(fTopDiff);
                            aAnchorDistDiff["distTDiff"] <<= basegfx::fround(
                                o3tl::convert(fTopDiff, o3tl::Length::mm100, o3tl::Length::twip));

                            const double fBottomDiff = aB2DWrapRange.getMaxY() - aShapeRange.getMaxY();
                            m_pImpl->m_nBottomMargin += basegfx::fround(fBottomDiff);
                            aAnchorDistDiff["distBDiff"] <<= basegfx::fround(
                                o3tl::convert(fBottomDiff, o3tl::Length::mm100, o3tl::Length::twip));

                            const double fLeftDiff = aShapeRange.getMinX() - aB2DWrapRange.getMinX();
                            m_pImpl->m_nLeftMargin += basegfx::fround(fLeftDiff);
                            aAnchorDistDiff["distLDiff"] <<= basegfx::fround(
                                o3tl::convert(fLeftDiff, o3tl::Length::mm100, o3tl::Length::twip));

                            const double fRightDiff = aB2DWrapRange.getMaxX() - aShapeRange.getMaxX();
                            m_pImpl->m_nRightMargin += basegfx::fround(fRightDiff);
                            aAnchorDistDiff["distRDiff"] <<= basegfx::fround(
                                o3tl::convert(fRightDiff, o3tl::Length::mm100, o3tl::Length::twip));

                            m_pImpl->m_aInteropGrabBag["AnchorDistDiff"]
                                <<= aAnchorDistDiff.getAsConstPropertyValueList();

                            // FixMe: tdf#141880. LibreOffice cannot handle negative horizontal margin in contour wrap
                            if (m_pImpl->m_nLeftMargin < 0)
                                m_pImpl->m_nLeftMargin = 0;
                            if (m_pImpl->m_nRightMargin < 0)
                                m_pImpl->m_nRightMargin = 0;
                        }
                        else if (!bIsDiagram && !bIsWordprocessingCanvas) // text::WrapTextMode_THROUGH
                        {
                            // Word writes and evaluates the effectExtent in case of position
                            // type 'Alignment' (UI). We move these values to margin to approximate
                            // Word's rendering.
                            if (m_pImpl->m_oEffectExtentLeft)
                            {
                                m_pImpl->m_nLeftMargin
                                    += oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentLeft);
                            }
                            if (m_pImpl->m_oEffectExtentTop)
                            {
                                m_pImpl->m_nTopMargin
                                    += oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentTop);
                            }
                            if (m_pImpl->m_oEffectExtentRight)
                            {
                                m_pImpl->m_nRightMargin
                                    += oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentRight);
                            }
                            if (m_pImpl->m_oEffectExtentBottom)
                            {
                                m_pImpl->m_nBottomMargin
                                    += oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentBottom);
                            }
                        }

                        // FixMe: tdf#141880 LibreOffice cannot handle negative vertical margins
                        // although they are allowed in ODF.
                        if (m_pImpl->m_nTopMargin < 0)
                            m_pImpl->m_nTopMargin = 0;
                        if (m_pImpl->m_nBottomMargin < 0)
                            m_pImpl->m_nBottomMargin = 0;
                    }

                    if (bUseShape && m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
                    {
                        // If we are here, this is a drawingML shape. For those, only dmapper (and not oox) knows the anchoring infos (just like for Writer pictures).
                        // But they aren't Writer pictures, either (which are already handled above).
                        uno::Reference< beans::XPropertySet > xShapeProps(m_xShape, uno::UNO_QUERY_THROW);

                        if (m_pImpl->m_nWrap == text::WrapTextMode_THROUGH && m_pImpl->m_nHoriRelation == text::RelOrientation::FRAME)
                        {
                            // text::RelOrientation::FRAME is OOXML's "column", which behaves as if
                            // layout-in-cell would be always off.
                            m_pImpl->m_bLayoutInCell = false;
                        }

                        // Anchored: Word only supports at-char in that case.
                        text::TextContentAnchorType eAnchorType = text::TextContentAnchorType_AT_CHARACTER;

                        if (m_pImpl->m_bHidden)
                        {
                            xShapeProps->setPropertyValue("Visible", uno::Any(false));
                            xShapeProps->setPropertyValue("Printable", uno::Any(false));
                        }

                        // Avoid setting AnchorType for TextBoxes till SwTextBoxHelper::syncProperty() doesn't handle transition.
                        bool bTextBox = false;
                        xShapeProps->getPropertyValue("TextBox") >>= bTextBox;

                        // The positioning change caused by LayoutInCell doesn't sync well
                        // in the text / frame duo. So the compatibility fix only correctly
                        // positions the frame and not the text currently.
                        // tdf#135943: Instead of half-fixing and making a complete mess,
                        // just avoid until layout's repositioning is sync'd to the text frame.
                        if (m_pImpl->m_bLayoutInCell && bTextBox)
                            m_pImpl->m_bLayoutInCell = !m_pImpl->m_bCompatForcedLayoutInCell;

                        xShapeProps->setPropertyValue("AnchorType", uno::Any(eAnchorType));

                        if (m_pImpl->m_nVertRelation == text::RelOrientation::TEXT_LINE)
                        {
                            // Word's "line" is "below the bottom of the line", our TEXT_LINE is
                            // "towards top, from the bottom of the line", so invert the vertical
                            // position.
                            awt::Point aPoint = xShape->getPosition();
                            aPoint.Y *= -1;
                            xShape->setPosition(aPoint);
                        }

                        if (m_pImpl->m_bLayoutInCell && bTextBox && m_pImpl->m_rDomainMapper.IsInTable()
                            && m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_FRAME)
                            m_pImpl->m_nHoriRelation = text::RelOrientation::FRAME;
                        if(m_pImpl->m_rDomainMapper.IsInTable())
                            xShapeProps->setPropertyValue(getPropertyName(PROP_FOLLOW_TEXT_FLOW),
                                uno::Any(m_pImpl->m_bLayoutInCell));
                        //only the position orientation is handled in applyPosition()
                        m_pImpl->applyPosition(xShapeProps);

                        uno::Reference<lang::XServiceInfo> xServiceInfo(m_xShape, uno::UNO_QUERY_THROW);
                        if (xServiceInfo->supportsService("com.sun.star.drawing.GroupShape") ||
                                xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
                        {
                            // You would expect that position and rotation are
                            // independent, but they are not. Till we are not
                            // there yet to handle all scaling, translation and
                            // rotation with a single transformation matrix,
                            // make sure there is no graphic rotation set when we set
                            // the position.
                            sal_Int32 nRotation = 0;
                            if (xServiceInfo->supportsService("com.sun.star.drawing.GraphicObjectShape"))
                            {
                                xShapeProps->getPropertyValue("RotateAngle") >>= nRotation;
                            }
                            if (nRotation)
                                xShapeProps->setPropertyValue("RotateAngle", uno::Any(sal_Int32(0)));

                            // Position of the groupshape should be set after children have been added.
                            // Long-term we should get rid of positioning group
                            // shapes, though. Do it for top-level ones with
                            // absolute page position as a start.
                            // fdo#80555: also set position for graphic shapes here
                            if (!isTopGroupObj(m_xShape)
                                || m_pImpl->m_nHoriRelation != text::RelOrientation::PAGE_FRAME
                                || m_pImpl->m_nVertRelation != text::RelOrientation::PAGE_FRAME)
                                m_xShape->setPosition(
                                    awt::Point(m_pImpl->m_nLeftPosition, m_pImpl->m_nTopPosition));

                            if (nRotation)
                                xShapeProps->setPropertyValue("RotateAngle", uno::Any(nRotation));
                        }


                        m_pImpl->applyRelativePosition(xShapeProps, /*bRelativeOnly=*/true);

                        xShapeProps->setPropertyValue("SurroundContour", uno::Any(m_pImpl->m_bContour));
                        xShapeProps->setPropertyValue("ContourOutside", uno::Any(m_pImpl->m_bContourOutside));
                        m_pImpl->applyMargins(xShapeProps);
                        xShapeProps->setPropertyValue("Opaque", uno::Any(m_pImpl->m_bOpaque));
                        xShapeProps->setPropertyValue("Surround", uno::Any(static_cast<sal_Int32>(m_pImpl->m_nWrap)));
                        m_pImpl->applyZOrder(xShapeProps);
                        m_pImpl->applyName(xShapeProps);
                        m_pImpl->applyHyperlink(xShapeProps, bUseShape);
                        xShapeProps->setPropertyValue("AllowOverlap",
                                                      uno::Any(m_pImpl->m_bAllowOverlap));

                        // Get the grab-bag set by oox, merge with our one and then put it back.
                        comphelper::SequenceAsHashMap aInteropGrabBag(xShapeProps->getPropertyValue("InteropGrabBag"));
                        aInteropGrabBag.update(m_pImpl->getInteropGrabBag());
                        xShapeProps->setPropertyValue("InteropGrabBag", uno::Any(aInteropGrabBag.getAsConstPropertyValueList()));
                    }
                    else if (bUseShape && m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_INLINE)
                    {
                        uno::Reference< beans::XPropertySet > xShapeProps(m_xShape, uno::UNO_QUERY_THROW);
                        m_pImpl->applyMargins(xShapeProps);
                        m_pImpl->applyZOrder(xShapeProps);
                        m_pImpl->applyName(xShapeProps);
                        comphelper::SequenceAsHashMap aInteropGrabBag(xShapeProps->getPropertyValue("InteropGrabBag"));
                        aInteropGrabBag.update(m_pImpl->getInteropGrabBag());
                        xShapeProps->setPropertyValue("InteropGrabBag", uno::Any(aInteropGrabBag.getAsConstPropertyValueList()));
                    }
                }
            }
        break;
        case NS_ooxml::LN_CT_Inline_distT:
            m_pImpl->m_nTopMargin = 0;
        break;
        case NS_ooxml::LN_CT_Inline_distB:
            m_pImpl->m_nBottomMargin = 0;
        break;
        case NS_ooxml::LN_CT_Inline_distL:
            m_pImpl->m_nLeftMargin = 0;
        break;
        case NS_ooxml::LN_CT_Inline_distR:
            m_pImpl->m_nRightMargin = 0;
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
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::Any(text::RelOrientation::FRAME));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromH_leftMargin:
                case NS_ooxml::LN_ST_SizeRelFromH_outsideMargin:
                    if (m_xShape.is())
                    {
                        // Here we handle the relative size of the width of some shape.
                        // The size of the shape's width is going to be relative to the size of the left margin.
                        // E.g.: (left margin = 8 && relative size = 150%) -> width of some shape = 12.
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::Any(text::RelOrientation::PAGE_LEFT));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromH_rightMargin:
                case NS_ooxml::LN_ST_SizeRelFromH_insideMargin:
                    if (m_xShape.is())
                    {
                        // Same as the left margin above.
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::Any(text::RelOrientation::PAGE_RIGHT));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromH_page:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::Any(text::RelOrientation::PAGE_FRAME));
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
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::Any(text::RelOrientation::FRAME));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromV_page:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::Any(text::RelOrientation::PAGE_FRAME));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromV_topMargin:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::Any(text::RelOrientation::PAGE_PRINT_AREA));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromV_bottomMargin:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
                    }
                    break;
                default:
                    SAL_WARN("writerfilter", "GraphicImport::lcl_attribute: unhandled NS_ooxml::LN_CT_SizeRelV_relativeFrom value: " << nIntValue);
                    break;
                }
            }
            break;
        default:
#ifdef DBG_UTIL
            TagLogger::getInstance().element("unhandled");
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


void GraphicImport::ProcessShapeOptions(Value const & rValue)
{
    sal_Int32 nIntValue = rValue.getInt();
    switch( m_pImpl->m_nShapeOptionType )
    {
        case NS_ooxml::LN_CT_Anchor_distL:
            m_pImpl->m_nLeftMargin = nIntValue / 360;
            m_pImpl->m_nLeftMarginOrig = m_pImpl->m_nLeftMargin;
        break;
        case NS_ooxml::LN_CT_Anchor_distT:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->m_nTopMargin = nIntValue / 360;
        break;
        case NS_ooxml::LN_CT_Anchor_distR:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustLRWrapForWordMargins()
            m_pImpl->m_nRightMargin = nIntValue / 360;
        break;
        case NS_ooxml::LN_CT_Anchor_distB:
            //todo: changes have to be applied depending on the orientation, see SwWW8ImplReader::AdjustULWrapForWordMargins()
            m_pImpl->m_nBottomMargin = nIntValue / 360;
        break;
        default:
            OSL_FAIL( "shape option unsupported?");
    }
}


void GraphicImport::lcl_sprm(Sprm& rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();

    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Inline_extent:
        case NS_ooxml::LN_CT_Inline_effectExtent:
        case NS_ooxml::LN_CT_Inline_docPr:
        case NS_ooxml::LN_CT_Inline_cNvGraphicFramePr:
        case NS_ooxml::LN_CT_NonVisualGraphicFrameProperties_graphicFrameLocks:
        case NS_ooxml::LN_CT_Inline_a_graphic:
        case NS_ooxml::LN_CT_Anchor_simplePos_elem:
        case NS_ooxml::LN_CT_Anchor_extent:
        case NS_ooxml::LN_CT_Anchor_effectExtent:
        case NS_ooxml::LN_EG_WrapType_wrapSquare:
        case NS_ooxml::LN_EG_WrapType_wrapTight:
        case NS_ooxml::LN_EG_WrapType_wrapThrough:
        case NS_ooxml::LN_CT_Anchor_docPr:
        case NS_ooxml::LN_CT_NonVisualDrawingProps_extLst:
        case NS_ooxml::LN_CT_Anchor_cNvGraphicFramePr:
        case NS_ooxml::LN_CT_Anchor_a_graphic:
        case NS_ooxml::LN_CT_WrapPath_start:
        case NS_ooxml::LN_CT_WrapPath_lineTo:
        case NS_ooxml::LN_graphic_graphic:
        case NS_ooxml::LN_pic_pic:
        case NS_ooxml::LN_dgm_relIds:
        case NS_ooxml::LN_lc_lockedCanvas:
        case NS_ooxml::LN_c_chart:
        case NS_ooxml::LN_wps_wsp:
        case NS_ooxml::LN_wpg_wgp:
        case NS_ooxml::LN_sizeRelH_sizeRelH:
        case NS_ooxml::LN_sizeRelV_sizeRelV:
        case NS_ooxml::LN_hlinkClick_hlinkClick:
        case NS_ooxml::LN_wpc_wpc:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                pProperties->resolve(*this);
            }

            // We'll map these to PARALLEL, save the original wrap type.
            if (nSprmId == NS_ooxml::LN_EG_WrapType_wrapTight)
                m_pImpl->m_aInteropGrabBag["EG_WrapType"] <<= OUString("wrapTight");
            else if (nSprmId == NS_ooxml::LN_EG_WrapType_wrapThrough)
                m_pImpl->m_aInteropGrabBag["EG_WrapType"] <<= OUString("wrapThrough");

            switch (nSprmId)
            {
                case NS_ooxml::LN_EG_WrapType_wrapSquare:
                case NS_ooxml::LN_EG_WrapType_wrapThrough:
                case NS_ooxml::LN_EG_WrapType_wrapTight:
                {
                    // tdf#137850: Word >= 2013 seems to ignore bBehindDoc except for wrapNone, but older versions honour it.
                    if (m_pImpl->m_bBehindDoc && m_pImpl->m_rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() > 14)
                        m_pImpl->m_bOpaque = true;
                }
                break;
            }

        }
        break;
        case NS_ooxml::LN_CT_WrapTight_wrapPolygon:
        case NS_ooxml::LN_CT_WrapThrough_wrapPolygon:
            {
                WrapPolygonHandler aHandler;

                resolveSprmProps(aHandler, rSprm);

                m_pImpl->mpWrapPolygon = aHandler.getPolygon();

                // Save the wrap path in case we can't handle it natively: drawinglayer shapes, TextFrames.
                m_pImpl->m_aInteropGrabBag["CT_WrapPath"] <<= m_pImpl->mpWrapPolygon->getPointSequenceSequence();
            }
            break;
        case NS_ooxml::LN_CT_Anchor_positionH:
        {
            // Use a special handler for the positioning
            auto pHandler = std::make_shared<PositionHandler>( m_pImpl->m_rPositionOffsets, m_pImpl->m_rAligns );
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                pProperties->resolve( *pHandler );
                if( !m_pImpl->m_bUseSimplePos )
                {
                    m_pImpl->m_nHoriRelation = pHandler->relation();
                    m_pImpl->m_bPageToggle = pHandler->GetPageToggle();
                    m_pImpl->m_nHoriOrient = pHandler->orientation();
                    m_pImpl->m_nLeftPosition = pHandler->position();

                    // Left adjustments: if horizontally aligned to left of margin, then remove the
                    // left wrapping.
                    if (m_pImpl->m_nHoriOrient == text::HoriOrientation::LEFT)
                    {
                        if (m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA)
                        {
                            m_pImpl->m_nLeftMargin = 0;
                        }
                    }
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_Anchor_positionV:
        {
            // Use a special handler for the positioning
            auto pHandler = std::make_shared<PositionHandler>( m_pImpl->m_rPositionOffsets, m_pImpl->m_rAligns);
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                pProperties->resolve( *pHandler );
                if( !m_pImpl->m_bUseSimplePos )
                {
                    m_pImpl->m_nVertRelation = pHandler->relation();
                    m_pImpl->m_nVertOrient = pHandler->orientation();
                    m_pImpl->m_nTopPosition = pHandler->position();
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_SizeRelH_pctWidth:
        case NS_ooxml::LN_CT_SizeRelV_pctHeight:
            if (m_pImpl->m_rPositivePercentages.empty())
                break;

            if (m_xShape.is())
            {
                sal_Int16 nPositivePercentage = rtl::math::round(m_pImpl->m_rPositivePercentages.front().toDouble() / oox::drawingml::PER_PERCENT);

                if (nPositivePercentage)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                    OUString aProperty = nSprmId == NS_ooxml::LN_CT_SizeRelH_pctWidth ? OUString("RelativeWidth") : OUString("RelativeHeight");

                    sal_Int32 nTextPreRotateAngle = 0;
                    uno::Any aAny;
                    if (xPropertySet->getPropertySetInfo()->hasPropertyByName(
                            "CustomShapeGeometry"))
                    {
                        aAny = xPropertySet->getPropertyValue("CustomShapeGeometry");
                    }
                    comphelper::SequenceAsHashMap aCustomShapeGeometry(aAny);
                    auto it = aCustomShapeGeometry.find("TextPreRotateAngle");
                    if (it != aCustomShapeGeometry.end())
                    {
                        nTextPreRotateAngle = it->second.get<sal_Int32>();
                    }
                    if (nTextPreRotateAngle == 0)
                    {
                        xPropertySet->setPropertyValue(aProperty,
                                                       uno::Any(nPositivePercentage));
                    }
                }
            }

            // Make sure the token is consumed even if xShape is an empty
            // reference.
            m_pImpl->m_rPositivePercentages.pop();
            break;
        case NS_ooxml::LN_EG_WrapType_wrapNone:
            //depending on the behindDoc attribute text wraps through behind or in front of the object
            m_pImpl->m_nWrap = text::WrapTextMode_THROUGH;

            // Wrap though means the margins defined earlier should not be
            // respected.
            m_pImpl->m_nLeftMargin = 0;
            m_pImpl->m_nTopMargin = 0;
            m_pImpl->m_nRightMargin = 0;
            m_pImpl->m_nBottomMargin = 0;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapTopAndBottom:
            // tdf#137850: Word >= 2013 seems to ignore bBehindDoc except for wrapNone, but older versions honour it.
            if (m_pImpl->m_bBehindDoc && m_pImpl->m_rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() > 14)
                 m_pImpl->m_bOpaque = true;
            m_pImpl->m_nWrap = text::WrapTextMode_NONE;
        break;
        case NS_ooxml::LN_CT_GraphicalObject_graphicData:
            {
                m_pImpl->m_bIsGraphic = true;

                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties )
                    pProperties->resolve(*this);
            }
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_a_hlinkClick:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties )
                    pProperties->resolve( *this );
            }
        break;
        default:
            SAL_WARN("writerfilter", "GraphicImport::lcl_sprm: unhandled token: " << nSprmId);
        break;
    }
}

void GraphicImport::lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& /*ref*/)
{
}

uno::Reference<text::XTextContent> GraphicImport::createGraphicObject(uno::Reference<graphic::XGraphic> const & rxGraphic,
                                                                      uno::Reference<beans::XPropertySet> const & xShapeProps)
{
    uno::Reference<text::XTextContent> xGraphicObject;
    try
    {
        if (rxGraphic.is())
        {
            uno::Reference< beans::XPropertySet > xGraphicObjectProperties(
                m_xTextFactory->createInstance("com.sun.star.text.TextGraphicObject"),
                uno::UNO_QUERY_THROW);
            xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_GRAPHIC), uno::Any(rxGraphic));
            xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_ANCHOR_TYPE),
                uno::Any( m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_ANCHOR ?
                                    text::TextContentAnchorType_AT_CHARACTER :
                                    text::TextContentAnchorType_AS_CHARACTER ));
            xGraphicObject.set( xGraphicObjectProperties, uno::UNO_QUERY_THROW );

            //shapes have only one border
            table::BorderLine2 aBorderLine;
            GraphicBorderLine& rBorderLine = m_pImpl->m_aBorders[0];
            if (rBorderLine.isEmpty() && xShapeProps.is() && xShapeProps->getPropertyValue("LineStyle").get<drawing::LineStyle>() != drawing::LineStyle_NONE)
            {
                // In case we got no border tokens and we have the
                // original shape, then use its line properties as the
                // border.
                aBorderLine.Color = xShapeProps->getPropertyValue("LineColor").get<sal_Int32>();
                aBorderLine.LineWidth = xShapeProps->getPropertyValue("LineWidth").get<sal_Int32>();
            }
            else
            {
                aBorderLine.Color = 0;
                aBorderLine.InnerLineWidth = 0;
                aBorderLine.OuterLineWidth = static_cast<sal_Int16>(rBorderLine.nLineWidth);
                aBorderLine.LineDistance = 0;
            }
            PropertyIds const aBorderProps[] =
            {
                PROP_LEFT_BORDER,
                PROP_RIGHT_BORDER,
                PROP_TOP_BORDER,
                PROP_BOTTOM_BORDER
            };

            for(PropertyIds const & rBorderProp : aBorderProps)
                xGraphicObjectProperties->setPropertyValue(getPropertyName(rBorderProp), uno::Any(aBorderLine));

            // setting graphic object shadow properties
            if (m_pImpl->m_bShadow)
            {
                // Shadow width is approximated by average of X and Y
                table::ShadowFormat aShadow;
                sal_uInt32 nShadowColor = m_pImpl->m_nShadowColor & 0x00FFFFFF; // The shadow color we get is RGB only.
                sal_Int32 nShadowWidth = (abs(m_pImpl->m_nShadowXDistance)
                                          + abs(m_pImpl->m_nShadowYDistance)) / 2;

                aShadow.ShadowWidth = nShadowWidth;
                sal_uInt8 nShadowTransparence = float(m_pImpl->m_nShadowTransparence) * 2.55;
                nShadowColor |= (nShadowTransparence << 24); // Add transparence to the color.
                aShadow.Color = nShadowColor;
                // Distances -ve for top and right, +ve for bottom and left
                if (m_pImpl->m_nShadowXDistance > 0)
                {
                    if (m_pImpl->m_nShadowYDistance > 0)
                        aShadow.Location = table::ShadowLocation_BOTTOM_RIGHT;
                    else
                        aShadow.Location = table::ShadowLocation_TOP_RIGHT;
                }
                else
                {
                    if (m_pImpl->m_nShadowYDistance > 0)
                        aShadow.Location = table::ShadowLocation_BOTTOM_LEFT;
                    else
                        aShadow.Location = table::ShadowLocation_TOP_LEFT;
                }

                xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_SHADOW_FORMAT), uno::Any(aShadow));
            }

            // setting properties for all types
            if( m_pImpl->m_bPositionProtected )
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_POSITION_PROTECTED ),
                    uno::Any(true));
            if( m_pImpl->m_bSizeProtected )
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_SIZE_PROTECTED ),
                    uno::Any(true));

            xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_DECORATIVE), uno::Any(m_pImpl->m_bDecorative));
            sal_Int32 nWidth = - m_pImpl->m_nLeftPosition;
            if (m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
            {
                //adjust margins
                if( (m_pImpl->m_nHoriOrient == text::HoriOrientation::LEFT &&
                    (m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                        m_pImpl->m_nHoriRelation == text::RelOrientation::FRAME) ) ||
                    (m_pImpl->m_nHoriOrient == text::HoriOrientation::INSIDE &&
                        m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ))
                    m_pImpl->m_nLeftMargin = 0;
                if((m_pImpl->m_nHoriOrient == text::HoriOrientation::RIGHT &&
                    (m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                        m_pImpl->m_nHoriRelation == text::RelOrientation::FRAME) ) ||
                    (m_pImpl->m_nHoriOrient == text::HoriOrientation::INSIDE &&
                        m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA ))
                    m_pImpl->m_nRightMargin = 0;
                // adjust top/bottom margins
                if( m_pImpl->m_nVertOrient == text::VertOrientation::TOP &&
                    ( m_pImpl->m_nVertRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                        m_pImpl->m_nVertRelation == text::RelOrientation::PAGE_FRAME))
                    m_pImpl->m_nTopMargin = 0;
                if( m_pImpl->m_nVertOrient == text::VertOrientation::BOTTOM &&
                    ( m_pImpl->m_nVertRelation == text::RelOrientation::PAGE_PRINT_AREA ||
                        m_pImpl->m_nVertRelation == text::RelOrientation::PAGE_FRAME))
                    m_pImpl->m_nBottomMargin = 0;
                if( m_pImpl->m_nVertOrient == text::VertOrientation::BOTTOM &&
                    m_pImpl->m_nVertRelation == text::RelOrientation::PAGE_PRINT_AREA )
                    m_pImpl->m_nBottomMargin = 0;
                //adjust alignment
                if( m_pImpl->m_nHoriOrient == text::HoriOrientation::INSIDE &&
                    m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_FRAME )
                {
                    // convert 'left to page' to 'from left -<width> to page text area'
                    m_pImpl->m_nHoriOrient = text::HoriOrientation::NONE;
                    m_pImpl->m_nHoriRelation = text::RelOrientation::PAGE_PRINT_AREA;
                    m_pImpl->m_nLeftPosition = - nWidth;
                }
                else if( m_pImpl->m_nHoriOrient == text::HoriOrientation::OUTSIDE &&
                    m_pImpl->m_nHoriRelation == text::RelOrientation::PAGE_FRAME )
                {
                    // convert 'right to page' to 'from left 0 to right page border'
                    m_pImpl->m_nHoriOrient = text::HoriOrientation::NONE;
                    m_pImpl->m_nHoriRelation = text::RelOrientation::PAGE_RIGHT;
                    m_pImpl->m_nLeftPosition = 0;
                }

                if (m_pImpl->m_nVertRelation == text::RelOrientation::TEXT_LINE)
                {
                    // Word's "line" is "below the bottom of the line", our TEXT_LINE is
                    // "towards top, from the bottom of the line", so invert the vertical position.
                    m_pImpl->m_nTopPosition *= -1;
                }

                m_pImpl->applyPosition(xGraphicObjectProperties);
                m_pImpl->applyRelativePosition(xGraphicObjectProperties);
                if( !m_pImpl->m_bOpaque )
                {
                    xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::Any(m_pImpl->m_bOpaque));
                }
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_SURROUND ),
                    uno::Any(static_cast<sal_Int32>(m_pImpl->m_nWrap)));
                if( m_pImpl->m_rDomainMapper.IsInTable())
                    xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_FOLLOW_TEXT_FLOW ),
                        uno::Any(m_pImpl->m_bLayoutInCell));

                xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_ALLOW_OVERLAP),
                                                           uno::Any(m_pImpl->m_bAllowOverlap));

                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_SURROUND_CONTOUR ),
                    uno::Any(m_pImpl->m_bContour));
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_CONTOUR_OUTSIDE ),
                    uno::Any(m_pImpl->m_bContourOutside));
                m_pImpl->applyMargins(xGraphicObjectProperties);
            }

            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_ADJUST_CONTRAST ),
                uno::Any(static_cast<sal_Int16>(m_pImpl->m_nContrast)));
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_ADJUST_LUMINANCE ),
                uno::Any(static_cast<sal_Int16>(m_pImpl->m_nBrightness)));
            if(m_pImpl->m_eColorMode != drawing::ColorMode_STANDARD)
            {
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_GRAPHIC_COLOR_MODE ),
                    uno::Any(m_pImpl->m_eColorMode));
            }

            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_BACK_COLOR ),
                uno::Any( GraphicImport_Impl::nFillColor ));
            m_pImpl->applyZOrder(xGraphicObjectProperties);

            //there seems to be no way to detect the original size via _real_ API
            uno::Reference< beans::XPropertySet > xGraphicProperties(rxGraphic, uno::UNO_QUERY_THROW);

            if (m_pImpl->mpWrapPolygon)
            {
                uno::Any aContourPolyPolygon;
                awt::Size aGraphicSize;
                WrapPolygon::Pointer_t pCorrected;
                xGraphicProperties->getPropertyValue(getPropertyName(PROP_SIZE100th_M_M)) >>= aGraphicSize;
                if (aGraphicSize.Width && aGraphicSize.Height)
                {
                    pCorrected = m_pImpl->mpWrapPolygon->correctWordWrapPolygon(aGraphicSize);
                }
                else
                {
                    xGraphicProperties->getPropertyValue(getPropertyName(PROP_SIZE_PIXEL)) >>= aGraphicSize;
                    if (aGraphicSize.Width && aGraphicSize.Height)
                    {
                        pCorrected = m_pImpl->mpWrapPolygon->correctWordWrapPolygonPixel(aGraphicSize);
                    }
                }

                text::GraphicCrop aGraphicCrop;
                xShapeProps->getPropertyValue("GraphicCrop") >>= aGraphicCrop;
                if (aGraphicCrop.Top != 0 || aGraphicCrop.Bottom != 0 || aGraphicCrop.Left != 0
                    || aGraphicCrop.Right != 0)
                {
                    // Word's wrap polygon deals with a canvas which has the size of the already
                    // cropped graphic, correct our polygon to have the same render result.
                    pCorrected = pCorrected->correctCrop(aGraphicSize, aGraphicCrop);
                }

                if (pCorrected)
                {
                    aContourPolyPolygon <<= pCorrected->getPointSequenceSequence();
                    xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_CONTOUR_POLY_POLYGON),
                        aContourPolyPolygon);
                    // We should bring it to front, even if wp:anchor's behindDoc="1",
                    // because otherwise paragraph background (if set) overlaps the graphic
                    // TODO: if paragraph's background becomes bottommost, then remove this hack
                    xGraphicObjectProperties->setPropertyValue("Opaque", uno::Any(true));
                }
            }


            if (m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_INLINE
                || m_pImpl->m_rGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
            {
                if( m_pImpl->getXSize() && m_pImpl->getYSize() )
                    xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_SIZE),
                        uno::Any( awt::Size( m_pImpl->getXSize(), m_pImpl->getYSize() )));
                m_pImpl->applyMargins(xGraphicObjectProperties);
                m_pImpl->applyName(xGraphicObjectProperties);
                m_pImpl->applyHyperlink(xGraphicObjectProperties, false);
            }

            // Handle horizontal flip.
            bool bMirrored = false;
            xShapeProps->getPropertyValue("IsMirrored") >>= bMirrored;
            if (bMirrored)
            {
                xGraphicObjectProperties->setPropertyValue("HoriMirroredOnEvenPages",
                                                           uno::Any(true));
                xGraphicObjectProperties->setPropertyValue("HoriMirroredOnOddPages",
                                                           uno::Any(true));
            }
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("writerfilter", "");
    }
    return xGraphicObject;
}


void GraphicImport::data(const sal_uInt8* buf, size_t len)
{
    uno::Reference< io::XInputStream > xIStream = new XInputStreamHelper( buf, len );
    beans::PropertyValues aMediaProperties{ comphelper::makePropertyValue(
        getPropertyName(PROP_INPUT_STREAM), xIStream) };

    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Reference<graphic::XGraphicProvider> xGraphicProvider(graphic::GraphicProvider::create(m_xComponentContext));
    uno::Reference<graphic::XGraphic> xGraphic = xGraphicProvider->queryGraphic(aMediaProperties);
    m_xGraphicObject = createGraphicObject(xGraphic, xPropertySet);
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

void GraphicImport::lcl_utext(const sal_Unicode * /*_data*/, size_t /*len*/)
{
}

void GraphicImport::lcl_props(const writerfilter::Reference<Properties>::Pointer_t& /*ref*/)
{
}

void GraphicImport::lcl_table(Id /*name*/, const writerfilter::Reference<Table>::Pointer_t& /*ref*/)
{
}

void GraphicImport::lcl_substream(Id /*name*/, const writerfilter::Reference<Stream>::Pointer_t& /*ref*/)
{
}

void GraphicImport::lcl_startShape(uno::Reference<drawing::XShape> const&)
{
}

void GraphicImport::lcl_endShape( )
{
}

bool GraphicImport::IsGraphic() const
{
    return m_pImpl->m_bIsGraphic;
}

sal_Int32 GraphicImport::GetLeftMarginOrig() const
{
    return m_pImpl->m_nLeftMarginOrig;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
