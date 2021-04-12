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
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <vcl/window.hxx>

#include <set>

UIObjectUnoObj::UIObjectUnoObj(std::unique_ptr<UIObject> pObj):
    UIObjectBase(m_aMutex),
    mpObj(std::move(pObj)),
    mReady(true)
{
}

UIObjectUnoObj::~UIObjectUnoObj()
{
    {
        std::scoped_lock<std::mutex> lk3(mMutex);
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
    std::scoped_lock<std::mutex> lk(mMutex);
    mReady = true;
    cv.notify_all();
}

namespace {

class ExecuteWrapper
{
    std::function<void()> mFunc;
    Link<Timer*, void> mHandler;
    volatile bool mbSignal;

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
        Idle aIdle;
        {
            mFunc();
            aIdle.SetDebugName("UI Test Idle Handler2");
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
    if (!mpObj)
        throw css::uno::RuntimeException();

    std::unique_lock<std::mutex> lk(mMutex);
    mAction = rAction;
    mPropValues = rPropValues;
    mReady = false;
    auto aIdle = std::make_unique<Idle>();
    aIdle->SetDebugName("UI Test Idle Handler");
    aIdle->SetPriority(TaskPriority::HIGHEST);

    std::function<void()> func = [this](){

        SolarMutexGuard aGuard;
        StringMap aMap;
        for (const auto& rPropVal : std::as_const(mPropValues))
        {
            OUString aVal;
            if (!(rPropVal.Value >>= aVal))
                continue;

            aMap[rPropVal.Name] = aVal;
        }
        mpObj->execute(mAction, aMap);
    };

    ExecuteWrapper* pWrapper = new ExecuteWrapper(func, LINK(this, UIObjectUnoObj, NotifyHdl));
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
    for (auto const& elem : aMap)
    {
        aProps[i].Name = elem.first;
        aProps[i].Value <<= elem.second;
        ++i;
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
    for (auto const& child : aChildren)
    {
        aRet[i] = child;
        ++i;
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
    if (!mpObj)
        throw css::uno::RuntimeException();

    SolarMutexGuard aGuard;
    return mpObj->dumpHierarchy();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
