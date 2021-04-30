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
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/weak.hxx>

#include <ucbhelper/content.hxx>

class FSStorage : public css::lang::XTypeProvider
                , public css::embed::XStorage
                , public css::embed::XHierarchicalStorageAccess
                , public css::beans::XPropertySet
                , public ::cppu::OWeakObject
{
    ::osl::Mutex m_aMutex;
    OUString  m_aURL;
    ::ucbhelper::Content m_aContent;
    sal_Int32 m_nMode;
    std::unique_ptr<::comphelper::OInterfaceContainerHelper2> m_pListenersContainer; // list of listeners
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

public:

    FSStorage(  const ::ucbhelper::Content& aContent,
                sal_Int32 nMode,
                css::uno::Reference< css::uno::XComponentContext > const & xContext );

    virtual ~FSStorage() override;

    ucbhelper::Content& GetContent();

    static void CopyStreamToSubStream( const OUString& aSourceURL,
                                const css::uno::Reference< css::embed::XStorage >& xDest,
                                const OUString& aNewEntryName );

    void CopyContentToStorage_Impl(ucbhelper::Content& rContent,
                                   const css::uno::Reference<css::embed::XStorage>& xDest);

    static bool MakeFolderNoUI( const OUString& rFolder );

    //  XInterface

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;

    virtual void SAL_CALL acquire() noexcept override;

    virtual void SAL_CALL release() noexcept override;

    //  XTypeProvider

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    //  XStorage

    virtual void SAL_CALL copyToStorage( const css::uno::Reference< css::embed::XStorage >& xDest ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openEncryptedStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode, const OUString& aPass ) override;

    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneStreamElement(
            const OUString& aStreamName ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const OUString& aStreamName, const OUString& aPass ) override;

    virtual void SAL_CALL copyLastCommitTo(
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage ) override;

    virtual void SAL_CALL copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage ) override;

    virtual sal_Bool SAL_CALL isStreamElement( const OUString& aElementName ) override;

    virtual sal_Bool SAL_CALL isStorageElement( const OUString& aElementName ) override;

    virtual void SAL_CALL removeElement( const OUString& aElementName ) override;

    virtual void SAL_CALL renameElement( const OUString& rEleName, const OUString& rNewName ) override;

    virtual void SAL_CALL copyElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& aNewName ) override;

    virtual void SAL_CALL moveElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& rNewName ) override;

    //  XNameAccess

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    virtual css::uno::Type SAL_CALL getElementType() override;

    virtual sal_Bool SAL_CALL hasElements() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    //  XPropertySet

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;

    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //  XHierarchicalStorageAccess

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const OUString& sStreamPath, ::sal_Int32 nOpenMode ) override;

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const OUString& sStreamName, ::sal_Int32 nOpenMode, const OUString& sPassword ) override;

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const OUString& sElementPath ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
