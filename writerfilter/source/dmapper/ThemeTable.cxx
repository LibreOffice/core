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

#include "ThemeTable.hxx"
#include <i18nlangtag/languagetag.hxx>
#include <ooxml/resourceids.hxx>

using namespace com::sun::star;

namespace writerfilter {
namespace dmapper
{

struct ThemeTable_Impl
{
    ThemeTable_Impl() :
        m_currentThemeFontId(0),
        m_currentFontThemeEntry(),
        m_supplementalFontId(0)
        {}
    std::map<sal_uInt32, std::map<sal_uInt32, OUString> > m_themeFontMap;
    sal_uInt32 m_currentThemeFontId;
    std::map<sal_uInt32, OUString> m_currentFontThemeEntry;
    OUString m_supplementalFontName;
    sal_uInt32 m_supplementalFontId;
    OUString m_themeFontLangEastAsia;
    OUString m_themeFontLangBidi;
};

ThemeTable::ThemeTable()
: LoggedProperties("ThemeTable")
, LoggedTable("ThemeTable")
, m_pImpl( new ThemeTable_Impl )
{

}

ThemeTable::~ThemeTable()
{
}

void ThemeTable::lcl_attribute(Id Name, Value & val)
{
    OUString sValue = val.getString();
    switch(Name)
    {
        case NS_ooxml::LN_CT_TextFont_typeface:
         if (!sValue.isEmpty())
             m_pImpl->m_currentFontThemeEntry[m_pImpl->m_currentThemeFontId] = sValue;
         break;
        case NS_ooxml::LN_CT_SupplementalFont_script:
            if (!sValue.isEmpty())
            {
                if (sValue == m_pImpl->m_themeFontLangBidi)
                    m_pImpl->m_supplementalFontId = NS_ooxml::LN_CT_FontCollection_cs;
                else if (sValue == m_pImpl->m_themeFontLangEastAsia)
                    m_pImpl->m_supplementalFontId = NS_ooxml::LN_CT_FontCollection_ea;
            }
            break;
        case NS_ooxml::LN_CT_SupplementalFont_typeface:
            if (!sValue.isEmpty())
                m_pImpl->m_supplementalFontName = sValue;
            break;
        default:
        {
#ifdef DEBUG_WRITERFILTER
            TagLogger::getInstance().element("unhandled");
#endif
        }
    }
    if(m_pImpl->m_supplementalFontId && m_pImpl->m_supplementalFontName.getLength() > 0)
    {
        m_pImpl->m_currentFontThemeEntry[m_pImpl->m_supplementalFontId] = m_pImpl->m_supplementalFontName;
        m_pImpl->m_supplementalFontName.clear();
        m_pImpl->m_supplementalFontId = 0;
    }
}

void ThemeTable::lcl_sprm(Sprm& rSprm)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("ThemeTable.sprm");
    TagLogger::getInstance().chars(rSprm.toString());
#endif

    m_pImpl->m_supplementalFontName.clear();
    m_pImpl->m_supplementalFontId = 0;

    sal_uInt32 nSprmId = rSprm.getId();
    switch(nSprmId)
    {
    case NS_ooxml::LN_CT_BaseStyles_fontScheme:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
                pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_FontScheme_majorFont:
    case NS_ooxml::LN_CT_FontScheme_minorFont:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        m_pImpl->m_currentFontThemeEntry = std::map<sal_uInt32, OUString>();
        if( pProperties.get())
                pProperties->resolve(*this);
        m_pImpl->m_themeFontMap[nSprmId] = m_pImpl->m_currentFontThemeEntry;
    }
    break;
    case NS_ooxml::LN_CT_FontCollection_latin:
    case NS_ooxml::LN_CT_FontCollection_ea:
    case NS_ooxml::LN_CT_FontCollection_cs:
    {
        m_pImpl->m_currentThemeFontId = nSprmId;
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
                pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_FontCollection_font:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get())
            pProperties->resolve(*this);
    }
    break;
    default:
        {
#ifdef DEBUG_WRITERFILTER
            TagLogger::getInstance().element("unhandled");
#endif
        }
    }
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void ThemeTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("ThemeTable.entry");
#endif

    ref->resolve(*this);

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

