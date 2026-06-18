/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <unotools/fontdefs.hxx>
#include <unotools/fontcfg.hxx>
#include <rtl/ustrbuf.hxx>

#include <string_view>
#include <unordered_map>

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

//return true if the character is stripped from the string
static bool toOnlyLowerAsciiOrStrip(sal_Unicode c, OUStringBuffer &rName, sal_Int32 nIndex, sal_Int32& rLen)
{
    // not lowercase Ascii
    if (c < 'a' || c > 'z')
    {
        // To Lowercase-Ascii
        if ( (c >= 'A') && (c <= 'Z') )
        {
            c += 'a' - 'A';
            rName[nIndex] = c;
        }
        else if( ((c < '0') || (c > '9')) && (c != ';') && (c != '(') && (c != ')') ) // not 0-9, semicolon, or brackets
        {
            // Remove white spaces and special characters
            rName.remove(nIndex, 1);
            rLen--;
            return true;
        }
    }
    return false;
}

OUString GetEnglishSearchFontName(std::u16string_view rInName)
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
                rName[ i ] = c;
                if (toOnlyLowerAsciiOrStrip(c, rName, i, nLen))
                   continue;
            }
            else
            {
                // Only Fontnames with None-Ascii-Characters must be translated
                bNeedTranslation = true;
            }
        }
        else if (toOnlyLowerAsciiOrStrip(c, rName, i, nLen))
            continue;

        i++;
    }
    OUString rNameStr = rName.makeStringAndClear();
    // translate normalized localized name to its normalized English ASCII name
    if( bNeedTranslation )
    {
        typedef std::unordered_map<OUString, OUString> FontNameDictionary;
        static FontNameDictionary const aDictionary = {
            {u"\uBC14\uD0D5"_ustr, u"batang"_ustr},
            {u"\uBC14\uD0D5\uCCB4"_ustr, u"batangche"_ustr},
            {u"\uAD81\uC11C"_ustr, u"gungshu"_ustr},
            {u"\uAD81\uC11C\uCCB4"_ustr, u"gungshuche"_ustr},
            {u"\uAD74\uB9BC"_ustr, u"gulim"_ustr},
            {u"\uAD74\uB9BC\uCCB4"_ustr, u"gulimche"_ustr},
            {u"\uB3CB\uC6C0"_ustr, u"dotum"_ustr},
            {u"\uB3CB\uC6C0\uCCB4"_ustr, u"dotumche"_ustr},
            {u"\u5B8B\u4F53"_ustr, u"simsun"_ustr},
            {u"\u65B0\u5B8B\u4F53"_ustr, u"nsimsun"_ustr},
            {u"\u9ED1\u4F53"_ustr, u"simhei"_ustr},
            {u"\u6977\u4F53"_ustr, u"simkai"_ustr},
            {u"\u4E2D\u6613\u5B8B\u4F53"_ustr, u"zycjksun"_ustr},
            {u"\u4E2D\u6613\u9ED1\u4F53"_ustr, u"zycjkhei"_ustr},
            {u"\u4E2D\u6613\u6977\u4F53"_ustr, u"zycjkkai"_ustr},
            {u"\u65B9\u6B63\u9ED1\u4F53"_ustr, u"fzhei"_ustr},
            {u"\u65B9\u6B63\u6977\u4F53"_ustr, u"fzkai"_ustr},
            {u"\u65B9\u6B63\u5B8B\u4E00"_ustr, u"fzsong"_ustr},
            {u"\u65B9\u6B63\u4E66\u5B8B"_ustr, u"fzshusong"_ustr},
            {u"\u65B9\u6B63\u4EFF\u5B8B"_ustr, u"fzfangsong"_ustr},
            // Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
            // First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
            {u"m\u7B80\u9ED1"_ustr, u"mhei"_ustr},
            {u"m\u6F60\u7AAA"_ustr, u"mhei"_ustr},
            {u"m\u7B80\u6977\u566C"_ustr, u"mkai"_ustr},
            {u"m\u6F60\u7FF1\u628E"_ustr, u"mkai"_ustr},
            {u"m\u7B80\u5B8B"_ustr, u"msong"_ustr},
            {u"m\u6F60\u51BC"_ustr, u"msong"_ustr},
            {u"m\u7B80\u592B\u5B8B"_ustr, u"cfangsong"_ustr},
            {u"m\u6F60\u6E98\u51BC"_ustr, u"cfangsong"_ustr},
            {u"\u7D30\u660E\u9AD4"_ustr, u"mingliu"_ustr},
            {u"\u65B0\u7D30\u660E\u9AD4"_ustr, u"pmingliu"_ustr},
            {u"\u6865"_ustr, u"hei"_ustr},
            {u"\u6B61"_ustr, u"kai"_ustr},
            {u"\u6D69\u6E67"_ustr, u"ming"_ustr},
            {u"ms\u30B4\u30B7\u30C3\u30AF"_ustr, u"msgothic"_ustr},
            {u"msp\u30B4\u30B7\u30C3\u30AF"_ustr, u"mspgothic"_ustr},
            {u"ms\u660E\u671D"_ustr, u"msmincho"_ustr},
            {u"msp\u660E\u671D"_ustr, u"mspmincho"_ustr},
            {u"\u5FAE\u8EDF\u6B63\u9ED1\u9AD4"_ustr, u"microsoftjhenghei"_ustr},
            {u"\u5FAE\u8F6F\u96C5\u9ED1"_ustr, u"microsoftyahei"_ustr},
            {u"\u30e1\u30a4\u30ea\u30aa"_ustr, u"meiryo"_ustr},
            {u"hg\u660E\u671Dl"_ustr, u"hgminchol"_ustr},
            {u"hg\u30B4\u30B7\u30C3\u30AFb"_ustr, u"hggothicb"_ustr},
            {u"hgp\u660E\u671Dl"_ustr, u"hgpminchol"_ustr},
            {u"hgp\u30B4\u30B7\u30C3\u30AFb"_ustr, u"hgpgothicb"_ustr},
            {u"hg\u660E\u671Dlsun"_ustr, u"hgmincholsun"_ustr},
            {u"hg\u30B4\u30B7\u30C3\u30AFbsun"_ustr, u"hggothicbsun"_ustr},
            {u"hgp\u660E\u671Dlsun"_ustr, u"hgpmincholsun"_ustr},
            {u"hgp\u30B4\u30B7\u30C3\u30AFbsun"_ustr, u"hgpgothicbsun"_ustr},
            {u"hg\u5E73\u6210\u660E\u671D\u4F53"_ustr, u"hgheiseimin"_ustr},
            {u"hg\u5E73\u6210\u660E\u671D\u4F53w3x12"_ustr, u"hgheiseimin"_ustr},
            {u"ipa\u660E\u671D"_ustr, u"ipamincho"_ustr},
            {u"ipap\u660E\u671D"_ustr, u"ipapmincho"_ustr},
            {u"ipa\u30B4\u30B7\u30C3\u30AF"_ustr, u"ipagothic"_ustr},
            {u"ipap\u30B4\u30B7\u30C3\u30AF"_ustr, u"ipapgothic"_ustr},
            {u"ipaui\u30B4\u30B7\u30C3\u30AF"_ustr, u"ipauigothic"_ustr},
            {u"takao\u660E\u671D"_ustr, u"takaomincho"_ustr},
            {u"takaop\u660E\u671D"_ustr, u"takaopmincho"_ustr},
            {u"takao\u30B4\u30B7\u30C3\u30AF"_ustr, u"takaogothic"_ustr},
            {u"takaop\u30B4\u30B7\u30C3\u30AF"_ustr, u"takaopgothic"_ustr},
            {u"\u3055\u3056\u306A\u307F\u660E\u671D"_ustr, u"sazanamimincho"_ustr},
            {u"\u3055\u3056\u306A\u307F\u30B4\u30B7\u30C3\u30AF"_ustr, u"sazanamigothic"_ustr},
            {u"\u6771\u98A8\u660E\u671D"_ustr, u"kochimincho"_ustr},
            {u"\u6771\u98A8\u30B4\u30B7\u30C3\u30AF"_ustr, u"kochigothic"_ustr},
            {u"\uC36C\uB3CB\uC6C0"_ustr, u"sundotum"_ustr},
            {u"\uC36C\uAD74\uB9BC"_ustr, u"sungulim"_ustr},
            {u"\uC36C\uBC14\uD0D5"_ustr, u"sunbatang"_ustr},
            {u"\uBC31\uBB35\uB3CB\uC6C0"_ustr, u"baekmukdotum"_ustr},
            {u"\uBC31\uBB35\uAD74\uB9BC"_ustr, u"baekmukgulim"_ustr},
            {u"\uBC31\uBB35\uBC14\uD0D5"_ustr, u"baekmukbatang"_ustr},
            {u"\u65B9\u6B63\u9ED1\u4F53"_ustr, u"fzheiti"_ustr},
            {u"\u65B9\u6B63\u9ED1\u9AD4"_ustr, u"fzheiti"_ustr},
            {u"\u65B9\u6B63\u6977\u4F53"_ustr, u"fzkaiti"_ustr},
            {u"\u65B9\u6B63\u6977\u9AD4"_ustr, u"fzkaitib"_ustr},
            {u"\u65B9\u6B63\u660E\u9AD4"_ustr, u"fzmingtib"_ustr},
            {u"\u65B9\u6B63\u5B8B\u4F53"_ustr, u"fzsongti"_ustr},
            {u"hy\uACAC\uBA85\uC870"_ustr, u"hymyeongjoextra"_ustr},
            {u"hy\uC2E0\uBA85\uC870"_ustr, u"hysinmyeongjomedium"_ustr},
            {u"hy\uC911\uACE0\uB515"_ustr, u"hygothicmedium"_ustr},
            {u"hy\uADF8\uB798\uD53Dm"_ustr, u"hygraphicmedium"_ustr},
            {u"hy\uADF8\uB798\uD53D"_ustr, u"hygraphic"_ustr},
            {u"\uC0C8\uAD74\uB9BC"_ustr, u"newgulim"_ustr},
            {u"\uC36C\uAD81\uC11C"_ustr, u"sungungseo"_ustr},
            {u"hy\uAD81\uC11Cb"_ustr, u"hygungsobold"_ustr},
            {u"hy\uAD81\uC11C"_ustr, u"hygungso"_ustr},
            {u"\uC36C\uD5E4\uB4DC\uB77C\uC778"_ustr, u"sunheadline"_ustr},
            {u"hy\uD5E4\uB4DC\uB77C\uC778m"_ustr, u"hyheadlinemedium"_ustr},
            {u"hy\uD5E4\uB4DC\uB77C\uC778"_ustr, u"hyheadline"_ustr},
            {u"\uD734\uBA3C\uC61B\uCCB4"_ustr, u"yetr"_ustr},
            {u"hy\uACAC\uACE0\uB515"_ustr, u"hygothicextra"_ustr},
            {u"\uC36C\uBAA9\uD310"_ustr, u"sunmokpan"_ustr},
            {u"\uC36C\uC5FD\uC11C"_ustr, u"sunyeopseo"_ustr},
            {u"\uC36C\uBC31\uC1A1"_ustr, u"sunbaeksong"_ustr},
            {u"hy\uC5FD\uC11Cl"_ustr, u"hypostlight"_ustr},
            {u"hy\uC5FD\uC11C"_ustr, u"hypost"_ustr},
            {u"\uD734\uBA3C\uB9E4\uC9C1\uCCB4"_ustr, u"magicr"_ustr},
            {u"\uC36C\uD06C\uB9AC\uC2A4\uD0C8"_ustr, u"suncrystal"_ustr},
            {u"\uC36C\uC0D8\uBB3C"_ustr, u"sunsaemmul"_ustr},
            {u"hy\uC595\uC740\uC0D8\uBB3Cm"_ustr, u"hyshortsamulmedium"_ustr},
            {u"hy\uC595\uC740\uC0D8\uBB3C"_ustr, u"hyshortsamul"_ustr},
            {u"\uD55C\uCEF4\uBC14\uD0D5"_ustr, u"haansoftbatang"_ustr},
            {u"\uD55C\uCEF4\uB3CB\uC6C0"_ustr, u"haansoftdotum"_ustr},
            {u"\uD55C\uC591\uD574\uC11C"_ustr, u"hyhaeseo"_ustr},
            {u"md\uC194\uCCB4"_ustr, u"mdsol"_ustr},
            {u"md\uAC1C\uC131\uCCB4"_ustr, u"mdgaesung"_ustr},
            {u"md\uC544\uD2B8\uCCB4"_ustr, u"mdart"_ustr},
            {u"md\uC544\uB871\uCCB4"_ustr, u"mdalong"_ustr},
            {u"md\uC774\uC19D\uCCB4"_ustr, u"mdeasop"_ustr},
            {u"hg\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e"_ustr, u"hggothice"_ustr},
            {u"hgp\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e"_ustr, u"hgpgothice"_ustr},
            {u"hgs\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e"_ustr, u"hgsgothice"_ustr},
            {u"hg\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m"_ustr, u"hggothicm"_ustr},
            {u"hgp\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m"_ustr, u"hgpgothicm"_ustr},
            {u"hgs\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m"_ustr, u"hgsgothicm"_ustr},
            {u"hg\u884C\u66F8\u4F53"_ustr, u"hggyoshotai"_ustr},
            {u"hgp\u884C\u66F8\u4F53"_ustr, u"hgpgyoshotai"_ustr},
            {u"hgs\u884C\u66F8\u4F53"_ustr, u"hgsgyoshotai"_ustr},
            {u"hg\u6559\u79D1\u66F8\u4F53"_ustr, u"hgkyokashotai"_ustr},
            {u"hgp\u6559\u79D1\u66F8\u4F53"_ustr, u"hgpkyokashotai"_ustr},
            {u"hgs\u6559\u79D1\u66F8\u4F53"_ustr, u"hgskyokashotai"_ustr},
            {u"hg\u660E\u671Db"_ustr, u"hgminchob"_ustr},
            {u"hgp\u660E\u671Db"_ustr, u"hgpminchob"_ustr},
            {u"hgs\u660E\u671Db"_ustr, u"hgsminchob"_ustr},
            {u"hg\u660E\u671De"_ustr, u"hgminchoe"_ustr},
            {u"hgp\u660E\u671De"_ustr, u"hgpminchoe"_ustr},
            {u"hgs\u660E\u671De"_ustr, u"hgsminchoe"_ustr},
            {u"hg\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53"_ustr, u"hgsoeikakupoptai"_ustr},
            {u"hgp\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53"_ustr, u"hgpsoeikakupopta"_ustr},
            {u"hgs\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53"_ustr, u"hgssoeikakupopta"_ustr},
            {u"hg\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb"_ustr,
             u"hgsoeipresenceeb"_ustr},
            {u"hgp\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb"_ustr,
             u"hgpsoeipresenceeb"_ustr},
            {u"hgs\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb"_ustr,
             u"hgssoeipresenceeb"_ustr},
            {u"hg\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub"_ustr, u"hgsoeikakugothicub"_ustr},
            {u"hgp\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub"_ustr, u"hgpsoeikakugothicub"_ustr},
            {u"hgs\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub"_ustr, u"hgssoeikakugothicub"_ustr},
            {u"hg\u6B63\u6977\u66F8\u4F53-pro"_ustr, u"hgseikaishotaipro"_ustr},
            {u"hg\u4E38\uFF7A\uFF9E\uFF7C\uFF6F\uFF78-pro"_ustr, u"hgmarugothicmpro"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u660E\u671Dpro"_ustr, u"hiraginominchopro"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u660E\u671Dpron"_ustr, u"hiraginominchopron"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4\u30B7\u30C3\u30AF"_ustr, u"hiraginosans"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4pro"_ustr, u"hiraginokakugothicpro"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4pron"_ustr, u"hiraginokakugothicpron"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u4E38\u30B4pro"_ustr, u"hiraginomarugothicpro"_ustr},
            {u"\u30D2\u30E9\u30AE\u30CE\u4E38\u30B4pron"_ustr, u"hiraginomarugothicpron"_ustr},
            {u"\u6E38\u30B4\u30B7\u30C3\u30AF"_ustr, u"yugothic"_ustr},
            {u"\u6E38\u30B4\u30B7\u30C3\u30AF\u4F53"_ustr, u"yugothictai"_ustr},
            {u"\u6E38\u660E\u671D"_ustr, u"yumincho"_ustr},
            {u"\u6E38\u660E\u671D\u4F53"_ustr, u"yuminchotai"_ustr},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AF"_ustr, u"sourcehansans"_ustr},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AFjp"_ustr, u"sourcehansansjp"_ustr},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AFhw"_ustr, u"sourcehansanshw"_ustr},
            {u"\u6E90\u30CE\u660E\u671D"_ustr, u"sourcehanserif"_ustr},
            {u"\u6E90\u30CE\u660E\u671Djp"_ustr, u"sourcehanserifjp"_ustr},
            {u"ipamj\u660E\u671D"_ustr, u"ipamjmincho"_ustr},
            {u"ipaex\u30B4\u30B7\u30C3\u30AF"_ustr, u"ipaexgothic"_ustr},
            {u"ipaex\u660E\u671D"_ustr, u"ipaexmimcho"_ustr}};

        FontNameDictionary::const_iterator it = aDictionary.find( rNameStr );
        if( it != aDictionary.end() )
            rNameStr = it->second;
    }

    return rNameStr;
}

