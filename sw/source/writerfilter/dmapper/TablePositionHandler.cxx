/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "TablePositionHandler.hxx"
#include "ConversionHelper.hxx"
#include "TagLogger.hxx"
#include <ooxml/resourceids.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <comphelper/sequenceashashmap.hxx>

namespace writerfilter::dmapper
{
using namespace ::com::sun::star;

TablePositionHandler::TablePositionHandler()
    : LoggedProperties("TablePositionHandler")
{
}

TablePositionHandler::~TablePositionHandler() = default;

void TablePositionHandler::lcl_attribute(Id nId, Value& rVal)
{
    switch (nId)
    {
        case NS_ooxml::LN_CT_TblPPr_vertAnchor:
            m_aVertAnchor = rVal.getString();
            break;
        case NS_ooxml::LN_CT_TblPPr_tblpYSpec:
            m_aYSpec = rVal.getString();
            break;
        case NS_ooxml::LN_CT_TblPPr_horzAnchor:
            m_aHorzAnchor = rVal.getString();
            break;
        case NS_ooxml::LN_CT_TblPPr_tblpXSpec:
            m_aXSpec = rVal.getString();
            break;
        case NS_ooxml::LN_CT_TblPPr_tblpY:
            m_nY = rVal.getInt();
            break;
        case NS_ooxml::LN_CT_TblPPr_tblpX:
            m_nX = rVal.getInt();
            break;
        case NS_ooxml::LN_CT_TblPPr_leftFromText:
            m_nLeftFromText = rVal.getInt();
            break;
        case NS_ooxml::LN_CT_TblPPr_rightFromText:
            m_nRightFromText = rVal.getInt();
            break;
        case NS_ooxml::LN_CT_TblPPr_topFromText:
            m_nTopFromText = rVal.getInt();
            break;
        case NS_ooxml::LN_CT_TblPPr_bottomFromText:
            m_nBottomFromText = rVal.getInt();
            break;
        default:
#ifdef DBG_UTIL
            TagLogger::getInstance().element("unhandled");
#endif
            break;
    }
}

void TablePositionHandler::lcl_sprm(Sprm& /*rSprm*/) {}

uno::Sequence<beans::PropertyValue> TablePositionHandler::getTablePosition() const
{
    comphelper::SequenceAsHashMap aFrameProperties;

    aFrameProperties["LeftBorderDistance"] <<= sal_Int32(0);
    aFrameProperties["RightBorderDistance"] <<= sal_Int32(0);
    aFrameProperties["TopBorderDistance"] <<= sal_Int32(0);
    aFrameProperties["BottomBorderDistance"] <<= sal_Int32(0);

    aFrameProperties["LeftMargin"] <<= ConversionHelper::convertTwipToMM100(m_nLeftFromText);
    aFrameProperties["RightMargin"] <<= ConversionHelper::convertTwipToMM100(m_nRightFromText);
    aFrameProperties["TopMargin"] <<= ConversionHelper::convertTwipToMM100(m_nTopFromText);
    aFrameProperties["BottomMargin"] <<= ConversionHelper::convertTwipToMM100(m_nBottomFromText);

    table::BorderLine2 aEmptyBorder;
    aFrameProperties["TopBorder"] <<= aEmptyBorder;
    aFrameProperties["BottomBorder"] <<= aEmptyBorder;
    aFrameProperties["LeftBorder"] <<= aEmptyBorder;
    aFrameProperties["RightBorder"] <<= aEmptyBorder;

    // Horizontal positioning
    sal_Int16 nHoriOrient = text::HoriOrientation::NONE;
    if (m_aXSpec == "center")
        nHoriOrient = text::HoriOrientation::CENTER;
    else if (m_aXSpec == "inside")
        nHoriOrient = text::HoriOrientation::INSIDE;
    else if (m_aXSpec == "left")
        nHoriOrient = text::HoriOrientation::LEFT;
    else if (m_aXSpec == "outside")
        nHoriOrient = text::HoriOrientation::OUTSIDE;
    else if (m_aXSpec == "right")
        nHoriOrient = text::HoriOrientation::RIGHT;

    sal_Int16 nHoriOrientRelation;
    if (m_aHorzAnchor == "margin")
        nHoriOrientRelation = text::RelOrientation::PAGE_PRINT_AREA;
    else if (m_aHorzAnchor == "page")
        nHoriOrientRelation = text::RelOrientation::PAGE_FRAME;
    else if (m_aHorzAnchor == "text")
        nHoriOrientRelation = text::RelOrientation::FRAME;

    aFrameProperties["HoriOrient"] <<= nHoriOrient;
    aFrameProperties["HoriOrientRelation"] <<= nHoriOrientRelation;
    aFrameProperties["HoriOrientPosition"] <<= ConversionHelper::convertTwipToMM100(m_nX);

    // Vertical positioning
    sal_Int16 nVertOrient = text::VertOrientation::NONE;
    if (m_aYSpec == "bottom")
        nVertOrient = text::VertOrientation::BOTTOM;
    else if (m_aYSpec == "center")
        nVertOrient = text::VertOrientation::CENTER;
    else if (m_aYSpec == "top")
        nVertOrient = text::VertOrientation::TOP;
    // TODO There are a few cases we can't map ATM.

    sal_Int16 nVertOrientRelation;
    if (m_aVertAnchor == "margin")
        nVertOrientRelation = text::RelOrientation::PAGE_PRINT_AREA;
    else if (m_aVertAnchor == "page")
        nVertOrientRelation = text::RelOrientation::PAGE_FRAME;
    else if (m_aVertAnchor == "text")
        nVertOrientRelation = text::RelOrientation::FRAME;

    aFrameProperties["VertOrient"] <<= nVertOrient;
    aFrameProperties["VertOrientRelation"] <<= nVertOrientRelation;
    aFrameProperties["VertOrientPosition"] <<= ConversionHelper::convertTwipToMM100(m_nY);
    aFrameProperties["FillTransparence"] <<= sal_Int32(100);

    if (m_nTableOverlap == NS_ooxml::LN_Value_ST_TblOverlap_never)
    {
        // NS_ooxml::LN_Value_ST_TblOverlap_overlap is the default, both in OOXML and in Writer.
        aFrameProperties["AllowOverlap"] <<= false;
    }

    return aFrameProperties.getAsConstPropertyValueList();
}

bool TablePositionHandler::operator==(const TablePositionHandler& rHandler) const
{
    return m_aVertAnchor == rHandler.m_aVertAnchor && m_aYSpec == rHandler.m_aYSpec
           && m_aHorzAnchor == rHandler.m_aHorzAnchor && m_aXSpec == rHandler.m_aXSpec
           && m_nY == rHandler.m_nY && m_nX == rHandler.m_nX;
}

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
