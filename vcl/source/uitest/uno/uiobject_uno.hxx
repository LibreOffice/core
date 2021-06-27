/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/test/XUIObject.hpp>

#include <condition_variable>
#include <memory>
#include <mutex>

#include <vcl/uitest/uiobject.hxx>
#include <vcl/task.hxx>

typedef ::cppu::WeakComponentImplHelper <
    css::ui::test::XUIObject, css::lang::XServiceInfo
    > UIObjectBase;

class UIObjectUnoObj final : public cppu::BaseMutex,
    public UIObjectBase, public Task
{
public:
    explicit UIObjectUnoObj(std::unique_ptr<UIObject> pObj);
    virtual ~UIObjectUnoObj() override;

    css::uno::Reference<css::ui::test::XUIObject> SAL_CALL getChild(const OUString& rID) override;

    void SAL_CALL executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& xPropValues) override;

    css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getState() override;

    css::uno::Sequence<OUString> SAL_CALL getChildren() override;

    OUString SAL_CALL getType() override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    OUString SAL_CALL getHierarchy() override;

    virtual sal_uInt64 UpdateMinPeriod(sal_uInt64) const override;
    virtual void Invoke() override;

private:
    std::unique_ptr<UIObject> mpObj;

    std::condition_variable cv;
    std::mutex mMutex;
    bool mReady;

    OUString m_sAction;
    StringMap m_aPropertyMap;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
