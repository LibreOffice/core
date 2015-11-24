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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_BUFFEREDINPUTSTREAM_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_BUFFEREDINPUTSTREAM_HXX

#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>


namespace chelp {

    class BufferedInputStream
        : public cppu::OWeakObject,
          public css::io::XInputStream,
          public css::io::XSeekable
    {
    private:

        sal_Int32      m_nBufferLocation;
        sal_Int32      m_nBufferSize;
        sal_Int8      *m_pBuffer;
        osl::Mutex     m_aMutex;

    public:

        explicit BufferedInputStream(
            const css::uno::Reference<css::io::XInputStream>& xInputStream);

        virtual ~BufferedInputStream();

        virtual css::uno::Any SAL_CALL
        queryInterface( const css::uno::Type& rType )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL acquire() throw() override;

        virtual void SAL_CALL release() throw() override;


        virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData,
                                              sal_Int32 nBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData,
                                                  sal_Int32 nMaxBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int32 SAL_CALL available()
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL closeInput()
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL seek( sal_Int64 location )
            throw( css::lang::IllegalArgumentException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int64 SAL_CALL getPosition()
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int64 SAL_CALL getLength()
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;
    };


    extern css::uno::Reference<css::io::XInputStream>
    turnToSeekable(
        const css::uno::Reference<css::io::XInputStream>& xInputStream);

}

#endif // INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_BUFFEREDINPUTSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