std::u16string_view GetNextFontToken( std::u16string_view rTokenStr, sal_Int32& rIndex )
{
    // check for valid start index
    size_t nStringLen = rTokenStr.size();
    if( o3tl::make_unsigned(rIndex) >= nStringLen )
    {
        rIndex = -1;
        return {};
    }

    // find the next token delimiter and return the token substring
    const sal_Unicode* pStr = rTokenStr.data() + rIndex;
    const sal_Unicode* pEnd = rTokenStr.data() + nStringLen;
    for(; pStr < pEnd; ++pStr )
        if( (*pStr == ';') || (*pStr == ',') )
            break;

    sal_Int32 nTokenStart = rIndex;
    sal_Int32 nTokenLen;
    if( pStr < pEnd )
    {
        rIndex = sal::static_int_cast<sal_Int32>(pStr - rTokenStr.data());
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

    return rTokenStr.substr( nTokenStart, nTokenLen );
}

static bool ImplIsFontToken( std::u16string_view rName, std::u16string_view rToken )
{
    sal_Int32  nIndex = 0;
    do
    {
        std::u16string_view aTempName = GetNextFontToken( rName, nIndex );
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

OUString GetSubsFontName( std::u16string_view rName, SubsFontFlags nFlags )
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

    if (nFlags & SubsFontFlags::MS)
    {
        const utl::FontNameAttr* pAttr = utl::FontSubstConfiguration::get().getSubstInfo( aOrgName );
        if (pAttr)
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

bool IsOpenSymbol(std::u16string_view rFontName)
{
    sal_Int32 nIndex = 0;
    std::u16string_view sFamilyNm(GetNextFontToken(rFontName, nIndex));
    return (o3tl::equalsIgnoreAsciiCase(sFamilyNm, "starsymbol") ||
        o3tl::equalsIgnoreAsciiCase(sFamilyNm, "opensymbol"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
