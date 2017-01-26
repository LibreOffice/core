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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILINPSTR_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILINPSTR_HXX

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>

#include "filrec.hxx"

namespace fileaccess {

    class TaskManager;

    class XInputStream_impl
        : public cppu::OWeakObject,
          public css::lang::XTypeProvider,
          public css::io::XInputStream,
          public css::io::XSeekable
    {
    public:

        XInputStream_impl( const OUString& aUncPath, bool bLock );

        virtual ~XInputStream_impl() override;

        /**
         *  Returns an error code as given by filerror.hxx
         */

        sal_Int32 SAL_CALL CtorSuccess() { return m_nErrorCode;}
        sal_Int32 SAL_CALL getMinorError() { return m_nMinorErrorCode;}


        // XTypeProvider

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

        virtual css::uno::Any SAL_CALL
        queryInterface( const css::uno::Type& rType ) override;

        virtual void SAL_CALL
        acquire()
            throw() override;

        virtual void SAL_CALL
        release()
            throw() override;

        virtual sal_Int32 SAL_CALL
        readBytes(
            css::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead ) override;

        virtual sal_Int32 SAL_CALL
        readSomeBytes(
            css::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead ) override;

        virtual void SAL_CALL
        skipBytes( sal_Int32 nBytesToSkip ) override;

        virtual sal_Int32 SAL_CALL
        available() override;

        virtual void SAL_CALL
        closeInput() override;

        virtual void SAL_CALL
        seek( sal_Int64 location ) override;

        virtual sal_Int64 SAL_CALL
        getPosition() override;

        virtual sal_Int64 SAL_CALL
        getLength() override;

    private:

        bool                                               m_nIsOpen;

        ReconnectingFile                                   m_aFile;

        sal_Int32                                          m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;
    };
} // end namespace XInputStream_impl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
