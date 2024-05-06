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

#include "tp_ErrorBars.hxx"

using namespace ::com::sun::star;

namespace chart
{

ErrorBarsTabPage::ErrorBarsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_ErrorBars.ui"_ustr, u"tp_ErrorBars"_ustr, &rInAttrs)
    , m_aErrorBarResources(m_xBuilder.get(), pController, rInAttrs, /* bNoneAvailable = */ false)
{
}

std::unique_ptr<SfxTabPage> ErrorBarsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<ErrorBarsTabPage>(pPage, pController, *rOutAttrs);
}

bool ErrorBarsTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    m_aErrorBarResources.FillItemSet( *rOutAttrs );
    return true;
}

void ErrorBarsTabPage::Reset( const SfxItemSet* rInAttrs )
{
    m_aErrorBarResources.Reset( *rInAttrs );
}

void ErrorBarsTabPage::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    m_aErrorBarResources.SetAxisMinorStepWidthForErrorBarDecimals( fMinorStepWidth );
}

void ErrorBarsTabPage::SetErrorBarType( ErrorBarResources::tErrorBarType eNewType )
{
    m_aErrorBarResources.SetErrorBarType( eNewType );
}

void ErrorBarsTabPage::SetChartDocumentForRangeChoosing(
    const rtl::Reference<::chart::ChartModel> & xChartDocument )
{
    m_aErrorBarResources.SetChartDocumentForRangeChoosing( xChartDocument );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
