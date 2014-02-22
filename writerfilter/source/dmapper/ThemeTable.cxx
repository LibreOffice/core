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

#include <ThemeTable.hxx>
#include <ooxml/resourceids.hxx>
#include "dmapperLoggers.hxx"

#ifdef DEBUG_DOMAINMAPPER
#include <resourcemodel/QNameToString.hxx>
#endif

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
: LoggedProperties(dmapper_logger, "ThemeTable")
, LoggedTable(dmapper_logger, "ThemeTable")
, m_pImpl( new ThemeTable_Impl )
{

}

ThemeTable::~ThemeTable()
{
    delete m_pImpl;
}

void ThemeTable::lcl_attribute(Id Name, Value & val)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("ThemeTable.attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(Name));
    dmapper_logger->attribute("value", val.toString());
#endif
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
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->element("unhandled");
#endif
        }
    }
    if(m_pImpl->m_supplementalFontId && m_pImpl->m_supplementalFontName.getLength() > 0)
    {
        m_pImpl->m_currentFontThemeEntry[m_pImpl->m_supplementalFontId] = m_pImpl->m_supplementalFontName;
        m_pImpl->m_supplementalFontName = "";
        m_pImpl->m_supplementalFontId = 0;
    }
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}

void ThemeTable::lcl_sprm(Sprm& rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("ThemeTable.sprm");
    dmapper_logger->chars(rSprm.toString());
#endif

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
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->element("unhandled");
#endif
        }
    }
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}

void ThemeTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("ThemeTable.entry");
#endif

    ref->resolve(*this);

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}

OUString ThemeTable::getStringForTheme(const Id id)
{
    std::map<sal_uInt32, OUString> tmpThemeFontMap;
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
                  return (Iter)->second;
             return OUString();
        }
    case NS_ooxml::LN_Value_ST_Theme_majorBidi:
    case NS_ooxml::LN_Value_ST_Theme_minorBidi:
        {
             std::map<sal_uInt32, OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_cs);
             if (Iter != tmpThemeFontMap.end())
                 return (Iter)->second;
             return OUString();
        }
    case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
    case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
        {
             std::map<sal_uInt32, OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_ea);
             if (Iter != tmpThemeFontMap.end())
                 return (Iter)->second;
             return OUString();
        }
    default:
    return OUString();
    }
}

void ThemeTable::setThemeFontLangProperties(uno::Sequence<beans::PropertyValue> aPropSeq)
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

OUString ThemeTable::fromLocaleToScriptTag(OUString sLocale)
{
    return fromLCIDToScriptTag(LanguageTag::convertToLanguageType(sLocale));
}

