/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <FillBitmapLinkTracker.hxx>

#include <format.hxx>
#include <node.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/lnkbase.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xdef.hxx>
#include <vcl/graph.hxx>

namespace
{
// Templated link class for SwFormat/SwContentNode to get its attribute set and
// write a pool item back to it.
template <typename Host> struct HostOps;

template <> struct HostOps<SwFormat>
{
    static const SwAttrSet* getAttrSet(const SwFormat& rFormat) { return &rFormat.GetAttrSet(); }
    static void setAttr(SwFormat& rFormat, const SfxPoolItem& rItem)
    {
        rFormat.SetFormatAttr(rItem);
    }
};

template <> struct HostOps<SwContentNode>
{
    static const SwAttrSet* getAttrSet(const SwContentNode& rNode) { return rNode.GetpSwAttrSet(); }
    static void setAttr(SwContentNode& rNode, const SfxPoolItem& rItem) { rNode.SetAttr(rItem); }
};

// One link per XFillBitmapItem-owner that currently has a deferred
// XFillBitmapItem. The owner's destructor reports its death to the tracker,
// which releases the link while m_rHost is still valid, so the link itself
// observes nothing and m_rHost cannot dangle.
template <typename Host>
class SwHostFillBitmapLink final : public sfx2::SvBaseLink
{
    sw::FillBitmapLinkTracker& m_rTracker;
    Host& m_rHost;
    sfx2::LinkManager& m_rLinkMgr;

public:
    SwHostFillBitmapLink(sw::FillBitmapLinkTracker& rTracker, Host& rHost,
                         sfx2::LinkManager& rLinkMgr)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB)
        , m_rTracker(rTracker)
        , m_rHost(rHost)
        , m_rLinkMgr(rLinkMgr)
    {
    }

    UpdateResult DataChanged(const OUString& rMimeType, const css::uno::Any& rValue) override
    {
        tools::SvRef<SvBaseLink> xSelf(this);

        Graphic aGrf;
        if (!m_rLinkMgr.GetGraphicFromAny(rMimeType, rValue, aGrf, nullptr))
            return ERROR_GENERAL;
        if (aGrf.GetType() == GraphicType::Default)
            return ERROR_GENERAL;

        OUString aName;
        if (const SwAttrSet* pSet = HostOps<Host>::getAttrSet(m_rHost))
            if (const XFillBitmapItem* pItem = pSet->GetItemIfSet(XATTR_FILLBITMAP, false))
                aName = pItem->GetName();
        // Write the resolved graphic back to this one host. The write
        // re-enters onFillBitmapURLChanged with a now-resolved item, so tell
        // the tracker to ignore that notification for this host and keep us
        // registered (like the svx tracker's resolved links). Removing us
        // here instead would free the link inside SvBaseLink::Update, which
        // still reads its members after DataChanged returns.
        m_rTracker.setUpdatingHost(&m_rHost);
        HostOps<Host>::setAttr(m_rHost, XFillBitmapItem(aName, aGrf));
        m_rTracker.clearUpdatingHost();
        return SUCCESS;
    }
};
}

namespace sw
{
FillBitmapLinkTracker::FillBitmapLinkTracker(sfx2::LinkManager& rLinkMgr)
    : m_rLinkMgr(rLinkMgr)
{
}

FillBitmapLinkTracker::~FillBitmapLinkTracker()
{
    for (const auto& rEntry : m_aLinks)
        m_rLinkMgr.Remove(rEntry.second.get());
    m_aLinks.clear();
    for (const auto& rEntry : m_aNodeLinks)
        m_rLinkMgr.Remove(rEntry.second.get());
    m_aNodeLinks.clear();
}

template <typename Host>
void FillBitmapLinkTracker::onURLChangedImpl(std::map<Host*, tools::SvRef<sfx2::SvBaseLink>>& rMap,
                                             Host& rHost, std::u16string_view rNewURL)
{
    // ignore the write-back of a graphic this tracker just resolved for rHost
    if (isUpdatingHost(&rHost))
        return;

    auto it = rMap.find(&rHost);
    if (it != rMap.end())
    {
        m_rLinkMgr.Remove(it->second.get());
        rMap.erase(it);
    }
    if (rNewURL.empty())
        return;
    tools::SvRef<sfx2::SvBaseLink> xLink(new SwHostFillBitmapLink<Host>(*this, rHost, m_rLinkMgr));
    m_rLinkMgr.InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientGraphic, rNewURL);
    rMap.emplace(&rHost, std::move(xLink));
}

void FillBitmapLinkTracker::onFillBitmapURLChanged(SwFormat& rFormat, std::u16string_view rNewURL)
{
    onURLChangedImpl(m_aLinks, rFormat, rNewURL);
}

void FillBitmapLinkTracker::onFillBitmapURLChanged(SwContentNode& rNode, std::u16string_view rNewURL)
{
    onURLChangedImpl(m_aNodeLinks, rNode, rNewURL);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
