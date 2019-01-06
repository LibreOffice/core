/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_CORE_APITESTBASE_HXX
#define INCLUDED_SW_QA_CORE_APITESTBASE_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <unordered_map>

namespace apitest
{
class ApiTestBase
{
protected:
    static bool extstsProperty(css::uno::Reference<css::beans::XPropertySet> const& rxPropertySet,
                               OUString const& rPropertyName)
    {
        css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo(
            rxPropertySet->getPropertySetInfo());
        return xPropertySetInfo->hasPropertyByName(rPropertyName);
    }

    static bool
    isPropertyReadOnly(css::uno::Reference<css::beans::XPropertySet> const& rxPropertySet,
                       OUString const& rPropertyName)
    {
        css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo(
            rxPropertySet->getPropertySetInfo());
        css::uno::Sequence<css::beans::Property> xProperties = xPropertySetInfo->getProperties();

        for (auto const& rProperty : xProperties)
        {
            if (rProperty.Name == rPropertyName)
                return (rProperty.Attributes & com::sun::star::beans::PropertyAttribute::READONLY)
                       != 0;
        }

        return false;
    }

    virtual ~ApiTestBase() {}

    virtual std::unordered_map<OUString, css::uno::Reference<css::uno::XInterface>> init() = 0;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
