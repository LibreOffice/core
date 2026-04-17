/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVX_FILLBITMAPLINK_HXX
#define INCLUDED_SVX_FILLBITMAPLINK_HXX

#include <svx/svxdllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <functional>
#include <vector>

class SdrModel;
class SfxItemPool;

namespace com::sun::star::beans
{
class XPropertySet;
}

namespace sfx2
{
class LinkManager;
}

// Returns true if the pool contains any XATTR_FILLBITMAP items with
// unresolved remote URLs (GraphicType::Default with non-empty originURL).
SVXCORE_DLLPUBLIC bool hasDeferredFillBitmapLinks(const SfxItemPool& rPool);

// Scan pool surrogates for XATTR_FILLBITMAP items with unresolved remote
// URLs (GraphicType::Default with non-empty originURL) and register an
// sfx2::SvBaseLink for each. When the link is updated, the fetched graphic
// replaces the pool item and fnInvalidate is called to trigger a repaint.
SVXCORE_DLLPUBLIC void registerFillBitmapLinks(SfxItemPool& rPool, sfx2::LinkManager& rLinkMgr,
                                               std::function<void()> fnInvalidate);

// Convenience overload for the drawing layer: registers fill bitmap links
// and flushes all ViewObjectContacts on all pages when the graphic arrives.
SVXCORE_DLLPUBLIC void registerFillBitmapLinks(SdrModel& rModel, sfx2::LinkManager& rLinkMgr);

// Register links for form controls with deferred remote image URLs.
// Each entry is a (weak control reference, URL) pair. Dead references
// are silently skipped. When updated, ImageURL is set on the control
// to trigger the fetch.
SVXCORE_DLLPUBLIC void registerDeferredFormImageLinks(
    const std::vector<std::pair<css::uno::WeakReference<css::beans::XPropertySet>, OUString>>&
        rEntries,
    sfx2::LinkManager& rLinkMgr);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
