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

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <svx/ChartThemeType.hxx>
#include "ChartThemePanel.hxx"
#include "ChartThemeControl.hxx"

#include <ChartController.hxx>
#include <ChartModel.hxx>

#include <sfx2/weldutils.hxx>
#include <svtools/toolbarmenu.hxx>

#include <algorithm>

#include <comphelper/processfactory.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#include <unotools/fcm.hxx>
#include <ChartModelHelper.hxx>

using namespace css;

namespace chart::sidebar
{
namespace
{
ChartThemeControl* getChartThemeControl(const ToolbarUnoDispatcher& rToolBoxColor)
{
    const uno::Reference<frame::XToolbarController> xController
        = rToolBoxColor.GetControllerForCommand(sUnoChartTheme);
    const auto pToolBoxLineStyleControl = dynamic_cast<ChartThemeControl*>(xController.get());
    return pToolBoxLineStyleControl;
}
} // end unnamed namespace

class ThemeWrapper final : public IThemeHandler
{
public:
    ThemeWrapper(rtl::Reference<ChartModel> mxModel, ChartThemeControl* pControl,
                 ChartController* pController);

    void updateModel(const rtl::Reference<ChartModel>& xModel);
    void updateData() const;

    void select(sal_uInt32 nIndex) override;
    [[nodiscard]] VclPtr<VirtualDevice> makePictureFromThemedChart(sal_uInt32 nIndex) override;

private:
    rtl::Reference<ChartModel> mxModel;
    ChartController* mpController = nullptr;
    ChartThemeControl* mpControl = nullptr;
};

ThemeWrapper::ThemeWrapper(rtl::Reference<ChartModel> xModel, ChartThemeControl* pControl,
                           ChartController* pController)
    : mxModel(std::move(xModel))
    , mpController(pController)
    , mpControl(pControl)
{
}

void ThemeWrapper::updateModel(const rtl::Reference<ChartModel>& xModel) { mxModel = xModel; }

void ThemeWrapper::updateData() const
{
    util::URL aUrl;
    aUrl.Complete = sUnoChartTheme;

    frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aUrl;
    aEvent.IsEnabled = true;

    if (mpControl)
        mpControl->statusChanged(aEvent);
}

void ThemeWrapper::select(const sal_uInt32 nIndex) { mpController->setTheme(nIndex); }

VclPtr<VirtualDevice> ThemeWrapper::makePictureFromThemedChart(sal_uInt32 nIndex)
{
    if (mpController)
    {
        // Make a copy of the chart to take screenshot with different style
        rtl::Reference<ChartModel> mxModel2 = new ChartModel(*mxModel);
        auto xComponent = comphelper::getProcessComponentContext();
        rtl::Reference<ChartController> pController2 = new ChartController(xComponent);
        utl::ConnectFrameControllerModel(nullptr, pController2, mxModel2);
        // Change the cloned chart style
        pController2->setTheme(nIndex);

        //make a picture from it
        VclPtr<VirtualDevice> device1 = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);

        awt::Size aSize = mxModel->getVisualAreaSize(1);

        // TODO: find a better way to render thumbnail from a chart
        //use scaleText property to scale
        if (ReferenceSizeProvider::getAutoResizeState(mxModel2)
            != ReferenceSizeProvider::AUTO_RESIZE_YES)
        {
            awt::Size aPageSize(ChartModelHelper::getPageSize(mxModel2));
            ReferenceSizeProvider aRSP(aPageSize, mxModel2);
            aRSP.toggleAutoResizeState();
        }

        int scale = 4;
        mxModel2->setVisualAreaSize(1, awt::Size(aSize.Width / scale, aSize.Height / scale));
        device1->SetOutputSize(Size(aSize.Width / scale, aSize.Height / scale));

        pController2->PrePaint();
        drawinglayer::primitive2d::bChartThumbnailRendered = true;
        pController2->execute_Paint(
            *device1, tools::Rectangle(0, 0, aSize.Width / scale, aSize.Height / scale));
        drawinglayer::primitive2d::bChartThumbnailRendered = false;

        return device1;
    }
    return nullptr;
}

ChartThemePanel::ChartThemePanel(weld::Widget* pParent,
                                 const uno::Reference<frame::XFrame>& rxFrame,
                                 ChartController* pController)
    : PanelLayout(pParent, "ChartThemePanel", "modules/schart/ui/sidebartheme.ui")
    , mxModel(pController->getChartModel())
    , mpController(pController)
    , mxModifyListener(new ChartSidebarModifyListener(this))
    , mbModelValid(true)
    , mxThemeTB(m_xBuilder->weld_toolbar("themetype"))
    , mxThemeDispatch(new ToolbarUnoDispatcher(*mxThemeTB, *m_xBuilder, rxFrame))
    , mxSaveToNewTheme(m_xBuilder->weld_button(u"savetonew"_ustr))
{
    Initialize();
}

ChartThemePanel::~ChartThemePanel()
{
    doUpdateModel(nullptr);
    mxThemeDispatch.reset();
    mxThemeTB.reset();
    mxSaveToNewTheme.reset();
}

void ChartThemePanel::Initialize()
{
    mxModel->addModifyListener(mxModifyListener);

    ChartThemeControl* pThemeControl = getChartThemeControl(*mxThemeDispatch);
    assert(pThemeControl);
    mxThemeWrapper = std::make_shared<ThemeWrapper>(mxModel, pThemeControl, mpController);
    pThemeControl->setThemeHandler(mxThemeWrapper);

    mxSaveToNewTheme->connect_clicked(LINK(this, ChartThemePanel, ClickSaveToNewHdl));
    mxSaveToNewTheme->set_visible(true);

    updateData();
}

void ChartThemePanel::updateData()
{
    if (!mbModelValid)
        return;
    mxThemeWrapper->updateData();
}

std::unique_ptr<PanelLayout> ChartThemePanel::Create(weld::Widget* pParent,
                                                     const uno::Reference<frame::XFrame>& rxFrame,
                                                     ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartThemePanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartThemePanel::Create", nullptr,
                                             1);

    return std::make_unique<ChartThemePanel>(pParent, rxFrame, pController);
}

void ChartThemePanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
    updateData();
}

void ChartThemePanel::HandleContextChange(const vcl::EnumContext&) { updateData(); }

void ChartThemePanel::NotifyItemUpdate(sal_uInt16 /*nSID*/, SfxItemState /*eState*/,
                                       const SfxPoolItem* /*pState*/)
{
}

void ChartThemePanel::modelInvalid() { mbModelValid = false; }

void ChartThemePanel::doUpdateModel(const rtl::Reference<ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxModifyListener);
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxThemeWrapper->updateModel(mxModel);

    mxModel->addModifyListener(mxModifyListener);
}

void ChartThemePanel::updateModel(const uno::Reference<frame::XModel> xModel)
{
    const auto pModel = dynamic_cast<ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

IMPL_LINK_NOARG(ChartThemePanel, ClickSaveToNewHdl, weld::Button&, void)
{
    mpController->saveTheme(10000000);
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
