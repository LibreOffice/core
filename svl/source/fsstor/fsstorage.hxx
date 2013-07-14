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

#ifndef __XSTORAGE_HXX_
#define __XSTORAGE_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <ucbhelper/content.hxx>

struct FSStorage_Impl;
class FSStorage : public ::com::sun::star::lang::XTypeProvider
                , public ::com::sun::star::embed::XStorage
                , public ::com::sun::star::embed::XHierarchicalStorageAccess
                , public ::com::sun::star::beans::XPropertySet
                , public ::cppu::OWeakObject
{
    ::osl::Mutex m_aMutex;
    FSStorage_Impl* m_pImpl;

protected:

public:

    FSStorage(  const ::ucbhelper::Content& aContent,
                sal_Int32 nMode,
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext );

    virtual ~FSStorage();

    ::ucbhelper::Content* GetContent();

    void CopyStreamToSubStream( const OUString& aSourceURL,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                const OUString& aNewEntryName );

    void CopyContentToStorage_Impl( ::ucbhelper::Content* pContent,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest );

    static sal_Bool MakeFolderNoUI( const OUString& rFolder );

    //  XInterface

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    //  XTypeProvider

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    //  XStorage

    virtual void SAL_CALL copyToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openEncryptedStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode, const OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneStreamElement(
            const OUString& aStreamName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const OUString& aStreamName, const OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyLastCommitTo(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStreamElement( const OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStorageElement( const OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeElement( const OUString& aElementName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL renameElement( const OUString& rEleName, const OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyElementTo(    const OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const OUString& aNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL moveElementTo(    const OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //  XNameAccess

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasElements()
        throw ( ::com::sun::star::uno::RuntimeException );

    //  XComponent

    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //  XPropertySet

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& PropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //  XHierarchicalStorageAccess

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const OUString& sStreamName, ::sal_Int32 nOpenMode, const OUString& sPassword )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const OUString& sElementPath )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
