/*************************************************************************
 *
 *  $RCSfile: dbcharset.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCHARSET_HXX_
#include <connectivity/dbcharset.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=========================================================================
    //= OCharsetMap
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetMap::OCharsetMap()
    {
    }

    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    sal_Bool OCharsetMap::approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const
    {
        sal_Bool bIsMimeEncoding = 0 != ( _rInfo.Flags & RTL_TEXTENCODING_INFO_MIME );
        OSL_ENSURE( !bIsMimeEncoding || rtl_getMimeCharsetFromTextEncoding( _eEncoding ),
                "OCharsetMap::OCharsetMap: inconsistence in rtl!" );
        return bIsMimeEncoding;
    }

    //-------------------------------------------------------------------------
    OCharsetMap::~OCharsetMap()
    {
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator OCharsetMap::begin() const
    {
        ensureConstructed( );
        return CharsetIterator(this, m_aEncodings.begin() );
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator    OCharsetMap::find(const rtl_TextEncoding _eEncoding) const
    {
        ensureConstructed( );
        return CharsetIterator( this, m_aEncodings.find( _eEncoding ) );
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator    OCharsetMap::find(const ::rtl::OUString& _rIanaName, const IANA&) const
    {
        ensureConstructed( );

        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
        if ( _rIanaName.getLength() )
        {
            // byte string conversion
            ::rtl::OString sMimeByteString( _rIanaName.getStr(), _rIanaName.getLength(), RTL_TEXTENCODING_ASCII_US );
            // look up
            eEncoding = rtl_getTextEncodingFromMimeCharset( sMimeByteString.getStr() );

            if ( RTL_TEXTENCODING_DONTKNOW == eEncoding )
            {   // if we're here, the name is not empty, but unknown -> this is an invalid name
                return end();
            }
        }

        return find( eEncoding );
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator OCharsetMap::end() const
    {
        ensureConstructed( );

        return CharsetIterator( this, m_aEncodings.end() );
    }

    //=========================================================================
    //= CharsetIteratorDerefHelper
    //=========================================================================
    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper::CharsetIteratorDerefHelper( const CharsetIteratorDerefHelper& _rSource )
        :m_eEncoding( _rSource.m_eEncoding )
        ,m_aIanaName( _rSource.m_aIanaName )
    {
    }

    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper:: CharsetIteratorDerefHelper(const rtl_TextEncoding _eEncoding, const ::rtl::OUString& _rIanaName )
        :m_eEncoding( _eEncoding )
        ,m_aIanaName( _rIanaName )
    {
    }

    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper::CharsetIteratorDerefHelper()
        :m_eEncoding(RTL_TEXTENCODING_DONTKNOW)
    {
    }

    //=========================================================================
    //= OCharsetMap::CharsetIterator
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator::CharsetIterator(const OCharsetMap* _pContainer, OCharsetMap::TextEncBag::const_iterator _aPos )
        :m_pContainer( _pContainer )
        ,m_aPos( _aPos )
    {
        OSL_ENSURE( m_pContainer, "OCharsetMap::CharsetIterator::CharsetIterator : invalid container!" );
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator::CharsetIterator(const CharsetIterator& _rSource)
        :m_pContainer( _rSource.m_pContainer )
        ,m_aPos( _rSource.m_aPos )
    {
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator::~CharsetIterator()
    {
    }

    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper OCharsetMap::CharsetIterator::operator*() const
    {
        OSL_ENSURE( m_aPos != m_pContainer->m_aEncodings.end(), "OCharsetMap::CharsetIterator::operator*: invalid position!");

        rtl_TextEncoding eEncoding = *m_aPos;
        ::rtl::OUString sIanaName;

        if ( RTL_TEXTENCODING_DONTKNOW != eEncoding )
        {   // it's not the virtual "system charset"
            const char* pIanaName = rtl_getMimeCharsetFromTextEncoding( eEncoding );
            OSL_ENSURE( pIanaName, "OCharsetMap::CharsetIterator: invalid mime name!" );
            if ( pIanaName )
                sIanaName = ::rtl::OUString::createFromAscii( pIanaName );
        }
        return CharsetIteratorDerefHelper( eEncoding, sIanaName );
    }

    //-------------------------------------------------------------------------
    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator++()
    {
        OSL_ENSURE( m_aPos != m_pContainer->m_aEncodings.end(), "OCharsetMap::CharsetIterator::operator++ : invalid position!" );
        if ( m_aPos != m_pContainer->m_aEncodings.end())
            ++m_aPos;
        return *this;
    }

    //-------------------------------------------------------------------------
    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator--()
    {
        OSL_ENSURE( m_aPos != m_pContainer->m_aEncodings.begin(), "OCharsetMap::CharsetIterator::operator-- : invalid position!" );
        if ( m_aPos != m_pContainer->m_aEncodings.begin() )
            --m_aPos;
        return *this;
    }

    //-------------------------------------------------------------------------
    bool operator==(const OCharsetMap::CharsetIterator& lhs, const OCharsetMap::CharsetIterator& rhs)
    {
        return ( lhs.m_pContainer == rhs.m_pContainer ) && ( lhs.m_aPos == rhs.m_aPos );
    }

//.........................................................................
}   // namespace dbtools
//.........................................................................

