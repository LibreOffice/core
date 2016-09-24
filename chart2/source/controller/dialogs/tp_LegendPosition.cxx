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

#include "tp_LegendPosition.hxx"
#include "ResId.hxx"
#include "ResourceIds.hrc"
#include "res_LegendPosition.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include <svx/chrtitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchLegendPosTabPage::SchLegendPosTabPage(vcl::Window* pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage( pWindow
                 ,"tp_LegendPosition"
                 ,"modules/schart/ui/tp_LegendPosition.ui"
                 , &rInAttrs )
    , m_aLegendPositionResources(*this)
{
    get(m_pLbTextDirection,"LB_LEGEND_TEXTDIR");

    m_pLbTextDirection->SetDropDownLineCount(3);
}

SchLegendPosTabPage::~SchLegendPosTabPage()
{
    disposeOnce();
}

void SchLegendPosTabPage::dispose()
{
    m_pLbTextDirection.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SchLegendPosTabPage::Create(vcl::Window* pWindow, const SfxItemSet* rOutAttrs)
{
    return VclPtr<SchLegendPosTabPage>::Create(pWindow, *rOutAttrs);
}

bool SchLegendPosTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    m_aLegendPositionResources.writeToItemSet(*rOutAttrs);

    if( m_pLbTextDirection->GetSelectEntryCount() > 0 )
        rOutAttrs->Put( SvxFrameDirectionItem( m_pLbTextDirection->GetSelectEntryValue(), EE_PARA_WRITINGDIR ) );

    return true;
}

void SchLegendPosTabPage::Reset(const SfxItemSet* rInAttrs)
{
    m_aLegendPositionResources.initFromItemSet(*rInAttrs);

    const SfxPoolItem* pPoolItem = nullptr;
    if( rInAttrs->GetItemState( EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_pLbTextDirection->SelectEntryValue( SvxFrameDirection(static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue()) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
