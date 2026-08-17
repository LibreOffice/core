/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <memory>
#include <svx/sidebar/LinePropertyPanelBase.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/drawitem.hxx>
#include <svx/linectrl.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/unoapi.hxx>
#include <svx/xdef.hxx>
#include <svx/xtable.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/kit.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/bitmap.hxx>

using namespace css;
using namespace css::uno;

constexpr OUString SELECTWIDTH = u"SelectWidth"_ustr;

constexpr OUString ARROW_STYLES[]
    = { u"Arrow"_ustr,  u"Line"_ustr,      u"Circle"_ustr,
        u"Square"_ustr, u"Square 45"_ustr, u"Diamond"_ustr };

constexpr OUString NONE_ID = u"none"_ustr;
constexpr OUString MORE_STYLES_ID = u"more"_ustr;

namespace svx::sidebar {

// trigger disabling the arrows if the none line style is selected
class LineStyleNoneChange
{
private:
    LinePropertyPanelBase& m_rPanel;

public:
    LineStyleNoneChange(LinePropertyPanelBase& rPanel)
        : m_rPanel(rPanel)
    {
    }

    void operator()(bool bLineStyleNone)
    {
        m_rPanel.SetNoneLineStyle(bLineStyleNone);
    }
};

namespace
{
    SvxLineStyleToolBoxControl* getLineStyleToolBoxControl(const ToolbarUnoDispatcher& rToolBoxColor)
    {
        css::uno::Reference<css::frame::XToolbarController> xController = rToolBoxColor.GetControllerForCommand(u".uno:XLineStyle"_ustr);
        SvxLineStyleToolBoxControl* pToolBoxLineStyleControl = dynamic_cast<SvxLineStyleToolBoxControl*>(xController.get());
        return pToolBoxLineStyleControl;
    }
}


LinePropertyPanelBase::LinePropertyPanelBase(
    weld::Widget* pParent,
    const uno::Reference<css::frame::XFrame>& rxFrame)
:   PanelLayout(pParent, u"LinePropertyPanel"_ustr, u"svx/ui/sidebarline.ui"_ustr),
    mxTBColor(m_xBuilder->weld_toolbar(u"color"_ustr)),
    mxColorDispatch(new ToolbarUnoDispatcher(*mxTBColor, *m_xBuilder, rxFrame)),
    mxLineStyleTB(m_xBuilder->weld_toolbar(u"linestyle"_ustr)),
    mxLineStyleDispatch(new ToolbarUnoDispatcher(*mxLineStyleTB, *m_xBuilder, rxFrame)),
    mnWidthCoreValue(0),
    mxFrame(rxFrame),
    mxFTWidth(m_xBuilder->weld_label(u"widthlabel"_ustr)),
    mxTBWidth(m_xBuilder->weld_toolbar(u"width"_ustr)),
    mxFTTransparency(m_xBuilder->weld_label(u"translabel"_ustr)),
    mxMFTransparent(m_xBuilder->weld_metric_spin_button(u"linetransparency"_ustr, FieldUnit::PERCENT)),
    mxArrowStartFT(m_xBuilder->weld_label(u"startarrowlabel"_ustr)),
    mxArrowEndFT(m_xBuilder->weld_label(u"endarrowlabel"_ustr)),
    mxLBStart(m_xBuilder->weld_combo_box(u"startarrowstyle"_ustr)),
    mxLBEnd(m_xBuilder->weld_combo_box(u"endarrowstyle"_ustr)),
    mxLineWidthPopup(new LineWidthPopup(mxTBWidth.get(), *this)),
    mxLineStyleNoneChange(new LineStyleNoneChange(*this)),
    mnTrans(0),
    meMapUnit(MapUnit::MapMM),
    mbWidthValuable(true),
    mbArrowSupported(true),
    mbNoneLineStyle(false)
{
    Initialize();
}

LinePropertyPanelBase::~LinePropertyPanelBase()
{
    mxLineWidthPopup.reset();
    mxFTWidth.reset();
    mxTBWidth.reset();
    mxColorDispatch.reset();
    mxTBColor.reset();
    mxFTTransparency.reset();
    mxMFTransparent.reset();
    mxLineStyleDispatch.reset();
    mxLineStyleTB.reset();
    mxLBStart.reset();
    mxLBEnd.reset();
    mxArrowStartFT.reset();
    mxArrowEndFT.reset();
}

void LinePropertyPanelBase::Initialize()
{
    mxTBWidth->set_item_popover(SELECTWIDTH, mxLineWidthPopup->getTopLevel());

    mxTBWidth->connect_clicked(LINK(this, LinePropertyPanelBase, ToolboxWidthSelectHdl));
    mxTBWidth->connect_menu_toggled(LINK(this, LinePropertyPanelBase, ToolboxWidthToggleMenuHdl));

    mxMFTransparent->connect_value_changed(LINK(this, LinePropertyPanelBase, ChangeTransparentHdl));

    mxLBStart->connect_changed(LINK(this, LinePropertyPanelBase, ChangeStartHdl));
    mxLBEnd->connect_changed(LINK(this, LinePropertyPanelBase, ChangeEndHdl));

    // Draw an arrow preview in front of each style name in the dropdown.
    mxLBStart->connect_custom_get_size(LINK(this, LinePropertyPanelBase, GetSizeHdl));
    mxLBEnd->connect_custom_get_size(LINK(this, LinePropertyPanelBase, GetSizeHdl));
    mxLBStart->connect_custom_render(LINK(this, LinePropertyPanelBase, RenderStartHdl));
    mxLBEnd->connect_custom_render(LINK(this, LinePropertyPanelBase, RenderEndHdl));
    mxLBStart->set_custom_renderer(true);
    mxLBEnd->set_custom_renderer(true);

    SvxLineStyleToolBoxControl* pLineStyleControl = getLineStyleToolBoxControl(*mxLineStyleDispatch);
    pLineStyleControl->setLineStyleIsNoneFunction(*mxLineStyleNoneChange);
}

void LinePropertyPanelBase::updateLineTransparence(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxFTTransparency->set_sensitive(false);
        mxMFTransparent->set_sensitive(false);
    }
    else
    {
        mxFTTransparency->set_sensitive(true);
        mxMFTransparent->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineTransparenceItem* pItem = dynamic_cast<const XLineTransparenceItem*>(pState))
        {
            mnTrans = pItem->GetValue();
            mxMFTransparent->set_value(mnTrans, FieldUnit::PERCENT);
            return;
        }
    }

    mxMFTransparent->set_value(0, FieldUnit::PERCENT);//add
    mxMFTransparent->set_text(OUString());
}

