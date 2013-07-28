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
#include "ResId.hxx"
#include "TabPages.hrc"
#include "TabPageNotifiable.hxx"

using namespace ::com::sun::star;

namespace chart
{

ErrorBarsTabPage::ErrorBarsTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, SchResId( TP_YERRORBAR ), rInAttrs ),
        m_aErrorBarResources(
            this,
            // the parent is the tab control, of which the parent is the dialog
            pParent->GetParentDialog(),
            rInAttrs, /* bNoneAvailable = */ false )
{
    FreeResource();
}

ErrorBarsTabPage::~ErrorBarsTabPage()
{
}

SfxTabPage* ErrorBarsTabPage::Create(
    Window* pParent, const SfxItemSet& rOutAttrs )
{
    return new ErrorBarsTabPage( pParent, rOutAttrs );
}

sal_Bool ErrorBarsTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    return m_aErrorBarResources.FillItemSet( rOutAttrs );
}

void ErrorBarsTabPage::Reset( const SfxItemSet& rInAttrs )
{
    m_aErrorBarResources.Reset( rInAttrs );
}

void ErrorBarsTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        m_aErrorBarResources.FillValueSets();
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
    const uno::Reference< chart2::XChartDocument > & xChartDocument )
{
    m_aErrorBarResources.SetChartDocumentForRangeChoosing( xChartDocument );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
