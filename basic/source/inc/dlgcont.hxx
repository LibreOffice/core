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

#ifndef BASIC_DIALOGCONTAINER_HXX
#define BASIC_DIALOGCONTAINER_HXX

#include "namecont.hxx"

#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#include "com/sun/star/resource/XStringResourcePersistence.hpp"

#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>

namespace basic
{

//============================================================================

class SfxDialogLibraryContainer : public SfxLibraryContainer
{
    // Methods to distinguish between different library types
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

    virtual void onNewRootStorage();

    virtual const sal_Char* SAL_CALL    getInfoFileName() const;
    virtual const sal_Char* SAL_CALL    getOldInfoFileName() const;
    virtual const sal_Char* SAL_CALL    getLibElementFileExtension() const;
    virtual const sal_Char* SAL_CALL    getLibrariesDir() const;

public:
    SfxDialogLibraryContainer( void );
    SfxDialogLibraryContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // Methods XStorageBasedLibraryContainer
    virtual void SAL_CALL storeLibrariesToStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& RootStorage )
            throw (::com::sun::star::uno::RuntimeException);

    // Resource handling
    ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourcePersistence >
        implCreateStringResource( class SfxDialogLibrary* pDialog );

    // Methods XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName( )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException);
    // XLibraryQueryExecutable
    virtual sal_Bool SAL_CALL HasExecutableCode(const rtl::OUString&)
        throw (::com::sun::star::uno::RuntimeException);
    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static();
    static ::rtl::OUString getImplementationName_static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );
};

//============================================================================

typedef ::cppu::ImplHelper1 <   ::com::sun::star::resource::XStringResourceSupplier
                            >   SfxDialogLibrary_BASE;

class SfxDialogLibrary  :public SfxLibrary
                        ,public SfxDialogLibrary_BASE
{
    SfxDialogLibraryContainer*                                      m_pParent;
    ::com::sun::star::uno::Reference
        < ::com::sun::star::resource::XStringResourcePersistence>   m_xStringResourcePersistence;
    ::rtl::OUString                                                 m_aName;

    // Provide modify state including resources
    virtual sal_Bool isModified( void );
    virtual void storeResources( void );
    virtual void storeResourcesAsURL( const ::rtl::OUString& URL, const ::rtl::OUString& NewName );
    virtual void storeResourcesToURL( const ::rtl::OUString& URL,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler  );
    virtual void storeResourcesToStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage );

public:
    SfxDialogLibrary
    (
        ModifiableHelper& _rModifiable,
        const ::rtl::OUString& aName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI,
        SfxDialogLibraryContainer* pParent
    );

    SfxDialogLibrary
    (
        ModifiableHelper& _rModifiable,
        const ::rtl::OUString& aName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMSF,
        const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xSFI,
        const ::rtl::OUString& aLibInfoFileURL, const ::rtl::OUString& aStorageURL, sal_Bool ReadOnly,
        SfxDialogLibraryContainer* pParent
    );

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XStringResourceSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver >
        SAL_CALL getStringResource(  ) throw (::com::sun::star::uno::RuntimeException);

    ::rtl::OUString getName( void )
        { return m_aName; }

    ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourcePersistence >
        getStringResourcePersistence( void )
    {
        return m_xStringResourcePersistence;
    }

    static bool containsValidDialog( const ::com::sun::star::uno::Any& aElement );

protected:
    virtual bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement ) const;
};

}   // namespace basic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
