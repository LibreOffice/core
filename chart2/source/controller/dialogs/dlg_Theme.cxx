/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <dlg_Theme.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/fcm.hxx>
#include <vcl/virdev.hxx>
#include <ChartModelHelper.hxx>
#include <svx/ChartThemeType.hxx>

using namespace css;

namespace chart
{
SchThemeDlg::SchThemeDlg(weld::Window* pWindow, ChartController* pController)
    : GenericDialogController(pWindow, u"modules/schart/ui/dlg_Theme.ui"_ustr,
                              u"ChartThemeDialog"_ustr)
    , mxModel(pController->getChartModel())
    , mpController(pController)
    , mxThemeIconView(m_xBuilder->weld_icon_view(u"themeview"_ustr))
    , mxSaveTheme(m_xBuilder->weld_button(u"save"_ustr))
    , mxLoadTheme(m_xBuilder->weld_button(u"load"_ustr))
    , mxDeleteTheme(m_xBuilder->weld_button(u"delete"_ustr))
    , mxSaveToNewTheme(m_xBuilder->weld_button(u"savetonew"_ustr))
{
    mxThemeIconView->connect_selection_changed(LINK(this, SchThemeDlg, ThemeSelectedHdl));

    mxSaveTheme->connect_clicked(LINK(this, SchThemeDlg, ClickSaveHdl));
    mxLoadTheme->connect_clicked(LINK(this, SchThemeDlg, ClickLoadHdl));
    mxDeleteTheme->connect_clicked(LINK(this, SchThemeDlg, ClickDeleteHdl));
    mxSaveToNewTheme->connect_clicked(LINK(this, SchThemeDlg, ClickSaveToNewHdl));

    mxSaveTheme->set_sensitive(false);
    mxDeleteTheme->set_sensitive(false);

    mxSaveTheme->set_visible(true);
    mxLoadTheme->set_visible(true);
    mxDeleteTheme->set_visible(true);
    mxSaveToNewTheme->set_visible(true);

    mxThemeIconView->set_item_width(200);

    int nThemeCount = ChartThemesType::getInstance().getThemesCount();
    for (int i = 0; i < nThemeCount; i++)
    {
        ScopedVclPtr<VirtualDevice> device1 = makeImage(i);

        OUString sId = OUString::number(i);
        OUString sLayoutName = OUString::number(i);
        mxThemeIconView->insert(i, &sLayoutName, &sId, device1, nullptr);

        device1.disposeAndClear();
    }
    mxThemeIconView->select(0);
}

VclPtr<VirtualDevice> SchThemeDlg::makeImage(int nIndex)
{
    // clone the chart
    rtl::Reference<ChartModel> mxModel2 = new ChartModel(*mxModel);
    auto xComponent = comphelper::getProcessComponentContext();
    rtl::Reference<ChartController> pController2 = new ChartController(xComponent);
    utl::ConnectFrameControllerModel(nullptr, pController2, mxModel2);

    // Change the cloned chart theme
    pController2->setTheme(nIndex);

    //make a picture from it
    awt::Size aSize = mxModel->getVisualAreaSize(1); //embed::Aspects::MSOLE_CONTENT
    if (ReferenceSizeProvider::getAutoResizeState(mxModel2)
        != ReferenceSizeProvider::AUTO_RESIZE_YES)
    {
        awt::Size aPageSize(ChartModelHelper::getPageSize(mxModel2));
        ReferenceSizeProvider aRSP(aPageSize, mxModel2);
        aRSP.toggleAutoResizeState();
    }

    ScopedVclPtr<VirtualDevice> device2
        = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);

    // Todo: find a faster way to render scaled down charts
    int scale = 1;
    mxModel2->setVisualAreaSize(1, awt::Size(aSize.Width / scale, aSize.Height / scale));
    //device1->SetOutputSize(Size(aSize.Width, aSize.Height));
    device2->SetOutputSize(Size(aSize.Width / scale, aSize.Height / scale));

    GDIMetaFile aMTF;
    aMTF.Record(device2);

    pController2->PrePaint();
    //pController2->execute_Paint(*device1, tools::Rectangle(0, 0, aSize.Width, aSize.Height));
    bChartThumbnailRendered = true;
    pController2->execute_Paint(*device2,
                                tools::Rectangle(0, 0, aSize.Width / scale, aSize.Height / scale));
    bChartThumbnailRendered = false;
    aMTF.Stop();
    aMTF.WindStart();
    aMTF.Scale(1 / 64.0, 1 / 64.0);
    aMTF.WindStart();
    aMTF.Play(*device2);

