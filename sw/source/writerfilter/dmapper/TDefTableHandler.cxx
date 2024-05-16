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
#include "TDefTableHandler.hxx"
#include "PropertyMap.hxx"
#include "ConversionHelper.hxx"
#include <ooxml/resourceids.hxx>
#include <filter/msfilter/util.hxx>
#include <tools/color.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <comphelper/sequence.hxx>

namespace writerfilter::dmapper {

using namespace ::com::sun::star;


TDefTableHandler::TDefTableHandler() :
LoggedProperties("TDefTableHandler"),
m_nLineWidth(0),
m_nLineType(0),
m_nLineColor(0)
{
}


TDefTableHandler::~TDefTableHandler()
{
}

OUString TDefTableHandler::getBorderTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_ST_Border_nil: return u"nil"_ustr;
        case NS_ooxml::LN_Value_ST_Border_none: return u"none"_ustr;
        case NS_ooxml::LN_Value_ST_Border_single: return u"single"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thick: return u"thick"_ustr;
        case NS_ooxml::LN_Value_ST_Border_double: return u"double"_ustr;
        case NS_ooxml::LN_Value_ST_Border_dotted: return u"dotted"_ustr;
        case NS_ooxml::LN_Value_ST_Border_dashed: return u"dashed"_ustr;
        case NS_ooxml::LN_Value_ST_Border_dotDash: return u"dotDash"_ustr;
        case NS_ooxml::LN_Value_ST_Border_dotDotDash: return u"dotDotDash"_ustr;
        case NS_ooxml::LN_Value_ST_Border_triple: return u"triple"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thinThickSmallGap: return u"thinThickSmallGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thickThinSmallGap: return u"thickThinSmallGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thinThickThinSmallGap: return u"thinThickThinSmallGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thinThickMediumGap: return u"thinThickMediumGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thickThinMediumGap: return u"thickThinMediumGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thinThickThinMediumGap: return u"thinThickThinMediumGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thinThickLargeGap: return u"thinThickLargeGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thickThinLargeGap: return u"thickThinLargeGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_thinThickThinLargeGap: return u"thinThickThinLargeGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_wave: return u"wave"_ustr;
        case NS_ooxml::LN_Value_ST_Border_doubleWave: return u"doubleWave"_ustr;
        case NS_ooxml::LN_Value_ST_Border_dashSmallGap: return u"dashSmallGap"_ustr;
        case NS_ooxml::LN_Value_ST_Border_dashDotStroked: return u"dashDotStroked"_ustr;
        case NS_ooxml::LN_Value_ST_Border_threeDEmboss: return u"threeDEmboss"_ustr;
        case NS_ooxml::LN_Value_ST_Border_threeDEngrave: return u"threeDEngrave"_ustr;
        case NS_ooxml::LN_Value_ST_Border_outset: return u"outset"_ustr;
        case NS_ooxml::LN_Value_ST_Border_inset: return u"inset"_ustr;
        case NS_ooxml::LN_Value_ST_Border_apples: return u"apples"_ustr;
        case NS_ooxml::LN_Value_ST_Border_archedScallops: return u"archedScallops"_ustr;
        case NS_ooxml::LN_Value_ST_Border_babyPacifier: return u"babyPacifier"_ustr;
        case NS_ooxml::LN_Value_ST_Border_babyRattle: return u"babyRattle"_ustr;
        case NS_ooxml::LN_Value_ST_Border_balloons3Colors: return u"balloons3Colors"_ustr;
        case NS_ooxml::LN_Value_ST_Border_balloonsHotAir: return u"balloonsHotAir"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicBlackDashes: return u"basicBlackDashes"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicBlackDots: return u"basicBlackDots"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicBlackSquares: return u"basicBlackSquares"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicThinLines: return u"basicThinLines"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicWhiteDashes: return u"basicWhiteDashes"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicWhiteDots: return u"basicWhiteDots"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicWhiteSquares: return u"basicWhiteSquares"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicWideInline: return u"basicWideInline"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicWideMidline: return u"basicWideMidline"_ustr;
        case NS_ooxml::LN_Value_ST_Border_basicWideOutline: return u"basicWideOutline"_ustr;
        case NS_ooxml::LN_Value_ST_Border_bats: return u"bats"_ustr;
        case NS_ooxml::LN_Value_ST_Border_birds: return u"birds"_ustr;
        case NS_ooxml::LN_Value_ST_Border_birdsFlight: return u"birdsFlight"_ustr;
        case NS_ooxml::LN_Value_ST_Border_cabins: return u"cabins"_ustr;
        case NS_ooxml::LN_Value_ST_Border_cakeSlice: return u"cakeSlice"_ustr;
        case NS_ooxml::LN_Value_ST_Border_candyCorn: return u"candyCorn"_ustr;
        case NS_ooxml::LN_Value_ST_Border_celticKnotwork: return u"celticKnotwork"_ustr;
        case NS_ooxml::LN_Value_ST_Border_certificateBanner: return u"certificateBanner"_ustr;
        case NS_ooxml::LN_Value_ST_Border_chainLink: return u"chainLink"_ustr;
        case NS_ooxml::LN_Value_ST_Border_champagneBottle: return u"champagneBottle"_ustr;
        case NS_ooxml::LN_Value_ST_Border_checkedBarBlack: return u"checkedBarBlack"_ustr;
        case NS_ooxml::LN_Value_ST_Border_checkedBarColor: return u"checkedBarColor"_ustr;
        case NS_ooxml::LN_Value_ST_Border_checkered: return u"checkered"_ustr;
        case NS_ooxml::LN_Value_ST_Border_christmasTree: return u"christmasTree"_ustr;
        case NS_ooxml::LN_Value_ST_Border_circlesLines: return u"circlesLines"_ustr;
        case NS_ooxml::LN_Value_ST_Border_circlesRectangles: return u"circlesRectangles"_ustr;
        case NS_ooxml::LN_Value_ST_Border_classicalWave: return u"classicalWave"_ustr;
        case NS_ooxml::LN_Value_ST_Border_clocks: return u"clocks"_ustr;
        case NS_ooxml::LN_Value_ST_Border_compass: return u"compass"_ustr;
        case NS_ooxml::LN_Value_ST_Border_confetti: return u"confetti"_ustr;
        case NS_ooxml::LN_Value_ST_Border_confettiGrays: return u"confettiGrays"_ustr;
        case NS_ooxml::LN_Value_ST_Border_confettiOutline: return u"confettiOutline"_ustr;
        case NS_ooxml::LN_Value_ST_Border_confettiStreamers: return u"confettiStreamers"_ustr;
        case NS_ooxml::LN_Value_ST_Border_confettiWhite: return u"confettiWhite"_ustr;
        case NS_ooxml::LN_Value_ST_Border_cornerTriangles: return u"cornerTriangles"_ustr;
        case NS_ooxml::LN_Value_ST_Border_couponCutoutDashes: return u"couponCutoutDashes"_ustr;
        case NS_ooxml::LN_Value_ST_Border_couponCutoutDots: return u"couponCutoutDots"_ustr;
        case NS_ooxml::LN_Value_ST_Border_crazyMaze: return u"crazyMaze"_ustr;
        case NS_ooxml::LN_Value_ST_Border_creaturesButterfly: return u"creaturesButterfly"_ustr;
        case NS_ooxml::LN_Value_ST_Border_creaturesFish: return u"creaturesFish"_ustr;
        case NS_ooxml::LN_Value_ST_Border_creaturesInsects: return u"creaturesInsects"_ustr;
        case NS_ooxml::LN_Value_ST_Border_creaturesLadyBug: return u"creaturesLadyBug"_ustr;
        case NS_ooxml::LN_Value_ST_Border_crossStitch: return u"crossStitch"_ustr;
        case NS_ooxml::LN_Value_ST_Border_cup: return u"cup"_ustr;
        case NS_ooxml::LN_Value_ST_Border_decoArch: return u"decoArch"_ustr;
        case NS_ooxml::LN_Value_ST_Border_decoArchColor: return u"decoArchColor"_ustr;
        case NS_ooxml::LN_Value_ST_Border_decoBlocks: return u"decoBlocks"_ustr;
        case NS_ooxml::LN_Value_ST_Border_diamondsGray: return u"diamondsGray"_ustr;
        case NS_ooxml::LN_Value_ST_Border_doubleD: return u"doubleD"_ustr;
        case NS_ooxml::LN_Value_ST_Border_doubleDiamonds: return u"doubleDiamonds"_ustr;
        case NS_ooxml::LN_Value_ST_Border_earth1: return u"earth1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_earth2: return u"earth2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_eclipsingSquares1: return u"eclipsingSquares1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_eclipsingSquares2: return u"eclipsingSquares2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_eggsBlack: return u"eggsBlack"_ustr;
        case NS_ooxml::LN_Value_ST_Border_fans: return u"fans"_ustr;
        case NS_ooxml::LN_Value_ST_Border_film: return u"film"_ustr;
        case NS_ooxml::LN_Value_ST_Border_firecrackers: return u"firecrackers"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersBlockPrint: return u"flowersBlockPrint"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersDaisies: return u"flowersDaisies"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersModern1: return u"flowersModern1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersModern2: return u"flowersModern2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersPansy: return u"flowersPansy"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersRedRose: return u"flowersRedRose"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersRoses: return u"flowersRoses"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersTeacup: return u"flowersTeacup"_ustr;
        case NS_ooxml::LN_Value_ST_Border_flowersTiny: return u"flowersTiny"_ustr;
        case NS_ooxml::LN_Value_ST_Border_gems: return u"gems"_ustr;
        case NS_ooxml::LN_Value_ST_Border_gingerbreadMan: return u"gingerbreadMan"_ustr;
        case NS_ooxml::LN_Value_ST_Border_gradient: return u"gradient"_ustr;
        case NS_ooxml::LN_Value_ST_Border_handmade1: return u"handmade1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_handmade2: return u"handmade2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_heartBalloon: return u"heartBalloon"_ustr;
        case NS_ooxml::LN_Value_ST_Border_heartGray: return u"heartGray"_ustr;
        case NS_ooxml::LN_Value_ST_Border_hearts: return u"hearts"_ustr;
        case NS_ooxml::LN_Value_ST_Border_heebieJeebies: return u"heebieJeebies"_ustr;
        case NS_ooxml::LN_Value_ST_Border_holly: return u"holly"_ustr;
        case NS_ooxml::LN_Value_ST_Border_houseFunky: return u"houseFunky"_ustr;
        case NS_ooxml::LN_Value_ST_Border_hypnotic: return u"hypnotic"_ustr;
        case NS_ooxml::LN_Value_ST_Border_iceCreamCones: return u"iceCreamCones"_ustr;
        case NS_ooxml::LN_Value_ST_Border_lightBulb: return u"lightBulb"_ustr;
        case NS_ooxml::LN_Value_ST_Border_lightning1: return u"lightning1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_lightning2: return u"lightning2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_mapPins: return u"mapPins"_ustr;
        case NS_ooxml::LN_Value_ST_Border_mapleLeaf: return u"mapleLeaf"_ustr;
        case NS_ooxml::LN_Value_ST_Border_mapleMuffins: return u"mapleMuffins"_ustr;
        case NS_ooxml::LN_Value_ST_Border_marquee: return u"marquee"_ustr;
        case NS_ooxml::LN_Value_ST_Border_marqueeToothed: return u"marqueeToothed"_ustr;
        case NS_ooxml::LN_Value_ST_Border_moons: return u"moons"_ustr;
        case NS_ooxml::LN_Value_ST_Border_mosaic: return u"mosaic"_ustr;
        case NS_ooxml::LN_Value_ST_Border_musicNotes: return u"musicNotes"_ustr;
        case NS_ooxml::LN_Value_ST_Border_northwest: return u"northwest"_ustr;
        case NS_ooxml::LN_Value_ST_Border_ovals: return u"ovals"_ustr;
        case NS_ooxml::LN_Value_ST_Border_packages: return u"packages"_ustr;
        case NS_ooxml::LN_Value_ST_Border_palmsBlack: return u"palmsBlack"_ustr;
        case NS_ooxml::LN_Value_ST_Border_palmsColor: return u"palmsColor"_ustr;
        case NS_ooxml::LN_Value_ST_Border_paperClips: return u"paperClips"_ustr;
        case NS_ooxml::LN_Value_ST_Border_papyrus: return u"papyrus"_ustr;
        case NS_ooxml::LN_Value_ST_Border_partyFavor: return u"partyFavor"_ustr;
        case NS_ooxml::LN_Value_ST_Border_partyGlass: return u"partyGlass"_ustr;
        case NS_ooxml::LN_Value_ST_Border_pencils: return u"pencils"_ustr;
        case NS_ooxml::LN_Value_ST_Border_people: return u"people"_ustr;
        case NS_ooxml::LN_Value_ST_Border_peopleWaving: return u"peopleWaving"_ustr;
        case NS_ooxml::LN_Value_ST_Border_peopleHats: return u"peopleHats"_ustr;
        case NS_ooxml::LN_Value_ST_Border_poinsettias: return u"poinsettias"_ustr;
        case NS_ooxml::LN_Value_ST_Border_postageStamp: return u"postageStamp"_ustr;
        case NS_ooxml::LN_Value_ST_Border_pumpkin1: return u"pumpkin1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_pushPinNote2: return u"pushPinNote2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_pushPinNote1: return u"pushPinNote1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_pyramids: return u"pyramids"_ustr;
        case NS_ooxml::LN_Value_ST_Border_pyramidsAbove: return u"pyramidsAbove"_ustr;
        case NS_ooxml::LN_Value_ST_Border_quadrants: return u"quadrants"_ustr;
        case NS_ooxml::LN_Value_ST_Border_rings: return u"rings"_ustr;
        case NS_ooxml::LN_Value_ST_Border_safari: return u"safari"_ustr;
        case NS_ooxml::LN_Value_ST_Border_sawtooth: return u"sawtooth"_ustr;
        case NS_ooxml::LN_Value_ST_Border_sawtoothGray: return u"sawtoothGray"_ustr;
        case NS_ooxml::LN_Value_ST_Border_scaredCat: return u"scaredCat"_ustr;
        case NS_ooxml::LN_Value_ST_Border_seattle: return u"seattle"_ustr;
        case NS_ooxml::LN_Value_ST_Border_shadowedSquares: return u"shadowedSquares"_ustr;
        case NS_ooxml::LN_Value_ST_Border_sharksTeeth: return u"sharksTeeth"_ustr;
        case NS_ooxml::LN_Value_ST_Border_shorebirdTracks: return u"shorebirdTracks"_ustr;
        case NS_ooxml::LN_Value_ST_Border_skyrocket: return u"skyrocket"_ustr;
        case NS_ooxml::LN_Value_ST_Border_snowflakeFancy: return u"snowflakeFancy"_ustr;
        case NS_ooxml::LN_Value_ST_Border_snowflakes: return u"snowflakes"_ustr;
        case NS_ooxml::LN_Value_ST_Border_sombrero: return u"sombrero"_ustr;
        case NS_ooxml::LN_Value_ST_Border_southwest: return u"southwest"_ustr;
        case NS_ooxml::LN_Value_ST_Border_stars: return u"stars"_ustr;
        case NS_ooxml::LN_Value_ST_Border_starsTop: return u"starsTop"_ustr;
        case NS_ooxml::LN_Value_ST_Border_stars3d: return u"stars3d"_ustr;
        case NS_ooxml::LN_Value_ST_Border_starsBlack: return u"starsBlack"_ustr;
        case NS_ooxml::LN_Value_ST_Border_starsShadowed: return u"starsShadowed"_ustr;
        case NS_ooxml::LN_Value_ST_Border_sun: return u"sun"_ustr;
        case NS_ooxml::LN_Value_ST_Border_swirligig: return u"swirligig"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tornPaper: return u"tornPaper"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tornPaperBlack: return u"tornPaperBlack"_ustr;
        case NS_ooxml::LN_Value_ST_Border_trees: return u"trees"_ustr;
        case NS_ooxml::LN_Value_ST_Border_triangleParty: return u"triangleParty"_ustr;
        case NS_ooxml::LN_Value_ST_Border_triangles: return u"triangles"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tribal1: return u"tribal1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tribal2: return u"tribal2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tribal3: return u"tribal3"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tribal4: return u"tribal4"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tribal5: return u"tribal5"_ustr;
        case NS_ooxml::LN_Value_ST_Border_tribal6: return u"tribal6"_ustr;
        case NS_ooxml::LN_Value_ST_Border_twistedLines1: return u"twistedLines1"_ustr;
        case NS_ooxml::LN_Value_ST_Border_twistedLines2: return u"twistedLines2"_ustr;
        case NS_ooxml::LN_Value_ST_Border_vine: return u"vine"_ustr;
        case NS_ooxml::LN_Value_ST_Border_waveline: return u"waveline"_ustr;
        case NS_ooxml::LN_Value_ST_Border_weavingAngles: return u"weavingAngles"_ustr;
        case NS_ooxml::LN_Value_ST_Border_weavingBraid: return u"weavingBraid"_ustr;
        case NS_ooxml::LN_Value_ST_Border_weavingRibbon: return u"weavingRibbon"_ustr;
        case NS_ooxml::LN_Value_ST_Border_weavingStrips: return u"weavingStrips"_ustr;
        case NS_ooxml::LN_Value_ST_Border_whiteFlowers: return u"whiteFlowers"_ustr;
        case NS_ooxml::LN_Value_ST_Border_woodwork: return u"woodwork"_ustr;
        case NS_ooxml::LN_Value_ST_Border_xIllusions: return u"xIllusions"_ustr;
        case NS_ooxml::LN_Value_ST_Border_zanyTriangles: return u"zanyTriangles"_ustr;
        case NS_ooxml::LN_Value_ST_Border_zigZag: return u"zigZag"_ustr;
        case NS_ooxml::LN_Value_ST_Border_zigZagStitch: return u"zigZagStitch"_ustr;
        default: break;
    }
    return OUString();
}