OUString ThemeTable::getStringForTheme(const Id id)
{
    switch (id)
    {
        case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
            return OUString("majorEastAsia");
        case NS_ooxml::LN_Value_ST_Theme_majorBidi:
            return OUString("majorBidi");
        case NS_ooxml::LN_Value_ST_Theme_majorAscii:
            return OUString("majorAscii");
        case NS_ooxml::LN_Value_ST_Theme_majorHAnsi:
            return OUString("majorHAnsi");
        case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
            return OUString("minorEastAsia");
        case NS_ooxml::LN_Value_ST_Theme_minorBidi:
            return OUString("minorBidi");
        case NS_ooxml::LN_Value_ST_Theme_minorAscii:
            return OUString("minorAscii");
        case NS_ooxml::LN_Value_ST_Theme_minorHAnsi:
            return OUString("minorHAnsi");
    }
    return OUString();
}
const OUString ThemeTable::getFontNameForTheme(const Id id) const
{
    std::map<sal_uInt32, OUString> tmpThemeFontMap;
    switch (id)
    {
    case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
    case NS_ooxml::LN_Value_ST_Theme_majorBidi:
    case NS_ooxml::LN_Value_ST_Theme_majorAscii:
    case NS_ooxml::LN_Value_ST_Theme_majorHAnsi:
        tmpThemeFontMap = m_pImpl->m_themeFontMap[NS_ooxml::LN_CT_FontScheme_majorFont];
    break;
    case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
    case NS_ooxml::LN_Value_ST_Theme_minorBidi:
    case NS_ooxml::LN_Value_ST_Theme_minorAscii:
    case NS_ooxml::LN_Value_ST_Theme_minorHAnsi:
        tmpThemeFontMap = m_pImpl->m_themeFontMap[NS_ooxml::LN_CT_FontScheme_minorFont];
    break;
    default:
        return OUString();
    }

    switch (id)
    {
    case NS_ooxml::LN_Value_ST_Theme_majorAscii:
    case NS_ooxml::LN_Value_ST_Theme_majorHAnsi:
    case NS_ooxml::LN_Value_ST_Theme_minorAscii:
    case NS_ooxml::LN_Value_ST_Theme_minorHAnsi:
        {
            std::map<sal_uInt32, OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_latin);
            if (Iter != tmpThemeFontMap.end())
                      return Iter->second;
            return OUString();
        }
    case NS_ooxml::LN_Value_ST_Theme_majorBidi:
    case NS_ooxml::LN_Value_ST_Theme_minorBidi:
        {
             std::map<sal_uInt32, OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_cs);
             if (Iter != tmpThemeFontMap.end())
                 return Iter->second;
             return OUString();
        }
    case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
    case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
        {
             std::map<sal_uInt32, OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_ea);
             if (Iter != tmpThemeFontMap.end())
                 return Iter->second;
             return OUString();
        }
    default:
    return OUString();
    }
}

void ThemeTable::setThemeFontLangProperties(const uno::Sequence<beans::PropertyValue>& aPropSeq)
{
    for (sal_Int32 i = 0 ; i < aPropSeq.getLength() ; i ++)
    {
        OUString sLocaleName;
        aPropSeq.getConstArray()[i].Value >>= sLocaleName;
        if (aPropSeq.getConstArray()[i].Name == "eastAsia")
            m_pImpl->m_themeFontLangEastAsia = fromLocaleToScriptTag(sLocaleName);
        if (aPropSeq.getConstArray()[i].Name == "bidi")
            m_pImpl->m_themeFontLangBidi = fromLocaleToScriptTag(sLocaleName);

    }
}

OUString ThemeTable::fromLocaleToScriptTag(const OUString& sLocale)
{
    return fromLCIDToScriptTag(LanguageTag::convertToLanguageType(sLocale));
}

