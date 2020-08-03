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
#pragma once

#include <memory>
#include <vcl/weld.hxx>
#include <svl/itemset.hxx>

#include "res_ErrorBar.hxx"

namespace com::sun::star::frame { class XModel; }

namespace chart
{

class InsertErrorBarsDialog : public weld::GenericDialogController
{
public:
    InsertErrorBarsDialog(weld::Window* pParent, const SfxItemSet& rMyAttrs,
                          const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument,
                          ErrorBarResources::tErrorBarType eType);

    void SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth );

    static double getAxisMinorStepWidthForErrorBarDecimals(
        const css::uno::Reference< css::frame::XModel >& xChartModel,
        const css::uno::Reference< css::uno::XInterface >& xChartView,
        const OUString& rSelectedObjectCID );

    void FillItemSet( SfxItemSet& rOutAttrs );

private:
    std::unique_ptr< ErrorBarResources >    m_apErrorBarResources;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