OUString TDefTableHandler::getThemeColorTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_St_ThemeColor_dark1: return u"dark1"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_light1: return u"light1"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_dark2: return u"dark2"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_light2: return u"light2"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_accent1: return u"accent1"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_accent2: return u"accent2"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_accent3: return u"accent3"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_accent4: return u"accent4"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_accent5: return u"accent5"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_accent6: return u"accent6"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_hyperlink: return u"hyperlink"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_followedHyperlink: return u"followedHyperlink"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_none: return u"none"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_background1: return u"background1"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_text1: return u"text1"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_background2: return u"background2"_ustr;
        case NS_ooxml::LN_Value_St_ThemeColor_text2: return u"text2"_ustr;
        default: break;
    }
    return OUString();
}

model::ThemeColorType TDefTableHandler::getThemeColorTypeIndex(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_St_ThemeColor_dark1:
                return model::ThemeColorType::Dark1;
        case NS_ooxml::LN_Value_St_ThemeColor_light1:
                return model::ThemeColorType::Light1;
        case NS_ooxml::LN_Value_St_ThemeColor_dark2:
                return model::ThemeColorType::Dark2;
        case NS_ooxml::LN_Value_St_ThemeColor_light2:
                return model::ThemeColorType::Light2;
        case NS_ooxml::LN_Value_St_ThemeColor_accent1:
                return model::ThemeColorType::Accent1;
        case NS_ooxml::LN_Value_St_ThemeColor_accent2:
                return model::ThemeColorType::Accent2;
        case NS_ooxml::LN_Value_St_ThemeColor_accent3:
                return model::ThemeColorType::Accent3;
        case NS_ooxml::LN_Value_St_ThemeColor_accent4:
                return model::ThemeColorType::Accent4;
        case NS_ooxml::LN_Value_St_ThemeColor_accent5:
                return model::ThemeColorType::Accent5;
        case NS_ooxml::LN_Value_St_ThemeColor_accent6:
                return model::ThemeColorType::Accent6;
        case NS_ooxml::LN_Value_St_ThemeColor_hyperlink:
                return model::ThemeColorType::Hyperlink;
        case NS_ooxml::LN_Value_St_ThemeColor_followedHyperlink:
                return model::ThemeColorType::FollowedHyperlink;
        case NS_ooxml::LN_Value_St_ThemeColor_none:
                return model::ThemeColorType::Unknown;
        case NS_ooxml::LN_Value_St_ThemeColor_background1:
                return model::ThemeColorType::Light1;
        case NS_ooxml::LN_Value_St_ThemeColor_text1:
                return model::ThemeColorType::Dark1;
        case NS_ooxml::LN_Value_St_ThemeColor_background2:
                return model::ThemeColorType::Light2;
        case NS_ooxml::LN_Value_St_ThemeColor_text2:
                return model::ThemeColorType::Dark2;
        default:
                break;
    }
    return model::ThemeColorType::Unknown;
}

