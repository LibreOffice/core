/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SOURCE_UITEST_UNO_UIOBJECT_UNO_HXX
#define INCLUDED_VCL_SOURCE_UITEST_UNO_UIOBJECT_UNO_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/test/XUIObject.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <memory>

#include <vcl/uitest/uiobject.hxx>

typedef ::cppu::WeakComponentImplHelper <
    css::ui::test::XUIObject, css::lang::XServiceInfo
    > UIObjectBase;

class UIObjectUnoObj : public cppu::BaseMutex,
    public UIObjectBase
{
private:
    std::unique_ptr<UIObject> mpObj;

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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
