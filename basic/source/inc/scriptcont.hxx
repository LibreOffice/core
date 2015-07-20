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

#ifndef INCLUDED_BASIC_SOURCE_INC_SCRIPTCONT_HXX
#define INCLUDED_BASIC_SOURCE_INC_SCRIPTCONT_HXX

#include "namecont.hxx"
#include <basic/basmgr.hxx>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <comphelper/uno3.hxx>

namespace basic
{



class SfxScriptLibraryContainer : public SfxLibraryContainer, public OldBasicPassword
{
    OUString maScriptLanguage;
    css::uno::Reference< css::container::XNameAccess > mxCodeNameAccess;

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const OUString& aName ) SAL_OVERRIDE;
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const OUString& aName, const OUString& aLibInfoFileURL,
          const OUString& StorageURL, bool ReadOnly ) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL createEmptyLibraryElement() SAL_OVERRIDE;
    virtual bool SAL_CALL isLibraryElementValid(const css::uno::Any& rElement) const SAL_OVERRIDE;
    virtual void SAL_CALL writeLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const css::uno::Reference< css::io::XOutputStream >& xOutput
    )
        throw(css::uno::Exception) SAL_OVERRIDE;

    virtual css::uno::Any SAL_CALL importLibraryElement
    (
        const css::uno::Reference< css::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const OUString& aFile,
        const css::uno::Reference< css::io::XInputStream >& xElementStream ) SAL_OVERRIDE;

    virtual void SAL_CALL importFromOldStorage( const OUString& aFile ) SAL_OVERRIDE;

    virtual SfxLibraryContainer* createInstanceImpl() SAL_OVERRIDE;


    // Password encryption
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage>& xStorage, const css::uno::Reference< css::task::XInteractionHandler >& Handler ) SAL_OVERRIDE;

    // New variant for library export
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const css::uno::Reference< css::embed::XStorage >& xStorage,
                        const OUString& aTargetURL,
                        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& rToUseSFI, const css::uno::Reference< css::task::XInteractionHandler >& Handler ) SAL_OVERRIDE;

    virtual bool implLoadPasswordLibrary( SfxLibrary* pLib, const OUString& Name,
                                          bool bVerifyPasswordOnly=false )
            throw(css::lang::WrappedTargetException,
                  css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void onNewRootStorage() SAL_OVERRIDE;


    // OldBasicPassword interface
    virtual void setLibraryPassword( const OUString& rLibraryName, const OUString& rPassword ) SAL_OVERRIDE;

    virtual const sal_Char* SAL_CALL    getInfoFileName() const SAL_OVERRIDE;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const SAL_OVERRIDE;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const SAL_OVERRIDE;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const SAL_OVERRIDE;

public:
    SfxScriptLibraryContainer();
    SfxScriptLibraryContainer( const css::uno::Reference< css::embed::XStorage >& xStorage );


    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const OUString& Name )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const OUString& Name )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const OUString& Name, const OUString& Password )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL changeLibraryPassword( const OUString& Name,
        const OUString& OldPassword, const OUString& NewPassword )
        throw (css::lang::IllegalArgumentException,
               css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XLibraryQueryExecutable
    virtual sal_Bool SAL_CALL HasExecutableCode(const OUString&)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


typedef std::unordered_map< OUString, css::script::ModuleInfo, OUStringHash, ::std::equal_to< OUString > > ModuleInfoMap;

typedef ::cppu::ImplHelper1< css::script::vba::XVBAModuleInfo > SfxScriptLibrary_BASE;

class SfxScriptLibrary : public SfxLibrary, public SfxScriptLibrary_BASE
{
    friend class SfxScriptLibraryContainer;

    typedef std::unordered_map< OUString, css::script::ModuleInfo, OUStringHash > ModuleInfoMap;

    bool mbLoadedSource;
    bool mbLoadedBinary;
    ModuleInfoMap mModuleInfos;

    // Provide modify state including resources
    virtual bool isModified() SAL_OVERRIDE;
    virtual void storeResources() SAL_OVERRIDE;
    virtual void storeResourcesAsURL( const OUString& URL, const OUString& NewName ) SAL_OVERRIDE;
    virtual void storeResourcesToURL( const OUString& URL,
        const css::uno::Reference< css::task::XInteractionHandler >& xHandler ) SAL_OVERRIDE;
    virtual void storeResourcesToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) SAL_OVERRIDE;
    virtual bool isLoadedStorable() SAL_OVERRIDE;

public:
    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI
    );

    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::ucb::XSimpleFileAccess3 >& xSFI,
        const OUString& aLibInfoFileURL, const OUString& aStorageURL, bool ReadOnly
    );

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XVBAModuleInfo
    virtual css::script::ModuleInfo SAL_CALL getModuleInfo( const OUString& ModuleName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasModuleInfo( const OUString& ModuleName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertModuleInfo( const OUString& ModuleName, const css::script::ModuleInfo& ModuleInfo ) throw (css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModuleInfo( const OUString& ModuleName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static bool containsValidModule( const css::uno::Any& _rElement );

protected:
    virtual bool SAL_CALL isLibraryElementValid(const css::uno::Any& rElement) const SAL_OVERRIDE;
};



}   // namespace basic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
