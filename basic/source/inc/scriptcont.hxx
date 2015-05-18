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
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > mxCodeNameAccess;

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const OUString& aName ) SAL_OVERRIDE;
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const OUString& aName, const OUString& aLibInfoFileURL,
          const OUString& StorageURL, bool ReadOnly ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL createEmptyLibraryElement() SAL_OVERRIDE;
    virtual bool SAL_CALL isLibraryElementValid(const css::uno::Any& rElement) const SAL_OVERRIDE;
    virtual void SAL_CALL writeLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutput
    )
        throw(::com::sun::star::uno::Exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any SAL_CALL importLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const OUString& aElementName,
        const OUString& aFile,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xElementStream ) SAL_OVERRIDE;

    virtual void SAL_CALL importFromOldStorage( const OUString& aFile ) SAL_OVERRIDE;

    virtual SfxLibraryContainer* createInstanceImpl() SAL_OVERRIDE;


    // Password encryption
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler ) SAL_OVERRIDE;

    // New variant for library export
    virtual bool implStorePasswordLibrary( SfxLibrary* pLib, const OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        const OUString& aTargetURL,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& rToUseSFI, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler ) SAL_OVERRIDE;

    virtual bool implLoadPasswordLibrary( SfxLibrary* pLib, const OUString& Name,
                                          bool bVerifyPasswordOnly=false )
            throw(::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    virtual void onNewRootStorage() SAL_OVERRIDE;


    // OldBasicPassword interface
    virtual void setLibraryPassword( const OUString& rLibraryName, const OUString& rPassword ) SAL_OVERRIDE;
    virtual OUString getLibraryPassword( const OUString& rLibraryName ) SAL_OVERRIDE;
    virtual void clearLibraryPassword( const OUString& rLibraryName ) SAL_OVERRIDE;
    virtual bool hasLibraryPassword( const OUString& rLibraryName ) SAL_OVERRIDE;

    virtual const sal_Char* SAL_CALL    getInfoFileName() const SAL_OVERRIDE;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const SAL_OVERRIDE;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const SAL_OVERRIDE;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const SAL_OVERRIDE;

public:
    SfxScriptLibraryContainer();
    SfxScriptLibraryContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );


    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const OUString& Name, const OUString& Password )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL changeLibraryPassword( const OUString& Name,
        const OUString& OldPassword, const OUString& NewPassword )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XLibraryQueryExecutable
    virtual sal_Bool SAL_CALL HasExecutableCode(const OUString&)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // Methods XServiceInfo
    virtual OUString SAL_CALL getImplementationName( )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


typedef std::unordered_map< OUString, ::com::sun::star::script::ModuleInfo, OUStringHash, ::std::equal_to< OUString > > ModuleInfoMap;

typedef ::cppu::ImplHelper1< ::com::sun::star::script::vba::XVBAModuleInfo > SfxScriptLibrary_BASE;

class SfxScriptLibrary : public SfxLibrary, public SfxScriptLibrary_BASE
{
    friend class SfxScriptLibraryContainer;

    typedef std::unordered_map< OUString, ::com::sun::star::script::ModuleInfo, OUStringHash > ModuleInfoMap;

    bool mbLoadedSource;
    bool mbLoadedBinary;
    ModuleInfoMap mModuleInfos;

    // Provide modify state including resources
    virtual bool isModified() SAL_OVERRIDE;
    virtual void storeResources() SAL_OVERRIDE;
    virtual void storeResourcesAsURL( const OUString& URL, const OUString& NewName ) SAL_OVERRIDE;
    virtual void storeResourcesToURL( const OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) SAL_OVERRIDE;
    virtual void storeResourcesToStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage ) SAL_OVERRIDE;
    virtual bool isLoadedStorable() SAL_OVERRIDE;

public:
    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xSFI
    );

    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess3 >& xSFI,
        const OUString& aLibInfoFileURL, const OUString& aStorageURL, bool ReadOnly
    );

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XVBAModuleInfo
    virtual ::com::sun::star::script::ModuleInfo SAL_CALL getModuleInfo( const OUString& ModuleName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasModuleInfo( const OUString& ModuleName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertModuleInfo( const OUString& ModuleName, const ::com::sun::star::script::ModuleInfo& ModuleInfo ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModuleInfo( const OUString& ModuleName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static bool containsValidModule( const ::com::sun::star::uno::Any& _rElement );

protected:
    virtual bool SAL_CALL isLibraryElementValid(const css::uno::Any& rElement) const SAL_OVERRIDE;
};



}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
