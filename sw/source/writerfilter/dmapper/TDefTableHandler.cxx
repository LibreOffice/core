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
        case NS_ooxml::LN_Value_ST_Border_nil: return "nil";
        case NS_ooxml::LN_Value_ST_Border_none: return "none";
        case NS_ooxml::LN_Value_ST_Border_single: return "single";
        case NS_ooxml::LN_Value_ST_Border_thick: return "thick";
        case NS_ooxml::LN_Value_ST_Border_double: return "double";
        case NS_ooxml::LN_Value_ST_Border_dotted: return "dotted";
        case NS_ooxml::LN_Value_ST_Border_dashed: return "dashed";
        case NS_ooxml::LN_Value_ST_Border_dotDash: return "dotDash";
        case NS_ooxml::LN_Value_ST_Border_dotDotDash: return "dotDotDash";
        case NS_ooxml::LN_Value_ST_Border_triple: return "triple";
        case NS_ooxml::LN_Value_ST_Border_thinThickSmallGap: return "thinThickSmallGap";
        case NS_ooxml::LN_Value_ST_Border_thickThinSmallGap: return "thickThinSmallGap";
        case NS_ooxml::LN_Value_ST_Border_thinThickThinSmallGap: return "thinThickThinSmallGap";
        case NS_ooxml::LN_Value_ST_Border_thinThickMediumGap: return "thinThickMediumGap";
        case NS_ooxml::LN_Value_ST_Border_thickThinMediumGap: return "thickThinMediumGap";
        case NS_ooxml::LN_Value_ST_Border_thinThickThinMediumGap: return "thinThickThinMediumGap";
        case NS_ooxml::LN_Value_ST_Border_thinThickLargeGap: return "thinThickLargeGap";
        case NS_ooxml::LN_Value_ST_Border_thickThinLargeGap: return "thickThinLargeGap";
        case NS_ooxml::LN_Value_ST_Border_thinThickThinLargeGap: return "thinThickThinLargeGap";
        case NS_ooxml::LN_Value_ST_Border_wave: return "wave";
        case NS_ooxml::LN_Value_ST_Border_doubleWave: return "doubleWave";
        case NS_ooxml::LN_Value_ST_Border_dashSmallGap: return "dashSmallGap";
        case NS_ooxml::LN_Value_ST_Border_dashDotStroked: return "dashDotStroked";
        case NS_ooxml::LN_Value_ST_Border_threeDEmboss: return "threeDEmboss";
        case NS_ooxml::LN_Value_ST_Border_threeDEngrave: return "threeDEngrave";
        case NS_ooxml::LN_Value_ST_Border_outset: return "outset";
        case NS_ooxml::LN_Value_ST_Border_inset: return "inset";
        case NS_ooxml::LN_Value_ST_Border_apples: return "apples";
        case NS_ooxml::LN_Value_ST_Border_archedScallops: return "archedScallops";
        case NS_ooxml::LN_Value_ST_Border_babyPacifier: return "babyPacifier";
        case NS_ooxml::LN_Value_ST_Border_babyRattle: return "babyRattle";
        case NS_ooxml::LN_Value_ST_Border_balloons3Colors: return "balloons3Colors";
        case NS_ooxml::LN_Value_ST_Border_balloonsHotAir: return "balloonsHotAir";
        case NS_ooxml::LN_Value_ST_Border_basicBlackDashes: return "basicBlackDashes";
        case NS_ooxml::LN_Value_ST_Border_basicBlackDots: return "basicBlackDots";
        case NS_ooxml::LN_Value_ST_Border_basicBlackSquares: return "basicBlackSquares";
        case NS_ooxml::LN_Value_ST_Border_basicThinLines: return "basicThinLines";
        case NS_ooxml::LN_Value_ST_Border_basicWhiteDashes: return "basicWhiteDashes";
        case NS_ooxml::LN_Value_ST_Border_basicWhiteDots: return "basicWhiteDots";
        case NS_ooxml::LN_Value_ST_Border_basicWhiteSquares: return "basicWhiteSquares";
        case NS_ooxml::LN_Value_ST_Border_basicWideInline: return "basicWideInline";
        case NS_ooxml::LN_Value_ST_Border_basicWideMidline: return "basicWideMidline";
        case NS_ooxml::LN_Value_ST_Border_basicWideOutline: return "basicWideOutline";
        case NS_ooxml::LN_Value_ST_Border_bats: return "bats";
        case NS_ooxml::LN_Value_ST_Border_birds: return "birds";
        case NS_ooxml::LN_Value_ST_Border_birdsFlight: return "birdsFlight";
        case NS_ooxml::LN_Value_ST_Border_cabins: return "cabins";
        case NS_ooxml::LN_Value_ST_Border_cakeSlice: return "cakeSlice";
        case NS_ooxml::LN_Value_ST_Border_candyCorn: return "candyCorn";
        case NS_ooxml::LN_Value_ST_Border_celticKnotwork: return "celticKnotwork";
        case NS_ooxml::LN_Value_ST_Border_certificateBanner: return "certificateBanner";
        case NS_ooxml::LN_Value_ST_Border_chainLink: return "chainLink";
        case NS_ooxml::LN_Value_ST_Border_champagneBottle: return "champagneBottle";
        case NS_ooxml::LN_Value_ST_Border_checkedBarBlack: return "checkedBarBlack";
        case NS_ooxml::LN_Value_ST_Border_checkedBarColor: return "checkedBarColor";
        case NS_ooxml::LN_Value_ST_Border_checkered: return "checkered";
        case NS_ooxml::LN_Value_ST_Border_christmasTree: return "christmasTree";
        case NS_ooxml::LN_Value_ST_Border_circlesLines: return "circlesLines";
        case NS_ooxml::LN_Value_ST_Border_circlesRectangles: return "circlesRectangles";
        case NS_ooxml::LN_Value_ST_Border_classicalWave: return "classicalWave";
        case NS_ooxml::LN_Value_ST_Border_clocks: return "clocks";
        case NS_ooxml::LN_Value_ST_Border_compass: return "compass";
        case NS_ooxml::LN_Value_ST_Border_confetti: return "confetti";
        case NS_ooxml::LN_Value_ST_Border_confettiGrays: return "confettiGrays";
        case NS_ooxml::LN_Value_ST_Border_confettiOutline: return "confettiOutline";
        case NS_ooxml::LN_Value_ST_Border_confettiStreamers: return "confettiStreamers";
        case NS_ooxml::LN_Value_ST_Border_confettiWhite: return "confettiWhite";
        case NS_ooxml::LN_Value_ST_Border_cornerTriangles: return "cornerTriangles";
        case NS_ooxml::LN_Value_ST_Border_couponCutoutDashes: return "couponCutoutDashes";
        case NS_ooxml::LN_Value_ST_Border_couponCutoutDots: return "couponCutoutDots";
        case NS_ooxml::LN_Value_ST_Border_crazyMaze: return "crazyMaze";
        case NS_ooxml::LN_Value_ST_Border_creaturesButterfly: return "creaturesButterfly";
        case NS_ooxml::LN_Value_ST_Border_creaturesFish: return "creaturesFish";
        case NS_ooxml::LN_Value_ST_Border_creaturesInsects: return "creaturesInsects";
        case NS_ooxml::LN_Value_ST_Border_creaturesLadyBug: return "creaturesLadyBug";
        case NS_ooxml::LN_Value_ST_Border_crossStitch: return "crossStitch";
        case NS_ooxml::LN_Value_ST_Border_cup: return "cup";
        case NS_ooxml::LN_Value_ST_Border_decoArch: return "decoArch";
        case NS_ooxml::LN_Value_ST_Border_decoArchColor: return "decoArchColor";
        case NS_ooxml::LN_Value_ST_Border_decoBlocks: return "decoBlocks";
        case NS_ooxml::LN_Value_ST_Border_diamondsGray: return "diamondsGray";
        case NS_ooxml::LN_Value_ST_Border_doubleD: return "doubleD";
        case NS_ooxml::LN_Value_ST_Border_doubleDiamonds: return "doubleDiamonds";
        case NS_ooxml::LN_Value_ST_Border_earth1: return "earth1";
        case NS_ooxml::LN_Value_ST_Border_earth2: return "earth2";
        case NS_ooxml::LN_Value_ST_Border_eclipsingSquares1: return "eclipsingSquares1";
        case NS_ooxml::LN_Value_ST_Border_eclipsingSquares2: return "eclipsingSquares2";
        case NS_ooxml::LN_Value_ST_Border_eggsBlack: return "eggsBlack";
        case NS_ooxml::LN_Value_ST_Border_fans: return "fans";
        case NS_ooxml::LN_Value_ST_Border_film: return "film";
        case NS_ooxml::LN_Value_ST_Border_firecrackers: return "firecrackers";
        case NS_ooxml::LN_Value_ST_Border_flowersBlockPrint: return "flowersBlockPrint";
        case NS_ooxml::LN_Value_ST_Border_flowersDaisies: return "flowersDaisies";
        case NS_ooxml::LN_Value_ST_Border_flowersModern1: return "flowersModern1";
        case NS_ooxml::LN_Value_ST_Border_flowersModern2: return "flowersModern2";
        case NS_ooxml::LN_Value_ST_Border_flowersPansy: return "flowersPansy";
        case NS_ooxml::LN_Value_ST_Border_flowersRedRose: return "flowersRedRose";
        case NS_ooxml::LN_Value_ST_Border_flowersRoses: return "flowersRoses";
        case NS_ooxml::LN_Value_ST_Border_flowersTeacup: return "flowersTeacup";
        case NS_ooxml::LN_Value_ST_Border_flowersTiny: return "flowersTiny";
        case NS_ooxml::LN_Value_ST_Border_gems: return "gems";
        case NS_ooxml::LN_Value_ST_Border_gingerbreadMan: return "gingerbreadMan";
        case NS_ooxml::LN_Value_ST_Border_gradient: return "gradient";
        case NS_ooxml::LN_Value_ST_Border_handmade1: return "handmade1";
        case NS_ooxml::LN_Value_ST_Border_handmade2: return "handmade2";
        case NS_ooxml::LN_Value_ST_Border_heartBalloon: return "heartBalloon";
        case NS_ooxml::LN_Value_ST_Border_heartGray: return "heartGray";
        case NS_ooxml::LN_Value_ST_Border_hearts: return "hearts";
        case NS_ooxml::LN_Value_ST_Border_heebieJeebies: return "heebieJeebies";
        case NS_ooxml::LN_Value_ST_Border_holly: return "holly";
        case NS_ooxml::LN_Value_ST_Border_houseFunky: return "houseFunky";
        case NS_ooxml::LN_Value_ST_Border_hypnotic: return "hypnotic";
        case NS_ooxml::LN_Value_ST_Border_iceCreamCones: return "iceCreamCones";
        case NS_ooxml::LN_Value_ST_Border_lightBulb: return "lightBulb";
        case NS_ooxml::LN_Value_ST_Border_lightning1: return "lightning1";
        case NS_ooxml::LN_Value_ST_Border_lightning2: return "lightning2";
        case NS_ooxml::LN_Value_ST_Border_mapPins: return "mapPins";
        case NS_ooxml::LN_Value_ST_Border_mapleLeaf: return "mapleLeaf";
        case NS_ooxml::LN_Value_ST_Border_mapleMuffins: return "mapleMuffins";
        case NS_ooxml::LN_Value_ST_Border_marquee: return "marquee";
        case NS_ooxml::LN_Value_ST_Border_marqueeToothed: return "marqueeToothed";
        case NS_ooxml::LN_Value_ST_Border_moons: return "moons";
        case NS_ooxml::LN_Value_ST_Border_mosaic: return "mosaic";
        case NS_ooxml::LN_Value_ST_Border_musicNotes: return "musicNotes";
        case NS_ooxml::LN_Value_ST_Border_northwest: return "northwest";
        case NS_ooxml::LN_Value_ST_Border_ovals: return "ovals";
        case NS_ooxml::LN_Value_ST_Border_packages: return "packages";
        case NS_ooxml::LN_Value_ST_Border_palmsBlack: return "palmsBlack";
        case NS_ooxml::LN_Value_ST_Border_palmsColor: return "palmsColor";
        case NS_ooxml::LN_Value_ST_Border_paperClips: return "paperClips";
        case NS_ooxml::LN_Value_ST_Border_papyrus: return "papyrus";
        case NS_ooxml::LN_Value_ST_Border_partyFavor: return "partyFavor";
        case NS_ooxml::LN_Value_ST_Border_partyGlass: return "partyGlass";
        case NS_ooxml::LN_Value_ST_Border_pencils: return "pencils";
        case NS_ooxml::LN_Value_ST_Border_people: return "people";
        case NS_ooxml::LN_Value_ST_Border_peopleWaving: return "peopleWaving";
        case NS_ooxml::LN_Value_ST_Border_peopleHats: return "peopleHats";
        case NS_ooxml::LN_Value_ST_Border_poinsettias: return "poinsettias";
        case NS_ooxml::LN_Value_ST_Border_postageStamp: return "postageStamp";
        case NS_ooxml::LN_Value_ST_Border_pumpkin1: return "pumpkin1";
        case NS_ooxml::LN_Value_ST_Border_pushPinNote2: return "pushPinNote2";
        case NS_ooxml::LN_Value_ST_Border_pushPinNote1: return "pushPinNote1";
        case NS_ooxml::LN_Value_ST_Border_pyramids: return "pyramids";
        case NS_ooxml::LN_Value_ST_Border_pyramidsAbove: return "pyramidsAbove";
        case NS_ooxml::LN_Value_ST_Border_quadrants: return "quadrants";
        case NS_ooxml::LN_Value_ST_Border_rings: return "rings";
        case NS_ooxml::LN_Value_ST_Border_safari: return "safari";
        case NS_ooxml::LN_Value_ST_Border_sawtooth: return "sawtooth";
        case NS_ooxml::LN_Value_ST_Border_sawtoothGray: return "sawtoothGray";
        case NS_ooxml::LN_Value_ST_Border_scaredCat: return "scaredCat";
        case NS_ooxml::LN_Value_ST_Border_seattle: return "seattle";
        case NS_ooxml::LN_Value_ST_Border_shadowedSquares: return "shadowedSquares";
        case NS_ooxml::LN_Value_ST_Border_sharksTeeth: return "sharksTeeth";
        case NS_ooxml::LN_Value_ST_Border_shorebirdTracks: return "shorebirdTracks";
        case NS_ooxml::LN_Value_ST_Border_skyrocket: return "skyrocket";
        case NS_ooxml::LN_Value_ST_Border_snowflakeFancy: return "snowflakeFancy";
        case NS_ooxml::LN_Value_ST_Border_snowflakes: return "snowflakes";
        case NS_ooxml::LN_Value_ST_Border_sombrero: return "sombrero";
        case NS_ooxml::LN_Value_ST_Border_southwest: return "southwest";
        case NS_ooxml::LN_Value_ST_Border_stars: return "stars";
        case NS_ooxml::LN_Value_ST_Border_starsTop: return "starsTop";
        case NS_ooxml::LN_Value_ST_Border_stars3d: return "stars3d";
        case NS_ooxml::LN_Value_ST_Border_starsBlack: return "starsBlack";
        case NS_ooxml::LN_Value_ST_Border_starsShadowed: return "starsShadowed";
        case NS_ooxml::LN_Value_ST_Border_sun: return "sun";
        case NS_ooxml::LN_Value_ST_Border_swirligig: return "swirligig";
        case NS_ooxml::LN_Value_ST_Border_tornPaper: return "tornPaper";
        case NS_ooxml::LN_Value_ST_Border_tornPaperBlack: return "tornPaperBlack";
        case NS_ooxml::LN_Value_ST_Border_trees: return "trees";
        case NS_ooxml::LN_Value_ST_Border_triangleParty: return "triangleParty";
        case NS_ooxml::LN_Value_ST_Border_triangles: return "triangles";
        case NS_ooxml::LN_Value_ST_Border_tribal1: return "tribal1";
        case NS_ooxml::LN_Value_ST_Border_tribal2: return "tribal2";
        case NS_ooxml::LN_Value_ST_Border_tribal3: return "tribal3";
        case NS_ooxml::LN_Value_ST_Border_tribal4: return "tribal4";
        case NS_ooxml::LN_Value_ST_Border_tribal5: return "tribal5";
        case NS_ooxml::LN_Value_ST_Border_tribal6: return "tribal6";
        case NS_ooxml::LN_Value_ST_Border_twistedLines1: return "twistedLines1";
        case NS_ooxml::LN_Value_ST_Border_twistedLines2: return "twistedLines2";
        case NS_ooxml::LN_Value_ST_Border_vine: return "vine";
        case NS_ooxml::LN_Value_ST_Border_waveline: return "waveline";
        case NS_ooxml::LN_Value_ST_Border_weavingAngles: return "weavingAngles";
        case NS_ooxml::LN_Value_ST_Border_weavingBraid: return "weavingBraid";
        case NS_ooxml::LN_Value_ST_Border_weavingRibbon: return "weavingRibbon";
        case NS_ooxml::LN_Value_ST_Border_weavingStrips: return "weavingStrips";
        case NS_ooxml::LN_Value_ST_Border_whiteFlowers: return "whiteFlowers";
        case NS_ooxml::LN_Value_ST_Border_woodwork: return "woodwork";
        case NS_ooxml::LN_Value_ST_Border_xIllusions: return "xIllusions";
        case NS_ooxml::LN_Value_ST_Border_zanyTriangles: return "zanyTriangles";
        case NS_ooxml::LN_Value_ST_Border_zigZag: return "zigZag";
        case NS_ooxml::LN_Value_ST_Border_zigZagStitch: return "zigZagStitch";
        default: break;
    }
    return OUString();
}

