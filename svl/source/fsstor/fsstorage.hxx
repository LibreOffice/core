/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __XSTORAGE_HXX_
#define __XSTORAGE_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
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
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    virtual ~FSStorage();

    ::ucbhelper::Content* GetContent();

    void CopyStreamToSubStream( const ::rtl::OUString& aSourceURL,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                const ::rtl::OUString& aNewEntryName );

    void CopyContentToStorage_Impl( ::ucbhelper::Content* pContent,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest );

    static sal_Bool MakeFolderNoUI( const String& rFolder, sal_Bool bNewOnly );

    //____________________________________________________________________________________________________
    //  XInterface
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    //____________________________________________________________________________________________________
    //  XTypeProvider
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XStorage
    //____________________________________________________________________________________________________

    virtual void SAL_CALL copyToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneStreamElement(
            const ::rtl::OUString& aStreamName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, const ::rtl::OUString& aPass )
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
            const ::rtl::OUString& aStorName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStreamElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStorageElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL renameElement( const ::rtl::OUString& rEleName, const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL moveElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XNameAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasElements()
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XPropertySet
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XHierarchicalStorageAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const ::rtl::OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const ::rtl::OUString& sStreamName, ::sal_Int32 nOpenMode, const ::rtl::OUString& sPassword )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const ::rtl::OUString& sElementPath )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
