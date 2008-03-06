/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scriptcont.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:53:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef BASIC_SCRIPTCONTAINER_HXX
#define BASIC_SCRIPTCONTAINER_HXX

#ifndef BASIC_NAMECONTAINER_HXX
#include "namecont.hxx"
#endif

#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif

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
    virtual sal_Bool SAL_CALL isLibraryElementValid( ::com::sun::star::uno::Any aElement );
    virtual void SAL_CALL writeLibraryElement
    (
        ::com::sun::star::uno::Any aElement,
        const ::rtl::OUString& aElementName,
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutput
    )
        throw(::com::sun::star::uno::Exception);

    virtual ::com::sun::star::uno::Any SAL_CALL importLibraryElement
        ( const ::rtl::OUString& aFile,
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

class SfxScriptLibrary : public SfxLibrary
{
    friend class SfxScriptLibraryContainer;

    sal_Bool mbLoadedSource;
    sal_Bool mbLoadedBinary;

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
};

}   // namespace base

#endif

