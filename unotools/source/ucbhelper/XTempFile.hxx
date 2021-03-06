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
#pragma once

#include <memory>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

namespace com::sun::star::uno { class XComponentContext; }

class SvStream;
namespace utl { class TempFile; }


typedef ::cppu::WeakImplHelper< css::io::XTempFile
    , css::io::XInputStream
    , css::io::XOutputStream
    , css::io::XTruncate
    , css::beans::XPropertySet
    , css::beans::XFastPropertySet
    , css::beans::XPropertyAccess
    , css::lang::XServiceInfo> OTempFileBase;

class OTempFileService : public OTempFileBase
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
    //  XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    //  XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

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

    // XPropertySet
    virtual ::css::uno::Reference< ::css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::css::uno::Any& aValue ) override;
    virtual ::css::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::css::uno::Reference< ::css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::css::uno::Reference< ::css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::css::uno::Reference< ::css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::css::uno::Reference< ::css::beans::XVetoableChangeListener >& aListener ) override;
    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const ::css::uno::Any& aValue ) override;
    virtual ::css::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) override;
    // XPropertyAccess
    virtual ::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL setPropertyValues( const ::css::uno::Sequence< ::css::beans::PropertyValue >& aProps ) override;


    virtual ~OTempFileService () override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
