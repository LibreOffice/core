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

#include <dlg_ChartType.hxx>
#include "tp_ChartType.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

ChartTypeDialog::ChartTypeDialog(weld::Window* pParent,
                                 const uno::Reference<frame::XModel>& xChartModel)
    : GenericDialogController(pParent, "modules/schart/ui/charttypedialog.ui", "ChartTypeDialog")
    , m_xChartModel(xChartModel)
    , m_xContentArea(m_xDialog->weld_content_area())
{
    m_xChartTypeTabPage = std::make_unique<ChartTypeTabPage>(
        m_xContentArea.get(), this, uno::Reference<XChartDocument>::query(m_xChartModel),
        false /*don't show title description*/);

    m_xChartTypeTabPage->initializePage();
}

ChartTypeDialog::~ChartTypeDialog() { m_xChartTypeTabPage.reset(); }

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
