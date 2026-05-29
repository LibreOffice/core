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
#include <svx/sdr/contact/viewcontact.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xdef.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/graph.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace
{
// Link class for fill bitmap items with remote URLs.
// DataChanged receives the fetched graphic and updates the actual
// XFillBitmapItem in the attribute pool so the drawing layer picks up
// the resolved image on the next repaint.
class FillBitmapLink final : public sfx2::SvBaseLink
{
    SfxItemPool& m_rPool;
    sfx2::LinkManager& m_rLinkMgr;
    std::function<void()> m_fnInvalidate;

public:
    FillBitmapLink(SfxItemPool& rPool, sfx2::LinkManager& rLinkMgr,
                   std::function<void()> fnInvalidate)
        : SvBaseLink(SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB)
        , m_rPool(rPool)
        , m_rLinkMgr(rLinkMgr)
        , m_fnInvalidate(std::move(fnInvalidate))
    {
    }

    virtual UpdateResult DataChanged(const OUString& rMimeType,
                                     const css::uno::Any& rValue) override
    {
        Graphic aGrf;
        if (!m_rLinkMgr.GetGraphicFromAny(rMimeType, rValue, aGrf, nullptr))
            return ERROR_GENERAL;
        if (aGrf.GetType() == GraphicType::Default)
            return ERROR_GENERAL;

        OUString aURL;
        sfx2::LinkManager::GetDisplayNames(this, nullptr, &aURL);
        GraphicObject aGrfObj(aGrf);

        // Replace the item in all attribute sets via pool surrogates
        m_rPool.iterateItemSurrogates(
            XATTR_FILLBITMAP, [&aURL, &aGrfObj](SfxItemPool::SurrogateData& rData) -> bool {
                auto& rItem = static_cast<const XFillBitmapItem&>(rData.getItem());
                const Graphic& rGrf = rItem.GetGraphicObject().GetGraphic();
                if (rGrf.GetType() == GraphicType::Default && rGrf.getOriginURL() == aURL)
                {
                    rData.setItem(std::make_unique<XFillBitmapItem>(aGrfObj));
                }
                return true;
            });

        if (m_fnInvalidate)
            m_fnInvalidate();

        return SUCCESS;
    }
};
}

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

void registerFillBitmapLinks(SfxItemPool& rPool, sfx2::LinkManager& rLinkMgr,
                             std::function<void()> fnInvalidate)
{
    for (const SfxPoolItem* pItem : rPool.GetItemSurrogates(XATTR_FILLBITMAP))
    {
        OUString aURL = getDeferredOriginURL(static_cast<const XFillBitmapItem&>(*pItem));
        if (aURL.isEmpty())
            continue;
        tools::SvRef<sfx2::SvBaseLink> xLink(new FillBitmapLink(rPool, rLinkMgr, fnInvalidate));
        rLinkMgr.InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientGraphic, aURL);
    }
}

void registerFillBitmapLinks(SdrModel& rModel, sfx2::LinkManager& rLinkMgr)
{
    registerFillBitmapLinks(rModel.GetItemPool(), rLinkMgr, [&rModel]() {
        for (sal_uInt16 nPage = 0; nPage < rModel.GetPageCount(); ++nPage)
            rModel.GetPage(nPage)->GetViewContact().flushViewObjectContacts();
        for (sal_uInt16 nPage = 0; nPage < rModel.GetMasterPageCount(); ++nPage)
            rModel.GetMasterPage(nPage)->GetViewContact().flushViewObjectContacts();
    });
}

namespace
{
// Per-host fill bitmap link for a single SdrObject. Unlike FillBitmapLink,
// which patches every pool surrogate sharing the URL, this writes the fetched
// graphic back only to its own host. It listens to the host via sdr::ObjectUser
// and asks the tracker to drop it when the host is destroyed, so it never
// outlives the object.
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
                                     const css::uno::Any& rValue) override
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
        m_rTracker.setUpdatingObject(m_pObj);
        m_pObj->SetMergedItemSetAndBroadcast(aSet);
        m_rTracker.setUpdatingObject(nullptr);
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
        for (const auto& rEntry : m_aObjLinks)
            pLinkMgr->Remove(rEntry.second.get());
    m_aObjLinks.clear();
}

void FillBitmapLinkTracker::onFillBitmapURLChanged(SdrObject& rObj, std::u16string_view rNewURL)
{
    // ignore the write-back of a graphic this tracker just resolved for rObj
    if (&rObj == m_pUpdatingObj)
        return;

    sfx2::LinkManager* pLinkMgr = m_rModel.GetLinkManager();
    if (!pLinkMgr)
        return;

    auto it = m_aObjLinks.find(&rObj);
    if (it != m_aObjLinks.end())
    {
        pLinkMgr->Remove(it->second.get());
        m_aObjLinks.erase(it);
    }
    if (rNewURL.empty())
        return;
    tools::SvRef<sfx2::SvBaseLink> xLink(new SdrObjectFillBitmapLink(*this, rObj, *pLinkMgr));
    pLinkMgr->InsertFileLink(*xLink, sfx2::SvBaseLinkObjectType::ClientGraphic, rNewURL);
    m_aObjLinks.emplace(&rObj, std::move(xLink));
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
                                     const css::uno::Any& /*rValue*/) override
    {
        if (!m_xControl.is())
            return ERROR_GENERAL;

        OUString aURL;
        sfx2::LinkManager::GetDisplayNames(this, nullptr, &aURL);

        m_xControl->setPropertyValue(u"ImageURL"_ustr, css::uno::Any(aURL));

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
