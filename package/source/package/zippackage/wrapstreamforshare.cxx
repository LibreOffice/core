/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#include <osl/diagnose.h>

#include "wrapstreamforshare.hxx"

using namespace ::com::sun::star;


WrapStreamForShare::WrapStreamForShare( const uno::Reference< io::XInputStream >& xInStream,
                                        const SotMutexHolderRef& rMutexRef )
: m_rMutexRef( rMutexRef )
, m_xInStream( xInStream )
, m_nCurPos( 0 )
{
    m_xSeekable = uno::Reference< io::XSeekable >( m_xInStream, uno::UNO_QUERY );
    if ( !m_rMutexRef.Is() || !m_xInStream.is() || !m_xSeekable.is() )
    {
        OSL_ENSURE( sal_False, "Wrong initialization of wrapping stream!\n" );
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    }
}

WrapStreamForShare::~WrapStreamForShare()
{
}

// XInputStream
sal_Int32 SAL_CALL WrapStreamForShare::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    m_xSeekable->seek( m_nCurPos );

    sal_Int32 nRead = m_xInStream->readBytes( aData, nBytesToRead );
    m_nCurPos += nRead;

    return nRead;
}

sal_Int32 SAL_CALL WrapStreamForShare::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    m_xSeekable->seek( m_nCurPos );

    sal_Int32 nRead = m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
    m_nCurPos += nRead;

    return nRead;
}

void SAL_CALL WrapStreamForShare::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    m_xSeekable->seek( m_nCurPos );

    m_xInStream->skipBytes( nBytesToSkip );
    m_nCurPos = m_xSeekable->getPosition();
}

sal_Int32 SAL_CALL WrapStreamForShare::available()
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return m_xInStream->available();
}

void SAL_CALL WrapStreamForShare::closeInput()
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    // the package is the owner so it will close the stream
    // m_xInStream->closeInput();
    m_xInStream = uno::Reference< io::XInputStream >();
    m_xSeekable = uno::Reference< io::XSeekable >();
}

// XSeekable
void SAL_CALL WrapStreamForShare::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    // let stream implementation do all the checking
    m_xSeekable->seek( location );

    m_nCurPos = m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL WrapStreamForShare::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return m_nCurPos;
}

sal_Int64 SAL_CALL WrapStreamForShare::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return m_xSeekable->getLength();
}

