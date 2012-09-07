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
#include <osl/file.hxx>
#include <stdio.h>
#include <rtl/tencinfo.h>
#include <vcl/temporaryfonts.hxx>

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
        case NS_ooxml::LN_CT_Font_embedRegular:
        case NS_ooxml::LN_CT_Font_embedBold:
        case NS_ooxml::LN_CT_Font_embedItalic:
        case NS_ooxml::LN_CT_Font_embedBoldItalic:
        {
            writerfilter::Reference< Properties >::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get( ))
            {
                EmbeddedFontHandler handler( m_pImpl->pCurrentEntry->sFontName,
                    nSprmId == NS_ooxml::LN_CT_Font_embedRegular ? ""
                    : nSprmId == NS_ooxml::LN_CT_Font_embedBold ? "b"
                    : nSprmId == NS_ooxml::LN_CT_Font_embedItalic ? "i"
                    : nSprmId == NS_ooxml::LN_CT_Font_embedBoldItalic ? "bi" : "?" );
                pProperties->resolve( handler );
            }
            break;
        }
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

EmbeddedFontHandler::EmbeddedFontHandler( const OUString& _fontName, const char* _style )
: LoggedProperties(dmapper_logger, "EmbeddedFontHandler")
, fontName( _fontName )
, style( _style )
{
}

EmbeddedFontHandler::~EmbeddedFontHandler()
{
    if( !inputStream.is())
        return;
    OUString fileUrl = TemporaryFonts::fileUrlForFont( fontName, style );
    osl::File file( fileUrl );
    switch( file.open( osl_File_OpenFlag_Create | osl_File_OpenFlag_Write ))
    {
        case osl::File::E_None:
            break; // ok
        case osl::File::E_EXIST:
            return; // Assume it's already been added correctly.
        default:
            SAL_WARN( "writerfilter", "Cannot open file for temporary font" );
            inputStream->closeInput();
            return;
    }
    if( !fontKey.isEmpty())
    { // unobfuscate
        uno::Sequence< sal_Int8 > buffer;
        int read = inputStream->readBytes( buffer, 32 );
        if( read < 32 )
        {
            SAL_WARN( "writerfilter", "Embedded font too small" );
            inputStream->closeInput();
            file.close();
            osl::File::remove( fileUrl );
            return;
        }
        //  1 3 5 7 10 2  5 7 20 2  5 7 9 1 3 5
        // {62E79491-959F-41E9-B76B-6B32631DEA5C}
        static const int pos[ 16 ] = { 35, 33, 31, 29, 27, 25, 22, 20, 17, 15, 12, 10, 7, 5, 3, 1 };
        char key[ 16 ];
        for( int i = 0;
             i < 16;
             ++i )
        {
            int v1 = fontKey[ pos[ i ]];
            int v2 = fontKey[ pos[ i ] + 1 ];
            assert(( v1 >= '0' && v1 <= '9' ) || ( v1 >= 'A' && v1 <= 'F' ));
            assert(( v2 >= '0' && v2 <= '9' ) || ( v2 >= 'A' && v2 <= 'F' ));
            int val = ( v1 - ( v1 <= '9' ? '0' : 'A' - 10 )) * 16 + v2 - ( v2 <= '9' ? '0' : 'A' - 10 );
            key[ i ] = val;
        }
        for( int i = 0;
             i < 16;
             ++i )
        {
            buffer[ i ] ^= key[ i ];
            buffer[ i + 16 ] ^= key[ i ];
        }
        sal_uInt64 dummy;
        file.write( buffer.getConstArray(), 32, dummy );
    }
    for(;;)
    {
        uno::Sequence< sal_Int8 > buffer;
        int read = inputStream->readBytes( buffer, 1024 );
        sal_uInt64 dummy;
        if( read > 0 )
            file.write( buffer.getConstArray(), read, dummy );
        if( read < 1024 )
            break;
    }
    inputStream->closeInput();
    if( file.close() != osl::File::E_None )
    {
        SAL_WARN( "writerfilter", "Writing temporary font file failed" );
        osl::File::remove( fileUrl );
        return;
    }
    TemporaryFonts::activateFont( fontName, fileUrl );
}

void EmbeddedFontHandler::lcl_attribute( Id name, Value& val )
{
    OUString sValue = val.getString();
    switch( name )
    {
        case NS_ooxml::LN_CT_FontRel_fontKey:
            fontKey = sValue;
            break;
        case NS_ooxml::LN_CT_Rel_id:
            id = sValue;
            break;
        case NS_ooxml::LN_CT_FontRel_subsetted:
            break; // TODO? Let's just ignore this for now and hope
                   // it doesn't break anything.
        case NS_ooxml::LN_inputstream: // the actual font data as stream
            val.getAny() >>= inputStream;
            break;
        default:
            break;
    }
}

void EmbeddedFontHandler::lcl_sprm( Sprm& )
{
}


}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
