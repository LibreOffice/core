/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <comphelper/comphelperdllapi.h>
#include <cppuhelper/implbase.hxx>
#include <vector>

namespace comphelper
{
typedef std::vector<css::uno::Sequence<css::beans::PropertyValue>> IndexedPropertyValues;

class COMPHELPER_DLLPUBLIC IndexedPropertyValuesContainer final
    : public cppu::WeakImplHelper<css::container::XIndexContainer, css::lang::XServiceInfo>
{
public:
    IndexedPropertyValuesContainer() noexcept;

    // XIndexContainer
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, const css::uno::Any& aElement) override;
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex) override;

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex(sal_Int32 nIndex, const css::uno::Any& aElement) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    IndexedPropertyValues maProperties;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
