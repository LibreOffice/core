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

#ifndef _SVX_UNOGALTHEME_HXX
#define _SVX_UNOGALTHEME_HXX

#include <list>

#include <cppuhelper/implbase1.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/gallery/XGalleryTheme.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
