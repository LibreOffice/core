/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomemorystream.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:06:13 $
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





