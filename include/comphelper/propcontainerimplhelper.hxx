/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainerhelper.hxx>
#include <comphelper/propimplhelper.hxx>

namespace comphelper
{
/**
   Combines OPropertyImplHelper, OPropertyContainerHelper, and
   OPropertyArrayUsageHelper into a single base class — the template
   replacement for OPropertyContainer2.

   Derived classes just need to implement createArrayHelper() (typically
   calling describeProperties + wrapping in OPropertyArrayHelper) and
   call registerProperty() in their constructor.

   @tparam BaseClass  Same as OPropertyImplHelper: must satisfy IsUnoImplBase.
   @tparam Derived    The final class (CRTP), needed by OPropertyArrayUsageHelper
                       for per-class static caching of the property array.
   @tparam Ifc        Optional additional UNO interfaces.
*/
template <IsUnoImplBase BaseClass, typename Derived, typename... Ifc>
class SAL_DLLPUBLIC_TEMPLATE OPropertyContainerImplHelper
    : public OPropertyImplHelper<BaseClass, Ifc...>,
      public OPropertyContainerHelper,
      public OPropertyArrayUsageHelper<Derived>
{
public:
    template <typename... Arg>
    explicit OPropertyContainerImplHelper(Arg&&... arg)
        : OPropertyImplHelper<BaseClass, Ifc...>(std::forward<Arg>(arg)...)
    {
    }

protected:
    cppu::IPropertyArrayHelper& getInfoHelper() override { return *this->getArrayHelper(); }

    bool convertFastPropertyValue(std::unique_lock<std::mutex>&, css::uno::Any& rConvertedValue,
                                  css::uno::Any& rOldValue, sal_Int32 nHandle,
                                  const css::uno::Any& rValue) override
    {
        return OPropertyContainerHelper::convertFastPropertyValue(rConvertedValue, rOldValue,
                                                                  nHandle, rValue);
    }

    void setFastPropertyValue_NoBroadcast(std::unique_lock<std::mutex>&, sal_Int32 nHandle,
                                          const css::uno::Any& rValue) override
    {
        OPropertyContainerHelper::setFastPropertyValue(nHandle, rValue);
    }

    using OPropertyImplHelper<BaseClass, Ifc...>::getFastPropertyValue;
    void getFastPropertyValue(std::unique_lock<std::mutex>&, css::uno::Any& rValue,
                              sal_Int32 nHandle) const override
    {
        OPropertyContainerHelper::getFastPropertyValue(rValue, nHandle);
    }
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
