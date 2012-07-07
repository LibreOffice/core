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
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#define _ZIP_PACKAGE_FOLDER_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <HashMaps.hxx>
#include <ZipPackageEntry.hxx>
#include <cppuhelper/implbase2.hxx>
#include <vector>

namespace com { namespace sun { namespace star {
namespace beans
{
    struct PropertyValue;
}
namespace packages
{
    class ContentInfo;
}
} } }

class ZipOutputStream;
struct ZipEntry;
typedef void* rtlRandomPool;

class ZipPackageFolder : public cppu::ImplInheritanceHelper2
<
    ZipPackageEntry,
    ::com::sun::star::container::XNameContainer,
    ::com::sun::star::container::XEnumerationAccess
>
{
protected:
    ContentHash maContents;
    const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    sal_Int32 m_nFormat;
    ::rtl::OUString m_sVersion;

public:

    ZipPackageFolder( const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& xFactory,
                      sal_Int32 nFormat,
                      sal_Bool bAllowRemoveOnInsert );
    virtual ~ZipPackageFolder();

    ::rtl::OUString& GetVersion() { return m_sVersion; }
    void SetVersion( const ::rtl::OUString& aVersion ) { m_sVersion = aVersion; }

    sal_Bool LookForUnexpectedODF12Streams( const ::rtl::OUString& aPath );

    void setChildStreamsTypeByExtension( const ::com::sun::star::beans::StringPair& aPair );

    void doInsertByName ( ZipPackageEntry *pEntry, sal_Bool bSetParent )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    com::sun::star::packages::ContentInfo & doGetByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    static void copyZipEntry( ZipEntry &rDest, const ZipEntry &rSource);
    static const ::com::sun::star::uno::Sequence < sal_Int8 >& static_getImplementationId();

    void setPackageFormat_Impl( sal_Int32 nFormat ) { m_nFormat = nFormat; }
    void setRemoveOnInsertMode_Impl( sal_Bool bRemove ) { this->mbAllowRemoveOnInsert = bRemove; }

    bool saveChild(const rtl::OUString &rShortName, const com::sun::star::packages::ContentInfo &rInfo, rtl::OUString &rPath, std::vector < com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > > &rManList, ZipOutputStream & rZipOut, const com::sun::star::uno::Sequence < sal_Int8 >& rEncryptionKey, rtlRandomPool & rRandomPool);

    // Recursive functions
    void  saveContents(rtl::OUString &rPath, std::vector < com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > > &rManList, ZipOutputStream & rZipOut, const com::sun::star::uno::Sequence< sal_Int8 > &rEncryptionKey, rtlRandomPool & rRandomPool)
        throw(::com::sun::star::uno::RuntimeException);
    void  releaseUpwardRef();

    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  )
        throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
