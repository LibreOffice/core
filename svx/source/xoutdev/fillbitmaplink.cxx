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

#include <svx/fillbitmaplink.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svl/itempool.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xdef.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/graph.hxx>

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
        GraphicObject aGrfObj(std::move(aGrf));

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

namespace
{
OUString getDeferredOriginURL(const XFillBitmapItem& rItem)
{
    const Graphic& rGrf = rItem.GetGraphicObject().GetGraphic();
    if (rGrf.GetType() == GraphicType::Default)
        return rGrf.getOriginURL();
    return OUString();
}
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
