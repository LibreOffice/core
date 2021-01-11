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

#include "GraphicHelpers.hxx"
#include "TagLogger.hxx"
#include <dmapper/GraphicZOrderHelper.hxx>
#include "PropertyIds.hxx"

#include <ooxml/resourceids.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#include <oox/drawingml/drawingmltypes.hxx>
#include <sal/log.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <tools/diagnose_ex.h>

#include <iostream>

namespace writerfilter::dmapper {

using namespace com::sun::star;

PositionHandler::PositionHandler( std::pair<OUString, OUString>& rPositionOffsets, std::pair<OUString, OUString>& rAligns ) :
LoggedProperties("PositionHandler"),
m_nOrient(text::VertOrientation::NONE),
m_nRelation(text::RelOrientation::FRAME),
m_nPosition(0),
m_rPositionOffsets(rPositionOffsets),
m_rAligns(rAligns)
{
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
                switch ( nIntValue )
                {
                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_margin:
                        m_nRelation = text::RelOrientation::PAGE_PRINT_AREA;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_page:
                        m_nRelation = text::RelOrientation::PAGE_FRAME;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_topMargin:
                        m_nRelation = text::RelOrientation::PAGE_PRINT_AREA_TOP;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_bottomMargin:
                        m_nRelation = text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_paragraph:
                        m_nRelation = text::RelOrientation::FRAME;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromV_line:
                        m_nRelation = text::RelOrientation::TEXT_LINE;
                        break;

                    // TODO There are some other unhandled values
                    default:
                        SAL_WARN("writerfilter", "unhandled case (" << nIntValue << ") in NS_ooxml::LN_CT_PosV_relativeFrom");
                }
            }
            break;

