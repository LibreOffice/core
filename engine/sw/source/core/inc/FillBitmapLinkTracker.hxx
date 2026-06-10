/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_FILLBITMAPLINKTRACKER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FILLBITMAPLINKTRACKER_HXX

#include <rtl/ustring.hxx>
#include <tools/ref.hxx>
#include <map>
#include <string_view>
#include <variant>

class SwContentNode;
class SwFormat;

namespace sfx2
{
class LinkManager;
class SvBaseLink;
}

namespace sw
{
// Owns one sfx2::SvBaseLink per SwFormat or SwContentNode that currently
// holds a deferred XFillBitmapItem URL. Links are registered as soon as
// the deferred item is set.
//
// When the graphic resolves, DataChanged calls SetFormatAttr / SetAttr on the
// host with the fetched graphic. A host destroyed while the document lives
// reports its death here with an empty URL, so a link never outlives its
// host. During ~SwDoc the hosts stay silent and the tracker destructor
// releases the remaining links instead.
class FillBitmapLinkTracker final
{
public:
    explicit FillBitmapLinkTracker(sfx2::LinkManager& rLinkMgr);
    ~FillBitmapLinkTracker();

    // Called by SwFormat::SwClientNotify / SwContentNode::SwClientNotify
    // when XATTR_FILLBITMAP changes in the host's own attribute set, and
    // by the host's destructor to drop the stale entry. rNewURL is the
    // deferred origin URL of the new item, or empty when the item is
    // absent, already resolved, or the host is being destroyed.
    void onFillBitmapURLChanged(SwFormat& rFormat, std::u16string_view rNewURL);
    void onFillBitmapURLChanged(SwContentNode& rNode, std::u16string_view rNewURL);

    bool hasLinks() const { return !m_aLinks.empty() || !m_aNodeLinks.empty(); }

    // Suppress the onFillBitmapURLChanged that a link's own write-back of the
    // resolved graphic would otherwise raise for its host. The host is held by
    // its own type so a write-back is matched only against the same host type.
    template <typename Host> void setUpdatingHost(Host* pHost) { m_aUpdatingHost = pHost; }
    void clearUpdatingHost() { m_aUpdatingHost = std::monostate{}; }

private:
    template <typename Host>
    void onURLChangedImpl(std::map<Host*, tools::SvRef<sfx2::SvBaseLink>>& rMap, Host& rHost,
                          std::u16string_view rNewURL);

    template <typename Host> bool isUpdatingHost(Host* pHost) const
    {
        auto const* ppHost = std::get_if<Host*>(&m_aUpdatingHost);
        return ppHost && *ppHost == pHost;
    }

    sfx2::LinkManager& m_rLinkMgr;
    /// std::monostate is there to indicate no host is being updated
    std::variant<std::monostate, SwFormat*, SwContentNode*> m_aUpdatingHost;
    std::map<SwFormat*, tools::SvRef<sfx2::SvBaseLink>> m_aLinks;
    std::map<SwContentNode*, tools::SvRef<sfx2::SvBaseLink>> m_aNodeLinks;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