OUString TDefTableHandler::getThemeColorTypeString(sal_Int32 nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_St_ThemeColor_dark1: return "dark1";
        case NS_ooxml::LN_Value_St_ThemeColor_light1: return "light1";
        case NS_ooxml::LN_Value_St_ThemeColor_dark2: return "dark2";
        case NS_ooxml::LN_Value_St_ThemeColor_light2: return "light2";
        case NS_ooxml::LN_Value_St_ThemeColor_accent1: return "accent1";
        case NS_ooxml::LN_Value_St_ThemeColor_accent2: return "accent2";
        case NS_ooxml::LN_Value_St_ThemeColor_accent3: return "accent3";
        case NS_ooxml::LN_Value_St_ThemeColor_accent4: return "accent4";
        case NS_ooxml::LN_Value_St_ThemeColor_accent5: return "accent5";
        case NS_ooxml::LN_Value_St_ThemeColor_accent6: return "accent6";
        case NS_ooxml::LN_Value_St_ThemeColor_hyperlink: return "hyperlink";
        case NS_ooxml::LN_Value_St_ThemeColor_followedHyperlink: return "followedHyperlink";
        case NS_ooxml::LN_Value_St_ThemeColor_none: return "none";
        case NS_ooxml::LN_Value_St_ThemeColor_background1: return "background1";
        case NS_ooxml::LN_Value_St_ThemeColor_text1: return "text1";
        case NS_ooxml::LN_Value_St_ThemeColor_background2: return "background2";
        case NS_ooxml::LN_Value_St_ThemeColor_text2: return "text2";
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
            appendGrabBag("sz", OUString::number(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_val:
            m_nLineType = nIntValue;
            appendGrabBag("val", TDefTableHandler::getBorderTypeString(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_color:
            appendGrabBag("color", msfilter::util::ConvertColorOU(Color(ColorTransparency,nIntValue)));
            m_nLineColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_space:
            appendGrabBag("space", OUString::number(nIntValue));
        break;
        case NS_ooxml::LN_CT_Border_shadow:
            //if 1 then line has shadow - unsupported
        case NS_ooxml::LN_CT_Border_frame:
            // ignored
        break;
        case NS_ooxml::LN_CT_Border_themeColor:
            appendGrabBag("themeColor", TDefTableHandler::getThemeColorTypeString(nIntValue));
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
