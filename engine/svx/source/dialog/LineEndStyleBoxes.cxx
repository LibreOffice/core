/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <LineEndStyleBoxes.hxx>

#include <sfx2/objsh.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/drawitem.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/unoapi.hxx>
#include <svx/xdef.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xtable.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/kit.hxx>
#include <vcl/bitmap.hxx>

#include <utility>

namespace
{
// the arrow styles offered by the boxes, by their API name
constexpr OUString ARROW_STYLES[]
    = { u"Arrow"_ustr,  u"Line"_ustr,      u"Circle"_ustr,
        u"Square"_ustr, u"Square 45"_ustr, u"Diamond"_ustr };

constexpr OUString NONE_ID = u"none"_ustr;
constexpr OUString MORE_STYLES_ID = u"more"_ustr;

const basegfx::B2DPolyPolygon& getLineEndValue(const XLineStartItem& rItem)
{
    return rItem.GetLineStartValue();
}
const basegfx::B2DPolyPolygon& getLineEndValue(const XLineEndItem& rItem)
{
    return rItem.GetLineEndValue();
}
}

namespace svx
{
LineEndStyleBoxes::LineEndStyleBoxes(std::unique_ptr<weld::Label> xStartLabel,
                                     std::unique_ptr<weld::ComboBox> xStartBox,
                                     std::unique_ptr<weld::Label> xEndLabel,
                                     std::unique_ptr<weld::ComboBox> xEndBox,
                                     std::function<void(const XLineStartItem&)> aSetLineStart,
                                     std::function<void(const XLineEndItem&)> aSetLineEnd,
                                     css::uno::Reference<css::frame::XFrame> xFrame)
    : maStart{ std::move(xStartLabel), std::move(xStartBox), {} }
    , maEnd{ std::move(xEndLabel), std::move(xEndBox), {} }
    , maSetLineStart(std::move(aSetLineStart))
    , maSetLineEnd(std::move(aSetLineEnd))
    , mxFrame(std::move(xFrame))
{
    maStart.mxBox->connect_changed(LINK(this, LineEndStyleBoxes, ChangeStartHdl));
    maEnd.mxBox->connect_changed(LINK(this, LineEndStyleBoxes, ChangeEndHdl));

    maStart.mxBox->connect_custom_render(LINK(this, LineEndStyleBoxes, RenderStartHdl));
    maEnd.mxBox->connect_custom_render(LINK(this, LineEndStyleBoxes, RenderEndHdl));
    for (End* pEnd : { &maStart, &maEnd })
    {
        pEnd->mxBox->connect_custom_get_size(LINK(this, LineEndStyleBoxes, GetSizeHdl));
        pEnd->mxBox->set_custom_renderer(true);
        pEnd->mxBox->set_render_selected_entry(true);
    }
}

LineEndStyleBoxes::~LineEndStyleBoxes() {}

void LineEndStyleBoxes::set_sensitive(bool bSensitive)
{
    for (End* pEnd : { &maStart, &maEnd })
        pEnd->mxBox->set_sensitive(bSensitive);
}

void LineEndStyleBoxes::set_visible(bool bVisible)
{
    for (End* pEnd : { &maStart, &maEnd })
    {
        pEnd->mxLabel->set_visible(bVisible);
        pEnd->mxBox->set_visible(bVisible);
    }
}

void LineEndStyleBoxes::FillBox(End& rEnd, tools::Long nExtraStyle)
{
    weld::ComboBox& rBox = *rEnd.mxBox;

    rBox.freeze();
    rBox.clear();

    rBox.append(NONE_ID, comphelper::COKit::isActive() ? SvxResId(RID_SVXSTR_INVISIBLE)
                                                       : SvxResId(RID_SVXSTR_NONE));

    if (mxLineEndList.is())
    {
        const tools::Long nCount = mxLineEndList->Count();
        for (const OUString& rApiName : ARROW_STYLES)
        {
            for (tools::Long i = 0; i < nCount; ++i)
            {
                const OUString& rName = mxLineEndList->GetLineEnd(i)->GetName();
                if (SvxUnogetApiNameForItem(sal_uInt16(XATTR_LINEEND), rName) != rApiName)
                    continue;

                rBox.append(OUString::number(i), rName);
                break;
            }
        }

        // The applied style when it is not one of the offered ones: it is
        // there to be shown as a preview like the rest, but it stays out of
        // the list, which offers the six styles and nothing else.
        if (nExtraStyle >= 0 && nExtraStyle < mxLineEndList->Count()
            && rBox.find_id(OUString::number(nExtraStyle)) == -1)
        {
            rBox.append(OUString::number(nExtraStyle),
                        mxLineEndList->GetLineEnd(nExtraStyle)->GetName());
            rBox.set_entry_hidden(rBox.get_count() - 1, true);
        }
    }

    rBox.append_separator(OUString());
    rBox.append(MORE_STYLES_ID, SvxResId(RID_SVXSTR_MORE_STYLES));

    rBox.thaw();
}

void LineEndStyleBoxes::SelectEntry(End& rEnd)
{
    weld::ComboBox& rBox = *rEnd.mxBox;
    const std::optional<basegfx::B2DPolyPolygon>& roPolygon = rEnd.moPolygon;

    if (roPolygon && !roPolygon->count())
    {
        rBox.set_active(rBox.find_id(NONE_ID));
        rBox.save_value();
        return;
    }

    if (roPolygon && mxLineEndList.is())
    {
        const tools::Long nCount = mxLineEndList->Count();
        for (tools::Long i = 0; i < nCount; ++i)
        {
            const XLineEndEntry* pEntry = mxLineEndList->GetLineEnd(i);
            if (pEntry->GetLineEnd() != *roPolygon)
                continue;

            if (rBox.find_id(OUString::number(i)) == -1)
                FillBox(rEnd, i);

            rBox.set_active(rBox.find_id(OUString::number(i)));
            rBox.save_value();
            return;
        }
    }

    rBox.set_active(-1);
    rBox.set_entry_text(OUString());
    rBox.save_value();
}

const XLineEndEntry* LineEndStyleBoxes::GetSelected(const End& rEnd) const
{
    if (!mxLineEndList.is())
        return nullptr;

    const OUString sId = rEnd.mxBox->get_active_id();
    if (sId.isEmpty() || sId == NONE_ID || sId == MORE_STYLES_ID)
        return nullptr;

    const tools::Long nIndex = sId.toInt32();
    if (nIndex < 0 || nIndex >= mxLineEndList->Count())
        return nullptr;

    return mxLineEndList->GetLineEnd(nIndex);
}

template <class ItemType>
void LineEndStyleBoxes::UpdateEnd(End& rEnd, bool bSetOrDefault, const SfxPoolItem* pState)
{
    rEnd.moPolygon.reset();
    if (bSetOrDefault)
    {
        if (const ItemType* pItem = dynamic_cast<const ItemType*>(pState))
            rEnd.moPolygon = getLineEndValue(*pItem);
    }
    SelectEntry(rEnd);
}

bool LineEndStyleBoxes::NotifyItemUpdate(sal_uInt16 nSId, SfxItemState eState,
                                        const SfxPoolItem* pState)
{
    const bool bSetOrDefault = eState >= SfxItemState::DEFAULT;

    switch (nSId)
    {
        case SID_ATTR_LINE_START:
        case SID_ATTR_LINE_END:
            if (!mxLineEndList.is())
                updateLineEndList();

            if (SID_ATTR_LINE_START == nSId)
                updateLineStart(bSetOrDefault, pState);
            else
                updateLineEnd(bSetOrDefault, pState);
            return true;

        case SID_LINEEND_LIST:
            updateLineEndList();
            return true;
    }

    return false;
}

void LineEndStyleBoxes::updateLineStart(bool bSetOrDefault, const SfxPoolItem* pState)
{
    UpdateEnd<XLineStartItem>(maStart, bSetOrDefault, pState);
}

void LineEndStyleBoxes::updateLineEnd(bool bSetOrDefault, const SfxPoolItem* pState)
{
    UpdateEnd<XLineEndItem>(maEnd, bSetOrDefault, pState);
}

void LineEndStyleBoxes::updateLineEndList()
{
    // The line-end list is not delivered through the ControllerItem's item
    // state (pState is null for list slots)
    if (const SfxObjectShell* pSh = SfxObjectShell::Current())
    {
        if (const SvxLineEndListItem* pItem = pSh->GetItem(SID_LINEEND_LIST))
            mxLineEndList = pItem->GetLineEndList();
    }

    for (End* pEnd : { &maStart, &maEnd })
    {
        FillBox(*pEnd);
        // reselect according to the last known arrow of that end
        SelectEntry(*pEnd);
    }
}

void LineEndStyleBoxes::RenderEntry(const weld::ComboBox::render_args& rArgs, bool bStart)
{
    const OUString& rId = std::get<3>(rArgs);

    if (rId == MORE_STYLES_ID)
        return;

    Bitmap aBitmap;
    if (rId != NONE_ID && mxLineEndList.is())
    {
        const tools::Long nIndex = rId.toInt32();
        if (nIndex >= 0 && nIndex < mxLineEndList->Count())
            aBitmap = mxLineEndList->GetUiBitmap(nIndex);
    }

    SvxLineEndLB::RenderPreview(std::get<0>(rArgs), std::get<1>(rArgs), aBitmap, bStart);
}

IMPL_LINK(LineEndStyleBoxes, RenderStartHdl, weld::ComboBox::render_args, aArgs, void)
{
    RenderEntry(aArgs, true);
}

IMPL_LINK(LineEndStyleBoxes, RenderEndHdl, weld::ComboBox::render_args, aArgs, void)
{
    RenderEntry(aArgs, false);
}

IMPL_LINK(LineEndStyleBoxes, GetSizeHdl, vcl::RenderContext&, rDevice, Size)
{
    return SvxLineEndLB::GetPreviewSize(rDevice, mxLineEndList);
}

template <class ItemType> std::optional<ItemType> LineEndStyleBoxes::Pick(End& rEnd)
{
    const OUString sId = rEnd.mxBox->get_active_id();

    if (sId == MORE_STYLES_ID)
    {
        SelectEntry(rEnd);
        comphelper::dispatchCommand(u".uno:FormatLine"_ustr, mxFrame, {});
        return {};
    }

    basegfx::B2DPolyPolygon aPolygon;
    std::optional<ItemType> oItem;

    if (sId == NONE_ID)
        oItem.emplace();
    else if (const XLineEndEntry* pLineEnd = GetSelected(rEnd))
    {
        aPolygon = pLineEnd->GetLineEnd();
        oItem.emplace(pLineEnd->GetName(), aPolygon);
    }

    if (oItem)
    {
        rEnd.moPolygon = aPolygon;
        rEnd.mxBox->save_value();
    }

    return oItem;
}

IMPL_LINK_NOARG(LineEndStyleBoxes, ChangeStartHdl, weld::ComboBox&, void)
{
    const std::optional<XLineStartItem> oItem = Pick<XLineStartItem>(maStart);
    if (oItem)
        maSetLineStart(*oItem);
}

IMPL_LINK_NOARG(LineEndStyleBoxes, ChangeEndHdl, weld::ComboBox&, void)
{
    const std::optional<XLineEndItem> oItem = Pick<XLineEndItem>(maEnd);
    if (oItem)
        maSetLineEnd(*oItem);
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
