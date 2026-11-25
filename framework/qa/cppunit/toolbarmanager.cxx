/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <vector>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>
#include <test/unoapi_test.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/vclevent.hxx>

namespace
{
class ToolBarManagerTest : public UnoApiTest
{
public:
    ToolBarManagerTest()
        : UnoApiTest(u"/framework/qa/cppunit/data/"_ustr)
    {
    }
};

// Calls the function object at pInstance; makes a Link of a capturing lambda
template <typename Function> void CallFunction(void* pInstance, VclSimpleEvent& rEvent)
{
    (*static_cast<Function*>(pInstance))(rEvent);
}

CPPUNIT_TEST_FIXTURE(ToolBarManagerTest, testOverflowPopupEndsDuringDispose)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        // No toolbar manager here (see LayoutManager::LayoutManager)
        return;
    }

    loadFromURL(u"private:factory/swriter"_ustr);
    Scheduler::ProcessEventsToIdle(); // finish loading before the close
    auto xFrame = mxComponent.queryThrow<css::frame::XModel>()
                      ->getCurrentController()
                      ->getFrame()
                      .queryThrow<css::beans::XPropertySet>();
    auto xLayoutManager
        = xFrame->getPropertyValue(u"LayoutManager"_ustr).queryThrow<css::frame::XLayoutManager>();
    auto xElement = xLayoutManager->getElement(u"private:resource/toolbar/standardbar"_ustr);
    CPPUNIT_ASSERT(xElement);

    SolarMutexGuard aGuard;
    VclPtr<vcl::Window> pToolBox
        = VCLUnoHelper::GetWindow(xElement->getRealInterface().query<css::awt::XWindow>());
    CPPUNIT_ASSERT(pToolBox);

    // Narrow the toolbar, and click its menu button to open the overflow toolbar
    pToolBox->SetSizePixel(Size(100, pToolBox->GetSizePixel().Height()));
    Size aSize = pToolBox->GetOutputSizePixel();
    pToolBox->MouseButtonDown(MouseEvent(Point(aSize.Width() - 3, aSize.Height() / 2), 1,
                                         MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT));

    // The overflow toolbar's popup mode ends while its manager is being disposed
    int nEndRequests = 0, nEndEvents = 0;
    auto aEndPopup = [&](VclSimpleEvent& rEvent) {
        if (rEvent.GetId() == VclEventId::WindowEndPopupMode)
        {
            ++nEndEvents;
            return;
        }
        if (rEvent.GetId() != VclEventId::ToolboxItemWindowChanged)
            return;
        vcl::Window* pWindow = static_cast<VclWindowEvent&>(rEvent).GetWindow();
        DockingManager* pDockingManager = vcl::Window::GetDockingManager();
        if (!pDockingManager->GetFloatingWindow(pWindow)) // only the overflow toolbar
            return;
        pDockingManager->EndPopupMode(pWindow);
        ++nEndRequests;
        Scheduler::ProcessEventsToIdle(); // deliver the posted popup end now
    };
    auto aListener = LINK_NONMEMBER(&aEndPopup, CallFunction<decltype(aEndPopup)>);
    Application::AddEventListener(aListener);
    comphelper::ScopeGuard aRemoveListener([&] { Application::RemoveEventListener(aListener); });
    mxComponent.queryThrow<css::util::XCloseable>()->close(true);
    mxComponent.clear();

    // Without the fix, the second dispose of the overflow manager crashed
    CPPUNIT_ASSERT_EQUAL(1, nEndRequests);
    CPPUNIT_ASSERT_EQUAL(1, nEndEvents);
}

CPPUNIT_TEST_FIXTURE(ToolBarManagerTest, testRefillDuringDispose)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        // No toolbar manager here (see LayoutManager::LayoutManager)
        return;
    }

    loadFromURL(u"private:factory/swriter"_ustr);
    Scheduler::ProcessEventsToIdle(); // finish loading before the close
    auto xFrame = mxComponent.queryThrow<css::frame::XModel>()
                      ->getCurrentController()
                      ->getFrame()
                      .queryThrow<css::beans::XPropertySet>();
    auto xLayoutManager
        = xFrame->getPropertyValue(u"LayoutManager"_ustr).queryThrow<css::frame::XLayoutManager>();
    auto xElement = xLayoutManager->getElement(u"private:resource/toolbar/textobjectbar"_ustr);
    CPPUNIT_ASSERT(xElement);

    SolarMutexGuard aGuard;
    VclPtr<ToolBox> pToolBox = dynamic_cast<ToolBox*>(
        VCLUnoHelper::GetWindow(xElement->getRealInterface().query<css::awt::XWindow>()));
    CPPUNIT_ASSERT(pToolBox);
    auto xSettings = xElement.queryThrow<css::ui::XUIElementSettings>();

    auto GetItemWindows = [&] {
        std::vector<vcl::Window*> aWindows;
        for (ToolBox::ImplToolItems::size_type i = 0; i < pToolBox->GetItemCount(); ++i)
        {
            if (vcl::Window* pWindow = pToolBox->GetItemWindow(pToolBox->GetItemId(i)))
                aWindows.push_back(pWindow);
        }
        return aWindows;
    };

    // The toolbar is refilled while its manager is being disposed
    int nRefills = 0, nNewWindows = 0;
    auto aRefill = [&](VclSimpleEvent& rEvent) {
        if (rEvent.GetId() != VclEventId::ToolboxItemWindowChanged
            || static_cast<VclWindowEvent&>(rEvent).GetWindow() != pToolBox || nRefills != 0)
            return;
        ++nRefills;
        std::vector<vcl::Window*> aOldWindows = GetItemWindows();
        xSettings->updateSettings();
        nNewWindows = std::ranges::count_if(GetItemWindows(), [&](vcl::Window* pWindow) {
            return std::ranges::find(aOldWindows, pWindow) == aOldWindows.end();
        });
    };
    auto aListener = LINK_NONMEMBER(&aRefill, CallFunction<decltype(aRefill)>);
    Application::AddEventListener(aListener);
    comphelper::ScopeGuard aRemoveListener([&] { Application::RemoveEventListener(aListener); });
    mxComponent.queryThrow<css::util::XCloseable>()->close(true);
    mxComponent.clear();

    CPPUNIT_ASSERT_EQUAL(1, nRefills);
    // Without the fix, the refill created new controllers and their item windows
    CPPUNIT_ASSERT_EQUAL(0, nNewWindows);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