void LinePropertyPanelBase::updateLineWidth(bool bDisabled, bool bSetOrDefault,
        const SfxPoolItem* pState)
{
    if(bDisabled)
    {
        mxTBWidth->set_sensitive(false);
        mxFTWidth->set_sensitive(false);
    }
    else
    {
        mxTBWidth->set_sensitive(true);
        mxFTWidth->set_sensitive(true);
    }

    if(bSetOrDefault)
    {
        if (const XLineWidthItem* pItem = dynamic_cast<const XLineWidthItem*>(pState))
        {
            mnWidthCoreValue = pItem->GetValue();
            mbWidthValuable = true;
            return;
        }
    }

    mbWidthValuable = false;
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ToolboxWidthSelectHdl, const OUString&, void)
{
    mxTBWidth->set_menu_item_active(SELECTWIDTH, !mxTBWidth->get_menu_item_active(SELECTWIDTH));
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ToolboxWidthToggleMenuHdl, const OUString&, void)
{
    if (mxTBWidth->get_menu_item_active(SELECTWIDTH))
        mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::EndLineWidthPopup()
{
    mxTBWidth->set_menu_item_active(SELECTWIDTH, false);
}

IMPL_LINK_NOARG( LinePropertyPanelBase, ChangeTransparentHdl, weld::MetricSpinButton&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(mxMFTransparent->get_value(FieldUnit::PERCENT));
    XLineTransparenceItem aItem( nVal );

    setLineTransparency(aItem);
}

void LinePropertyPanelBase::SetWidth(tools::Long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::ActivateControls()
{
    mxLBStart->set_sensitive(!mbNoneLineStyle);
    mxLBEnd->set_sensitive(!mbNoneLineStyle);

    mxArrowStartFT->set_visible(mbArrowSupported);
    mxArrowEndFT->set_visible(mbArrowSupported);
    mxLBStart->set_visible(mbArrowSupported);
    mxLBEnd->set_visible(mbArrowSupported);
}

void LinePropertyPanelBase::FillLineEndListBox(weld::ComboBox& rListBox, tools::Long nExtraStyle)
{
    rListBox.freeze();
    rListBox.clear();

    rListBox.append(NONE_ID, comphelper::COKit::isActive() ? SvxResId(RID_SVXSTR_INVISIBLE)
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

                rListBox.append(OUString::number(i), rName);
                break;
            }
        }

        // The applied style when it is not one of the offered ones: it is
        // there to be shown as a preview like the rest, but it stays out of
        // the list, which offers the six styles and nothing else.
        if (nExtraStyle >= 0 && nExtraStyle < mxLineEndList->Count()
            && rListBox.find_id(OUString::number(nExtraStyle)) == -1)
        {
            rListBox.append(OUString::number(nExtraStyle),
                            mxLineEndList->GetLineEnd(nExtraStyle)->GetName());
            rListBox.set_entry_hidden(rListBox.get_count() - 1, true);
        }
    }

    rListBox.append_separator(OUString());
    rListBox.append(MORE_STYLES_ID, SvxResId(RID_SVXSTR_MORE_STYLES));

    rListBox.thaw();
}

