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

void SAL_CALL UIObjectUnoObj::executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rPropValues)
{
    if (!mpObj)
        throw css::uno::RuntimeException();

    {
        SolarMutexGuard aGuard;
        StringMap aMap;
        for (sal_Int32 i = 0, n = rPropValues.getLength(); i < n; ++i)
        {
            OUString aVal;
            if (!(rPropValues[i].Value >>= aVal))
                continue;

            aMap[rPropValues[i].Name] = aVal;
        }
        mpObj->execute(rAction, aMap);
    };
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
