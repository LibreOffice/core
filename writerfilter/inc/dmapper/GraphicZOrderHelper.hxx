/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/XPropertySet.hpp>
#include <map>

namespace writerfilter::dmapper
{
class GraphicZOrderHelper
{
public:
    void addItem(css::uno::Reference<css::beans::XPropertySet> const& props,
                 sal_Int64 relativeHeight);

    // must run adjustRelativeHeight before findZOrder - to set zOrder priorities
    static void adjustRelativeHeight(sal_Int64& rRelativeHeight, bool bIsZIndex, bool bIsBehindText,
                                     bool bIsInHeader);
    sal_Int32 findZOrder(sal_Int64 relativeHeight, bool bOldStyle = false);

private:
    using Items = std::map<sal_Int64, css::uno::Reference<css::beans::XPropertySet>>;
    Items m_items;
};

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
