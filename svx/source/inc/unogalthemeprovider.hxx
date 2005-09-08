/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unogalthemeprovider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:28:45 $
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

#ifndef _SVX_UNOGALTHEMEPROVIDER_HXX
#define _SVX_UNOGALTHEMEPROVIDER_HXX

#include <cppuhelper/implbase2.hxx>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_GALLERY_XGALLERYTHEMEPROVIDER_HPP_
#include <com/sun/star/gallery/XGalleryThemeProvider.hpp>
#endif

class Gallery;

namespace unogallery {

// -------------------------
// - some helper functions -
// -------------------------

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GalleryThemeProvider_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( ::com::sun::star::uno::Exception );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL GalleryThemeProvider_getSupportedServiceNames() throw();
::rtl::OUString SAL_CALL GalleryThemeProvider_getImplementationName() throw();

// -----------------
// - GalleryThemes -
// -----------------

class GalleryThemeProvider : public ::cppu::WeakImplHelper2< ::com::sun::star::lang::XInitialization,
                                                             ::com::sun::star::gallery::XGalleryThemeProvider >
{
public:

    GalleryThemeProvider();
    ~GalleryThemeProvider();

    static ::rtl::OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

    // XGalleryThemeProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::gallery::XGalleryTheme > SAL_CALL insertNewByName( const ::rtl::OUString& ThemeName ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& ThemeName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

private:

    Gallery*    mpGallery;
    sal_Bool    mbHiddenThemes;
};

}

#endif
