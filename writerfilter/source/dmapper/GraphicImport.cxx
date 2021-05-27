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
#include <svx/svdtrans.hxx>
#include <svx/unoapi.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
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

using namespace css;

namespace
{
bool isTopGroupObj(const uno::Reference<drawing::XShape>& xShape)
{
    SdrObject* pObject = GetSdrObjectFromXShape(xShape);
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
    sal_Int32 nXSize;
    bool      bXSizeValid;
    sal_Int32 nYSize;
    bool      bYSizeValid;

public:
    GraphicImportType eGraphicImportType;
    DomainMapper&   rDomainMapper;

    sal_Int32 nLeftPosition;
    sal_Int32 nTopPosition;

    bool      bUseSimplePos;
    sal_Int32 zOrder;

    sal_Int16 nHoriOrient;
    sal_Int16 nHoriRelation;
    bool bPageToggle = false;
    sal_Int16 nVertOrient;
    sal_Int16 nVertRelation;
    text::WrapTextMode nWrap;
    bool      bLayoutInCell;
    bool      bCompatForcedLayoutInCell;
    bool bAllowOverlap = true;
    bool      bOpaque;
    bool      bBehindDoc;
    bool      bContour;
    bool      bContourOutside;
    WrapPolygon::Pointer_t mpWrapPolygon;

    sal_Int32 nLeftMargin;
    sal_Int32 nLeftMarginOrig = 0;
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

    static constexpr sal_Int32 nFillColor = 0xffffffff;

    drawing::ColorMode eColorMode;

    GraphicBorderLine   aBorders[4];

    bool            bIsGraphic;

    bool            bSizeProtected;
    bool            bPositionProtected;
    bool            bHidden;

    sal_Int32       nShapeOptionType;

    OUString sName;
    OUString sAlternativeText;
    OUString title;
    OUString sHyperlinkURL;
    std::pair<OUString, OUString>& m_rPositionOffsets;
    std::pair<OUString, OUString>& m_rAligns;
    std::queue<OUString>& m_rPositivePercentages;
    OUString sAnchorId;
    comphelper::SequenceAsHashMap m_aInteropGrabBag;
    std::optional<sal_Int32> m_oEffectExtentLeft;
    std::optional<sal_Int32> m_oEffectExtentTop;
    std::optional<sal_Int32> m_oEffectExtentRight;
    std::optional<sal_Int32> m_oEffectExtentBottom;

    GraphicImport_Impl(GraphicImportType eImportType, DomainMapper& rDMapper, std::pair<OUString, OUString>& rPositionOffsets, std::pair<OUString, OUString>& rAligns, std::queue<OUString>& rPositivePercentages) :
        nXSize(0)
        ,bXSizeValid(false)
        ,nYSize(0)
        ,bYSizeValid(false)
        ,eGraphicImportType( eImportType )
        ,rDomainMapper( rDMapper )
        ,nLeftPosition(0)
        ,nTopPosition(0)
        ,bUseSimplePos(false)
        ,zOrder(-1)
        ,nHoriOrient(   text::HoriOrientation::NONE )
        ,nHoriRelation( text::RelOrientation::FRAME )
        ,nVertOrient(  text::VertOrientation::NONE )
        ,nVertRelation( text::RelOrientation::FRAME )
        ,nWrap(text::WrapTextMode_NONE)
        ,bLayoutInCell(true)
        ,bCompatForcedLayoutInCell(false)
        ,bOpaque( !rDMapper.IsInHeaderFooter() )
        ,bBehindDoc(false)
        ,bContour(false)
        ,bContourOutside(true)
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
        ,eColorMode( drawing::ColorMode_STANDARD )
        ,bIsGraphic(false)
        ,bSizeProtected(false)
        ,bPositionProtected(false)
        ,bHidden(false)
        ,nShapeOptionType(0)
        ,m_rPositionOffsets(rPositionOffsets)
        ,m_rAligns(rAligns)
        ,m_rPositivePercentages(rPositivePercentages)
    {
        if (eGraphicImportType == GraphicImportType::IMPORT_AS_DETECTED_INLINE
            && !rDMapper.IsInShape())
        {
            zOrder = 0;
        }
    }

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

    bool isYSizeValid() const
    {
        return bYSizeValid;
    }

    void applyMargins(const uno::Reference< beans::XPropertySet >& xGraphicObjectProperties) const
    {
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_LEFT_MARGIN ), uno::makeAny(nLeftMargin));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_RIGHT_MARGIN ), uno::makeAny(nRightMargin));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_TOP_MARGIN ), uno::makeAny(nTopMargin));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_BOTTOM_MARGIN ), uno::makeAny(nBottomMargin));
    }

    void applyPosition(const uno::Reference< beans::XPropertySet >& xGraphicObjectProperties) const
    {
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HORI_ORIENT          ),
                uno::makeAny(nHoriOrient));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_VERT_ORIENT          ),
                uno::makeAny(nVertOrient));
    }

    void applyRelativePosition(const uno::Reference< beans::XPropertySet >& xGraphicObjectProperties, bool bRelativeOnly = false) const
    {
        if (!bRelativeOnly)
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HORI_ORIENT_POSITION),
                                                       uno::makeAny(nLeftPosition));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HORI_ORIENT_RELATION ),
                uno::makeAny(nHoriRelation));
        xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_PAGE_TOGGLE),
                                                   uno::makeAny(bPageToggle));
        if (!bRelativeOnly)
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_VERT_ORIENT_POSITION),
                                                       uno::makeAny(nTopPosition));
        xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_VERT_ORIENT_RELATION ),
                uno::makeAny(nVertRelation));
    }

    void applyZOrder(uno::Reference<beans::XPropertySet> const & xGraphicObjectProperties) const
    {
        if (zOrder >= 0)
        {
            // tdf#120760 Send objects with behinddoc=true to the back.
            sal_Int32 nZOrder = zOrder;
            if (bBehindDoc && rDomainMapper.IsInHeaderFooter())
                nZOrder -= SAL_MAX_INT32;
            GraphicZOrderHelper* pZOrderHelper = rDomainMapper.graphicZOrderHelper();
            bool bOldStyle = eGraphicImportType == GraphicImportType::IMPORT_AS_DETECTED_INLINE;
            xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_Z_ORDER),
                uno::makeAny(pZOrderHelper->findZOrder(nZOrder, bOldStyle)));
            pZOrderHelper->addItem(xGraphicObjectProperties, nZOrder);
        }
    }

    void applyName(uno::Reference<beans::XPropertySet> const & xGraphicObjectProperties) const
    {
        try
        {
            // Ask the graphic naming helper to find out the name for this
            // object: It's around till the end of the import, so it remembers
            // what's the first free name.
            uno::Reference< container::XNamed > xNamed( xGraphicObjectProperties, uno::UNO_QUERY_THROW );
            xNamed->setName(rDomainMapper.GetGraphicNamingHelper().NameGraphic(sName));

            if ( sHyperlinkURL.getLength() > 0 )
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_HYPER_LINK_U_R_L ),
                    uno::makeAny ( sHyperlinkURL ));
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_DESCRIPTION ),
                uno::makeAny( sAlternativeText ));
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_TITLE ),
                uno::makeAny( title ));
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("writerfilter", "failed");
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

