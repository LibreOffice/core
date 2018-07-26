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

#include <connectivity/dbcharset.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>


namespace dbtools
{

    OCharsetMap::OCharsetMap()
    {
    }


    void OCharsetMap::lateConstruct()
    {
        const rtl_TextEncoding eFirstEncoding = RTL_TEXTENCODING_DONTKNOW;
        const rtl_TextEncoding eLastEncoding = 100;     // TODO: a define in rtl/textenc.h would be fine here ...
        OSL_ENSURE( 0 == eFirstEncoding, "OCharsetMap::OCharsetMap: somebody changed the numbers!" );

        rtl_TextEncodingInfo aInfo; aInfo.StructSize = sizeof( rtl_TextEncodingInfo );
        for ( rtl_TextEncoding eEncoding = eFirstEncoding; eEncoding < eLastEncoding; ++eEncoding )
        {
            if  (   ( RTL_TEXTENCODING_DONTKNOW == eEncoding )  // this is always allowed - it has the special meaning "system encoding"
                ||  (   rtl_getTextEncodingInfo( eEncoding, &aInfo )
                    &&  approveEncoding( eEncoding, aInfo )
                    )
                )
            {
                m_aEncodings.insert( eEncoding );
            }
        }

        OSL_ENSURE( find( RTL_TEXTENCODING_MS_1252 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding ANSI!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_APPLE_ROMAN ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding macintosh!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_437 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM437!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_850) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM850!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_860 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM860!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_861 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM861!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_863 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM863!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_865 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM865!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_IBM_866 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding IBM866!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_DONTKNOW ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding SYSTEM!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_UTF8 ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding UTF-8!" );
        OSL_ENSURE( find( RTL_TEXTENCODING_BIG5_HKSCS ) != end(), "OCharsetMap::lateConstruct: missing compatibility encoding Big5-HKSCS!" );
    }


    bool OCharsetMap::approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const
    {
        bool bIsMimeEncoding = 0 != ( _rInfo.Flags & RTL_TEXTENCODING_INFO_MIME );
        OSL_ENSURE( !bIsMimeEncoding || rtl_getMimeCharsetFromTextEncoding( _eEncoding ),
                "OCharsetMap::OCharsetMap: inconsistence in rtl!" );
        return bIsMimeEncoding;
    }


    OCharsetMap::~OCharsetMap()
    {
    }


    OCharsetMap::CharsetIterator OCharsetMap::begin() const
    {
        ensureConstructed( );
        return CharsetIterator(this, m_aEncodings.begin() );
    }


    OCharsetMap::CharsetIterator    OCharsetMap::find(const rtl_TextEncoding _eEncoding) const
    {
        ensureConstructed( );
        return CharsetIterator( this, m_aEncodings.find( _eEncoding ) );
    }


    OCharsetMap::CharsetIterator    OCharsetMap::findIanaName(const OUString& _rIanaName) const
    {
        ensureConstructed( );

        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
        if ( !_rIanaName.isEmpty() )
        {
            // byte string conversion
            OString sMimeByteString( _rIanaName.getStr(), _rIanaName.getLength(), RTL_TEXTENCODING_ASCII_US );
            // look up
            eEncoding = rtl_getTextEncodingFromMimeCharset( sMimeByteString.getStr() );

            if ( RTL_TEXTENCODING_DONTKNOW == eEncoding )
            {   // if we're here, the name is not empty, but unknown -> this is an invalid name
                return end();
            }
        }

        return find( eEncoding );
    }


    OCharsetMap::CharsetIterator OCharsetMap::end() const
    {
        ensureConstructed( );

        return CharsetIterator( this, m_aEncodings.end() );
    }


    CharsetIteratorDerefHelper::CharsetIteratorDerefHelper( const CharsetIteratorDerefHelper& _rSource )
        :m_eEncoding( _rSource.m_eEncoding )
        ,m_aIanaName( _rSource.m_aIanaName )
    {
    }


    CharsetIteratorDerefHelper:: CharsetIteratorDerefHelper(const rtl_TextEncoding _eEncoding, const OUString& _rIanaName )
        :m_eEncoding( _eEncoding )
        ,m_aIanaName( _rIanaName )
    {
    }

    OCharsetMap::CharsetIterator::CharsetIterator(const OCharsetMap* _pContainer, OCharsetMap::TextEncBag::const_iterator const & _aPos )
        :m_pContainer( _pContainer )
        ,m_aPos( _aPos )
    {
        OSL_ENSURE( m_pContainer, "OCharsetMap::CharsetIterator::CharsetIterator : invalid container!" );
    }

    CharsetIteratorDerefHelper OCharsetMap::CharsetIterator::operator*() const
    {
        OSL_ENSURE( m_aPos != m_pContainer->m_aEncodings.end(), "OCharsetMap::CharsetIterator::operator*: invalid position!");

        rtl_TextEncoding eEncoding = *m_aPos;
        OUString sIanaName;

        if ( RTL_TEXTENCODING_DONTKNOW != eEncoding )
        {   // it's not the virtual "system charset"
            const char* pIanaName = rtl_getMimeCharsetFromTextEncoding( eEncoding );
            OSL_ENSURE( pIanaName, "OCharsetMap::CharsetIterator: invalid mime name!" );
            if ( pIanaName )
                sIanaName = OUString::createFromAscii( pIanaName );
        }
        return CharsetIteratorDerefHelper( eEncoding, sIanaName );
    }


    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator++()
    {
        OSL_ENSURE( m_aPos != m_pContainer->m_aEncodings.end(), "OCharsetMap::CharsetIterator::operator++ : invalid position!" );
        if ( m_aPos != m_pContainer->m_aEncodings.end())
            ++m_aPos;
        return *this;
    }


    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator--()
    {
        OSL_ENSURE( m_aPos != m_pContainer->m_aEncodings.begin(), "OCharsetMap::CharsetIterator::operator-- : invalid position!" );
        if ( m_aPos != m_pContainer->m_aEncodings.begin() )
            --m_aPos;
        return *this;
    }


    bool operator==(const OCharsetMap::CharsetIterator& lhs, const OCharsetMap::CharsetIterator& rhs)
    {
        return ( lhs.m_pContainer == rhs.m_pContainer ) && ( lhs.m_aPos == rhs.m_aPos );
    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
