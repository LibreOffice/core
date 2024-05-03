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
#ifndef INCLUDED_PACKAGE_INC_ZIPPACKAGEFOLDER_HXX
#define INCLUDED_PACKAGE_INC_ZIPPACKAGEFOLDER_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "ZipPackageEntry.hxx"
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <string_view>
#include <unordered_map>
#include <vector>

class ZipOutputStream;
struct ZipEntry;
class ZipPackageFolder;
class ZipPackageStream;

struct ZipContentInfo
{
    rtl::Reference < ZipPackageEntry > xPackageEntry;
    bool bFolder;
    union
    {
        ZipPackageFolder *pFolder;
        ZipPackageStream *pStream;
    };
    ZipContentInfo( ZipPackageStream * pNewStream );
    ZipContentInfo( ZipPackageFolder * pNewFolder );
    ZipContentInfo( const ZipContentInfo& );
    ZipContentInfo( ZipContentInfo&& );
    ZipContentInfo& operator=( const ZipContentInfo& );
    ZipContentInfo& operator=( ZipContentInfo&& );

    ~ZipContentInfo();
};

typedef std::unordered_map < OUString,
                             ZipContentInfo > ContentHash;

class ZipPackageFolder final : public cppu::ImplInheritanceHelper
<
    ZipPackageEntry,
    css::container::XNameContainer,
    css::container::XEnumerationAccess
>
{
private:
    ContentHash maContents;
    OUString m_sVersion;

public:

    ZipPackageFolder( const css::uno::Reference < css::uno::XComponentContext >& xContext,
                      sal_Int32 nFormat,
                      bool bAllowRemoveOnInsert );
    virtual ~ZipPackageFolder() override;

    const OUString& GetVersion() const { return m_sVersion; }
    void SetVersion( const OUString& aVersion ) { m_sVersion = aVersion; }

    bool LookForUnexpectedODF12Streams(std::u16string_view aPath, bool isWholesomeEncryption);

    void setChildStreamsTypeByExtension( const css::beans::StringPair& aPair );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::container::ElementExistException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void doInsertByName ( ZipPackageEntry *pEntry, bool bSetParent );

    ZipContentInfo& doGetByName( const OUString& aName );

    void setPackageFormat_Impl( sal_Int32 nFormat ) { m_nFormat = nFormat; }
    void setRemoveOnInsertMode_Impl( bool bRemove ) { mbAllowRemoveOnInsert = bRemove; }

    virtual bool saveChild( const OUString &rPath,
                            std::vector < css::uno::Sequence < css::beans::PropertyValue > > &rManList,
                            ZipOutputStream & rZipOut,
                            const css::uno::Sequence < sal_Int8 >& rEncryptionKey,
                            ::std::optional<sal_Int32> oPBKDF2IterationCount,
                            ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> oArgon2Args) override;

    // Recursive functions
    /// @throws css::uno::RuntimeException
    void saveContents(
            const OUString &rPath,
            std::vector < css::uno::Sequence < css::beans::PropertyValue > > &rManList,
            ZipOutputStream & rZipOut,
            const css::uno::Sequence< sal_Int8 > &rEncryptionKey,
            ::std::optional<sal_Int32> oPBKDF2IterationCount,
            ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> oArgon2Args) const;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
