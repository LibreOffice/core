/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <tools/ref.hxx>
#include <map>
#include <string_view>
#include <variant>
#include <vector>

class SdrModel;
class SdrObject;
class SdrPage;
class SfxItemPool;
class SfxStyleSheet;
class XFillBitmapItem;

namespace com::sun::star::beans
{
class XPropertySet;
}

namespace sfx2
{
class LinkManager;
}

// Returns the origin URL if rItem carries a deferred graphic
// (GraphicType::Default with non-empty originURL), otherwise empty.
SVXCORE_DLLPUBLIC OUString getDeferredOriginURL(const XFillBitmapItem& rItem);

// Returns true if the pool contains any XATTR_FILLBITMAP items with
// unresolved remote URLs (GraphicType::Default with non-empty originURL).
SVXCORE_DLLPUBLIC bool hasDeferredFillBitmapLinks(const SfxItemPool& rPool);

namespace sdr
{
// Owns one sfx2::SvBaseLink per host (SdrObject or SdrPage background) that
// currently holds a deferred remote XFillBitmapItem, registered as soon as the
// item is set on the host (SdrObject via AttributeProperties::ItemChange,
// SdrPage via SdrPageProperties::PutItem) rather than by a later pool scan. A
// model that wants this enables it by holding a tracker; host attribute changes
// then route through onFillBitmapURLChanged. Each link writes the fetched
// graphic back only to its own host and is bound to the host lifetime via
// sdr::ObjectUser / sdr::PageUser, so it never outlives the host and the entry
// can be updated or broken individually in Edit, Links to External Files.
class SVXCORE_DLLPUBLIC FillBitmapLinkTracker
{
public:
    explicit FillBitmapLinkTracker(SdrModel& rModel);
    ~FillBitmapLinkTracker();

    // rNewURL is the deferred origin URL of the new item, or empty when the
    // item is absent, already resolved, or the host is going away. The
    // SfxStyleSheet host covers a fill shared by every object or page using
    // that style (e.g. an Impress slide background on the Background style),
    // the drawing-layer counterpart of a fill on a Writer paragraph style.
    void onFillBitmapURLChanged(SdrObject& rObj, std::u16string_view rNewURL);
    void onFillBitmapURLChanged(SdrPage& rPage, std::u16string_view rNewURL);
    void onFillBitmapURLChanged(SfxStyleSheet& rStyle, std::u16string_view rNewURL);

    // Suppress the onFillBitmapURLChanged that a link's own write-back of the
    // resolved graphic would otherwise raise for its host. The host is held by
    // its own type so a write-back is matched only against the same host type.
    template <typename Host> void setUpdatingHost(Host* pHost) { m_aUpdatingHost = pHost; }
    void clearUpdatingHost() { m_aUpdatingHost = std::monostate{}; }

private:
    template <typename Host, typename Link>
    void onURLChangedImpl(std::map<Host*, tools::SvRef<sfx2::SvBaseLink>>& rMap, Host& rHost,
                          std::u16string_view rNewURL);

    template <typename Host> bool isUpdatingHost(Host* pHost) const
    {
        auto const* ppHost = std::get_if<Host*>(&m_aUpdatingHost);
        return ppHost && *ppHost == pHost;
    }

    SdrModel& m_rModel;
    /// std::monostate is there to indicate no host is being updated
    std::variant<std::monostate, SdrObject*, SdrPage*, SfxStyleSheet*> m_aUpdatingHost;
    std::map<SdrObject*, tools::SvRef<sfx2::SvBaseLink>> m_aObjLinks;
    std::map<SdrPage*, tools::SvRef<sfx2::SvBaseLink>> m_aPageLinks;
    std::map<SfxStyleSheet*, tools::SvRef<sfx2::SvBaseLink>> m_aStyleLinks;
};
}

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
