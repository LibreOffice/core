/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <TablePositionHandler.hxx>
#include <DomainMapperTableHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

TablePositionHandler::TablePositionHandler() :
    LoggedProperties(dmapper_logger, "TablePositionHandler"),
    m_aVertAnchor( "margin" ),
    m_aYSpec( ),
    m_aHorzAnchor( "text" ),
    m_aXSpec( ),
    m_nY( 0 ),
    m_nX( 0 ),
    m_nLeftFromText(0),
    m_nRightFromText(0),
    m_nTopFromText(0),
    m_nBottomFromText(0)
{
}

TablePositionHandler::~TablePositionHandler()
{
}


void TablePositionHandler::lcl_attribute(Id rName, Value& rVal)
{
    switch (rName)
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
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->element("unhandled");
#endif
            break;
    }
}


void TablePositionHandler::lcl_sprm(Sprm& /*rSprm*/)
{
}


uno::Sequence<beans::PropertyValue> TablePositionHandler::getTablePosition() const
{
    uno::Sequence< beans::PropertyValue > aFrameProperties(18);
    beans::PropertyValue* pFrameProperties = aFrameProperties.getArray();

    pFrameProperties[0].Name = "LeftBorderDistance";
    pFrameProperties[0].Value <<= sal_Int32(0);
    pFrameProperties[1].Name = "RightBorderDistance";
    pFrameProperties[1].Value <<= sal_Int32(0);
    pFrameProperties[2].Name = "TopBorderDistance";
    pFrameProperties[2].Value <<= sal_Int32(0);
    pFrameProperties[3].Name = "BottomBorderDistance";
    pFrameProperties[3].Value <<= sal_Int32(0);

    pFrameProperties[4].Name = "LeftMargin";
    pFrameProperties[4].Value <<= ConversionHelper::convertTwipToMM100(m_nLeftFromText);
    pFrameProperties[5].Name = "RightMargin";
    pFrameProperties[5].Value <<= ConversionHelper::convertTwipToMM100(m_nRightFromText);
    pFrameProperties[6].Name = "TopMargin";
    pFrameProperties[6].Value <<= ConversionHelper::convertTwipToMM100(m_nTopFromText);
    pFrameProperties[7].Name = "BottomMargin";
    pFrameProperties[7].Value <<= ConversionHelper::convertTwipToMM100(m_nBottomFromText);

    table::BorderLine2 aEmptyBorder;
    pFrameProperties[8].Name = "TopBorder";
    pFrameProperties[8].Value <<= aEmptyBorder;
    pFrameProperties[9].Name = "BottomBorder";
    pFrameProperties[9].Value <<= aEmptyBorder;
    pFrameProperties[10].Name = "LeftBorder";
    pFrameProperties[10].Value <<= aEmptyBorder;
    pFrameProperties[11].Name = "RightBorder";
    pFrameProperties[11].Value <<= aEmptyBorder;

    // Horizontal positioning
    sal_Int16 nHoriOrient = text::HoriOrientation::NONE;
    if ( m_aXSpec == "center" )
        nHoriOrient = text::HoriOrientation::CENTER;
    else if ( m_aXSpec == "inside" )
        nHoriOrient = text::HoriOrientation::INSIDE;
    else if ( m_aXSpec == "left" )
        nHoriOrient = text::HoriOrientation::LEFT;
    else if ( m_aXSpec == "outside" )
        nHoriOrient = text::HoriOrientation::OUTSIDE;
    else if ( m_aXSpec == "right" )
        nHoriOrient = text::HoriOrientation::RIGHT;

    sal_Int16 nHoriOrientRelation;
    if ( m_aHorzAnchor == "margin" )
        nHoriOrientRelation = text::RelOrientation::PAGE_PRINT_AREA;
    else if ( m_aHorzAnchor == "page" )
        nHoriOrientRelation = text::RelOrientation::PAGE_FRAME;
    else if ( m_aHorzAnchor == "text" )
        nHoriOrientRelation = text::RelOrientation::FRAME;

    pFrameProperties[12].Name = "HoriOrient";
    pFrameProperties[12].Value <<= nHoriOrient;
    pFrameProperties[13].Name = "HoriOrientRelation";
    pFrameProperties[13].Value <<= nHoriOrientRelation;
    pFrameProperties[14].Name = "HoriOrientPosition";
    pFrameProperties[14].Value <<= m_nX;


    // Vertical positioning
    sal_Int16 nVertOrient = text::VertOrientation::NONE;
    if ( m_aYSpec == "bottom" )
        nVertOrient = text::VertOrientation::BOTTOM;
    else if ( m_aYSpec == "center" )
        nVertOrient = text::VertOrientation::CENTER;
    else if ( m_aYSpec == "top" )
        nVertOrient = text::VertOrientation::TOP;
    // TODO There are a few cases we can't map ATM.


    sal_Int16 nVertOrientRelation;
    if ( m_aVertAnchor == "margin" )
        nVertOrientRelation = text::RelOrientation::PAGE_PRINT_AREA;
    else if ( m_aVertAnchor == "page" )
        nVertOrientRelation = text::RelOrientation::PAGE_FRAME;
    else if ( m_aVertAnchor == "text" )
        nVertOrientRelation = text::RelOrientation::FRAME;

    pFrameProperties[15].Name = "VertOrient";
    pFrameProperties[15].Value <<= nVertOrient;
    pFrameProperties[16].Name = "VertOrientRelation";
    pFrameProperties[16].Value <<= nVertOrientRelation;
    pFrameProperties[17].Name = "VertOrientPosition";
    pFrameProperties[17].Value <<= m_nY;

    return aFrameProperties;
}

bool TablePositionHandler::operator== (const TablePositionHandler& rHandler) const
{
    return m_aVertAnchor == rHandler.m_aVertAnchor &&
           m_aYSpec == rHandler.m_aYSpec &&
           m_aHorzAnchor == rHandler.m_aHorzAnchor &&
           m_aXSpec == rHandler.m_aXSpec &&
           m_nY == rHandler.m_nY &&
           m_nX == rHandler.m_nX;
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
