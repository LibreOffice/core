/*************************************************************************
 *
 *  $RCSfile: ocompinstream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-09-11 10:14:42 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#include "ocompinstream.hxx"

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include "owriteablestream.hxx"

using namespace ::com::sun::star;

//-----------------------------------------------
OInputCompStream::OInputCompStream( OWriteStream_Impl& aImpl )
: m_pImpl( &aImpl )
, m_rMutexRef( m_pImpl->m_rMutexRef )
, m_pInterfaceContainer( NULL )
{
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex is provided!\n" );
    if ( !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster
}

//-----------------------------------------------
OInputCompStream::OInputCompStream( OWriteStream_Impl& aImpl,
                                    uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps )
: m_pImpl( &aImpl )
, m_rMutexRef( m_pImpl->m_rMutexRef )
, m_pInterfaceContainer( NULL )
, m_xStream( xStream )
, m_aProperties( aProps )
{
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex is provided!\n" );
    if ( !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster

    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );
}

//-----------------------------------------------
OInputCompStream::~OInputCompStream()
{
    {
        ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

        if ( m_pImpl )
        {
            m_refCount++;
            dispose();
        }

        if ( m_pInterfaceContainer )
            delete m_pInterfaceContainer;
    }
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputCompStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    return m_xStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputCompStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    return m_xStream->readSomeBytes( aData, nMaxBytesToRead );

}

//-----------------------------------------------
void SAL_CALL OInputCompStream::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    m_xStream->skipBytes( nBytesToSkip );

}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputCompStream::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        throw uno::RuntimeException();

    return m_xStream->available();

}

//-----------------------------------------------
void SAL_CALL OInputCompStream::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    dispose();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OInputCompStream::getInputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xStream.is() )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OInputCompStream::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    return uno::Reference< io::XOutputStream >();
}

//-----------------------------------------------
void OInputCompStream::InternalDispose()
{
    // can be called only by OWriteStream_Impl
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject*>( this ) );
        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_xStream->closeInput();

    m_pImpl = NULL;
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::dispose(  )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject*>( this ) );
        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_xStream->closeInput();

    m_pImpl->InputStreamDisposed( this );
    m_pImpl = NULL;
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OInterfaceContainerHelper( m_pImpl->m_rMutexRef->GetMutex() );

    m_pInterfaceContainer->addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( xListener );
}

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OInputCompStream::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    throw beans::PropertyVetoException(); // TODO
}


//-----------------------------------------------
uno::Any SAL_CALL OInputCompStream::getPropertyValue( const ::rtl::OUString& aPropertyName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName.equalsAscii( "MediaType" )
      || aPropertyName.equalsAscii( "Size" )
      || aPropertyName.equalsAscii( "Encrypted" )
      || aPropertyName.equalsAscii( "Compressed" ) )
    {
        for ( sal_Int32 aInd = 0; aInd < m_aProperties.getLength(); aInd++ )
        {
            if ( m_aProperties[aInd].Name.equals( aPropertyName ) )
            {
                return m_aProperties[aInd].Value;
            }
        }
    }
    else if ( aPropertyName.equalsAscii( "EncriptionKey" ) )
        throw lang::WrappedTargetException(); // TODO: PropertyVetoException

    throw beans::UnknownPropertyException();

    // not reachable
    return uno::Any();
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::addPropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::removePropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const uno::Reference< beans::XPropertyChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::addVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::removeVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


