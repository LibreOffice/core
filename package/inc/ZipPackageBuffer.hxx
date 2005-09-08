/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ZipPackageBuffer.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:59:39 $
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
#ifndef _ZIP_PACKAGE_BUFFER_HXX
#define _ZIP_PACKAGE_BUFFER_HXX

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX
#include <cppuhelper/implbase3.hxx>
#endif

class ZipPackage;

class ZipPackageBuffer : public ::cppu::WeakImplHelper3
<
    com::sun::star::io::XInputStream,
    com::sun::star::io::XOutputStream,
    com::sun::star::io::XSeekable
>
{
protected:
    com::sun::star::uno::Sequence < sal_Int8 > m_aBuffer;
    sal_Int64 m_nBufferSize, m_nEnd, m_nCurrent;
    sal_Bool m_bMustInitBuffer;
public:
    ZipPackageBuffer(sal_Int64 nNewBufferSize);
    ZipPackageBuffer( com::sun::star::uno::Sequence < sal_Int8 > &nNewBuffer );
    virtual ~ZipPackageBuffer(void);

    inline void realloc ( sal_Int32 nSize ) { m_aBuffer.realloc ( nSize ); }
    inline const sal_Int8 * getConstArray () const { return m_aBuffer.getConstArray(); }
    inline const com::sun::star::uno::Sequence < sal_Int8> & getSequence () const { return m_aBuffer; }

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};
#endif
