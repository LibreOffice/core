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
#ifndef INCLUDED_UNOTOOLS_SOURCE_UCBHELPER_XTEMPFILE_HXX
#define INCLUDED_UNOTOOLS_SOURCE_UCBHELPER_XTEMPFILE_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <osl/mutex.hxx>

class SvStream;
namespace utl { class TempFile; }


typedef ::cppu::WeakImplHelper< css::io::XTempFile
    , css::io::XInputStream
    , css::io::XOutputStream
    , css::io::XTruncate > OTempFileBase;

class OTempFileService : public OTempFileBase
    , public ::cppu::PropertySetMixin< css::io::XTempFile >
{
protected:
    ::utl::TempFile* mpTempFile;
    ::osl::Mutex maMutex;
    SvStream* mpStream;
    bool mbRemoveFile;
    bool mbInClosed;
    bool mbOutClosed;

    sal_Int64 mnCachedPos;
    bool mbHasCachedPos;

    void checkError () const;
    void checkConnected ();

public:
    explicit OTempFileService (css::uno::Reference< css::uno::XComponentContext > const & context);

    //Methods
    //  XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  )
        throw () override;
    virtual void SAL_CALL release(  )
        throw () override;
    //  XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  )
        throw (css::uno::RuntimeException, std::exception) override;

    //  XTempFile
    virtual sal_Bool SAL_CALL getRemoveFile()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRemoveFile( sal_Bool _removefile )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getUri()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getResourceName()
        throw (css::uno::RuntimeException, std::exception) override;

    // XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL available(  )
        throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput(  )
        throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    // XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< ::sal_Int8 >& aData )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL flush(  )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeOutput(  )
        throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength(  )
        throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    // XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  )
        throw (css::uno::RuntimeException, std::exception) override;
    // XTruncate
    virtual void SAL_CALL truncate()
        throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

    virtual ~OTempFileService ();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