OUString ThemeTable::fromLCIDToScriptTag(LanguageType lang)
{
    
    
    switch (lang)
    {
        case 0x429  :  
        case 0x401  :  
        case 0x801  :  
        case 0xc01  :  
        case 0x1001 :  
        case 0x1401 :  
        case 0x1801 :  
        case 0x1c01 :  
        case 0x2001 :  
        case 0x2401 :  
        case 0x2801 :  
        case 0x2c01 :  
        case 0x3001 :  
        case 0x3401 :  
        case 0x3801 :  
        case 0x3c01 :  
        case 0x4001 :  
        case 0x420  :  
        case 0x846  :  
        case 0x859  :  
        case 0x45f  :  
        case 0x460  :  
        case 0x463  :  
        case 0x48c  :  
            return OUString("Arab");
        case 0x42b  :  
            return OUString("Armn");
        case 0x445  :  
        case 0x845  :  
        case 0x44d  :  
        case 0x458  :  
            return OUString("Beng");
        case 0x45d  :  
            return OUString("Cans");
        case 0x45c  :  
            return OUString("Cher");
        case 0x419  :  
        case 0x402  :  
        case 0x281a :  
        case 0x422  :  
        case 0x819  :  
        case 0xc1a  :  
        case 0x1c1a :  
        case 0x201a :  
        case 0x301a :  
        case 0x423  :  
        case 0x428  :  
        case 0x82c  :  
        case 0x42f  :  
        case 0x43f  :  
        case 0x440  :  
        case 0x843  :  
        case 0x444  :  
        case 0x450  :  
        case 0x46d  :  
        case 0x485  :  
            return OUString("Cyrl");
        case 0x439  :  
        case 0x44e  :  
        case 0x44f  :  
        case 0x457  :  
        case 0x459  :  
        case 0x860  :  
        case 0x461  :  
        case 0x861  :  
            return OUString("Deva");
        case 0x45e  :  
        case 0x473  :  
        case 0x873  :  
            return OUString("Ethi");
        case 0x437  :  
            return OUString("Geor");
        case 0x408  :  
            return OUString("Grek");
        case 0x447  :  
            return OUString("Gujr");
        case 0x446  :  
            return OUString("Guru");
        case 0x412  :  
            return OUString("Hang");
        case 0x804  :  
        case 0x1004 :  
            return OUString("Hans");
        case 0x404  :  
        case 0xc04  :  
        case 0x1404 :  
            return OUString("Hant");
        case 0x40d  :  
        case 0x43d  :  
            return OUString("Hebr");
        case 0x411  :  
            return OUString("Jpan");
        case 0x453  :  
            return OUString("Khmr");
        case 0x44b  :  
            return OUString("Knda");
        case 0x454  :  
            return OUString("Laoo");
        case 0x409  :  
        case 0xc09  :  
        case 0x809  :  
        case 0x1009 :  
        case 0x403  :  
        case 0x406  :  
        case 0x413  :  
        case 0x813  :  
        case 0x479  :  
        case 0x40b  :  
        case 0x40c  :  
        case 0xc0c  :  
        case 0x407  :  
        case 0x807  :  
        case 0xc07  :  
        case 0x1007 :  
        case 0x1407 :  
        case 0x410  :  
        case 0x414  :  
        case 0x814  :  
        case 0x416  :  
        case 0x816  :  
        case 0x40a  :  
        case 0x41d  :  
        case 0x405  :  
        case 0x40e  :  
        case 0x415  :  
        case 0x41f  :  
        case 0x42d  :  
        case 0x424  :  
        case 0x426  :  
        case 0x427  :  
        case 0x418  :  
        case 0x818  :  
        case 0x241a :  
        case 0x41a  :  
        case 0x491  :  
        case 0x83c  :  
        case 0x430  :  
        case 0x431  :  
        case 0x432  :  
        case 0x433  :  
        case 0x434  :  
        case 0x435  :  
        case 0x436  :  
        case 0x425  :  
        case 0x456  :  
        case 0x41b  :  
        case 0x1409 :  
        case 0x1809 :  
        case 0x1c09 :  
        case 0x2009 :  
        case 0x2409 :  
        case 0x2809 :  
        case 0x2c09 :  
        case 0x3009 :  
        case 0x3409 :  
        case 0x3809 :  
        case 0x3c09 :  
        case 0x4009 :  
        case 0x4409 :  
        case 0x4809 :  
        case 0x80a  :  
        case 0xc0a  :  
        case 0x100a :  
        case 0x140a :  
        case 0x180a :  
        case 0x1c0a :  
        case 0x200a :  
        case 0x240a :  
        case 0x280a :  
        case 0x2c0a :  
        case 0x300a :  
        case 0x340a :  
        case 0x380a :  
        case 0x3c0a :  
        case 0x400a :  
        case 0x440a :  
        case 0x480a :  
        case 0x4c0a :  
        case 0x500a :  
        case 0x540a :  
        case 0x80c  :  
        case 0x100c :  
        case 0x140c :  
        case 0x180c :  
        case 0x1c0c :  
        case 0x200c :  
        case 0x240c :  
        case 0x280c :  
        case 0x2c0c :  
        case 0x300c :  
        case 0x340c :  
        case 0x3c0c :  
        case 0x380c :  
        case 0x40f  :  
        case 0x810  :  
        case 0x417  :  
        case 0x81a  :  
        case 0x101a :  
        case 0x141a :  
        case 0x181a :  
        case 0x2c1a :  
        case 0x41c  :  
        case 0x81d  :  
        case 0x421  :  
        case 0x42c  :  
        case 0x42e  :  
        case 0x82e  :  
        case 0x438  :  
        case 0x43a  :  
        case 0x43b  :  
        case 0x83b  :  
        case 0xc3b  :  
        case 0x103b :  
        case 0x143b :  
        case 0x183b :  
        case 0x1c3b :  
        case 0x203b :  
        case 0x243b :  
        case 0x43e  :  
        case 0x83e  :  
        case 0x441  :  
        case 0x442  :  
        case 0x443  :  
        case 0x452  :  
        case 0x85d  :  
        case 0x85f  :  
        case 0x462  :  
        case 0x464  :  
        case 0x466  :  
        case 0x467  :  
        case 0x468  :  
        case 0x469  :  
        case 0x46a  :  
        case 0x46b  :  
        case 0x86b  :  
        case 0xc6b  :  
        case 0x46c  :  
        case 0x46e  :  
        case 0x46f  :  
        case 0x470  :  
        case 0x471  :  
        case 0x472  :  
        case 0x474  :  
        case 0x475  :  
        case 0x476  :  
        case 0x477  :  
        case 0x47a  :  
        case 0x47c  :  
        case 0x47e  :  
        case 0x481  :  
        case 0x482  :  
        case 0x483  :  
        case 0x484  :  
        case 0x486  :  
        case 0x487  :  
        case 0x488  :  
            return OUString("Latn");
        case 0x44c  :  
            return OUString("Mlym");
        case 0x850  :  
            return OUString("Mong");
        case 0x455  :  
            return OUString("Mymr");
        case 0x448  :  
            return OUString("Orya");
        case 0x45b  :  
            return OUString("Sinh");
        case 0x45a  :  
            return OUString("Syrc");
        case 0x449  :  
            return OUString("Taml");
        case 0x44a  :  
            return OUString("Telu");
        case 0x465  :  
            return OUString("Thaa");
        case 0x41e  :  
            return OUString("Thai");
        case 0x451  :  
        case 0x851  :  
            return OUString("Tibt");
        case 0x480  :  
            return OUString("Uigh");
        case 0x42a  :  
            return OUString("Viet");
        case 0x478  :  
            return OUString("Yiii");
        default:
            return OUString("");
    }
}

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
