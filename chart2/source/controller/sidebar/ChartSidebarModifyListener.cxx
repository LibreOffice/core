/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartSidebarModifyListener.hxx"

namespace chart {
namespace sidebar {

ChartSidebarModifyListenerParent::~ChartSidebarModifyListenerParent()
{
}

ChartSidebarModifyListener::ChartSidebarModifyListener(ChartSidebarModifyListenerParent* pParent):
    mpParent(pParent)
{
}

ChartSidebarModifyListener::~ChartSidebarModifyListener()
{
}

void ChartSidebarModifyListener::modified(const css::lang::EventObject& /*rEvent*/)
{
    if (mpParent)
        mpParent->updateData();
}

void ChartSidebarModifyListener::disposing(const css::lang::EventObject& /*rEvent*/)
{
    if (!mpParent)
        return;

    mpParent->modelInvalid();
    mpParent = nullptr;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
