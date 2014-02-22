/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <CellColorHandler.hxx>
#include <PropertyMap.hxx>
#include <ConversionHelper.hxx>
#include <TDefTableHandler.hxx>
#include <ooxml/resourceids.hxx>
#include <com/sun/star/drawing/ShadingPattern.hpp>
#include <sal/macros.h>
#include <filter/msfilter/util.hxx>
#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star::drawing;
using namespace ::writerfilter;


CellColorHandler::CellColorHandler() :
LoggedProperties(dmapper_logger, "CellColorHandler"),
m_nShadingPattern( ShadingPattern::CLEAR ),
m_nColor( 0xffffffff ),
m_nFillColor( 0xffffffff ),
    m_OutputFormat( Form )
{
}

CellColorHandler::~CellColorHandler()
{
}



uno::Any lcl_ConvertShd(sal_Int32 nIntValue)
{
    OUString aRet;
    
    switch (nIntValue)
    {
        case 0: aRet = "clear"; break;
        case 1: aRet = "solid"; break;
        case 2: aRet = "pct5"; break;
        case 3: aRet = "pct10"; break;
        case 4: aRet = "pct20"; break;
        case 5: aRet = "pct25"; break;
        case 6: aRet = "pct30"; break;
        case 7: aRet = "pct40"; break;
        case 8: aRet = "pct50"; break;
        case 9: aRet = "pct60"; break;
        case 10: aRet = "pct70"; break;
        case 11: aRet = "pct75"; break;
        case 12: aRet = "pct80"; break;
        case 13: aRet = "pct90"; break;
        case 14: aRet = "horzStripe"; break;
        case 15: aRet = "vertStripe"; break;
        case 17: aRet = "reverseDiagStripe"; break;
        case 16: aRet = "diagStripe"; break;
        case 18: aRet = "horzCross"; break;
        case 19: aRet = "diagCross"; break;
        case 20: aRet = "thinHorzStripe"; break;
        case 21: aRet = "thinVertStripe"; break;
        case 23: aRet = "thinReverseDiagStripe"; break;
        case 22: aRet = "thinDiagStripe"; break;
        case 24: aRet = "thinHorzCross"; break;
        case 25: aRet = "thinDiagCross"; break;
        case 37: aRet = "pct12"; break;
        case 38: aRet = "pct15"; break;
        case 43: aRet = "pct35"; break;
        case 44: aRet = "pct37"; break;
        case 46: aRet = "pct45"; break;
        case 49: aRet = "pct55"; break;
        case 51: aRet = "pct62"; break;
        case 52: aRet = "pct65"; break;
        case 57: aRet = "pct85"; break;
        case 58: aRet = "pct87"; break;
        case 60: aRet = "pct95"; break;
        case 65535: aRet = "nil"; break;
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
            createGrabBag("fill", uno::makeAny(OStringToOUString(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true), RTL_TEXTENCODING_UTF8)));
            if( nIntValue == OOXML_COLOR_AUTO )
                nIntValue = 0xffffff; 
            m_nFillColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_Shd_color:
            createGrabBag("color", uno::makeAny(OStringToOUString(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true), RTL_TEXTENCODING_UTF8)));
            if( nIntValue == OOXML_COLOR_AUTO )
                nIntValue = 0; 
            
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

    
    static const sal_Int32 eMSGrayScale[] =
    {
        
           0,   
        
        1000,   
        
          50,   
         100,   
         200,   
         250,   
         300,   
         400,   
         500,   
         600,   
         700,   
         750,   
         800,   
         900,   
        
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
        
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
        
          25,   
          75,   
         125,   
         150,   
         175,   
         225,   
         275,   
         325,   
         350,   
         375,   
         425,   
         450,   
         475,   
         525,   
         550,   
         575,   
         625,   
         650,   
         675,   
         725,   
         775,   
         825,   
         850,   
         875,   
         925,   
         950,   
         975    
    };

    if( m_nShadingPattern >= (sal_Int32)SAL_N_ELEMENTS( eMSGrayScale ) )
        m_nShadingPattern = 0;

    sal_Int32 nWW8BrushStyle = eMSGrayScale[m_nShadingPattern];
    sal_Int32 nApplyColor = 0;
    if( !nWW8BrushStyle )
    {
        
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

    
    if (m_OutputFormat == Character)
    {
        static sal_Int32 aWWShadingPatterns[ ] =
        {
            ShadingPattern::CLEAR,
            ShadingPattern::SOLID,
            ShadingPattern::PCT5,
            ShadingPattern::PCT10,
            ShadingPattern::PCT20,
            ShadingPattern::PCT25,
            ShadingPattern::PCT30,
            ShadingPattern::PCT40,
            ShadingPattern::PCT50,
            ShadingPattern::PCT60,
            ShadingPattern::PCT70,
            ShadingPattern::PCT75,
            ShadingPattern::PCT80,
            ShadingPattern::PCT90,
            ShadingPattern::HORZ_STRIPE,
            ShadingPattern::VERT_STRIPE,
            ShadingPattern::REVERSE_DIAG_STRIPE,
            ShadingPattern::DIAG_STRIPE,
            ShadingPattern::HORZ_CROSS,
            ShadingPattern::DIAG_CROSS,
            ShadingPattern::THIN_HORZ_STRIPE,
            ShadingPattern::THIN_VERT_STRIPE,
            ShadingPattern::THIN_REVERSE_DIAG_STRIPE,
            ShadingPattern::THIN_DIAG_STRIPE,
            ShadingPattern::THIN_HORZ_CROSS,
            ShadingPattern::THIN_DIAG_CROSS,
            ShadingPattern::UNUSED_1,
            ShadingPattern::UNUSED_2,
            ShadingPattern::UNUSED_3,
            ShadingPattern::UNUSED_4,
            ShadingPattern::UNUSED_5,
            ShadingPattern::UNUSED_6,
            ShadingPattern::UNUSED_7,
            ShadingPattern::UNUSED_8,
            ShadingPattern::UNUSED_9,
            ShadingPattern::PCT2,
            ShadingPattern::PCT7,
            ShadingPattern::PCT12,
            ShadingPattern::PCT15,
            ShadingPattern::PCT17,
            ShadingPattern::PCT22,
            ShadingPattern::PCT27,
            ShadingPattern::PCT32,
            ShadingPattern::PCT35,
            ShadingPattern::PCT37,
            ShadingPattern::PCT42,
            ShadingPattern::PCT45,
            ShadingPattern::PCT47,
            ShadingPattern::PCT52,
            ShadingPattern::PCT55,
            ShadingPattern::PCT57,
            ShadingPattern::PCT62,
            ShadingPattern::PCT65,
            ShadingPattern::PCT67,
            ShadingPattern::PCT72,
            ShadingPattern::PCT77,
            ShadingPattern::PCT82,
            ShadingPattern::PCT85,
            ShadingPattern::PCT87,
            ShadingPattern::PCT92,
            ShadingPattern::PCT95,
            ShadingPattern::PCT97
        };

        
        pPropertyMap->Insert(PROP_CHAR_SHADING_VALUE, uno::makeAny( aWWShadingPatterns[m_nShadingPattern] ));
    }

    pPropertyMap->Insert( m_OutputFormat == Form ? PROP_BACK_COLOR
                        : m_OutputFormat == Paragraph ? PROP_PARA_BACK_COLOR
                        : PROP_CHAR_BACK_COLOR, uno::makeAny( nApplyColor ));
    return pPropertyMap;
}

void CellColorHandler::createGrabBag(OUString aName, uno::Any aAny)
{
    if (m_aInteropGrabBagName.isEmpty())
        return;

    beans::PropertyValue aValue;
    aValue.Name = aName;
    aValue.Value = aAny;
    m_aInteropGrabBag.push_back(aValue);
}

void CellColorHandler::enableInteropGrabBag(OUString aName)
{
    m_aInteropGrabBagName = aName;
}

beans::PropertyValue CellColorHandler::getInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = m_aInteropGrabBagName;

    uno::Sequence<beans::PropertyValue> aSeq(m_aInteropGrabBag.size());
    beans::PropertyValue* pSeq = aSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = m_aInteropGrabBag.begin(); i != m_aInteropGrabBag.end(); ++i)
        *pSeq++ = *i;

    aRet.Value = uno::makeAny(aSeq);
    return aRet;
}

void CellColorHandler::disableInteropGrabBag()
{
    m_aInteropGrabBagName = "";
    m_aInteropGrabBag.clear();
}

sal_Bool CellColorHandler::isInteropGrabBagEnabled()
{
    return !(m_aInteropGrabBagName.isEmpty());
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