GraphicImport::GraphicImport(uno::Reference<uno::XComponentContext> const& xComponentContext,
                             uno::Reference<lang::XMultiServiceFactory> const& xTextFactory,
                             DomainMapper& rDMapper,
                             GraphicImportType eImportType,
                             std::pair<OUString, OUString>& rPositionOffsets,
                             std::pair<OUString, OUString>& rAligns,
                             std::queue<OUString>& rPositivePercentages)
: LoggedProperties("GraphicImport")
, LoggedTable("GraphicImport")
, LoggedStream("GraphicImport")
, m_pImpl(new GraphicImport_Impl(eImportType, rDMapper, rPositionOffsets, rAligns, rPositivePercentages))
, m_xComponentContext(xComponentContext)
, m_xTextFactory(xTextFactory)
{
}

GraphicImport::~GraphicImport()
{
}

com::sun::star::awt::Point GraphicImport::GetGraphicObjectPosition() const
{
    return (com::sun::star::awt::Point(m_pImpl->nLeftPosition, m_pImpl->nTopPosition));
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

        xSet->setPropertyValue(aGrabBagPropName, uno::makeAny(comphelper::containerToSequence(aGrabBag)));
    }
}

static bool lcl_bHasGroupSlantedChild (const SdrObject* pObj)
{
    // Returns true, if a child object differs more then 0.02deg from horizontal or vertical.
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
            const tools::Long nRot = NormAngle36000(pSubObj->GetRotateAngle());
            if ((3 < nRot && nRot < 8997) || (9003 < nRot && nRot < 17997)
                || (18003 < nRot && nRot < 26997) || (27003 < nRot && nRot < 35997))
                return true;
    }
    return false;
}

