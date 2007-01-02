/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgcont.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:40:39 $
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

#ifndef BASIC_DIALOGCONTAINER_HXX
#define BASIC_DIALOGCONTAINER_HXX

#ifndef BASIC_NAMECONTAINER_HXX
#include "namecont.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_XSTRINGRESOURCESUPPLIER_HPP_
#include <com/sun/star/resource/XStringResourceSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_RESOURCE_XSTRINGRESOURCEPERSISTANCE_HPP_
#include "com/sun/star/resource/XStringResourcePersistance.hpp"
#endif

#include <cppuhelper/implbase1.hxx>

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

    virtual void implSetStorage( const ::com::sun::star::uno::Reference
        < ::com::sun::star::embed::XStorage >& xStorage );

    sal_Bool init( const ::rtl::OUString& aInitialisationParam,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage =
                            ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >() );
    protected:
        using SfxLibraryContainer::init;

public:
    SfxDialogLibraryContainer( void );
    SfxDialogLibraryContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    virtual void SAL_CALL storeLibrariesToStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    // Methods XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception,
                   ::com::sun::star::uno::RuntimeException);

    // Resource handling
    ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourcePersistance >
        implCreateStringResource( class SfxDialogLibrary* pDialog );

    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static();
    static ::rtl::OUString getImplementationName_static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );
};

//============================================================================

typedef ::cppu::ImplInheritanceHelper1<
        SfxLibrary, ::com::sun::star::resource::XStringResourceSupplier> SfxDialogLibrary_BASE;

class SfxDialogLibrary : public SfxDialogLibrary_BASE
{
    SfxDialogLibraryContainer*                                      m_pParent;
    ::com::sun::star::uno::Reference
        < ::com::sun::star::resource::XStringResourcePersistance>   m_xStringResourcePersistance;
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
        const ::rtl::OUString& aName,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF,
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSFI,
        SfxDialogLibraryContainer* pParent
    );

    SfxDialogLibrary
    (
        const ::rtl::OUString& aName,
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF,
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSFI,
        const ::rtl::OUString& aLibInfoFileURL, const ::rtl::OUString& aStorageURL, sal_Bool ReadOnly,
        SfxDialogLibraryContainer* pParent
    );

    // XStringResourceSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver >
        SAL_CALL getStringResource(  ) throw (::com::sun::star::uno::RuntimeException);

    ::rtl::OUString getName( void )
        { return m_aName; }

    ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourcePersistance >
        getStringResourcePersistance( void )
    {
        return m_xStringResourcePersistance;
    }
};

}   // namespace basic

#endif

