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
#include "ConversionHelper.hxx"
#include "GraphicHelpers.hxx"

#include <ooxml/resourceids.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>

#include "dmapperLoggers.hxx"

#include <iostream>
using namespace std;

namespace writerfilter {
namespace dmapper {

using namespace com::sun::star;

int PositionHandler::savedPositionOffsetV = 0;
int PositionHandler::savedPositionOffsetH = 0;
int PositionHandler::savedAlignV = text::VertOrientation::NONE;
int PositionHandler::savedAlignH = text::HoriOrientation::NONE;

PositionHandler::PositionHandler( bool vertical ) :
LoggedProperties(dmapper_logger, "PositionHandler")
{
    m_nRelation = text::RelOrientation::FRAME;
    if( vertical )
    {
        m_nPosition = savedPositionOffsetV;
        m_nOrient = savedAlignV;
        savedPositionOffsetV = 0;
        savedAlignV = text::VertOrientation::NONE;
    }
    else
    {
        m_nPosition = savedPositionOffsetH;
        m_nOrient = savedAlignH;
        savedPositionOffsetH = 0;
        savedAlignH = text::HoriOrientation::NONE;
    }
}

PositionHandler::~PositionHandler( )
{
}

void PositionHandler::lcl_attribute( Id aName, Value& rVal )
{
    sal_Int32 nIntValue = rVal.getInt( );
    switch ( aName )
    {
        case NS_ooxml::LN_CT_PosV_relativeFrom:
            {
                // TODO There are some other unhandled values
                static const Id pVertRelValues[] =
                {
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_margin,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_page,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_paragraph,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_line
                };

                static const sal_Int16 pVertRelations[] =
                {
                    text::RelOrientation::PAGE_PRINT_AREA,
                    text::RelOrientation::PAGE_FRAME,
                    text::RelOrientation::FRAME,
                    text::RelOrientation::TEXT_LINE
                };

                for ( int i = 0; i < 4; i++ )
                {
                    if ( pVertRelValues[i] == sal_uInt32( nIntValue ) )
                        m_nRelation = pVertRelations[i];
                }
            }
            break;
        case NS_ooxml::LN_CT_PosH_relativeFrom:
            {
                // TODO There are some other unhandled values
                static const Id pHoriRelValues[] =
                {
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_margin,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_page,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_column,
                    NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_character
                };

                static const sal_Int16 pHoriRelations[] =
                {
                    text::RelOrientation::PAGE_PRINT_AREA,
                    text::RelOrientation::PAGE_FRAME,
                    text::RelOrientation::FRAME,
                    text::RelOrientation::CHAR,
                };

                for ( int i = 0; i < 4; i++ )
                {
                    if ( pHoriRelValues[i] == sal_uInt32( nIntValue ) )
                        m_nRelation = pHoriRelations[i];
                }
            }
            break;
        default:
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->element("unhandled");
#endif
            break;
    }
}

void PositionHandler::lcl_sprm( Sprm& )
{
}

sal_Int16 PositionHandler::orientation() const
{
    if( m_nRelation == text::RelOrientation::TEXT_LINE )
    { // It appears that to 'line of text' alignment is backwards to other alignments,
      // 'top' meaning putting on top of the line instead of having top at the line.
        if( m_nOrient == text::VertOrientation::TOP )
            return text::VertOrientation::BOTTOM;
        else if( m_nOrient == text::VertOrientation::BOTTOM )
            return text::VertOrientation::TOP;
    }
    return m_nOrient;
}

sal_Int16 PositionHandler::relation() const
{
    return m_nRelation;
}

sal_Int32 PositionHandler::position() const
{
    return m_nPosition;
}

void PositionHandler::setPositionOffset(const OUString & sText, bool vertical)
{
    if( vertical )
        savedPositionOffsetV = ConversionHelper::convertEMUToMM100( sText.toInt32());
    else
        savedPositionOffsetH = ConversionHelper::convertEMUToMM100( sText.toInt32());
}

void PositionHandler::setAlignH(const OUString & sText)
{
    if( sText == "left")
        savedAlignH = text::HoriOrientation::LEFT;
    else if( sText == "right" )
        savedAlignH = text::HoriOrientation::RIGHT;
    else if( sText == "center" )
        savedAlignH = text::HoriOrientation::CENTER;
    else if( sText == "inside" )
        savedAlignH = text::HoriOrientation::INSIDE;
    else if( sText == "outside" )
        savedAlignH = text::HoriOrientation::OUTSIDE;
}

void PositionHandler::setAlignV(const OUString & sText)
{
    if( sText == "top" )
        savedAlignV = text::VertOrientation::TOP;
    else if( sText == "bottom" )
        savedAlignV = text::VertOrientation::BOTTOM;
    else if( sText == "center" )
        savedAlignV = text::VertOrientation::CENTER;
    else if( sText == "inside" )
        savedAlignV = text::VertOrientation::NONE;
    else if( sText == "outside" )
        savedAlignV = text::VertOrientation::NONE;
}

WrapHandler::WrapHandler( ) :
LoggedProperties(dmapper_logger, "WrapHandler"),
    m_nType( 0 ),
    m_nSide( 0 )
{
}

WrapHandler::~WrapHandler( )
{
}

void WrapHandler::lcl_attribute( Id aName, Value& rVal )
{
    switch ( aName )
    {
        case NS_ooxml::LN_CT_Wrap_type:
            m_nType = sal_Int32( rVal.getInt( ) );
            break;
        case NS_ooxml::LN_CT_Wrap_side:
            m_nSide = sal_Int32( rVal.getInt( ) );
            break;
        default:;
    }
}

void WrapHandler::lcl_sprm( Sprm& )
{
}

sal_Int32 WrapHandler::getWrapMode( )
{
    // The wrap values do not map directly to our wrap mode,
    // e.g. none in .docx actually means through in LO.
    sal_Int32 nMode = com::sun::star::text::WrapTextMode_THROUGHT;

    switch ( m_nType )
    {
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_square:
        // through and tight are somewhat complicated, approximate
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_tight:
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_through:
            {
                switch ( m_nSide )
                {
                    case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapSide_left:
                        nMode = com::sun::star::text::WrapTextMode_LEFT;
                        break;
                    case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapSide_right:
                        nMode = com::sun::star::text::WrapTextMode_RIGHT;
                        break;
                    default:
                        nMode = com::sun::star::text::WrapTextMode_PARALLEL;
                }
            }
            break;
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_topAndBottom:
            nMode = com::sun::star::text::WrapTextMode_NONE;
            break;
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_none:
        default:
            nMode = com::sun::star::text::WrapTextMode_THROUGHT;
    }

    return nMode;
}


void GraphicZOrderHelper::addItem( uno::Reference< beans::XPropertySet > props, sal_Int32 relativeHeight )
{
    items[ relativeHeight ] = props;
}

// The relativeHeight value in .docx is an arbitrary number, where only the relative ordering matters.
// But in Writer, the z-order is index in 0..(numitems-1) range, so whenever a new item needs to be
// added in the proper z-order, it is necessary to find the proper index.
sal_Int32 GraphicZOrderHelper::findZOrder( sal_Int32 relativeHeight )
{
    Items::const_iterator it = items.begin();
    while( it != items.end())
    {
        // std::map is iterated sorted by key
        // if there is an item that has the same z-order, we belong under it
        if( it->first >= relativeHeight )
            break; // this is the first one higher, we belong right before it
        else
            ++it;
    }
    if( it == items.end()) // we're topmost
    {
        if( items.empty())
            return 0;
        sal_Int32 itemZOrder(0);
        --it;
        if( it->second->getPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier()
            .GetName( PROP_Z_ORDER )) >>= itemZOrder )
            return itemZOrder + 1; // after the topmost
    }
    else
    {
        sal_Int32 itemZOrder(0);
        if( it->second->getPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier()
            .GetName( PROP_Z_ORDER )) >>= itemZOrder )
            return itemZOrder; // before the item
    }
    SAL_WARN( "writerfilter", "findZOrder() didn't find item z-order" );
    return 0; // this should not(?) happen
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
