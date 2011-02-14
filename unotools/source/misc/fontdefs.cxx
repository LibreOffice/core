/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include <unotools/fontdefs.hxx>
#include <unotools/fontcfg.hxx>
#include <hash_map>

struct ImplLocalizedFontName
{
    const char*         mpEnglishName;
    const sal_Unicode*  mpLocalizedNames;
};

// TODO: where did the 0,0 delimiters come from? A single 0 should suffice...
static sal_Unicode const aBatang[] = { 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aBatangChe[] = { 0xBC14, 0xD0D5, 0xCCB4, 0, 0 };
static sal_Unicode const aGungsuh[] = { 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aGungsuhChe[] = { 0xAD81, 0xC11C, 0xCCB4, 0, 0 };
static sal_Unicode const aGulim[] = { 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aGulimChe[] = { 0xAD74, 0xB9BC, 0xCCB4, 0, 0 };
static sal_Unicode const aDotum[] = { 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aDotumChe[] = { 0xB3CB, 0xC6C0, 0xCCB4, 0, 0 };
static sal_Unicode const aSimSun[] = { 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aNSimSun[] = { 0x65B0, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aSimHei[] = { 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aSimKai[] = { 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const azycjkSun[] = { 0x4E2D, 0x6613, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const azycjkHei[] = { 0x4E2D, 0x6613, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const azycjkKai[] = { 0x4E2D, 0x6613, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFZHei[] = { 0x65B9, 0x6B63, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aFZKai[] = { 0x65B9, 0x6B63, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFZSongYI[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4E00, 0, 0 };
static sal_Unicode const aFZShuSong[] = { 0x65B9, 0x6B63, 0x4E66, 0x5B8B, 0, 0 };
static sal_Unicode const aFZFangSong[] = { 0x65B9, 0x6B63, 0x4EFF, 0x5B8B, 0, 0 };
// Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
// First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
static sal_Unicode const aMHei[] = { 'm', 0x7B80, 0x9ED1, 0, 'm', 0x6F60, 0x7AAA, 0, 0 };
static sal_Unicode const aMKai[] = { 'm', 0x7B80, 0x6977, 0x566C, 0, 'm', 0x6F60, 0x7FF1, 0x628E, 0, 0 };
static sal_Unicode const aMSong[] = { 'm', 0x7B80, 0x5B8B, 0, 'm', 0x6F60, 0x51BC, 0, 0 };
static sal_Unicode const aCFangSong[] = { 'm', 0x7B80, 0x592B, 0x5B8B, 0, 'm', 0x6F60, 0x6E98, 0x51BC, 0, 0 };
static sal_Unicode const aMingLiU[] = { 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aPMingLiU[] = { 0x65B0, 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aHei[] = { 0x6865, 0, 0 };
static sal_Unicode const aKai[] = { 0x6B61, 0, 0 };
static sal_Unicode const aMing[] = { 0x6D69, 0x6E67, 0, 0 };
static sal_Unicode const aMSGothic[] = { 'm','s',       0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSPGothic[] = { 'm','s','p',  0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSMincho[] = { 'm', 's',      0x660E, 0x671D, 0 };
static sal_Unicode const aMSPMincho[] = { 'm','s','p',  0x660E, 0x671D, 0 };
static sal_Unicode const aMSYaHei[] = { 0x5FAE, 0x8F6F, 0x96C5, 0x9ED1, 0 };
static sal_Unicode const aMSJhengHei[] = { 0x5FAE, 0x8EDF, 0x6B63, 0x9ED1, 0x9AD4, 0 };
static sal_Unicode const aMeiryo[]    = { 0x30e1, 0x30a4, 0x30ea, 0x30aa, 0 };
static sal_Unicode const aHGMinchoL[] = { 'h','g',      0x660E, 0x671D, 'l', 0, 0 };
static sal_Unicode const aHGGothicB[] = { 'h','g',      0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 0 };
static sal_Unicode const aHGPMinchoL[] = { 'h','g','p', 0x660E, 0x671D, 'l', 0 };
static sal_Unicode const aHGPGothicB[] = { 'h','g','p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 0 };
static sal_Unicode const aHGMinchoLSun[] = { 'h','g',   0x660E, 0x671D, 'l', 's', 'u', 'n', 0 };
static sal_Unicode const aHGPMinchoLSun[] = { 'h','g','p', 0x660E, 0x671D, 'l', 's', 'u', 'n', 0 };
static sal_Unicode const aHGGothicBSun[] = { 'h', 'g', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 's', 'u', 'n', 0 };
static sal_Unicode const aHGPGothicBSun[] = { 'h', 'g', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 's', 'u', 'n', 0 };
static sal_Unicode const aHGHeiseiMin[] = { 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 0, 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 'w', '3', 'x', '1', '2', 0, 0 };
static sal_Unicode const aIPAMincho[] =  { 'i', 'p', 'a', 0x660E, 0x671D, 0 };
static sal_Unicode const aIPAPMincho[] = { 'i', 'p', 'a', 'p', 0x660E, 0x671D, 0 };
static sal_Unicode const aIPAGothic[] =  { 'i', 'p', 'a',  0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
static sal_Unicode const aIPAPGothic[] =  { 'i', 'p', 'a', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
static sal_Unicode const aIPAUIGothic[] =  { 'i', 'p', 'a', 'u', 'i', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
static sal_Unicode const aSazanamiMincho[] = { 0x3055, 0x3056, 0x306A, 0x307F, 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aSazanamiGothic[] = { 0x3055, 0x3056, 0x306A, 0x307F, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aKochiMincho[] = { 0x6771, 0x98A8, 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aKochiGothic[] = { 0x6771, 0x98A8, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aSunDotum[] = { 0xC36C, 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aSunGulim[] = { 0xC36C, 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aSunBatang[] = { 0xC36C, 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aBaekmukDotum[] = { 0xBC31, 0xBB35, 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aBaekmukGulim[] = { 0xBC31, 0xBB35, 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aBaekmukBatang[] = { 0xBC31, 0xBB35, 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aFzMingTi[] = { 0x65B9, 0x6B63, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aFzHeiTiTW[]= { 0x65B9, 0x6B63, 0x9ED1, 0x9AD4, 0, 0 };
static sal_Unicode const aFzKaiTiTW[]= { 0x65B9, 0x6B63, 0x6977, 0x9AD4, 0, 0 };
static sal_Unicode const aFzHeiTiCN[]= { 0x65B9, 0x6B63, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aFzKaiTiCN[]= { 0x65B9, 0x6B63, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFzSongTi[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aHYMyeongJoExtra[]         = { 'h', 'y', 0xACAC, 0xBA85, 0xC870, 0, 0 };
static sal_Unicode const aHYSinMyeongJoMedium[]     = { 'h', 'y', 0xC2E0, 0xBA85, 0xC870, 0, 0 };
static sal_Unicode const aHYGothicMedium[]          = { 'h', 'y', 0xC911, 0xACE0, 0xB515, 0, 0 };
static sal_Unicode const aHYGraphicMedium[]         = { 'h', 'y', 0xADF8, 0xB798, 0xD53D, 'm', 0, 0 };
static sal_Unicode const aHYGraphic[]               = { 'h', 'y', 0xADF8, 0xB798, 0xD53D, 0, 0 };
static sal_Unicode const aNewGulim[]                = { 0xC0C8, 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aSunGungseo[]              = { 0xC36C, 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aHYGungSoBold[]            = { 'h','y', 0xAD81, 0xC11C, 'b', 0, 0 };
static sal_Unicode const aHYGungSo[]                 = { 'h','y', 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aSunHeadLine[]             = { 0xC36C, 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 0, 0 };
static sal_Unicode const aHYHeadLineMedium[]        = { 'h', 'y', 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 'm', 0, 0 };
static sal_Unicode const aHYHeadLine[]              = { 'h', 'y', 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 0, 0 };
static sal_Unicode const aYetR[]                    = { 0xD734, 0xBA3C, 0xC61B, 0xCCB4, 0, 0 };
static sal_Unicode const aHYGothicExtra[]           = { 'h', 'y', 0xACAC, 0xACE0, 0xB515, 0, 0 };
static sal_Unicode const aSunMokPan[]               = { 0xC36C, 0xBAA9, 0xD310, 0, 0 };
static sal_Unicode const aSunYeopseo[]              = { 0xC36C, 0xC5FD, 0xC11C, 0, 0 };
static sal_Unicode const aSunBaekSong[]              = { 0xC36C, 0xBC31, 0xC1A1, 0, 0 };
static sal_Unicode const aHYPostLight[]             = { 'h', 'y', 0xC5FD, 0xC11C, 'l', 0, 0 };
static sal_Unicode const aHYPost[]                  = { 'h', 'y', 0xC5FD, 0xC11C, 0, 0 };
static sal_Unicode const aMagicR[]                  = { 0xD734, 0xBA3C, 0xB9E4, 0xC9C1, 0xCCB4, 0, 0 };
static sal_Unicode const aSunCrystal[]              = { 0xC36C, 0xD06C, 0xB9AC, 0xC2A4, 0xD0C8, 0, 0 };
static sal_Unicode const aSunSaemmul[]              = { 0xC36C, 0xC0D8, 0xBB3C, 0, 0 };
static sal_Unicode const aHaansoftBatang[]          = { 0xD55C, 0xCEF4, 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aHaansoftDotum[]           = { 0xD55C, 0xCEF4, 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aHyhaeseo[]                = { 0xD55C, 0xC591, 0xD574, 0xC11C, 0, 0 };
static sal_Unicode const aMDSol[]                   = { 'm', 'd', 0xC194, 0xCCB4, 0, 0 };
static sal_Unicode const aMDGaesung[]               = { 'm', 'd', 0xAC1C, 0xC131, 0xCCB4, 0, 0 };
static sal_Unicode const aMDArt[]                   = { 'm', 'd', 0xC544, 0xD2B8, 0xCCB4, 0, 0 };
static sal_Unicode const aMDAlong[]                 = { 'm', 'd', 0xC544, 0xB871, 0xCCB4, 0, 0 };
static sal_Unicode const aMDEasop[]                 = { 'm', 'd', 0xC774, 0xC19D, 0xCCB4, 0, 0 };
static sal_Unicode const aHYShortSamulMedium[]      = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 'm', 0 };
static sal_Unicode const aHYShortSamul[]            = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 0 };
static sal_Unicode const aHGGothicE[]               = { 'h','g', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'e', 0 };
static sal_Unicode const aHGPGothicE[]              = { 'h','g','p', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'e', 0 };
static sal_Unicode const aHGSGothicE[]              = { 'h','g','s', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'e', 0 };
static sal_Unicode const aHGGothicM[]               = { 'h','g', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'm', 0 };
static sal_Unicode const aHGPGothicM[]              = { 'h','g','p', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'm', 0 };
static sal_Unicode const aHGSGothicM[]              = { 'h','g','s', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'm', 0 };
static sal_Unicode const aHGGyoshotai[]             = { 'h','g', 0x884C, 0x66F8, 0x4F53, 0 };
static sal_Unicode const aHGPGyoshotai[]            = { 'h','g','p', 0x884C, 0x66F8, 0x4F53, 0 };
static sal_Unicode const aHGSGyoshotai[]            = { 'h','g','s', 0x884C, 0x66F8, 0x4F53, 0 };
static sal_Unicode const aHGKyokashotai[]           = { 'h','g', 0x6559, 0x79D1, 0x66F8, 0x4F53, 0 };
static sal_Unicode const aHGPKyokashotai[]          = { 'h','g','p', 0x6559, 0x79D1, 0x66F8, 0x4F53, 0 };
static sal_Unicode const aHGSKyokashotai[]          = { 'h','g','s', 0x6559, 0x79D1, 0x66F8, 0x4F53, 0 };
static sal_Unicode const aHGMinchoB[]               = { 'h','g', 0x660E, 0x671D, 'b', 0 };
static sal_Unicode const aHGPMinchoB[]              = { 'h','g','p', 0x660E, 0x671D, 'b', 0 };
static sal_Unicode const aHGSMinchoB[]              = { 'h','g','s', 0x660E, 0x671D, 'b', 0 };
static sal_Unicode const aHGMinchoE[]               = { 'h','g', 0x660E, 0x671D, 'e', 0 };
static sal_Unicode const aHGPMinchoE[]              = { 'h','g','p', 0x660E, 0x671D, 'e', 0 };
static sal_Unicode const aHGSMinchoE[]              = { 'h','g','s', 0x660E, 0x671D, 'e', 0 };
static sal_Unicode const aHGSoeiKakupoptai[]        = { 'h','g', 0x5275,0x82F1,0x89D2,0xFF8E,
                            0xFF9F,0xFF6F,0xFF8C,0xFF9F,0x4F53,0};
static sal_Unicode const aHGPSoeiKakupoptai[]       = { 'h','g', 'p', 0x5275,0x82F1,0x89D2,0xFF8E,
                            0xFF9F,0xFF6F,0xFF8C,0xFF9F,0x4F53,0};
static sal_Unicode const aHGSSoeiKakupoptai[]       = { 'h','g', 's', 0x5275,0x82F1,0x89D2,0xFF8E,
                            0xFF9F,0xFF6F,0xFF8C,0xFF9F,0x4F53,0};
static sal_Unicode const aHGSoeiPresenceEB[]        = { 'h','g', 0x5275,0x82F1,0xFF8C,0xFF9F,
                            0xFF9A,0xFF7E,0xFF9E,0xFF9D,0xFF7D, 'e','b',0};
static sal_Unicode const aHGPSoeiPresenceEB[]       = { 'h','g','p', 0x5275,0x82F1,0xFF8C,0xFF9F,
                            0xFF9A,0xFF7E,0xFF9E,0xFF9D,0xFF7D, 'e','b',0};
static sal_Unicode const aHGSSoeiPresenceEB[]       = { 'h','g','s', 0x5275,0x82F1,0xFF8C,0xFF9F,
                            0xFF9A,0xFF7E,0xFF9E,0xFF9D,0xFF7D, 'e','b',0};
static sal_Unicode const aHGSoeiKakugothicUB[]      = { 'h','g', 0x5275,0x82F1,0x89D2,0xFF7A,
                            0xFF9E,0xFF7C,0xFF6F,0xFF78,'u','b',0};
static sal_Unicode const aHGPSoeiKakugothicUB[]     = { 'h','g','p', 0x5275,0x82F1,0x89D2,0xFF7A,
                            0xFF9E,0xFF7C,0xFF6F,0xFF78,'u','b',0};
static sal_Unicode const aHGSSoeiKakugothicUB[]     = { 'h','g','s', 0x5275,0x82F1,0x89D2,0xFF7A,
                            0xFF9E,0xFF7C,0xFF6F,0xFF78,'u','b',0};
static sal_Unicode const aHGSeikaishotaiPRO[]       = { 'h','g', 0x6B63,0x6977,0x66F8,0x4F53, '-','p','r','o',0};
static sal_Unicode const aHGMaruGothicMPRO[]        = { 'h','g', 0x4E38,0xFF7A,0xFF9E,0xFF7C,0xFF6F,0xFF78, '-','p','r','o',0};
static sal_Unicode const aHiraginoMinchoPro[]       = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x660E, 0x671D, 'p','r','o',0};
static sal_Unicode const aHiraginoMinchoProN[]      = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x660E, 0x671D, 'p','r','o','n',0};
static sal_Unicode const aHiraginoKakuGothicPro[]   = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x89D2, 0x30B4, 'p','r','o',0};
static sal_Unicode const aHiraginoKakuGothicProN[]  = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x89D2, 0x30B4, 'p','r','o','n',0};
static sal_Unicode const aHiraginoMaruGothicPro[]   = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x4E38, 0x30B4, 'p','r','o',0};
static sal_Unicode const aHiraginoMaruGothicProN[]  = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x4E38, 0x30B4, 'p','r','o','n',0};

static ImplLocalizedFontName aImplLocalizedNamesList[] =
{
{   "batang",               aBatang },
{   "batangche",            aBatangChe },
{   "gungshu",              aGungsuh },
{   "gungshuche",           aGungsuhChe },
{   "gulim",                aGulim },
{   "gulimche",             aGulimChe },
{   "dotum",                aDotum },
{   "dotumche",             aDotumChe },
{   "simsun",               aSimSun },
{   "nsimsun",              aNSimSun },
{   "simhei",               aSimHei },
{   "simkai",               aSimKai },
{   "zycjksun",             azycjkSun },
{   "zycjkhei",             azycjkHei },
{   "zycjkkai",             azycjkKai },
{   "fzhei",                aFZHei },
{   "fzkai",                aFZKai },
{   "fzsong",               aFZSongYI },
{   "fzshusong",            aFZShuSong },
{   "fzfangsong",           aFZFangSong },
{   "mhei",                 aMHei },
{   "mkai",                 aMKai },
{   "msong",                aMSong },
{   "cfangsong",            aCFangSong },
{   "mingliu",              aMingLiU },
{   "pmingliu",             aPMingLiU },
{   "hei",                  aHei },
{   "kai",                  aKai },
{   "ming",                 aMing },
{   "msgothic",             aMSGothic },
{   "mspgothic",            aMSPGothic },
{   "msmincho",             aMSMincho },
{   "mspmincho",            aMSPMincho },
{   "microsoftjhenghei",    aMSJhengHei },
{   "microsoftyahei",       aMSYaHei },
{   "meiryo",               aMeiryo },
{   "hgminchol",            aHGMinchoL },
{   "hggothicb",            aHGGothicB },
{   "hgpminchol",           aHGPMinchoL },
{   "hgpgothicb",           aHGPGothicB },
{   "hgmincholsun",         aHGMinchoLSun },
{   "hggothicbsun",         aHGGothicBSun },
{   "hgpmincholsun",        aHGPMinchoLSun },
{   "hgpgothicbsun",        aHGPGothicBSun },
{   "hgheiseimin",          aHGHeiseiMin },
{   "ipamincho",            aIPAMincho },
{   "ipapmincho",           aIPAPMincho },
{   "ipagothic",            aIPAGothic },
{   "ipapgothic",           aIPAPGothic },
{   "ipauigothic",          aIPAUIGothic },
{   "sazanamimincho",       aSazanamiMincho },
{   "sazanamigothic",       aSazanamiGothic },
{   "kochimincho",          aKochiMincho },
{   "kochigothic",          aKochiGothic },
{   "sundotum",             aSunDotum },
{   "sungulim",             aSunGulim },
{   "sunbatang",            aSunBatang },
{   "baekmukdotum",         aBaekmukDotum },
{   "baekmukgulim",         aBaekmukGulim },
{   "baekmukbatang",        aBaekmukBatang },
{   "fzheiti",              aFzHeiTiCN },
{   "fzheiti",              aFzHeiTiTW },
{   "fzkaiti",              aFzKaiTiCN },
{   "fzkaitib",             aFzKaiTiTW },
{   "fzmingtib",            aFzMingTi },
{   "fzsongti",             aFzSongTi },
{   "hymyeongjoextra",      aHYMyeongJoExtra },
{   "hysinmyeongjomedium",  aHYSinMyeongJoMedium },
{   "hygothicmedium",       aHYGothicMedium },
{   "hygraphicmedium",      aHYGraphicMedium },
{   "hygraphic",            aHYGraphic },
{   "newgulim",             aNewGulim },
{   "sungungseo",           aSunGungseo },
{   "hygungsobold",         aHYGungSoBold },
{   "hygungso",             aHYGungSo },
{   "sunheadline",          aSunHeadLine },
{   "hyheadlinemedium",     aHYHeadLineMedium },
{   "hyheadline",           aHYHeadLine },
{   "yetr",                 aYetR },
{   "hygothicextra",        aHYGothicExtra },
{   "sunmokpan",            aSunMokPan },
{   "sunyeopseo",           aSunYeopseo },
{   "sunbaeksong",          aSunBaekSong },
{   "hypostlight",          aHYPostLight },
{   "hypost",               aHYPost },
{   "magicr",               aMagicR },
{   "suncrystal",           aSunCrystal },
{   "sunsaemmul",           aSunSaemmul },
{   "hyshortsamulmedium",   aHYShortSamulMedium },
{   "hyshortsamul",         aHYShortSamul },
{   "haansoftbatang",       aHaansoftBatang },
{   "haansoftdotum",        aHaansoftDotum },
{   "hyhaeseo",             aHyhaeseo },
{   "mdsol",                aMDSol },
{   "mdgaesung",            aMDGaesung },
{   "mdart",                aMDArt },
{   "mdalong",              aMDAlong },
{   "mdeasop",              aMDEasop },
{   "hggothice",            aHGGothicE },
{   "hgpgothice",           aHGPGothicE },
{   "hgpothice",            aHGSGothicE },
{   "hggothicm",            aHGGothicM },
{   "hgpgothicm",           aHGPGothicM },
{   "hgpgothicm",           aHGSGothicM },
{   "hggyoshotai",          aHGGyoshotai },
{   "hgpgyoshotai",         aHGPGyoshotai },
{   "hgsgyoshotai",         aHGSGyoshotai },
{   "hgkyokashotai",        aHGKyokashotai },
{   "hgpkyokashotai",       aHGPKyokashotai },
{   "hgskyokashotai",       aHGSKyokashotai },
{   "hgminchob",            aHGMinchoB },
{   "hgpminchob",           aHGPMinchoB },
{   "hgsminchob",           aHGSMinchoB },
{   "hgminchoe",            aHGMinchoE },
{   "hgpminchoe",           aHGPMinchoE },
{   "hgsminchoe",           aHGSMinchoE },
{   "hgsoeikakupoptai",     aHGSoeiKakupoptai },
{   "hgpsoeikakupopta",     aHGPSoeiKakupoptai },
{   "hgssoeikakupopta",     aHGSSoeiKakupoptai },
{   "hgsoeipresenceeb",     aHGSoeiPresenceEB },
{   "hgpsoeipresenceeb",    aHGPSoeiPresenceEB },
{   "hgssoeipresenceeb",    aHGSSoeiPresenceEB },
{   "hgsoeikakugothicub",   aHGSoeiKakugothicUB },
{   "hgpsoeikakugothicub",  aHGPSoeiKakugothicUB },
{   "hgssoeikakugothicub",  aHGSSoeiKakugothicUB },
{   "hgseikaishotaipro",    aHGSeikaishotaiPRO },
{   "hgmarugothicmpro",     aHGMaruGothicMPRO },
{   "hiraginominchopro",    aHiraginoMinchoPro },
{   "hiraginominchopron",   aHiraginoMinchoProN },
{   "hiraginokakugothicpro", aHiraginoKakuGothicPro },
{   "hiraginokakugothicpron", aHiraginoKakuGothicProN },
{   "hiraginomarugothicpro", aHiraginoMaruGothicPro },
{   "hiraginomarugothicpron", aHiraginoMaruGothicProN },
{   NULL,                   NULL },
};

// -----------------------------------------------------------------------

void GetEnglishSearchFontName( String& rName )
{
    bool        bNeedTranslation = false;
    xub_StrLen  nLen = rName.Len();

    // Remove trailing whitespaces
    xub_StrLen i = nLen;
    while ( i && (rName.GetChar( i-1 ) < 32) )
        i--;
    if ( i != nLen )
        rName.Erase( i );

    // Remove Script at the end
    // Scriptname must be the last part of the fontname and
    // looks like "fontname (scriptname)". So there can only be a
    // script name at the and of the fontname, when the last char is ')'
    if ( (nLen >= 3) && rName.GetChar( nLen-1 ) == ')' )
    {
        int nOpen = 1;
        xub_StrLen nTempLen = nLen-2;
        while ( nTempLen )
        {
            if ( rName.GetChar( nTempLen ) == '(' )
            {
                nOpen--;
                if ( !nOpen )
                {
                    // Remove Space at the end
                    if ( nTempLen && (rName.GetChar( nTempLen-1 ) == ' ') )
                        nTempLen--;
                    rName.Erase( nTempLen );
                    nLen = nTempLen;
                    break;
                }
            }
            if ( rName.GetChar( nTempLen ) == ')' )
                nOpen++;
            nTempLen--;
        }
    }

    // remove all whitespaces and converts to lower case ASCII
    // TODO: better transliteration to ASCII e.g. all digits
    i = 0;
    while ( i < nLen )
    {
        sal_Unicode c = rName.GetChar( i );
        if ( c > 127 )
        {
            // Translate to Lowercase-ASCII
            // FullWidth-ASCII to half ASCII
            if ( (c >= 0xFF00) && (c <= 0xFF5E) )
            {
                c -= 0xFF00-0x0020;
                // Upper to Lower
                if ( (c >= 'A') && (c <= 'Z') )
                    c += 'a' - 'A';
                rName.SetChar( i, c );
            }
            else
            {
                // Only Fontnames with None-Ascii-Characters must be translated
                bNeedTranslation = true;
            }
        }
        // not lowercase Ascii
        else if ( !((c >= 'a') && (c <= 'z')) )
        {
            // To Lowercase-Ascii
            if ( (c >= 'A') && (c <= 'Z') )
            {
                c += 'a' - 'A';
                rName.SetChar( i, c );
            }
            else if( ((c < '0') || (c > '9')) && (c != ';') ) // not 0-9 or semicolon
            {
                // Remove white spaces and special characters
                rName.Erase( i, 1 );
                nLen--;
                continue;
            }
        }

        i++;
    }

    // translate normalized localized name to its normalized English ASCII name
    if( bNeedTranslation )
    {
        typedef std::hash_map<const String, const char*,FontNameHash> FontNameDictionary;
        static FontNameDictionary aDictionary( sizeof(aImplLocalizedNamesList) / sizeof(*aImplLocalizedNamesList) );
        // the font name dictionary needs to be intialized once
        if( aDictionary.empty() )
        {
            // TODO: check if all dictionary entries are already normalized?
            const ImplLocalizedFontName* pList = aImplLocalizedNamesList;
            for(; pList->mpEnglishName; ++pList )
                aDictionary[ pList->mpLocalizedNames ] = pList->mpEnglishName;
        }

        FontNameDictionary::const_iterator it = aDictionary.find( rName );
        if( it != aDictionary.end() )
            rName.AssignAscii( it->second );
    }
}

// -----------------------------------------------------------------------

String GetNextFontToken( const String& rTokenStr, xub_StrLen& rIndex )
{
    // check for valid start index
    int nStringLen = rTokenStr.Len();
    if( rIndex >= nStringLen )
    {
        rIndex = STRING_NOTFOUND;
        return String();
    }

    // find the next token delimiter and return the token substring
    const sal_Unicode* pStr = rTokenStr.GetBuffer() + rIndex;
    const sal_Unicode* pEnd = rTokenStr.GetBuffer() + nStringLen;
    for(; pStr < pEnd; ++pStr )
        if( (*pStr == ';') || (*pStr == ',') )
            break;

    xub_StrLen nTokenStart = rIndex;
    xub_StrLen nTokenLen;
    if( pStr < pEnd )
    {
        rIndex = sal::static_int_cast<xub_StrLen>(pStr - rTokenStr.GetBuffer());
        nTokenLen = rIndex - nTokenStart;
        ++rIndex; // skip over token separator
    }
    else
    {
        // no token delimiter found => handle last token
        rIndex = STRING_NOTFOUND;
        nTokenLen = STRING_LEN;

        // optimize if the token string consists of just one token
        if( !nTokenStart )
            return rTokenStr;
    }

    return String( rTokenStr, nTokenStart, nTokenLen );
}

// TODO: get rid of this in another incompatible build with SW project.
// SW's WW8 and RTF filters still use this (from fontcvt.hxx)
String GetFontToken( const String& rTokenStr, xub_StrLen nToken, xub_StrLen& rIndex )
{
    // skip nToken Tokens
    for( xub_StrLen i = 0; (i < nToken) && (rIndex != STRING_NOTFOUND); ++i )
        GetNextFontToken( rTokenStr, rIndex );

    return GetNextFontToken( rTokenStr, rIndex );
}

// =======================================================================

static bool ImplIsFontToken( const String& rName, const String& rToken )
{
    String      aTempName;
    xub_StrLen  nIndex = 0;
    do
    {
        aTempName = GetNextFontToken( rName, nIndex );
        if ( rToken == aTempName )
            return true;
    }
    while ( nIndex != STRING_NOTFOUND );

    return false;
}

// -----------------------------------------------------------------------

static void ImplAppendFontToken( String& rName, const String& rNewToken )
{
    if ( rName.Len() )
    {
        rName.Append( ';' );
        rName.Append( rNewToken );
    }
    else
        rName = rNewToken;
}

void AddTokenFontName( String& rName, const String& rNewToken )
{
    if ( !ImplIsFontToken( rName, rNewToken ) )
        ImplAppendFontToken( rName, rNewToken );
}

// =======================================================================

String GetSubsFontName( const String& rName, sal_uLong nFlags )
{
    String aName;

    xub_StrLen nIndex = 0;
    String aOrgName = GetNextFontToken( rName, nIndex );
    GetEnglishSearchFontName( aOrgName );

    // #93662# do not try to replace StarSymbol with MS only font
    if( nFlags == (SUBSFONT_MS|SUBSFONT_ONLYONE)
    &&  ( aOrgName.EqualsAscii( "starsymbol" )
      ||  aOrgName.EqualsAscii( "opensymbol" ) ) )
        return aName;

    const utl::FontNameAttr* pAttr = utl::FontSubstConfiguration::get()->getSubstInfo( aOrgName );
    if ( pAttr )
    {
        for( int i = 0; i < 3; i++ )
        {
            const ::std::vector< String >* pVector = NULL;
            switch( i )
            {
                case 0:
                    if( nFlags & SUBSFONT_MS  &&  pAttr->MSSubstitutions.size() )
                        pVector = &pAttr->MSSubstitutions;
                    break;
                case 1:
                    if( nFlags & SUBSFONT_PS  &&  pAttr->PSSubstitutions.size() )
                        pVector = &pAttr->PSSubstitutions;
                    break;
                case 2:
                    if( nFlags & SUBSFONT_HTML  &&  pAttr->HTMLSubstitutions.size() )
                        pVector = &pAttr->HTMLSubstitutions;
                    break;
            }
            if( ! pVector )
                continue;
            for( ::std::vector< String >::const_iterator it = pVector->begin(); it != pVector->end(); ++it )
                if( ! ImplIsFontToken( rName, *it ) )
                {
                    ImplAppendFontToken( aName, *it );
                    if( nFlags & SUBSFONT_ONLYONE )
                    {
                        i = 4;
                        break;
                    }
                }
        }
    }

    return aName;
}

// -----------------------------------------------------------------------

// TODO: use a more generic String hash
int FontNameHash::operator()( const String& rStr ) const
{
    // this simple hash just has to be good enough for font names
    int nHash = 0;
    const int nLen = rStr.Len();
    const sal_Unicode* p = rStr.GetBuffer();
    switch( nLen )
    {
        default: nHash = (p[0]<<16) - (p[1]<<8) + p[2];
                 nHash += nLen;
                 p += nLen - 3;
                 // fall through
        case 3:  nHash += (p[2]<<16);   // fall through
        case 2:  nHash += (p[1]<<8);    // fall through
        case 1:  nHash += p[0];         // fall through
        case 0:  break;
    };

    return nHash;
}

