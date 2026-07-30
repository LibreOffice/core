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
#include <svx/xtable.hxx>
#include <comphelper/kit.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace css;
using namespace css::uno;

constexpr OUString SELECTWIDTH = u"SelectWidth"_ustr;

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

void LinePropertyPanelBase::FillLineEndListBox(weld::ComboBox& rListBox)
{
    rListBox.freeze();
    rListBox.clear();

    const OUString sNone(comphelper::COKit::isActive()
                             ? SvxResId(RID_SVXSTR_INVISIBLE)
                             : SvxResId(RID_SVXSTR_NONE));
    rListBox.append(OUString::number(0), sNone);

    if (mxLineEndList.is())
    {
        const tools::Long nCount = mxLineEndList->Count();
        for (tools::Long i = 0; i < nCount; ++i)
            rListBox.append(OUString::number(i + 1), mxLineEndList->GetLineEnd(i)->GetName());
    }

    rListBox.thaw();
}

void LinePropertyPanelBase::SelectLineEndEntry(
    weld::ComboBox& rListBox, const std::optional<basegfx::B2DPolyPolygon>& roPolygon)
{
    if (!roPolygon)
    {
        rListBox.set_active(-1);
        rListBox.save_value();
        return;
    }

    if (roPolygon->count() == 0)
    {
        rListBox.set_active(0);
        rListBox.save_value();
        return;
    }

    if (mxLineEndList.is())
    {
        const tools::Long nCount = mxLineEndList->Count();
        for (tools::Long i = 0; i < nCount; ++i)
        {
            if (mxLineEndList->GetLineEnd(i)->GetLineEnd() == *roPolygon)
            {
                rListBox.set_active(i + 1); // +1 for the "none" entry
                rListBox.save_value();
                return;
            }
        }
    }

    rListBox.set_active(-1);
    rListBox.save_value();
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
    vcl::RenderContext& rDevice = std::get<0>(rArgs);
    const tools::Rectangle& rRect = std::get<1>(rArgs);
    const OUString& rId = std::get<3>(rArgs);

    const sal_uInt32 nId = rId.toUInt32();
    const tools::Long nMidY = rRect.Top() + rRect.GetHeight() / 2;

    if (nId == 0)
    {
        // "no arrowhead": a short plain line stub
        rDevice.SetLineColor(Application::GetSettings().GetStyleSettings().GetFieldTextColor());
        rDevice.DrawLine(Point(rRect.Left() + 2, nMidY), Point(rRect.Left() + 16, nMidY));
        return;
    }

    if (!mxLineEndList.is() || mxLineEndList->Count() < static_cast<tools::Long>(nId))
        return;

    const Bitmap aBitmap = mxLineEndList->GetUiBitmap(nId - 1);
    if (aBitmap.IsEmpty())
        return;

    const Size aBmpSize(aBitmap.GetSizePixel());
    const tools::Long nHalfW = aBmpSize.Width() / 2;
    const Point aSrcPt(bStart ? 0 : nHalfW, 0);
    const Size aHalfSize(nHalfW, aBmpSize.Height());
    rDevice.DrawBitmap(Point(rRect.Left() + 2, nMidY - aBmpSize.Height() / 2), aHalfSize, aSrcPt,
                       aHalfSize, aBitmap);
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
    tools::Long nImgWidth = 16; // fallback for the "none" line stub
    tools::Long nHeight = rDevice.GetTextHeight();

    if (mxLineEndList.is())
    {
        const tools::Long nCount = mxLineEndList->Count();
        for (tools::Long i = 0; i < nCount; ++i)
        {
            const Bitmap aBmp = mxLineEndList->GetUiBitmap(i);
            if (!aBmp.IsEmpty())
            {
                nImgWidth = std::max<tools::Long>(nImgWidth, aBmp.GetSizePixel().Width() / 2);
                nHeight = std::max<tools::Long>(nHeight, aBmp.GetSizePixel().Height());
            }
        }
    }

    return Size(nImgWidth + 4, nHeight + 4);
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeStartHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos = mxLBStart->get_active();
    if (nPos == -1)
        return;

    std::unique_ptr<XLineStartItem> pItem;
    if (nPos == 0)
        pItem.reset(new XLineStartItem());
    else if (mxLineEndList.is() && mxLineEndList->Count() > static_cast<tools::Long>(nPos - 1))
        pItem.reset(new XLineStartItem(mxLBStart->get_active_text(),
                                       mxLineEndList->GetLineEnd(nPos - 1)->GetLineEnd()));

    if (pItem)
    {
        moStartPolygon = pItem->GetLineStartValue();
        mxLBStart->save_value();
        setLineStart(*pItem);
    }
}

IMPL_LINK_NOARG(LinePropertyPanelBase, ChangeEndHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos = mxLBEnd->get_active();
    if (nPos == -1)
        return;

    std::unique_ptr<XLineEndItem> pItem;
    if (nPos == 0)
        pItem.reset(new XLineEndItem());
    else if (mxLineEndList.is() && mxLineEndList->Count() > static_cast<tools::Long>(nPos - 1))
        pItem.reset(new XLineEndItem(mxLBEnd->get_active_text(),
                                     mxLineEndList->GetLineEnd(nPos - 1)->GetLineEnd()));

    if (pItem)
    {
        moEndPolygon = pItem->GetLineEndValue();
        mxLBEnd->save_value();
        setLineEnd(*pItem);
    }
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
