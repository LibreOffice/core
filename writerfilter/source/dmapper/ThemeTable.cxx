/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ThemeTable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:42:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_FONTTABLE_HXX
#include <ThemeTable.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#endif
#include <stdio.h>

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
        }
    }
}

void ThemeTable::sprm(Sprm& rSprm)
{
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
        }
    }
}

void ThemeTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    // printf ( "ThemeTable::entry\n");
    ref->resolve(*this);
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
