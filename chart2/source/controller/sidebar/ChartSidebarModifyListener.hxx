/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSIDEBAR_MODIFYLISTENER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSIDEBAR_MODIFYLISTENER_HXX

#include <com/sun/star/util/XModifyListener.hpp>
#include <cppuhelper/implbase.hxx>

namespace chart {
namespace sidebar {

class ChartSidebarModifyListenerParent
{
public:
    virtual ~ChartSidebarModifyListenerParent();

    virtual void updateData() = 0;

    virtual void modelInvalid() = 0;
};

class ChartSidebarModifyListener : public cppu::WeakImplHelper<css::util::XModifyListener>
{
public:

    explicit ChartSidebarModifyListener(ChartSidebarModifyListenerParent* pParent);
    virtual ~ChartSidebarModifyListener();

    virtual void SAL_CALL modified(const css::lang::EventObject& rEvent)
        throw (::css::uno::RuntimeException, ::std::exception) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent)
        throw (::css::uno::RuntimeException, ::std::exception) override;

private:
    ChartSidebarModifyListenerParent* mpParent;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
