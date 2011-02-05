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

#ifndef BASIC_SCRIPTCONTAINER_HXX
#define BASIC_SCRIPTCONTAINER_HXX

#include "namecont.hxx"
#include <basic/basmgr.hxx>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <comphelper/uno3.hxx>

class BasicManager;

//============================================================================

namespace basic
{

class SfxScriptLibraryContainer : public SfxLibraryContainer, public OldBasicPassword
{
    ::rtl::OUString maScriptLanguage;

    // Methods to distinguish between deffirent library types
    virtual SfxLibrary* SAL_CALL implCreateLibrary( const ::rtl::OUString& aName );
    virtual SfxLibrary* SAL_CALL implCreateLibraryLink
        ( const ::rtl::OUString& aName, const ::rtl::OUString& aLibInfoFileURL,
          const ::rtl::OUString& StorageURL, sal_Bool ReadOnly );
    virtual ::com::sun::star::uno::Any SAL_CALL createEmptyLibraryElement( void );
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const;
    virtual void SAL_CALL writeLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const ::rtl::OUString& aElementName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutput
    )
        throw(::com::sun::star::uno::Exception);

    virtual ::com::sun::star::uno::Any SAL_CALL importLibraryElement
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& xLibrary,
        const ::rtl::OUString& aElementName,
        const ::rtl::OUString& aFile,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xElementStream );

    virtual void SAL_CALL importFromOldStorage( const ::rtl::OUString& aFile );

    virtual SfxLibraryContainer* createInstanceImpl( void );


    // Password encryption
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    // New variant for library export
    virtual sal_Bool implStorePasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& aName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                        const ::rtl::OUString& aTargetURL,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xToUseSFI, const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );

    virtual sal_Bool implLoadPasswordLibrary( SfxLibrary* pLib, const ::rtl::OUString& Name,
        sal_Bool bVerifyPasswordOnly=false )
            throw(::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    virtual void onNewRootStorage();


    // OldBasicPassword interface
    virtual void setLibraryPassword( const String& rLibraryName, const String& rPassword );
    virtual String getLibraryPassword( const String& rLibraryName );
    virtual void clearLibraryPassword( const String& rLibraryName );
    virtual sal_Bool hasLibraryPassword( const String& rLibraryName );

    virtual const sal_Char* SAL_CALL    getInfoFileName() const;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const;

public:
    SfxScriptLibraryContainer( void );
    SfxScriptLibraryContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );


    // Methods XLibraryContainerPassword
    virtual sal_Bool SAL_CALL isLibraryPasswordProtected( const ::rtl::OUString& Name )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLibraryPasswordVerified( const ::rtl::OUString& Name )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL verifyLibraryPassword( const ::rtl::OUString& Name, const ::rtl::OUString& Password )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeLibraryPassword( const ::rtl::OUString& Name,
        const ::rtl::OUString& OldPassword, const ::rtl::OUString& NewPassword )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    // XLibraryQueryExecutable
    virtual sal_Bool SAL_CALL HasExecutableCode(const rtl::OUString&)
        throw (::com::sun::star::uno::RuntimeException);
    // Methods XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException);

    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static();
    static ::rtl::OUString getImplementationName_static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );

};

//============================================================================
typedef boost::unordered_map< ::rtl::OUString, ::com::sun::star::script::ModuleInfo, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > ModuleInfoMap;

typedef ::cppu::ImplHelper1 <   ::com::sun::star::script::vba::XVBAModuleInfo
                            >   SfxScriptLibrary_BASE;

class SfxScriptLibrary : public SfxLibrary
                       , public SfxScriptLibrary_BASE
{
    friend class SfxScriptLibraryContainer;

    sal_Bool mbLoadedSource;
    sal_Bool mbLoadedBinary;
    ModuleInfoMap mModuleInfos;

    // Provide modify state including resources
    virtual sal_Bool isModified( void );
    virtual void storeResources( void );
    virtual void storeResourcesAsURL( const ::rtl::OUString& URL, const ::rtl::OUString& NewName );
    virtual void storeResourcesToURL( const ::rtl::OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );
    virtual void storeResourcesToStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage );

public:
    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI
    );

    SfxScriptLibrary
    (
        ModifiableHelper& _rModifiable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI,
        const ::rtl::OUString& aLibInfoFileURL, const ::rtl::OUString& aStorageURL, sal_Bool ReadOnly
    );

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XVBAModuleInfo
    virtual ::com::sun::star::script::ModuleInfo SAL_CALL getModuleInfo( const ::rtl::OUString& ModuleName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasModuleInfo( const ::rtl::OUString& ModuleName ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertModuleInfo( const ::rtl::OUString& ModuleName, const ::com::sun::star::script::ModuleInfo& ModuleInfo ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModuleInfo( const ::rtl::OUString& ModuleName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    static bool containsValidModule( const ::com::sun::star::uno::Any& _rElement );

protected:
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const;
};

}   // namespace base

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
