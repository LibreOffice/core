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
#include "precompiled_toolkit.hxx"


#include <toolkit/helper/unomemorystream.hxx>
#include <algorithm>

//  ----------------------------------------------------
//  class UnoMemoryStream
//  ----------------------------------------------------
UnoMemoryStream::UnoMemoryStream( sal_uInt32 nInitSize, sal_uInt32 nInitResize )
    : SvMemoryStream( nInitSize, nInitResize )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoMemoryStream::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::io::XInputStream*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}


// ::com::sun::star::io::XInputStream
sal_Int32 UnoMemoryStream::readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nRead = available();
    if ( nRead > nBytesToRead )
        nRead = nBytesToRead;

    rData = ::com::sun::star::uno::Sequence< sal_Int8 >( nRead );
    Read( rData.getArray(), nRead );

    return nRead;
}

sal_Int32 UnoMemoryStream::readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nAvailable = available();
    if( nAvailable )
    {
        return readBytes( rData, std::min( nMaxBytesToRead , nAvailable ) );
    }
    else
    {
        // Not the most effective method, but it sticks to the specification
        return readBytes( rData, 1 );
    }
}

void UnoMemoryStream::skipBytes( sal_Int32 nBytesToSkip ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SeekRel( nBytesToSkip );
}

sal_Int32 UnoMemoryStream::available() throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nStreamPos = Tell();
    sal_uInt32 nEnd = Seek( STREAM_SEEK_TO_END );
    Seek( nStreamPos );
    return nEnd - nStreamPos;
}

void UnoMemoryStream::closeInput() throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // nothing to do
}





