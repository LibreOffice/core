/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _FILSTR_HXX_
#define _FILSTR_HXX_

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"
#include <com/sun/star/ucb/XContentProvider.hpp>

#include "filrec.hxx"

namespace fileaccess {

    // forward:
    class shell;
    class XInputStreamForStream;
    class XOutputStreamForStream;

    class XStream_impl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::io::XStream,
          public com::sun::star::io::XSeekable,
          public com::sun::star::io::XInputStream,
          public com::sun::star::io::XOutputStream,
          public com::sun::star::io::XTruncate,
          public com::sun::star::io::XAsyncOutputMonitor
    {
        friend class XInputStreamForStream;
        friend class XOutputStreamForStream;

    public:

        XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath, sal_Bool bLock );

        /**
         *  Returns an error code as given by filerror.hxx
         */

        sal_Int32 SAL_CALL CtorSuccess();
        sal_Int32 SAL_CALL getMinorError();

        virtual ~XStream_impl();


        // OWeakObject

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        // XStream

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getInputStream(  )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL
        getOutputStream(  )
            throw( com::sun::star::uno::RuntimeException );


        // XTruncate

        virtual void SAL_CALL truncate( void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );


        // XInputStream

        sal_Int32 SAL_CALL
        readBytes(
            com::sun::star::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);

        sal_Int32 SAL_CALL
        readSomeBytes(
            com::sun::star::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);


        void SAL_CALL
        skipBytes(
            sal_Int32 nBytesToSkip )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int32 SAL_CALL
        available(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        void SAL_CALL
        closeInput(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        // XSeekable

        void SAL_CALL
        seek(
            sal_Int64 location )
            throw( com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL
        getLength(
            void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );


        // XOutputStream

        void SAL_CALL
        writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);



        void SAL_CALL
        flush()
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);


        void SAL_CALL
        closeOutput(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL waitForCompletion()
            throw (
                com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException);

    private:

        osl::Mutex   m_aMutex;
        bool         m_bInputStreamCalled,m_bOutputStreamCalled;

        shell*       m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
        sal_Bool     m_nIsOpen;

        sal_Bool     m_bLock;

        ReconnectingFile    m_aFile;

        sal_Int32                                          m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Implementation methods

        void SAL_CALL
        closeStream(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

    };

}  // end namespace XStream_impl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
