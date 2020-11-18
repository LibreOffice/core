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

#include <vector>

#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/gallery/XGalleryTheme.hpp>

class Gallery;
class GalleryTheme;
struct GalleryObject;
namespace unogallery { class GalleryItem; }

namespace unogallery {


class GalleryTheme : public ::cppu::WeakImplHelper<
                                                        css::gallery::XGalleryTheme,
                                                        css::lang::XServiceInfo >,
                     public SfxListener
{
    friend class ::unogallery::GalleryItem;

public:

    explicit GalleryTheme( std::u16string_view rThemeName );
    virtual ~GalleryTheme() override;

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;

    // XGalleryThemes
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL update(  ) override;
    virtual ::sal_Int32 SAL_CALL insertURLByIndex( const OUString& URL, ::sal_Int32 Index ) override;
    virtual ::sal_Int32 SAL_CALL insertGraphicByIndex( const css::uno::Reference< css::graphic::XGraphic >& Graphic, ::sal_Int32 Index ) override;
    virtual ::sal_Int32 SAL_CALL insertDrawingByIndex( const css::uno::Reference< css::lang::XComponent >& Drawing, ::sal_Int32 Index ) override;
    virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) override;

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

private:

    typedef ::std::vector< ::unogallery::GalleryItem* > GalleryItemVector;

    GalleryItemVector maItemVector;
    ::Gallery*      mpGallery;
    ::GalleryTheme* mpTheme;

    ::GalleryTheme* implGetTheme() const { return mpTheme;}

    void            implReleaseItems( GalleryObject const * pObj );

    void            implRegisterGalleryItem( ::unogallery::GalleryItem& rItem );
    void            implDeregisterGalleryItem( ::unogallery::GalleryItem& rItem );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
