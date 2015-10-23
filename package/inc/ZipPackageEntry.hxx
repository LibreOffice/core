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
#ifndef INCLUDED_PACKAGE_INC_ZIPPACKAGEENTRY_HXX
#define INCLUDED_PACKAGE_INC_ZIPPACKAGEENTRY_HXX

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ZipEntry.hxx>
#include <cppuhelper/implbase.hxx>

#include <vector>

typedef void* rtlRandomPool;
class ZipOutputStream;
class ZipPackageFolder;

class ZipPackageEntry : public cppu::WeakImplHelper
<
    css::container::XNamed,
    css::container::XChild,
    css::lang::XUnoTunnel,
    css::beans::XPropertySet,
    css::lang::XServiceInfo
>
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    OUString msName;
    bool mbIsFolder:1;
    bool mbAllowRemoveOnInsert:1;
    OUString msMediaType;
    ZipPackageFolder* mpParent;
    sal_Int32 m_nFormat;

public:
    ZipEntry aEntry;
    ZipPackageEntry();
    virtual ~ZipPackageEntry();

    const OUString& GetMediaType () const { return msMediaType; }
    void SetMediaType ( const OUString & sNewType) { msMediaType = sNewType; }
    void doSetParent ( ZipPackageFolder * pNewParent, bool bInsert );
    bool IsFolder ( ) { return mbIsFolder; }
    void SetFolder ( bool bSetFolder ) { mbIsFolder = bSetFolder; }

    virtual bool saveChild( const OUString &rPath,
                            std::vector < css::uno::Sequence < css::beans::PropertyValue > > &rManList,
                            ZipOutputStream & rZipOut,
                            const css::uno::Sequence < sal_Int8 >& rEncryptionKey,
                            const rtlRandomPool &rRandomPool ) = 0;

    void clearParent()
    {
        // xParent.clear();
        mpParent = NULL;
    }
    // XNamed
    virtual OUString SAL_CALL getName(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName )
        throw(css::uno::RuntimeException, std::exception) override;
    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent )
        throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(css::uno::RuntimeException, std::exception) override = 0;
    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
        throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override = 0;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override = 0;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
        throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
        throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
