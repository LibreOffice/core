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
#include <TDefTableHandler.hxx>
#include <PropertyMap.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <filter/msfilter/util.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;



TDefTableHandler::TDefTableHandler(bool bOOXML) :
LoggedProperties(dmapper_logger, "TDefTableHandler"),
m_nLineWidth(0),
m_nLineType(0),
m_nLineColor(0),
m_nLineDistance(0),
m_bOOXML( bOOXML )
{
}


TDefTableHandler::~TDefTableHandler()
{
}

OUString TDefTableHandler::getBorderTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case 255: return OUString("nil");
        case 0: return OUString("none");
        case 1: return OUString("single");
        case 2: return OUString("thick");
        case 3: return OUString("double");
        case 6: return OUString("dotted");
        case 7: return OUString("dashed");
        case 8: return OUString("dotDash");
        case 9: return OUString("dotDotDash");
        case 10: return OUString("triple");
        case 11: return OUString("thinThickSmallGap");
        case 12: return OUString("thickThinSmallGap");
        case 13: return OUString("thinThickThinSmallGap");
        case 14: return OUString("thinThickMediumGap");
        case 15: return OUString("thickThinMediumGap");
        case 16: return OUString("thinThickThinMediumGap");
        case 17: return OUString("thinThickLargeGap");
        case 18: return OUString("thickThinLargeGap");
        case 19: return OUString("thinThickThinLargeGap");
        case 20: return OUString("wave");
        case 21: return OUString("doubleWave");
        case 22: return OUString("dashSmallGap");
        case 23: return OUString("dashDotStroked");
        case 24: return OUString("threeDEmboss");
        case 25: return OUString("threeDEngrave");
        case 26: return OUString("outset");
        case 27: return OUString("inset");
        case 64: return OUString("apples");
        case 65: return OUString("archedScallops");
        case 66: return OUString("babyPacifier");
        case 67: return OUString("babyRattle");
        case 68: return OUString("balloons3Colors");
        case 69: return OUString("balloonsHotAir");
        case 70: return OUString("basicBlackDashes");
        case 71: return OUString("basicBlackDots");
        case 72: return OUString("basicBlackSquares");
        case 73: return OUString("basicThinLines");
        case 74: return OUString("basicWhiteDashes");
        case 75: return OUString("basicWhiteDots");
        case 76: return OUString("basicWhiteSquares");
        case 77: return OUString("basicWideInline");
        case 78: return OUString("basicWideMidline");
        case 79: return OUString("basicWideOutline");
        case 80: return OUString("bats");
        case 81: return OUString("birds");
        case 82: return OUString("birdsFlight");
        case 83: return OUString("cabins");
        case 84: return OUString("cakeSlice");
        case 85: return OUString("candyCorn");
        case 86: return OUString("celticKnotwork");
        case 87: return OUString("certificateBanner");
        case 88: return OUString("chainLink");
        case 89: return OUString("champagneBottle");
        case 90: return OUString("checkedBarBlack");
        case 91: return OUString("checkedBarColor");
        case 92: return OUString("checkered");
        case 93: return OUString("christmasTree");
        case 94: return OUString("circlesLines");
        case 95: return OUString("circlesRectangles");
        case 96: return OUString("classicalWave");
        case 97: return OUString("clocks");
        case 98: return OUString("compass");
        case 99: return OUString("confetti");
        case 100: return OUString("confettiGrays");
        case 101: return OUString("confettiOutline");
        case 102: return OUString("confettiStreamers");
        case 103: return OUString("confettiWhite");
        case 104: return OUString("cornerTriangles");
        case 105: return OUString("couponCutoutDashes");
        case 106: return OUString("couponCutoutDots");
        case 107: return OUString("crazyMaze");
        case 108: return OUString("creaturesButterfly");
        case 109: return OUString("creaturesFish");
        case 110: return OUString("creaturesInsects");
        case 111: return OUString("creaturesLadyBug");
        case 112: return OUString("crossStitch");
        case 113: return OUString("cup");
        case 114: return OUString("decoArch");
        case 115: return OUString("decoArchColor");
        case 116: return OUString("decoBlocks");
        case 117: return OUString("diamondsGray");
        case 118: return OUString("doubleD");
        case 119: return OUString("doubleDiamonds");
        case 120: return OUString("earth1");
        case 121: return OUString("earth2");
        case 122: return OUString("eclipsingSquares1");
        case 123: return OUString("eclipsingSquares2");
        case 124: return OUString("eggsBlack");
        case 125: return OUString("fans");
        case 126: return OUString("film");
        case 127: return OUString("firecrackers");
        case 128: return OUString("flowersBlockPrint");
        case 129: return OUString("flowersDaisies");
        case 130: return OUString("flowersModern1");
        case 131: return OUString("flowersModern2");
        case 132: return OUString("flowersPansy");
        case 133: return OUString("flowersRedRose");
        case 134: return OUString("flowersRoses");
        case 135: return OUString("flowersTeacup");
        case 136: return OUString("flowersTiny");
        case 137: return OUString("gems");
        case 138: return OUString("gingerbreadMan");
        case 139: return OUString("gradient");
        case 140: return OUString("handmade1");
        case 141: return OUString("handmade2");
        case 142: return OUString("heartBalloon");
        case 143: return OUString("heartGray");
        case 144: return OUString("hearts");
        case 145: return OUString("heebieJeebies");
        case 146: return OUString("holly");
        case 147: return OUString("houseFunky");
        case 148: return OUString("hypnotic");
        case 149: return OUString("iceCreamCones");
        case 150: return OUString("lightBulb");
        case 151: return OUString("lightning1");
        case 152: return OUString("lightning2");
        case 153: return OUString("mapPins");
        case 154: return OUString("mapleLeaf");
        case 155: return OUString("mapleMuffins");
        case 156: return OUString("marquee");
        case 157: return OUString("marqueeToothed");
        case 158: return OUString("moons");
        case 159: return OUString("mosaic");
        case 160: return OUString("musicNotes");
        case 161: return OUString("northwest");
        case 162: return OUString("ovals");
        case 163: return OUString("packages");
        case 164: return OUString("palmsBlack");
        case 165: return OUString("palmsColor");
        case 166: return OUString("paperClips");
        case 167: return OUString("papyrus");
        case 168: return OUString("partyFavor");
        case 169: return OUString("partyGlass");
        case 170: return OUString("pencils");
        case 171: return OUString("people");
        case 172: return OUString("peopleWaving");
        case 173: return OUString("peopleHats");
        case 174: return OUString("poinsettias");
        case 175: return OUString("postageStamp");
        case 176: return OUString("pumpkin1");
        case 177: return OUString("pushPinNote2");
        case 178: return OUString("pushPinNote1");
        case 179: return OUString("pyramids");
        case 180: return OUString("pyramidsAbove");
        case 181: return OUString("quadrants");
        case 182: return OUString("rings");
        case 183: return OUString("safari");
        case 184: return OUString("sawtooth");
        case 185: return OUString("sawtoothGray");
        case 186: return OUString("scaredCat");
        case 187: return OUString("seattle");
        case 188: return OUString("shadowedSquares");
        case 189: return OUString("sharksTeeth");
        case 190: return OUString("shorebirdTracks");
        case 191: return OUString("skyrocket");
        case 192: return OUString("snowflakeFancy");
        case 193: return OUString("snowflakes");
        case 194: return OUString("sombrero");
        case 195: return OUString("southwest");
        case 196: return OUString("stars");
        case 197: return OUString("starsTop");
        case 198: return OUString("stars3d");
        case 199: return OUString("starsBlack");
        case 200: return OUString("starsShadowed");
        case 201: return OUString("sun");
        case 202: return OUString("swirligig");
        case 203: return OUString("tornPaper");
        case 204: return OUString("tornPaperBlack");
        case 205: return OUString("trees");
        case 206: return OUString("triangleParty");
        case 207: return OUString("triangles");
        case 208: return OUString("tribal1");
        case 209: return OUString("tribal2");
        case 210: return OUString("tribal3");
        case 211: return OUString("tribal4");
        case 212: return OUString("tribal5");
        case 213: return OUString("tribal6");
        case 214: return OUString("twistedLines1");
        case 215: return OUString("twistedLines2");
        case 216: return OUString("vine");
        case 217: return OUString("waveline");
        case 218: return OUString("weavingAngles");
        case 219: return OUString("weavingBraid");
        case 220: return OUString("weavingRibbon");
        case 221: return OUString("weavingStrips");
        case 222: return OUString("whiteFlowers");
        case 223: return OUString("woodwork");
        case 224: return OUString("xIllusions");
        case 225: return OUString("zanyTriangles");
        case 226: return OUString("zigZag");
        case 227: return OUString("zigZagStitch");
        default: break;
    }
    return OUString();
}

