/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unomemorystream.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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





