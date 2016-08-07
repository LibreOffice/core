/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <ThemeTable.hxx>
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#endif
#ifdef DEBUG_DMAPPER_THEME_TABLE
#include <resourcemodel/QNameToString.hxx>
#endif
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
        m_currentFontThemeEntry() {}
    std::map<sal_uInt32, std::map<sal_uInt32, ::rtl::OUString> > m_themeFontMap;
    sal_uInt32 m_currentThemeFontId;
    std::map<sal_uInt32, ::rtl::OUString> m_currentFontThemeEntry;
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
#ifdef DEBUG_DMAPPER_THEME_TABLE
    dmapper_logger->startElement("ThemeTable.attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(Name));
    dmapper_logger->attribute("value", val.toString());
#endif
    // int nIntValue = val.getInt();
    ::rtl::OUString sValue = val.getString();
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
            dmapper_logger->element("ThemeTable.unhandled");
#endif
        }
    }
#ifdef DEBUG_DMAPPER_THEME_TABLE
    dmapper_logger->endElement("ThemeTable.attribute");
#endif
}

void ThemeTable::lcl_sprm(Sprm& rSprm)
{
#ifdef DEBUG_DMAPPER_THEME_TABLE
    dmapper_logger->startElement("ThemeTable.sprm");
    dmapper_logger->chars(rSprm.toString());
#endif

    sal_uInt32 nSprmId = rSprm.getId();
    (void)nSprmId;

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    rtl::OUString sStringValue = pValue->getString();

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
#ifdef DEBUG_DMAPPER_THEME_TABLE
            dmapper_logger->element("ThemeTable.unhandled");
#endif
        }
    }
#ifdef DEBUG_DMAPPER_THEME_TABLE
    dmapper_logger->endElement("ThemeTable.sprm");
#endif
}

void ThemeTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_DMAPPER_THEME_TABLE
    dmapper_logger->startElement("ThemeTable.entry");
#endif

    ref->resolve(*this);

#ifdef DEBUG_DMAPPER_THEME_TABLE
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
