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
#ifndef INCLUDED_UCB_SOURCE_UCP_GVFS_GVFS_STREAM_HXX
#define INCLUDED_UCB_SOURCE_UCP_GVFS_GVFS_STREAM_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <libgnomevfs/gnome-vfs-handle.h>

namespace gvfs
{

class Stream : public ::com::sun::star::io::XStream,
           public ::com::sun::star::io::XInputStream,
           public ::com::sun::star::io::XOutputStream,
           public ::com::sun::star::io::XTruncate,
           public ::com::sun::star::io::XSeekable,
           public ::cppu::OWeakObject
{
private:
    GnomeVFSHandle  *m_handle;
    GnomeVFSFileInfo m_info;
    osl::Mutex       m_aMutex;
    sal_Bool         m_eof;
    sal_Bool         m_bInputStreamCalled;
    sal_Bool         m_bOutputStreamCalled;

    void throwOnError( GnomeVFSResult result )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

    void closeStream()
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

public:
    Stream ( GnomeVFSHandle         *handle,
         const GnomeVFSFileInfo *aInfo );
    virtual ~Stream();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & type )
        throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw () SAL_OVERRIDE
            { OWeakObject::acquire(); }
    virtual void SAL_CALL release()
        throw() SAL_OVERRIDE
            { OWeakObject::release(); }

    // XStream
    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL getInputStream(  )
        throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  )
        throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(
              ::com::sun::star::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nBytesToRead )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::BufferSizeExceededException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL readSomeBytes(
            ::com::sun::star::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nMaxBytesToRead )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::BufferSizeExceededException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::BufferSizeExceededException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL available()
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual void SAL_CALL closeInput()
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual sal_Int64 SAL_CALL getPosition()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    virtual sal_Int64 SAL_CALL getLength()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

        // XOutputStream
    virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    virtual void SAL_CALL flush()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException) SAL_OVERRIDE;


        virtual void SAL_CALL closeOutput()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XTruncate
    virtual void SAL_CALL truncate()
        throw( com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
};

}
#endif // INCLUDED_UCB_SOURCE_UCP_GVFS_GVFS_STREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
