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
            {u"\uBC14\uD0D5"_ustr, "batang"},
            {u"\uBC14\uD0D5\uCCB4"_ustr, "batangche"},
            {u"\uAD81\uC11C"_ustr, "gungshu"},
            {u"\uAD81\uC11C\uCCB4"_ustr, "gungshuche"},
            {u"\uAD74\uB9BC"_ustr, "gulim"},
            {u"\uAD74\uB9BC\uCCB4"_ustr, "gulimche"},
            {u"\uB3CB\uC6C0"_ustr, "dotum"},
            {u"\uB3CB\uC6C0\uCCB4"_ustr, "dotumche"},
            {u"\u5B8B\u4F53"_ustr, "simsun"},
            {u"\u65B0\u5B8B\u4F53"_ustr, "nsimsun"},
            {u"\u9ED1\u4F53"_ustr, "simhei"},
            {u"\u6977\u4F53"_ustr, "simkai"},
            {u"\u4E2D\u6613\u5B8B\u4F53"_ustr, "zycjksun"},
            {u"\u4E2D\u6613\u9ED1\u4F53"_ustr, "zycjkhei"},
            {u"\u4E2D\u6613\u6977\u4F53"_ustr, "zycjkkai"},
            {u"\u65B9\u6B63\u9ED1\u4F53"_ustr, "fzhei"},
            {u"\u65B9\u6B63\u6977\u4F53"_ustr, "fzkai"},
            {u"\u65B9\u6B63\u5B8B\u4E00"_ustr, "fzsong"},
            {u"\u65B9\u6B63\u4E66\u5B8B"_ustr, "fzshusong"},
            {u"\u65B9\u6B63\u4EFF\u5B8B"_ustr, "fzfangsong"},
            // Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
            // First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
            {u"m\u7B80\u9ED1"_ustr, "mhei"},
            {u"m\u6F60\u7AAA"_ustr, "mhei"},
            {u"m\u7B80\u6977\u566C"_ustr, "mkai"},
            {u"m\u6F60\u7FF1\u628E"_ustr, "mkai"},
            {u"m\u7B80\u5B8B"_ustr, "msong"},
            {u"m\u6F60\u51BC"_ustr, "msong"},
            {u"m\u7B80\u592B\u5B8B"_ustr, "cfangsong"},
            {u"m\u6F60\u6E98\u51BC"_ustr, "cfangsong"},
            {u"\u7D30\u660E\u9AD4"_ustr, "mingliu"},
            {u"\u65B0\u7D30\u660E\u9AD4"_ustr, "pmingliu"},
            {u"\u6865"_ustr, "hei"},
            {u"\u6B61"_ustr, "kai"},
            {u"\u6D69\u6E67"_ustr, "ming"},
            {u"ms\u30B4\u30B7\u30C3\u30AF"_ustr, "msgothic"},
            {u"msp\u30B4\u30B7\u30C3\u30AF"_ustr, "mspgothic"},
            {u"ms\u660E\u671D"_ustr, "msmincho"},
            {u"msp\u660E\u671D"_ustr, "mspmincho"},
            {u"\u5FAE\u8EDF\u6B63\u9ED1\u9AD4"_ustr, "microsoftjhenghei"},
            {u"\u5FAE\u8F6F\u96C5\u9ED1"_ustr, "microsoftyahei"},
            {u"\u30e1\u30a4\u30ea\u30aa"_ustr, "meiryo"},
            {u"hg\u660E\u671Dl"_ustr, "hgminchol"},
            {u"hg\u30B4\u30B7\u30C3\u30AFb"_ustr, "hggothicb"},
            {u"hgp\u660E\u671Dl"_ustr, "hgpminchol"},
            {u"hgp\u30B4\u30B7\u30C3\u30AFb"_ustr, "hgpgothicb"},
            {u"hg\u660E\u671Dlsun"_ustr, "hgmincholsun"},
            {u"hg\u30B4\u30B7\u30C3\u30AFbsun"_ustr, "hggothicbsun"},
            {u"hgp\u660E\u671Dlsun"_ustr, "hgpmincholsun"},
            {u"hgp\u30B4\u30B7\u30C3\u30AFbsun"_ustr, "hgpgothicbsun"},
            {u"hg\u5E73\u6210\u660E\u671D\u4F53"_ustr, "hgheiseimin"},
            {u"hg\u5E73\u6210\u660E\u671D\u4F53w3x12"_ustr, "hgheiseimin"},
            {u"ipa\u660E\u671D"_ustr, "ipamincho"},
            {u"ipap\u660E\u671D"_ustr, "ipapmincho"},
            {u"ipa\u30B4\u30B7\u30C3\u30AF"_ustr, "ipagothic"},
            {u"ipap\u30B4\u30B7\u30C3\u30AF"_ustr, "ipapgothic"},
            {u"ipaui\u30B4\u30B7\u30C3\u30AF"_ustr, "ipauigothic"},
            {u"takao\u660E\u671D"_ustr, "takaomincho"},
            {u"takaop\u660E\u671D"_ustr, "takaopmincho"},
            {u"takao\u30B4\u30B7\u30C3\u30AF"_ustr, "takaogothic"},
            {u"takaop\u30B4\u30B7\u30C3\u30AF"_ustr, "takaopgothic"},
            {u"\u3055\u3056\u306A\u307F\u660E\u671D"_ustr, "sazanamimincho"},
            {u"\u3055\u3056\u306A\u307F\u30B4\u30B7\u30C3\u30AF"_ustr, "sazanamigothic"},
            {u"\u6771\u98A8\u660E\u671D"_ustr, "kochimincho"},
            {u"\u6771\u98A8\u30B4\u30B7\u30C3\u30AF"_ustr, "kochigothic"},
            {u"\uC36C\uB3CB\uC6C0"_ustr, "sundotum"},
            {u"\uC36C\uAD74\uB9BC"_ustr, "sungulim"},
            {u"\uC36C\uBC14\uD0D5"_ustr, "sunbatang"},
            {u"\uBC31\uBB35\uB3CB\uC6C0"_ustr, "baekmukdotum"},
            {u"\uBC31\uBB35\uAD74\uB9BC"_ustr, "baekmukgulim"},
            {u"\uBC31\uBB35\uBC14\uD0D5"_ustr, "baekmukbatang"},
            {u"\u65B9\u6B63\u9ED1\u4F53"_ustr, "fzheiti"},
            {u"\u65B9\u6B63\u9ED1\u9AD4"_ustr, "fzheiti"},
            {u"\u65B9\u6B63\u6977\u4F53"_ustr, "fzkaiti"},
            {u"\u65B9\u6B63\u6977\u9AD4"_ustr, "fzkaitib"},
            {u"\u65B9\u6B63\u660E\u9AD4"_ustr, "fzmingtib"},
            {u"\u65B9\u6B63\u5B8B\u4F53"_ustr, "fzsongti"},
            {u"hy\uACAC\uBA85\uC870"_ustr, "hymyeongjoextra"},
            {u"hy\uC2E0\uBA85\uC870"_ustr, "hysinmyeongjomedium"},
            {u"hy\uC911\uACE0\uB515"_ustr, "hygothicmedium"},
            {u"hy\uADF8\uB798\uD53Dm"_ustr, "hygraphicmedium"},
            {u"hy\uADF8\uB798\uD53D"_ustr, "hygraphic"},
            {u"\uC0C8\uAD74\uB9BC"_ustr, "newgulim"},
            {u"\uC36C\uAD81\uC11C"_ustr, "sungungseo"},
            {u"hy\uAD81\uC11Cb"_ustr, "hygungsobold"},
            {u"hy\uAD81\uC11C"_ustr, "hygungso"},
            {u"\uC36C\uD5E4\uB4DC\uB77C\uC778"_ustr, "sunheadline"},
            {u"hy\uD5E4\uB4DC\uB77C\uC778m"_ustr, "hyheadlinemedium"},
            {u"hy\uD5E4\uB4DC\uB77C\uC778"_ustr, "hyheadline"},
            {u"\uD734\uBA3C\uC61B\uCCB4"_ustr, "yetr"},
            {u"hy\uACAC\uACE0\uB515"_ustr, "hygothicextra"},
            {u"\uC36C\uBAA9\uD310"_ustr, "sunmokpan"},
            {u"\uC36C\uC5FD\uC11C"_ustr, "sunyeopseo"},
            {u"\uC36C\uBC31\uC1A1"_ustr, "sunbaeksong"},
            {u"hy\uC5FD\uC11Cl"_ustr, "hypostlight"},
            {u"hy\uC5FD\uC11C"_ustr, "hypost"},
            {u"\uD734\uBA3C\uB9E4\uC9C1\uCCB4"_ustr, "magicr"},
            {u"\uC36C\uD06C\uB9AC\uC2A4\uD0C8"_ustr, "suncrystal"},
            {u"\uC36C\uC0D8\uBB3C"_ustr, "sunsaemmul"},
            {u"hy\uC595\uC740\uC0D8\uBB3Cm"_ustr, "hyshortsamulmedium"},
            {u"hy\uC595\uC740\uC0D8\uBB3C"_ustr, "hyshortsamul"},
            {u"\uD55C\uCEF4\uBC14\uD0D5"_ustr, "haansoftbatang"},
            {u"\uD55C\uCEF4\uB3CB\uC6C0"_ustr, "haansoftdotum"},
            {u"\uD55C\uC591\uD574\uC11C"_ustr, "hyhaeseo"},
            {u"md\uC194\uCCB4"_ustr, "mdsol"},
            {u"md\uAC1C\uC131\uCCB4"_ustr, "mdgaesung"},
            {u"md\uC544\uD2B8\uCCB4"_ustr, "mdart"},
            {u"md\uC544\uB871\uCCB4"_ustr, "mdalong"},
            {u"md\uC774\uC19D\uCCB4"_ustr, "mdeasop"},
            {u"hg\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e"_ustr, "hggothice"},
            {u"hgp\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e"_ustr, "hgpgothice"},
            {u"hgs\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e"_ustr, "hgsgothice"},
            {u"hg\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m"_ustr, "hggothicm"},
            {u"hgp\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m"_ustr, "hgpgothicm"},
            {u"hgs\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m"_ustr, "hgsgothicm"},
            {u"hg\u884C\u66F8\u4F53"_ustr, "hggyoshotai"},
            {u"hgp\u884C\u66F8\u4F53"_ustr, "hgpgyoshotai"},
            {u"hgs\u884C\u66F8\u4F53"_ustr, "hgsgyoshotai"},
            {u"hg\u6559\u79D1\u66F8\u4F53"_ustr, "hgkyokashotai"},
            {u"hgp\u6559\u79D1\u66F8\u4F53"_ustr, "hgpkyokashotai"},
            {u"hgs\u6559\u79D1\u66F8\u4F53"_ustr, "hgskyokashotai"},
            {u"hg\u660E\u671Db"_ustr, "hgminchob"},
            {u"hgp\u660E\u671Db"_ustr, "hgpminchob"},
            {u"hgs\u660E\u671Db"_ustr, "hgsminchob"},
            {u"hg\u660E\u671De"_ustr, "hgminchoe"},
            {u"hgp\u660E\u671De"_ustr, "hgpminchoe"},
            {u"hgs\u660E\u671De"_ustr, "hgsminchoe"},
            {u"hg\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53"_ustr, "hgsoeikakupoptai"},
            {u"hgp\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53"_ustr, "hgpsoeikakupopta"},
            {u"hgs\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53"_ustr, "hgssoeikakupopta"},
            {u"hg\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb"_ustr,
             "hgsoeipresenceeb"},
            {u"hgp\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb"_ustr,
             "hgpsoeipresenceeb"},
            {u"hgs\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb"_ustr,
             "hgssoeipresenceeb"},
            {u"hg\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub"_ustr, "hgsoeikakugothicub"},
            {u"hgp\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub"_ustr, "hgpsoeikakugothicub"},
            {u"hgs\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub"_ustr, "hgssoeikakugothicub"},
            {u"hg\u6B63\u6977\u66F8\u4F53-pro"_ustr, "hgseikaishotaipro"},
            {u"hg\u4E38\uFF7A\uFF9E\uFF7C\uFF6F\uFF78-pro"_ustr, "hgmarugothicmpro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u660E\u671Dpro"_ustr, "hiraginominchopro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u660E\u671Dpron"_ustr, "hiraginominchopron"},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4\u30B7\u30C3\u30AF"_ustr, "hiraginosans"},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4pro"_ustr, "hiraginokakugothicpro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4pron"_ustr, "hiraginokakugothicpron"},
            {u"\u30D2\u30E9\u30AE\u30CE\u4E38\u30B4pro"_ustr, "hiraginomarugothicpro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u4E38\u30B4pron"_ustr, "hiraginomarugothicpron"},
            {u"\u6E38\u30B4\u30B7\u30C3\u30AF"_ustr, "yugothic"},
            {u"\u6E38\u30B4\u30B7\u30C3\u30AF\u4F53"_ustr, "yugothictai"},
            {u"\u6E38\u660E\u671D"_ustr, "yumincho"},
            {u"\u6E38\u660E\u671D\u4F53"_ustr, "yuminchotai"},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AF"_ustr, "sourcehansans"},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AFjp"_ustr, "sourcehansansjp"},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AFhw"_ustr, "sourcehansanshw"},
            {u"\u6E90\u30CE\u660E\u671D"_ustr, "sourcehanserif"},
            {u"\u6E90\u30CE\u660E\u671Djp"_ustr, "sourcehanserifjp"},
            {u"ipamj\u660E\u671D"_ustr, "ipamjmincho"},
            {u"ipaex\u30B4\u30B7\u30C3\u30AF"_ustr, "ipaexgothic"},
            {u"ipaex\u660E\u671D"_ustr, "ipaexmimcho"}};

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
