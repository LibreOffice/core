/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <ThemeTable.hxx>
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#endif
#include <stdio.h>
#ifdef DEBUG_DOMAINMAPPER
#include "dmapperLoggers.hxx"
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
    std::map<sal_uInt32, std::map<sal_uInt32, ::rtl::OUString> > m_themeFontMap;
    sal_uInt32 m_currentThemeFontId;
    std::map<sal_uInt32, ::rtl::OUString> m_currentFontThemeEntry;
};

ThemeTable::ThemeTable() :
    m_pImpl( new ThemeTable_Impl )
{
    // printf("ThemeTable::ThemeTable()\n");
}

ThemeTable::~ThemeTable()
{
    delete m_pImpl;
}

void ThemeTable::attribute(Id Name, Value & val)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("ThemeTable.attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(Name));
    dmapper_logger->attribute("value", val.toString());
#endif
    // int nIntValue = val.getInt();
    ::rtl::OUString sValue = val.getString();
    // printf ( "ThemeTable::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)Name, (unsigned int)nIntValue, ::rtl::OUStringToOString(sValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: ThemeTable_attributedata */
    switch(Name)
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TextFont_typeface:
         if (sValue.getLength())
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
    dmapper_logger->endElement("ThemeTable.attribute");
#endif
}

void ThemeTable::sprm(Sprm& rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("ThemeTable.sprm");
    dmapper_logger->chars(rSprm.toString());
#endif
    
    sal_uInt32 nSprmId = rSprm.getId();
    (void)nSprmId;

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    rtl::OUString sStringValue = pValue->getString();

    // printf ( "ThemeTable::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());

    /* WRITERFILTERSTATUS: table: ThemeTable_sprm */
    switch(nSprmId)
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_BaseStyles_fontScheme:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
                pProperties->resolve(*this);
    }
    break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_FontScheme_majorFont:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_FontScheme_minorFont:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        m_pImpl->m_currentFontThemeEntry = std::map<sal_uInt32, rtl::OUString>();
            if( pProperties.get())
                pProperties->resolve(*this);
            m_pImpl->m_themeFontMap[nSprmId] = m_pImpl->m_currentFontThemeEntry;
    }
    break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_FontCollection_latin:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_FontCollection_ea:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
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
    dmapper_logger->endElement("ThemeTable.sprm");
#endif
}

void ThemeTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("ThemeTable.entry");
#endif

    ref->resolve(*this);
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("ThemeTable.entry");
#endif
}

const ::rtl::OUString ThemeTable::getFontNameForTheme(const Id id) const
{
    std::map<sal_uInt32, ::rtl::OUString> tmpThemeFontMap;
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
        return ::rtl::OUString();
    }
    
    switch (id)
    {
    case NS_ooxml::LN_Value_ST_Theme_majorAscii:
    case NS_ooxml::LN_Value_ST_Theme_majorHAnsi:
    case NS_ooxml::LN_Value_ST_Theme_minorAscii:
    case NS_ooxml::LN_Value_ST_Theme_minorHAnsi:
    {
         std::map<sal_uInt32, ::rtl::OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_latin);
             if (Iter != tmpThemeFontMap.end())
                  return (Iter)->second;
             return ::rtl::OUString();
        }
    case NS_ooxml::LN_Value_ST_Theme_majorBidi:
    case NS_ooxml::LN_Value_ST_Theme_minorBidi:
        {
             std::map<sal_uInt32, ::rtl::OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_cs);
             if (Iter != tmpThemeFontMap.end())
                 return (Iter)->second;
             return ::rtl::OUString();
        }
    case NS_ooxml::LN_Value_ST_Theme_majorEastAsia:
    case NS_ooxml::LN_Value_ST_Theme_minorEastAsia:
        {
             std::map<sal_uInt32, ::rtl::OUString>::const_iterator Iter = tmpThemeFontMap.find(NS_ooxml::LN_CT_FontCollection_ea);
             if (Iter != tmpThemeFontMap.end())
                 return (Iter)->second;
             return ::rtl::OUString();
        }
    default:
    return ::rtl::OUString();
    }
}

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
