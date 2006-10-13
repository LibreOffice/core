/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oseekinstream.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:49:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include "oseekinstream.hxx"
#include "owriteablestream.hxx"

using namespace ::com::sun::star;

OInputSeekStream::OInputSeekStream( OWriteStream_Impl& pImpl,
                                    uno::Reference < io::XStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int16 nStorageType )
: OInputCompStream( pImpl, nStorageType )
{
    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );

    if ( xStream.is() )
    {
        m_xStream = xStream->getInputStream();
        m_xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );

        OSL_ENSURE( m_xStream.is(), "No input stream is provided!\n" );
        OSL_ENSURE( m_xSeekable.is(), "No seeking support!\n" );
    }

    m_aProperties = aProps;
}

OInputSeekStream::OInputSeekStream( OWriteStream_Impl& pImpl,
                                    uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int16 nStorageType )
: OInputCompStream( pImpl, xStream, aProps, nStorageType )
{
    if ( m_xStream.is() )
    {
        m_xSeekable = uno::Reference< io::XSeekable >( m_xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xSeekable.is(), "No seeking support!\n" );
    }
}

OInputSeekStream::OInputSeekStream( uno::Reference < io::XStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int16 nStorageType )
: OInputCompStream( nStorageType )
{
    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );

    if ( xStream.is() )
    {
        m_xStream = xStream->getInputStream();
        m_xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );

        OSL_ENSURE( m_xStream.is(), "No input stream is provided!\n" );
        OSL_ENSURE( m_xSeekable.is(), "No seeking support!\n" );
    }

    m_aProperties = aProps;
}

OInputSeekStream::OInputSeekStream( uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int16 nStorageType )
: OInputCompStream( xStream, aProps, nStorageType )
{
    if ( m_xStream.is() )
    {
        m_xSeekable = uno::Reference< io::XSeekable >( m_xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xSeekable.is(), "No seeking support!\n" );
    }
}

OInputSeekStream::~OInputSeekStream()
{
}

uno::Sequence< uno::Type > SAL_CALL OInputSeekStream::getTypes()
        throw ( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

        if ( pTypeCollection == NULL )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                    ::getCppuType(( const uno::Reference< io::XSeekable >* )NULL ),
                    OInputCompStream::getTypes() );

            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

uno::Any SAL_CALL OInputSeekStream::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    uno::Any aReturn( ::cppu::queryInterface( rType,
                                           static_cast< io::XSeekable* >( this ) ) );

    if ( aReturn.hasValue() == sal_True )
    {
        return aReturn ;
    }

    return OInputCompStream::queryInterface( rType ) ;
}

void SAL_CALL OInputSeekStream::acquire()
        throw()
{
    OInputCompStream::acquire();
}

void SAL_CALL OInputSeekStream::release()
        throw()
{
    OInputCompStream::release();
}


void SAL_CALL OInputSeekStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

sal_Int64 SAL_CALL OInputSeekStream::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL OInputSeekStream::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

