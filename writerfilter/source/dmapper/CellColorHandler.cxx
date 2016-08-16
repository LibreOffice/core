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
#include <CellColorHandler.hxx>
#include <PropertyMap.hxx>
#include <ConversionHelper.hxx>
#include <TDefTableHandler.hxx>
#include <ooxml/resourceids.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/ShadingPattern.hpp>
#include <sal/macros.h>
#include <filter/msfilter/util.hxx>
#include <comphelper/sequence.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

CellColorHandler::CellColorHandler() :
LoggedProperties("CellColorHandler"),
m_nShadingPattern( drawing::ShadingPattern::CLEAR ),
m_nColor( 0xffffffff ),
m_nFillColor( 0xffffffff ),
m_bAutoFillColor( true ),
    m_OutputFormat( Form )
{
}

CellColorHandler::~CellColorHandler()
{
}

// ST_Shd strings are converted to integers by the tokenizer, store strings in
// the InteropGrabBag
uno::Any lcl_ConvertShd(sal_Int32 nIntValue)
{
    OUString aRet;
    // This should be in sync with the ST_Shd list in ooxml's model.xml.
    switch (nIntValue)
    {
        case NS_ooxml::LN_Value_ST_Shd_clear: aRet = "clear"; break;
        case NS_ooxml::LN_Value_ST_Shd_solid: aRet = "solid"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct5: aRet = "pct5"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct10: aRet = "pct10"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct20: aRet = "pct20"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct25: aRet = "pct25"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct30: aRet = "pct30"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct40: aRet = "pct40"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct50: aRet = "pct50"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct60: aRet = "pct60"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct70: aRet = "pct70"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct75: aRet = "pct75"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct80: aRet = "pct80"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct90: aRet = "pct90"; break;
        case NS_ooxml::LN_Value_ST_Shd_horzStripe: aRet = "horzStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_vertStripe: aRet = "vertStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_reverseDiagStripe: aRet = "reverseDiagStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_diagStripe: aRet = "diagStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_horzCross: aRet = "horzCross"; break;
        case NS_ooxml::LN_Value_ST_Shd_diagCross: aRet = "diagCross"; break;
        case NS_ooxml::LN_Value_ST_Shd_thinHorzStripe: aRet = "thinHorzStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_thinVertStripe: aRet = "thinVertStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_thinReverseDiagStripe: aRet = "thinReverseDiagStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_thinDiagStripe: aRet = "thinDiagStripe"; break;
        case NS_ooxml::LN_Value_ST_Shd_thinHorzCross: aRet = "thinHorzCross"; break;
        case NS_ooxml::LN_Value_ST_Shd_thinDiagCross: aRet = "thinDiagCross"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct12: aRet = "pct12"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct15: aRet = "pct15"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct35: aRet = "pct35"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct37: aRet = "pct37"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct45: aRet = "pct45"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct55: aRet = "pct55"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct62: aRet = "pct62"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct65: aRet = "pct65"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct85: aRet = "pct85"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct87: aRet = "pct87"; break;
        case NS_ooxml::LN_Value_ST_Shd_pct95: aRet = "pct95"; break;
        case NS_ooxml::LN_Value_ST_Shd_nil: aRet = "nil"; break;
    }
    return uno::makeAny(aRet);
}

void CellColorHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch( rName )
    {
        case NS_ooxml::LN_CT_Shd_val:
        {
            createGrabBag("val", lcl_ConvertShd(nIntValue));
            m_nShadingPattern = nIntValue;
        }
        break;
        case NS_ooxml::LN_CT_Shd_fill:
            createGrabBag("fill", uno::makeAny(OUString::fromUtf8(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true))));
            if( nIntValue == OOXML_COLOR_AUTO )
                nIntValue = 0xffffff; //fill color auto means white
            else
                m_bAutoFillColor = false;

            m_nFillColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_Shd_color:
            createGrabBag("color", uno::makeAny(OUString::fromUtf8(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true))));
            if( nIntValue == OOXML_COLOR_AUTO )
                nIntValue = 0; //shading color auto means black
            //color of the shading
            m_nColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_Shd_themeFill:
            createGrabBag("themeFill", uno::makeAny(TDefTableHandler::getThemeColorTypeString(nIntValue)));
        break;
        case NS_ooxml::LN_CT_Shd_themeFillShade:
            createGrabBag("themeFillShade", uno::makeAny(OUString::number(nIntValue, 16)));
        break;
        case NS_ooxml::LN_CT_Shd_themeFillTint:
            createGrabBag("themeFillTint", uno::makeAny(OUString::number(nIntValue, 16)));
            break;
        case NS_ooxml::LN_CT_Shd_themeColor:
            createGrabBag("themeColor", uno::makeAny(TDefTableHandler::getThemeColorTypeString(nIntValue)));
        break;
        case NS_ooxml::LN_CT_Shd_themeShade:
            createGrabBag("themeShade", uno::makeAny(OUString::number(nIntValue, 16)));
        break;
        case NS_ooxml::LN_CT_Shd_themeTint:
            createGrabBag("themeTint", uno::makeAny(OUString::number(nIntValue, 16)));
            break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

