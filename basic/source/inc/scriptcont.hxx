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
#include <basic/basmgr.hxx>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>

namespace basic
{


class SfxScriptLibraryContainer : public SfxLibraryContainer, public OldBasicPassword
{
    css::uno::Reference< css::container::XNameAccess > mxCodeNameAccess;

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


    // Password encryption
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage>& xStorage, const css::uno::Reference< css::task::XInteractionHandler >& Handler ) override;

    // New variant for library export
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage >& xStorage,
                        const OUString& aTargetURL,
                        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI, const css::uno::Reference< css::task::XInteractionHandler >& Handler ) override;

    virtual bool implLoadPasswordLibrary( SfxLibrary* pLib, const OUString& Name,
                                          bool bVerifyPasswordOnly=false ) override;

    virtual void onNewRootStorage() override;


    // OldBasicPassword interface
    virtual void setLibraryPassword( const OUString& rLibraryName, const OUString& rPassword ) override;

    virtual const char*    getInfoFileName() const override;
    virtual const char*    getOldInfoFileName() const override;
    virtual const char*    getLibElementFileExtension() const override;
    virtual const char*    getLibrariesDir() const override;

public:
    SfxScriptLibraryContainer();
    SfxScriptLibraryContainer( const css::uno::Reference< css::embed::XStorage >& xStorage );


    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const OUString& Name, const OUString& Password ) override;
    virtual void SAL_CALL changeLibraryPassword( const OUString& Name,
        const OUString& OldPassword, const OUString& NewPassword ) override;
    // XLibraryQueryExecutable
    virtual sal_Bool SAL_CALL HasExecutableCode(const OUString&) override;
    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( ) override;
};


typedef std::unordered_map< OUString, css::script::ModuleInfo > ModuleInfoMap;

typedef ::cppu::ImplHelper1< css::script::vba::XVBAModuleInfo > SfxScriptLibrary_BASE;

class SfxScriptLibrary : public SfxLibrary, public SfxScriptLibrary_BASE
{
    friend class SfxScriptLibraryContainer;

    typedef std::unordered_map< OUString, css::script::ModuleInfo > ModuleInfoMap;

    bool mbLoadedSource;
    bool mbLoadedBinary;
    ModuleInfoMap mModuleInfo;

    // Provide modify state including resources
    virtual bool isModified() override;
    virtual void storeResources() override;
    virtual void storeResourcesAsURL( const OUString& URL, const OUString& NewName ) override;
    virtual void storeResourcesToURL( const OUString& URL,
        const css::uno::Reference< css::task::XInteractionHandler >& xHandler ) override;
    virtual void storeResourcesToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    virtual bool isLoadedStorable() override;

public:
    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI
    );

    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI,
        const OUString& aLibInfoFileURL, const OUString& aStorageURL, bool ReadOnly
    );

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XVBAModuleInfo
    virtual css::script::ModuleInfo SAL_CALL getModuleInfo( const OUString& ModuleName ) override;
    virtual sal_Bool SAL_CALL hasModuleInfo( const OUString& ModuleName ) override;
    virtual void SAL_CALL insertModuleInfo( const OUString& ModuleName, const css::script::ModuleInfo& ModuleInfo ) override;
    virtual void SAL_CALL removeModuleInfo( const OUString& ModuleName ) override;

    static bool containsValidModule( const css::uno::Any& _rElement );

protected:
    virtual bool isLibraryElementValid(const css::uno::Any& rElement) const override;
};


}   // namespace basic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
