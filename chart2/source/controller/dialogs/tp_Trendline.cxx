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
#include "TabPages.hrc"

namespace chart
{

TrendlineTabPage::TrendlineTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, SchResId( TP_TRENDLINE ), rInAttrs ),
        m_aTrendlineResources( this, rInAttrs )
{
    FreeResource();
}

TrendlineTabPage::~TrendlineTabPage()
{
}

SfxTabPage* TrendlineTabPage::Create(
    Window* pParent, const SfxItemSet& rOutAttrs )
{
    return new TrendlineTabPage( pParent, rOutAttrs );
}

sal_Bool TrendlineTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    return m_aTrendlineResources.FillItemSet( rOutAttrs );
}

void TrendlineTabPage::Reset( const SfxItemSet& rInAttrs )
{
    m_aTrendlineResources.Reset( rInAttrs );
}

void TrendlineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        m_aTrendlineResources.FillValueSets();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
