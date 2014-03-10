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
private:
    ContentHash maContents;
    sal_Int32 m_nFormat;
    OUString m_sVersion;

public:

    ZipPackageFolder( sal_Int32 nFormat,
                      sal_Bool bAllowRemoveOnInsert );
    virtual ~ZipPackageFolder();

    OUString& GetVersion() { return m_sVersion; }
    void SetVersion( const OUString& aVersion ) { m_sVersion = aVersion; }

    sal_Bool LookForUnexpectedODF12Streams( const OUString& aPath );

    void setChildStreamsTypeByExtension( const ::com::sun::star::beans::StringPair& aPair );

    void doInsertByName ( ZipPackageEntry *pEntry, sal_Bool bSetParent )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    com::sun::star::packages::ContentInfo & doGetByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    static void copyZipEntry( ZipEntry &rDest, const ZipEntry &rSource);
    static ::com::sun::star::uno::Sequence < sal_Int8 > static_getImplementationId();

    void setPackageFormat_Impl( sal_Int32 nFormat ) { m_nFormat = nFormat; }
    void setRemoveOnInsertMode_Impl( sal_Bool bRemove ) { this->mbAllowRemoveOnInsert = bRemove; }

    bool saveChild(const OUString &rShortName, const com::sun::star::packages::ContentInfo &rInfo, OUString &rPath, std::vector < com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > > &rManList, ZipOutputStream & rZipOut, const com::sun::star::uno::Sequence < sal_Int8 >& rEncryptionKey, rtlRandomPool & rRandomPool);

    // Recursive functions
    void  saveContents(OUString &rPath, std::vector < com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > > &rManList, ZipOutputStream & rZipOut, const com::sun::star::uno::Sequence< sal_Int8 > &rEncryptionKey, rtlRandomPool & rRandomPool)
        throw(::com::sun::star::uno::RuntimeException);
    void  releaseUpwardRef();

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
