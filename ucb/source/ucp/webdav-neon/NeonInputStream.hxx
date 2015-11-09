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
#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONINPUTSTREAM_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONINPUTSTREAM_HXX

#include <config_lgpl.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
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
        virtual ~NeonInputStream();

        // Add some data to the end of the stream
        void AddToStream( const char * inBuf, sal_Int32 inLen );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & type )
                            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL acquire()
                            throw () override
                                { OWeakObject::acquire(); }

    virtual void SAL_CALL release()
                            throw() override
                                { OWeakObject::release(); }


    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(
              css::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nBytesToRead )
                throw( css::io::NotConnectedException,
                          css::io::BufferSizeExceededException,
                          css::io::IOException,
                          css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL readSomeBytes(
            css::uno::Sequence< sal_Int8 > & aData,
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

    // XSeekable
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

} // namespace webdav_ucp
#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONINPUTSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