OUString TDefTableHandler::getThemeColorTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_St_ThemeColor_dark1: return OUString("dark1");
        case NS_ooxml::LN_Value_St_ThemeColor_light1: return OUString("light1");
        case NS_ooxml::LN_Value_St_ThemeColor_dark2: return OUString("dark2");
        case NS_ooxml::LN_Value_St_ThemeColor_light2: return OUString("light2");
        case NS_ooxml::LN_Value_St_ThemeColor_accent1: return OUString("accent1");
        case NS_ooxml::LN_Value_St_ThemeColor_accent2: return OUString("accent2");
        case NS_ooxml::LN_Value_St_ThemeColor_accent3: return OUString("accent3");
        case NS_ooxml::LN_Value_St_ThemeColor_accent4: return OUString("accent4");
        case NS_ooxml::LN_Value_St_ThemeColor_accent5: return OUString("accent5");
        case NS_ooxml::LN_Value_St_ThemeColor_accent6: return OUString("accent6");
        case NS_ooxml::LN_Value_St_ThemeColor_hyperlink: return OUString("hyperlink");
        case NS_ooxml::LN_Value_St_ThemeColor_followedHyperlink: return OUString("followedHyperlink");
        case NS_ooxml::LN_Value_St_ThemeColor_none: return OUString("none");
        case NS_ooxml::LN_Value_St_ThemeColor_background1: return OUString("background1");
        case NS_ooxml::LN_Value_St_ThemeColor_text1: return OUString("text1");
        case NS_ooxml::LN_Value_St_ThemeColor_background2: return OUString("background2");
        case NS_ooxml::LN_Value_St_ThemeColor_text2: return OUString("text2");
        default: break;
    }
    return OUString();
}

void TDefTableHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
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
            appendGrabBag("color", OStringToOUString(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true), RTL_TEXTENCODING_UTF8));
            m_nLineColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_space:
            appendGrabBag("space", OUString::number(nIntValue));
            m_nLineDistance = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            //if 1 then line has shadow - unsupported
        case NS_ooxml::LN_CT_Border_frame:
            // ignored
        break;
        case NS_ooxml::LN_CT_Border_themeColor:
            appendGrabBag("themeColor", TDefTableHandler::getThemeColorTypeString(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_themeTint:
        case NS_ooxml::LN_CT_Border_themeShade:
            // ignored
        break;
        default:
            OSL_FAIL("unknown attribute");
    }
}


void TDefTableHandler::localResolve(Id rName, writerfilter::Reference<Properties>::Pointer_t pProperties)
{
    if( pProperties.get())
    {
        m_nLineWidth = m_nLineType = m_nLineColor = m_nLineDistance = 0;
        std::vector<beans::PropertyValue> aSavedGrabBag;
        if (!m_aInteropGrabBagName.isEmpty())
        {
            aSavedGrabBag = m_aInteropGrabBag;
            m_aInteropGrabBag.clear();
        }
        pProperties->resolve( *this );
        table::BorderLine2 aBorderLine;
        ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                                                        aBorderLine, m_bOOXML );
        bool rtl = false; // TODO
        switch( rName )
        {
            case NS_ooxml::LN_CT_TcBorders_top:
                m_aTopBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("top"));
            break;
            case NS_ooxml::LN_CT_TcBorders_start:
                if( rtl )
                    m_aRightBorderLines.push_back(aBorderLine);
                else
                    m_aLeftBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("start"));
            break;
            case NS_ooxml::LN_CT_TcBorders_left:
                m_aLeftBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("left"));
            break;
            case NS_ooxml::LN_CT_TcBorders_bottom:
                m_aBottomBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("bottom"));
            break;
            case NS_ooxml::LN_CT_TcBorders_end:
                if( rtl )
                    m_aLeftBorderLines.push_back(aBorderLine);
                else
                    m_aRightBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("end"));
            break;
            case NS_ooxml::LN_CT_TcBorders_right:
                m_aRightBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("right"));
            break;
            case NS_ooxml::LN_CT_TcBorders_insideH:
                m_aInsideHBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("insideH"));
            break;
            case NS_ooxml::LN_CT_TcBorders_insideV:
                m_aInsideVBorderLines.push_back(aBorderLine);
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("insideV"));
            break;
            case NS_ooxml::LN_CT_TcBorders_tl2br:
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("tl2br"));
            break;
            case NS_ooxml::LN_CT_TcBorders_tr2bl:
                if (!m_aInteropGrabBagName.isEmpty())
                    aSavedGrabBag.push_back(getInteropGrabBag("tr2bl"));
            break;
            default:;
        }
        if (!m_aInteropGrabBagName.isEmpty())
            m_aInteropGrabBag = aSavedGrabBag;
    }
}


