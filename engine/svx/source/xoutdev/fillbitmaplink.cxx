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

#include <svx/fillbitmaplink.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdrobjectuser.hxx>
#include <svx/sdrpageuser.hxx>
#include <svl/hint.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/lstner.hxx>
#include <svl/style.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xdef.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/graph.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

OUString getDeferredOriginURL(const XFillBitmapItem& rItem)
{
    const Graphic& rGrf = rItem.GetGraphicObject().GetGraphic();
    if (rGrf.GetType() == GraphicType::Default)
        return rGrf.getOriginURL();
    return OUString();
}

bool hasDeferredFillBitmapLinks(const SfxItemPool& rPool)
{
    for (const SfxPoolItem* pItem : rPool.GetItemSurrogates(XATTR_FILLBITMAP))
    {
        if (!getDeferredOriginURL(static_cast<const XFillBitmapItem&>(*pItem)).isEmpty())
            return true;
    }
    return false;
}

namespace
{
// Per-host fill bitmap link. Unlike FillBitmapLink, which patches every pool
// surrogate sharing the URL, this writes the fetched graphic back only to its
// own host. It listens to the host (SdrObject via sdr::ObjectUser, SdrPage via
// sdr::PageUser) and asks the tracker to drop it when the host is destroyed, so
// it never outlives the host.
class SdrObjectFillBitmapLink final : public sfx2::SvBaseLink, public sdr::ObjectUser
{
    sdr::FillBitmapLinkTracker& m_rTracker;
    SdrObject* m_pObj;
    sfx2::LinkManager& m_rLinkMgr;

public:
    SdrObjectFillBitmapLink(sdr::FillBitmapLinkTracker& rTracker, SdrObject& rObj,
                            sfx2::LinkManager& rLinkMgr)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB)
        , m_rTracker(rTracker)
        , m_pObj(&rObj)
        , m_rLinkMgr(rLinkMgr)
    {
        m_pObj->AddObjectUser(*this);
    }

    virtual ~SdrObjectFillBitmapLink() override
    {
        if (m_pObj)
            m_pObj->RemoveObjectUser(*this);
    }

    virtual UpdateResult DataChanged(const OUString& rMimeType,
                                     const cpo::uno::Any& rValue) override
    {
        tools::SvRef<SvBaseLink> xSelf(this);

        if (!m_pObj)
            return ERROR_GENERAL;

        Graphic aGrf;
        if (!m_rLinkMgr.GetGraphicFromAny(rMimeType, rValue, aGrf, nullptr))
            return ERROR_GENERAL;
        if (aGrf.GetType() == GraphicType::Default)
            return ERROR_GENERAL;

        // Write the resolved graphic back to this one host. The write re-enters
        // AttributeProperties::ItemChange with a now-resolved item, so tell the
        // tracker to ignore the resulting empty-URL notification for this host
        // and keep us registered (like the other resolved links).
        const OUString aName = m_pObj->GetMergedItem(XATTR_FILLBITMAP).GetName();
        SfxItemSetFixed<XATTR_FILLBITMAP, XATTR_FILLBITMAP> aSet(m_pObj->GetObjectItemPool());
        aSet.Put(XFillBitmapItem(aName, aGrf));
        m_rTracker.setUpdatingHost(m_pObj);
        m_pObj->SetMergedItemSetAndBroadcast(aSet);
        m_rTracker.clearUpdatingHost();
        return SUCCESS;
    }

    virtual void ObjectInDestruction(const SdrObject& rObject) override
    {
        // do not RemoveObjectUser here, the host clears its list itself
        m_pObj = nullptr;
        tools::SvRef<SvBaseLink> xSelf(this);
        m_rTracker.onFillBitmapURLChanged(const_cast<SdrObject&>(rObject), u"");
    }
};

