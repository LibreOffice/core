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

#ifndef INCLUDED_SVX_SOURCE_UNOGALLERY_UNOGALTHEME_HXX
#define INCLUDED_SVX_SOURCE_UNOGALLERY_UNOGALTHEME_HXX

#include <list>

#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/gallery/XGalleryTheme.hpp>

class Gallery;
class GalleryTheme;
struct GalleryObject;
namespace unogallery { class GalleryItem; }

namespace unogallery {


// - GalleryTheme -


class GalleryTheme : public ::cppu::WeakImplHelper<
                                                        ::com::sun::star::gallery::XGalleryTheme,
                                                        ::com::sun::star::lang::XServiceInfo >,
                     public SfxListener
{
    friend class ::unogallery::GalleryItem;

public:

    explicit GalleryTheme( const OUString& rThemeName );
    virtual ~GalleryTheme();

    static OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< OUString >  getSupportedServiceNames_Static() throw();

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XGalleryThemes
    virtual OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL update(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL insertURLByIndex( const OUString& URL, ::sal_Int32 Index ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL insertGraphicByIndex( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& Graphic, ::sal_Int32 Index ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL insertDrawingByIndex( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& Drawing, ::sal_Int32 Index ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

private:

    typedef ::std::list< ::unogallery::GalleryItem* > GalleryItemList;

    GalleryItemList maItemList;
    ::Gallery*      mpGallery;
    ::GalleryTheme* mpTheme;

    ::GalleryTheme* implGetTheme() const { return mpTheme;}

    void            implReleaseItems( GalleryObject* pObj );

    void            implRegisterGalleryItem( ::unogallery::GalleryItem& rItem );
    void            implDeregisterGalleryItem( ::unogallery::GalleryItem& rItem );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
