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

#include <FontTable.hxx>
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include <vector>
#include <stdio.h>
#include <rtl/tencinfo.h>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper
{

struct FontTable_Impl
{
    std::vector< FontEntry::Pointer_t > aFontEntries;
    FontEntry::Pointer_t pCurrentEntry;
    FontTable_Impl() {}
};

FontTable::FontTable()
: LoggedProperties(dmapper_logger, "FontTable")
, LoggedTable(dmapper_logger, "FontTable")
, LoggedStream(dmapper_logger, "FontTable")
, m_pImpl( new FontTable_Impl )
{
}

FontTable::~FontTable()
{
    delete m_pImpl;
}

void FontTable::lcl_attribute(Id Name, Value & val)
{
    OSL_ENSURE( m_pImpl->pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    OUString sValue = val.getString();
    switch(Name)
    {
        case NS_rtf::LN_CBFFNM1:
            m_pImpl->pCurrentEntry->sFontName1 = sValue;
        break;
        case NS_rtf::LN_PRQ:
            m_pImpl->pCurrentEntry->nPitchRequest = static_cast<sal_Int16>( nIntValue );
        break;
        case NS_rtf::LN_FTRUETYPE:
            m_pImpl->pCurrentEntry->bTrueType = nIntValue == 1 ? true : false;
        break;
        case NS_rtf::LN_UNUSED1_3: //unused
        case NS_rtf::LN_FF: //unused
        case NS_rtf::LN_UNUSED1_7: //unused
        break;
        case NS_rtf::LN_WWEIGHT:
            m_pImpl->pCurrentEntry->nBaseWeight = nIntValue;
        break;
        case NS_rtf::LN_CHS:
            m_pImpl->pCurrentEntry->nTextEncoding = nIntValue;
        break;
        case NS_rtf::LN_IXCHSZALT:
        break;
        case NS_rtf::LN_PANOSE:
            m_pImpl->pCurrentEntry->sPanose += sValue;
        break;
        case NS_rtf::LN_FS:
            m_pImpl->pCurrentEntry->sFontSignature += sValue;
        break;
        case NS_rtf::LN_F:
        break;
        case NS_rtf::LN_ALTFONTNAME:
            m_pImpl->pCurrentEntry->sAlternativeFont = sValue;
        break;
        case NS_rtf::LN_XSZFFN:
        case NS_ooxml::LN_CT_Font_name:
            m_pImpl->pCurrentEntry->sFontName = sValue;
        break;
        case NS_ooxml::LN_CT_Charset_val:
            // w:characterSet has higher priority, set only if that one is not set
            if( m_pImpl->pCurrentEntry->nTextEncoding == RTL_TEXTENCODING_DONTKNOW )
                m_pImpl->pCurrentEntry->nTextEncoding = rtl_getTextEncodingFromWindowsCharset( nIntValue );
        break;
        case NS_ooxml::LN_CT_Charset_characterSet:
        {
            OString tmp;
            sValue.convertToString( &tmp, RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );
            m_pImpl->pCurrentEntry->nTextEncoding = rtl_getTextEncodingFromMimeCharset( tmp.getStr() );
        break;
        }
        default:
        {
            //----> debug
            int nVal = val.getInt();
            ++nVal;
            //<---- debug
        }
    }
}

void FontTable::lcl_sprm(Sprm& rSprm)
{
    OSL_ENSURE( m_pImpl->pCurrentEntry, "current entry has to be set here");
    if(!m_pImpl->pCurrentEntry)
        return ;
    sal_uInt32 nSprmId = rSprm.getId();

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    OUString sStringValue = pValue->getString();
    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Font_charset:
            resolveSprm( rSprm );
            break;
    }
}

void FontTable::resolveSprm(Sprm & r_Sprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = r_Sprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void FontTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    //create a new font entry
    OSL_ENSURE( !m_pImpl->pCurrentEntry, "current entry has to be NULL here");
    m_pImpl->pCurrentEntry.reset(new FontEntry);
    ref->resolve(*this);
    //append it to the table
    m_pImpl->aFontEntries.push_back( m_pImpl->pCurrentEntry );
    m_pImpl->pCurrentEntry.reset();
}

void FontTable::lcl_startSectionGroup()
{
}

void FontTable::lcl_endSectionGroup()
{
}

void FontTable::lcl_startParagraphGroup()
{
}

void FontTable::lcl_endParagraphGroup()
{
}

void FontTable::lcl_startCharacterGroup()
{
}

void FontTable::lcl_endCharacterGroup()
{
}

void FontTable::lcl_text(const sal_uInt8*, size_t )
{
}

void FontTable::lcl_utext(const sal_uInt8* , size_t)
{
}

void FontTable::lcl_props(writerfilter::Reference<Properties>::Pointer_t)
{
}

void FontTable::lcl_table(Id, writerfilter::Reference<Table>::Pointer_t)
{
}

void FontTable::lcl_substream(Id, ::writerfilter::Reference<Stream>::Pointer_t)
{
}

void FontTable::lcl_info(const string& )
{
}

void FontTable::lcl_startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > )
{
}

void FontTable::lcl_endShape( )
{
}

const FontEntry::Pointer_t FontTable::getFontEntry(sal_uInt32 nIndex)
{
    return (m_pImpl->aFontEntries.size() > nIndex)
        ?   m_pImpl->aFontEntries[nIndex]
        :   FontEntry::Pointer_t();
}

sal_uInt32 FontTable::size()
{
    return m_pImpl->aFontEntries.size();
}

}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
