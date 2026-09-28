/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/bytereader.hxx>
#include <cppuhelper/implbase.hxx>
#include <mutex>

#include "filrec.hxx"

enum class TaskHandlerErr;

namespace fileaccess {

class XStream_impl :  public cppu::WeakImplHelper<
    css::io::XStream,
    css::io::XSeekable,
    css::io::XInputStream,
    css::io::XOutputStream,
    css::io::XTruncate,
    css::io::XAsyncOutputMonitor >,
    public comphelper::ByteReader
    {

    public:

        XStream_impl( const OUString& aUncPath, bool bLock );

        /**
         *  Returns an error code as given by filerror.hxx
         */

        TaskHandlerErr CtorSuccess() const { return m_nErrorCode;}
        sal_Int32 getMinorError() const { return m_nMinorErrorCode;}

        virtual ~XStream_impl() override;

        // XStream

        virtual cpo::uno::Reference< css::io::XInputStream >
        getInputStream() override;

        virtual cpo::uno::Reference< css::io::XOutputStream >
        getOutputStream() override;


        // XTruncate

        virtual void truncate() override;


        // XInputStream

        sal_Int32
        readBytes(
            cpo::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead ) override;

        sal_Int32
        readSomeBytes(
            cpo::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead ) override;


        void
        skipBytes( sal_Int32 nBytesToSkip ) override;

        sal_Int32
        available() override;

        void
        closeInput() override;

        // XSeekable

        void
        seek( sal_Int64 location ) override;

        sal_Int64
        getPosition() override;

        sal_Int64
        getLength() override;


        // XOutputStream

        void
        writeBytes( const cpo::uno::Sequence< sal_Int8 >& aData ) override;


        void
        flush() override;


        void
        closeOutput() override;

        virtual void waitForCompletion() override;

        // utl::ByteReader
        virtual sal_Int32
        readSomeBytes(
            sal_Int8* aData,
            sal_Int32 nMaxBytesToRead ) override;

    private:

        std::mutex   m_aMutex;
        bool         m_bInputStreamCalled,m_bOutputStreamCalled;
        bool         m_nIsOpen;

        ReconnectingFile    m_aFile;

        TaskHandlerErr                                     m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Implementation methods

        /// @throws css::io::NotConnectedException
        /// @throws css::io::IOException
        /// @throws cpo::uno::RuntimeException
        void
        closeStream();

    };

}  // end namespace XStream_impl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
