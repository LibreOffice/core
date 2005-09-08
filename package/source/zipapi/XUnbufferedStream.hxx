/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XUnbufferedStream.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:14:59 $
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
#ifndef _XUNBUFFERED_STREAM_HXX
#define _XUNBUFFERED_STREAM_HXX

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _VOS_REF_H_
#include <vos/ref.hxx>
#endif
#ifndef _INFLATER_HXX
#include <Inflater.hxx>
#endif
#ifndef _ZIP_ENTRY_HXX_
#include <ZipEntry.hxx>
#endif
#ifndef _CRC32_HXX_
#include <CRC32.hxx>
#endif

#define UNBUFF_STREAM_DATA          0
#define UNBUFF_STREAM_RAW           1
#define UNBUFF_STREAM_WRAPPEDRAW    2

class EncryptionData;
typedef void* rtlCipher;
class XUnbufferedStream : public cppu::WeakImplHelper1
<
    com::sun::star::io::XInputStream
>
{
protected:
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > mxZipStream;
    com::sun::star::uno::Reference < com::sun::star::io::XSeekable > mxZipSeek;
    com::sun::star::uno::Sequence < sal_Int8 > maCompBuffer, maHeader;
    ZipEntry maEntry;
    vos::ORef < EncryptionData > mxData;
    rtlCipher maCipher;
    Inflater maInflater;
    sal_Bool mbRawStream, mbWrappedRaw, mbFinished;
    sal_Int16 mnHeaderToRead;
    sal_Int64 mnZipCurrent, mnZipEnd, mnZipSize, mnMyCurrent;
    CRC32 maCRC;
    sal_Bool mbCheckCRC;

public:
    XUnbufferedStream( ZipEntry & rEntry,
                 com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewZipStream,
                 const vos::ORef < EncryptionData > &rData,
                 sal_Int8 nStreamMode,
                 sal_Bool bIsEncrypted,
                 const ::rtl::OUString& aMediaType,
                 sal_Bool bRecoveryMode );

    // allows to read package raw stream
    XUnbufferedStream( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& xRawStream,
                 const vos::ORef < EncryptionData > &rData );


    virtual ~XUnbufferedStream();

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
    // XSeekable
    /*
    virtual void SAL_CALL seek( sal_Int64 location )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    */
};
#endif
