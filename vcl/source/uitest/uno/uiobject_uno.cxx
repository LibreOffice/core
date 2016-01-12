/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uiobject_uno.hxx"
#include <vcl/svapp.hxx>

UIObjectUnoObj::UIObjectUnoObj(std::unique_ptr<UIObject> pObj):
    UIObjectBase(m_aMutex),
    mpObj(std::move(pObj))
{
}

UIObjectUnoObj::~UIObjectUnoObj()
{
}

css::uno::Reference<css::ui::test::XUIObject> SAL_CALL UIObjectUnoObj::getChild(const OUString& rID)
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::unique_ptr<UIObject> pObj = mpObj->get_child(rID);
    return new UIObjectUnoObj(std::move(pObj));
}

void SAL_CALL UIObjectUnoObj::executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& rPropValues)
    throw (css::uno::RuntimeException, std::exception)
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
}

css::uno::Sequence<css::beans::PropertyValue> UIObjectUnoObj::getState()
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    StringMap aMap = mpObj->get_state();
    css::uno::Sequence<css::beans::PropertyValue> aProps(aMap.size());
    sal_Int32 i = 0;
    for (auto itr = aMap.begin(), itrEnd = aMap.end(); itr != itrEnd; ++itr, ++i)
    {
        aProps[i].Name = itr->first;
        aProps[i].Value = css::uno::makeAny(itr->second);
    }

    return aProps;
}

OUString SAL_CALL UIObjectUnoObj::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("org.libreoffice.uitest.UIObject");
}

sal_Bool UIObjectUnoObj::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UIObjectUnoObj::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aServiceNames(1);
    aServiceNames[0] = "com.sun.star.ui.test.UIObject";
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
