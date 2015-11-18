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
#include <comphelper/sequence.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;



TDefTableHandler::TDefTableHandler() :
LoggedProperties("TDefTableHandler"),
m_nLineWidth(0),
m_nLineType(0),
m_nLineColor(0),
m_nLineDistance(0)
{
}


TDefTableHandler::~TDefTableHandler()
{
}

OUString TDefTableHandler::getBorderTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_ST_Border_nil: return OUString("nil");
        case NS_ooxml::LN_Value_ST_Border_none: return OUString("none");
        case NS_ooxml::LN_Value_ST_Border_single: return OUString("single");
        case NS_ooxml::LN_Value_ST_Border_thick: return OUString("thick");
        case NS_ooxml::LN_Value_ST_Border_double: return OUString("double");
        case NS_ooxml::LN_Value_ST_Border_dotted: return OUString("dotted");
        case NS_ooxml::LN_Value_ST_Border_dashed: return OUString("dashed");
        case NS_ooxml::LN_Value_ST_Border_dotDash: return OUString("dotDash");
        case NS_ooxml::LN_Value_ST_Border_dotDotDash: return OUString("dotDotDash");
        case NS_ooxml::LN_Value_ST_Border_triple: return OUString("triple");
        case NS_ooxml::LN_Value_ST_Border_thinThickSmallGap: return OUString("thinThickSmallGap");
        case NS_ooxml::LN_Value_ST_Border_thickThinSmallGap: return OUString("thickThinSmallGap");
        case NS_ooxml::LN_Value_ST_Border_thinThickThinSmallGap: return OUString("thinThickThinSmallGap");
        case NS_ooxml::LN_Value_ST_Border_thinThickMediumGap: return OUString("thinThickMediumGap");
        case NS_ooxml::LN_Value_ST_Border_thickThinMediumGap: return OUString("thickThinMediumGap");
        case NS_ooxml::LN_Value_ST_Border_thinThickThinMediumGap: return OUString("thinThickThinMediumGap");
        case NS_ooxml::LN_Value_ST_Border_thinThickLargeGap: return OUString("thinThickLargeGap");
        case NS_ooxml::LN_Value_ST_Border_thickThinLargeGap: return OUString("thickThinLargeGap");
        case NS_ooxml::LN_Value_ST_Border_thinThickThinLargeGap: return OUString("thinThickThinLargeGap");
        case NS_ooxml::LN_Value_ST_Border_wave: return OUString("wave");
        case NS_ooxml::LN_Value_ST_Border_doubleWave: return OUString("doubleWave");
        case NS_ooxml::LN_Value_ST_Border_dashSmallGap: return OUString("dashSmallGap");
        case NS_ooxml::LN_Value_ST_Border_dashDotStroked: return OUString("dashDotStroked");
        case NS_ooxml::LN_Value_ST_Border_threeDEmboss: return OUString("threeDEmboss");
        case NS_ooxml::LN_Value_ST_Border_threeDEngrave: return OUString("threeDEngrave");
        case NS_ooxml::LN_Value_ST_Border_outset: return OUString("outset");
        case NS_ooxml::LN_Value_ST_Border_inset: return OUString("inset");
        case NS_ooxml::LN_Value_ST_Border_apples: return OUString("apples");
        case NS_ooxml::LN_Value_ST_Border_archedScallops: return OUString("archedScallops");
        case NS_ooxml::LN_Value_ST_Border_babyPacifier: return OUString("babyPacifier");
        case NS_ooxml::LN_Value_ST_Border_babyRattle: return OUString("babyRattle");
        case NS_ooxml::LN_Value_ST_Border_balloons3Colors: return OUString("balloons3Colors");
        case NS_ooxml::LN_Value_ST_Border_balloonsHotAir: return OUString("balloonsHotAir");
        case NS_ooxml::LN_Value_ST_Border_basicBlackDashes: return OUString("basicBlackDashes");
        case NS_ooxml::LN_Value_ST_Border_basicBlackDots: return OUString("basicBlackDots");
        case NS_ooxml::LN_Value_ST_Border_basicBlackSquares: return OUString("basicBlackSquares");
        case NS_ooxml::LN_Value_ST_Border_basicThinLines: return OUString("basicThinLines");
        case NS_ooxml::LN_Value_ST_Border_basicWhiteDashes: return OUString("basicWhiteDashes");
        case NS_ooxml::LN_Value_ST_Border_basicWhiteDots: return OUString("basicWhiteDots");
        case NS_ooxml::LN_Value_ST_Border_basicWhiteSquares: return OUString("basicWhiteSquares");
        case NS_ooxml::LN_Value_ST_Border_basicWideInline: return OUString("basicWideInline");
        case NS_ooxml::LN_Value_ST_Border_basicWideMidline: return OUString("basicWideMidline");
        case NS_ooxml::LN_Value_ST_Border_basicWideOutline: return OUString("basicWideOutline");
        case NS_ooxml::LN_Value_ST_Border_bats: return OUString("bats");
        case NS_ooxml::LN_Value_ST_Border_birds: return OUString("birds");
        case NS_ooxml::LN_Value_ST_Border_birdsFlight: return OUString("birdsFlight");
        case NS_ooxml::LN_Value_ST_Border_cabins: return OUString("cabins");
        case NS_ooxml::LN_Value_ST_Border_cakeSlice: return OUString("cakeSlice");
        case NS_ooxml::LN_Value_ST_Border_candyCorn: return OUString("candyCorn");
        case NS_ooxml::LN_Value_ST_Border_celticKnotwork: return OUString("celticKnotwork");
        case NS_ooxml::LN_Value_ST_Border_certificateBanner: return OUString("certificateBanner");
        case NS_ooxml::LN_Value_ST_Border_chainLink: return OUString("chainLink");
        case NS_ooxml::LN_Value_ST_Border_champagneBottle: return OUString("champagneBottle");
        case NS_ooxml::LN_Value_ST_Border_checkedBarBlack: return OUString("checkedBarBlack");
        case NS_ooxml::LN_Value_ST_Border_checkedBarColor: return OUString("checkedBarColor");
        case NS_ooxml::LN_Value_ST_Border_checkered: return OUString("checkered");
        case NS_ooxml::LN_Value_ST_Border_christmasTree: return OUString("christmasTree");
        case NS_ooxml::LN_Value_ST_Border_circlesLines: return OUString("circlesLines");
        case NS_ooxml::LN_Value_ST_Border_circlesRectangles: return OUString("circlesRectangles");
        case NS_ooxml::LN_Value_ST_Border_classicalWave: return OUString("classicalWave");
        case NS_ooxml::LN_Value_ST_Border_clocks: return OUString("clocks");
        case NS_ooxml::LN_Value_ST_Border_compass: return OUString("compass");
        case NS_ooxml::LN_Value_ST_Border_confetti: return OUString("confetti");
        case NS_ooxml::LN_Value_ST_Border_confettiGrays: return OUString("confettiGrays");
        case NS_ooxml::LN_Value_ST_Border_confettiOutline: return OUString("confettiOutline");
        case NS_ooxml::LN_Value_ST_Border_confettiStreamers: return OUString("confettiStreamers");
        case NS_ooxml::LN_Value_ST_Border_confettiWhite: return OUString("confettiWhite");
        case NS_ooxml::LN_Value_ST_Border_cornerTriangles: return OUString("cornerTriangles");
        case NS_ooxml::LN_Value_ST_Border_couponCutoutDashes: return OUString("couponCutoutDashes");
        case NS_ooxml::LN_Value_ST_Border_couponCutoutDots: return OUString("couponCutoutDots");
        case NS_ooxml::LN_Value_ST_Border_crazyMaze: return OUString("crazyMaze");
        case NS_ooxml::LN_Value_ST_Border_creaturesButterfly: return OUString("creaturesButterfly");
        case NS_ooxml::LN_Value_ST_Border_creaturesFish: return OUString("creaturesFish");
        case NS_ooxml::LN_Value_ST_Border_creaturesInsects: return OUString("creaturesInsects");
        case NS_ooxml::LN_Value_ST_Border_creaturesLadyBug: return OUString("creaturesLadyBug");
        case NS_ooxml::LN_Value_ST_Border_crossStitch: return OUString("crossStitch");
        case NS_ooxml::LN_Value_ST_Border_cup: return OUString("cup");
        case NS_ooxml::LN_Value_ST_Border_decoArch: return OUString("decoArch");
        case NS_ooxml::LN_Value_ST_Border_decoArchColor: return OUString("decoArchColor");
        case NS_ooxml::LN_Value_ST_Border_decoBlocks: return OUString("decoBlocks");
        case NS_ooxml::LN_Value_ST_Border_diamondsGray: return OUString("diamondsGray");
        case NS_ooxml::LN_Value_ST_Border_doubleD: return OUString("doubleD");
        case NS_ooxml::LN_Value_ST_Border_doubleDiamonds: return OUString("doubleDiamonds");
        case NS_ooxml::LN_Value_ST_Border_earth1: return OUString("earth1");
        case NS_ooxml::LN_Value_ST_Border_earth2: return OUString("earth2");
        case NS_ooxml::LN_Value_ST_Border_eclipsingSquares1: return OUString("eclipsingSquares1");
        case NS_ooxml::LN_Value_ST_Border_eclipsingSquares2: return OUString("eclipsingSquares2");
        case NS_ooxml::LN_Value_ST_Border_eggsBlack: return OUString("eggsBlack");
        case NS_ooxml::LN_Value_ST_Border_fans: return OUString("fans");
        case NS_ooxml::LN_Value_ST_Border_film: return OUString("film");
        case NS_ooxml::LN_Value_ST_Border_firecrackers: return OUString("firecrackers");
        case NS_ooxml::LN_Value_ST_Border_flowersBlockPrint: return OUString("flowersBlockPrint");
        case NS_ooxml::LN_Value_ST_Border_flowersDaisies: return OUString("flowersDaisies");
        case NS_ooxml::LN_Value_ST_Border_flowersModern1: return OUString("flowersModern1");
        case NS_ooxml::LN_Value_ST_Border_flowersModern2: return OUString("flowersModern2");
        case NS_ooxml::LN_Value_ST_Border_flowersPansy: return OUString("flowersPansy");
        case NS_ooxml::LN_Value_ST_Border_flowersRedRose: return OUString("flowersRedRose");
        case NS_ooxml::LN_Value_ST_Border_flowersRoses: return OUString("flowersRoses");
        case NS_ooxml::LN_Value_ST_Border_flowersTeacup: return OUString("flowersTeacup");
        case NS_ooxml::LN_Value_ST_Border_flowersTiny: return OUString("flowersTiny");
        case NS_ooxml::LN_Value_ST_Border_gems: return OUString("gems");
        case NS_ooxml::LN_Value_ST_Border_gingerbreadMan: return OUString("gingerbreadMan");
        case NS_ooxml::LN_Value_ST_Border_gradient: return OUString("gradient");
        case NS_ooxml::LN_Value_ST_Border_handmade1: return OUString("handmade1");
        case NS_ooxml::LN_Value_ST_Border_handmade2: return OUString("handmade2");
        case NS_ooxml::LN_Value_ST_Border_heartBalloon: return OUString("heartBalloon");
        case NS_ooxml::LN_Value_ST_Border_heartGray: return OUString("heartGray");
        case NS_ooxml::LN_Value_ST_Border_hearts: return OUString("hearts");
        case NS_ooxml::LN_Value_ST_Border_heebieJeebies: return OUString("heebieJeebies");
        case NS_ooxml::LN_Value_ST_Border_holly: return OUString("holly");
        case NS_ooxml::LN_Value_ST_Border_houseFunky: return OUString("houseFunky");
        case NS_ooxml::LN_Value_ST_Border_hypnotic: return OUString("hypnotic");
        case NS_ooxml::LN_Value_ST_Border_iceCreamCones: return OUString("iceCreamCones");
        case NS_ooxml::LN_Value_ST_Border_lightBulb: return OUString("lightBulb");
        case NS_ooxml::LN_Value_ST_Border_lightning1: return OUString("lightning1");
        case NS_ooxml::LN_Value_ST_Border_lightning2: return OUString("lightning2");
        case NS_ooxml::LN_Value_ST_Border_mapPins: return OUString("mapPins");
        case NS_ooxml::LN_Value_ST_Border_mapleLeaf: return OUString("mapleLeaf");
        case NS_ooxml::LN_Value_ST_Border_mapleMuffins: return OUString("mapleMuffins");
        case NS_ooxml::LN_Value_ST_Border_marquee: return OUString("marquee");
        case NS_ooxml::LN_Value_ST_Border_marqueeToothed: return OUString("marqueeToothed");
        case NS_ooxml::LN_Value_ST_Border_moons: return OUString("moons");
        case NS_ooxml::LN_Value_ST_Border_mosaic: return OUString("mosaic");
        case NS_ooxml::LN_Value_ST_Border_musicNotes: return OUString("musicNotes");
        case NS_ooxml::LN_Value_ST_Border_northwest: return OUString("northwest");
        case NS_ooxml::LN_Value_ST_Border_ovals: return OUString("ovals");
        case NS_ooxml::LN_Value_ST_Border_packages: return OUString("packages");
        case NS_ooxml::LN_Value_ST_Border_palmsBlack: return OUString("palmsBlack");
        case NS_ooxml::LN_Value_ST_Border_palmsColor: return OUString("palmsColor");
        case NS_ooxml::LN_Value_ST_Border_paperClips: return OUString("paperClips");
        case NS_ooxml::LN_Value_ST_Border_papyrus: return OUString("papyrus");
        case NS_ooxml::LN_Value_ST_Border_partyFavor: return OUString("partyFavor");
        case NS_ooxml::LN_Value_ST_Border_partyGlass: return OUString("partyGlass");
        case NS_ooxml::LN_Value_ST_Border_pencils: return OUString("pencils");
        case NS_ooxml::LN_Value_ST_Border_people: return OUString("people");
        case NS_ooxml::LN_Value_ST_Border_peopleWaving: return OUString("peopleWaving");
        case NS_ooxml::LN_Value_ST_Border_peopleHats: return OUString("peopleHats");
        case NS_ooxml::LN_Value_ST_Border_poinsettias: return OUString("poinsettias");
        case NS_ooxml::LN_Value_ST_Border_postageStamp: return OUString("postageStamp");
        case NS_ooxml::LN_Value_ST_Border_pumpkin1: return OUString("pumpkin1");
        case NS_ooxml::LN_Value_ST_Border_pushPinNote2: return OUString("pushPinNote2");
        case NS_ooxml::LN_Value_ST_Border_pushPinNote1: return OUString("pushPinNote1");
        case NS_ooxml::LN_Value_ST_Border_pyramids: return OUString("pyramids");
        case NS_ooxml::LN_Value_ST_Border_pyramidsAbove: return OUString("pyramidsAbove");
        case NS_ooxml::LN_Value_ST_Border_quadrants: return OUString("quadrants");
        case NS_ooxml::LN_Value_ST_Border_rings: return OUString("rings");
        case NS_ooxml::LN_Value_ST_Border_safari: return OUString("safari");
        case NS_ooxml::LN_Value_ST_Border_sawtooth: return OUString("sawtooth");
        case NS_ooxml::LN_Value_ST_Border_sawtoothGray: return OUString("sawtoothGray");
        case NS_ooxml::LN_Value_ST_Border_scaredCat: return OUString("scaredCat");
        case NS_ooxml::LN_Value_ST_Border_seattle: return OUString("seattle");
        case NS_ooxml::LN_Value_ST_Border_shadowedSquares: return OUString("shadowedSquares");
        case NS_ooxml::LN_Value_ST_Border_sharksTeeth: return OUString("sharksTeeth");
        case NS_ooxml::LN_Value_ST_Border_shorebirdTracks: return OUString("shorebirdTracks");
        case NS_ooxml::LN_Value_ST_Border_skyrocket: return OUString("skyrocket");
        case NS_ooxml::LN_Value_ST_Border_snowflakeFancy: return OUString("snowflakeFancy");
        case NS_ooxml::LN_Value_ST_Border_snowflakes: return OUString("snowflakes");
        case NS_ooxml::LN_Value_ST_Border_sombrero: return OUString("sombrero");
        case NS_ooxml::LN_Value_ST_Border_southwest: return OUString("southwest");
        case NS_ooxml::LN_Value_ST_Border_stars: return OUString("stars");
        case NS_ooxml::LN_Value_ST_Border_starsTop: return OUString("starsTop");
        case NS_ooxml::LN_Value_ST_Border_stars3d: return OUString("stars3d");
        case NS_ooxml::LN_Value_ST_Border_starsBlack: return OUString("starsBlack");
        case NS_ooxml::LN_Value_ST_Border_starsShadowed: return OUString("starsShadowed");
        case NS_ooxml::LN_Value_ST_Border_sun: return OUString("sun");
        case NS_ooxml::LN_Value_ST_Border_swirligig: return OUString("swirligig");
        case NS_ooxml::LN_Value_ST_Border_tornPaper: return OUString("tornPaper");
        case NS_ooxml::LN_Value_ST_Border_tornPaperBlack: return OUString("tornPaperBlack");
        case NS_ooxml::LN_Value_ST_Border_trees: return OUString("trees");
        case NS_ooxml::LN_Value_ST_Border_triangleParty: return OUString("triangleParty");
        case NS_ooxml::LN_Value_ST_Border_triangles: return OUString("triangles");
        case NS_ooxml::LN_Value_ST_Border_tribal1: return OUString("tribal1");
        case NS_ooxml::LN_Value_ST_Border_tribal2: return OUString("tribal2");
        case NS_ooxml::LN_Value_ST_Border_tribal3: return OUString("tribal3");
        case NS_ooxml::LN_Value_ST_Border_tribal4: return OUString("tribal4");
        case NS_ooxml::LN_Value_ST_Border_tribal5: return OUString("tribal5");
        case NS_ooxml::LN_Value_ST_Border_tribal6: return OUString("tribal6");
        case NS_ooxml::LN_Value_ST_Border_twistedLines1: return OUString("twistedLines1");
        case NS_ooxml::LN_Value_ST_Border_twistedLines2: return OUString("twistedLines2");
        case NS_ooxml::LN_Value_ST_Border_vine: return OUString("vine");
        case NS_ooxml::LN_Value_ST_Border_waveline: return OUString("waveline");
        case NS_ooxml::LN_Value_ST_Border_weavingAngles: return OUString("weavingAngles");
        case NS_ooxml::LN_Value_ST_Border_weavingBraid: return OUString("weavingBraid");
        case NS_ooxml::LN_Value_ST_Border_weavingRibbon: return OUString("weavingRibbon");
        case NS_ooxml::LN_Value_ST_Border_weavingStrips: return OUString("weavingStrips");
        case NS_ooxml::LN_Value_ST_Border_whiteFlowers: return OUString("whiteFlowers");
        case NS_ooxml::LN_Value_ST_Border_woodwork: return OUString("woodwork");
        case NS_ooxml::LN_Value_ST_Border_xIllusions: return OUString("xIllusions");
        case NS_ooxml::LN_Value_ST_Border_zanyTriangles: return OUString("zanyTriangles");
        case NS_ooxml::LN_Value_ST_Border_zigZag: return OUString("zigZag");
        case NS_ooxml::LN_Value_ST_Border_zigZagStitch: return OUString("zigZagStitch");
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
            appendGrabBag("color", OUString::fromUtf8(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true)));
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
        ConversionHelper::MakeBorderLine(m_nLineWidth, m_nLineType, m_nLineColor, aBorderLine, /*bIsOOXML=*/true);
        const bool rtl = false; // TODO
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

void TDefTableHandler::fillCellProperties(
            size_t nCell, ::std::shared_ptr< TablePropertyMap > pCellProperties ) const
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

    aRet.Value = uno::makeAny(comphelper::containerToSequence(m_aInteropGrabBag));
    m_aInteropGrabBag.clear();
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
