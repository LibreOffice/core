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
#include <ooxml/resourceids.hxx>
#include <filter/msfilter/util.hxx>
#include <comphelper/sequence.hxx>

namespace writerfilter {

namespace dmapper {

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
    const int nBorderCount(BORDER_COUNT);
    std::fill_n(m_aFilledLines, nBorderCount, false);
    std::fill_n(m_aBorderLines, nBorderCount, table::BorderLine2());
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
            appendGrabBag("color", OUString::fromUtf8(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true)));
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
    BorderPosition pos = BORDER_COUNT; // invalid pos
    const bool rtl = false; // TODO detect
    OUString aBorderPos;
    switch( rSprm.getId())
    {
        case NS_ooxml::LN_CT_TblBorders_top:
            pos = BORDER_TOP;
            aBorderPos = "top";
            break;
        case NS_ooxml::LN_CT_TblBorders_start:
            pos = rtl ? BORDER_RIGHT : BORDER_LEFT;
            aBorderPos = "start";
            break;
        case NS_ooxml::LN_CT_TblBorders_left:
            pos = BORDER_LEFT;
            aBorderPos = "left";
            break;
        case NS_ooxml::LN_CT_TblBorders_bottom:
            pos = BORDER_BOTTOM;
            aBorderPos = "bottom";
            break;
        case NS_ooxml::LN_CT_TblBorders_end:
            pos = rtl ? BORDER_LEFT : BORDER_RIGHT;
            aBorderPos = "end";
            break;
        case NS_ooxml::LN_CT_TblBorders_right:
            pos = BORDER_RIGHT;
            aBorderPos = "right";
            break;
        case NS_ooxml::LN_CT_TblBorders_insideH:
            pos = BORDER_HORIZONTAL;
            aBorderPos = "insideH";
            break;
        case NS_ooxml::LN_CT_TblBorders_insideV:
            pos = BORDER_VERTICAL;
            aBorderPos = "insideV";
            break;
        default:
            break;
    }
    if( pos != BORDER_COUNT )
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
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
}

PropertyMapPtr  BorderHandler::getProperties()
{
    static const PropertyIds aPropNames[BORDER_COUNT] =
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
        for( sal_Int32 nProp = 0; nProp < BORDER_COUNT; ++nProp)
        {
            if ( m_aFilledLines[nProp] ) {
                pPropertyMap->Insert( aPropNames[nProp], uno::makeAny( m_aBorderLines[nProp] ) );
            }
        }
    }
    return pPropertyMap;
}
/*-------------------------------------------------------------------------
    used only in OOXML import
  -----------------------------------------------------------------------*/
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

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
