/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <svtools/sampletext.hxx>
#include <vcl/font.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/fontcharmap.hxx>
#include <i18nutil/unicode.hxx>
#include <sal/log.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <vector>
#include <map>

// This should only be used when a commonly used font incorrectly declares its
// coverage. If you add a font here, please leave a note explaining the issue
// that caused it to be added
static UScriptCode lcl_getHardCodedScriptNameForFont (const OutputDevice &rDevice)
{
    const OUString &rName = rDevice.GetFont().GetFamilyName();

    if (rName == "GB18030 Bitmap")
    {
        // As of OSX 10.9, the system font "GB18030 Bitmap" incorrectly declares
        // that it only covers "Phoenician" when in fact it's a Chinese font.
        return USCRIPT_HAN;
    }
    else if (rName == "BiauKai")
    {
        // "BiauKai" makes crazy claims to cover BUGINESE, SUNDANESE, etc
        // but in fact it's a Traditional Chinese font.
        return USCRIPT_TRADITIONAL_HAN;
    }
    else if (rName == "GungSeo" || rName == "PCMyungjo" || rName == "PilGi")
    {
        // These have no OS/2 tables, but we know they are Korean fonts.
        return USCRIPT_KOREAN;
    }
    else if (rName == "Hei" || rName == "Kai")
    {
        // These have no OS/2 tables, but we know they are Chinese fonts.
        return USCRIPT_HAN;
    }
    else if (rName.startsWith("Bangla "))
    {
        // "Bangla Sangam MN" claims it supports MALAYALAM, but it doesn't
        // "Bangla MN" claims just DEVANAGARI and not an additional BENGALI
        return USCRIPT_BENGALI;
    }
    else if (rName.startsWith("Gurmukhi "))
    {
        // "Gurmukhi MN" claims it supports TAMIL, but it doesn't
        return USCRIPT_GURMUKHI;
    }
    else if (rName.startsWith("Kannada "))
    {
        // "Kannada MN" claims it supports TAMIL, but it doesn't
        return USCRIPT_KANNADA;
    }
    else if (rName.startsWith("Lao "))
    {
        // "Lao Sangam MN" claims it supports TAMIL, but it doesn't
        return USCRIPT_LAO;
    }
    else if (rName.startsWith("Malayalam "))
    {
        // "Malayalam MN" claims it supports TAMIL, but it doesn't
        return USCRIPT_MALAYALAM;
    }
    else if (rName.startsWith("Sinhala "))
    {
        // "Sinhala MN" claims it supports CYRILLIC
        return USCRIPT_SINHALA;
    }
    else if (rName.startsWith("Telugu "))
    {
        // "Telugu MN" claims it supports TAMIL, but it doesn't
        return USCRIPT_TELUGU;
    }
    else if (rName.startsWith("Myanmar "))
    {
        return USCRIPT_MYANMAR;
    }
    else if (rName == "InaiMathi")
    {
        // "InaiMathi" claims it supports GOTHIC and CJK_UNIFIED_IDEOGRAPHS as well as
        // TAMIL, but it doesn't
        return USCRIPT_TAMIL;
    }
    else if (rName == "Hannotate TC" || rName == "HanziPen TC" || rName == "Heiti TC" || rName == "Weibei TC")
    {
        // These fonts claim support for ARMENIAN and a bunch of other stuff they don't support
        return USCRIPT_TRADITIONAL_HAN;
    }
    else if (rName == "Hannotate SC" || rName == "HanziPen SC" || rName == "Heiti SC" || rName == "Weibei SC")
    {
        // These fonts claim support for ARMENIAN and a bunch of other stuff they don't support
        return USCRIPT_SIMPLIFIED_HAN;
    }
    return USCRIPT_INVALID_CODE;
}

bool isSymbolFont(const vcl::Font &rFont)
{
    return (rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL) ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Apple Color Emoji") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("cmsy10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("cmex10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("esint10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("feta26") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("jsMath-cmsy10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("jsMath-cmex10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("msam10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("msbm10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("wasy10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Denemo") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("GlyphBasic1") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("GlyphBasic2") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("GlyphBasic3") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("GlyphBasic4") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Letters Laughing") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("MusiQwik") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("MusiSync") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("stmary10") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Symbol") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Webdings") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Wingdings") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Wingdings 2") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Wingdings 3") ||
            rFont.GetFamilyName().equalsIgnoreAsciiCase("Bookshelf Symbol 7") ||
            rFont.GetFamilyName().startsWith("STIXIntegrals") ||
            rFont.GetFamilyName().startsWith("STIXNonUnicode") ||
            rFont.GetFamilyName().startsWith("STIXSize") ||
            rFont.GetFamilyName().startsWith("STIXVariants") ||
            IsOpenSymbol(rFont.GetFamilyName());
}

bool canRenderNameOfSelectedFont(OutputDevice const &rDevice)
{
    const vcl::Font &rFont = rDevice.GetFont();
    return !isSymbolFont(rFont) && ( -1 == rDevice.HasGlyphs(rFont, rFont.GetFamilyName()) );
}

OUString makeShortRepresentativeSymbolTextForSelectedFont(OutputDevice const &rDevice)
{
    if (rDevice.GetFont().GetFamilyName() == "Symbol")
    {
        static constexpr OUString aImplAppleSymbolText =
            u"\u03BC\u2202\u2211\u220F\u03C0\u222B\u03A9\u221A"_ustr;
        bool bHasSampleTextGlyphs
            = (-1 == rDevice.HasGlyphs(rDevice.GetFont(), aImplAppleSymbolText));
        //It's the Apple version
        if (bHasSampleTextGlyphs)
            return aImplAppleSymbolText;
        static constexpr OUStringLiteral aImplAdobeSymbolText =
            u"\uF06D\uF0B6\uF0E5\uF0D5\uF070\uF0F2\uF057\uF0D6";
        return aImplAdobeSymbolText;
    }

    const bool bOpenSymbol = IsOpenSymbol(rDevice.GetFont().GetFamilyName());

    if (!bOpenSymbol)
    {
        FontCharMapRef xFontCharMap;
        bool bHasCharMap = rDevice.GetFontCharMap(xFontCharMap);
        if( bHasCharMap )
        {
            // use some sample characters available in the font
            sal_Unicode aText[8];

            // start just above the PUA used by most symbol fonts
            sal_uInt32 cNewChar = 0xFF00;

            const int nMaxCount = SAL_N_ELEMENTS(aText) - 1;
            int nSkip = xFontCharMap->GetCharCount() / nMaxCount;
            if( nSkip > 10 )
                nSkip = 10;
            else if( nSkip <= 0 )
                nSkip = 1;
            for( int i = 0; i < nMaxCount; ++i )
            {
                sal_uInt32 cOldChar = cNewChar;
                for( int j = nSkip; --j >= 0; )
                    cNewChar = xFontCharMap->GetPrevChar( cNewChar );
                if( cOldChar == cNewChar )
                    break;
                aText[ i ] = static_cast<sal_Unicode>(cNewChar); // TODO: support UCS4 samples
                aText[ i+1 ] = 0;
            }

            return OUString(aText);
        }
    }

    static const sal_Unicode aImplSymbolFontText[] = {
        0xF021,0xF032,0xF043,0xF054,0xF065,0xF076,0xF0B7,0xF0C8,0};
    static const sal_Unicode aImplStarSymbolText[] = {
        0x2702,0x2708,0x270D,0xE033,0x2211,0x2288,0};
    const sal_Unicode* pText = bOpenSymbol ? aImplStarSymbolText : aImplSymbolFontText;
    OUString sSampleText(pText);
    bool bHasSampleTextGlyphs = (-1 == rDevice.HasGlyphs(rDevice.GetFont(), sSampleText));
    return bHasSampleTextGlyphs ? sSampleText : OUString();
}

//These ones are typically for use in the font dropdown box beside the
//fontname, so say things roughly like "Script/Alphabet/Name-Of-Major-Language"

