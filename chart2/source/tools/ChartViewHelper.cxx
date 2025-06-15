/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ChartViewHelper.hxx>
#include <ChartModel.hxx>
#include <ChartView.hxx>
#include <servicenames.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

void ChartViewHelper::setViewToDirtyState(const rtl::Reference<::chart::ChartModel>& xChartModel)
{
    try
    {
        if (xChartModel.is())
        {
            Reference<util::XModifyListener> xModifyListener(xChartModel->createChartView());
            if (xModifyListener.is())
            {
                lang::EventObject aEvent(static_cast<cppu::OWeakObject*>(xChartModel.get()));
                xModifyListener->modified(aEvent);
            }
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ChartViewHelper::setViewToDirtyState_UNO(
    const css::uno::Reference<css::chart2::XChartDocument>& xChartModel)
{
    if (auto pChartModel = dynamic_cast<ChartModel*>(xChartModel.get()))
        setViewToDirtyState(rtl::Reference(pChartModel));
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
