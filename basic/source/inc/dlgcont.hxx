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

#pragma once

#include "namecont.hxx"

#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include <com/sun/star/resource/XStringResourcePersistence.hpp>

#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>

namespace basic
{


class SfxDialogLibraryContainer : public SfxLibraryContainer
{
    // Methods to distinguish between different library types
    virtual rtl::Reference<SfxLibrary> implCreateLibrary( const OUString& aName ) override;
    virtual rtl::Reference<SfxLibrary> implCreateLibraryLink
        ( const OUString& aName, const OUString& aLibInfoFileURL,
          const OUString& StorageURL, bool ReadOnly ) override;
    virtual css::uno::Any createEmptyLibraryElement() override;
    virtual bool isLibraryElementValid(const css::uno::Any& rElement) const override;
    virtual void writeLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const css::uno::Reference< css::io::XOutputStream >& xOutput
    ) override;

    virtual css::uno::Any importLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const OUString& aFile,
        const css::uno::Reference< css::io::XInputStream >& xElementStream ) override;

    virtual void importFromOldStorage( const OUString& aFile ) override;

    virtual SfxLibraryContainer* createInstanceImpl() override;

    virtual void onNewRootStorage() override;

    virtual const char*    getInfoFileName() const override;
    virtual const char*    getOldInfoFileName() const override;
    virtual const char*    getLibElementFileExtension() const override;
    virtual const char*    getLibrariesDir() const override;

public:
    SfxDialogLibraryContainer();
    SfxDialogLibraryContainer( const css::uno::Reference< css::embed::XStorage >& xStorage );

    // Methods XStorageBasedLibraryContainer
    virtual void SAL_CALL storeLibrariesToStorage(
        const css::uno::Reference< css::embed::XStorage >& RootStorage ) override;

    // Resource handling
    css::uno::Reference< css::resource::XStringResourcePersistence >
        implCreateStringResource( class SfxDialogLibrary* pDialog );

    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( ) override;
    // XLibraryQueryExecutable
    virtual sal_Bool SAL_CALL HasExecutableCode(const OUString&) override;
};


typedef ::cppu::ImplHelper1 <   css::resource::XStringResourceSupplier
                            >   SfxDialogLibrary_BASE;

class SfxDialogLibrary  :public SfxLibrary
                        ,public SfxDialogLibrary_BASE
{
    SfxDialogLibraryContainer*                                        m_pParent;
    css::uno::Reference< css::resource::XStringResourcePersistence>   m_xStringResourcePersistence;
    OUString                                                          m_aName;

    // Provide modify state including resources
    virtual bool isModified() override;
    virtual void storeResources() override;
    virtual void storeResourcesAsURL( const OUString& URL, const OUString& NewName ) override;
    virtual void storeResourcesToURL( const OUString& URL,
        const css::uno::Reference< css::task::XInteractionHandler >& xHandler  ) override;
    virtual void storeResourcesToStorage( const css::uno::Reference
        < css::embed::XStorage >& xStorage ) override;

public:
    SfxDialogLibrary
    (
        ModifiableHelper& _rModifiable,
        const OUString& aName,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI,
        SfxDialogLibraryContainer* pParent
    );

    SfxDialogLibrary
    (
        ModifiableHelper& _rModifiable,
        const OUString& aName,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI,
        const OUString& aLibInfoFileURL, const OUString& aStorageURL, bool ReadOnly,
        SfxDialogLibraryContainer* pParent
    );

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XStringResourceSupplier
    virtual css::uno::Reference< css::resource::XStringResourceResolver >
        SAL_CALL getStringResource(  ) override;

    const OUString& getName() const
        { return m_aName; }

    const css::uno::Reference< css::resource::XStringResourcePersistence >&
        getStringResourcePersistence() const
    {
        return m_xStringResourcePersistence;
    }

    static bool containsValidDialog( const css::uno::Any& aElement );

protected:
    virtual bool isLibraryElementValid(const css::uno::Any& rElement) const override;
};

}   // namespace basic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
