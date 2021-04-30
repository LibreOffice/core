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
#pragma once

#include <config_lgpl.h>
#include <sal/types.h>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>


namespace webdav_ucp
{

// A simple XInputStream implementation provided specifically for use
// by the DAVSession::GET method.
class NeonInputStream : public css::io::XInputStream,
                        public css::io::XSeekable,
                        public ::cppu::OWeakObject
{
    private:
        css::uno::Sequence< sal_Int8 > mInputBuffer;
        sal_Int64 mLen;
        sal_Int64 mPos;

    public:
                 NeonInputStream();
        virtual ~NeonInputStream() override;

        // Add some data to the end of the stream
        void AddToStream( const char * inBuf, sal_Int32 inLen );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & type ) override;

    virtual void SAL_CALL acquire()
                            noexcept override
                                { OWeakObject::acquire(); }

    virtual void SAL_CALL release()
                            noexcept override
                                { OWeakObject::release(); }


    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(
              css::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nBytesToRead ) override;

    virtual sal_Int32 SAL_CALL readSomeBytes(
            css::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nMaxBytesToRead ) override;

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;

    virtual sal_Int32 SAL_CALL available() override;

    virtual void SAL_CALL closeInput() override;

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;

    virtual sal_Int64 SAL_CALL getPosition() override;

    virtual sal_Int64 SAL_CALL getLength() override;
};

} // namespace webdav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
