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
#include "TabPages.hrc"
#include "res_LegendPosition.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include <svx/chrtitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchLegendPosTabPage::SchLegendPosTabPage(Window* pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage( pWindow
                 ,"tp_LegendPosition"
                 ,"modules/schart/ui/tp_LegendPosition.ui"
                 , rInAttrs )
    , m_aLegendPositionResources(*this)
{
    get(m_pLbTextDirection,"LB_LEGEND_TEXTDIR");

    m_pLbTextDirection->SetDropDownLineCount(3);
}

SfxTabPage* SchLegendPosTabPage::Create(Window* pWindow, const SfxItemSet& rOutAttrs)
{
    return new SchLegendPosTabPage(pWindow, rOutAttrs);
}

sal_Bool SchLegendPosTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_aLegendPositionResources.writeToItemSet(rOutAttrs);

    if( m_pLbTextDirection->GetSelectEntryCount() > 0 )
        rOutAttrs.Put( SfxInt32Item( EE_PARA_WRITINGDIR, m_pLbTextDirection->GetSelectEntryValue() ) );

    return sal_True;
}

void SchLegendPosTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_aLegendPositionResources.initFromItemSet(rInAttrs);

    const SfxPoolItem* pPoolItem = 0;
    if( rInAttrs.GetItemState( EE_PARA_WRITINGDIR, true, &pPoolItem ) == SFX_ITEM_SET )
        m_pLbTextDirection->SelectEntryValue( SvxFrameDirection(((const SvxFrameDirectionItem*)pPoolItem)->GetValue()) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
