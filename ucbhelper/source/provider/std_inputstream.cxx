/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucbhelper/std_inputstream.hxx"

using namespace std;
using namespace com::sun::star;

namespace ucbhelper
{
    StdInputStream::StdInputStream( boost::shared_ptr< istream > pStream ) :
        m_pStream( pStream ),
        m_nLength( 0 )
    {
        if ( m_pStream.get() )
        {
            streampos nInitPos = m_pStream->tellg( );
            m_pStream->seekg( 0, ios_base::end );
            streampos nEndPos = m_pStream->tellg( );
            m_pStream->seekg( nInitPos, ios_base::beg );

            m_nLength = sal_Int64( nEndPos - nInitPos );
        }
    }

    StdInputStream::~StdInputStream()
    {
    }

    uno::Any SAL_CALL StdInputStream::queryInterface( const uno::Type& rType ) throw ( uno::RuntimeException, std::exception )
    {
        uno::Any aRet = ::cppu::queryInterface( rType,
                                          ( static_cast< XInputStream* >( this ) ),
                                          ( static_cast< XSeekable* >( this ) ) );

        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    void SAL_CALL StdInputStream::acquire( ) throw( )
    {
        OWeakObject::acquire();
    }

    void SAL_CALL StdInputStream::release( ) throw( )
    {
        OWeakObject::release();
    }

    sal_Int32 SAL_CALL StdInputStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( io::NotConnectedException, io::BufferSizeExceededException,
               io::IOException, uno::RuntimeException, std::exception)
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( 0 <= nBytesToRead && aData.getLength() < nBytesToRead )
            aData.realloc( nBytesToRead );

        if ( !m_pStream.get() )
            throw io::IOException( );

        sal_Int32 nRead = 0;
        try
        {
            m_pStream->read( reinterpret_cast< char* >( aData.getArray( ) ), nBytesToRead );
            nRead = m_pStream->gcount();
        }
        catch ( const ios_base::failure& e )
        {
            SAL_INFO( "ucbhelper", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }

        return nRead;
    }

    sal_Int32 SAL_CALL StdInputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
        throw( io::NotConnectedException, io::BufferSizeExceededException,
               io::IOException, uno::RuntimeException, std::exception)
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( 0 <= nMaxBytesToRead && aData.getLength() < nMaxBytesToRead )
            aData.realloc( nMaxBytesToRead );

        if ( !m_pStream.get() )
            throw io::IOException( );

        sal_Int32 nRead = 0;
        try
        {
            nRead = m_pStream->readsome( reinterpret_cast< char* >( aData.getArray( ) ), nMaxBytesToRead );
        }
        catch ( const ios_base::failure& e )
        {
            SAL_INFO( "ucbhelper", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }
        return nRead;
    }

    void SAL_CALL StdInputStream::skipBytes( sal_Int32 nBytesToSkip )
        throw( io::NotConnectedException, io::BufferSizeExceededException,
               io::IOException, uno::RuntimeException, std::exception )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pStream.get() )
            throw io::IOException( );

        try
        {
            m_pStream->seekg( nBytesToSkip, ios_base::cur );
        }
        catch ( const ios_base::failure& e )
        {
            SAL_INFO( "ucbhelper", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }
    }

    sal_Int32 SAL_CALL StdInputStream::available( )
        throw(io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception )
    {
        return sal::static_int_cast< sal_Int32 >( m_nLength - getPosition() );
    }

    void SAL_CALL StdInputStream::closeInput( )
        throw( io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception)
    {
        // No need to implement this for an istream
    }

    void SAL_CALL StdInputStream::seek( sal_Int64 location )
        throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException, std::exception )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( location < 0 || location > m_nLength )
            throw lang::IllegalArgumentException(
                    "Location can't be negative or greater than the length",
                    static_cast< cppu::OWeakObject* >( this ), 0 );

        if ( !m_pStream.get() )
            throw io::IOException( );

        try
        {
            m_pStream->clear( ); // may be needed to rewind the stream
            m_pStream->seekg( location, ios_base::beg );
        }
        catch ( const ios_base::failure& e )
        {
            SAL_INFO( "ucbhelper", "StdInputStream::readBytes() error: " << e.what() );
            throw io::IOException( );
        }
    }

    sal_Int64 SAL_CALL StdInputStream::getPosition( )
        throw( io::IOException, uno::RuntimeException, std::exception )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pStream.get() )
            throw io::IOException( );

        sal_Int64 nPos = m_pStream->tellg( );
        if ( -1 == nPos )
            throw io::IOException( );

        return nPos;
    }

    sal_Int64 SAL_CALL StdInputStream::getLength( )
        throw ( io::IOException, uno::RuntimeException, std::exception )
    {
        return m_nLength;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
