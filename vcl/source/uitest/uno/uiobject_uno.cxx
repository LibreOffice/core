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
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include "uiobject_uno.hxx"
#include <utility>
#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/link.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <vcl/window.hxx>

#include <set>

class Timer;

namespace {

struct Notifier {
    std::condition_variable cv;
    std::mutex mMutex;
    bool mReady = false;

    DECL_LINK( NotifyHdl, Timer*, void );
};

}

UIObjectUnoObj::UIObjectUnoObj(std::unique_ptr<UIObject> pObj):
    mpObj(std::move(pObj))
{
    assert(mpObj);
}

UIObjectUnoObj::~UIObjectUnoObj()
{
    SolarMutexGuard aGuard;
    mpObj.reset();
}

css::uno::Reference<css::ui::test::XUIObject> SAL_CALL UIObjectUnoObj::getChild(const OUString& rID)
{
    SolarMutexGuard aGuard;
    std::unique_ptr<UIObject> pObj = mpObj->get_child(rID);
    SAL_WARN_IF(!pObj, "vcl", "child " << rID << " of parent " << mpObj->dumpState() << " does not exist");
    return new UIObjectUnoObj(std::move(pObj));
}

IMPL_LINK_NOARG(Notifier, NotifyHdl, Timer*, void)
{
    std::scoped_lock<std::mutex> lk(mMutex);
    mReady = true;
    cv.notify_all();
}

namespace {

class ExecuteWrapper
{
    std::function<void()> mFunc;
    Link<Timer*, void> mHandler;
    std::atomic<bool> mbSignal;

public:

    ExecuteWrapper(std::function<void()> func, Link<Timer*, void> handler):
        mFunc(std::move(func)),
        mHandler(handler),
        mbSignal(false)
    {
    }

    void setSignal()
    {
        mbSignal = true;
    }

    DECL_LINK( ExecuteActionHdl, Timer*, void );
};


IMPL_LINK_NOARG(ExecuteWrapper, ExecuteActionHdl, Timer*, void)
{
    {
        Idle aIdle("UI Test Idle Handler2");
        {
            mFunc();
            aIdle.SetPriority(TaskPriority::LOWEST);
            aIdle.SetInvokeHandler(mHandler);
            aIdle.Start();
        }

        while (!mbSignal) {
            Application::Reschedule();
        }
    }
    delete this;
}

}

void SAL_CALL UIObjectUnoObj::executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rPropValues)
{
    auto aIdle = std::make_unique<Idle>("UI Test Idle Handler");
    aIdle->SetPriority(TaskPriority::HIGHEST);

    std::function<void()> func = [&rAction, &rPropValues, this](){

        SolarMutexGuard aGuard;
        StringMap aMap;
        for (const auto& rPropVal : rPropValues)
        {
            OUString aVal;
            if (!(rPropVal.Value >>= aVal))
                continue;

            aMap[rPropVal.Name] = aVal;
        }
        mpObj->execute(rAction, aMap);
    };

    Notifier notifier;
    ExecuteWrapper* pWrapper = new ExecuteWrapper(std::move(func), LINK(&notifier, Notifier, NotifyHdl));
    aIdle->SetInvokeHandler(LINK(pWrapper, ExecuteWrapper, ExecuteActionHdl));
    {
        SolarMutexGuard aGuard;
        aIdle->Start();
    }

    {
        std::unique_lock<std::mutex> lk(notifier.mMutex);
        notifier.cv.wait(lk, [&notifier]{return notifier.mReady;});
    }
    pWrapper->setSignal();

    SolarMutexGuard aGuard;
    aIdle.reset();
    Scheduler::ProcessEventsToIdle();
}

css::uno::Sequence<css::beans::PropertyValue> UIObjectUnoObj::getState()
{
    SolarMutexGuard aGuard;
    StringMap aMap = mpObj->get_state();
    css::uno::Sequence<css::beans::PropertyValue> aProps(aMap.size());
    std::transform(aMap.begin(), aMap.end(), aProps.getArray(),
                   [](auto const& elem)
                   { return comphelper::makePropertyValue(elem.first, elem.second); });

    return aProps;
}

css::uno::Sequence<OUString> UIObjectUnoObj::getChildren()
{
    std::set<OUString> aChildren;

    {
        SolarMutexGuard aGuard;
        aChildren = mpObj->get_children();
    }

    css::uno::Sequence<OUString> aRet(aChildren.size());
    std::copy(aChildren.begin(), aChildren.end(), aRet.getArray());

    return aRet;
}

OUString SAL_CALL UIObjectUnoObj::getType()
{
    SolarMutexGuard aGuard;
    return mpObj->get_type();
}

OUString SAL_CALL UIObjectUnoObj::getImplementationName()
{
    return "org.libreoffice.uitest.UIObject";
}

sal_Bool UIObjectUnoObj::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UIObjectUnoObj::getSupportedServiceNames()
{
    return { "com.sun.star.ui.test.UIObject" };
}

OUString SAL_CALL UIObjectUnoObj::getHierarchy()
{
    SolarMutexGuard aGuard;
    return mpObj->dumpHierarchy();
}

sal_Bool SAL_CALL UIObjectUnoObj::equals(const css::uno::Reference<css::ui::test::XUIObject>& rOther)
{
    return mpObj->equals(*static_cast<UIObjectUnoObj&>(*rOther).mpObj);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