// Page-background counterpart of SdrObjectFillBitmapLink, bound to the page via
// sdr::PageUser and writing the resolved graphic back through SdrPageProperties.
class SdrPageFillBitmapLink final : public sfx2::SvBaseLink, public sdr::PageUser
{
    sdr::FillBitmapLinkTracker& m_rTracker;
    SdrPage* m_pPage;
    sfx2::LinkManager& m_rLinkMgr;

public:
    SdrPageFillBitmapLink(sdr::FillBitmapLinkTracker& rTracker, SdrPage& rPage,
                          sfx2::LinkManager& rLinkMgr)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB)
        , m_rTracker(rTracker)
        , m_pPage(&rPage)
        , m_rLinkMgr(rLinkMgr)
    {
        m_pPage->AddPageUser(*this);
    }

    virtual ~SdrPageFillBitmapLink() override
    {
        if (m_pPage)
            m_pPage->RemovePageUser(*this);
    }

    virtual UpdateResult DataChanged(const OUString& rMimeType,
                                     const cpo::uno::Any& rValue) override
    {
        tools::SvRef<SvBaseLink> xSelf(this);

        if (!m_pPage)
            return ERROR_GENERAL;

        Graphic aGrf;
        if (!m_rLinkMgr.GetGraphicFromAny(rMimeType, rValue, aGrf, nullptr))
            return ERROR_GENERAL;
        if (aGrf.GetType() == GraphicType::Default)
            return ERROR_GENERAL;

        SdrPageProperties& rProps = m_pPage->getSdrPageProperties();
        OUString aName;
        if (const XFillBitmapItem* pItem
            = rProps.GetItemSet().GetItemIfSet(XATTR_FILLBITMAP, false))
            aName = pItem->GetName();
        m_rTracker.setUpdatingHost(m_pPage);
        rProps.PutItem(XFillBitmapItem(aName, aGrf));
        m_rTracker.clearUpdatingHost();
        return SUCCESS;
    }

    virtual void PageInDestruction(const SdrPage& rPage) override
    {
        // do not RemovePageUser here, the host clears its list itself
        m_pPage = nullptr;
        tools::SvRef<SvBaseLink> xSelf(this);
        m_rTracker.onFillBitmapURLChanged(const_cast<SdrPage&>(rPage), u"");
    }
};

// Style-sheet counterpart, for a fill shared by every host using the style
// (e.g. an Impress slide background on the Background pseudo-style). Listens to
// the style sheet's broadcaster for SfxHintId::Dying and writes the resolved
// graphic back into the style sheet's item set so all users repaint.
class SdrStyleSheetFillBitmapLink final : public sfx2::SvBaseLink, public SfxListener
{
    sdr::FillBitmapLinkTracker& m_rTracker;
    SfxStyleSheet* m_pStyle;
    sfx2::LinkManager& m_rLinkMgr;

public:
    SdrStyleSheetFillBitmapLink(sdr::FillBitmapLinkTracker& rTracker, SfxStyleSheet& rStyle,
                                sfx2::LinkManager& rLinkMgr)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB)
        , m_rTracker(rTracker)
        , m_pStyle(&rStyle)
        , m_rLinkMgr(rLinkMgr)
    {
        StartListening(rStyle);
    }

    virtual UpdateResult DataChanged(const OUString& rMimeType,
                                     const cpo::uno::Any& rValue) override
    {
        tools::SvRef<SvBaseLink> xSelf(this);

        if (!m_pStyle)
            return ERROR_GENERAL;

        Graphic aGrf;
        if (!m_rLinkMgr.GetGraphicFromAny(rMimeType, rValue, aGrf, nullptr))
            return ERROR_GENERAL;
        if (aGrf.GetType() == GraphicType::Default)
            return ERROR_GENERAL;

        SfxItemSet& rSet = m_pStyle->GetItemSet();
        OUString aName;
        if (const XFillBitmapItem* pItem = rSet.GetItemIfSet(XATTR_FILLBITMAP, false))
            aName = pItem->GetName();
        rSet.Put(XFillBitmapItem(aName, aGrf));
        m_pStyle->Broadcast(SfxHint(SfxHintId::DataChanged));
        return SUCCESS;
    }

    virtual void Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint) override
    {
        if (rHint.GetId() != SfxHintId::Dying)
            return;
        SfxStyleSheet* pStyle = m_pStyle;
        m_pStyle = nullptr;
        tools::SvRef<SvBaseLink> xSelf(this);
        if (pStyle)
            m_rTracker.onFillBitmapURLChanged(*pStyle, u"");
    }
};
}

