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

#ifndef INCLUDED_SVL_SOURCE_FSSTOR_FSSTORAGE_HXX
#define INCLUDED_SVL_SOURCE_FSSTOR_FSSTORAGE_HXX

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
class FSStorage : public css::lang::XTypeProvider
                , public css::embed::XStorage
                , public css::embed::XHierarchicalStorageAccess
                , public css::beans::XPropertySet
                , public ::cppu::OWeakObject
{
    ::osl::Mutex m_aMutex;
    FSStorage_Impl* m_pImpl;

protected:

public:

    FSStorage(  const ::ucbhelper::Content& aContent,
                sal_Int32 nMode,
                css::uno::Reference< css::uno::XComponentContext > xContext );

    virtual ~FSStorage();

    ::ucbhelper::Content* GetContent();

    static void CopyStreamToSubStream( const OUString& aSourceURL,
                                const css::uno::Reference< css::embed::XStorage >& xDest,
                                const OUString& aNewEntryName );

    void CopyContentToStorage_Impl( ::ucbhelper::Content* pContent,
                                    const css::uno::Reference< css::embed::XStorage >& xDest );

    static bool MakeFolderNoUI( const OUString& rFolder );

    //  XInterface

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL acquire() throw() override;

    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XStorage

    virtual void SAL_CALL copyToStorage( const css::uno::Reference< css::embed::XStorage >& xDest )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openEncryptedStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode, const OUString& aPass )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneStreamElement(
            const OUString& aStreamName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const OUString& aStreamName, const OUString& aPass )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyLastCommitTo(
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isStreamElement( const OUString& aElementName )
        throw ( css::container::NoSuchElementException,
                css::lang::IllegalArgumentException,
                css::embed::InvalidStorageException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isStorageElement( const OUString& aElementName )
        throw ( css::container::NoSuchElementException,
                css::lang::IllegalArgumentException,
                css::embed::InvalidStorageException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeElement( const OUString& aElementName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL renameElement( const OUString& rEleName, const OUString& rNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& aNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL moveElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& rNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    //  XNameAccess

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw ( css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Type SAL_CALL getElementType()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasElements()
        throw ( css::uno::RuntimeException, std::exception ) override;

    //  XComponent

    virtual void SAL_CALL dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    //  XPropertySet

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
        throw ( css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    //  XHierarchicalStorageAccess

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const OUString& sStreamName, ::sal_Int32 nOpenMode, const OUString& sPassword )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const OUString& sElementPath )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
