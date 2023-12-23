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

#include "FontTable.hxx"
#include <o3tl/deleter.hxx>
#include <ooxml/resourceids.hxx>
#include <utility>
#include <vector>
#include <sal/log.hxx>
#include <rtl/tencinfo.h>
#include <vcl/embeddedfontshelper.hxx>
#include <unotools/fontdefs.hxx>

using namespace com::sun::star;

namespace writerfilter::dmapper
{

struct FontTable_Impl
{
    std::unique_ptr<EmbeddedFontsHelper, o3tl::default_delete<EmbeddedFontsHelper>> xEmbeddedFontHelper;
    std::vector< FontEntry::Pointer_t > aFontEntries;
    FontEntry::Pointer_t pCurrentEntry;
    FontTable_Impl() {}
};

FontTable::FontTable()
: LoggedProperties("FontTable")
, LoggedTable("FontTable")
, LoggedStream("FontTable")
, m_pImpl( new FontTable_Impl )
{
}

FontTable::~FontTable()
{
}

void FontTable::lcl_attribute(Id Name, Value & val)
{
    SAL_WARN_IF( !m_pImpl->pCurrentEntry, "writerfilter.dmapper", "current entry has to be set here" );
    if(!m_pImpl->pCurrentEntry)
        return ;
    int nIntValue = val.getInt();
    OUString sValue = val.getString();
    switch(Name)
    {
        case NS_ooxml::LN_CT_Pitch_val:
            if (static_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Pitch_fixed)
                ;
            else if (static_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Pitch_variable)
                ;
            else if (static_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Pitch_default)
                ;
            else
                SAL_WARN("writerfilter.dmapper", "FontTable::lcl_attribute: unhandled NS_ooxml::CT_Pitch_val: " << nIntValue);
            break;
        case NS_ooxml::LN_CT_Font_name:
            m_pImpl->pCurrentEntry->sFontName = sValue;
            break;
        case NS_ooxml::LN_CT_Charset_val:
            // w:characterSet has higher priority, set only if that one is not set
            if( m_pImpl->pCurrentEntry->nTextEncoding == RTL_TEXTENCODING_DONTKNOW )
            {
                m_pImpl->pCurrentEntry->nTextEncoding = rtl_getTextEncodingFromWindowsCharset( nIntValue );
                if( IsOpenSymbol( m_pImpl->pCurrentEntry->sFontName ))
                    m_pImpl->pCurrentEntry->nTextEncoding = RTL_TEXTENCODING_SYMBOL;
            }
            break;
        case NS_ooxml::LN_CT_Charset_characterSet:
        {
            OString tmp;
            sValue.convertToString( &tmp, RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );
            m_pImpl->pCurrentEntry->nTextEncoding = rtl_getTextEncodingFromMimeCharset( tmp.getStr() );
            // Older LO versions used to write incorrect character set for OpenSymbol, fix.
            if( IsOpenSymbol( m_pImpl->pCurrentEntry->sFontName ))
                m_pImpl->pCurrentEntry->nTextEncoding = RTL_TEXTENCODING_SYMBOL;
            break;
        }
        default: ;
    }
}

void FontTable::lcl_sprm(Sprm& rSprm)
{
    SAL_WARN_IF( !m_pImpl->pCurrentEntry, "writerfilter.dmapper", "current entry has to be set here" );
    if(!m_pImpl->pCurrentEntry)
        return ;
    sal_uInt32 nSprmId = rSprm.getId();

    switch(nSprmId)
    {
        case NS_ooxml::LN_CT_Font_charset:
        case NS_ooxml::LN_CT_Font_pitch:
            resolveSprm( rSprm );
            break;
        case NS_ooxml::LN_CT_Font_embedRegular:
        case NS_ooxml::LN_CT_Font_embedBold:
        case NS_ooxml::LN_CT_Font_embedItalic:
        case NS_ooxml::LN_CT_Font_embedBoldItalic:
        {
            writerfilter::Reference< Properties >::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                EmbeddedFontHandler handler(*this, m_pImpl->pCurrentEntry->sFontName,
                    nSprmId == NS_ooxml::LN_CT_Font_embedRegular ? u""
                    : nSprmId == NS_ooxml::LN_CT_Font_embedBold ? u"b"
                    : nSprmId == NS_ooxml::LN_CT_Font_embedItalic ? u"i"
                    : /*NS_ooxml::LN_CT_Font_embedBoldItalic*/ u"bi" );
                pProperties->resolve( handler );
            }
            break;
        }
        case NS_ooxml::LN_CT_Font_altName:
            break;
        case NS_ooxml::LN_CT_Font_panose1:
            break;
        case NS_ooxml::LN_CT_Font_family:
            break;
        case NS_ooxml::LN_CT_Font_sig:
            break;
        case NS_ooxml::LN_CT_Font_notTrueType:
            break;
        default:
            SAL_WARN("writerfilter.dmapper", "FontTable::lcl_sprm: unhandled token: " << nSprmId);
            break;
    }
}

