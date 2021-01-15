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
#include "BorderHandler.hxx"
#include "TDefTableHandler.hxx"
#include "PropertyMap.hxx"
#include "ConversionHelper.hxx"
#include <com/sun/star/table/BorderLine2.hpp>
#include <o3tl/enumarray.hxx>
#include <o3tl/enumrange.hxx>
#include <ooxml/resourceids.hxx>
#include <filter/msfilter/util.hxx>
#include <comphelper/sequence.hxx>
#include <tools/color.hxx>

namespace writerfilter::dmapper {

using namespace ::com::sun::star;


BorderHandler::BorderHandler( bool bOOXML ) :
LoggedProperties("BorderHandler"),
m_nLineWidth(15), // Word default, in twips
m_nLineType(0),
m_nLineColor(0),
m_nLineDistance(0),
m_bShadow(false),
m_bOOXML( bOOXML )
{
    m_aFilledLines.fill(false);
    m_aBorderLines.fill(table::BorderLine2());
}

BorderHandler::~BorderHandler()
{
}

void BorderHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch( rName )
    {
        case NS_ooxml::LN_CT_Border_sz:
            //  width of a single line in 1/8 pt, max of 32 pt -> twip * 5 / 2.
            m_nLineWidth = nIntValue * 5 / 2;
            appendGrabBag("sz", OUString::number(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_val:
            m_nLineType = nIntValue;
            appendGrabBag("val", TDefTableHandler::getBorderTypeString(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_color:
            m_nLineColor = nIntValue;
            appendGrabBag("color", OUString::fromUtf8(msfilter::util::ConvertColor(Color(ColorTransparency, nIntValue))));
        break;
        case NS_ooxml::LN_CT_Border_space: // border distance in points
            m_nLineDistance = ConversionHelper::convertTwipToMM100( nIntValue * 20 );
            appendGrabBag("space", OUString::number(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            m_bShadow = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_frame:
        case NS_ooxml::LN_CT_Border_themeTint:
            appendGrabBag("themeTint", OUString::number(nIntValue, 16));
            break;
        case NS_ooxml::LN_CT_Border_themeColor:
            appendGrabBag("themeColor", TDefTableHandler::getThemeColorTypeString(nIntValue));
            break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

void BorderHandler::lcl_sprm(Sprm & rSprm)
{
    BorderPosition pos;
    const bool rtl = false; // TODO detect
    OUString aBorderPos;
    switch( rSprm.getId())
    {
        case NS_ooxml::LN_CT_TblBorders_top:
            pos = BorderPosition::Top;
            aBorderPos = "top";
            break;
        case NS_ooxml::LN_CT_TblBorders_start:
            pos = rtl ? BorderPosition::Right : BorderPosition::Left;
            aBorderPos = "start";
            break;
        case NS_ooxml::LN_CT_TblBorders_left:
            pos = BorderPosition::Left;
            aBorderPos = "left";
            break;
        case NS_ooxml::LN_CT_TblBorders_bottom:
            pos = BorderPosition::Bottom;
            aBorderPos = "bottom";
            break;
        case NS_ooxml::LN_CT_TblBorders_end:
            pos = rtl ? BorderPosition::Left : BorderPosition::Right;
            aBorderPos = "end";
            break;
        case NS_ooxml::LN_CT_TblBorders_right:
            pos = BorderPosition::Right;
            aBorderPos = "right";
            break;
        case NS_ooxml::LN_CT_TblBorders_insideH:
            pos = BorderPosition::Horizontal;
            aBorderPos = "insideH";
            break;
        case NS_ooxml::LN_CT_TblBorders_insideV:
            pos = BorderPosition::Vertical;
            aBorderPos = "insideV";
            break;
        default:
            return;
    }
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties)
    {
        std::vector<beans::PropertyValue> aSavedGrabBag;
        if (!m_aInteropGrabBagName.isEmpty())
        {
            aSavedGrabBag = m_aInteropGrabBag;
            m_aInteropGrabBag.clear();
        }
        pProperties->resolve(*this);
        if (!m_aInteropGrabBagName.isEmpty())
        {
            aSavedGrabBag.push_back(getInteropGrabBag(aBorderPos));
            m_aInteropGrabBag = aSavedGrabBag;
        }
    }
    ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                           m_aBorderLines[ pos ], m_bOOXML );
    m_aFilledLines[ pos ] = true;
}

PropertyMapPtr  BorderHandler::getProperties()
{
    static const o3tl::enumarray<BorderPosition, PropertyIds> aPropNames =
    {
        PROP_TOP_BORDER,
        PROP_LEFT_BORDER,
        PROP_BOTTOM_BORDER,
        PROP_RIGHT_BORDER,
        META_PROP_HORIZONTAL_BORDER,
        META_PROP_VERTICAL_BORDER
    };
    PropertyMapPtr pPropertyMap(new PropertyMap);
    // don't fill in default properties
    if( m_bOOXML )
    {
        for( auto nProp: o3tl::enumrange<BorderPosition>())
        {
            if ( m_aFilledLines[nProp] ) {
                pPropertyMap->Insert( aPropNames[nProp], uno::makeAny( m_aBorderLines[nProp] ) );
            }
        }
    }
    return pPropertyMap;
}

table::BorderLine2 BorderHandler::getBorderLine()
{
    table::BorderLine2 aBorderLine;
    ConversionHelper::MakeBorderLine( m_nLineWidth, m_nLineType, m_nLineColor, aBorderLine, m_bOOXML );
    return aBorderLine;
}


void BorderHandler::enableInteropGrabBag(const OUString& aName)
{
    m_aInteropGrabBagName = aName;
}

beans::PropertyValue BorderHandler::getInteropGrabBag(const OUString& aName)
{
    beans::PropertyValue aRet;
    if (aName.isEmpty())
        aRet.Name = m_aInteropGrabBagName;
    else
        aRet.Name = aName;

    aRet.Value <<= comphelper::containerToSequence(m_aInteropGrabBag);
    return aRet;
}

void BorderHandler::appendGrabBag(const OUString& aKey, const OUString& aValue)
{
    beans::PropertyValue aProperty;
    aProperty.Name = aKey;
    aProperty.Value <<= aValue;
    m_aInteropGrabBag.push_back(aProperty);
}

} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
