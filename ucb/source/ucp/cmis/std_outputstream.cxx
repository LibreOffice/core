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
#include <sal/log.hxx>

#include <std_outputstream.hxx>

using namespace std;
using namespace com::sun::star;

namespace cmis
{
    StdOutputStream::StdOutputStream( boost::shared_ptr< ostream > const & pStream ) :
        m_pStream( pStream )
    {
    }

    StdOutputStream::~StdOutputStream()
    {
        if ( m_pStream.get( ) )
            m_pStream->setstate( ios::eofbit );
    }

    uno::Any SAL_CALL StdOutputStream::queryInterface( const uno::Type& rType )
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
            SAL_INFO( "ucb.ucp.cmis", "Exception caught when calling write: " << e.what() );
            throw io::IOException( );
        }
    }

    void SAL_CALL StdOutputStream::flush ( )
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
            SAL_INFO( "ucb.ucp.cmis", "Exception caught when calling flush: " << e.what() );
            throw io::IOException( );
        }
    }

    void SAL_CALL StdOutputStream::closeOutput ( )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_pStream.get() )
            throw io::IOException( );

        m_pStream->setstate( ios_base::eofbit );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