void TDefTableHandler::lcl_sprm(Sprm & rSprm)
{
    switch( rSprm.getId() )
    {
        case NS_ooxml::LN_CT_TcBorders_top:
        case NS_ooxml::LN_CT_TcBorders_left:
        case NS_ooxml::LN_CT_TcBorders_start:
        case NS_ooxml::LN_CT_TcBorders_bottom:
        case NS_ooxml::LN_CT_TcBorders_right:
        case NS_ooxml::LN_CT_TcBorders_end:
        case NS_ooxml::LN_CT_TcBorders_insideH:
        case NS_ooxml::LN_CT_TcBorders_insideV:
        case NS_ooxml::LN_CT_TcBorders_tl2br:
        case NS_ooxml::LN_CT_TcBorders_tr2bl:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            localResolve( rSprm.getId(), pProperties );
        }
        break;
        default:;
    }
}


PropertyMapPtr  TDefTableHandler::getRowProperties() const
{
    PropertyMapPtr pPropertyMap(new PropertyMap);

    // Writer only wants the separators, Word provides also the outer border positions
    if( m_aCellBorderPositions.size() > 2 )
    {
        //determine table width
        double nFullWidth = m_aCellBorderPositions[m_aCellBorderPositions.size() - 1] - m_aCellBorderPositions[0];
        //the positions have to be distibuted in a range of 10000
        const double nFullWidthRelative = 10000.;
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_aCellBorderPositions.size() - 2 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        for( sal_uInt32 nBorder = 1; nBorder < m_aCellBorderPositions.size() - 1; ++nBorder )
        {
            sal_Int16 nRelPos =
                sal::static_int_cast< sal_Int16 >(double(m_aCellBorderPositions[nBorder]) * nFullWidthRelative / nFullWidth );

            pSeparators[nBorder - 1].Position =  nRelPos;
            pSeparators[nBorder - 1].IsVisible = sal_True;
        }
        pPropertyMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );
    }

    return pPropertyMap;
}


void TDefTableHandler::fillCellProperties(
            size_t nCell, ::boost::shared_ptr< TablePropertyMap > pCellProperties ) const
{
    if( m_aCellBorderPositions.size() > nCell )
    {
        sal_Int16 nVertOrient = text::VertOrientation::NONE;
        switch( m_aCellVertAlign[nCell] ) //0 - top 1 - center 2 - bottom
        {
            case 1: nVertOrient = text::VertOrientation::CENTER; break;
            case 2: nVertOrient = text::VertOrientation::BOTTOM; break;
            default:;
        }
        pCellProperties->Insert( PROP_VERT_ORIENT, uno::makeAny( nVertOrient ) );
    }
    if( m_aTopBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_TOP_BORDER, uno::makeAny( m_aTopBorderLines[nCell] ) );
    if( m_aLeftBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_LEFT_BORDER, uno::makeAny( m_aLeftBorderLines[nCell] ) );
    if( m_aBottomBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_BOTTOM_BORDER, uno::makeAny( m_aBottomBorderLines[nCell] ) );
    if( m_aRightBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_RIGHT_BORDER, uno::makeAny( m_aRightBorderLines[nCell] ) );
    if( m_aInsideHBorderLines.size() > nCell )
        pCellProperties->Insert( META_PROP_HORIZONTAL_BORDER, uno::makeAny( m_aInsideHBorderLines[nCell] ) );
    if( m_aInsideVBorderLines.size() > nCell )
        pCellProperties->Insert( META_PROP_VERTICAL_BORDER, uno::makeAny( m_aInsideVBorderLines[nCell] ) );
}


sal_Int32 TDefTableHandler::getTableWidth() const
{
    sal_Int32 nWidth = 0;
    if( m_aCellBorderPositions.size() > 1 )
    {
        //determine table width
        nWidth = m_aCellBorderPositions[m_aCellBorderPositions.size() - 1] - m_aCellBorderPositions[0];
    }
    return nWidth;
}


size_t TDefTableHandler::getCellCount() const
{
    return m_aCellVertAlign.size();
}

void TDefTableHandler::enableInteropGrabBag(const OUString& aName)
{
    m_aInteropGrabBagName = aName;
}

beans::PropertyValue TDefTableHandler::getInteropGrabBag(const OUString& aName)
{
    beans::PropertyValue aRet;
    if (aName.isEmpty())
        aRet.Name = m_aInteropGrabBagName;
    else
        aRet.Name = aName;

    uno::Sequence<beans::PropertyValue> aSeq(m_aInteropGrabBag.size());
    beans::PropertyValue* pSeq = aSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = m_aInteropGrabBag.begin(); i != m_aInteropGrabBag.end(); ++i)
        *pSeq++ = *i;
    m_aInteropGrabBag.clear();

    aRet.Value = uno::makeAny(aSeq);
    return aRet;
}

void TDefTableHandler::appendGrabBag(const OUString& aKey, const OUString& aValue)
{
    beans::PropertyValue aProperty;
    aProperty.Name = aKey;
    aProperty.Value = uno::makeAny(aValue);
    m_aInteropGrabBag.push_back(aProperty);
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