void LinePropertyPanelBase::SelectLineEndEntry(
    weld::ComboBox& rListBox, const std::optional<basegfx::B2DPolyPolygon>& roPolygon)
{
    if (roPolygon && !roPolygon->count())
    {
        rListBox.set_active(rListBox.find_id(NONE_ID));
        rListBox.save_value();
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

            if (rListBox.find_id(OUString::number(i)) == -1)
                FillLineEndListBox(rListBox, i);

            rListBox.set_active(rListBox.find_id(OUString::number(i)));
            rListBox.save_value();
            return;
        }
    }

    rListBox.set_active(-1);
    rListBox.set_entry_text(OUString());
    rListBox.save_value();
}

const XLineEndEntry* LinePropertyPanelBase::GetSelectedLineEnd(const weld::ComboBox& rListBox) const
{
    if (!mxLineEndList.is())
        return nullptr;

    const OUString sId = rListBox.get_active_id();
    if (sId.isEmpty() || sId == NONE_ID || sId == MORE_STYLES_ID)
        return nullptr;

    const tools::Long nIndex = sId.toInt32();
    if (nIndex < 0 || nIndex >= mxLineEndList->Count())
        return nullptr;

    return mxLineEndList->GetLineEnd(nIndex);
}

void LinePropertyPanelBase::updateLineStart(bool /*bDisabled*/, bool bSetOrDefault,
                                            const SfxPoolItem* pState)
{
    moStartPolygon.reset();
    if (bSetOrDefault)
    {
        if (const XLineStartItem* pItem = dynamic_cast<const XLineStartItem*>(pState))
            moStartPolygon = pItem->GetLineStartValue();
    }
    SelectLineEndEntry(*mxLBStart, moStartPolygon);
}

