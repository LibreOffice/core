/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <optional>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <mutex>
#include <unotools/tempfile.hxx>

namespace cpo::uno { class XComponentContext; }

class SvStream;


typedef ::cppu::WeakImplHelper< css::io::XTempFile
    , css::io::XInputStream
    , css::io::XOutputStream
    , css::io::XTruncate
    , css::beans::XPropertySet
    , css::beans::XFastPropertySet
    , css::beans::XPropertyAccess
    , css::lang::XServiceInfo> OTempFileBase;

class OTempFileService : public OTempFileBase, public comphelper::ByteReader
{
    std::optional<utl::TempFileNamed> mpTempFile;
    std::mutex maMutex;
    SvStream* mpStream;
    bool mbRemoveFile;
    bool mbInClosed;
    bool mbOutClosed;

protected:
    void checkError () const;
    void checkConnected ();

public:
    explicit OTempFileService (css::uno::Reference< cpo::uno::XComponentContext > const & context);

    //Methods
    //  XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

    //  XServiceInfo
    virtual bool supportsService(const OUString& sServiceName) override;
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    //  XTempFile
    virtual bool getRemoveFile() override;
    virtual void setRemoveFile( bool _removefile ) override;
    virtual OUString getUri() override;
    virtual OUString getResourceName() override;

    // XInputStream
    virtual ::sal_Int32 readBytes( cpo::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) override;
    virtual ::sal_Int32 readSomeBytes( cpo::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) override;
    virtual void skipBytes( ::sal_Int32 nBytesToSkip ) override;
    virtual ::sal_Int32 available(  ) override;
    virtual void closeInput(  ) override;
    // XOutputStream
    virtual void writeBytes( const cpo::uno::Sequence< ::sal_Int8 >& aData ) override;
    virtual void flush(  ) override;
    virtual void closeOutput(  ) override;
    // XSeekable
    virtual void seek( sal_Int64 location ) override;
    virtual sal_Int64 getPosition(  ) override;
    virtual sal_Int64 getLength(  ) override;
    // XStream
    virtual css::uno::Reference< css::io::XInputStream > getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > getOutputStream(  ) override;
    // XTruncate
    virtual void truncate() override;

    // XPropertySet
    virtual ::css::uno::Reference< ::css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue( const ::rtl::OUString& aPropertyName, const ::cpo::uno::Any& aValue ) override;
    virtual ::cpo::uno::Any getPropertyValue( const ::rtl::OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::css::uno::Reference< ::css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::css::uno::Reference< ::css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::css::uno::Reference< ::css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::css::uno::Reference< ::css::beans::XVetoableChangeListener >& aListener ) override;
    // XFastPropertySet
    virtual void setFastPropertyValue( ::sal_Int32 nHandle, const ::cpo::uno::Any& aValue ) override;
    virtual ::cpo::uno::Any getFastPropertyValue( ::sal_Int32 nHandle ) override;
    // XPropertyAccess
    virtual ::cpo::uno::Sequence< ::css::beans::PropertyValue > getPropertyValues() override;
    virtual void setPropertyValues( const ::cpo::uno::Sequence< ::css::beans::PropertyValue >& aProps ) override;

    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) override;

    virtual ~OTempFileService () override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
