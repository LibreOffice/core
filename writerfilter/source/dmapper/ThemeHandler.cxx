/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ThemeHandler.hxx"
#include <i18nlangtag/languagetag.hxx>
#include <ooxml/resourceids.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace com::sun::star;

namespace writerfilter::dmapper
{
namespace
{
OUString fromLCIDToScriptTag(LanguageType lang)
{
    // conversion list from:
    // http://blogs.msdn.com/b/officeinteroperability/archive/2013/04/22/office-open-xml-themes-schemes-and-fonts.aspx
    switch (static_cast<sal_uInt16>(lang))
    {
        case 0x429: // lidFarsi
        case 0x401: // lidArabic
        case 0x801: // lidIraq
        case 0xc01: // lidEgyptian
        case 0x1001: // lidLibya
        case 0x1401: // lidAlgerian
        case 0x1801: // lidMorocco
        case 0x1c01: // lidTunisia
        case 0x2001: // lidOman
        case 0x2401: // lidYemen
        case 0x2801: // lidSyria
        case 0x2c01: // lidJordan
        case 0x3001: // lidLebanon
        case 0x3401: // lidKuwait
        case 0x3801: // lidUAE
        case 0x3c01: // lidBahrain
        case 0x4001: // lidQatar
        case 0x420: // lidUrdu
        case 0x846: // lidPunjabiPakistan
        case 0x859: // lidSindhiPakistan
        case 0x45f: // lidTamazight
        case 0x460: // lidKashmiri
        case 0x463: // lidPashto
        case 0x48c: // lidDari
            return "Arab";
        case 0x42b: // lidArmenian
            return "Armn";
        case 0x445: // lidBengali
        case 0x845: // lidBengaliBangladesh
        case 0x44d: // lidAssamese
        case 0x458: // lidManipuri
            return "Beng";
        case 0x45d: // lidInuktitut
            return "Cans";
        case 0x45c: // lidCherokee
            return "Cher";
        case 0x419: // lidRussian
        case 0x402: // lidBulgarian
        case 0x281a: // lidSerbianCyrillic
        case 0x422: // lidUkranian
        case 0x819: // lidRussianMoldavia
        case 0xc1a: // lidSerbianCyrillicSerbMont
        case 0x1c1a: // lidSerbianBosniaHerzegovinaCyrillic
        case 0x201a: // lidBosnianBosniaHerzegovinaCyrillic
        case 0x301a: // lidSerbianMontenegroCyrillic
        case 0x423: // lidByelorussian
        case 0x428: // lidTajik
        case 0x82c: // lidAzeriCyrillic
        case 0x42f: // lidMacedonian
        case 0x43f: // lidKazakh
        case 0x440: // lidKyrgyz
        case 0x843: // lidUzbekCyrillic
        case 0x444: // lidTatar
        case 0x450: // lidMongolian
        case 0x46d: // lidBashkir
        case 0x485: // lidSakha
            return "Cyrl";
        case 0x439: // lidHindi
        case 0x44e: // lidMarathi
        case 0x44f: // lidSanskrit
        case 0x457: // lidKonkani
        case 0x459: // lidSindhi
        case 0x860: // lidKashmiriIndia
        case 0x461: // lidNepali
        case 0x861: // lidNepaliIndia
            return "Deva";
        case 0x45e: // lidAmharic
        case 0x473: // lidTigrignaEthiopic
        case 0x873: // lidTigrignaEritrea
            return "Ethi";
        case 0x437: // lidGeorgian
            return "Geor";
        case 0x408: // lidGreek
            return "Grek";
        case 0x447: // lidGujarati
            return "Gujr";
        case 0x446: // lidPunjabi
            return "Guru";
        case 0x412: // lidKoreanExtWansung
            return "Hang";
        case 0x804: // lidChineseSimp
        case 0x1004: // lidSingapore
            return "Hans";
        case 0x404: // lidChineseTrad
        case 0xc04: // lidHongkong
        case 0x1404: // lidMacau
            return "Hant";
        case 0x40d: // lidHebrew
        case 0x43d: // lidYiddish
            return "Hebr";
        case 0x411: // lidJapanese
            return "Jpan";
        case 0x453: // lidKhmer
            return "Khmr";
        case 0x44b: // lidKannada
            return "Knda";
        case 0x454: // lidLao
            return "Laoo";
        case 0x409: // lidAmerican
        case 0xc09: // lidAustralian
        case 0x809: // lidBritish
        case 0x1009: // lidEnglishCanadian
        case 0x403: // lidCatalan
        case 0x406: // lidDanish
        case 0x413: // lidDutch
        case 0x813: // lidDutchBelgian
        case 0x479: // lidPapiamentu
        case 0x40b: // lidFinnish
        case 0x40c: // lidFrench
        case 0xc0c: // lidFrenchCanadian
        case 0x407: // lidGerman
        case 0x807: // lidSwissGerman
        case 0xc07: // lidAustrianGerman
        case 0x1007: // lidGermanLuxembourg
        case 0x1407: // lidGermanLiechtenstein
        case 0x410: // lidItalian
        case 0x414: // lidNorskBokmal
        case 0x814: // lidNorskNynorsk
        case 0x416: // lidPortBrazil
        case 0x816: // lidPortIberian
        case 0x40a: // lidSpanish
        case 0x41d: // lidSwedish
        case 0x405: // lidCzech
        case 0x40e: // lidHungarian
        case 0x415: // lidPolish
        case 0x41f: // lidTurkish
        case 0x42d: // lidBasque
        case 0x424: // lidSlovenian
        case 0x426: // lidLatvian
        case 0x427: // lidLithuanian
        case 0x418: // lidRomanian
        case 0x818: // lidRomanianMoldavia
        case 0x241a: // lidSerbianLatin
        case 0x41a: // lidCroatian, lidCroat
        case 0x491: // lidGaelicScots
        case 0x83c: // lidGaelicIrish
        case 0x430: // lidSutu
        case 0x431: // lidTsonga
        case 0x432: // lidTswana
        case 0x433: // lidVenda
        case 0x434: // lidXhosa
        case 0x435: // lidZulu
        case 0x436: // lidAfrikaans
        case 0x425: // lidEstonian
        case 0x456: // lidGalician
        case 0x41b: // lidSlovak
        case 0x1409: // lidEnglishNewZealand
        case 0x1809: // lidEnglishIreland
        case 0x1c09: // lidEnglishSouthAfrica
        case 0x2009: // lidEnglishJamaica
        case 0x2409: // lidEnglishCaribbean
        case 0x2809: // lidEnglishBelize
        case 0x2c09: // lidEnglishTrinidad
        case 0x3009: // lidEnglishZimbabwe
        case 0x3409: // lidEnglishPhilippines
        case 0x3809: // lidEnglishIndonesia
        case 0x3c09: // lidEnglishHongKong
        case 0x4009: // lidEnglishIndia
        case 0x4409: // lidEnglishMalaysia
        case 0x4809: // lidEnglishSingapore
        case 0x80a: // lidSpanishMexican, lidMexican
        case 0xc0a: // lidSpanishModern
        case 0x100a: // lidGuatemala
        case 0x140a: // lidCostaRica
        case 0x180a: // lidPanama
        case 0x1c0a: // lidDominicanRepublic
        case 0x200a: // lidSpanishSA, lidVenezuela
        case 0x240a: // lidColombia
        case 0x280a: // lidPeru
        case 0x2c0a: // lidArgentina
        case 0x300a: // lidEcuador
        case 0x340a: // lidChile
        case 0x380a: // lidUruguay
        case 0x3c0a: // lidParguay
        case 0x400a: // lidBolivia
        case 0x440a: // lidElSalvador
        case 0x480a: // lidHonduras
        case 0x4c0a: // lidNicaragua
        case 0x500a: // lidPuertoRico
        case 0x540a: // lidSpanishUS
        case 0x80c: // lidFrenchBelgian
        case 0x100c: // lidFrenchSwiss
        case 0x140c: // lidFrenchLuxembourg
        case 0x180c: // lidFrenchMonaco
        case 0x1c0c: // lidFrenchWestIndies
        case 0x200c: // lidFrenchReunion
        case 0x240c: // lidFrenchCongoDRC, lidFrenchZaire
        case 0x280c: // lidFrenchSenegal
        case 0x2c0c: // lidFrenchCameroon
        case 0x300c: // lidFrenchCotedIvoire
        case 0x340c: // lidFrenchMali
        case 0x3c0c: // lidFrenchHaiti
        case 0x380c: // lidFrenchMorocco
        case 0x40f: // lidIcelandic
        case 0x810: // lidItalianSwiss
        case 0x417: // lidRhaetoRomanic, lidRomanic
        case 0x81a: // lidSerbianLatinSerbMont, lidCroatSerbo
        case 0x101a: // lidBosniaHerzegovina
        case 0x141a: // lidBosnianBosniaHerzegovinaLatin
        case 0x181a: // lidSerbianBosniaHerzegovinaLatin
        case 0x2c1a: // lidSerbianMontenegroLatin
        case 0x41c: // lidAlbanian
        case 0x81d: // lidSwedishFinland
        case 0x421: // lidBahasa, lidIndonesian
        case 0x42c: // lidAzeriLatin
        case 0x42e: // lidSorbian
        case 0x82e: // lidLowerSorbian
        case 0x438: // lidFaeroese
        case 0x43a: // lidMaltese
        case 0x43b: // lidSamiLappish
        case 0x83b: // lidNorthSamiSwe
        case 0xc3b: // lidNorthernSamiFi
        case 0x103b: // lidLuleSamiNor
        case 0x143b: // lidLuleSamiSwe
        case 0x183b: // lidSouthSamiNor
        case 0x1c3b: // lidSouthSamiSwe
        case 0x203b: // lidSkoltSami
        case 0x243b: // lidInariSami
        case 0x43e: // lidMalaysian
        case 0x83e: // lidMalayBrunei
        case 0x441: // lidSwahili
        case 0x442: // lidTurkmen
        case 0x443: // lidUzbekLatin
        case 0x452: // lidWelsh
        case 0x85d: // lidInuktitutLatin
        case 0x85f: // lidTamazightLatin
        case 0x462: // lidFrisian
        case 0x464: // lidFilipino
        case 0x466: // lidEdo
        case 0x467: // lidFulfulde
        case 0x468: // lidHausa
        case 0x469: // lidIbibio
        case 0x46a: // lidYoruba
        case 0x46b: // lidQuechuaBol
        case 0x86b: // lidQuechuaEcu
        case 0xc6b: // lidQuechuaPe
        case 0x46c: // lidSesothoSaLeboa
        case 0x46e: // lidLuxembourgish
        case 0x46f: // lidGreenlandic
        case 0x470: // lidIgbo
        case 0x471: // lidKanuri
        case 0x472: // lidOromo
        case 0x474: // lidGuarani
        case 0x475: // lidHawaiian
        case 0x476: // lidLatin
        case 0x477: // lidSomali
        case 0x47a: // lidMapudungun
        case 0x47c: // lidMohawk
        case 0x47e: // lidBreton
        case 0x481: // lidMaori
        case 0x482: // lidOccitan
        case 0x483: // lidCorsican
        case 0x484: // lidAlsatian
        case 0x486: // lidKiche
        case 0x487: // lidKinyarwanda
        case 0x488: // lidWolof
            return "Latn";
        case 0x44c: // lidMalayalam
            return "Mlym";
        case 0x850: // lidMongolianMongo
            return "Mong";
        case 0x455: // lidBurmese
            return "Mymr";
        case 0x448: // lidOriya
            return "Orya";
        case 0x45b: // lidSinhalese
            return "Sinh";
        case 0x45a: // lidSyriac
            return "Syrc";
        case 0x449: // lidTamil
            return "Taml";
        case 0x44a: // lidTelugu
            return "Telu";
        case 0x465: // lidMaldivian
            return "Thaa";
        case 0x41e: // lidThai
            return "Thai";
        case 0x451: // lidTibetan
        case 0x851: // lidBhutanese
            return "Tibt";
        case 0x480: // lidUighur
            return "Uigh";
        case 0x42a: // lidVietnamese
            return "Viet";
        case 0x478: // lidYi
            return "Yiii";
        default:
            return OUString();
    }
}

OUString fromLocaleToScriptTag(const OUString& sLocale)
{
    return fromLCIDToScriptTag(LanguageTag::convertToLanguageType(sLocale));
}

OUString resolveMajorMinorTypeFace(model::FontScheme const& rFontSheme, const Id id)
{
    switch (id)
    {
        case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
            return rFontSheme.getMajorAsian().maTypeface;
        case NS_ooxml::LN_Value_ST_Theme_majorBidi:
            return rFontSheme.getMajorComplex().maTypeface;
        case NS_ooxml::LN_Value_ST_Theme_majorAscii:
        case NS_ooxml::LN_Value_ST_Theme_majorHAnsi:
            return rFontSheme.getMajorLatin().maTypeface;
            break;
        case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
            return rFontSheme.getMinorAsian().maTypeface;
        case NS_ooxml::LN_Value_ST_Theme_minorBidi:
            return rFontSheme.getMinorComplex().maTypeface;
        case NS_ooxml::LN_Value_ST_Theme_minorAscii:
        case NS_ooxml::LN_Value_ST_Theme_minorHAnsi:
            return rFontSheme.getMinorLatin().maTypeface;
            break;
        default:
            break;
    }
    return OUString();
}

OUString resolveSupplementalFontList(model::FontScheme const& rFontSheme, const Id id,
                                     std::u16string_view rLangAsia, std::u16string_view rLangBidi)
{
    switch (id)
    {
        case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
            return rFontSheme.findMajorSupplementalTypeface(rLangAsia);
        case NS_ooxml::LN_Value_ST_Theme_majorBidi:
            return rFontSheme.findMajorSupplementalTypeface(rLangBidi);
        case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
            return rFontSheme.findMinorSupplementalTypeface(rLangAsia);
        case NS_ooxml::LN_Value_ST_Theme_minorBidi:
            return rFontSheme.findMinorSupplementalTypeface(rLangBidi);
        default:
            break;
    }
    return OUString();
}

} // end anonymous namespace

ThemeHandler::ThemeHandler(oox::drawingml::ThemePtr const& pTheme,
                           const css::uno::Sequence<css::beans::PropertyValue>& rLangProperties)
    : mpTheme(pTheme)
{
    for (const auto& rProperty : rLangProperties)
    {
        OUString sLocaleName;
        rProperty.Value >>= sLocaleName;
        if (rProperty.Name == "eastAsia")
            maThemeFontLangEastAsia = fromLocaleToScriptTag(sLocaleName);
        if (rProperty.Name == "bidi")
            maThemeFontLangBidi = fromLocaleToScriptTag(sLocaleName);
    }
}

OUString ThemeHandler::getStringForTheme(const Id id)
{
    switch (id)
    {
        case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
            return "majorEastAsia";
        case NS_ooxml::LN_Value_ST_Theme_majorBidi:
            return "majorBidi";
        case NS_ooxml::LN_Value_ST_Theme_majorAscii:
            return "majorAscii";
        case NS_ooxml::LN_Value_ST_Theme_majorHAnsi:
            return "majorHAnsi";
        case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
            return "minorEastAsia";
        case NS_ooxml::LN_Value_ST_Theme_minorBidi:
            return "minorBidi";
        case NS_ooxml::LN_Value_ST_Theme_minorAscii:
            return "minorAscii";
        case NS_ooxml::LN_Value_ST_Theme_minorHAnsi:
            return "minorHAnsi";
    }
    return OUString();
}

OUString ThemeHandler::getFontNameForTheme(const Id id) const
{
    auto pModelTheme = mpTheme->getTheme();
    model::FontScheme const& rFontScheme = pModelTheme->getFontScheme();
    OUString aSupplementalTypeFace = resolveSupplementalFontList(
        rFontScheme, id, maThemeFontLangEastAsia, maThemeFontLangBidi);
    if (!aSupplementalTypeFace.isEmpty())
        return aSupplementalTypeFace;
    OUString aTypeFace = resolveMajorMinorTypeFace(rFontScheme, id);
    return aTypeFace;
}

} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
