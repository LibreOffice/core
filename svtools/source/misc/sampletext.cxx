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
#include <vcl/metric.hxx>
#include <vcl/fontcharmap.hxx>
#include <i18nutil/unicode.hxx>
#include <sal/log.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

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
        // These fonts claim support for ARMENIAN and a bunch of other stuff they doesn't support
        return USCRIPT_TRADITIONAL_HAN;
    }
    else if (rName == "Hannotate SC" || rName == "HanziPen SC" || rName == "Heiti SC" || rName == "Weibei SC")
    {
        // These fonts claim support for ARMENIAN and a bunch of other stuff they doesn't support
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
            IsStarSymbol(rFont.GetFamilyName());
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
        static const sal_Unicode aImplAppleSymbolText[] = {
            0x03BC, 0x2202, 0x2211, 0x220F, 0x03C0, 0x222B, 0x03A9, 0x221A, 0};
        OUString sSampleText(aImplAppleSymbolText);
        bool bHasSampleTextGlyphs = (-1 == rDevice.HasGlyphs(rDevice.GetFont(), sSampleText));
        //It's the Apple version
        if (bHasSampleTextGlyphs)
            return OUString(aImplAppleSymbolText);
        static const sal_Unicode aImplAdobeSymbolText[] = {
            0xF06D, 0xF0B6, 0xF0E5, 0xF0D5, 0xF070, 0xF0F2, 0xF057, 0xF0D6, 0};
        return OUString(aImplAdobeSymbolText);
    }

    const bool bOpenSymbol = IsStarSymbol(rDevice.GetFont().GetFamilyName());

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
            static const sal_Unicode aGrek[] = {
                0x0391, 0x03BB, 0x03C6, 0x03AC, 0x03B2, 0x03B7, 0x03C4, 0x03BF
            };
            sSampleText = OUString(aGrek, SAL_N_ELEMENTS(aGrek));
            break;
        }
        case USCRIPT_HEBREW:
        {
            static const sal_Unicode aHebr[] = {
                0x05D0, 0x05B8, 0x05DC, 0x05B6, 0x05E3, 0x05BE, 0x05D1, 0x05B5,
                0x05BC, 0x05D9, 0x05EA, 0x0020, 0x05E2, 0x05B4, 0x05D1, 0x05B0,
                0x05E8, 0x05B4, 0x05D9
            };
            sSampleText = OUString(aHebr, SAL_N_ELEMENTS(aHebr));
            break;
        }
        case USCRIPT_ARABIC:
        {
            static const sal_Unicode aArab[] = {
                0x0623, 0x0628, 0x062C, 0x062F, 0x064A, 0x0629, 0x0020, 0x0639,
                0x0631, 0x0628, 0x064A, 0x0629
            };
            sSampleText = OUString(aArab, SAL_N_ELEMENTS(aArab));
            break;
        }
        case USCRIPT_ARMENIAN:
        {
            static const sal_Unicode aArmenian[] = {
                0x0561, 0x0575, 0x0562, 0x0578, 0x0582, 0x0562, 0x0565,
                0x0576
            };
            sSampleText = OUString(aArmenian, SAL_N_ELEMENTS(aArmenian));
            break;
        }
        case USCRIPT_DEVANAGARI:
        {
            static const sal_Unicode aDeva[] = {
                0x0926, 0x0947, 0x0935, 0x0928, 0x093E, 0x0917, 0x0930, 0x0940
            };
            sSampleText = OUString(aDeva, SAL_N_ELEMENTS(aDeva));
            break;
        }
        case USCRIPT_BENGALI:
        {
            static const sal_Unicode aBeng[] = {
                0x09AC, 0x09BE, 0x0982, 0x09B2, 0x09BE, 0x0020, 0x09B2, 0x09BF,
                0x09AA, 0x09BF
            };
            sSampleText = OUString(aBeng, SAL_N_ELEMENTS(aBeng));
            break;
        }
        case USCRIPT_GURMUKHI:
        {
            static const sal_Unicode aGuru[] = {
                0x0A17, 0x0A41, 0x0A30, 0x0A2E, 0x0A41, 0x0A16, 0x0A40
            };
            sSampleText = OUString(aGuru, SAL_N_ELEMENTS(aGuru));
            break;
        }
        case USCRIPT_GUJARATI:
        {
            static const sal_Unicode aGujr[] = {
                0x0A97, 0x0AC1, 0x0A9C, 0x0AB0, 0x0ABE, 0x0AA4, 0x0aC0, 0x0020,
                0x0AB2, 0x0ABF, 0x0AAA, 0x0ABF
            };
            sSampleText = OUString(aGujr, SAL_N_ELEMENTS(aGujr));
            break;
        }
        case USCRIPT_ORIYA:
        {
            static const sal_Unicode aOrya[] = {
                0x0B09, 0x0B24, 0x0B4D, 0x0B15, 0x0B33, 0x0020, 0x0B32, 0x0B3F,
                0x0B2A, 0x0B3F
            };
            sSampleText = OUString(aOrya, SAL_N_ELEMENTS(aOrya));
            break;
        }
        case USCRIPT_TAMIL:
        {
            static const sal_Unicode aTaml[] = {
                0x0B85, 0x0BB0, 0x0BBF, 0x0B9A, 0x0BCD, 0x0B9A, 0x0BC1, 0x0BB5,
                0x0B9F, 0x0BBF
            };
            sSampleText = OUString(aTaml, SAL_N_ELEMENTS(aTaml));
            break;
        }
        case USCRIPT_TELUGU:
        {
            static const sal_Unicode aTelu[] = {
                0x0C24, 0x0C46, 0x0C32, 0x0C41, 0x0C17, 0x0C41
            };
            sSampleText = OUString(aTelu, SAL_N_ELEMENTS(aTelu));
            break;
        }
        case USCRIPT_KANNADA:
        {
            static const sal_Unicode aKnda[] = {
                0x0C95, 0x0CA8, 0x0CCD, 0x0CA8, 0x0CA1, 0x0020, 0x0CB2, 0x0CBF,
                0x0CAA, 0x0CBF
            };
            sSampleText = OUString(aKnda, SAL_N_ELEMENTS(aKnda));
            break;
        }
        case USCRIPT_MALAYALAM:
        {
            static const sal_Unicode aMlym[] = {
                0x0D2E, 0x0D32, 0x0D2F, 0x0D3E, 0x0D33, 0x0D32, 0x0D3F, 0x0D2A,
                0x0D3F
            };
            sSampleText = OUString(aMlym, SAL_N_ELEMENTS(aMlym));
            break;
        }
        case USCRIPT_THAI:
        {
            static const sal_Unicode aThai[] = {
                0x0E2D, 0x0E31, 0x0E01, 0x0E29, 0x0E23, 0x0E44, 0x0E17, 0x0E22
            };
            sSampleText = OUString(aThai, SAL_N_ELEMENTS(aThai));
            break;
        }
        case USCRIPT_LAO:
        {
            static const sal_Unicode aLao[] = {
                0x0EAD, 0x0EB1, 0x0E81, 0x0EAA, 0x0EAD, 0x0E99, 0x0EA5, 0x0EB2,
                0x0EA7
            };
            sSampleText = OUString(aLao, SAL_N_ELEMENTS(aLao));
            break;
        }
        case USCRIPT_GEORGIAN:
        {
            static const sal_Unicode aGeorgian[] = {
                0x10D3, 0x10D0, 0x10DB, 0x10EC, 0x10D4, 0x10E0, 0x10DA, 0x10DD,
                0x10D1, 0x10D0
            };
            sSampleText = OUString(aGeorgian, SAL_N_ELEMENTS(aGeorgian));
            break;
        }
        case USCRIPT_HANGUL:
        case USCRIPT_KOREAN:
        {
            static const sal_Unicode aHang[] = {
                0xD55C, 0xAE00
            };
            sSampleText = OUString(aHang, SAL_N_ELEMENTS(aHang));
            break;
        }
        case USCRIPT_TIBETAN:
        {
            static const sal_Unicode aTibt[] = {
                0x0F51, 0x0F56, 0x0F74, 0x0F0B, 0x0F45, 0x0F53, 0x0F0B
            };
            sSampleText = OUString(aTibt, SAL_N_ELEMENTS(aTibt));
            break;
        }
        case USCRIPT_SYRIAC:
        {
            static const sal_Unicode aSyri[] = {
                0x0723, 0x071B, 0x072A, 0x0722, 0x0713, 0x0720, 0x0710
            };
            sSampleText = OUString(aSyri, SAL_N_ELEMENTS(aSyri));
            break;
        }
        case USCRIPT_THAANA:
        {
            static const sal_Unicode aThaa[] = {
                0x078C, 0x07A7, 0x0782, 0x07A6
            };
            sSampleText = OUString(aThaa, SAL_N_ELEMENTS(aThaa));
            break;
        }
        case USCRIPT_SINHALA:
        {
            static const sal_Unicode aSinh[] = {
                0x0DC1, 0x0DD4, 0x0DAF, 0x0DCA, 0x0DB0, 0x0020, 0x0DC3, 0x0DD2,
                0x0D82, 0x0DC4, 0x0DBD
            };
            sSampleText = OUString(aSinh, SAL_N_ELEMENTS(aSinh));
            break;
        }
        case USCRIPT_MYANMAR:
        {
            static const sal_Unicode aMymr[] = {
                0x1019, 0x103C, 0x1014, 0x103A, 0x1019, 0x102C, 0x1021, 0x1000,
                0x1039, 0x1001, 0x101B, 0x102C
            };
            sSampleText = OUString(aMymr, SAL_N_ELEMENTS(aMymr));
            break;
        }
        case USCRIPT_ETHIOPIC:
        {
            static const sal_Unicode aEthi[] = {
                0x130D, 0x12D5, 0x12DD
            };
            sSampleText = OUString(aEthi, SAL_N_ELEMENTS(aEthi));
            break;
        }
        case USCRIPT_CHEROKEE:
        {
            static const sal_Unicode aCher[] = {
                0x13D7, 0x13AA, 0x13EA, 0x13B6, 0x13D9, 0x13D7
            };
            sSampleText = OUString(aCher, SAL_N_ELEMENTS(aCher));
            break;
        }
        case USCRIPT_KHMER:
        {
            static const sal_Unicode aKhmr[] = {
                0x17A2, 0x1780, 0x17D2, 0x1781, 0x179A, 0x1780, 0x17D2, 0x179A,
                0x1798, 0x1781, 0x17C1, 0x1798, 0x179A, 0x1797, 0x17B6, 0x179F,
                0x17B6
            };
            sSampleText = OUString(aKhmr, SAL_N_ELEMENTS(aKhmr));
            break;
        }
        case USCRIPT_MONGOLIAN:
        {
            static const sal_Unicode aMongolian[] = {
                0x182A, 0x1822, 0x1834, 0x1822, 0x182D, 0x180C
            };
            sSampleText = OUString(aMongolian, SAL_N_ELEMENTS(aMongolian));
            break;
        }
        case USCRIPT_TAGALOG:
        {
            static const sal_Unicode aTagalog[] = {
                0x170A, 0x170A, 0x170C, 0x1712
            };
            sSampleText = OUString(aTagalog, SAL_N_ELEMENTS(aTagalog));
            break;
        }
        case USCRIPT_NEW_TAI_LUE:
        {
            static const sal_Unicode aTalu[] = {
                0x1991, 0x19BA, 0x199F, 0x19B9, 0x19C9
            };
            sSampleText = OUString(aTalu, SAL_N_ELEMENTS(aTalu));
            break;
        }
        case USCRIPT_TRADITIONAL_HAN:
        {
            static const sal_Unicode aHant[] = {
                0x7E41
            };
            sSampleText = OUString(aHant, SAL_N_ELEMENTS(aHant));
            break;
        }
        case USCRIPT_SIMPLIFIED_HAN:
        {
            static const sal_Unicode aHans[] = {
                0x7B80
            };
            sSampleText = OUString(aHans, SAL_N_ELEMENTS(aHans));
            break;
        }
        case USCRIPT_HAN:
        {
            static const sal_Unicode aSimplifiedAndTraditionalChinese[] = {
                0x7B80, 0x7E41
            };
            sSampleText = OUString(aSimplifiedAndTraditionalChinese,
                SAL_N_ELEMENTS(aSimplifiedAndTraditionalChinese));
            break;
        }
        case USCRIPT_JAPANESE:
        {
            static const sal_Unicode aJpan[] = {
                0x65E5, 0x672C, 0x8A9E
            };
            sSampleText = OUString(aJpan, SAL_N_ELEMENTS(aJpan));
            break;
        }
        case USCRIPT_YI:
        {
            static const sal_Unicode aYiii[] = {
                0xA188,  0xA320, 0xA071, 0xA0B7
            };
            sSampleText = OUString(aYiii, SAL_N_ELEMENTS(aYiii));
            break;
        }
        case USCRIPT_PHAGS_PA:
        {
            static const sal_Unicode aPhag[] = {
                0xA84F, 0xA861, 0xA843, 0x0020, 0xA863, 0xA861, 0xA859, 0x0020,
                0xA850, 0xA85C, 0xA85E
            };
            sSampleText = OUString(aPhag, SAL_N_ELEMENTS(aPhag));
            break;
        }
        case USCRIPT_TAI_LE:
        {
            static const sal_Unicode aTale[] = {
                0x1956, 0x196D, 0x1970, 0x1956, 0x196C, 0x1973, 0x1951, 0x1968,
                0x1952, 0x1970
            };
            sSampleText = OUString(aTale, SAL_N_ELEMENTS(aTale));
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

OUString makeRepresentativeTextForScript(UScriptCode eScript)
{
    OUString sSampleText;
    switch (eScript)
    {
        case USCRIPT_TRADITIONAL_HAN:
        case USCRIPT_SIMPLIFIED_HAN:
        case USCRIPT_HAN:
        {
            //Three Character Classic
            static const sal_Unicode aZh[] = {
                0x4EBA, 0x4E4B, 0x521D, 0x0020, 0x6027, 0x672C, 0x5584
            };
            sSampleText = OUString(aZh, SAL_N_ELEMENTS(aZh));
            break;
        }
        case USCRIPT_JAPANESE:
        {
            //'Beautiful Japanese'
            static const sal_Unicode aJa[] = {
                0x7F8E, 0x3057, 0x3044, 0x65E5, 0x672C, 0x8A9E
            };
            sSampleText = OUString(aJa, SAL_N_ELEMENTS(aJa));
            break;
        }
        case USCRIPT_KOREAN:
        case USCRIPT_HANGUL:
        {
            //The essential condition for...
            static const sal_Unicode aKo[] = {
                0xD0A4, 0xC2A4, 0xC758, 0x0020, 0xACE0, 0xC720, 0xC870,
                0xAC74, 0xC740
            };
            sSampleText = OUString(aKo, SAL_N_ELEMENTS(aKo));
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
            static const sal_Unicode aGrek[] = {
                0x0391, 0x0392
            };
            sSampleText = OUString(aGrek, SAL_N_ELEMENTS(aGrek));
            break;
        }
        case USCRIPT_HEBREW:
        {
            static const sal_Unicode aHebr[] = {
                0x05D0, 0x05D1
            };
            sSampleText = OUString(aHebr, SAL_N_ELEMENTS(aHebr));
            break;
        }
        default:
            break;
    }
    return sSampleText;
}

OUString makeMinimalTextForScript(UScriptCode eScript)
{
    return makeShortMinimalTextForScript(eScript);
}

//These ones are typically for use in the font preview window in format
//character

//There we generally know the language. Though its possible for the language to
//be "none".

//Currently we fall back to makeShortRepresentativeTextForScript when we don't
//have suitable strings
OUString makeRepresentativeTextForLanguage(LanguageType eLang)
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
        static const sal_Unicode aAs[] = {
            0x0985, 0x09B8, 0x09AE, 0x09C0, 0x09AF, 0x09BC, 0x09BE,
            0x0020, 0x0986, 0x0996, 0x09F0
        };
        sRet = OUString(aAs, SAL_N_ELEMENTS(aAs));
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
        static const sal_Unicode aGa[] = {
            'T', 0x00E9, 'a', 'c', 's', ' ', 'S', 'a', 'm', 'p', 'l', 'a', 'c', 'h'
        };
        sRet = OUString(aGa, SAL_N_ELEMENTS(aGa));
    }

    return sRet;
}

