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

#include "tp_Trendline.hxx"
#include "ResId.hxx"
#include "ResourceIds.hrc"
#include <vcl/settings.hxx>

namespace chart
{

TrendlineTabPage::TrendlineTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, "TP_TRENDLINE", "modules/schart/ui/tp_Trendline.ui", &rInAttrs ),
        m_aTrendlineResources( this, rInAttrs )
{
}

VclPtr<SfxTabPage> TrendlineTabPage::Create(
    vcl::Window* pParent, const SfxItemSet* rOutAttrs )
{
    return VclPtr<TrendlineTabPage>::Create( pParent, *rOutAttrs );
}

bool TrendlineTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    return m_aTrendlineResources.FillItemSet( rOutAttrs );
}

void TrendlineTabPage::Reset( const SfxItemSet* rInAttrs )
{
    m_aTrendlineResources.Reset( *rInAttrs );
}

void TrendlineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
        m_aTrendlineResources.FillValueSets();
}

void TrendlineTabPage::SetNumFormatter( SvNumberFormatter* pNumFormatter )
{
    m_aTrendlineResources.SetNumFormatter( pNumFormatter );
}

void TrendlineTabPage::SetNbPoints( sal_Int32 nNbPoints )
{
    m_aTrendlineResources.SetNbPoints( nNbPoints );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