void CellColorHandler::lcl_sprm(Sprm & rSprm)
{
    (void)rSprm;
}

TablePropertyMapPtr  CellColorHandler::getProperties()
{
    TablePropertyMapPtr pPropertyMap(new TablePropertyMap);

    // Code from binary word filter (the values are out of 1000)
    sal_Int32 nWW8BrushStyle = 0;
    switch (m_nShadingPattern)
    {
        // Clear-Brush
        case NS_ooxml::LN_Value_ST_Shd_clear: nWW8BrushStyle = 0; break;
        // Solid-Brush
        case NS_ooxml::LN_Value_ST_Shd_solid: nWW8BrushStyle = 1000; break;
        // Percent values
        case NS_ooxml::LN_Value_ST_Shd_pct5: nWW8BrushStyle = 50; break;
        case NS_ooxml::LN_Value_ST_Shd_pct10: nWW8BrushStyle = 100; break;
        case NS_ooxml::LN_Value_ST_Shd_pct20: nWW8BrushStyle = 200; break;
        case NS_ooxml::LN_Value_ST_Shd_pct25: nWW8BrushStyle = 250; break;
        case NS_ooxml::LN_Value_ST_Shd_pct30: nWW8BrushStyle = 300; break;
        case NS_ooxml::LN_Value_ST_Shd_pct40: nWW8BrushStyle = 400; break;
        case NS_ooxml::LN_Value_ST_Shd_pct50: nWW8BrushStyle = 500; break;
        case NS_ooxml::LN_Value_ST_Shd_pct60: nWW8BrushStyle = 600; break;
        case NS_ooxml::LN_Value_ST_Shd_pct70: nWW8BrushStyle = 700; break;
        case NS_ooxml::LN_Value_ST_Shd_pct75: nWW8BrushStyle = 750; break;
        case NS_ooxml::LN_Value_ST_Shd_pct80: nWW8BrushStyle = 800; break;
        case NS_ooxml::LN_Value_ST_Shd_pct90: nWW8BrushStyle = 900; break;
        // Special cases
        case NS_ooxml::LN_Value_ST_Shd_horzStripe: nWW8BrushStyle = 333; break; // Dark Horizontal
        case NS_ooxml::LN_Value_ST_Shd_vertStripe: nWW8BrushStyle = 333; break; // Dark Vertical
        case NS_ooxml::LN_Value_ST_Shd_reverseDiagStripe: nWW8BrushStyle = 333; break; // Dark Forward Diagonal
        case NS_ooxml::LN_Value_ST_Shd_diagStripe: nWW8BrushStyle = 333; break; // Dark Backward Diagonal
        case NS_ooxml::LN_Value_ST_Shd_horzCross: nWW8BrushStyle = 333; break; // Dark Cross
        case NS_ooxml::LN_Value_ST_Shd_diagCross: nWW8BrushStyle = 333; break; // Dark Diagonal Cross
        case NS_ooxml::LN_Value_ST_Shd_thinHorzStripe: nWW8BrushStyle = 333; break; // Horizontal
        case NS_ooxml::LN_Value_ST_Shd_thinVertStripe: nWW8BrushStyle = 333; break; // Vertical
        case NS_ooxml::LN_Value_ST_Shd_thinReverseDiagStripe: nWW8BrushStyle = 333; break; // Forward Diagonal
        case NS_ooxml::LN_Value_ST_Shd_thinDiagStripe: nWW8BrushStyle = 333; break; // Backward Diagonal
        case NS_ooxml::LN_Value_ST_Shd_thinHorzCross: nWW8BrushStyle = 333; break; // Cross
        case NS_ooxml::LN_Value_ST_Shd_thinDiagCross: nWW8BrushStyle = 333; break;   // 25   Diagonal Cross
        // Different shading types
        case NS_ooxml::LN_Value_ST_Shd_pct12: nWW8BrushStyle = 125; break;
        case NS_ooxml::LN_Value_ST_Shd_pct15: nWW8BrushStyle = 150; break;
        case NS_ooxml::LN_Value_ST_Shd_pct35: nWW8BrushStyle = 350; break;
        case NS_ooxml::LN_Value_ST_Shd_pct37: nWW8BrushStyle = 375; break;
        case NS_ooxml::LN_Value_ST_Shd_pct45: nWW8BrushStyle = 450; break;
        case NS_ooxml::LN_Value_ST_Shd_pct55: nWW8BrushStyle = 550; break;
        case NS_ooxml::LN_Value_ST_Shd_pct62: nWW8BrushStyle = 625; break;
        case NS_ooxml::LN_Value_ST_Shd_pct65: nWW8BrushStyle = 650; break;
        case NS_ooxml::LN_Value_ST_Shd_pct85: nWW8BrushStyle = 850; break;
        case NS_ooxml::LN_Value_ST_Shd_pct87: nWW8BrushStyle = 875; break;
        case NS_ooxml::LN_Value_ST_Shd_pct95: nWW8BrushStyle = 950; break;
    };

    sal_Int32 nApplyColor = 0;
    if( !nWW8BrushStyle )
    {
        // Clear-Brush
        nApplyColor = m_nFillColor;
    }
    else
    {
        sal_Int32 nFore = m_nColor;
        sal_Int32 nBack = m_nFillColor;

        sal_uInt32 nRed = ((nFore & 0xff0000)>>0x10) * nWW8BrushStyle;
        sal_uInt32 nGreen = ((nFore & 0xff00)>>0x8) * nWW8BrushStyle;
        sal_uInt32 nBlue = (nFore & 0xff) * nWW8BrushStyle;
        nRed += ((nBack & 0xff0000)>>0x10)  * (1000L - nWW8BrushStyle);
        nGreen += ((nBack & 0xff00)>>0x8)* (1000L - nWW8BrushStyle);
        nBlue += (nBack & 0xff) * (1000L - nWW8BrushStyle);

        nApplyColor = ( (nRed/1000) << 0x10 ) + ((nGreen/1000) << 8) + nBlue/1000;
    }

    // Check if it is a 'Character'
    if (m_OutputFormat == Character)
    {
        sal_Int32 nShadingPattern = drawing::ShadingPattern::CLEAR;
        switch (m_nShadingPattern)
        {
        case NS_ooxml::LN_Value_ST_Shd_clear: nShadingPattern = drawing::ShadingPattern::CLEAR; break;
        case NS_ooxml::LN_Value_ST_Shd_solid: nShadingPattern = drawing::ShadingPattern::SOLID; break;
        case NS_ooxml::LN_Value_ST_Shd_pct5: nShadingPattern = drawing::ShadingPattern::PCT5; break;
        case NS_ooxml::LN_Value_ST_Shd_pct10: nShadingPattern = drawing::ShadingPattern::PCT10; break;
        case NS_ooxml::LN_Value_ST_Shd_pct20: nShadingPattern = drawing::ShadingPattern::PCT20; break;
        case NS_ooxml::LN_Value_ST_Shd_pct25: nShadingPattern = drawing::ShadingPattern::PCT25; break;
        case NS_ooxml::LN_Value_ST_Shd_pct30: nShadingPattern = drawing::ShadingPattern::PCT30; break;
        case NS_ooxml::LN_Value_ST_Shd_pct40: nShadingPattern = drawing::ShadingPattern::PCT40; break;
        case NS_ooxml::LN_Value_ST_Shd_pct50: nShadingPattern = drawing::ShadingPattern::PCT50; break;
        case NS_ooxml::LN_Value_ST_Shd_pct60: nShadingPattern = drawing::ShadingPattern::PCT60; break;
        case NS_ooxml::LN_Value_ST_Shd_pct70: nShadingPattern = drawing::ShadingPattern::PCT70; break;
        case NS_ooxml::LN_Value_ST_Shd_pct75: nShadingPattern = drawing::ShadingPattern::PCT75; break;
        case NS_ooxml::LN_Value_ST_Shd_pct80: nShadingPattern = drawing::ShadingPattern::PCT80; break;
        case NS_ooxml::LN_Value_ST_Shd_pct90: nShadingPattern = drawing::ShadingPattern::PCT90; break;
        case NS_ooxml::LN_Value_ST_Shd_horzStripe: nShadingPattern = drawing::ShadingPattern::HORZ_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_vertStripe: nShadingPattern = drawing::ShadingPattern::VERT_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_reverseDiagStripe: nShadingPattern = drawing::ShadingPattern::REVERSE_DIAG_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_diagStripe: nShadingPattern = drawing::ShadingPattern::DIAG_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_horzCross: nShadingPattern = drawing::ShadingPattern::HORZ_CROSS; break;
        case NS_ooxml::LN_Value_ST_Shd_diagCross: nShadingPattern = drawing::ShadingPattern::DIAG_CROSS; break;
        case NS_ooxml::LN_Value_ST_Shd_thinHorzStripe: nShadingPattern = drawing::ShadingPattern::THIN_HORZ_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_thinVertStripe: nShadingPattern = drawing::ShadingPattern::THIN_VERT_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_thinReverseDiagStripe: nShadingPattern = drawing::ShadingPattern::THIN_REVERSE_DIAG_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_thinDiagStripe: nShadingPattern = drawing::ShadingPattern::THIN_DIAG_STRIPE; break;
        case NS_ooxml::LN_Value_ST_Shd_thinHorzCross: nShadingPattern = drawing::ShadingPattern::THIN_HORZ_CROSS; break;
        case NS_ooxml::LN_Value_ST_Shd_thinDiagCross: nShadingPattern = drawing::ShadingPattern::THIN_DIAG_CROSS; break;
        case NS_ooxml::LN_Value_ST_Shd_pct12: nShadingPattern = drawing::ShadingPattern::PCT12; break;
        case NS_ooxml::LN_Value_ST_Shd_pct15: nShadingPattern = drawing::ShadingPattern::PCT15; break;
        case NS_ooxml::LN_Value_ST_Shd_pct35: nShadingPattern = drawing::ShadingPattern::PCT35; break;
        case NS_ooxml::LN_Value_ST_Shd_pct37: nShadingPattern = drawing::ShadingPattern::PCT37; break;
        case NS_ooxml::LN_Value_ST_Shd_pct45: nShadingPattern = drawing::ShadingPattern::PCT45; break;
        case NS_ooxml::LN_Value_ST_Shd_pct55: nShadingPattern = drawing::ShadingPattern::PCT55; break;
        case NS_ooxml::LN_Value_ST_Shd_pct62: nShadingPattern = drawing::ShadingPattern::PCT62; break;
        case NS_ooxml::LN_Value_ST_Shd_pct65: nShadingPattern = drawing::ShadingPattern::PCT65; break;
        case NS_ooxml::LN_Value_ST_Shd_pct85: nShadingPattern = drawing::ShadingPattern::PCT85; break;
        case NS_ooxml::LN_Value_ST_Shd_pct87: nShadingPattern = drawing::ShadingPattern::PCT87; break;
        case NS_ooxml::LN_Value_ST_Shd_pct95: nShadingPattern = drawing::ShadingPattern::PCT95; break;
        };

        // Write the shading pattern property
        pPropertyMap->Insert(PROP_CHAR_SHADING_VALUE, uno::makeAny( nShadingPattern ));
    }

    if (m_OutputFormat == Paragraph)
    {
        // If brush style = clear and FillColor = COLOR_AUTO, then don't enable the fill style - just pre-select the default color
        if (nWW8BrushStyle || !m_bAutoFillColor)
            pPropertyMap->Insert(PROP_FILL_STYLE, uno::makeAny(drawing::FillStyle_SOLID));

        pPropertyMap->Insert(PROP_FILL_COLOR, uno::makeAny(nApplyColor));
    }
    else
        pPropertyMap->Insert( m_OutputFormat == Form ? PROP_BACK_COLOR
                            : PROP_CHAR_BACK_COLOR, uno::makeAny( nApplyColor ));

    createGrabBag("originalColor", uno::makeAny(OUString::fromUtf8(msfilter::util::ConvertColor(nApplyColor, true))));

    return pPropertyMap;
}

void CellColorHandler::createGrabBag(const OUString& aName, const uno::Any& rAny)
{
    if (m_aInteropGrabBagName.isEmpty())
        return;

    beans::PropertyValue aValue;
    aValue.Name = aName;
    aValue.Value = rAny;
    m_aInteropGrabBag.push_back(aValue);
}

void CellColorHandler::enableInteropGrabBag(const OUString& aName)
{
    m_aInteropGrabBagName = aName;
}

beans::PropertyValue CellColorHandler::getInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = m_aInteropGrabBagName;
    aRet.Value = uno::makeAny(comphelper::containerToSequence(m_aInteropGrabBag));
    return aRet;
}

void CellColorHandler::disableInteropGrabBag()
{
    m_aInteropGrabBagName.clear();
    m_aInteropGrabBag.clear();
}

bool CellColorHandler::isInteropGrabBagEnabled()
{
    return !(m_aInteropGrabBagName.isEmpty());
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
