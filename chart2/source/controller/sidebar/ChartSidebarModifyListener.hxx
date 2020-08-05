/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/util/XModifyListener.hpp>
#include <cppuhelper/implbase.hxx>

namespace chart::sidebar {

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
    virtual ~ChartSidebarModifyListener() override;

    virtual void SAL_CALL modified(const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override;

private:
    ChartSidebarModifyListenerParent* mpParent;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