//Here we don't always know the language of course, only the script that can be
//written with the font. Often that's one single language written in that
//script, or a handful of related languages where the name of the script is the
//same between languages, or the name in the major language is known by most
//readers of the minor languages, e.g. Yiddish is written with the HEBREW
//script as well, the vast majority of Yiddish readers will be able to read
//Hebrew as well.
OUString makeShortRepresentativeTextForScript(UScriptCode eScript)
{
    OUString sSampleText;
    switch (eScript)
    {
        case USCRIPT_GREEK:
        {
            static constexpr OUStringLiteral aGrek =
                u"\u0391\u03BB\u03C6\u03AC\u03B2\u03B7\u03C4\u03BF";
            sSampleText = aGrek;
            break;
        }
        case USCRIPT_HEBREW:
        {
            static constexpr OUStringLiteral aHebr =
                u"\u05D0\u05DC\u05E3\u05BE\u05D1\u05D9\u05EA "
                "\u05E2\u05D1\u05E8\u05D9";
            sSampleText = aHebr;
            break;
        }
        case USCRIPT_ARABIC:
        {
            static constexpr OUStringLiteral aArab =
                u"\u0623\u0628\u062C\u062F\u064A\u0629 \u0639"
                "\u0631\u0628\u064A\u0629";
            sSampleText = aArab;
            break;
        }
        case USCRIPT_ARMENIAN:
        {
            static constexpr OUStringLiteral aArmenian =
                u"\u0561\u0575\u0562\u0578\u0582\u0562\u0565"
                "\u0576";
            sSampleText = aArmenian;
            break;
        }
        case USCRIPT_DEVANAGARI:
        {
            static constexpr OUStringLiteral aDeva =
                u"\u0926\u0947\u0935\u0928\u093E\u0917\u0930\u0940";
            sSampleText = aDeva;
            break;
        }
        case USCRIPT_BENGALI:
        {
            static constexpr OUStringLiteral aBeng =
                u"\u09AC\u09BE\u0982\u09B2\u09BE \u09B2\u09BF"
                "\u09AA\u09BF";
            sSampleText = aBeng;
            break;
        }
        case USCRIPT_GURMUKHI:
        {
            static constexpr OUStringLiteral aGuru =
                u"\u0A17\u0A41\u0A30\u0A2E\u0A41\u0A16\u0A40";
            sSampleText = aGuru;
            break;
        }
        case USCRIPT_GUJARATI:
        {
            static constexpr OUStringLiteral aGujr =
                u"\u0A97\u0AC1\u0A9C\u0AB0\u0ABE\u0AA4\u0aC0 "
                "\u0AB2\u0ABF\u0AAA\u0ABF";
            sSampleText = aGujr;
            break;
        }
        case USCRIPT_ORIYA:
        {
            static constexpr OUStringLiteral aOrya =
                u"\u0B09\u0B24\u0B4D\u0B15\u0B33 \u0B32\u0B3F"
                "\u0B2A\u0B3F";
            sSampleText = aOrya;
            break;
        }
        case USCRIPT_TAMIL:
        {
            static constexpr OUStringLiteral aTaml =
                u"\u0B85\u0BB0\u0BBF\u0B9A\u0BCD\u0B9A\u0BC1\u0BB5"
                "\u0B9F\u0BBF";
            sSampleText = aTaml;
            break;
        }
        case USCRIPT_TELUGU:
        {
            static constexpr OUStringLiteral aTelu =
                u"\u0C24\u0C46\u0C32\u0C41\u0C17\u0C41";
            sSampleText = aTelu;
            break;
        }
        case USCRIPT_KANNADA:
        {
            static constexpr OUStringLiteral aKnda =
                u"\u0C95\u0CA8\u0CCD\u0CA8\u0CA1 \u0CB2\u0CBF"
                "\u0CAA\u0CBF";
            sSampleText = aKnda;
            break;
        }
        case USCRIPT_MALAYALAM:
        {
            static constexpr OUStringLiteral aMlym =
                u"\u0D2E\u0D32\u0D2F\u0D3E\u0D33\u0D32\u0D3F\u0D2A"
                "\u0D3F";
            sSampleText = aMlym;
            break;
        }
        case USCRIPT_THAI:
        {
            static constexpr OUStringLiteral aThai =
                u"\u0E2D\u0E31\u0E01\u0E29\u0E23\u0E44\u0E17\u0E22";
            sSampleText = aThai;
            break;
        }
        case USCRIPT_LAO:
        {
            static constexpr OUStringLiteral aLao =
                u"\u0EAD\u0EB1\u0E81\u0EAA\u0EAD\u0E99\u0EA5\u0EB2"
                "\u0EA7";
            sSampleText = aLao;
            break;
        }
        case USCRIPT_GEORGIAN:
        {
            static constexpr OUStringLiteral aGeorgian =
                u"\u10D3\u10D0\u10DB\u10EC\u10D4\u10E0\u10DA\u10DD"
                "\u10D1\u10D0";
            sSampleText = aGeorgian;
            break;
        }
        case USCRIPT_JAMO:
        case USCRIPT_HANGUL:
        case USCRIPT_KOREAN:
        {
            static constexpr OUStringLiteral aHang =
                u"\uD55C\uAE00";
            sSampleText = aHang;
            break;
        }
        case USCRIPT_TIBETAN:
        {
            static constexpr OUStringLiteral aTibt =
                u"\u0F51\u0F56\u0F74\u0F0B\u0F45\u0F53\u0F0B";
            sSampleText = aTibt;
            break;
        }
        case USCRIPT_SYRIAC:
        {
            static constexpr OUStringLiteral aSyri =
                u"\u0723\u071B\u072A\u0722\u0713\u0720\u0710";
            sSampleText = aSyri;
            break;
        }
        case USCRIPT_THAANA:
        {
            static constexpr OUStringLiteral aThaa =
                u"\u078C\u07A7\u0782\u07A6";
            sSampleText = aThaa;
            break;
        }
        case USCRIPT_SINHALA:
        {
            static constexpr OUStringLiteral aSinh =
                u"\u0DC1\u0DD4\u0DAF\u0DCA\u0DB0 \u0DC3\u0DD2"
                "\u0D82\u0DC4\u0DBD";
            sSampleText = aSinh;
            break;
        }
        case USCRIPT_MYANMAR:
        {
            static constexpr OUStringLiteral aMymr =
                u"\u1019\u103C\u1014\u103A\u1019\u102C\u1021\u1000"
                "\u1039\u1001\u101B\u102C";
            sSampleText = aMymr;
            break;
        }
        case USCRIPT_ETHIOPIC:
        {
            static constexpr OUStringLiteral aEthi =
                u"\u130D\u12D5\u12DD";
            sSampleText = aEthi;
            break;
        }
        case USCRIPT_CHEROKEE:
        {
            static constexpr OUStringLiteral aCher =
                u"\u13D7\u13AA\u13EA\u13B6\u13D9\u13D7";
            sSampleText = aCher;
            break;
        }
        case USCRIPT_KHMER:
        {
            static constexpr OUStringLiteral aKhmr =
                u"\u17A2\u1780\u17D2\u1781\u179A\u1780\u17D2\u179A"
                "\u1798\u1781\u17C1\u1798\u179A\u1797\u17B6\u179F"
                "\u17B6";
            sSampleText = aKhmr;
            break;
        }
        case USCRIPT_MONGOLIAN:
        {
            static constexpr OUStringLiteral aMongolian =
                u"\u182A\u1822\u1834\u1822\u182D\u180C";
            sSampleText = aMongolian;
            break;
        }
        case USCRIPT_TAGALOG:
        {
            static constexpr OUStringLiteral aTagalog =
                u"\u170A\u170A\u170C\u1712";
            sSampleText = aTagalog;
            break;
        }
        case USCRIPT_NEW_TAI_LUE:
        {
            static constexpr OUStringLiteral aTalu =
                u"\u1991\u19BA\u199F\u19B9\u19C9";
            sSampleText = aTalu;
            break;
        }
        case USCRIPT_TRADITIONAL_HAN:
        {
            static constexpr OUStringLiteral aHant =
                u"\u7E41";
            sSampleText = aHant;
            break;
        }
        case USCRIPT_SIMPLIFIED_HAN:
        {
            static constexpr OUStringLiteral aHans =
                u"\u7B80";
            sSampleText = aHans;
            break;
        }
        case USCRIPT_HAN:
        {
            static constexpr OUStringLiteral aSimplifiedAndTraditionalChinese =
                u"\u7B80\u7E41";
            sSampleText = aSimplifiedAndTraditionalChinese;
            break;
        }
        case USCRIPT_JAPANESE:
        {
            static constexpr OUStringLiteral aJpan =
                u"\u65E5\u672C\u8A9E";
            sSampleText = aJpan;
            break;
        }
        case USCRIPT_YI:
        {
            static constexpr OUStringLiteral aYiii =
                u"\uA188\uA320\uA071\uA0B7";
            sSampleText = aYiii;
            break;
        }
        case USCRIPT_PHAGS_PA:
        {
            static constexpr OUStringLiteral aPhag =
                u"\uA84F\uA861\uA843 \uA863\uA861\uA859 "
                u"\uA850\uA85C\uA85E";
            sSampleText = aPhag;
            break;
        }
        case USCRIPT_TAI_LE:
        {
            static constexpr OUStringLiteral aTale =
                u"\u1956\u196D\u1970\u1956\u196C\u1973\u1951\u1968"
                "\u1952\u1970";
            sSampleText = aTale;
            break;
        }
        case USCRIPT_LATIN:
            sSampleText = "Lorem ipsum";
            break;
        default:
            break;
    }
    return sSampleText;
}

static OUString makeRepresentativeTextForScript(UScriptCode eScript)
{
    OUString sSampleText;
    switch (eScript)
    {
        case USCRIPT_TRADITIONAL_HAN:
        case USCRIPT_SIMPLIFIED_HAN:
        case USCRIPT_HAN:
        {
            //Three Character Classic
            static constexpr OUStringLiteral aZh =
                u"\u4EBA\u4E4B\u521D \u6027\u672C\u5584";
            sSampleText = aZh;
            break;
        }
        case USCRIPT_JAPANESE:
        {
            //'Beautiful Japanese'
            static constexpr OUStringLiteral aJa =
                u"\u7F8E\u3057\u3044\u65E5\u672C\u8A9E";
            sSampleText = aJa;
            break;
        }
        case USCRIPT_JAMO:
        case USCRIPT_KOREAN:
        case USCRIPT_HANGUL:
        {
            //The essential condition for...
            static constexpr OUStringLiteral aKo =
                u"\uD0A4\uC2A4\uC758 \uACE0\uC720\uC870"
                "\uAC74\uC740";
            sSampleText = aKo;
            break;
        }
        default:
            break;
    }

    if (sSampleText.isEmpty())
        sSampleText = makeShortRepresentativeTextForScript(eScript);
    return sSampleText;
}

