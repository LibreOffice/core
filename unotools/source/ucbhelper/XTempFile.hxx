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

#include <memory>
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
    std::unique_ptr<utl::TempFile> mpTempFile;
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
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  )
        throw () override;
    virtual void SAL_CALL release(  )
        throw () override;
    //  XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    //  XTempFile
    virtual sal_Bool SAL_CALL getRemoveFile() override;
    virtual void SAL_CALL setRemoveFile( sal_Bool _removefile ) override;
    virtual OUString SAL_CALL getUri() override;
    virtual OUString SAL_CALL getResourceName() override;

    // XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) override;
    virtual ::sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) override;
    virtual ::sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;
    // XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< ::sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL closeOutput(  ) override;
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition(  ) override;
    virtual sal_Int64 SAL_CALL getLength(  ) override;
    // XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) override;
    // XTruncate
    virtual void SAL_CALL truncate() override;

    virtual ~OTempFileService () override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
