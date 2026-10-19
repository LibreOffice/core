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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <rtl/ustring.hxx>

namespace oox::ppt
{
/** The identifier of a page, which every page of a document holds and which stays the same when
    the page is renamed. Empty where the page holds none. */
inline OUString getPageIdentifier(const cpo::uno::Reference<css::drawing::XDrawPage>& rxPage)
{
    OUString aIdentifier;
    cpo::uno::Reference<css::beans::XPropertySet> xPageProperties(rxPage, cpo::uno::UNO_QUERY);
    if (!xPageProperties.is())
        return aIdentifier;
    cpo::uno::Reference<css::beans::XPropertySetInfo> xInfo(xPageProperties->getPropertySetInfo());
    if (xInfo.is() && xInfo->hasPropertyByName(u"Guid"_ustr))
        xPageProperties->getPropertyValue(u"Guid"_ustr) >>= aIdentifier;
    return aIdentifier;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
