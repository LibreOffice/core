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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_INPUTSTREAM_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_INPUTSTREAM_HXX

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>


namespace chelp {

    class XInputStream_impl
        : public cppu::OWeakObject,
          public css::io::XInputStream,
          public css::io::XSeekable
    {

    public:
        explicit XInputStream_impl( const OUString& aUncPath );

        virtual ~XInputStream_impl();

        /**
         *  Returns an error code as given by filerror.hxx
         */

        bool SAL_CALL CtorSuccess() { return m_bIsOpen;}

        virtual css::uno::Any SAL_CALL
        queryInterface(
            const css::uno::Type& rType )
            throw( css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
        acquire(
            void )
            throw() override;

        virtual void SAL_CALL
        release(
            void )
            throw() override;

        virtual sal_Int32 SAL_CALL
        readBytes(
            css::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception) override;

        virtual sal_Int32 SAL_CALL
        readSomeBytes(
            css::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
        skipBytes(
            sal_Int32 nBytesToSkip )
            throw( css::io::NotConnectedException,
                   css::io::BufferSizeExceededException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int32 SAL_CALL
        available(
            void )
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        closeInput(
            void )
            throw( css::io::NotConnectedException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        seek(
            sal_Int64 location )
            throw( css::lang::IllegalArgumentException,
                   css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual sal_Int64 SAL_CALL
        getLength(
            void )
            throw( css::io::IOException,
                   css::uno::RuntimeException, std::exception ) override;

    private:
        bool                                               m_bIsOpen;
        osl::File                                          m_aFile;
    };


} // end namespace XInputStream_impl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
