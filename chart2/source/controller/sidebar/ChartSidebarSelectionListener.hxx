/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSIDEBARSELECTIONLISTENER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSIDEBARSELECTIONLISTENER_HXX

#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase.hxx>

#include "ObjectIdentifier.hxx"

#include <vector>

namespace chart {
namespace sidebar {

class ChartSidebarSelectionListenerParent
{
public:
    virtual ~ChartSidebarSelectionListenerParent();

    virtual void selectionChanged(bool bSelected) = 0;

    virtual void SelectionInvalid() = 0;
};

class ChartSidebarSelectionListener : public cppu::WeakImplHelper<css::view::XSelectionChangeListener>
{
public:

    // listen to all chart selection changes
    explicit ChartSidebarSelectionListener(ChartSidebarSelectionListenerParent* pParent);
    // only liste to the changes of eType
    ChartSidebarSelectionListener(ChartSidebarSelectionListenerParent* pParent, ObjectType eType);
    virtual ~ChartSidebarSelectionListener();

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& rEvent)
        throw (::css::uno::RuntimeException, ::std::exception) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent)
        throw (::css::uno::RuntimeException, ::std::exception) override;

    void setAcceptedTypes(const std::vector<ObjectType>& aTypes);

private:
    ChartSidebarSelectionListenerParent* mpParent;

    std::vector<ObjectType> maTypes;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