OUString ThemeTable::fromLCIDToScriptTag(LanguageType lang)
{
    // conversion list from:
    // http://blogs.msdn.com/b/officeinteroperability/archive/2013/04/22/office-open-xml-themes-schemes-and-fonts.aspx
    switch (static_cast<sal_uInt16>(lang))
    {
        case 0x429  :  // lidFarsi
        case 0x401  :  // lidArabic
        case 0x801  :  // lidIraq
        case 0xc01  :  // lidEgyptian
        case 0x1001 :  // lidLibya
        case 0x1401 :  // lidAlgerian
        case 0x1801 :  // lidMorocco
        case 0x1c01 :  // lidTunisia
        case 0x2001 :  // lidOman
        case 0x2401 :  // lidYemen
        case 0x2801 :  // lidSyria
        case 0x2c01 :  // lidJordan
        case 0x3001 :  // lidLebanon
        case 0x3401 :  // lidKuwait
        case 0x3801 :  // lidUAE
        case 0x3c01 :  // lidBahrain
        case 0x4001 :  // lidQatar
        case 0x420  :  // lidUrdu
        case 0x846  :  // lidPunjabiPakistan
        case 0x859  :  // lidSindhiPakistan
        case 0x45f  :  // lidTamazight
        case 0x460  :  // lidKashmiri
        case 0x463  :  // lidPashto
        case 0x48c  :  // lidDari
            return OUString("Arab");
        case 0x42b  :  // lidArmenian
            return OUString("Armn");
        case 0x445  :  // lidBengali
        case 0x845  :  // lidBengaliBangladesh
        case 0x44d  :  // lidAssamese
        case 0x458  :  // lidManipuri
            return OUString("Beng");
        case 0x45d  :  // lidInuktitut
            return OUString("Cans");
        case 0x45c  :  // lidCherokee
            return OUString("Cher");
        case 0x419  :  // lidRussian
        case 0x402  :  // lidBulgarian
        case 0x281a :  // lidSerbianCyrillic
        case 0x422  :  // lidUkranian
        case 0x819  :  // lidRussianMoldavia
        case 0xc1a  :  // lidSerbianCyrillicSerbMont
        case 0x1c1a :  // lidSerbianBosniaHerzegovinaCyrillic
        case 0x201a :  // lidBosnianBosniaHerzegovinaCyrillic
        case 0x301a :  // lidSerbianMontenegroCyrillic
        case 0x423  :  // lidByelorussian
        case 0x428  :  // lidTajik
        case 0x82c  :  // lidAzeriCyrillic
        case 0x42f  :  // lidMacedonian
        case 0x43f  :  // lidKazakh
        case 0x440  :  // lidKyrgyz
        case 0x843  :  // lidUzbekCyrillic
        case 0x444  :  // lidTatar
        case 0x450  :  // lidMongolian
        case 0x46d  :  // lidBashkir
        case 0x485  :  // lidSakha
            return OUString("Cyrl");
        case 0x439  :  // lidHindi
        case 0x44e  :  // lidMarathi
        case 0x44f  :  // lidSanskrit
        case 0x457  :  // lidKonkani
        case 0x459  :  // lidSindhi
        case 0x860  :  // lidKashmiriIndia
        case 0x461  :  // lidNepali
        case 0x861  :  // lidNepaliIndia
            return OUString("Deva");
        case 0x45e  :  // lidAmharic
        case 0x473  :  // lidTigrignaEthiopic
        case 0x873  :  // lidTigrignaEritrea
            return OUString("Ethi");
        case 0x437  :  // lidGeorgian
            return OUString("Geor");
        case 0x408  :  // lidGreek
            return OUString("Grek");
        case 0x447  :  // lidGujarati
            return OUString("Gujr");
        case 0x446  :  // lidPunjabi
            return OUString("Guru");
        case 0x412  :  // lidKoreanExtWansung
            return OUString("Hang");
        case 0x804  :  // lidChineseSimp
        case 0x1004 :  // lidSingapore
            return OUString("Hans");
        case 0x404  :  // lidChineseTrad
        case 0xc04  :  // lidHongkong
        case 0x1404 :  // lidMacau
            return OUString("Hant");
        case 0x40d  :  // lidHebrew
        case 0x43d  :  // lidYiddish
            return OUString("Hebr");
        case 0x411  :  // lidJapanese
            return OUString("Jpan");
        case 0x453  :  // lidKhmer
            return OUString("Khmr");
        case 0x44b  :  // lidKannada
            return OUString("Knda");
        case 0x454  :  // lidLao
            return OUString("Laoo");
        case 0x409  :  // lidAmerican
        case 0xc09  :  // lidAustralian
        case 0x809  :  // lidBritish
        case 0x1009 :  // lidEnglishCanadian
        case 0x403  :  // lidCatalan
        case 0x406  :  // lidDanish
        case 0x413  :  // lidDutch
        case 0x813  :  // lidDutchBelgian
        case 0x479  :  // lidPapiamentu
        case 0x40b  :  // lidFinnish
        case 0x40c  :  // lidFrench
        case 0xc0c  :  // lidFrenchCanadian
        case 0x407  :  // lidGerman
        case 0x807  :  // lidSwissGerman
        case 0xc07  :  // lidAustrianGerman
        case 0x1007 :  // lidGermanLuxembourg
        case 0x1407 :  // lidGermanLiechtenstein
        case 0x410  :  // lidItalian
        case 0x414  :  // lidNorskBokmal
        case 0x814  :  // lidNorskNynorsk
        case 0x416  :  // lidPortBrazil
        case 0x816  :  // lidPortIberian
        case 0x40a  :  // lidSpanish
        case 0x41d  :  // lidSwedish
        case 0x405  :  // lidCzech
        case 0x40e  :  // lidHungarian
        case 0x415  :  // lidPolish
        case 0x41f  :  // lidTurkish
        case 0x42d  :  // lidBasque
        case 0x424  :  // lidSlovenian
        case 0x426  :  // lidLatvian
        case 0x427  :  // lidLithuanian
        case 0x418  :  // lidRomanian
        case 0x818  :  // lidRomanianMoldavia
        case 0x241a :  // lidSerbianLatin
        case 0x41a  :  // lidCroatian, lidCroat
        case 0x491  :  // lidGaelicScots
        case 0x83c  :  // lidGaelicIrish
        case 0x430  :  // lidSutu
        case 0x431  :  // lidTsonga
        case 0x432  :  // lidTswana
        case 0x433  :  // lidVenda
        case 0x434  :  // lidXhosa
        case 0x435  :  // lidZulu
        case 0x436  :  // lidAfrikaans
        case 0x425  :  // lidEstonian
        case 0x456  :  // lidGalician
        case 0x41b  :  // lidSlovak
        case 0x1409 :  // lidEnglishNewZealand
        case 0x1809 :  // lidEnglishIreland
        case 0x1c09 :  // lidEnglishSouthAfrica
        case 0x2009 :  // lidEnglishJamaica
        case 0x2409 :  // lidEnglishCaribbean
        case 0x2809 :  // lidEnglishBelize
        case 0x2c09 :  // lidEnglishTrinidad
        case 0x3009 :  // lidEnglishZimbabwe
        case 0x3409 :  // lidEnglishPhilippines
        case 0x3809 :  // lidEnglishIndonesia
        case 0x3c09 :  // lidEnglishHongKong
        case 0x4009 :  // lidEnglishIndia
        case 0x4409 :  // lidEnglishMalaysia
        case 0x4809 :  // lidEnglishSingapore
        case 0x80a  :  // lidSpanishMexican, lidMexican
        case 0xc0a  :  // lidSpanishModern
        case 0x100a :  // lidGuatemala
        case 0x140a :  // lidCostaRica
        case 0x180a :  // lidPanama
        case 0x1c0a :  // lidDominicanRepublic
        case 0x200a :  // lidSpanishSA, lidVenezuela
        case 0x240a :  // lidColombia
        case 0x280a :  // lidPeru
        case 0x2c0a :  // lidArgentina
        case 0x300a :  // lidEcuador
        case 0x340a :  // lidChile
        case 0x380a :  // lidUruguay
        case 0x3c0a :  // lidParguay
        case 0x400a :  // lidBolivia
        case 0x440a :  // lidElSalvador
        case 0x480a :  // lidHonduras
        case 0x4c0a :  // lidNicaragua
        case 0x500a :  // lidPuertoRico
        case 0x540a :  // lidSpanishUS
        case 0x80c  :  // lidFrenchBelgian
        case 0x100c :  // lidFrenchSwiss
        case 0x140c :  // lidFrenchLuxembourg
        case 0x180c :  // lidFrenchMonaco
        case 0x1c0c :  // lidFrenchWestIndies
        case 0x200c :  // lidFrenchReunion
        case 0x240c :  // lidFrenchCongoDRC, lidFrenchZaire
        case 0x280c :  // lidFrenchSenegal
        case 0x2c0c :  // lidFrenchCameroon
        case 0x300c :  // lidFrenchCotedIvoire
        case 0x340c :  // lidFrenchMali
        case 0x3c0c :  // lidFrenchHaiti
        case 0x380c :  // lidFrenchMorocco
        case 0x40f  :  // lidIcelandic
        case 0x810  :  // lidItalianSwiss
        case 0x417  :  // lidRhaetoRomanic, lidRomanic
        case 0x81a  :  // lidSerbianLatinSerbMont, lidCroatSerbo
        case 0x101a :  // lidBosniaHerzegovina
        case 0x141a :  // lidBosnianBosniaHerzegovinaLatin
        case 0x181a :  // lidSerbianBosniaHerzegovinaLatin
        case 0x2c1a :  // lidSerbianMontenegroLatin
        case 0x41c  :  // lidAlbanian
        case 0x81d  :  // lidSwedishFinland
        case 0x421  :  // lidBahasa, lidIndonesian
        case 0x42c  :  // lidAzeriLatin
        case 0x42e  :  // lidSorbian
        case 0x82e  :  // lidLowerSorbian
        case 0x438  :  // lidFaeroese
        case 0x43a  :  // lidMaltese
        case 0x43b  :  // lidSamiLappish
        case 0x83b  :  // lidNorthSamiSwe
        case 0xc3b  :  // lidNorthernSamiFi
        case 0x103b :  // lidLuleSamiNor
        case 0x143b :  // lidLuleSamiSwe
        case 0x183b :  // lidSouthSamiNor
        case 0x1c3b :  // lidSouthSamiSwe
        case 0x203b :  // lidSkoltSami
        case 0x243b :  // lidInariSami
        case 0x43e  :  // lidMalaysian
        case 0x83e  :  // lidMalayBrunei
        case 0x441  :  // lidSwahili
        case 0x442  :  // lidTurkmen
        case 0x443  :  // lidUzbekLatin
        case 0x452  :  // lidWelsh
        case 0x85d  :  // lidInuktitutLatin
        case 0x85f  :  // lidTamazightLatin
        case 0x462  :  // lidFrisian
        case 0x464  :  // lidFilipino
        case 0x466  :  // lidEdo
        case 0x467  :  // lidFulfulde
        case 0x468  :  // lidHausa
        case 0x469  :  // lidIbibio
        case 0x46a  :  // lidYoruba
        case 0x46b  :  // lidQuechuaBol
        case 0x86b  :  // lidQuechuaEcu
        case 0xc6b  :  // lidQuechuaPe
        case 0x46c  :  // lidSesothoSaLeboa
        case 0x46e  :  // lidLuxembourgish
        case 0x46f  :  // lidGreenlandic
        case 0x470  :  // lidIgbo
        case 0x471  :  // lidKanuri
        case 0x472  :  // lidOromo
        case 0x474  :  // lidGuarani
        case 0x475  :  // lidHawaiian
        case 0x476  :  // lidLatin
        case 0x477  :  // lidSomali
        case 0x47a  :  // lidMapudungun
        case 0x47c  :  // lidMohawk
        case 0x47e  :  // lidBreton
        case 0x481  :  // lidMaori
        case 0x482  :  // lidOccitan
        case 0x483  :  // lidCorsican
        case 0x484  :  // lidAlsatian
        case 0x486  :  // lidKiche
        case 0x487  :  // lidKinyarwanda
        case 0x488  :  // lidWolof
            return OUString("Latn");
        case 0x44c  :  // lidMalayalam
            return OUString("Mlym");
        case 0x850  :  // lidMongolianMongo
            return OUString("Mong");
        case 0x455  :  // lidBurmese
            return OUString("Mymr");
        case 0x448  :  // lidOriya
            return OUString("Orya");
        case 0x45b  :  // lidSinhalese
            return OUString("Sinh");
        case 0x45a  :  // lidSyriac
            return OUString("Syrc");
        case 0x449  :  // lidTamil
            return OUString("Taml");
        case 0x44a  :  // lidTelugu
            return OUString("Telu");
        case 0x465  :  // lidMaldivian
            return OUString("Thaa");
        case 0x41e  :  // lidThai
            return OUString("Thai");
        case 0x451  :  // lidTibetan
        case 0x851  :  // lidBhutanese
            return OUString("Tibt");
        case 0x480  :  // lidUighur
            return OUString("Uigh");
        case 0x42a  :  // lidVietnamese
            return OUString("Viet");
        case 0x478  :  // lidYi
            return OUString("Yiii");
        default:
            return OUString();
    }
}

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
