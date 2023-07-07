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
            {u"\uBC14\uD0D5", "batang"},
            {u"\uBC14\uD0D5\uCCB4", "batangche"},
            {u"\uAD81\uC11C", "gungshu"},
            {u"\uAD81\uC11C\uCCB4", "gungshuche"},
            {u"\uAD74\uB9BC", "gulim"},
            {u"\uAD74\uB9BC\uCCB4", "gulimche"},
            {u"\uB3CB\uC6C0", "dotum"},
            {u"\uB3CB\uC6C0\uCCB4", "dotumche"},
            {u"\u5B8B\u4F53", "simsun"},
            {u"\u65B0\u5B8B\u4F53", "nsimsun"},
            {u"\u9ED1\u4F53", "simhei"},
            {u"\u6977\u4F53", "simkai"},
            {u"\u4E2D\u6613\u5B8B\u4F53", "zycjksun"},
            {u"\u4E2D\u6613\u9ED1\u4F53", "zycjkhei"},
            {u"\u4E2D\u6613\u6977\u4F53", "zycjkkai"},
            {u"\u65B9\u6B63\u9ED1\u4F53", "fzhei"},
            {u"\u65B9\u6B63\u6977\u4F53", "fzkai"},
            {u"\u65B9\u6B63\u5B8B\u4E00", "fzsong"},
            {u"\u65B9\u6B63\u4E66\u5B8B", "fzshusong"},
            {u"\u65B9\u6B63\u4EFF\u5B8B", "fzfangsong"},
            // Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
            // First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
            {u"m\u7B80\u9ED1", "mhei"},
            {u"m\u6F60\u7AAA", "mhei"},
            {u"m\u7B80\u6977\u566C", "mkai"},
            {u"m\u6F60\u7FF1\u628E", "mkai"},
            {u"m\u7B80\u5B8B", "msong"},
            {u"m\u6F60\u51BC", "msong"},
            {u"m\u7B80\u592B\u5B8B", "cfangsong"},
            {u"m\u6F60\u6E98\u51BC", "cfangsong"},
            {u"\u7D30\u660E\u9AD4", "mingliu"},
            {u"\u65B0\u7D30\u660E\u9AD4", "pmingliu"},
            {u"\u6865", "hei"},
            {u"\u6B61", "kai"},
            {u"\u6D69\u6E67", "ming"},
            {u"ms\u30B4\u30B7\u30C3\u30AF", "msgothic"},
            {u"msp\u30B4\u30B7\u30C3\u30AF", "mspgothic"},
            {u"ms\u660E\u671D", "msmincho"},
            {u"msp\u660E\u671D", "mspmincho"},
            {u"\u5FAE\u8EDF\u6B63\u9ED1\u9AD4", "microsoftjhenghei"},
            {u"\u5FAE\u8F6F\u96C5\u9ED1", "microsoftyahei"},
            {u"\u30e1\u30a4\u30ea\u30aa", "meiryo"},
            {u"hg\u660E\u671Dl", "hgminchol"},
            {u"hg\u30B4\u30B7\u30C3\u30AFb", "hggothicb"},
            {u"hgp\u660E\u671Dl", "hgpminchol"},
            {u"hgp\u30B4\u30B7\u30C3\u30AFb", "hgpgothicb"},
            {u"hg\u660E\u671Dlsun", "hgmincholsun"},
            {u"hg\u30B4\u30B7\u30C3\u30AFbsun", "hggothicbsun"},
            {u"hgp\u660E\u671Dlsun", "hgpmincholsun"},
            {u"hgp\u30B4\u30B7\u30C3\u30AFbsun", "hgpgothicbsun"},
            {u"hg\u5E73\u6210\u660E\u671D\u4F53", "hgheiseimin"},
            {u"hg\u5E73\u6210\u660E\u671D\u4F53w3x12", "hgheiseimin"},
            {u"ipa\u660E\u671D", "ipamincho"},
            {u"ipap\u660E\u671D", "ipapmincho"},
            {u"ipa\u30B4\u30B7\u30C3\u30AF", "ipagothic"},
            {u"ipap\u30B4\u30B7\u30C3\u30AF", "ipapgothic"},
            {u"ipaui\u30B4\u30B7\u30C3\u30AF", "ipauigothic"},
            {u"takao\u660E\u671D", "takaomincho"},
            {u"takaop\u660E\u671D", "takaopmincho"},
            {u"takao\u30B4\u30B7\u30C3\u30AF", "takaogothic"},
            {u"takaop\u30B4\u30B7\u30C3\u30AF", "takaopgothic"},
            {u"\u3055\u3056\u306A\u307F\u660E\u671D", "sazanamimincho"},
            {u"\u3055\u3056\u306A\u307F\u30B4\u30B7\u30C3\u30AF", "sazanamigothic"},
            {u"\u6771\u98A8\u660E\u671D", "kochimincho"},
            {u"\u6771\u98A8\u30B4\u30B7\u30C3\u30AF", "kochigothic"},
            {u"\uC36C\uB3CB\uC6C0", "sundotum"},
            {u"\uC36C\uAD74\uB9BC", "sungulim"},
            {u"\uC36C\uBC14\uD0D5", "sunbatang"},
            {u"\uBC31\uBB35\uB3CB\uC6C0", "baekmukdotum"},
            {u"\uBC31\uBB35\uAD74\uB9BC", "baekmukgulim"},
            {u"\uBC31\uBB35\uBC14\uD0D5", "baekmukbatang"},
            {u"\u65B9\u6B63\u9ED1\u4F53", "fzheiti"},
            {u"\u65B9\u6B63\u9ED1\u9AD4", "fzheiti"},
            {u"\u65B9\u6B63\u6977\u4F53", "fzkaiti"},
            {u"\u65B9\u6B63\u6977\u9AD4", "fzkaitib"},
            {u"\u65B9\u6B63\u660E\u9AD4", "fzmingtib"},
            {u"\u65B9\u6B63\u5B8B\u4F53", "fzsongti"},
            {u"hy\uACAC\uBA85\uC870", "hymyeongjoextra"},
            {u"hy\uC2E0\uBA85\uC870", "hysinmyeongjomedium"},
            {u"hy\uC911\uACE0\uB515", "hygothicmedium"},
            {u"hy\uADF8\uB798\uD53Dm", "hygraphicmedium"},
            {u"hy\uADF8\uB798\uD53D", "hygraphic"},
            {u"\uC0C8\uAD74\uB9BC", "newgulim"},
            {u"\uC36C\uAD81\uC11C", "sungungseo"},
            {u"hy\uAD81\uC11Cb", "hygungsobold"},
            {u"hy\uAD81\uC11C", "hygungso"},
            {u"\uC36C\uD5E4\uB4DC\uB77C\uC778", "sunheadline"},
            {u"hy\uD5E4\uB4DC\uB77C\uC778m", "hyheadlinemedium"},
            {u"hy\uD5E4\uB4DC\uB77C\uC778", "hyheadline"},
            {u"\uD734\uBA3C\uC61B\uCCB4", "yetr"},
            {u"hy\uACAC\uACE0\uB515", "hygothicextra"},
            {u"\uC36C\uBAA9\uD310", "sunmokpan"},
            {u"\uC36C\uC5FD\uC11C", "sunyeopseo"},
            {u"\uC36C\uBC31\uC1A1", "sunbaeksong"},
            {u"hy\uC5FD\uC11Cl", "hypostlight"},
            {u"hy\uC5FD\uC11C", "hypost"},
            {u"\uD734\uBA3C\uB9E4\uC9C1\uCCB4", "magicr"},
            {u"\uC36C\uD06C\uB9AC\uC2A4\uD0C8", "suncrystal"},
            {u"\uC36C\uC0D8\uBB3C", "sunsaemmul"},
            {u"hy\uC595\uC740\uC0D8\uBB3Cm", "hyshortsamulmedium"},
            {u"hy\uC595\uC740\uC0D8\uBB3C", "hyshortsamul"},
            {u"\uD55C\uCEF4\uBC14\uD0D5", "haansoftbatang"},
            {u"\uD55C\uCEF4\uB3CB\uC6C0", "haansoftdotum"},
            {u"\uD55C\uC591\uD574\uC11C", "hyhaeseo"},
            {u"md\uC194\uCCB4", "mdsol"},
            {u"md\uAC1C\uC131\uCCB4", "mdgaesung"},
            {u"md\uC544\uD2B8\uCCB4", "mdart"},
            {u"md\uC544\uB871\uCCB4", "mdalong"},
            {u"md\uC774\uC19D\uCCB4", "mdeasop"},
            {u"hg\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e", "hggothice"},
            {u"hgp\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e", "hgpgothice"},
            {u"hgs\uFF7A\uFF9E\uFF7C\uFF6F\uFF78e", "hgsgothice"},
            {u"hg\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m", "hggothicm"},
            {u"hgp\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m", "hgpgothicm"},
            {u"hgs\uFF7A\uFF9E\uFF7C\uFF6F\uFF78m", "hgsgothicm"},
            {u"hg\u884C\u66F8\u4F53", "hggyoshotai"},
            {u"hgp\u884C\u66F8\u4F53", "hgpgyoshotai"},
            {u"hgs\u884C\u66F8\u4F53", "hgsgyoshotai"},
            {u"hg\u6559\u79D1\u66F8\u4F53", "hgkyokashotai"},
            {u"hgp\u6559\u79D1\u66F8\u4F53", "hgpkyokashotai"},
            {u"hgs\u6559\u79D1\u66F8\u4F53", "hgskyokashotai"},
            {u"hg\u660E\u671Db", "hgminchob"},
            {u"hgp\u660E\u671Db", "hgpminchob"},
            {u"hgs\u660E\u671Db", "hgsminchob"},
            {u"hg\u660E\u671De", "hgminchoe"},
            {u"hgp\u660E\u671De", "hgpminchoe"},
            {u"hgs\u660E\u671De", "hgsminchoe"},
            {u"hg\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53", "hgsoeikakupoptai"},
            {u"hgp\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53", "hgpsoeikakupopta"},
            {u"hgs\u5275\u82F1\u89D2\uFF8E\uFF9F\uFF6F\uFF8C\uFF9F\u4F53", "hgssoeikakupopta"},
            {u"hg\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb", "hgsoeipresenceeb"},
            {u"hgp\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb", "hgpsoeipresenceeb"},
            {u"hgs\u5275\u82F1\uFF8C\uFF9F\uFF9A\uFF7E\uFF9E\uFF9D\uFF7Deb", "hgssoeipresenceeb"},
            {u"hg\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub", "hgsoeikakugothicub"},
            {u"hgp\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub", "hgpsoeikakugothicub"},
            {u"hgs\u5275\u82F1\u89D2\uFF7A\uFF9E\uFF7C\uFF6F\uFF78ub", "hgssoeikakugothicub"},
            {u"hg\u6B63\u6977\u66F8\u4F53-pro", "hgseikaishotaipro"},
            {u"hg\u4E38\uFF7A\uFF9E\uFF7C\uFF6F\uFF78-pro", "hgmarugothicmpro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u660E\u671Dpro", "hiraginominchopro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u660E\u671Dpron", "hiraginominchopron"},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4\u30B7\u30C3\u30AF", "hiraginosans"},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4pro", "hiraginokakugothicpro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u89D2\u30B4pron", "hiraginokakugothicpron"},
            {u"\u30D2\u30E9\u30AE\u30CE\u4E38\u30B4pro", "hiraginomarugothicpro"},
            {u"\u30D2\u30E9\u30AE\u30CE\u4E38\u30B4pron", "hiraginomarugothicpron"},
            {u"\u6E38\u30B4\u30B7\u30C3\u30AF", "yugothic"},
            {u"\u6E38\u30B4\u30B7\u30C3\u30AF\u4F53", "yugothictai"},
            {u"\u6E38\u660E\u671D", "yumincho"},
            {u"\u6E38\u660E\u671D\u4F53", "yuminchotai"},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AF", "sourcehansans"},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AFjp", "sourcehansansjp"},
            {u"\u6E90\u30CE\u89D2\u30B4\u30B7\u30C3\u30AFhw", "sourcehansanshw"},
            {u"\u6E90\u30CE\u660E\u671D", "sourcehanserif"},
            {u"\u6E90\u30CE\u660E\u671Djp", "sourcehanserifjp"},
            {u"ipamj\u660E\u671D", "ipamjmincho"},
            {u"ipaex\u30B4\u30B7\u30C3\u30AF", "ipaexgothic"},
            {u"ipaex\u660E\u671D", "ipaexmimcho"}};

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
