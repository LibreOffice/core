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

#include <unotools/fontdefs.hxx>
#include <unotools/fontcfg.hxx>
#include <rtl/ustrbuf.hxx>

#include <string_view>
#include <unordered_map>

sal_Unicode const aBatang[] = { 0xBC14, 0xD0D5, 0 };
sal_Unicode const aBatangChe[] = { 0xBC14, 0xD0D5, 0xCCB4, 0 };
sal_Unicode const aGungsuh[] = { 0xAD81, 0xC11C, 0 };
sal_Unicode const aGungsuhChe[] = { 0xAD81, 0xC11C, 0xCCB4, 0 };
sal_Unicode const aGulim[] = { 0xAD74, 0xB9BC, 0 };
sal_Unicode const aGulimChe[] = { 0xAD74, 0xB9BC, 0xCCB4, 0 };
sal_Unicode const aDotum[] = { 0xB3CB, 0xC6C0, 0 };
sal_Unicode const aDotumChe[] = { 0xB3CB, 0xC6C0, 0xCCB4, 0 };
sal_Unicode const aSimSun[] = { 0x5B8B, 0x4F53, 0 };
sal_Unicode const aNSimSun[] = { 0x65B0, 0x5B8B, 0x4F53, 0 };
sal_Unicode const aSimHei[] = { 0x9ED1, 0x4F53, 0 };
sal_Unicode const aSimKai[] = { 0x6977, 0x4F53, 0 };
sal_Unicode const azycjkSun[] = { 0x4E2D, 0x6613, 0x5B8B, 0x4F53, 0 };
sal_Unicode const azycjkHei[] = { 0x4E2D, 0x6613, 0x9ED1, 0x4F53, 0 };
sal_Unicode const azycjkKai[] = { 0x4E2D, 0x6613, 0x6977, 0x4F53, 0 };
sal_Unicode const aFZHei[] = { 0x65B9, 0x6B63, 0x9ED1, 0x4F53, 0 };
sal_Unicode const aFZKai[] = { 0x65B9, 0x6B63, 0x6977, 0x4F53, 0 };
sal_Unicode const aFZSongYI[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4E00, 0 };
sal_Unicode const aFZShuSong[] = { 0x65B9, 0x6B63, 0x4E66, 0x5B8B, 0 };
sal_Unicode const aFZFangSong[] = { 0x65B9, 0x6B63, 0x4EFF, 0x5B8B, 0 };
// Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
// First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
sal_Unicode const aMHei1[] = { 'm', 0x7B80, 0x9ED1, 0 };
sal_Unicode const aMHei2[] = { 'm', 0x6F60, 0x7AAA, 0 };
sal_Unicode const aMKai1[] = { 'm', 0x7B80, 0x6977, 0x566C, 0 };
sal_Unicode const aMKai2[] = { 'm', 0x6F60, 0x7FF1, 0x628E, 0 };
sal_Unicode const aMSong1[] = { 'm', 0x7B80, 0x5B8B, 0 };
sal_Unicode const aMSong2[] = { 'm', 0x6F60, 0x51BC, 0 };
sal_Unicode const aCFangSong1[] = { 'm', 0x7B80, 0x592B, 0x5B8B, 0 };
sal_Unicode const aCFangSong2[] = { 'm', 0x6F60, 0x6E98, 0x51BC, 0 };
sal_Unicode const aMingLiU[] = { 0x7D30, 0x660E, 0x9AD4, 0 };
sal_Unicode const aPMingLiU[] = { 0x65B0, 0x7D30, 0x660E, 0x9AD4, 0 };
sal_Unicode const aHei[] = { 0x6865, 0 };
sal_Unicode const aKai[] = { 0x6B61, 0 };
sal_Unicode const aMing[] = { 0x6D69, 0x6E67, 0 };
sal_Unicode const aMSGothic[] = { 'm','s',       0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aMSPGothic[] = { 'm','s','p',  0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aMSMincho[] = { 'm', 's',      0x660E, 0x671D, 0 };
sal_Unicode const aMSPMincho[] = { 'm','s','p',  0x660E, 0x671D, 0 };
sal_Unicode const aMSYaHei[] = { 0x5FAE, 0x8F6F, 0x96C5, 0x9ED1, 0 };
sal_Unicode const aMSJhengHei[] = { 0x5FAE, 0x8EDF, 0x6B63, 0x9ED1, 0x9AD4, 0 };
sal_Unicode const aMeiryo[]    = { 0x30e1, 0x30a4, 0x30ea, 0x30aa, 0 };
sal_Unicode const aHGMinchoL[] = { 'h','g',      0x660E, 0x671D, 'l', 0 };
sal_Unicode const aHGGothicB[] = { 'h','g',      0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 0 };
sal_Unicode const aHGPMinchoL[] = { 'h','g','p', 0x660E, 0x671D, 'l', 0 };
sal_Unicode const aHGPGothicB[] = { 'h','g','p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 0 };
sal_Unicode const aHGMinchoLSun[] = { 'h','g',   0x660E, 0x671D, 'l', 's', 'u', 'n', 0 };
sal_Unicode const aHGPMinchoLSun[] = { 'h','g','p', 0x660E, 0x671D, 'l', 's', 'u', 'n', 0 };
sal_Unicode const aHGGothicBSun[] = { 'h', 'g', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 's', 'u', 'n', 0 };
sal_Unicode const aHGPGothicBSun[] = { 'h', 'g', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 's', 'u', 'n', 0 };
sal_Unicode const aHGHeiseiMin1[] = { 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 0 };
sal_Unicode const aHGHeiseiMin2[] = { 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 'w', '3', 'x', '1', '2', 0 };
sal_Unicode const aIPAMincho[] =  { 'i', 'p', 'a', 0x660E, 0x671D, 0 };
sal_Unicode const aIPAPMincho[] = { 'i', 'p', 'a', 'p', 0x660E, 0x671D, 0 };
sal_Unicode const aIPAGothic[] =  { 'i', 'p', 'a',  0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aIPAPGothic[] =  { 'i', 'p', 'a', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aIPAUIGothic[] =  { 'i', 'p', 'a', 'u', 'i', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aTakaoMincho[] =  { 't', 'a', 'k', 'a', 'o', 0x660E, 0x671D, 0 };
sal_Unicode const aTakaoPMincho[] = { 't', 'a', 'k', 'a', 'o', 'p', 0x660E, 0x671D, 0 };
sal_Unicode const aTakaoGothic[] =  { 't', 'a', 'k', 'a', 'o',  0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aTakaoPGothic[] =  { 't', 'a', 'k', 'a', 'o', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aSazanamiMincho[] = { 0x3055, 0x3056, 0x306A, 0x307F, 0x660E, 0x671D, 0 };
sal_Unicode const aSazanamiGothic[] = { 0x3055, 0x3056, 0x306A, 0x307F, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aKochiMincho[] = { 0x6771, 0x98A8, 0x660E, 0x671D, 0 };
sal_Unicode const aKochiGothic[] = { 0x6771, 0x98A8, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };
sal_Unicode const aSunDotum[] = { 0xC36C, 0xB3CB, 0xC6C0, 0 };
sal_Unicode const aSunGulim[] = { 0xC36C, 0xAD74, 0xB9BC, 0 };
sal_Unicode const aSunBatang[] = { 0xC36C, 0xBC14, 0xD0D5, 0 };
sal_Unicode const aBaekmukDotum[] = { 0xBC31, 0xBB35, 0xB3CB, 0xC6C0, 0 };
sal_Unicode const aBaekmukGulim[] = { 0xBC31, 0xBB35, 0xAD74, 0xB9BC, 0 };
sal_Unicode const aBaekmukBatang[] = { 0xBC31, 0xBB35, 0xBC14, 0xD0D5, 0 };
sal_Unicode const aFzMingTi[] = { 0x65B9, 0x6B63, 0x660E, 0x9AD4, 0 };
sal_Unicode const aFzHeiTiTW[]= { 0x65B9, 0x6B63, 0x9ED1, 0x9AD4, 0 };
sal_Unicode const aFzKaiTiTW[]= { 0x65B9, 0x6B63, 0x6977, 0x9AD4, 0 };
sal_Unicode const aFzHeiTiCN[]= { 0x65B9, 0x6B63, 0x9ED1, 0x4F53, 0 };
sal_Unicode const aFzKaiTiCN[]= { 0x65B9, 0x6B63, 0x6977, 0x4F53, 0 };
sal_Unicode const aFzSongTi[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4F53, 0 };
sal_Unicode const aHYMyeongJoExtra[]         = { 'h', 'y', 0xACAC, 0xBA85, 0xC870, 0 };
sal_Unicode const aHYSinMyeongJoMedium[]     = { 'h', 'y', 0xC2E0, 0xBA85, 0xC870, 0 };
sal_Unicode const aHYGothicMedium[]          = { 'h', 'y', 0xC911, 0xACE0, 0xB515, 0 };
sal_Unicode const aHYGraphicMedium[]         = { 'h', 'y', 0xADF8, 0xB798, 0xD53D, 'm', 0 };
sal_Unicode const aHYGraphic[]               = { 'h', 'y', 0xADF8, 0xB798, 0xD53D, 0 };
sal_Unicode const aNewGulim[]                = { 0xC0C8, 0xAD74, 0xB9BC, 0 };
sal_Unicode const aSunGungseo[]              = { 0xC36C, 0xAD81, 0xC11C, 0 };
sal_Unicode const aHYGungSoBold[]            = { 'h','y', 0xAD81, 0xC11C, 'b', 0 };
sal_Unicode const aHYGungSo[]                 = { 'h','y', 0xAD81, 0xC11C, 0 };
sal_Unicode const aSunHeadLine[]             = { 0xC36C, 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 0 };
sal_Unicode const aHYHeadLineMedium[]        = { 'h', 'y', 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 'm', 0 };
sal_Unicode const aHYHeadLine[]              = { 'h', 'y', 0xD5E4, 0xB4DC, 0xB77C, 0xC778, 0 };
sal_Unicode const aYetR[]                    = { 0xD734, 0xBA3C, 0xC61B, 0xCCB4, 0 };
sal_Unicode const aHYGothicExtra[]           = { 'h', 'y', 0xACAC, 0xACE0, 0xB515, 0 };
sal_Unicode const aSunMokPan[]               = { 0xC36C, 0xBAA9, 0xD310, 0 };
sal_Unicode const aSunYeopseo[]              = { 0xC36C, 0xC5FD, 0xC11C, 0 };
sal_Unicode const aSunBaekSong[]              = { 0xC36C, 0xBC31, 0xC1A1, 0 };
sal_Unicode const aHYPostLight[]             = { 'h', 'y', 0xC5FD, 0xC11C, 'l', 0 };
sal_Unicode const aHYPost[]                  = { 'h', 'y', 0xC5FD, 0xC11C, 0 };
sal_Unicode const aMagicR[]                  = { 0xD734, 0xBA3C, 0xB9E4, 0xC9C1, 0xCCB4, 0 };
sal_Unicode const aSunCrystal[]              = { 0xC36C, 0xD06C, 0xB9AC, 0xC2A4, 0xD0C8, 0 };
sal_Unicode const aSunSaemmul[]              = { 0xC36C, 0xC0D8, 0xBB3C, 0 };
sal_Unicode const aHaansoftBatang[]          = { 0xD55C, 0xCEF4, 0xBC14, 0xD0D5, 0 };
sal_Unicode const aHaansoftDotum[]           = { 0xD55C, 0xCEF4, 0xB3CB, 0xC6C0, 0 };
sal_Unicode const aHyhaeseo[]                = { 0xD55C, 0xC591, 0xD574, 0xC11C, 0 };
sal_Unicode const aMDSol[]                   = { 'm', 'd', 0xC194, 0xCCB4, 0 };
sal_Unicode const aMDGaesung[]               = { 'm', 'd', 0xAC1C, 0xC131, 0xCCB4, 0 };
sal_Unicode const aMDArt[]                   = { 'm', 'd', 0xC544, 0xD2B8, 0xCCB4, 0 };
sal_Unicode const aMDAlong[]                 = { 'm', 'd', 0xC544, 0xB871, 0xCCB4, 0 };
sal_Unicode const aMDEasop[]                 = { 'm', 'd', 0xC774, 0xC19D, 0xCCB4, 0 };
sal_Unicode const aHYShortSamulMedium[]      = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 'm', 0 };
sal_Unicode const aHYShortSamul[]            = { 'h', 'y', 0xC595, 0xC740, 0xC0D8, 0xBB3C, 0 };
sal_Unicode const aHGGothicE[]               = { 'h','g', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'e', 0 };
sal_Unicode const aHGPGothicE[]              = { 'h','g','p', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'e', 0 };
sal_Unicode const aHGSGothicE[]              = { 'h','g','s', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'e', 0 };
sal_Unicode const aHGGothicM[]               = { 'h','g', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'm', 0 };
sal_Unicode const aHGPGothicM[]              = { 'h','g','p', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'm', 0 };
sal_Unicode const aHGSGothicM[]              = { 'h','g','s', 0xFF7A, 0xFF9E, 0xFF7C, 0xFF6F, 0xFF78, 'm', 0 };
sal_Unicode const aHGGyoshotai[]             = { 'h','g', 0x884C, 0x66F8, 0x4F53, 0 };
sal_Unicode const aHGPGyoshotai[]            = { 'h','g','p', 0x884C, 0x66F8, 0x4F53, 0 };
sal_Unicode const aHGSGyoshotai[]            = { 'h','g','s', 0x884C, 0x66F8, 0x4F53, 0 };
sal_Unicode const aHGKyokashotai[]           = { 'h','g', 0x6559, 0x79D1, 0x66F8, 0x4F53, 0 };
sal_Unicode const aHGPKyokashotai[]          = { 'h','g','p', 0x6559, 0x79D1, 0x66F8, 0x4F53, 0 };
sal_Unicode const aHGSKyokashotai[]          = { 'h','g','s', 0x6559, 0x79D1, 0x66F8, 0x4F53, 0 };
sal_Unicode const aHGMinchoB[]               = { 'h','g', 0x660E, 0x671D, 'b', 0 };
sal_Unicode const aHGPMinchoB[]              = { 'h','g','p', 0x660E, 0x671D, 'b', 0 };
sal_Unicode const aHGSMinchoB[]              = { 'h','g','s', 0x660E, 0x671D, 'b', 0 };
sal_Unicode const aHGMinchoE[]               = { 'h','g', 0x660E, 0x671D, 'e', 0 };
sal_Unicode const aHGPMinchoE[]              = { 'h','g','p', 0x660E, 0x671D, 'e', 0 };
sal_Unicode const aHGSMinchoE[]              = { 'h','g','s', 0x660E, 0x671D, 'e', 0 };
sal_Unicode const aHGSoeiKakupoptai[]        = { 'h','g', 0x5275,0x82F1,0x89D2,0xFF8E,
                            0xFF9F,0xFF6F,0xFF8C,0xFF9F,0x4F53,0};
sal_Unicode const aHGPSoeiKakupoptai[]       = { 'h','g', 'p', 0x5275,0x82F1,0x89D2,0xFF8E,
                            0xFF9F,0xFF6F,0xFF8C,0xFF9F,0x4F53,0};
sal_Unicode const aHGSSoeiKakupoptai[]       = { 'h','g', 's', 0x5275,0x82F1,0x89D2,0xFF8E,
                            0xFF9F,0xFF6F,0xFF8C,0xFF9F,0x4F53,0};
sal_Unicode const aHGSoeiPresenceEB[]        = { 'h','g', 0x5275,0x82F1,0xFF8C,0xFF9F,
                            0xFF9A,0xFF7E,0xFF9E,0xFF9D,0xFF7D, 'e','b',0};
sal_Unicode const aHGPSoeiPresenceEB[]       = { 'h','g','p', 0x5275,0x82F1,0xFF8C,0xFF9F,
                            0xFF9A,0xFF7E,0xFF9E,0xFF9D,0xFF7D, 'e','b',0};
sal_Unicode const aHGSSoeiPresenceEB[]       = { 'h','g','s', 0x5275,0x82F1,0xFF8C,0xFF9F,
                            0xFF9A,0xFF7E,0xFF9E,0xFF9D,0xFF7D, 'e','b',0};
sal_Unicode const aHGSoeiKakugothicUB[]      = { 'h','g', 0x5275,0x82F1,0x89D2,0xFF7A,
                            0xFF9E,0xFF7C,0xFF6F,0xFF78,'u','b',0};
sal_Unicode const aHGPSoeiKakugothicUB[]     = { 'h','g','p', 0x5275,0x82F1,0x89D2,0xFF7A,
                            0xFF9E,0xFF7C,0xFF6F,0xFF78,'u','b',0};
sal_Unicode const aHGSSoeiKakugothicUB[]     = { 'h','g','s', 0x5275,0x82F1,0x89D2,0xFF7A,
                            0xFF9E,0xFF7C,0xFF6F,0xFF78,'u','b',0};
sal_Unicode const aHGSeikaishotaiPRO[]       = { 'h','g', 0x6B63,0x6977,0x66F8,0x4F53, '-','p','r','o',0};
sal_Unicode const aHGMaruGothicMPRO[]        = { 'h','g', 0x4E38,0xFF7A,0xFF9E,0xFF7C,0xFF6F,0xFF78, '-','p','r','o',0};
sal_Unicode const aHiraginoMinchoPro[]       = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x660E, 0x671D, 'p','r','o',0};
sal_Unicode const aHiraginoMinchoProN[]      = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x660E, 0x671D, 'p','r','o','n',0};
sal_Unicode const aHiraginoKakuGothic[]      = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x89D2, 0x30B4, 0x30B7, 0x30C3, 0x30AF,0};
sal_Unicode const aHiraginoKakuGothicPro[]   = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x89D2, 0x30B4, 'p','r','o',0};
sal_Unicode const aHiraginoKakuGothicProN[]  = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x89D2, 0x30B4, 'p','r','o','n',0};
sal_Unicode const aHiraginoMaruGothicPro[]   = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x4E38, 0x30B4, 'p','r','o',0};
sal_Unicode const aHiraginoMaruGothicProN[]  = { 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x4E38, 0x30B4, 'p','r','o','n',0};

OUString StripScriptFromName(const OUString& _aName)
{
    // I worry that someone will have a font which *does* have
    // e.g. "Greek" legitimately at the end of its name :-(
    const char*const suffixes[] = { " baltic",
                              " ce",
                              " cyr",
                              " greek",
                              " tur",
                              " (arabic)",
                              " (hebrew)",
                              " (thai)",
                              " (vietnamese)"
                            };

    OUString aName = _aName;
    // These can be crazily piled up, e.g. Times New Roman CYR Greek
    bool bFinished = false;
    while (!bFinished)
    {
        bFinished = true;
        for (const char* suffix : suffixes)
        {
            size_t nLen = strlen(suffix);
            if (aName.endsWithIgnoreAsciiCaseAsciiL(suffix, nLen))
            {
                bFinished = false;
                aName = aName.copy(0, aName.getLength() - nLen);
            }
        }
    }
    return aName;
}

OUString GetEnglishSearchFontName(const OUString& rInName)
{
    OUStringBuffer rName(rInName);
    bool        bNeedTranslation = false;
    sal_Int32  nLen = rName.getLength();

    // Remove trailing whitespaces
    sal_Int32 i = nLen;
    while ( i && (rName[ i-1 ] < 32) )
        i--;
    if ( i != nLen )
         rName.truncate(i);

    nLen = rName.getLength();

    // remove all whitespaces and converts to lower case ASCII
    // TODO: better transliteration to ASCII e.g. all digits
    i = 0;
    while ( i < nLen )
    {
        sal_Unicode c = rName[ i ];
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

                rName[ i ] = c;

           }
            else
            {
                // Only Fontnames with None-Ascii-Characters must be translated
                bNeedTranslation = true;
            }
        }
        // not lowercase Ascii
        else if ( (c < 'a') || (c > 'z') )
        {
            // To Lowercase-Ascii
            if ( (c >= 'A') && (c <= 'Z') )
            {
                c += 'a' - 'A';
                rName[ i ] = c;
            }
            else if( ((c < '0') || (c > '9')) && (c != ';') && (c != '(') && (c != ')') ) // not 0-9, semicolon, or brackets
            {
                // Remove white spaces and special characters
                rName.remove(i,1);
                nLen--;
                continue;
            }
        }

        i++;
    }
    OUString rNameStr = rName.makeStringAndClear();
    // translate normalized localized name to its normalized English ASCII name
    if( bNeedTranslation )
    {
        typedef std::unordered_map<OUString, const char*> FontNameDictionary;
        static FontNameDictionary const aDictionary = {
            {aBatang, "batang"},
            {aBatangChe, "batangche"},
            {aGungsuh, "gungshu"},
            {aGungsuhChe, "gungshuche"},
            {aGulim, "gulim"},
            {aGulimChe, "gulimche"},
            {aDotum, "dotum"},
            {aDotumChe, "dotumche"},
            {aSimSun, "simsun"},
            {aNSimSun, "nsimsun"},
            {aSimHei, "simhei"},
            {aSimKai, "simkai"},
            {azycjkSun, "zycjksun"},
            {azycjkHei, "zycjkhei"},
            {azycjkKai, "zycjkkai"},
            {aFZHei, "fzhei"},
            {aFZKai, "fzkai"},
            {aFZSongYI, "fzsong"},
            {aFZShuSong, "fzshusong"},
            {aFZFangSong, "fzfangsong"},
            {aMHei1, "mhei"},
            {aMHei2, "mhei"},
            {aMKai1, "mkai"},
            {aMKai2, "mkai"},
            {aMSong1, "msong"},
            {aMSong2, "msong"},
            {aCFangSong1, "cfangsong"},
            {aCFangSong2, "cfangsong"},
            {aMingLiU, "mingliu"},
            {aPMingLiU, "pmingliu"},
            {aHei, "hei"},
            {aKai, "kai"},
            {aMing, "ming"},
            {aMSGothic, "msgothic"},
            {aMSPGothic, "mspgothic"},
            {aMSMincho, "msmincho"},
            {aMSPMincho, "mspmincho"},
            {aMSJhengHei, "microsoftjhenghei"},
            {aMSYaHei, "microsoftyahei"},
            {aMeiryo, "meiryo"},
            {aHGMinchoL, "hgminchol"},
            {aHGGothicB, "hggothicb"},
            {aHGPMinchoL, "hgpminchol"},
            {aHGPGothicB, "hgpgothicb"},
            {aHGMinchoLSun, "hgmincholsun"},
            {aHGGothicBSun, "hggothicbsun"},
            {aHGPMinchoLSun, "hgpmincholsun"},
            {aHGPGothicBSun, "hgpgothicbsun"},
            {aHGHeiseiMin1, "hgheiseimin"},
            {aHGHeiseiMin2, "hgheiseimin"},
            {aIPAMincho, "ipamincho"},
            {aIPAPMincho, "ipapmincho"},
            {aIPAGothic, "ipagothic"},
            {aIPAPGothic, "ipapgothic"},
            {aIPAUIGothic, "ipauigothic"},
            {aTakaoMincho, "takaomincho"},
            {aTakaoPMincho, "takaopmincho"},
            {aTakaoGothic, "takaogothic"},
            {aTakaoPGothic, "takaopgothic"},
            {aSazanamiMincho, "sazanamimincho"},
            {aSazanamiGothic, "sazanamigothic"},
            {aKochiMincho, "kochimincho"},
            {aKochiGothic, "kochigothic"},
            {aSunDotum, "sundotum"},
            {aSunGulim, "sungulim"},
            {aSunBatang, "sunbatang"},
            {aBaekmukDotum, "baekmukdotum"},
            {aBaekmukGulim, "baekmukgulim"},
            {aBaekmukBatang, "baekmukbatang"},
            {aFzHeiTiCN, "fzheiti"},
            {aFzHeiTiTW, "fzheiti"},
            {aFzKaiTiCN, "fzkaiti"},
            {aFzKaiTiTW, "fzkaitib"},
            {aFzMingTi, "fzmingtib"},
            {aFzSongTi, "fzsongti"},
            {aHYMyeongJoExtra, "hymyeongjoextra"},
            {aHYSinMyeongJoMedium, "hysinmyeongjomedium"},
            {aHYGothicMedium, "hygothicmedium"},
            {aHYGraphicMedium, "hygraphicmedium"},
            {aHYGraphic, "hygraphic"},
            {aNewGulim, "newgulim"},
            {aSunGungseo, "sungungseo"},
            {aHYGungSoBold, "hygungsobold"},
            {aHYGungSo, "hygungso"},
            {aSunHeadLine, "sunheadline"},
            {aHYHeadLineMedium, "hyheadlinemedium"},
            {aHYHeadLine, "hyheadline"},
            {aYetR, "yetr"},
            {aHYGothicExtra, "hygothicextra"},
            {aSunMokPan, "sunmokpan"},
            {aSunYeopseo, "sunyeopseo"},
            {aSunBaekSong, "sunbaeksong"},
            {aHYPostLight, "hypostlight"},
            {aHYPost, "hypost"},
            {aMagicR, "magicr"},
            {aSunCrystal, "suncrystal"},
            {aSunSaemmul, "sunsaemmul"},
            {aHYShortSamulMedium, "hyshortsamulmedium"},
            {aHYShortSamul, "hyshortsamul"},
            {aHaansoftBatang, "haansoftbatang"},
            {aHaansoftDotum, "haansoftdotum"},
            {aHyhaeseo, "hyhaeseo"},
            {aMDSol, "mdsol"},
            {aMDGaesung, "mdgaesung"},
            {aMDArt, "mdart"},
            {aMDAlong, "mdalong"},
            {aMDEasop, "mdeasop"},
            {aHGGothicE, "hggothice"},
            {aHGPGothicE, "hgpgothice"},
            {aHGSGothicE, "hgsgothice"},
            {aHGGothicM, "hggothicm"},
            {aHGPGothicM, "hgpgothicm"},
            {aHGSGothicM, "hgsgothicm"},
            {aHGGyoshotai, "hggyoshotai"},
            {aHGPGyoshotai, "hgpgyoshotai"},
            {aHGSGyoshotai, "hgsgyoshotai"},
            {aHGKyokashotai, "hgkyokashotai"},
            {aHGPKyokashotai, "hgpkyokashotai"},
            {aHGSKyokashotai, "hgskyokashotai"},
            {aHGMinchoB, "hgminchob"},
            {aHGPMinchoB, "hgpminchob"},
            {aHGSMinchoB, "hgsminchob"},
            {aHGMinchoE, "hgminchoe"},
            {aHGPMinchoE, "hgpminchoe"},
            {aHGSMinchoE, "hgsminchoe"},
            {aHGSoeiKakupoptai, "hgsoeikakupoptai"},
            {aHGPSoeiKakupoptai, "hgpsoeikakupopta"},
            {aHGSSoeiKakupoptai, "hgssoeikakupopta"},
            {aHGSoeiPresenceEB, "hgsoeipresenceeb"},
            {aHGPSoeiPresenceEB, "hgpsoeipresenceeb"},
            {aHGSSoeiPresenceEB, "hgssoeipresenceeb"},
            {aHGSoeiKakugothicUB, "hgsoeikakugothicub"},
            {aHGPSoeiKakugothicUB, "hgpsoeikakugothicub"},
            {aHGSSoeiKakugothicUB, "hgssoeikakugothicub"},
            {aHGSeikaishotaiPRO, "hgseikaishotaipro"},
            {aHGMaruGothicMPRO, "hgmarugothicmpro"},
            {aHiraginoMinchoPro, "hiraginominchopro"},
            {aHiraginoMinchoProN, "hiraginominchopron"},
            {aHiraginoKakuGothic, "hiraginosans"},
            {aHiraginoKakuGothicPro, "hiraginokakugothicpro"},
            {aHiraginoKakuGothicProN, "hiraginokakugothicpron"},
            {aHiraginoMaruGothicPro, "hiraginomarugothicpro"},
            {aHiraginoMaruGothicProN, "hiraginomarugothicpron"}};

        FontNameDictionary::const_iterator it = aDictionary.find( rNameStr );
        if( it != aDictionary.end() )
            rNameStr = OUString::createFromAscii ( it->second );
    }

    return rNameStr;
}