void LinePropertyPanelBase::updateLineEnd(bool /*bDisabled*/, bool bSetOrDefault,
                                          const SfxPoolItem* pState)
{
    moEndPolygon.reset();
    if (bSetOrDefault)
    {
        if (const XLineEndItem* pItem = dynamic_cast<const XLineEndItem*>(pState))
            moEndPolygon = pItem->GetLineEndValue();
    }
    SelectLineEndEntry(*mxLBEnd, moEndPolygon);
}

void LinePropertyPanelBase::updateLineEndList(const SfxPoolItem* /*pState*/)
{
    // The line-end list is not delivered through the ControllerItem's item
    // state (pState is null for list slots); fetch it from the object shell,
    // the same way AreaPropertyPanelBase obtains the gradient/hatch lists.
    if (const SfxObjectShell* pSh = SfxObjectShell::Current())
    {
        if (const SvxLineEndListItem* pItem = pSh->GetItem(SID_LINEEND_LIST))
            mxLineEndList = pItem->GetLineEndList();
    }

    FillLineEndListBox(*mxLBStart);
    FillLineEndListBox(*mxLBEnd);

    // reselect according to the last known start/end arrows
    SelectLineEndEntry(*mxLBStart, moStartPolygon);
    SelectLineEndEntry(*mxLBEnd, moEndPolygon);
}

void LinePropertyPanelBase::RenderLineEndEntry(const weld::ComboBox::render_args& rArgs, bool bStart)
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

IMPL_LINK(LinePropertyPanelBase, RenderStartHdl, weld::ComboBox::render_args, aArgs, void)
{
    RenderLineEndEntry(aArgs, true);
}

IMPL_LINK(LinePropertyPanelBase, RenderEndHdl, weld::ComboBox::render_args, aArgs, void)
{
    RenderLineEndEntry(aArgs, false);
}

IMPL_LINK(LinePropertyPanelBase, GetSizeHdl, vcl::RenderContext&, rDevice, Size)
{
    return SvxLineEndLB::GetPreviewSize(rDevice, mxLineEndList);
}

template <class ItemType>
std::optional<ItemType>
LinePropertyPanelBase::PickLineEnd(weld::ComboBox& rListBox,
                                   std::optional<basegfx::B2DPolyPolygon>& roPolygon)
{
    const OUString sId = rListBox.get_active_id();

    if (sId == MORE_STYLES_ID)
    {
        SelectLineEndEntry(rListBox, roPolygon);
        comphelper::dispatchCommand(u".uno:FormatLine"_ustr, mxFrame, {});
        return {};
    }

    basegfx::B2DPolyPolygon aPolygon;
    std::optional<ItemType> oItem;

    if (sId == NONE_ID)
        oItem.emplace();
    else if (const XLineEndEntry* pLineEnd = GetSelectedLineEnd(rListBox))
    {
        aPolygon = pLineEnd->GetLineEnd();
        oItem.emplace(pLineEnd->GetName(), aPolygon);
    }

    if (oItem)
    {
        roPolygon = aPolygon;
        rListBox.save_value();
    }

    return oItem;
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeStartHdl, weld::ComboBox&, void)
{
    const std::optional<XLineStartItem> oItem
        = PickLineEnd<XLineStartItem>(*mxLBStart, moStartPolygon);
    if (oItem)
        setLineStart(*oItem);
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeEndHdl, weld::ComboBox&, void)
{
    const std::optional<XLineEndItem> oItem = PickLineEnd<XLineEndItem>(*mxLBEnd, moEndPolygon);
    if (oItem)
        setLineEnd(*oItem);
}

void LinePropertyPanelBase::setMapUnit(MapUnit eMapUnit)
{
    meMapUnit = eMapUnit;
    mxLineWidthPopup->SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
}

void LinePropertyPanelBase::disableArrowHead()
{
    mbArrowSupported = false;
    ActivateControls();
}

void LinePropertyPanelBase::enableArrowHead()
{
    mbArrowSupported = true;
    ActivateControls();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
