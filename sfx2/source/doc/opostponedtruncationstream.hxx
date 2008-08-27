/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: opostponedtruncationstream.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SFX_OPOSTPONEDTRUNCATIONFILESTREAM_HXX
#define _SFX_OPOSTPONEDTRUNCATIONFILESTREAM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase6.hxx>
#include "sfx2/dllapi.h"

//==================================================================
// OPostponedTruncationFileStream
//
// Allows to get stream access to a file, where the first truncation
// of the file is postponed till the first writing. If no writing happens
// after the first truncation/creation, it has no effect. ( The postponing of
// the creation can be switched off during initialization. Here the postponing
// of the creation means that the file will be created immediatelly, but
// if nothing is written into it, it will be removed during destruction
// of the object. )
//
// On creation of this object the target file is scheduled for
// creation/truncation. But the action happens only during the first
// write access. After the first write access the object behaves
// itself as the original stream.
//==================================================================

struct PTFStreamData_Impl;
class SFX2_DLLPUBLIC OPostponedTruncationFileStream
        : public ::cppu::WeakImplHelper6 <
                                        ::com::sun::star::io::XStream,
                                        ::com::sun::star::io::XInputStream,
                                        ::com::sun::star::io::XOutputStream,
                                        ::com::sun::star::io::XTruncate,
                                        ::com::sun::star::io::XSeekable,
                                        ::com::sun::star::io::XAsyncOutputMonitor >
{
    ::osl::Mutex    m_aMutex;
    PTFStreamData_Impl* m_pStreamData;

    void CloseAll_Impl();

    void CheckScheduledTruncation_Impl();

public:

    OPostponedTruncationFileStream(
        const ::rtl::OUString& aURL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream,
        sal_Bool bDelete );

    ~OPostponedTruncationFileStream();

// com::sun::star::io::XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL available(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XTruncate
    virtual void SAL_CALL truncate(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XSeekable
    virtual void SAL_CALL seek( ::sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::io::XAsyncOutputMonitor
    virtual void SAL_CALL waitForCompletion(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

};

#endif //_SFX_OPOSTPONEDTRUNCATIONFILESTREAM_HXX