void FontTable::resolveSprm(Sprm & r_Sprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = r_Sprm.getProps();
    if( pProperties )
        pProperties->resolve(*this);
}

void FontTable::lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& ref)
{
    //create a new font entry
    SAL_WARN_IF( m_pImpl->pCurrentEntry, "writerfilter.dmapper", "current entry has to be NULL here" );
    m_pImpl->pCurrentEntry = new FontEntry;
    ref->resolve(*this);
    //append it to the table
    m_pImpl->aFontEntries.push_back( m_pImpl->pCurrentEntry );
    m_pImpl->pCurrentEntry.clear();
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

void FontTable::lcl_utext(const sal_Unicode*, size_t)
{
}

void FontTable::lcl_props(const writerfilter::Reference<Properties>::Pointer_t&)
{
}

void FontTable::lcl_table(Id, const writerfilter::Reference<Table>::Pointer_t&)
{
}

void FontTable::lcl_substream(Id, const writerfilter::Reference<Stream>::Pointer_t&)
{
}

void FontTable::lcl_startShape(uno::Reference<drawing::XShape> const&)
{
}

void FontTable::lcl_endShape( )
{
}

FontEntry::Pointer_t FontTable::getFontEntry(sal_uInt32 nIndex)
{
    return (m_pImpl->aFontEntries.size() > nIndex)
        ?   m_pImpl->aFontEntries[nIndex]
        :   FontEntry::Pointer_t();
}

sal_uInt32 FontTable::size()
{
    return m_pImpl->aFontEntries.size();
}

void FontTable::addEmbeddedFont(const css::uno::Reference<css::io::XInputStream>& stream,
                                const OUString& fontName, std::u16string_view extra,
                                std::vector<unsigned char> const & key)
{
    if (!m_pImpl->xEmbeddedFontHelper)
        m_pImpl->xEmbeddedFontHelper.reset(new EmbeddedFontsHelper);
    m_pImpl->xEmbeddedFontHelper->addEmbeddedFont(stream, fontName, extra, key);
}

EmbeddedFontHandler::EmbeddedFontHandler(FontTable& rFontTable, OUString _fontName, std::u16string_view style )
: LoggedProperties("EmbeddedFontHandler")
, m_fontTable( rFontTable )
, m_fontName(std::move( _fontName ))
, m_style( style )
{
}

EmbeddedFontHandler::~EmbeddedFontHandler()
{
    if( !m_inputStream.is())
        return;
    std::vector< unsigned char > key( 32 );
    if( !m_fontKey.isEmpty())
    {   // key for unobfuscating
        //  1 3 5 7 10 2  5 7 20 2  5 7 9 1 3 5
        // {62E79491-959F-41E9-B76B-6B32631DEA5C}
        static const int pos[ 16 ] = { 35, 33, 31, 29, 27, 25, 22, 20, 17, 15, 12, 10, 7, 5, 3, 1 };
        for( int i = 0;
             i < 16;
             ++i )
        {
            int v1 = m_fontKey[ pos[ i ]];
            int v2 = m_fontKey[ pos[ i ] + 1 ];
            assert(( v1 >= '0' && v1 <= '9' ) || ( v1 >= 'A' && v1 <= 'F' ));
            assert(( v2 >= '0' && v2 <= '9' ) || ( v2 >= 'A' && v2 <= 'F' ));
            int val = ( v1 - ( v1 <= '9' ? '0' : 'A' - 10 )) * 16 + v2 - ( v2 <= '9' ? '0' : 'A' - 10 );
            key[ i ] = val;
            key[ i + 16 ] = val;
        }
    }
    m_fontTable.addEmbeddedFont( m_inputStream, m_fontName, m_style, key );
    m_inputStream->closeInput();
}

void EmbeddedFontHandler::lcl_attribute( Id name, Value& val )
{
    switch( name )
    {
        case NS_ooxml::LN_CT_FontRel_fontKey:
            m_fontKey = val.getString();
            break;
        case NS_ooxml::LN_CT_Rel_id:
            break;
        case NS_ooxml::LN_CT_FontRel_subsetted:
            break; // TODO? Let's just ignore this for now and hope
                   // it doesn't break anything.
        case NS_ooxml::LN_inputstream: // the actual font data as stream
            val.getAny() >>= m_inputStream;
            break;
        default:
            break;
    }
}

void EmbeddedFontHandler::lcl_sprm( Sprm& )
{
}


}//namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