OUString GetNextFontToken( const OUString& rTokenStr, sal_Int32& rIndex )
{
    // check for valid start index
    sal_Int32 nStringLen = rTokenStr.getLength();
    if( rIndex >= nStringLen )
    {
        rIndex = -1;
        return OUString();
    }

    // find the next token delimiter and return the token substring
    const sal_Unicode* pStr = rTokenStr.getStr() + rIndex;
    const sal_Unicode* pEnd = rTokenStr.getStr() + nStringLen;
    for(; pStr < pEnd; ++pStr )
        if( (*pStr == ';') || (*pStr == ',') )
            break;

    sal_Int32 nTokenStart = rIndex;
    sal_Int32 nTokenLen;
    if( pStr < pEnd )
    {
        rIndex = sal::static_int_cast<sal_Int32>(pStr - rTokenStr.getStr());
        nTokenLen = rIndex - nTokenStart;
        ++rIndex; // skip over token separator
    }
    else
    {
        // no token delimiter found => handle last token
        rIndex = -1;

        // optimize if the token string consists of just one token
        if( !nTokenStart )
        {
            return rTokenStr;
        }
        else
        {
            nTokenLen = nStringLen - nTokenStart;
        }
    }

    return rTokenStr.copy( nTokenStart, nTokenLen );
}