void GraphicImport::lcl_attribute(Id nName, Value& rValue)
{
    sal_Int32 nIntValue = rValue.getInt();
    switch( nName )
    {
        case NS_ooxml::LN_CT_Hyperlink_URL://90682;
            m_pImpl->sHyperlinkURL = rValue.getString();
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
            m_pImpl->aBorders[BORDER_TOP].nLineWidth = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_val:
            //graphic borders don't support different line types
        break;
        case NS_ooxml::LN_CT_Border_space:
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            m_pImpl->aBorders[BORDER_TOP].bHasShadow = nIntValue != 0;
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
        case NS_ooxml::LN_CT_NonVisualDrawingProps_hidden:
            m_pImpl->bHidden = (nIntValue == 1);
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
            if (nIntValue > 0)
            {
                m_pImpl->bOpaque = false;
                m_pImpl->bBehindDoc = true;
            }
        break;
        case NS_ooxml::LN_CT_Anchor_locked: // 90990; - ignored
        break;
        case NS_ooxml::LN_CT_Anchor_layoutInCell: // 90991; - ignored
            // Starting in MSO 2013, anchors are ALWAYS considered to be laid out in table cell.
            m_pImpl->bCompatForcedLayoutInCell = !nIntValue
                && m_pImpl->rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() > 14
                && m_pImpl->rDomainMapper.IsInTable();
            m_pImpl->bLayoutInCell = m_pImpl->bCompatForcedLayoutInCell || nIntValue;
        break;
        case NS_ooxml::LN_CT_Anchor_hidden: // 90992; - ignored
        break;
        case NS_ooxml::LN_CT_Anchor_allowOverlap:
            m_pImpl->bAllowOverlap = nIntValue != 0;
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

                        uno::Reference<graphic::XGraphic> xGraphic;
                        xShapeProps->getPropertyValue("Graphic") >>= xGraphic;

                        sal_Int32 nRotation = 0;
                        xShapeProps->getPropertyValue("RotateAngle") >>= nRotation;

                        css::beans::PropertyValues aGrabBag;
                        xShapeProps->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                        // if the shape contains effects in the grab bag, we should not transform it
                        // in a XTextContent so those effects can be preserved
                        bool bContainsEffects = std::any_of(aGrabBag.begin(), aGrabBag.end(), [](const auto& rProp) {
                            return rProp.Name == "EffectProperties"
                                || rProp.Name == "3DEffectProperties"
                                || rProp.Name == "ArtisticEffectProperties";
                        });

                        xShapeProps->getPropertyValue("Shadow") >>= m_pImpl->bShadow;
                        if (m_pImpl->bShadow)
                        {
                            xShapeProps->getPropertyValue("ShadowXDistance") >>= m_pImpl->nShadowXDistance;
                            xShapeProps->getPropertyValue("ShadowYDistance") >>= m_pImpl->nShadowYDistance;
                            xShapeProps->getPropertyValue("ShadowColor") >>= m_pImpl->nShadowColor;
                            xShapeProps->getPropertyValue("ShadowTransparence") >>= m_pImpl->nShadowTransparence;
                        }

                        xShapeProps->getPropertyValue("GraphicColorMode") >>= m_pImpl->eColorMode;
                        xShapeProps->getPropertyValue("AdjustLuminance") >>= m_pImpl->nBrightness;
                        xShapeProps->getPropertyValue("AdjustContrast") >>= m_pImpl->nContrast;

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


                        xShapeProps->setPropertyValue
                            (getPropertyName(PROP_ANCHOR_TYPE),
                             uno::makeAny
                             (text::TextContentAnchorType_AS_CHARACTER));

                        // In Word, if a shape is anchored inline, that
                        // excludes being in the background.
                        xShapeProps->setPropertyValue("Opaque", uno::makeAny(true));

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
                                 uno::makeAny
                                 (m_pImpl->rDomainMapper.GetCurrentTextRange()));
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
                        const bool bIsGroupOrLine = xServiceInfo->supportsService("com.sun.star.drawing.GroupShape")
                            || xServiceInfo->supportsService("com.sun.star.drawing.LineShape");
                        SdrObject* pShape = GetSdrObjectFromXShape(m_xShape);
                        if ((bIsGroupOrLine && !lcl_bHasGroupSlantedChild(pShape) && nOOXAngle == 0)
                            || !bIsGroupOrLine)
                        {
                            if (m_pImpl->isXSizeValid())
                                aSize.Width = m_pImpl->getXSize();
                            if (m_pImpl->isYSizeValid())
                                aSize.Height = m_pImpl->getYSize();
                        }

                        sal_Int32 nRotation = 0;
                        if (bKeepRotation)
                        {
                            // Use internal API, getPropertyValue(RotateAngle)
                            // would use GetObjectRotation(), which is not what
                            // we want.
                            if (pShape)
                                nRotation = pShape->GetRotateAngle();
                        }
                        m_xShape->setSize(aSize);
                        if (bKeepRotation)
                        {
                            xShapeProps->setPropertyValue("RotateAngle", uno::makeAny(nRotation));
                            if (nRotation == 0)
                            {
                                // Include effect extent in the margin to bring Writer layout closer
                                // to Word. But do this for non-rotated shapes only, where effect
                                // extents map to increased margins as-is.

                                sal_Int32 nLineWidth{};
                                if (xShapeProps->getPropertySetInfo()->hasPropertyByName("LineWidth"))
                                {
                                    xShapeProps->getPropertyValue("LineWidth") >>= nLineWidth;
                                }

                                if (m_pImpl->m_oEffectExtentLeft)
                                {
                                    sal_Int32 nLeft = oox::drawingml::convertEmuToHmm(
                                        *m_pImpl->m_oEffectExtentLeft);
                                    if (nLeft >= nLineWidth / 2)
                                    {
                                        nLeft -= nLineWidth / 2;
                                    }
                                    m_pImpl->nLeftMargin += nLeft;
                                }
                                if (m_pImpl->m_oEffectExtentTop)
                                {
                                    sal_Int32 nTop = oox::drawingml::convertEmuToHmm(*m_pImpl->m_oEffectExtentTop);
                                    if (nTop >= nLineWidth / 2)
                                    {
                                        nTop -= nLineWidth / 2;
                                    }
                                    m_pImpl->nTopMargin += nTop;
                                }
                                if (m_pImpl->m_oEffectExtentRight)
                                {
                                    sal_Int32 nRight = oox::drawingml::convertEmuToHmm(
                                        *m_pImpl->m_oEffectExtentRight);
                                    if (nRight >= nLineWidth / 2)
                                    {
                                        nRight -= nLineWidth / 2;
                                    }
                                    m_pImpl->nRightMargin += nRight;
                                }
                                if (m_pImpl->m_oEffectExtentBottom)
                                {
                                    sal_Int32 nBottom = oox::drawingml::convertEmuToHmm(
                                        *m_pImpl->m_oEffectExtentBottom);
                                    if (nBottom >= nLineWidth / 2)
                                    {
                                        nBottom -= nLineWidth / 2;
                                    }
                                    m_pImpl->nBottomMargin += nBottom;
                                }
                            }
                        }

                        m_pImpl->bIsGraphic = true;

                        if (!m_pImpl->sAnchorId.isEmpty())
                        {
                            putPropertyToFrameGrabBag("AnchorId", uno::makeAny(m_pImpl->sAnchorId));
                        }

                        // Calculate mso unrotated rectangle and its center, needed below
                        awt::Size aImportSize(m_xShape->getSize()); // here only fallback
                        if (m_pImpl->isXSizeValid())
                            aImportSize.Width = m_pImpl->getXSize(); // Hmm
                        if (m_pImpl->isYSizeValid())
                            aImportSize.Height = m_pImpl->getYSize(); // Hmm
                        const awt::Point aImportPosition(GetGraphicObjectPosition()); // Hmm
                        const awt::Point aCentrum(aImportPosition.X + aImportSize.Width / 2,
                                                  aImportPosition.Y + aImportSize.Height / 2);

                        // In case of group and lines, rotations are incorported in the child shapes or
                        // points respectively in LO. MSO has rotation as separate property. The
                        // position refers to the unrotated rectangle of MSO. We need to adapt it to
                        // the left-top of the rotated shape.
                        if (bIsGroupOrLine)
                        {
                            // Get actual LO snap rectangle size of group or line.
                            awt::Size aLOSize(m_xShape->getSize()); //Hmm

                            // Set LO position. MSO rotation is done on shape center.
                            m_pImpl->nLeftPosition = aCentrum.X - aLOSize.Width / 2;
                            m_pImpl->nTopPosition = aCentrum.Y - aLOSize.Height / 2;
                            m_xShape->setPosition(GetGraphicObjectPosition());
                        }

                        // Margin correction
                        // In case of wrap "Square" or "in Line", MSO uses special rules to
                        // determine the rectangle into which the shape is placed, depending on
                        // rotation angle.
                        // If angle is smaller to horizontal than 45deg, the unrotated mso shape
                        // rectangle is used, whereby the height is expanded to the bounding
                        // rectangle height of the shape.
                        // If angle is larger to horizontal than 45deg, the 90deg rotated rectangle
                        // is used, whereby the width is expanded to the bounding width of the
                        // shape.
                        if (bIsGroupOrLine && (m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_INLINE
                            || (m_pImpl->nWrap == text::WrapTextMode_PARALLEL && !(m_pImpl->mpWrapPolygon))))
                        {

                            nOOXAngle = (nOOXAngle / 60000) % 180; // convert to degree in [0°,180°[

                            if (nOOXAngle >= 45 && nOOXAngle < 135)
                            {
                                const sal_Int32 nImportRot90Top(aCentrum.Y - aImportSize.Width / 2);
                                sal_Int32 nVertMarginOffset(m_pImpl->nTopPosition - nImportRot90Top);
                                nVertMarginOffset = std::max<sal_Int32>(nVertMarginOffset, 0);
                                m_pImpl->nTopMargin += nVertMarginOffset;
                                m_pImpl->nBottomMargin += nVertMarginOffset;
                            }
                            else
                            {
                                sal_Int32 nHoriMarginOffset(m_pImpl->nLeftPosition - aImportPosition.X);
                                nHoriMarginOffset = std::max<sal_Int32>(nHoriMarginOffset, 0);
                                m_pImpl->nLeftMargin += nHoriMarginOffset;
                                m_pImpl->nRightMargin += nHoriMarginOffset;
                            }
                        }
                    }

                    if (bUseShape && m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
                    {
                        // If we are here, this is a drawingML shape. For those, only dmapper (and not oox) knows the anchoring infos (just like for Writer pictures).
                        // But they aren't Writer pictures, either (which are already handled above).
                        uno::Reference< beans::XPropertySet > xShapeProps(m_xShape, uno::UNO_QUERY_THROW);

                        // This needs to be AT_PARAGRAPH by default and not AT_CHARACTER, otherwise shape will move when the user inserts a new paragraph.
                        text::TextContentAnchorType eAnchorType = text::TextContentAnchorType_AT_PARAGRAPH;

                        if (m_pImpl->bHidden)
                        {
                            xShapeProps->setPropertyValue("Visible", uno::makeAny(false));
                            xShapeProps->setPropertyValue("Printable", uno::makeAny(false));
                        }

                        // Avoid setting AnchorType for TextBoxes till SwTextBoxHelper::syncProperty() doesn't handle transition.
                        bool bTextBox = false;
                        xShapeProps->getPropertyValue("TextBox") >>= bTextBox;

                        // The positioning change caused by LayoutInCell doesn't sync well
                        // in the text / frame duo. So the compatibility fix only correctly
                        // positions the frame and not the text currently.
                        // tdf#135943: Instead of half-fixing and making a complete mess,
                        // just avoid until layout's repositioning is sync'd to the text frame.
                        if (m_pImpl->bLayoutInCell && bTextBox)
                            m_pImpl->bLayoutInCell = !m_pImpl->bCompatForcedLayoutInCell;

                        if (m_pImpl->nVertRelation == text::RelOrientation::TEXT_LINE)
                            eAnchorType = text::TextContentAnchorType_AT_CHARACTER;

                        xShapeProps->setPropertyValue("AnchorType", uno::makeAny(eAnchorType));

                        if (m_pImpl->nVertRelation == text::RelOrientation::TEXT_LINE)
                        {
                            // Word's "line" is "below the bottom of the line", our TEXT_LINE is
                            // "towards top, from the bottom of the line", so invert the vertical
                            // position.
                            awt::Point aPoint = xShape->getPosition();
                            aPoint.Y *= -1;
                            xShape->setPosition(aPoint);
                        }

                        if (m_pImpl->bLayoutInCell && bTextBox && m_pImpl->rDomainMapper.IsInTable()
                            && m_pImpl->nHoriRelation == text::RelOrientation::PAGE_FRAME)
                            m_pImpl->nHoriRelation = text::RelOrientation::FRAME;
                        if(m_pImpl->rDomainMapper.IsInTable())
                            xShapeProps->setPropertyValue(getPropertyName(PROP_FOLLOW_TEXT_FLOW),
                                uno::makeAny(m_pImpl->bLayoutInCell));
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
                                xShapeProps->setPropertyValue("RotateAngle", uno::makeAny(sal_Int32(0)));

                            // Position of the groupshape should be set after children have been added.
                            // Long-term we should get rid of positioning group
                            // shapes, though. Do it for top-level ones with
                            // absolute page position as a start.
                            // fdo#80555: also set position for graphic shapes here
                            if (!isTopGroupObj(m_xShape)
                                || m_pImpl->nHoriRelation != text::RelOrientation::PAGE_FRAME
                                || m_pImpl->nVertRelation != text::RelOrientation::PAGE_FRAME)
                                m_xShape->setPosition(
                                    awt::Point(m_pImpl->nLeftPosition, m_pImpl->nTopPosition));

                            if (nRotation)
                                xShapeProps->setPropertyValue("RotateAngle", uno::makeAny(nRotation));
                        }


                        m_pImpl->applyRelativePosition(xShapeProps, /*bRelativeOnly=*/true);

                        xShapeProps->setPropertyValue("SurroundContour", uno::makeAny(m_pImpl->bContour));
                        m_pImpl->applyMargins(xShapeProps);
                        xShapeProps->setPropertyValue("Opaque", uno::makeAny(m_pImpl->bOpaque));
                        xShapeProps->setPropertyValue("Surround", uno::makeAny(static_cast<sal_Int32>(m_pImpl->nWrap)));
                        m_pImpl->applyZOrder(xShapeProps);
                        m_pImpl->applyName(xShapeProps);
                        xShapeProps->setPropertyValue("AllowOverlap",
                                                      uno::makeAny(m_pImpl->bAllowOverlap));

                        // Get the grab-bag set by oox, merge with our one and then put it back.
                        comphelper::SequenceAsHashMap aInteropGrabBag(xShapeProps->getPropertyValue("InteropGrabBag"));
                        aInteropGrabBag.update(m_pImpl->getInteropGrabBag());
                        xShapeProps->setPropertyValue("InteropGrabBag", uno::makeAny(aInteropGrabBag.getAsConstPropertyValueList()));
                    }
                    else if (bUseShape && m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_INLINE)
                    {
                        uno::Reference< beans::XPropertySet > xShapeProps(m_xShape, uno::UNO_QUERY_THROW);
                        m_pImpl->applyMargins(xShapeProps);
                        m_pImpl->applyZOrder(xShapeProps);
                        comphelper::SequenceAsHashMap aInteropGrabBag(xShapeProps->getPropertyValue("InteropGrabBag"));
                        aInteropGrabBag.update(m_pImpl->getInteropGrabBag());
                        xShapeProps->setPropertyValue("InteropGrabBag", uno::makeAny(aInteropGrabBag.getAsConstPropertyValueList()));
                    }
                }
            }
        break;
        case NS_ooxml::LN_CT_Inline_distT:
            m_pImpl->nTopMargin = 0;
        break;
        case NS_ooxml::LN_CT_Inline_distB:
            m_pImpl->nBottomMargin = 0;
        break;
        case NS_ooxml::LN_CT_Inline_distL:
            m_pImpl->nLeftMargin = 0;
        break;
        case NS_ooxml::LN_CT_Inline_distR:
            m_pImpl->nRightMargin = 0;
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
                case NS_ooxml::LN_ST_SizeRelFromH_leftMargin:
                case NS_ooxml::LN_ST_SizeRelFromH_outsideMargin:
                    if (m_xShape.is())
                    {
                        // Here we handle the relative size of the width of some shape.
                        // The size of the shape's width is going to be relative to the size of the left margin.
                        // E.g.: (left margin = 8 && relative size = 150%) -> width of some shape = 12.
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::makeAny(text::RelOrientation::PAGE_LEFT));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromH_rightMargin:
                case NS_ooxml::LN_ST_SizeRelFromH_insideMargin:
                    if (m_xShape.is())
                    {
                        // Same as the left margin above.
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeWidthRelation", uno::makeAny(text::RelOrientation::PAGE_RIGHT));
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
                case NS_ooxml::LN_ST_SizeRelFromV_topMargin:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::makeAny(text::RelOrientation::PAGE_PRINT_AREA));
                    }
                    break;
                case NS_ooxml::LN_ST_SizeRelFromV_bottomMargin:
                    if (m_xShape.is())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(m_xShape, uno::UNO_QUERY);
                        xPropertySet->setPropertyValue("RelativeHeightRelation", uno::makeAny(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
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
    switch( m_pImpl->nShapeOptionType )
    {
        case NS_ooxml::LN_CT_Anchor_distL:
            m_pImpl->nLeftMargin = nIntValue / 360;
            m_pImpl->nLeftMarginOrig = m_pImpl->nLeftMargin;
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

    switch(nSprmId)
    {
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
        case NS_ooxml::LN_hlinkClick_hlinkClick:
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
                    if (m_pImpl->bBehindDoc && m_pImpl->rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() > 14)
                        m_pImpl->bOpaque = true;
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
        case NS_ooxml::LN_CT_Anchor_positionH: // 90976;
        {
            // Use a special handler for the positioning
            auto pHandler = std::make_shared<PositionHandler>( m_pImpl->m_rPositionOffsets, m_pImpl->m_rAligns );
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                pProperties->resolve( *pHandler );
                if( !m_pImpl->bUseSimplePos )
                {
                    m_pImpl->nHoriRelation = pHandler->relation();
                    m_pImpl->bPageToggle = pHandler->GetPageToggle();
                    m_pImpl->nHoriOrient = pHandler->orientation();
                    m_pImpl->nLeftPosition = pHandler->position();

                    // Left adjustments: if horizontally aligned to left of margin, then remove the
                    // left wrapping.
                    if (m_pImpl->nHoriOrient == text::HoriOrientation::LEFT)
                    {
                        if (m_pImpl->nHoriRelation == text::RelOrientation::PAGE_PRINT_AREA)
                        {
                            m_pImpl->nLeftMargin = 0;
                        }
                    }
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_Anchor_positionV: // 90977;
        {
            // Use a special handler for the positioning
            auto pHandler = std::make_shared<PositionHandler>( m_pImpl->m_rPositionOffsets, m_pImpl->m_rAligns);
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
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
                                                       uno::makeAny(nPositivePercentage));
                    }
                }
            }

            // Make sure the token is consumed even if xShape is an empty
            // reference.
            m_pImpl->m_rPositivePercentages.pop();
            break;
        case NS_ooxml::LN_EG_WrapType_wrapNone: // 90944; - doesn't contain attributes
            //depending on the behindDoc attribute text wraps through behind or in front of the object
            m_pImpl->nWrap = text::WrapTextMode_THROUGH;

            // Wrap though means the margins defined earlier should not be
            // respected.
            m_pImpl->nLeftMargin = 0;
            m_pImpl->nTopMargin = 0;
            m_pImpl->nRightMargin = 0;
            m_pImpl->nBottomMargin = 0;
        break;
        case NS_ooxml::LN_EG_WrapType_wrapTopAndBottom: // 90948;
            // tdf#137850: Word >= 2013 seems to ignore bBehindDoc except for wrapNone, but older versions honour it.
            if (m_pImpl->bBehindDoc && m_pImpl->rDomainMapper.GetSettingsTable()->GetWordCompatibilityMode() > 14)
                 m_pImpl->bOpaque = true;
            m_pImpl->nWrap = text::WrapTextMode_NONE;
        break;
        case NS_ooxml::LN_CT_GraphicalObject_graphicData:// 90660;
            {
                m_pImpl->bIsGraphic = true;

                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties )
                    pProperties->resolve(*this);
            }
        break;
        case NS_ooxml::LN_CT_NonVisualDrawingProps_a_hlinkClick: // 90689;
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

void GraphicImport::lcl_entry(writerfilter::Reference<Properties>::Pointer_t /*ref*/)
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
            xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_GRAPHIC), uno::makeAny(rxGraphic));
            xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_ANCHOR_TYPE),
                uno::makeAny( m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR ?
                                    text::TextContentAnchorType_AT_CHARACTER :
                                    text::TextContentAnchorType_AS_CHARACTER ));
            xGraphicObject.set( xGraphicObjectProperties, uno::UNO_QUERY_THROW );

            //shapes have only one border
            table::BorderLine2 aBorderLine;
            GraphicBorderLine& rBorderLine = m_pImpl->aBorders[0];
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
                xGraphicObjectProperties->setPropertyValue(getPropertyName(rBorderProp), uno::makeAny(aBorderLine));

            // setting graphic object shadow properties
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
                        aShadow.Location = table::ShadowLocation_BOTTOM_RIGHT;
                    else
                        aShadow.Location = table::ShadowLocation_TOP_RIGHT;
                }
                else
                {
                    if (m_pImpl->nShadowYDistance > 0)
                        aShadow.Location = table::ShadowLocation_BOTTOM_LEFT;
                    else
                        aShadow.Location = table::ShadowLocation_TOP_LEFT;
                }

                xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_SHADOW_FORMAT), uno::makeAny(aShadow));
            }

            // setting properties for all types
            if( m_pImpl->bPositionProtected )
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_POSITION_PROTECTED ),
                    uno::makeAny(true));
            if( m_pImpl->bSizeProtected )
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_SIZE_PROTECTED ),
                    uno::makeAny(true));

            sal_Int32 nWidth = - m_pImpl->nLeftPosition;
            if (m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
            {
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
                if( !m_pImpl->bOpaque )
                {
                    xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::makeAny(m_pImpl->bOpaque));
                }
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_SURROUND ),
                    uno::makeAny(static_cast<sal_Int32>(m_pImpl->nWrap)));
                if( m_pImpl->rDomainMapper.IsInTable())
                    xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_FOLLOW_TEXT_FLOW ),
                        uno::makeAny(m_pImpl->bLayoutInCell));

                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_SURROUND_CONTOUR ),
                    uno::makeAny(m_pImpl->bContour));
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_CONTOUR_OUTSIDE ),
                    uno::makeAny(m_pImpl->bContourOutside));
                m_pImpl->applyMargins(xGraphicObjectProperties);
            }

            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_ADJUST_CONTRAST ),
                uno::makeAny(static_cast<sal_Int16>(m_pImpl->nContrast)));
            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_ADJUST_LUMINANCE ),
                uno::makeAny(static_cast<sal_Int16>(m_pImpl->nBrightness)));
            if(m_pImpl->eColorMode != drawing::ColorMode_STANDARD)
            {
                xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_GRAPHIC_COLOR_MODE ),
                    uno::makeAny(m_pImpl->eColorMode));
            }

            xGraphicObjectProperties->setPropertyValue(getPropertyName( PROP_BACK_COLOR ),
                uno::makeAny( GraphicImport_Impl::nFillColor ));
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
                    xGraphicObjectProperties->setPropertyValue("Opaque", uno::makeAny(true));
                }
            }


            if(m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_INLINE || m_pImpl->eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
            {
                if( m_pImpl->getXSize() && m_pImpl->getYSize() )
                    xGraphicObjectProperties->setPropertyValue(getPropertyName(PROP_SIZE),
                        uno::makeAny( awt::Size( m_pImpl->getXSize(), m_pImpl->getYSize() )));
                m_pImpl->applyMargins(xGraphicObjectProperties);
                m_pImpl->applyName(xGraphicObjectProperties);
            }

            // Handle horizontal flip.
            bool bMirrored = false;
            xShapeProps->getPropertyValue("IsMirrored") >>= bMirrored;
            if (bMirrored)
            {
                xGraphicObjectProperties->setPropertyValue("HoriMirroredOnEvenPages",
                                                           uno::makeAny(true));
                xGraphicObjectProperties->setPropertyValue("HoriMirroredOnOddPages",
                                                           uno::makeAny(true));
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
    beans::PropertyValues aMediaProperties( 1 );
    aMediaProperties[0].Name = getPropertyName(PROP_INPUT_STREAM);

    uno::Reference< io::XInputStream > xIStream = new XInputStreamHelper( buf, len );
    aMediaProperties[0].Value <<= xIStream;

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

void GraphicImport::lcl_startShape(uno::Reference<drawing::XShape> const&)
{
}

void GraphicImport::lcl_endShape( )
{
}

bool GraphicImport::IsGraphic() const
{
    return m_pImpl->bIsGraphic;
}

sal_Int32 GraphicImport::GetLeftMarginOrig() const
{
    return m_pImpl->nLeftMarginOrig;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