namespace sdr
{
FillBitmapLinkTracker::FillBitmapLinkTracker(SdrModel& rModel)
    : m_rModel(rModel)
{
}

FillBitmapLinkTracker::~FillBitmapLinkTracker()
{
    if (sfx2::LinkManager* pLinkMgr = m_rModel.GetLinkManager())
    {
        for (const auto& rEntry : m_aObjLinks)
            pLinkMgr->Remove(rEntry.second.get());
        for (const auto& rEntry : m_aPageLinks)
            pLinkMgr->Remove(rEntry.second.get());
        for (const auto& rEntry : m_aStyleLinks)
            pLinkMgr->Remove(rEntry.second.get());
    }
    m_aObjLinks.clear();
    m_aPageLinks.clear();
    m_aStyleLinks.clear();
}

template <typename Host, typename Link>
void FillBitmapLinkTracker::onURLChangedImpl(std::map<Host*, tools::SvRef<sfx2::SvBaseLink>>& rMap,
                                             Host& rHost, std::u16string_view rNewURL)
{
    // ignore the write-back of a graphic this tracker just resolved for rHost
    if (isUpdatingHost(&rHost))
        return;

    sfx2::LinkManager* pLinkMgr = m_rModel.GetLinkManager();
    if (!pLinkMgr)
        return;

    auto it = rMap.find(&rHost);
    if (it != rMap.end())
    {
        pLinkMgr->Remove(it->second.get());
        rMap.erase(it);
    }
    if (rNewURL.empty())
        return;
    tools::SvRef<sfx2::SvBaseLink> xLink(new Link(*this, rHost, *pLinkMgr));
    pLinkMgr->InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientGraphic, rNewURL);
    rMap.emplace(&rHost, std::move(xLink));
}

void FillBitmapLinkTracker::onFillBitmapURLChanged(SdrObject& rObj, std::u16string_view rNewURL)
{
    onURLChangedImpl<SdrObject, SdrObjectFillBitmapLink>(m_aObjLinks, rObj, rNewURL);
}

void FillBitmapLinkTracker::onFillBitmapURLChanged(SdrPage& rPage, std::u16string_view rNewURL)
{
    onURLChangedImpl<SdrPage, SdrPageFillBitmapLink>(m_aPageLinks, rPage, rNewURL);
}

void FillBitmapLinkTracker::onFillBitmapURLChanged(SfxStyleSheet& rStyle,
                                                   std::u16string_view rNewURL)
{
    onURLChangedImpl<SfxStyleSheet, SdrStyleSheetFillBitmapLink>(m_aStyleLinks, rStyle, rNewURL);
}
}

namespace
{
// Link class for form control images with remote URLs.
// DataChanged sets ImageURL on the control to trigger the toolkit fetch.
class FormImageLink final : public sfx2::SvBaseLink
{
    css::uno::Reference<css::beans::XPropertySet> m_xControl;

public:
    FormImageLink(css::uno::Reference<css::beans::XPropertySet> xControl)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB)
        , m_xControl(std::move(xControl))
    {
    }

    virtual UpdateResult DataChanged(const OUString& /*rMimeType*/,
                                     const cpo::uno::Any& /*rValue*/) override
    {
        if (!m_xControl.is())
            return ERROR_GENERAL;

        OUString aURL;
        sfx2::LinkManager::GetDisplayNames(this, nullptr, &aURL);

        m_xControl->setPropertyValue(u"ImageURL"_ustr, cpo::uno::Any(aURL));

        return SUCCESS;
    }
};
}

void registerDeferredFormImageLinks(
    const std::vector<std::pair<css::uno::WeakReference<css::beans::XPropertySet>, OUString>>&
        rEntries,
    sfx2::LinkManager& rLinkMgr)
{
    for (const auto & [ xWeak, aURL ] : rEntries)
    {
        css::uno::Reference<css::beans::XPropertySet> xControl(xWeak);
        if (!xControl.is() || aURL.isEmpty())
            continue;
        tools::SvRef<sfx2::SvBaseLink> xLink(new FormImageLink(xControl));
        rLinkMgr.InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientGraphic, aURL);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
