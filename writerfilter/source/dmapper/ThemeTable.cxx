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
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include "dmapperLoggers.hxx"

#if DEBUG_DOMAINMAPPER
#include <resourcemodel/QNameToString.hxx>
#endif

namespace writerfilter {
namespace dmapper
{

struct ThemeTable_Impl
{
    ThemeTable_Impl() :
        m_currentThemeFontId(0),
        m_currentFontThemeEntry() {}
    std::map<sal_uInt32, std::map<sal_uInt32, OUString> > m_themeFontMap;
    sal_uInt32 m_currentThemeFontId;
    std::map<sal_uInt32, OUString> m_currentFontThemeEntry;
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

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
