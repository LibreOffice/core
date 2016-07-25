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
    virtual ~UIObjectUnoObj();

    css::uno::Reference<css::ui::test::XUIObject> SAL_CALL getChild(const OUString& rID)
        throw (css::uno::RuntimeException, std::exception) override;

    void SAL_CALL executeAction(const OUString& rAction, const css::uno::Sequence<css::beans::PropertyValue>& xPropValues)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getState()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getChildren()
        throw (css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getType()
        throw (css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
