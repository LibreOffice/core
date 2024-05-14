/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sal/log.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <utility>

#include "std_inputstream.hxx"

using namespace com::sun::star;

namespace cmis
{
    StdInputStream::StdInputStream( boost::shared_ptr< std::istream > pStream ) :
        m_pStream(std::move( pStream )),
        m_nLength( 0 )
    {
        if (m_pStream)
        {
            auto nInitPos = m_pStream->tellg( );
            m_pStream->seekg( 0, std::ios_base::end );
            auto nEndPos = m_pStream->tellg( );
            m_pStream->seekg( nInitPos, std::ios_base::beg );

            m_nLength = sal_Int64( nEndPos - nInitPos );
        }
    }

    StdInputStream::~StdInputStream()
    {
    }

    uno::Any SAL_CALL StdInputStream::queryInterface( const uno::Type& rType )
    {
        uno::Any aRet = ::cppu::queryInterface( rType,
                                          static_cast< XInputStream* >( this ),
                                          static_cast< XSeekable* >( this ) );

        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    void SAL_CALL StdInputStream::acquire( ) noexcept
    {
        OWeakObject::acquire();
    }

    void SAL_CALL StdInputStream::release( ) noexcept
    {
        OWeakObject::release();
    }

    sal_Int32 SAL_CALL StdInputStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    {
        std::scoped_lock aGuard( m_aMutex );

        if ( 0 <= nBytesToRead && aData.getLength() < nBytesToRead )
            aData.realloc( nBytesToRead );

        if (!m_pStream)
            throw io::IOException( );

        sal_Int32 nRead = 0;
        try
        {
            m_pStream->read( reinterpret_cast< char* >( aData.getArray( ) ), nBytesToRead );
            nRead = m_pStream->gcount();
        }
        catch ( const std::ios_base::failure& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }

        return nRead;
    }

    sal_Int32 SAL_CALL StdInputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
    {
        std::scoped_lock aGuard( m_aMutex );

        if ( 0 <= nMaxBytesToRead && aData.getLength() < nMaxBytesToRead )
            aData.realloc( nMaxBytesToRead );

        if (!m_pStream)
            throw io::IOException( );

        sal_Int32 nRead = 0;
        try
        {
            nRead = m_pStream->readsome( reinterpret_cast< char* >( aData.getArray( ) ), nMaxBytesToRead );
        }
        catch ( const std::ios_base::failure& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }
        return nRead;
    }

    void SAL_CALL StdInputStream::skipBytes( sal_Int32 nBytesToSkip )
    {
        std::scoped_lock aGuard( m_aMutex );

        if (!m_pStream)
            throw io::IOException( );

        try
        {
            m_pStream->seekg( nBytesToSkip, std::ios_base::cur );
        }
        catch ( const std::ios_base::failure& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }
    }

    sal_Int32 SAL_CALL StdInputStream::available( )
    {
        return std::min<sal_Int64>( SAL_MAX_INT32, m_nLength - getPosition() );
    }

    void SAL_CALL StdInputStream::closeInput( )
    {
        // No need to implement this for an istream
    }

    void SAL_CALL StdInputStream::seek( sal_Int64 location )
    {
        std::scoped_lock aGuard( m_aMutex );

        if ( location < 0 || location > m_nLength )
            throw lang::IllegalArgumentException(
                    u"Location can't be negative or greater than the length"_ustr,
                    getXWeak(), 0 );

        if (!m_pStream)
            throw io::IOException( );

        try
        {
            m_pStream->clear( ); // may be needed to rewind the stream
            m_pStream->seekg( location, std::ios_base::beg );
        }
        catch ( const std::ios_base::failure& e )
        {
            SAL_INFO( "ucb.ucp.cmis", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }
    }

    sal_Int64 SAL_CALL StdInputStream::getPosition( )
    {
        std::scoped_lock aGuard( m_aMutex );

        if (!m_pStream)
            throw io::IOException( );

        sal_Int64 nPos = m_pStream->tellg( );
        if ( -1 == nPos )
            throw io::IOException( );

        return nPos;
    }

    sal_Int64 SAL_CALL StdInputStream::getLength( )
    {
        return m_nLength;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