        case NS_ooxml::LN_CT_PosH_relativeFrom:
            {
                switch ( nIntValue )
                {
                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_margin:
                        m_nRelation = text::RelOrientation::PAGE_PRINT_AREA;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_page:
                        m_nRelation =  text::RelOrientation::PAGE_FRAME;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_insideMargin:
                        m_nRelation = text::RelOrientation::PAGE_FRAME;
                        m_bPageToggle = true;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_column:
                        m_nRelation = text::RelOrientation::FRAME;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_character:
                        m_nRelation = text::RelOrientation::CHAR;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_leftMargin:
                        m_nRelation = text::RelOrientation::PAGE_LEFT;
                        break;

                    case NS_ooxml::LN_Value_wordprocessingDrawing_ST_RelFromH_rightMargin:
                        m_nRelation = text::RelOrientation::PAGE_RIGHT;
                        break;

                    // TODO There are some other unhandled values
                    default:
                        SAL_WARN("writerfilter", "unhandled case (" << nIntValue << ") in NS_ooxml::LN_CT_PosH_relativeFrom");
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

void PositionHandler::lcl_sprm(Sprm& rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();

    switch (nSprmId)
    {
        case NS_ooxml::LN_CT_PosH_posOffset:
            m_nPosition = oox::drawingml::convertEmuToHmm(m_rPositionOffsets.first.toInt32());
            m_rPositionOffsets.first.clear();
            break;
        case NS_ooxml::LN_CT_PosV_posOffset:
            m_nPosition = oox::drawingml::convertEmuToHmm(m_rPositionOffsets.second.toInt32());
            m_rPositionOffsets.second.clear();
            break;
        case NS_ooxml::LN_CT_PosH_align:
        {
            OUString& rAlign = m_rAligns.first;
            if (rAlign == "left")
                m_nOrient = text::HoriOrientation::LEFT;
            else if (rAlign == "right")
                m_nOrient = text::HoriOrientation::RIGHT;
            else if (rAlign == "center")
                m_nOrient = text::HoriOrientation::CENTER;
            else if (rAlign == "inside")
                m_nOrient = text::HoriOrientation::INSIDE;
            else if (rAlign == "outside")
                m_nOrient = text::HoriOrientation::OUTSIDE;
            rAlign.clear();
            break;
        }
        case NS_ooxml::LN_CT_PosV_align:
        {
            OUString& rAlign = m_rAligns.second;
            if (rAlign == "top")
                m_nOrient = text::VertOrientation::TOP;
            else if (rAlign == "bottom")
                m_nOrient = text::VertOrientation::BOTTOM;
            else if (rAlign == "center")
                m_nOrient = text::VertOrientation::CENTER;
            else if (rAlign == "inside" && m_nRelation == text::RelOrientation::PAGE_PRINT_AREA_BOTTOM)
                m_nOrient = text::VertOrientation::TOP;
            else if (rAlign == "outside" && m_nRelation == text::RelOrientation::PAGE_PRINT_AREA_BOTTOM)
                m_nOrient = text::VertOrientation::BOTTOM;
            rAlign.clear();
            break;
        }
    }
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

WrapHandler::WrapHandler( ) :
LoggedProperties("WrapHandler"),
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

text::WrapTextMode WrapHandler::getWrapMode( ) const
{
    // The wrap values do not map directly to our wrap mode,
    // e.g. none in .docx actually means through in LO.
    text::WrapTextMode nMode = text::WrapTextMode_THROUGH;

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
                        nMode = text::WrapTextMode_LEFT;
                        break;
                    case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapSide_right:
                        nMode = text::WrapTextMode_RIGHT;
                        break;
                    default:
                        nMode = text::WrapTextMode_PARALLEL;
                }
            }
            break;
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_topAndBottom:
            nMode = text::WrapTextMode_NONE;
            break;
        case NS_ooxml::LN_Value_vml_wordprocessingDrawing_ST_WrapType_none:
        default:
            nMode = text::WrapTextMode_THROUGH;
    }

    return nMode;
}


void GraphicZOrderHelper::addItem(uno::Reference<beans::XPropertySet> const& props, sal_Int32 const relativeHeight)
{
    items[ relativeHeight ] = props;
}

// The relativeHeight value in .docx is an arbitrary number, where only the relative ordering matters.
// But in Writer, the z-order is index in 0..(numitems-1) range, so whenever a new item needs to be
// added in the proper z-order, it is necessary to find the proper index.
sal_Int32 GraphicZOrderHelper::findZOrder( sal_Int32 relativeHeight, bool bOldStyle )
{
    // std::map is iterated sorted by key
    auto it = std::find_if(items.cbegin(), items.cend(),
        [relativeHeight, bOldStyle](const Items::value_type& rItem) {
            // Old-style ordering differs in what should happen when there is already an item with the same z-order:
            // we belong under it in case of new-style, but we belong above it in case of old-style.
            return bOldStyle ? (rItem.first > relativeHeight) : (rItem.first >= relativeHeight);
        }
    );
    sal_Int32 itemZOrderOffset(0); // before the item
    if( it == items.end()) // we're topmost
    {
        if( items.empty())
            return 0;
        --it;
        itemZOrderOffset = 1; // after the topmost

        // Check if this shape has a textbox. If so, the textbox will have its own ZOrder, so
        // suggest a larger offset.
        bool bTextBox = false;
        uno::Reference<beans::XPropertySet> xShape = it->second;
        uno::Reference<beans::XPropertySetInfo> xInfo = xShape->getPropertySetInfo();
        if (xInfo->hasPropertyByName("TextBox"))
        {
            xShape->getPropertyValue("TextBox") >>= bTextBox;
        }
        if (bTextBox)
        {
            ++itemZOrderOffset;
        }
    }
    // SwXFrame::getPropertyValue throws uno::RuntimeException
    // when its GetFrameFormat() returns nullptr
    try {
        sal_Int32 itemZOrder(0);
        if( it->second->getPropertyValue(getPropertyName( PROP_Z_ORDER )) >>= itemZOrder )
            return itemZOrder + itemZOrderOffset;
    }
    catch (const uno::RuntimeException&) {
        TOOLS_WARN_EXCEPTION("writerfilter", "Exception when getting item z-order");
    }
    SAL_WARN( "writerfilter", "findZOrder() didn't find item z-order" );
    return 0; // this should not(?) happen
}

GraphicNamingHelper::GraphicNamingHelper()
    : m_nCounter(0)
{
}

OUString GraphicNamingHelper::NameGraphic(const OUString& rTemplate)
{
    OUString aRet = rTemplate;

    if (aRet.isEmpty())
    {
        // Empty template: then auto-generate a unique name.
        OUString aPrefix(SvxResId(STR_ObjNameSingulGRAF));
        aRet += aPrefix + OUString::number(++m_nCounter);
    }

    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
