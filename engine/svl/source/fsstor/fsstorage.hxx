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

#ifndef INCLUDED_SVL_SOURCE_FSSTOR_FSSTORAGE_HXX
#define INCLUDED_SVL_SOURCE_FSSTOR_FSSTORAGE_HXX

#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/weak.hxx>

#include <ucbhelper/content.hxx>

class FSStorage final : public css::lang::XTypeProvider
                , public css::embed::XStorage
                , public css::embed::XHierarchicalStorageAccess
                , public css::beans::XPropertySet
                , public ::cppu::OWeakObject
{
    std::mutex m_aMutex;
    OUString  m_aURL;
    ::ucbhelper::Content m_aContent;
    sal_Int32 m_nMode;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aListenersContainer; // list of listeners
    css::uno::Reference< cpo::uno::XComponentContext > m_xContext;

public:

    FSStorage(  const ::ucbhelper::Content& aContent,
                sal_Int32 nMode,
                css::uno::Reference< cpo::uno::XComponentContext > const & xContext );

    virtual ~FSStorage() override;

    ucbhelper::Content& GetContent();

    static void CopyStreamToSubStream( const OUString& aSourceURL,
                                const css::uno::Reference< css::embed::XStorage >& xDest,
                                const OUString& aNewEntryName );

    void CopyContentToStorage_Impl(ucbhelper::Content& rContent,
                                   const css::uno::Reference<css::embed::XStorage>& xDest);

    static bool MakeFolderNoUI( std::u16string_view rFolder );

    //  XInterface

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override;

    virtual void acquire() noexcept override;

    virtual void release() noexcept override;

    //  XTypeProvider

    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;

    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    //  XStorage

    virtual void copyToStorage( const css::uno::Reference< css::embed::XStorage >& xDest ) override;

    virtual css::uno::Reference< css::io::XStream > openStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode ) override;

    virtual css::uno::Reference< css::io::XStream > openEncryptedStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode, const OUString& aPass ) override;

    virtual css::uno::Reference< css::embed::XStorage > openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode ) override;

    virtual css::uno::Reference< css::io::XStream > cloneStreamElement(
            const OUString& aStreamName ) override;

    virtual css::uno::Reference< css::io::XStream > cloneEncryptedStreamElement(
            const OUString& aStreamName, const OUString& aPass ) override;

    virtual void copyLastCommitTo(
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage ) override;

    virtual void copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage ) override;

    virtual bool isStreamElement( const OUString& aElementName ) override;

    virtual bool isStorageElement( const OUString& aElementName ) override;

    virtual void removeElement( const OUString& aElementName ) override;

    virtual void renameElement( const OUString& rEleName, const OUString& rNewName ) override;

    virtual void copyElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& aNewName ) override;

    virtual void moveElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& rNewName ) override;

    //  XNameAccess

    virtual cpo::uno::Any getByName( const OUString& aName ) override;

    virtual cpo::uno::Sequence< OUString > getElementNames() override;

    virtual bool hasByName( const OUString& aName ) override;

    virtual cpo::uno::Type getElementType() override;

    virtual bool hasElements() override;

    //  XComponent

    virtual void dispose() override;

    virtual void addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    virtual void removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

    //  XPropertySet

    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;

    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;

    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;

    virtual void addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;

    virtual void removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

    virtual void addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    virtual void removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //  XHierarchicalStorageAccess

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > openStreamElementByHierarchicalName( const OUString& sStreamPath, ::sal_Int32 nOpenMode ) override;

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > openEncryptedStreamElementByHierarchicalName( const OUString& sStreamName, ::sal_Int32 nOpenMode, const OUString& sPassword ) override;

    virtual void removeStreamElementByHierarchicalName( const OUString& sElementPath ) override;

private:
    css::uno::Reference< css::embed::XStorage > openStorageElementImpl(
            std::unique_lock<std::mutex>& rGuard,
            std::u16string_view aStorName, sal_Int32 nStorageMode );
    css::uno::Reference< css::io::XStream > openStreamElementImpl(
            std::unique_lock<std::mutex>& rGuard,
            std::u16string_view aStreamName, sal_Int32 nOpenMode );
    void copyElementToImpl(std::unique_lock<std::mutex>& rGuard,
            std::u16string_view ElementName,
            const css::uno::Reference< css::embed::XStorage >& xDest,
            const OUString& rNewName);

    void disposeImpl(std::unique_lock<std::mutex>& rGuard);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
