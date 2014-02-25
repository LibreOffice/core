/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucbhelper/std_outputstream.hxx"

using namespace std;
using namespace com::sun::star;

namespace ucbhelper
{
    StdOutputStream::StdOutputStream( boost::shared_ptr< ostream > pStream ) :
        m_pStream( pStream )
    {
    }

    StdOutputStream::~StdOutputStream()
    {
        if ( m_pStream.get( ) )
            m_pStream->setstate( ios::eofbit );
    }

    uno::Any SAL_CALL StdOutputStream::queryInterface( const uno::Type& rType ) throw ( uno::RuntimeException, std::exception )
    {
        uno::Any aRet = ::cppu::queryInterface( rType, ( static_cast< XOutputStream* >( this ) ) );

        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    void SAL_CALL StdOutputStream::acquire( ) throw( )
    {
        OWeakObject::acquire();
    }

    void SAL_CALL StdOutputStream::release( ) throw( )
    {
        OWeakObject::release();
    }

    void SAL_CALL StdOutputStream::writeBytes ( const uno::Sequence< sal_Int8 >& aData )
        throw ( io::NotConnectedException, io::BufferSizeExceededException,
                io::IOException, uno::RuntimeException, std::exception )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pStream.get() )
            throw io::IOException( );

        try
        {
            m_pStream->write( reinterpret_cast< const char* >( aData.getConstArray( ) ), aData.getLength( ) );
        }
        catch ( const ios_base::failure& e )
        {
            SAL_INFO( "ucbhelper", "Exception caught when calling write: " << e.what() );
            throw io::IOException( );
        }
    }

    void SAL_CALL StdOutputStream::flush ( )
        throw ( io::NotConnectedException, io::BufferSizeExceededException,
                io::IOException, uno::RuntimeException, std::exception )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pStream.get() )
            throw io::IOException( );

        try
        {
            m_pStream->flush( );
        }
        catch ( const ios_base::failure& e )
        {
            SAL_INFO( "ucbhelper", "Exception caught when calling flush: " << e.what() );
            throw io::IOException( );
        }
    }

    void SAL_CALL StdOutputStream::closeOutput ( )
        throw ( io::NotConnectedException, io::BufferSizeExceededException,
                io::IOException, uno::RuntimeException, std::exception )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pStream.get() )
            throw io::IOException( );

        m_pStream->setstate( ios_base::eofbit );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