model::ThemeColorUsage TDefTableHandler::getThemeColorUsage(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_St_ThemeColor_background1:
        case NS_ooxml::LN_Value_St_ThemeColor_background2:
            return model::ThemeColorUsage::Background;
        case NS_ooxml::LN_Value_St_ThemeColor_text1:
        case NS_ooxml::LN_Value_St_ThemeColor_text2:
            return model::ThemeColorUsage::Text;
        default:
                break;
    }
    return model::ThemeColorUsage::Unknown;
}

void TDefTableHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch( rName )
    {
        case NS_ooxml::LN_CT_Border_sz:
            //  width of a single line in 1/8 pt, max of 32 pt -> twip * 5 / 2.
            m_nLineWidth = nIntValue * 5 / 2;
            appendGrabBag(u"sz"_ustr, OUString::number(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_val:
            m_nLineType = nIntValue;
            appendGrabBag(u"val"_ustr, TDefTableHandler::getBorderTypeString(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_color:
            appendGrabBag(u"color"_ustr, msfilter::util::ConvertColorOU(Color(ColorTransparency,nIntValue)));
            m_nLineColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_space:
            appendGrabBag(u"space"_ustr, OUString::number(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            //if 1 then line has shadow - unsupported
        case NS_ooxml::LN_CT_Border_frame:
            // ignored
        break;
        case NS_ooxml::LN_CT_Border_themeColor:
            appendGrabBag(u"themeColor"_ustr, TDefTableHandler::getThemeColorTypeString(nIntValue));
            m_eThemeColorType = TDefTableHandler::getThemeColorTypeIndex(nIntValue);
        break;
        case NS_ooxml::LN_CT_Border_themeTint:
            m_nThemeTint = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_themeShade:
            m_nThemeShade = nIntValue;
        break;
        default:
            OSL_FAIL("unknown attribute");
    }
}


void TDefTableHandler::localResolve(Id rName, const writerfilter::Reference<Properties>::Pointer_t& pProperties)
{
    if( !pProperties )
        return;

    m_nLineWidth = m_nLineType = m_nLineColor = 0;
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
                aSavedGrabBag.push_back(getInteropGrabBag(u"top"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_start:
            if( rtl )
                m_aRightBorderLines.push_back(aBorderLine);
            else
                m_aLeftBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"start"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_left:
            m_aLeftBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"left"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_bottom:
            m_aBottomBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"bottom"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_end:
            if( rtl )
                m_aLeftBorderLines.push_back(aBorderLine);
            else
                m_aRightBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"end"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_right:
            m_aRightBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"right"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_insideH:
            m_aInsideHBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"insideH"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_insideV:
            m_aInsideVBorderLines.push_back(aBorderLine);
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"insideV"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_tl2br:
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"tl2br"_ustr));
        break;
        case NS_ooxml::LN_CT_TcBorders_tr2bl:
            if (!m_aInteropGrabBagName.isEmpty())
                aSavedGrabBag.push_back(getInteropGrabBag(u"tr2bl"_ustr));
        break;
        default:;
    }
    if (!m_aInteropGrabBagName.isEmpty())
        m_aInteropGrabBag = aSavedGrabBag;
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

void TDefTableHandler::fillCellProperties( const ::tools::SvRef< TablePropertyMap >& pCellProperties ) const
{
    if( !m_aTopBorderLines.empty() )
        pCellProperties->Insert( PROP_TOP_BORDER, uno::Any( m_aTopBorderLines[0] ) );
    if( !m_aLeftBorderLines.empty() )
        pCellProperties->Insert( PROP_LEFT_BORDER, uno::Any( m_aLeftBorderLines[0] ) );
    if( !m_aBottomBorderLines.empty() )
        pCellProperties->Insert( PROP_BOTTOM_BORDER, uno::Any( m_aBottomBorderLines[0] ) );
    if( !m_aRightBorderLines.empty() )
        pCellProperties->Insert( PROP_RIGHT_BORDER, uno::Any( m_aRightBorderLines[0] ) );
    if( !m_aInsideHBorderLines.empty() )
        pCellProperties->Insert( META_PROP_HORIZONTAL_BORDER, uno::Any( m_aInsideHBorderLines[0] ) );
    if( !m_aInsideVBorderLines.empty() )
        pCellProperties->Insert( META_PROP_VERTICAL_BORDER, uno::Any( m_aInsideVBorderLines[0] ) );

    if (m_eThemeColorType != model::ThemeColorType::Unknown)
    {
        model::ComplexColor aComplexColor;
        aComplexColor.setThemeColor(m_eThemeColorType);

        if (m_nThemeTint > 0 )
        {
            sal_Int16 nTint = sal_Int16((255.0 - m_nThemeTint) * 10000.0 / 255.0);
            aComplexColor.addTransformation({model::TransformationType::Tint, nTint});
        }
        if (m_nThemeShade > 0)
        {
            sal_Int16 nShade = sal_Int16((255.0 - m_nThemeShade) * 10000.0 / 255.0);
            aComplexColor.addTransformation({model::TransformationType::Shade, nShade});
        }
    }
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

    aRet.Value <<= comphelper::containerToSequence(m_aInteropGrabBag);
    m_aInteropGrabBag.clear();
    return aRet;
}

void TDefTableHandler::appendGrabBag(const OUString& aKey, const OUString& aValue)
{
    beans::PropertyValue aProperty;
    aProperty.Name = aKey;
    aProperty.Value <<= aValue;
    m_aInteropGrabBag.push_back(aProperty);
}

} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
