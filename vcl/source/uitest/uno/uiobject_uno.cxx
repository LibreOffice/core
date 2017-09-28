/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include "uiobject_uno.hxx"
#include <utility>
#include <o3tl/make_unique.hxx>
#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <vcl/scheduler.hxx>

#include <set>
#include <chrono>
#include <thread>

UIObjectUnoObj::UIObjectUnoObj(std::unique_ptr<UIObject> pObj):
    UIObjectBase(m_aMutex),
    mpObj(std::move(pObj)),
    mReady(true)
{
}

UIObjectUnoObj::~UIObjectUnoObj()
{
    {
        std::lock_guard<std::mutex> lk3(mMutex);
    }
    SolarMutexGuard aGuard;
    mpObj.reset();
}

css::uno::Reference<css::ui::test::XUIObject> SAL_CALL UIObjectUnoObj::getChild(const OUString& rID)
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    SolarMutexGuard aGuard;
    std::unique_ptr<UIObject> pObj = mpObj->get_child(rID);
    return new UIObjectUnoObj(std::move(pObj));
}

IMPL_LINK_NOARG(UIObjectUnoObj, NotifyHdl, Timer*, void)
{
    std::lock_guard<std::mutex> lk(mMutex);
    mReady = true;
    cv.notify_all();
}

namespace {

class ExecuteWrapper
{
    std::function<void()> mFunc;
    Link<Timer*, void> mHandler;
    bool mbSignal;
    std::mutex mMutex;

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

    std::mutex& getMutex()
    {
        return mMutex;
    }

    DECL_LINK( ExecuteActionHdl, Timer*, void );
};


IMPL_LINK_NOARG(ExecuteWrapper, ExecuteActionHdl, Timer*, void)
{
    {
        Idle aIdle;
        {
            mFunc();
            aIdle.SetDebugName("UI Test Idle Handler2");
            aIdle.SetPriority(TaskPriority::LOWEST);
            aIdle.SetInvokeHandler(mHandler);
            aIdle.Start();
        }

        for (;;) {
            {
                std::unique_lock<std::mutex> lock(mMutex);
                if (mbSignal) {
                    break;
                }
            }
            Application::Reschedule();
        }
        std::unique_lock<std::mutex> lock(mMutex);
        while (!mbSignal)
        {
            // coverity[sleep] - intentional sleep while mutex held
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    delete this;
}

}

void SAL_CALL UIObjectUnoObj::executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rPropValues)
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    std::unique_lock<std::mutex> lk(mMutex);
    mAction = rAction;
    mPropValues = rPropValues;
    mReady = false;
    auto aIdle = o3tl::make_unique<Idle>();
    aIdle->SetDebugName("UI Test Idle Handler");
    aIdle->SetPriority(TaskPriority::HIGHEST);

    std::function<void()> func = [this](){

        SolarMutexGuard aGuard;
        StringMap aMap;
        for (sal_Int32 i = 0, n = mPropValues.getLength(); i < n; ++i)
        {
            OUString aVal;
            if (!(mPropValues[i].Value >>= aVal))
                continue;

            aMap[mPropValues[i].Name] = aVal;
        }
        mpObj->execute(mAction, aMap);
    };

    ExecuteWrapper* pWrapper = new ExecuteWrapper(func, LINK(this, UIObjectUnoObj, NotifyHdl));
    std::unique_lock<std::mutex>(pWrapper->getMutex());
    aIdle->SetInvokeHandler(LINK(pWrapper, ExecuteWrapper, ExecuteActionHdl));
    {
        SolarMutexGuard aGuard;
        aIdle->Start();
    }

    cv.wait(lk, [this]{return mReady;});
    pWrapper->setSignal();

    SolarMutexGuard aGuard;
    aIdle.reset();
}

css::uno::Sequence<css::beans::PropertyValue> UIObjectUnoObj::getState()
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    SolarMutexGuard aGuard;
    StringMap aMap = mpObj->get_state();
    css::uno::Sequence<css::beans::PropertyValue> aProps(aMap.size());
    sal_Int32 i = 0;
    for (auto itr = aMap.begin(), itrEnd = aMap.end(); itr != itrEnd; ++itr, ++i)
    {
        aProps[i].Name = itr->first;
        aProps[i].Value <<= itr->second;
    }

    return aProps;
}

css::uno::Sequence<OUString> UIObjectUnoObj::getChildren()
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    std::set<OUString> aChildren = mpObj->get_children();

    css::uno::Sequence<OUString> aRet(aChildren.size());
    sal_Int32 i = 0;
    for (auto itr = aChildren.begin(), itrEnd = aChildren.end(); itr != itrEnd; ++itr, ++i)
    {
        aRet[i] = *itr;
    }

    return aRet;
}

OUString SAL_CALL UIObjectUnoObj::getType()
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    return mpObj->get_type();
}

OUString SAL_CALL UIObjectUnoObj::getImplementationName()
{
    return OUString("org.libreoffice.uitest.UIObject");
}

sal_Bool UIObjectUnoObj::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UIObjectUnoObj::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aServiceNames(1);
    aServiceNames[0] = "com.sun.star.ui.test.UIObject";
    return aServiceNames;
}

OUString SAL_CALL UIObjectUnoObj::getHierarchy()
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    SolarMutexGuard aGuard;
    return mpObj->dumpHierarchy();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
