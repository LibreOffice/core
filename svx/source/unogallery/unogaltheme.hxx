/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unogaltheme.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:14:33 $
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

#ifndef _SVX_UNOGALTHEME_HXX
#define _SVX_UNOGALTHEME_HXX

#include <list>

#include <cppuhelper/implbase1.hxx>
#include <svtools/lstner.hxx>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_GALLERY_XGALLERYTHEME_HPP_
#include <com/sun/star/gallery/XGalleryTheme.hpp>
#endif

class Gallery;
class GalleryTheme;
struct GalleryObject;
namespace unogallery { class GalleryItem; }

namespace unogallery {

// ----------------
// - GalleryTheme -
// ----------------

class GalleryTheme : public ::cppu::WeakImplHelper1< ::com::sun::star::gallery::XGalleryTheme >,
                     public SfxListener
{
    friend class ::unogallery::GalleryItem;

public:

    GalleryTheme( const ::rtl::OUString& rThemeName );
    ~GalleryTheme();

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

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XGalleryThemes
    virtual ::rtl::OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL update(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL insertURLByIndex( const ::rtl::OUString& URL, ::sal_Int32 Index ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL insertGraphicByIndex( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& Graphic, ::sal_Int32 Index ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL insertDrawingByIndex( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& Drawing, ::sal_Int32 Index ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

private:

    typedef ::std::list< ::unogallery::GalleryItem* > GalleryItemList;

    GalleryItemList maItemList;
    ::Gallery*      mpGallery;
    ::GalleryTheme* mpTheme;

    ::GalleryTheme* implGetTheme() const;

    void            implReleaseItems( GalleryObject* pObj );

    void            implRegisterGalleryItem( ::unogallery::GalleryItem& rItem );
    void            implDeregisterGalleryItem( ::unogallery::GalleryItem& rItem );
};

}

#endif