static bool ImplIsFontToken( const OUString& rName, std::u16string_view rToken )
{
    OUString      aTempName;
    sal_Int32  nIndex = 0;
    do
    {
        aTempName = GetNextFontToken( rName, nIndex );
        if ( rToken == aTempName )
            return true;
    }
    while ( nIndex != -1 );

    return false;
}

static void ImplAppendFontToken( OUString& rName, std::u16string_view rNewToken )
{
    if ( !rName.isEmpty() )
    {
        rName += ";";
    }
    rName += rNewToken;
}

void AddTokenFontName( OUString& rName, std::u16string_view rNewToken )
{
    if ( !ImplIsFontToken( rName, rNewToken ) )
        ImplAppendFontToken( rName, rNewToken );
}

OUString GetSubsFontName( const OUString& rName, SubsFontFlags nFlags )
{
    OUString aName;

    sal_Int32 nIndex = 0;
    OUString aOrgName = GetEnglishSearchFontName(
                                GetNextFontToken( rName, nIndex ) );

    // #93662# do not try to replace StarSymbol with MS only font
    if( nFlags == (SubsFontFlags::MS|SubsFontFlags::ONLYONE)
    &&  ( aOrgName == "starsymbol"
      ||  aOrgName == "opensymbol" ) )
        return aName;

    const utl::FontNameAttr* pAttr = utl::FontSubstConfiguration::get().getSubstInfo( aOrgName );
    if ( pAttr && (nFlags & SubsFontFlags::MS) )
    {
        for( const auto& rSubstitution : pAttr->MSSubstitutions )
            if( ! ImplIsFontToken( rName, rSubstitution ) )
            {
                ImplAppendFontToken( aName, rSubstitution );
                if( nFlags & SubsFontFlags::ONLYONE )
                {
                    break;
                }
            }
    }

    return aName;
}

bool IsStarSymbol(const OUString &rFontName)
{
    sal_Int32 nIndex = 0;
    OUString sFamilyNm(GetNextFontToken(rFontName, nIndex));
    return (sFamilyNm.equalsIgnoreAsciiCase("starsymbol") ||
        sFamilyNm.equalsIgnoreAsciiCase("opensymbol"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
