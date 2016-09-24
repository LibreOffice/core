/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_INC_DMAPPER_GRAPHICZORDERHELPER_HXX
#define INCLUDED_WRITERFILTER_INC_DMAPPER_GRAPHICZORDERHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <map>

namespace writerfilter
{
namespace dmapper
{
class GraphicZOrderHelper
{
public:
    void addItem(css::uno::Reference<css::beans::XPropertySet> const& props, sal_Int32 relativeHeight);
    sal_Int32 findZOrder(sal_Int32 relativeHeight, bool bOldStyle = false);
private:
    typedef std::map< sal_Int32, css::uno::Reference<css::beans::XPropertySet> > Items;
    Items items;
};

} // namespace dmapper
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_INC_DMAPPER_GRAPHICZORDERHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