    //scale it into a fixed small image
    ScopedVclPtr<VirtualDevice> device1
        = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device1->SetOutputSizePixel(Size(ChartThemeThumbSizeX, ChartThemeThumbSizeY));
    device1->SetBackground(Wallpaper(COL_YELLOW));
    device1->Erase();

    BitmapEx aBmpEx = device2->GetBitmapEx(Point(), device2->GetOutputSizePixel());
    aBmpEx.Scale(Size(ChartThemeThumbSizeX, ChartThemeThumbSizeY), BmpScaleFlag::Fast);

    device1->DrawBitmapEx(Point(0, 0), aBmpEx);
    device2.disposeAndClear();

    return std::move(device1);
}

IMPL_LINK_NOARG(SchThemeDlg, ClickSaveHdl, weld::Button&, void)
{
    OUString sId = mxThemeIconView->get_selected_id();
    if (sId.isEmpty())
        return;
    int nIndex = sId.toInt32();
    mpController->saveTheme(nIndex);

    //update the image of the list
    // TODO: we could gain a bit of performance by not cloneing the chart
    // because the new theme was creayed from the selected chart.
    ScopedVclPtr<VirtualDevice> device1 = makeImage(nIndex);
    mxThemeIconView->set_image(nIndex, device1);
    device1.disposeAndClear();
}

IMPL_LINK_NOARG(SchThemeDlg, ClickLoadHdl, weld::Button&, void)
{
    OUString sId = mxThemeIconView->get_selected_id();
    if (sId.isEmpty())
        return;
    int nIndex = sId.toInt32();
    mpController->setTheme(nIndex);
}

IMPL_LINK_NOARG(SchThemeDlg, ClickDeleteHdl, weld::Button&, void)
{
    OUString sId = mxThemeIconView->get_selected_id();
    if (sId.isEmpty())
        return;
    int nIndex = sId.toInt32();
    // remove it from the themes
    ChartThemesType& aChartTypes = ChartThemesType::getInstance();
    aChartTypes.m_aThemes.erase(aChartTypes.m_aThemes.begin() + nIndex);
    // remove it from the list
    mxThemeIconView->remove(nIndex);
    // fix the numbers in the list.
    int nThemeCount = ChartThemesType::getInstance().getThemesCount();
    for (int i = nIndex; i < nThemeCount; i++)
    {
        sId = OUString::number(i);
        OUString sLayoutName = OUString::number(i);
        mxThemeIconView->set_id(i, sId);
        mxThemeIconView->set_text(i, sLayoutName);
    }
}

IMPL_LINK_NOARG(SchThemeDlg, ClickSaveToNewHdl, weld::Button&, void)
{
    ChartThemesType& aChartTypes = ChartThemesType::getInstance();
    int nIndex = aChartTypes.m_aThemes.size();
    // maximum 256 chart styles
    // Todo: maybe we could figure out some other way to limit this.
    if (nIndex >= 256)
    {
        return;
    }
    mpController->saveTheme(nIndex);

    //insert image to the list
    ScopedVclPtr<VirtualDevice> device1 = makeImage(nIndex);
    OUString sId = OUString::number(nIndex);
    OUString sLayoutName = OUString::number(nIndex);
    mxThemeIconView->insert(nIndex, &sLayoutName, &sId, device1, nullptr);
    device1.disposeAndClear();
}

IMPL_LINK_NOARG(SchThemeDlg, ThemeSelectedHdl, weld::IconView&, void)
{
    // enable / disable buttons based on the selected theme
    bool bSelCustomizable = false;
    bool bSelected = false;
    OUString sId = mxThemeIconView->get_selected_id();
    if (!sId.isEmpty())
    {
        int nIndex = sId.toInt32();
        int nPreDefCount = ChartThemesType::getThemesPreDefCount();
        // only non-PreDefined themes can be deleted / overwrited
        if (nIndex >= nPreDefCount)
            bSelCustomizable = true;
        bSelected = true;
    }
    mxSaveTheme->set_sensitive(bSelCustomizable);
    mxDeleteTheme->set_sensitive(bSelCustomizable);
    mxLoadTheme->set_sensitive(bSelected);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
