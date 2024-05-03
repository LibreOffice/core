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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "ZipEntry.hxx"
#include <cppuhelper/implbase.hxx>

#include <vector>
#include <optional>
#include <tuple>

class ZipOutputStream;
class ZipPackageFolder;

class ZipPackageEntry : public cppu::WeakImplHelper
<
    css::container::XNamed,
    css::container::XChild,
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
    virtual ~ZipPackageEntry() override;

    const OUString& GetMediaType() const { return msMediaType; }
    void SetMediaType(const OUString & sNewType) { msMediaType = sNewType; }
    void doSetParent(ZipPackageFolder * pNewParent);
    bool IsFolder() const { return mbIsFolder; }
    void SetFolder(const bool bSetFolder) { mbIsFolder = bSetFolder; }

    virtual bool saveChild( const OUString &rPath,
                            std::vector < css::uno::Sequence < css::beans::PropertyValue > > &rManList,
                            ZipOutputStream & rZipOut,
                            const css::uno::Sequence < sal_Int8 >& rEncryptionKey,
                            ::std::optional<sal_Int32> oPBKDF2IterationCount,
                            ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> oArgon2Args) = 0;

    void clearParent()
    {
        // xParent.clear();
        mpParent = nullptr;
    }
    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;
    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;
    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override = 0;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override = 0;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
