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
#ifndef INCLUDED_SVX_GALLERYITEM_HXX
#define INCLUDED_SVX_GALLERYITEM_HXX

#include <svx/svxdllapi.h>
#include <svl/poolitem.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XComponent.hpp>

// property names map those from css::gallery::GalleryItem
// with exception of "AsLink" and "FilterName"
#define SVXGALLERYITEM_TYPE     "GalleryItemType"
#define SVXGALLERYITEM_URL      "URL"
#define SVXGALLERYITEM_FILTER   "FilterName"
#define SVXGALLERYITEM_DRAWING  "Drawing"
#define SVXGALLERYITEM_GRAPHIC  "Graphic"
#define SVXGALLERYITEM_PARAMS   5
#define SVXGALLERYITEM_ARGNAME  "GalleryItem"

class SVX_DLLPUBLIC SvxGalleryItem : public SfxPoolItem
{
    sal_Int8 m_nType;
    OUString m_aURL;
    css::uno::Reference< css::lang::XComponent > m_xDrawing;
    css::uno::Reference< css::graphic::XGraphic > m_xGraphic;

public:
    static SfxPoolItem* CreateDefault();

    SvxGalleryItem();
    SvxGalleryItem( const SvxGalleryItem& );
    virtual ~SvxGalleryItem() override;

    sal_Int8 GetType() const { return m_nType; }
    const OUString& GetURL() const { return m_aURL; }
    const css::uno::Reference< css::graphic::XGraphic >& GetGraphic() const { return m_xGraphic; }

    // pure virtual methods from SfxPoolItem
    virtual bool         operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    // bridge to UNO
    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