namespace
{
#if OSL_DEBUG_LEVEL > 0
    void lcl_dump_unicode_coverage(const boost::optional<std::bitset<vcl::UnicodeCoverage::MAX_UC_ENUM>> &roIn)
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
        if (rIn[vcl::UnicodeCoverage::RESERVED5])
            SAL_INFO("svtools", "RESERVED5");
    }

    void lcl_dump_codepage_coverage(const boost::optional<std::bitset<vcl::CodePageCoverage::MAX_CP_ENUM>> &roIn)
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
        if (rIn[vcl::CodePageCoverage::CP437])
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

namespace
{
    UScriptCode attemptToDisambiguateHan(UScriptCode eScript, OutputDevice const &rDevice)
    {
        //If we're a CJK font, see if we seem to be tuned for C, J or K
        if (eScript == USCRIPT_HAN)
        {
            const vcl::Font &rFont = rDevice.GetFont();

            bool bKore = false, bJpan = false, bHant = false, bHans = false;

            static const sal_Unicode aKorean[] = { 0x3131 };
            OUString sKorean(aKorean, SAL_N_ELEMENTS(aKorean));
            if (-1 == rDevice.HasGlyphs(rFont, sKorean))
                bKore = true;

            static const sal_Unicode aJapanese[] = { 0x3007, 0x9F9D };
            OUString sJapanese(aJapanese, SAL_N_ELEMENTS(aJapanese));
            if (-1 == rDevice.HasGlyphs(rFont, sJapanese))
                bJpan = true;

            static const sal_Unicode aTraditionalChinese[] = { 0x570B };
            OUString sTraditionalChinese(aTraditionalChinese, SAL_N_ELEMENTS(aTraditionalChinese));
            if (-1 == rDevice.HasGlyphs(rFont, sTraditionalChinese))
                bHant = true;

            static const sal_Unicode aSimplifiedChinese[] = { 0x56FD };
            OUString sSimplifiedChinese(aSimplifiedChinese, SAL_N_ELEMENTS(aSimplifiedChinese));
            if (-1 == rDevice.HasGlyphs(rFont, sSimplifiedChinese))
                bHans = true;

            if (bKore && !bJpan && !bHans)
                eScript = USCRIPT_KOREAN;
            else if (bJpan && !bKore && !bHans)
                eScript = USCRIPT_JAPANESE;
            else if (bHant && !bHans && !bKore && !bJpan)
                eScript = USCRIPT_TRADITIONAL_HAN;
            else if (bHans && !bHant && !bKore && !bJpan)
                eScript = USCRIPT_SIMPLIFIED_HAN;
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
