/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILSTR_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILSTR_HXX

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <cppuhelper/implbase.hxx>

#include "filrec.hxx"

namespace fileaccess {

    // forward:
    class shell;

class XStream_impl :  public cppu::WeakImplHelper<
    css::io::XStream,
    css::io::XSeekable,
    css::io::XInputStream,
    css::io::XOutputStream,
    css::io::XTruncate,
    css::io::XAsyncOutputMonitor >
    {

    public:

        XStream_impl( shell* pMyShell,const OUString& aUncPath, bool bLock );

        /**
         *  Returns an error code as given by filerror.hxx
         */

        sal_Int32 SAL_CALL CtorSuccess() { return m_nErrorCode;}
        sal_Int32 SAL_CALL getMinorError() { return m_nMinorErrorCode;}

        virtual ~XStream_impl();

        // XStream

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getInputStream(  )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL
        getOutputStream(  )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;


        // XTruncate

        virtual void SAL_CALL truncate()
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;


        // XInputStream

        sal_Int32 SAL_CALL
        readBytes(
            com::sun::star::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception) override;

        sal_Int32 SAL_CALL
        readSomeBytes(
            com::sun::star::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception) override;


        void SAL_CALL
        skipBytes(
            sal_Int32 nBytesToSkip )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        sal_Int32 SAL_CALL
        available(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        void SAL_CALL
        closeInput(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        // XSeekable

        void SAL_CALL
        seek(
            sal_Int64 location )
            throw( com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        sal_Int64 SAL_CALL
        getLength(
            void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;


        // XOutputStream

        void SAL_CALL
        writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception) override;



        void SAL_CALL
        flush()
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception) override;


        void SAL_CALL
        closeOutput(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL waitForCompletion()
            throw (
                com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException, std::exception) override;

    private:

        osl::Mutex   m_aMutex;
        bool         m_bInputStreamCalled,m_bOutputStreamCalled;

        shell*       m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
        bool     m_nIsOpen;

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
