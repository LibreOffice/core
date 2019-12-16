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

#include <avmedia/mediaitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <rtl/ustrbuf.hxx>

#include <ucbhelper/content.hxx>

using namespace ::com::sun::star;

namespace avmedia
{
MediaItem::MediaItem( sal_uInt16 i_nWhich, AVMediaSetMask )
    : SfxPoolItem( i_nWhich )
{
}

MediaItem::MediaItem( const MediaItem& rItem )
    : SfxPoolItem( rItem )
{
}

MediaItem::~MediaItem()
{
}

struct MediaItem::Impl
{
};

bool MediaItem::QueryValue( css::uno::Any&, sal_uInt8 ) const
{
    return false;
}

bool MediaItem::GetPresentation( SfxItemPresentation, MapUnit, MapUnit, OUString&, const IntlWrapper& ) const
{
    return false;
}

bool MediaItem::PutValue( const css::uno::Any&, sal_uInt8 )
{
  return false;
}

AVMediaSetMask MediaItem::getMaskSet() const
{
    return AVMediaSetMask::NONE;
}

SfxPoolItem* MediaItem::CreateDefault()
{
    return new MediaItem;
}


MediaItem* MediaItem::Clone( SfxItemPool*) const
{
  return nullptr;
}

bool MediaItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem)); (void)rItem;
    return false;
}

} // namespace avmedia

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_framework_SoundHandler_get_implementation(css::uno::XComponentContext*,
                                                            css::uno::Sequence<css::uno::Any> const &)
{
    return NULL;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