OUString makeShortMinimalTextForScript(UScriptCode eScript)
{
    OUString sSampleText;
    switch (eScript)
    {
        case USCRIPT_GREEK:
        {
            static constexpr OUStringLiteral aGrek =
                u"\u0391\u0392";
            sSampleText = aGrek;
            break;
        }
        case USCRIPT_HEBREW:
        {
            static constexpr OUStringLiteral aHebr =
                u"\u05D0\u05D1";
            sSampleText = aHebr;
            break;
        }
        default:
            break;
    }
    return sSampleText;
}

static OUString makeMinimalTextForScript(UScriptCode eScript)
{
    return makeShortMinimalTextForScript(eScript);
}

//These ones are typically for use in the font preview window in format
//character

//There we generally know the language. Though it's possible for the language to
//be "none".

//Currently we fall back to makeShortRepresentativeTextForScript when we don't
//have suitable strings
static OUString makeRepresentativeTextForLanguage(LanguageType eLang)
{
    OUString sRet;
    LanguageType pri = primary(eLang);
    if( pri == primary(LANGUAGE_ARMENIAN) )
        sRet = makeRepresentativeTextForScript(USCRIPT_ARMENIAN);
    else if( pri == primary(LANGUAGE_CHINESE) )
        sRet = makeRepresentativeTextForScript(USCRIPT_HAN);
    else if( pri == primary(LANGUAGE_GREEK) )
        sRet = makeRepresentativeTextForScript(USCRIPT_GREEK);
    else if( pri.anyOf(
                primary(LANGUAGE_HEBREW),
                primary(LANGUAGE_YIDDISH)) )
        sRet = makeRepresentativeTextForScript(USCRIPT_HEBREW);
    else if( pri == primary(LANGUAGE_ARABIC_SAUDI_ARABIA) )
        sRet = makeRepresentativeTextForScript(USCRIPT_ARABIC);
    else if( pri == primary(LANGUAGE_HINDI) )
        sRet = makeRepresentativeTextForScript(USCRIPT_DEVANAGARI);
    else if( pri == primary(LANGUAGE_ASSAMESE) )
    {
        static constexpr OUStringLiteral aAs =
            u"\u0985\u09B8\u09AE\u09C0\u09AF\u09BC\u09BE"
            " \u0986\u0996\u09F0";
        sRet = aAs;
    }
    else if( pri == primary(LANGUAGE_BENGALI) )
        sRet = makeRepresentativeTextForScript(USCRIPT_BENGALI);
    else if( pri == primary(LANGUAGE_PUNJABI) )
        sRet = makeRepresentativeTextForScript(USCRIPT_GURMUKHI);
    else if( pri == primary(LANGUAGE_GUJARATI) )
        sRet = makeRepresentativeTextForScript(USCRIPT_GUJARATI);
    else if( pri == primary(LANGUAGE_ODIA) )
        sRet = makeRepresentativeTextForScript(USCRIPT_ORIYA);
    else if( pri == primary(LANGUAGE_TAMIL) )
        sRet = makeRepresentativeTextForScript(USCRIPT_TAMIL);
    else if( pri == primary(LANGUAGE_TELUGU) )
        sRet = makeRepresentativeTextForScript(USCRIPT_TELUGU);
    else if( pri == primary(LANGUAGE_KANNADA) )
        sRet = makeRepresentativeTextForScript(USCRIPT_KANNADA);
    else if( pri == primary(LANGUAGE_MALAYALAM) )
        sRet = makeRepresentativeTextForScript(USCRIPT_MALAYALAM);
    else if( pri == primary(LANGUAGE_THAI) )
        sRet = makeRepresentativeTextForScript(USCRIPT_THAI);
    else if( pri == primary(LANGUAGE_LAO) )
        sRet = makeRepresentativeTextForScript(USCRIPT_LAO);
    else if( pri == primary(LANGUAGE_GEORGIAN) )
        sRet = makeRepresentativeTextForScript(USCRIPT_GEORGIAN);
    else if( pri == primary(LANGUAGE_KOREAN) )
        sRet = makeRepresentativeTextForScript(USCRIPT_KOREAN);
    else if( pri == primary(LANGUAGE_TIBETAN) )
        sRet = makeRepresentativeTextForScript(USCRIPT_TIBETAN);
    else if( pri == primary(LANGUAGE_SYRIAC) )
        sRet = makeRepresentativeTextForScript(USCRIPT_SYRIAC);
    else if( pri == primary(LANGUAGE_SINHALESE_SRI_LANKA) )
        sRet = makeRepresentativeTextForScript(USCRIPT_SINHALA);
    else if( pri == primary(LANGUAGE_BURMESE) )
        sRet = makeRepresentativeTextForScript(USCRIPT_MYANMAR);
    else if( pri == primary(LANGUAGE_AMHARIC_ETHIOPIA) )
        sRet = makeRepresentativeTextForScript(USCRIPT_ETHIOPIC);
    else if( pri == primary(LANGUAGE_CHEROKEE_UNITED_STATES) )
         sRet = makeRepresentativeTextForScript(USCRIPT_CHEROKEE);
    else if( pri == primary(LANGUAGE_KHMER) )
        sRet = makeRepresentativeTextForScript(USCRIPT_KHMER);
    else if( pri == primary(LANGUAGE_MONGOLIAN_MONGOLIAN_LSO) )
    {
        if (eLang.anyOf(
             LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA,
             LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA,
             LANGUAGE_MONGOLIAN_MONGOLIAN_LSO))
                sRet = makeRepresentativeTextForScript(USCRIPT_MONGOLIAN);
    }
    else if( pri == primary(LANGUAGE_JAPANESE) )
        sRet = makeRepresentativeTextForScript(USCRIPT_JAPANESE);
    else if( pri == primary(LANGUAGE_YI) )
        sRet = makeRepresentativeTextForScript(USCRIPT_YI);
    else if( pri == primary(LANGUAGE_GAELIC_IRELAND) )
    {
        static constexpr OUStringLiteral aGa =
            u"T\u00E9acs Samplach";
        sRet = aGa;
    }

    return sRet;
}

