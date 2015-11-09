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

        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
        getInputStream(  )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL
        getOutputStream(  )
            throw( css::uno::RuntimeException, std::exception ) override;


        // XTruncate

        virtual void SAL_CALL truncate()
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;


        // XInputStream

        sal_Int32 SAL_CALL
        readBytes(
            css::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception) override;

        sal_Int32 SAL_CALL
        readSomeBytes(
            css::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception) override;


        void SAL_CALL
        skipBytes(
            sal_Int32 nBytesToSkip )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        sal_Int32 SAL_CALL
        available(
            void )
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        void SAL_CALL
        closeInput(
            void )
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        // XSeekable

        void SAL_CALL
        seek(
            sal_Int64 location )
            throw( css::lang::IllegalArgumentException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        sal_Int64 SAL_CALL
        getLength(
            void )
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;


        // XOutputStream

        void SAL_CALL
        writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception) override;



        void SAL_CALL
        flush()
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception) override;


        void SAL_CALL
        closeOutput(
            void )
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL waitForCompletion()
            throw (
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    private:

        osl::Mutex   m_aMutex;
        bool         m_bInputStreamCalled,m_bOutputStreamCalled;

        shell*       m_pMyShell;
        css::uno::Reference< css::ucb::XContentProvider > m_xProvider;
        bool     m_nIsOpen;

        ReconnectingFile    m_aFile;

        sal_Int32                                          m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Implementation methods

        void SAL_CALL
        closeStream(
            void )
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException );

    };

}  // end namespace XStream_impl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
