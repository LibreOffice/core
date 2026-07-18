/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <cpo/uno/Sequence.h>
#include <comphelper/comphelperdllapi.h>
#include <cppuhelper/implbase.hxx>
#include <vector>

namespace comphelper
{
typedef std::vector<cpo::uno::Sequence<css::beans::PropertyValue>> IndexedPropertyValues;

class COMPHELPER_DLLPUBLIC IndexedPropertyValuesContainer final
    : public cppu::WeakImplHelper<css::container::XIndexContainer, css::lang::XServiceInfo>
{
public:
    IndexedPropertyValuesContainer() noexcept;

    // XIndexContainer
    virtual void insertByIndex(sal_Int32 nIndex, const cpo::uno::Any& aElement) override;
    virtual void removeByIndex(sal_Int32 nIndex) override;

    // XIndexReplace
    virtual void replaceByIndex(sal_Int32 nIndex, const cpo::uno::Any& aElement) override;

    // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex(sal_Int32 nIndex) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    //XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

private:
    IndexedPropertyValues maProperties;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
