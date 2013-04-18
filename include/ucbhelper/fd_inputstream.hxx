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

#ifndef _UCBHELPER_FD_INPUTSTREAM_HXX_
#define _UCBHELPER_FD_INPUTSTREAM_HXX_

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <stdio.h>

#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper
{
    /** Implements a seekable InputStream
     *  working on a buffer.
     */
    class UCBHELPER_DLLPUBLIC FdInputStream
        : public cppu::OWeakObject,
          public com::sun::star::io::XInputStream,
          public com::sun::star::io::XSeekable
    {
    public:

        /** Defines the storage kind found
         *  on which the inputstream acts.
         */

        FdInputStream(FILE* tmpfl = 0);

        ~FdInputStream();

        virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL acquire(void) throw();

        virtual void SAL_CALL release(void) throw();

        virtual sal_Int32 SAL_CALL
        readBytes(css::uno::Sequence< sal_Int8 >& aData,
                  sal_Int32 nBytesToRead)
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException);

        virtual sal_Int32 SAL_CALL
        readSomeBytes(css::uno::Sequence< sal_Int8 >& aData,
                      sal_Int32 nMaxBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException);

        virtual void SAL_CALL
        skipBytes(sal_Int32 nBytesToSkip)
            throw(css::io::NotConnectedException,
                  css::io::BufferSizeExceededException,
                  css::io::IOException,
                  css::uno::RuntimeException );

        virtual sal_Int32 SAL_CALL
        available(void)
            throw(css::io::NotConnectedException,
                  css::io::IOException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL
        closeInput(void)
            throw(css::io::NotConnectedException,
                  css::io::IOException,
                  css::uno::RuntimeException);


        /** XSeekable
         */

        virtual void SAL_CALL
        seek(sal_Int64 location)
            throw(css::lang::IllegalArgumentException,
                  css::io::IOException,
                  css::uno::RuntimeException);


        virtual sal_Int64 SAL_CALL
        getPosition(void)
            throw(css::io::IOException,
                  css::uno::RuntimeException);


        virtual sal_Int64 SAL_CALL
        getLength(void)
            throw(css::io::IOException,
                  css::uno::RuntimeException);

        // additional
//          void append(const void* pBuffer,size_t size,size_t nmemb);

    private:

        osl::Mutex m_aMutex;
        FILE* m_tmpfl;
        sal_Int64 m_nLength;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