namespace
{
#if OSL_DEBUG_LEVEL > 0
    void lcl_dump_unicode_coverage(const std::optional<std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM>> &roIn)
    {
        if (!roIn)
        {
            SAL_INFO("svtools", "<NOTHING>");
            return;
        }
        auto & rIn(*roIn);
        if (rIn.none())
        {
            SAL_INFO("svtools", "<NONE>");
            return;
        }
        if (rIn[vcl::UnicodeCoverage::BASIC_LATIN])
            SAL_INFO("svtools", "BASIC_LATIN");
        if (rIn[vcl::UnicodeCoverage::LATIN_1_SUPPLEMENT])
            SAL_INFO("svtools", "LATIN_1_SUPPLEMENT");
        if (rIn[vcl::UnicodeCoverage::LATIN_EXTENDED_A])
            SAL_INFO("svtools", "LATIN_EXTENDED_A");
        if (rIn[vcl::UnicodeCoverage::LATIN_EXTENDED_B])
            SAL_INFO("svtools", "LATIN_EXTENDED_B");
        if (rIn[vcl::UnicodeCoverage::IPA_EXTENSIONS])
            SAL_INFO("svtools", "IPA_EXTENSIONS");
        if (rIn[vcl::UnicodeCoverage::SPACING_MODIFIER_LETTERS])
            SAL_INFO("svtools", "SPACING_MODIFIER_LETTERS");
        if (rIn[vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS])
            SAL_INFO("svtools", "COMBINING_DIACRITICAL_MARKS");
        if (rIn[vcl::UnicodeCoverage::GREEK_AND_COPTIC])
            SAL_INFO("svtools", "GREEK_AND_COPTIC");
        if (rIn[vcl::UnicodeCoverage::COPTIC])
            SAL_INFO("svtools", "COPTIC");
        if (rIn[vcl::UnicodeCoverage::CYRILLIC])
            SAL_INFO("svtools", "CYRILLIC");
        if (rIn[vcl::UnicodeCoverage::ARMENIAN])
            SAL_INFO("svtools", "ARMENIAN");
        if (rIn[vcl::UnicodeCoverage::HEBREW])
            SAL_INFO("svtools", "HEBREW");
        if (rIn[vcl::UnicodeCoverage::VAI])
            SAL_INFO("svtools", "VAI");
        if (rIn[vcl::UnicodeCoverage::ARABIC])
            SAL_INFO("svtools", "ARABIC");
        if (rIn[vcl::UnicodeCoverage::NKO])
            SAL_INFO("svtools", "NKO");
        if (rIn[vcl::UnicodeCoverage::DEVANAGARI])
            SAL_INFO("svtools", "DEVANAGARI");
        if (rIn[vcl::UnicodeCoverage::BENGALI])
            SAL_INFO("svtools", "BENGALI");
        if (rIn[vcl::UnicodeCoverage::GURMUKHI])
            SAL_INFO("svtools", "GURMUKHI");
        if (rIn[vcl::UnicodeCoverage::GUJARATI])
            SAL_INFO("svtools", "GUJARATI");
        if (rIn[vcl::UnicodeCoverage::ODIA])
            SAL_INFO("svtools", "ODIA");
        if (rIn[vcl::UnicodeCoverage::TAMIL])
            SAL_INFO("svtools", "TAMIL");
        if (rIn[vcl::UnicodeCoverage::TELUGU])
            SAL_INFO("svtools", "TELUGU");
        if (rIn[vcl::UnicodeCoverage::KANNADA])
            SAL_INFO("svtools", "KANNADA");
        if (rIn[vcl::UnicodeCoverage::MALAYALAM])
            SAL_INFO("svtools", "MALAYALAM");
        if (rIn[vcl::UnicodeCoverage::THAI])
            SAL_INFO("svtools", "THAI");
        if (rIn[vcl::UnicodeCoverage::LAO])
            SAL_INFO("svtools", "LAO");
        if (rIn[vcl::UnicodeCoverage::GEORGIAN])
            SAL_INFO("svtools", "GEORGIAN");
        if (rIn[vcl::UnicodeCoverage::BALINESE])
            SAL_INFO("svtools", "BALINESE");
        if (rIn[vcl::UnicodeCoverage::HANGUL_JAMO])
            SAL_INFO("svtools", "HANGUL_JAMO");
        if (rIn[vcl::UnicodeCoverage::LATIN_EXTENDED_ADDITIONAL])
            SAL_INFO("svtools", "LATIN_EXTENDED_ADDITIONAL");
        if (rIn[vcl::UnicodeCoverage::GREEK_EXTENDED])
            SAL_INFO("svtools", "GREEK_EXTENDED");
        if (rIn[vcl::UnicodeCoverage::GENERAL_PUNCTUATION])
            SAL_INFO("svtools", "GENERAL_PUNCTUATION");
        if (rIn[vcl::UnicodeCoverage::SUPERSCRIPTS_AND_SUBSCRIPTS])
            SAL_INFO("svtools", "SUPERSCRIPTS_AND_SUBSCRIPTS");
        if (rIn[vcl::UnicodeCoverage::CURRENCY_SYMBOLS])
            SAL_INFO("svtools", "CURRENCY_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS])
            SAL_INFO("svtools", "COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::LETTERLIKE_SYMBOLS])
            SAL_INFO("svtools", "LETTERLIKE_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::NUMBER_FORMS])
            SAL_INFO("svtools", "NUMBER_FORMS");
        if (rIn[vcl::UnicodeCoverage::ARROWS])
            SAL_INFO("svtools", "ARROWS");
        if (rIn[vcl::UnicodeCoverage::MATHEMATICAL_OPERATORS])
            SAL_INFO("svtools", "MATHEMATICAL_OPERATORS");
        if (rIn[vcl::UnicodeCoverage::MISCELLANEOUS_TECHNICAL])
            SAL_INFO("svtools", "MISCELLANEOUS_TECHNICAL");
        if (rIn[vcl::UnicodeCoverage::CONTROL_PICTURES])
            SAL_INFO("svtools", "CONTROL_PICTURES");
        if (rIn[vcl::UnicodeCoverage::OPTICAL_CHARACTER_RECOGNITION])
            SAL_INFO("svtools", "OPTICAL_CHARACTER_RECOGNITION");
        if (rIn[vcl::UnicodeCoverage::ENCLOSED_ALPHANUMERICS])
            SAL_INFO("svtools", "ENCLOSED_ALPHANUMERICS");
        if (rIn[vcl::UnicodeCoverage::BOX_DRAWING])
            SAL_INFO("svtools", "BOX_DRAWING");
        if (rIn[vcl::UnicodeCoverage::BLOCK_ELEMENTS])
            SAL_INFO("svtools", "BLOCK_ELEMENTS");
        if (rIn[vcl::UnicodeCoverage::GEOMETRIC_SHAPES])
            SAL_INFO("svtools", "GEOMETRIC_SHAPES");
        if (rIn[vcl::UnicodeCoverage::MISCELLANEOUS_SYMBOLS])
            SAL_INFO("svtools", "MISCELLANEOUS_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::DINGBATS])
            SAL_INFO("svtools", "DINGBATS");
        if (rIn[vcl::UnicodeCoverage::CJK_SYMBOLS_AND_PUNCTUATION])
            SAL_INFO("svtools", "CJK_SYMBOLS_AND_PUNCTUATION");
        if (rIn[vcl::UnicodeCoverage::HIRAGANA])
            SAL_INFO("svtools", "HIRAGANA");
        if (rIn[vcl::UnicodeCoverage::KATAKANA])
            SAL_INFO("svtools", "KATAKANA");
        if (rIn[vcl::UnicodeCoverage::BOPOMOFO])
            SAL_INFO("svtools", "BOPOMOFO");
        if (rIn[vcl::UnicodeCoverage::HANGUL_COMPATIBILITY_JAMO])
            SAL_INFO("svtools", "HANGUL_COMPATIBILITY_JAMO");
        if (rIn[vcl::UnicodeCoverage::PHAGS_PA])
            SAL_INFO("svtools", "PHAGS_PA");
        if (rIn[vcl::UnicodeCoverage::ENCLOSED_CJK_LETTERS_AND_MONTHS])
            SAL_INFO("svtools", "ENCLOSED_CJK_LETTERS_AND_MONTHS");
        if (rIn[vcl::UnicodeCoverage::CJK_COMPATIBILITY])
            SAL_INFO("svtools", "CJK_COMPATIBILITY");
        if (rIn[vcl::UnicodeCoverage::HANGUL_SYLLABLES])
            SAL_INFO("svtools", "HANGUL_SYLLABLES");
        if (rIn[vcl::UnicodeCoverage::NONPLANE_0])
            SAL_INFO("svtools", "NONPLANE_0");
        if (rIn[vcl::UnicodeCoverage::PHOENICIAN])
            SAL_INFO("svtools", "PHOENICIAN");
        if (rIn[vcl::UnicodeCoverage::CJK_UNIFIED_IDEOGRAPHS])
            SAL_INFO("svtools", "CJK_UNIFIED_IDEOGRAPHS");
        if (rIn[vcl::UnicodeCoverage::PRIVATE_USE_AREA_PLANE_0])
            SAL_INFO("svtools", "PRIVATE_USE_AREA_PLANE_0");
        if (rIn[vcl::UnicodeCoverage::CJK_STROKES])
            SAL_INFO("svtools", "CJK_STROKES");
        if (rIn[vcl::UnicodeCoverage::ALPHABETIC_PRESENTATION_FORMS])
            SAL_INFO("svtools", "ALPHABETIC_PRESENTATION_FORMS");
        if (rIn[vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_A])
            SAL_INFO("svtools", "ARABIC_PRESENTATION_FORMS_A");
        if (rIn[vcl::UnicodeCoverage::COMBINING_HALF_MARKS])
            SAL_INFO("svtools", "COMBINING_HALF_MARKS");
        if (rIn[vcl::UnicodeCoverage::VERTICAL_FORMS])
            SAL_INFO("svtools", "VERTICAL_FORMS");
        if (rIn[vcl::UnicodeCoverage::SMALL_FORM_VARIANTS])
            SAL_INFO("svtools", "SMALL_FORM_VARIANTS");
        if (rIn[vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_B])
            SAL_INFO("svtools", "ARABIC_PRESENTATION_FORMS_B");
        if (rIn[vcl::UnicodeCoverage::HALFWIDTH_AND_FULLWIDTH_FORMS])
            SAL_INFO("svtools", "HALFWIDTH_AND_FULLWIDTH_FORMS");
        if (rIn[vcl::UnicodeCoverage::SPECIALS])
            SAL_INFO("svtools", "SPECIALS");
        if (rIn[vcl::UnicodeCoverage::TIBETAN])
            SAL_INFO("svtools", "TIBETAN");
        if (rIn[vcl::UnicodeCoverage::SYRIAC])
            SAL_INFO("svtools", "SYRIAC");
        if (rIn[vcl::UnicodeCoverage::THAANA])
            SAL_INFO("svtools", "THAANA");
        if (rIn[vcl::UnicodeCoverage::SINHALA])
            SAL_INFO("svtools", "SINHALA");
        if (rIn[vcl::UnicodeCoverage::MYANMAR])
            SAL_INFO("svtools", "MYANMAR");
        if (rIn[vcl::UnicodeCoverage::ETHIOPIC])
            SAL_INFO("svtools", "ETHIOPIC");
        if (rIn[vcl::UnicodeCoverage::CHEROKEE])
            SAL_INFO("svtools", "CHEROKEE");
        if (rIn[vcl::UnicodeCoverage::UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS])
            SAL_INFO("svtools", "UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS");
        if (rIn[vcl::UnicodeCoverage::OGHAM])
            SAL_INFO("svtools", "OGHAM");
        if (rIn[vcl::UnicodeCoverage::RUNIC])
            SAL_INFO("svtools", "RUNIC");
        if (rIn[vcl::UnicodeCoverage::KHMER])
            SAL_INFO("svtools", "KHMER");
        if (rIn[vcl::UnicodeCoverage::MONGOLIAN])
            SAL_INFO("svtools", "MONGOLIAN");
        if (rIn[vcl::UnicodeCoverage::BRAILLE_PATTERNS])
            SAL_INFO("svtools", "BRAILLE_PATTERNS");
        if (rIn[vcl::UnicodeCoverage::YI_SYLLABLES])
            SAL_INFO("svtools", "YI_SYLLABLES");
        if (rIn[vcl::UnicodeCoverage::TAGALOG])
            SAL_INFO("svtools", "TAGALOG");
        if (rIn[vcl::UnicodeCoverage::OLD_ITALIC])
            SAL_INFO("svtools", "OLD_ITALIC");
        if (rIn[vcl::UnicodeCoverage::GOTHIC])
            SAL_INFO("svtools", "GOTHIC");
        if (rIn[vcl::UnicodeCoverage::DESERET])
            SAL_INFO("svtools", "DESERET");
        if (rIn[vcl::UnicodeCoverage::BYZANTINE_MUSICAL_SYMBOLS])
            SAL_INFO("svtools", "BYZANTINE_MUSICAL_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::MATHEMATICAL_ALPHANUMERIC_SYMBOLS])
            SAL_INFO("svtools", "MATHEMATICAL_ALPHANUMERIC_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::PRIVATE_USE_PLANE_15])
            SAL_INFO("svtools", "PRIVATE_USE_PLANE_15");
        if (rIn[vcl::UnicodeCoverage::VARIATION_SELECTORS])
            SAL_INFO("svtools", "VARIATION_SELECTORS");
        if (rIn[vcl::UnicodeCoverage::TAGS])
            SAL_INFO("svtools", "TAGS");
        if (rIn[vcl::UnicodeCoverage::LIMBU])
            SAL_INFO("svtools", "LIMBU");
        if (rIn[vcl::UnicodeCoverage::TAI_LE])
            SAL_INFO("svtools", "TAI_LE");
        if (rIn[vcl::UnicodeCoverage::NEW_TAI_LUE])
            SAL_INFO("svtools", "NEW_TAI_LUE");
        if (rIn[vcl::UnicodeCoverage::BUGINESE])
            SAL_INFO("svtools", "BUGINESE");
        if (rIn[vcl::UnicodeCoverage::GLAGOLITIC])
            SAL_INFO("svtools", "GLAGOLITIC");
        if (rIn[vcl::UnicodeCoverage::TIFINAGH])
            SAL_INFO("svtools", "TIFINAGH");
        if (rIn[vcl::UnicodeCoverage::YIJING_HEXAGRAM_SYMBOLS])
            SAL_INFO("svtools", "YIJING_HEXAGRAM_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::SYLOTI_NAGRI])
            SAL_INFO("svtools", "SYLOTI_NAGRI");
        if (rIn[vcl::UnicodeCoverage::LINEAR_B_SYLLABARY])
            SAL_INFO("svtools", "LINEAR_B_SYLLABARY");
        if (rIn[vcl::UnicodeCoverage::ANCIENT_GREEK_NUMBERS])
            SAL_INFO("svtools", "ANCIENT_GREEK_NUMBERS");
        if (rIn[vcl::UnicodeCoverage::UGARITIC])
            SAL_INFO("svtools", "UGARITIC");
        if (rIn[vcl::UnicodeCoverage::OLD_PERSIAN])
            SAL_INFO("svtools", "OLD_PERSIAN");
        if (rIn[vcl::UnicodeCoverage::SHAVIAN])
            SAL_INFO("svtools", "SHAVIAN");
        if (rIn[vcl::UnicodeCoverage::OSMANYA])
            SAL_INFO("svtools", "OSMANYA");
        if (rIn[vcl::UnicodeCoverage::CYPRIOT_SYLLABARY])
            SAL_INFO("svtools", "CYPRIOT_SYLLABARY");
        if (rIn[vcl::UnicodeCoverage::KHAROSHTHI])
            SAL_INFO("svtools", "KHAROSHTHI");
        if (rIn[vcl::UnicodeCoverage::TAI_XUAN_JING_SYMBOLS])
            SAL_INFO("svtools", "TAI_XUAN_JING_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::CUNEIFORM])
            SAL_INFO("svtools", "CUNEIFORM");
        if (rIn[vcl::UnicodeCoverage::COUNTING_ROD_NUMERALS])
            SAL_INFO("svtools", "COUNTING_ROD_NUMERALS");
        if (rIn[vcl::UnicodeCoverage::SUNDANESE])
            SAL_INFO("svtools", "SUNDANESE");
        if (rIn[vcl::UnicodeCoverage::LEPCHA])
            SAL_INFO("svtools", "LEPCHA");
        if (rIn[vcl::UnicodeCoverage::OL_CHIKI])
            SAL_INFO("svtools", "OL_CHIKI");
        if (rIn[vcl::UnicodeCoverage::SAURASHTRA])
            SAL_INFO("svtools", "SAURASHTRA");
        if (rIn[vcl::UnicodeCoverage::KAYAH_LI])
            SAL_INFO("svtools", "KAYAH_LI");
        if (rIn[vcl::UnicodeCoverage::REJANG])
            SAL_INFO("svtools", "REJANG");
        if (rIn[vcl::UnicodeCoverage::CHAM])
            SAL_INFO("svtools", "CHAM");
        if (rIn[vcl::UnicodeCoverage::ANCIENT_SYMBOLS])
            SAL_INFO("svtools", "ANCIENT_SYMBOLS");
        if (rIn[vcl::UnicodeCoverage::PHAISTOS_DISC])
            SAL_INFO("svtools", "PHAISTOS_DISC");
        if (rIn[vcl::UnicodeCoverage::CARIAN])
            SAL_INFO("svtools", "CARIAN");
        if (rIn[vcl::UnicodeCoverage::DOMINO_TILES])
            SAL_INFO("svtools", "DOMINO_TILES");
        if (rIn[vcl::UnicodeCoverage::RESERVED1])
            SAL_INFO("svtools", "RESERVED1");
        if (rIn[vcl::UnicodeCoverage::RESERVED2])
            SAL_INFO("svtools", "RESERVED2");
        if (rIn[vcl::UnicodeCoverage::RESERVED3])
            SAL_INFO("svtools", "RESERVED3");
        if (rIn[vcl::UnicodeCoverage::RESERVED4])
            SAL_INFO("svtools", "RESERVED4");
        if (!(rIn[vcl::UnicodeCoverage::RESERVED5]))
            return;

        SAL_INFO("svtools", "RESERVED5");
    }

    void lcl_dump_codepage_coverage(const std::optional<std::bitset<vcl::CodePageCoverage::MAX_CP_ENUM>> &roIn)
    {
        if (!roIn)
        {
            SAL_INFO("svtools", "<NOTHING>");
            return;
        }
        auto & rIn(*roIn);
        if (rIn.none())
        {
            SAL_INFO("svtools", "<NONE>");
            return;
        }
        if (rIn[vcl::CodePageCoverage::CP1252])
            SAL_INFO("svtools", "CP1252");
        if (rIn[vcl::CodePageCoverage::CP1250])
            SAL_INFO("svtools", "CP1250");
        if (rIn[vcl::CodePageCoverage::CP1251])
            SAL_INFO("svtools", "CP1251");
        if (rIn[vcl::CodePageCoverage::CP1253])
            SAL_INFO("svtools", "CP1253");
        if (rIn[vcl::CodePageCoverage::CP1254])
            SAL_INFO("svtools", "CP1254");
        if (rIn[vcl::CodePageCoverage::CP1255])
            SAL_INFO("svtools", "CP1255");
        if (rIn[vcl::CodePageCoverage::CP1256])
            SAL_INFO("svtools", "CP1256");
        if (rIn[vcl::CodePageCoverage::CP1257])
            SAL_INFO("svtools", "CP1257");
        if (rIn[vcl::CodePageCoverage::CP1258])
            SAL_INFO("svtools", "CP1258");
        if (rIn[vcl::CodePageCoverage::CP874])
            SAL_INFO("svtools", "CP874");
        if (rIn[vcl::CodePageCoverage::CP932])
            SAL_INFO("svtools", "CP932");
        if (rIn[vcl::CodePageCoverage::CP936])
            SAL_INFO("svtools", "CP936");
        if (rIn[vcl::CodePageCoverage::CP949])
            SAL_INFO("svtools", "CP949");
        if (rIn[vcl::CodePageCoverage::CP950])
            SAL_INFO("svtools", "CP950");
        if (rIn[vcl::CodePageCoverage::CP1361])
            SAL_INFO("svtools", "CP1361");
        if (rIn[vcl::CodePageCoverage::CP869])
            SAL_INFO("svtools", "CP869");
        if (rIn[vcl::CodePageCoverage::CP866])
            SAL_INFO("svtools", "CP866");
        if (rIn[vcl::CodePageCoverage::CP865])
            SAL_INFO("svtools", "CP865");
        if (rIn[vcl::CodePageCoverage::CP864])
            SAL_INFO("svtools", "CP864");
        if (rIn[vcl::CodePageCoverage::CP863])
            SAL_INFO("svtools", "CP863");
        if (rIn[vcl::CodePageCoverage::CP862])
            SAL_INFO("svtools", "CP862");
        if (rIn[vcl::CodePageCoverage::CP861])
            SAL_INFO("svtools", "CP861");
        if (rIn[vcl::CodePageCoverage::CP860])
            SAL_INFO("svtools", "CP860");
        if (rIn[vcl::CodePageCoverage::CP857])
            SAL_INFO("svtools", "CP857");
        if (rIn[vcl::CodePageCoverage::CP855])
            SAL_INFO("svtools", "CP855");
        if (rIn[vcl::CodePageCoverage::CP852])
            SAL_INFO("svtools", "CP852");
        if (rIn[vcl::CodePageCoverage::CP775])
            SAL_INFO("svtools", "CP775");
        if (rIn[vcl::CodePageCoverage::CP737])
            SAL_INFO("svtools", "CP737");
        if (rIn[vcl::CodePageCoverage::CP780])
            SAL_INFO("svtools", "CP780");
        if (rIn[vcl::CodePageCoverage::CP850])
            SAL_INFO("svtools", "CP850");
        if (!(rIn[vcl::CodePageCoverage::CP437]))
            return;

        SAL_INFO("svtools", "CP437");
    }
#endif

    std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> getMaskByScriptType(sal_Int16 nScriptType)
    {
        std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> aMask;
        aMask.set();

        for (size_t i = 0; i < vcl::UnicodeCoverage::MAX_UC_ENUM; ++i)
        {
            using vcl::UnicodeCoverage::UnicodeCoverageEnum;
            UScriptCode eScriptCode = otCoverageToScript(static_cast<UnicodeCoverageEnum>(i));
            if (unicode::getScriptClassFromUScriptCode(eScriptCode) == nScriptType)
                aMask.set(i, false);
        }

        return aMask;
    }

    //false for all bits considered "Latin" by LibreOffice
    std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> const & getLatinMask()
    {
        static std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> s_Mask(getMaskByScriptType(css::i18n::ScriptType::LATIN));
        return s_Mask;
    }

    //false for all bits considered "Asian" by LibreOffice
    std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> const & getCJKMask()
    {
        static std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> s_Mask(getMaskByScriptType(css::i18n::ScriptType::ASIAN));
        return s_Mask;
    }

    //false for all bits considered "Complex" by LibreOffice
    std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> const & getCTLMask()
    {
        static std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> s_Mask(getMaskByScriptType(css::i18n::ScriptType::COMPLEX));
        return s_Mask;
    }

    //false for all bits considered "WEAK" by LibreOffice
    std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> const & getWeakMask()
    {
        static std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> s_Mask(getMaskByScriptType(css::i18n::ScriptType::WEAK));
        return s_Mask;
    }

    //Nearly every font supports some basic Latin
    std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> getCommonLatnSubsetMask()
    {
        std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> aMask;
        aMask.set();
        aMask.set(vcl::UnicodeCoverage::BASIC_LATIN, false);
        aMask.set(vcl::UnicodeCoverage::LATIN_1_SUPPLEMENT, false);
        aMask.set(vcl::UnicodeCoverage::LATIN_EXTENDED_A, false);
        aMask.set(vcl::UnicodeCoverage::LATIN_EXTENDED_B, false);
        aMask.set(vcl::UnicodeCoverage::LATIN_EXTENDED_ADDITIONAL, false);
        return aMask;
    }

    template<size_t N>
    size_t find_first(std::bitset<N> const& rSet)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (rSet.test(i))
                return i;
        }
        assert(false); // see current usage
        return N;
    }

    UScriptCode getScript(const vcl::FontCapabilities &rFontCapabilities)
    {
        using vcl::UnicodeCoverage::UnicodeCoverageEnum;

        std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM> aMasked;
        if (rFontCapabilities.oUnicodeRange)
        {
            aMasked = *rFontCapabilities.oUnicodeRange & getWeakMask();
        }

        if (aMasked.count() == 1)
            return otCoverageToScript(static_cast<UnicodeCoverageEnum>(find_first(aMasked)));

        if (aMasked[vcl::UnicodeCoverage::ARABIC])
        {
            aMasked.set(vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_A, false);
            aMasked.set(vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_B, false);
            aMasked.set(vcl::UnicodeCoverage::NKO, false);
            //Probably strongly tuned for Arabic
            if (aMasked.count() == 1)
                return USCRIPT_ARABIC;
            if (aMasked.count() == 2 && aMasked[vcl::UnicodeCoverage::SYRIAC])
                return USCRIPT_SYRIAC;
        }

        if (aMasked[vcl::UnicodeCoverage::DEVANAGARI])
        {
            aMasked.set(vcl::UnicodeCoverage::DEVANAGARI, false);
            //Probably strongly tuned for a single Indic script
            if (aMasked.count() == 1)
                return otCoverageToScript(static_cast<UnicodeCoverageEnum>(find_first(aMasked)));
        }

        aMasked.set(vcl::UnicodeCoverage::GREEK_EXTENDED, false);
        aMasked.set(vcl::UnicodeCoverage::GREEK_AND_COPTIC, false);
        // tdf#88484
        // Some fonts set the Arabic Presentation Forms-B bit because they
        // support U+FEFF (Zero Width Space) which happens to be in that block
        // but it isn’t an Arabic code point. By the time we reach here we
        // decided this isn’t an Arabic font, so it should be safe.
        aMasked.set(vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_B, false);
        if (aMasked.count() == 1)
            return otCoverageToScript(static_cast<UnicodeCoverageEnum>(find_first(aMasked)));

        if (aMasked[vcl::UnicodeCoverage::CYRILLIC])
        {
            //Probably strongly tuned for Georgian
            if (aMasked.count() == 2 && aMasked[vcl::UnicodeCoverage::GEORGIAN])
                return USCRIPT_GEORGIAN;
        }

        aMasked &= getCJKMask();

        aMasked.set(vcl::UnicodeCoverage::CYRILLIC, false);
        aMasked.set(vcl::UnicodeCoverage::THAI, false);
        aMasked.set(vcl::UnicodeCoverage::DESERET, false);
        aMasked.set(vcl::UnicodeCoverage::PHAGS_PA, false);

        //So, possibly a CJK font
        if (!aMasked.count() && rFontCapabilities.oCodePageRange)
        {
            std::bitset<vcl::CodePageCoverage::MAX_CP_ENUM> aCJKCodePageMask;
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP932);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP936);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP949);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP950);
            aCJKCodePageMask.set(vcl::CodePageCoverage::CP1361);
            std::bitset<vcl::CodePageCoverage::MAX_CP_ENUM> aMaskedCodePage =
                *rFontCapabilities.oCodePageRange & aCJKCodePageMask;
            //fold Korean
            if (aMaskedCodePage[vcl::CodePageCoverage::CP1361])
            {
                aMaskedCodePage.set(vcl::CodePageCoverage::CP949);
                aMaskedCodePage.set(vcl::CodePageCoverage::CP1361, false);
            }

            if (aMaskedCodePage.count() == 1)
            {
                if (aMaskedCodePage[vcl::CodePageCoverage::CP932])
                    return USCRIPT_JAPANESE;
                if (aMaskedCodePage[vcl::CodePageCoverage::CP949])
                    return USCRIPT_KOREAN;
                if (aMaskedCodePage[vcl::CodePageCoverage::CP936])
                    return USCRIPT_SIMPLIFIED_HAN;
                if (aMaskedCodePage[vcl::CodePageCoverage::CP950])
                    return USCRIPT_TRADITIONAL_HAN;
            }

            if (aMaskedCodePage.count())
                return USCRIPT_HAN;
        }

        return USCRIPT_COMMON;
    }
}

const std::map<UScriptCode, std::vector<OUString>> distCjkMap =
{
    { USCRIPT_KOREAN,  { u" KR"_ustr, u"Korean"_ustr} },   // Korean
    { USCRIPT_JAPANESE, {u" JP"_ustr, u"Japanese"_ustr} } , // Japanese
    { USCRIPT_SIMPLIFIED_HAN, {u" SC"_ustr, u" GB"_ustr, u"S Chinese"_ustr} }, // Simplified Chinese Family
    { USCRIPT_TRADITIONAL_HAN, {u" TC"_ustr, u" HC"_ustr, u" TW"_ustr, u" HK"_ustr, u" MO"_ustr, u"T Chinese"_ustr} }// Traditional Chinese Family
};
namespace
{
    UScriptCode attemptToDisambiguateHan(UScriptCode eScript, OutputDevice const &rDevice)
    {
        //If we're a CJK font, see if we seem to be tuned for C, J or K
        if (eScript == USCRIPT_HAN)
        {
            const vcl::Font &rFont = rDevice.GetFont();

            bool bKore = false, bJpan = false, bHant = false, bHans = false;

            static constexpr OUStringLiteral sKorean = u"\u4E6D\u4E76\u596C";
            if (-1 == rDevice.HasGlyphs(rFont, sKorean))
                bKore = true;

            static constexpr OUStringLiteral sJapanese = u"\u5968\u67A0\u9D8F";
            if (-1 == rDevice.HasGlyphs(rFont, sJapanese))
                bJpan = true;

            static constexpr OUStringLiteral sTraditionalChinese = u"\u555F\u96DE";
            if (-1 == rDevice.HasGlyphs(rFont, sTraditionalChinese))
                bHant = true;

            static constexpr OUStringLiteral sSimplifiedChinese = u"\u4E61\u542F\u5956";
            if (-1 == rDevice.HasGlyphs(rFont, sSimplifiedChinese))
                bHans = true;

            if (bKore && !bJpan && !bHans && !bHant) {
                eScript = USCRIPT_KOREAN;
                return eScript;
            }
            else if (bJpan && !bKore && !bHans && !bHant) {
                eScript = USCRIPT_JAPANESE;
                return eScript;
            }
            else if (bHans && !bHant && !bKore && !bJpan) {
                eScript = USCRIPT_SIMPLIFIED_HAN;
                return eScript;
            }
            else if (bHant && !bHans && !bKore && !bJpan) {
                eScript = USCRIPT_TRADITIONAL_HAN;
                return eScript;
            }

            // for the last time, Check the ISO code strings or font specific strings
            const OUString &rName = rDevice.GetFont().GetFamilyName();
            std::map<UScriptCode, std::vector<OUString>>::const_iterator distCjkMapIt;
            for (distCjkMapIt = distCjkMap.begin(); distCjkMapIt != distCjkMap.end(); ++distCjkMapIt) {
                std::vector<OUString> cjkCodeList = distCjkMapIt->second;
                std::vector<OUString>::const_iterator cjkPtr;
                for (cjkPtr = cjkCodeList.begin(); cjkPtr != cjkCodeList.end(); ++cjkPtr) {
                    if (rName.indexOf(*cjkPtr) > 0) {
                        return distCjkMapIt->first;
                    }
                }
            }
            //otherwise fall-through as USCRIPT_HAN and expect a combined Hant/Hans preview
        }
        return eScript;
    }
}

OUString makeShortRepresentativeTextForSelectedFont(OutputDevice const &rDevice)
{
    UScriptCode eScript = lcl_getHardCodedScriptNameForFont(rDevice);
    if (eScript == USCRIPT_INVALID_CODE)
    {
        vcl::FontCapabilities aFontCapabilities;
        if (!rDevice.GetFontCapabilities(aFontCapabilities))
            return OUString();

#if OSL_DEBUG_LEVEL > 0
        lcl_dump_unicode_coverage(aFontCapabilities.oUnicodeRange);
        lcl_dump_codepage_coverage(aFontCapabilities.oCodePageRange);
#endif

        if (aFontCapabilities.oUnicodeRange)
            *aFontCapabilities.oUnicodeRange &= getCommonLatnSubsetMask();

        //If this font is probably tuned to display a single non-Latin
        //script and the font name is itself in Latin, then show a small
        //chunk of representative text for that script
        eScript = getScript(aFontCapabilities);
        if (eScript == USCRIPT_COMMON)
            return OUString();

        eScript = attemptToDisambiguateHan(eScript, rDevice);
    }

    OUString sSampleText = makeShortRepresentativeTextForScript(eScript);
    bool bHasSampleTextGlyphs = (-1 == rDevice.HasGlyphs(rDevice.GetFont(), sSampleText));
    return bHasSampleTextGlyphs ? sSampleText : OUString();
}

UScriptCode otCoverageToScript(vcl::UnicodeCoverage::UnicodeCoverageEnum eOTCoverage)
{
    UScriptCode eRet = USCRIPT_COMMON;
    switch (eOTCoverage)
    {
        case vcl::UnicodeCoverage::BASIC_LATIN:
        case vcl::UnicodeCoverage::LATIN_1_SUPPLEMENT:
        case vcl::UnicodeCoverage::LATIN_EXTENDED_A:
        case vcl::UnicodeCoverage::LATIN_EXTENDED_B:
            eRet = USCRIPT_LATIN;
            break;
        case vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS:
            eRet = USCRIPT_INHERITED;
            break;
        case vcl::UnicodeCoverage::GREEK_AND_COPTIC:
            eRet = USCRIPT_GREEK;
            break;
        case vcl::UnicodeCoverage::COPTIC:
            eRet = USCRIPT_COPTIC;
            break;
        case vcl::UnicodeCoverage::CYRILLIC:
            eRet = USCRIPT_CYRILLIC;
            break;
        case vcl::UnicodeCoverage::ARMENIAN:
            eRet = USCRIPT_ARMENIAN;
            break;
        case vcl::UnicodeCoverage::HEBREW:
            eRet = USCRIPT_HEBREW;
            break;
        case vcl::UnicodeCoverage::VAI:
            eRet = USCRIPT_VAI;
            break;
        case vcl::UnicodeCoverage::ARABIC:
            eRet = USCRIPT_ARABIC;
            break;
        case vcl::UnicodeCoverage::NKO:
            eRet = USCRIPT_NKO;
            break;
        case vcl::UnicodeCoverage::DEVANAGARI:
            eRet = USCRIPT_DEVANAGARI;
            break;
        case vcl::UnicodeCoverage::BENGALI:
            eRet = USCRIPT_BENGALI;
            break;
        case vcl::UnicodeCoverage::GURMUKHI:
            eRet = USCRIPT_GURMUKHI;
            break;
        case vcl::UnicodeCoverage::GUJARATI:
            eRet = USCRIPT_GUJARATI;
            break;
        case vcl::UnicodeCoverage::ODIA:
            eRet = USCRIPT_ORIYA;
            break;
        case vcl::UnicodeCoverage::TAMIL:
            eRet = USCRIPT_TAMIL;
            break;
        case vcl::UnicodeCoverage::TELUGU:
            eRet = USCRIPT_TELUGU;
            break;
        case vcl::UnicodeCoverage::KANNADA:
            eRet = USCRIPT_KANNADA;
            break;
        case vcl::UnicodeCoverage::MALAYALAM:
            eRet = USCRIPT_MALAYALAM;
            break;
        case vcl::UnicodeCoverage::THAI:
            eRet = USCRIPT_THAI;
            break;
        case vcl::UnicodeCoverage::LAO:
            eRet = USCRIPT_LAO;
            break;
        case vcl::UnicodeCoverage::GEORGIAN:
            eRet = USCRIPT_GEORGIAN;
            break;
        case vcl::UnicodeCoverage::BALINESE:
            eRet = USCRIPT_BALINESE;
            break;
        case vcl::UnicodeCoverage::HANGUL_JAMO:
            eRet = USCRIPT_HANGUL;
            break;
        case vcl::UnicodeCoverage::LATIN_EXTENDED_ADDITIONAL:
            eRet = USCRIPT_LATIN;
            break;
        case vcl::UnicodeCoverage::GREEK_EXTENDED:
            eRet = USCRIPT_GREEK;
            break;
        case vcl::UnicodeCoverage::CURRENCY_SYMBOLS:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS:
            eRet = USCRIPT_INHERITED;
            break;
        case vcl::UnicodeCoverage::LETTERLIKE_SYMBOLS:
        case vcl::UnicodeCoverage::NUMBER_FORMS:
        case vcl::UnicodeCoverage::ARROWS:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::MATHEMATICAL_OPERATORS:
            eRet = USCRIPT_MATHEMATICAL_NOTATION;
            break;
        case vcl::UnicodeCoverage::MISCELLANEOUS_TECHNICAL:
        case vcl::UnicodeCoverage::OPTICAL_CHARACTER_RECOGNITION:
        case vcl::UnicodeCoverage::BOX_DRAWING:
        case vcl::UnicodeCoverage::BLOCK_ELEMENTS:
        case vcl::UnicodeCoverage::GEOMETRIC_SHAPES:
        case vcl::UnicodeCoverage::MISCELLANEOUS_SYMBOLS:
        case vcl::UnicodeCoverage::DINGBATS:
        case vcl::UnicodeCoverage::CJK_SYMBOLS_AND_PUNCTUATION:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::HIRAGANA:
            eRet = USCRIPT_HIRAGANA;
            break;
        case vcl::UnicodeCoverage::KATAKANA:
            eRet = USCRIPT_KATAKANA;
            break;
        case vcl::UnicodeCoverage::BOPOMOFO:
            eRet = USCRIPT_BOPOMOFO;
            break;
        case vcl::UnicodeCoverage::HANGUL_COMPATIBILITY_JAMO:
            eRet = USCRIPT_HANGUL;
            break;
        case vcl::UnicodeCoverage::PHAGS_PA:
            eRet = USCRIPT_PHAGS_PA;
            break;
        case vcl::UnicodeCoverage::ENCLOSED_CJK_LETTERS_AND_MONTHS:
            eRet = USCRIPT_HANGUL;
            break;
        case vcl::UnicodeCoverage::CJK_COMPATIBILITY:
            eRet = USCRIPT_HAN;
            break;
        case vcl::UnicodeCoverage::HANGUL_SYLLABLES:
            eRet = USCRIPT_HANGUL;
            break;
        case vcl::UnicodeCoverage::PHOENICIAN:
            eRet = USCRIPT_PHOENICIAN;
            break;
        case vcl::UnicodeCoverage::CJK_UNIFIED_IDEOGRAPHS:
        case vcl::UnicodeCoverage::CJK_STROKES:
            eRet = USCRIPT_HAN;
            break;
        case vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_A:
            eRet = USCRIPT_ARABIC;
            break;
        case vcl::UnicodeCoverage::COMBINING_HALF_MARKS:
            eRet = USCRIPT_INHERITED;
            break;
        case vcl::UnicodeCoverage::ARABIC_PRESENTATION_FORMS_B:
            eRet = USCRIPT_ARABIC;
            break;
        case vcl::UnicodeCoverage::TIBETAN:
            eRet = USCRIPT_TIBETAN;
            break;
        case vcl::UnicodeCoverage::SYRIAC:
            eRet = USCRIPT_SYRIAC;
            break;
        case vcl::UnicodeCoverage::THAANA:
            eRet = USCRIPT_THAANA;
            break;
        case vcl::UnicodeCoverage::SINHALA:
            eRet = USCRIPT_SINHALA;
            break;
        case vcl::UnicodeCoverage::MYANMAR:
            eRet = USCRIPT_MYANMAR;
            break;
        case vcl::UnicodeCoverage::ETHIOPIC:
            eRet = USCRIPT_ETHIOPIC;
            break;
        case vcl::UnicodeCoverage::CHEROKEE:
            eRet = USCRIPT_CHEROKEE;
            break;
        case vcl::UnicodeCoverage::UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS:
            eRet = USCRIPT_CANADIAN_ABORIGINAL;
            break;
        case vcl::UnicodeCoverage::OGHAM:
            eRet = USCRIPT_OGHAM;
            break;
        case vcl::UnicodeCoverage::RUNIC:
            eRet = USCRIPT_RUNIC;
            break;
        case vcl::UnicodeCoverage::KHMER:
            eRet = USCRIPT_KHMER;
            break;
        case vcl::UnicodeCoverage::MONGOLIAN:
            eRet = USCRIPT_MONGOLIAN;
            break;
        case vcl::UnicodeCoverage::BRAILLE_PATTERNS:
            eRet = USCRIPT_BRAILLE;
            break;
        case vcl::UnicodeCoverage::YI_SYLLABLES:
            eRet = USCRIPT_YI;
            break;
        case vcl::UnicodeCoverage::TAGALOG:
            eRet = USCRIPT_TAGALOG;
            break;
        case vcl::UnicodeCoverage::OLD_ITALIC:
            eRet = USCRIPT_OLD_ITALIC;
            break;
        case vcl::UnicodeCoverage::GOTHIC:
            eRet = USCRIPT_GOTHIC;
            break;
        case vcl::UnicodeCoverage::DESERET:
            eRet = USCRIPT_DESERET;
            break;
        case vcl::UnicodeCoverage::BYZANTINE_MUSICAL_SYMBOLS:
        case vcl::UnicodeCoverage::MATHEMATICAL_ALPHANUMERIC_SYMBOLS:
        case vcl::UnicodeCoverage::PRIVATE_USE_PLANE_15:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::VARIATION_SELECTORS:
            eRet = USCRIPT_INHERITED;
            break;
        case vcl::UnicodeCoverage::TAGS:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::LIMBU:
            eRet = USCRIPT_LIMBU;
            break;
        case vcl::UnicodeCoverage::TAI_LE:
            eRet = USCRIPT_TAI_LE;
            break;
        case vcl::UnicodeCoverage::NEW_TAI_LUE:
            eRet = USCRIPT_NEW_TAI_LUE;
            break;
        case vcl::UnicodeCoverage::BUGINESE:
            eRet = USCRIPT_BUGINESE;
            break;
        case vcl::UnicodeCoverage::GLAGOLITIC:
            eRet = USCRIPT_GLAGOLITIC;
            break;
        case vcl::UnicodeCoverage::TIFINAGH:
            eRet = USCRIPT_TIFINAGH;
            break;
        case vcl::UnicodeCoverage::YIJING_HEXAGRAM_SYMBOLS:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::SYLOTI_NAGRI:
            eRet = USCRIPT_SYLOTI_NAGRI;
            break;
        case vcl::UnicodeCoverage::LINEAR_B_SYLLABARY:
            eRet = USCRIPT_LINEAR_B;
            break;
        case vcl::UnicodeCoverage::ANCIENT_GREEK_NUMBERS:
            eRet = USCRIPT_GREEK;
            break;
        case vcl::UnicodeCoverage::UGARITIC:
            eRet = USCRIPT_UGARITIC;
            break;
        case vcl::UnicodeCoverage::OLD_PERSIAN:
            eRet = USCRIPT_OLD_PERSIAN;
            break;
        case vcl::UnicodeCoverage::SHAVIAN:
            eRet = USCRIPT_SHAVIAN;
            break;
        case vcl::UnicodeCoverage::OSMANYA:
            eRet = USCRIPT_OSMANYA;
            break;
        case vcl::UnicodeCoverage::CYPRIOT_SYLLABARY:
            eRet = USCRIPT_CYPRIOT;
            break;
        case vcl::UnicodeCoverage::KHAROSHTHI:
            eRet = USCRIPT_KHAROSHTHI;
            break;
        case vcl::UnicodeCoverage::CUNEIFORM:
            eRet = USCRIPT_CUNEIFORM;
            break;
        case vcl::UnicodeCoverage::SUNDANESE:
            eRet = USCRIPT_SUNDANESE;
            break;
        case vcl::UnicodeCoverage::LEPCHA:
            eRet = USCRIPT_LEPCHA;
            break;
        case vcl::UnicodeCoverage::OL_CHIKI:
            eRet = USCRIPT_OL_CHIKI;
            break;
        case vcl::UnicodeCoverage::SAURASHTRA:
            eRet = USCRIPT_SAURASHTRA;
            break;
        case vcl::UnicodeCoverage::KAYAH_LI:
            eRet = USCRIPT_KAYAH_LI;
            break;
        case vcl::UnicodeCoverage::REJANG:
            eRet = USCRIPT_REJANG;
            break;
        case vcl::UnicodeCoverage::CHAM:
            eRet = USCRIPT_CHAM;
            break;
        case vcl::UnicodeCoverage::CARIAN:
            eRet = USCRIPT_CARIAN;
            break;
        case vcl::UnicodeCoverage::DOMINO_TILES:
        case vcl::UnicodeCoverage::TAI_XUAN_JING_SYMBOLS:
        case vcl::UnicodeCoverage::COUNTING_ROD_NUMERALS:
        case vcl::UnicodeCoverage::ANCIENT_SYMBOLS:
        case vcl::UnicodeCoverage::PHAISTOS_DISC:
            eRet = USCRIPT_SYMBOLS;
            break;
        case vcl::UnicodeCoverage::IPA_EXTENSIONS:
        case vcl::UnicodeCoverage::SPECIALS:
        case vcl::UnicodeCoverage::HALFWIDTH_AND_FULLWIDTH_FORMS:
        case vcl::UnicodeCoverage::VERTICAL_FORMS:
        case vcl::UnicodeCoverage::SMALL_FORM_VARIANTS:
        case vcl::UnicodeCoverage::ALPHABETIC_PRESENTATION_FORMS:
        case vcl::UnicodeCoverage::PRIVATE_USE_AREA_PLANE_0:
        case vcl::UnicodeCoverage::NONPLANE_0:
        case vcl::UnicodeCoverage::ENCLOSED_ALPHANUMERICS:
        case vcl::UnicodeCoverage::CONTROL_PICTURES:
        case vcl::UnicodeCoverage::SUPERSCRIPTS_AND_SUBSCRIPTS:
        case vcl::UnicodeCoverage::GENERAL_PUNCTUATION:
        case vcl::UnicodeCoverage::SPACING_MODIFIER_LETTERS:
        case vcl::UnicodeCoverage::RESERVED1:
        case vcl::UnicodeCoverage::RESERVED2:
        case vcl::UnicodeCoverage::RESERVED3:
        case vcl::UnicodeCoverage::RESERVED4:
        case vcl::UnicodeCoverage::RESERVED5:
        case vcl::UnicodeCoverage::MAX_UC_ENUM:
            break;
    }
    return eRet;
}

OUString makeRepresentativeTextForFont(sal_Int16 nScriptType, const vcl::Font &rFont)
{
    OUString sRet(makeRepresentativeTextForLanguage(rFont.GetLanguage()));

    ScopedVclPtrInstance< VirtualDevice > aDevice;
    if (sRet.isEmpty() || (-1 != aDevice->HasGlyphs(rFont, sRet)))
    {
        aDevice->SetFont(rFont);
        vcl::FontCapabilities aFontCapabilities;
        if (aDevice->GetFontCapabilities(aFontCapabilities))
        {
#if OSL_DEBUG_LEVEL > 0
            lcl_dump_unicode_coverage(aFontCapabilities.oUnicodeRange);
#endif

            if (aFontCapabilities.oUnicodeRange)
            {
                *aFontCapabilities.oUnicodeRange &= getWeakMask();

                if (nScriptType != css::i18n::ScriptType::ASIAN)
                {
                    *aFontCapabilities.oUnicodeRange &= getCJKMask();
                    aFontCapabilities.oCodePageRange.reset();
                }
                if (nScriptType != css::i18n::ScriptType::LATIN)
                    *aFontCapabilities.oUnicodeRange &= getLatinMask();
                if (nScriptType != css::i18n::ScriptType::COMPLEX)
                    *aFontCapabilities.oUnicodeRange &= getCTLMask();
            }

#if OSL_DEBUG_LEVEL > 0
            SAL_INFO("svtools", "minimal");
            lcl_dump_unicode_coverage(aFontCapabilities.oUnicodeRange);
            lcl_dump_codepage_coverage(aFontCapabilities.oCodePageRange);
#endif

            UScriptCode eScript = getScript(aFontCapabilities);

            if (nScriptType == css::i18n::ScriptType::ASIAN)
                eScript = attemptToDisambiguateHan(eScript, *aDevice);

            sRet = makeRepresentativeTextForScript(eScript);
        }

        if (sRet.isEmpty())
        {
            if (nScriptType == css::i18n::ScriptType::COMPLEX)
            {
                sRet = makeRepresentativeTextForScript(USCRIPT_HEBREW);
                if (-1 != aDevice->HasGlyphs(rFont, sRet))
                {
                    sRet = makeMinimalTextForScript(USCRIPT_HEBREW);
                    if (-1 != aDevice->HasGlyphs(rFont, sRet))
                        sRet = makeRepresentativeTextForScript(USCRIPT_ARABIC);
                }
            }
            else if (nScriptType == css::i18n::ScriptType::LATIN)
                sRet = makeRepresentativeTextForScript(USCRIPT_LATIN);
        }
    }

    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
