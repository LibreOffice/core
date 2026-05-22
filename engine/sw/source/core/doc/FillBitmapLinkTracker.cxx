/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <FillBitmapLinkTracker.hxx>

#include <IDocumentLinksAdministration.hxx>
#include <doc.hxx>
#include <format.hxx>
#include <node.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/lnkbase.hxx>
#include <svl/listener.hxx>
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
// XFillBitmapItem. The link listens to the XFillBitmapItem-owner for
// SfxHintId::Dying and asks the tracker to drop it, so links never outlive
// their host.
//
// When the remote graphic resolves, DataChanged writes the fetched bitmap back
// via setAttr; XFillBitmapItem-owner fires SwClientNotify, which calls back
// through onFillBitmapURLChanged with an empty URL and the tracker removes us.
template <typename Host>
class SwHostFillBitmapLink final : public sfx2::SvBaseLink, public SvtListener
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
        StartListening(rHost.GetNotifier());
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
        HostOps<Host>::setAttr(m_rHost, XFillBitmapItem(aName, aGrf));
        return SUCCESS;
    }

    void Notify(const SfxHint& rHint) override
    {
        if (rHint.GetId() != SfxHintId::Dying)
            return;
        tools::SvRef<SvBaseLink> xSelf(this);
        m_rTracker.onFillBitmapURLChanged(m_rHost, OUString());
    }
};
}

namespace sw
{
FillBitmapLinkTracker::FillBitmapLinkTracker(SwDoc& rDoc)
    : m_rDoc(rDoc)
{
}

FillBitmapLinkTracker::~FillBitmapLinkTracker()
{
    sfx2::LinkManager& rLinkMgr = m_rDoc.getIDocumentLinksAdministration().GetLinkManager();
    for (const auto& rEntry : m_aLinks)
        rLinkMgr.Remove(rEntry.second.get());
    m_aLinks.clear();
    for (const auto& rEntry : m_aNodeLinks)
        rLinkMgr.Remove(rEntry.second.get());
    m_aNodeLinks.clear();
}

template <typename Host>
void FillBitmapLinkTracker::onURLChangedImpl(std::map<Host*, tools::SvRef<sfx2::SvBaseLink>>& rMap,
                                             Host& rHost, std::u16string_view rNewURL)
{
    sfx2::LinkManager& rLinkMgr = m_rDoc.getIDocumentLinksAdministration().GetLinkManager();
    auto it = rMap.find(&rHost);
    if (it != rMap.end())
    {
        rLinkMgr.Remove(it->second.get());
        rMap.erase(it);
    }
    if (rNewURL.empty())
        return;
    tools::SvRef<sfx2::SvBaseLink> xLink(new SwHostFillBitmapLink<Host>(*this, rHost, rLinkMgr));
    rLinkMgr.InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientGraphic, rNewURL);
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
