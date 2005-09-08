/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ByteChucker.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:54:18 $
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
#ifndef _BYTE_CHUCKER_HXX_
#define _BYTE_CHUCKER_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_IO_BUFFERSIZEEXCEEDEDEXCEPTION_HPP_
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_NOTCONNECTEDEXCEPTION_HPP_
#include <com/sun/star/io/NotConnectedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace io { class XSeekable; class XOutputStream; }
} } }
class ByteChucker
{
protected:
    com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > xStream;
    com::sun::star::uno::Reference < com::sun::star::io::XSeekable > xSeek;
    com::sun::star::uno::Sequence < sal_Int8 > a1Sequence, a2Sequence, a4Sequence;
    sal_Int8 * const p1Sequence, * const p2Sequence, * const p4Sequence;

public:
    ByteChucker (com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xOstream);
    ~ByteChucker();

    // XOutputStream
    void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nLength = -1, const sal_Int8 * const pData = NULL)
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL flush(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL closeOutput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XSeekable
    sal_Int64 SAL_CALL seek( sal_Int64 location )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    sal_Int64 SAL_CALL getPosition(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    sal_Int64 SAL_CALL getLength(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    ByteChucker& operator << (sal_Int8 nInt8);
    ByteChucker& operator << (sal_Int16 nInt16);
    ByteChucker& operator << (sal_Int32 nInt32);
    ByteChucker& operator << (sal_uInt8 nuInt8);
    ByteChucker& operator << (sal_uInt16 nuInt16);
    ByteChucker& operator << (sal_uInt32 nuInt32);
};

#endif
