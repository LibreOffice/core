/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <condition_variable>
#include <mutex>
#include "uiobject_uno.hxx"
#include <utility>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <vcl/task.hxx>
#include <vcl/window.hxx>

#include <set>

UIObjectUnoObj::UIObjectUnoObj(std::unique_ptr<UIObject> pObj):
    UIObjectBase(m_aMutex),
    mpObj(std::move(pObj))
{
}

UIObjectUnoObj::~UIObjectUnoObj()
{
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

namespace
{
class ExecuteActionTask final : public Task
{
    std::condition_variable m_CV;
    std::mutex m_Mutex;
    bool m_bReady = false;

    OUString m_sAction;
    UIObject& m_rUIobject;
    StringMap m_PropertyMap;

    virtual sal_uInt64 UpdateMinPeriod(sal_uInt64) const override;
    virtual void Invoke() override;

public:
    ExecuteActionTask(const OUString& rAction,
                      const css::uno::Sequence<css::beans::PropertyValue>& rPropValues,
                      UIObject& rUIobject);
    void wait_for_invoke();
};

ExecuteActionTask::ExecuteActionTask(
    const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rPropValues,
    UIObject& rUIobject)
    : Task("UIObjectUnoObj::executeAction")
    , m_sAction(rAction)
    , m_rUIobject(rUIobject)
{
    for (const auto& rPropVal : rPropValues)
    {
        OUString aVal;
        if (!(rPropVal.Value >>= aVal))
            continue;
        m_PropertyMap[rPropVal.Name] = aVal;
    }
}

sal_uInt64 ExecuteActionTask::UpdateMinPeriod(sal_uInt64) const
{
    return Scheduler::ImmediateTimeoutMs;
}

void ExecuteActionTask::Invoke()
{
    m_rUIobject.execute(m_sAction, m_PropertyMap);
    {
        std::lock_guard<std::mutex> lk(m_Mutex);
        m_bReady = true;
    }
    m_CV.notify_one();
}

void ExecuteActionTask::wait_for_invoke()
{
    assert(!m_bReady);
    std::unique_lock<std::mutex> lk(m_Mutex);
    Start();
    m_CV.wait(lk, [this] { return m_bReady; });
}

} // namespace

void SAL_CALL UIObjectUnoObj::executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rPropValues)
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    ExecuteActionTask aTask(rAction, rPropValues, *mpObj);
    aTask.wait_for_invoke();

    SolarMutexGuard aGuard;
    Scheduler::ProcessEventsToIdle();
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

    std::set<OUString> aChildren;

    {
        SolarMutexGuard aGuard;
        aChildren = mpObj->get_children();
    }

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
    if (!mpObj)
        throw css::uno::RuntimeException();

    SolarMutexGuard aGuard;
    return mpObj->dumpHierarchy();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
