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

#include "tp_PointGeometry.hxx"
#include "res_BarGeometry.hxx"

#include <chartview/ChartSfxItemIds.hxx>

#include <svl/intitem.hxx>
#include <svx/svx3ditems.hxx>

namespace chart
{

SchLayoutTabPage::SchLayoutTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
     : SfxTabPage(pParent, "modules/schart/ui/tp_ChartType.ui", "tp_ChartType", &rInAttrs)
{
    m_pGeometryResources.reset(new BarGeometryResources(m_xBuilder.get()));
}

SchLayoutTabPage::~SchLayoutTabPage()
{
    disposeOnce();
}

void SchLayoutTabPage::dispose()
{
    m_pGeometryResources.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SchLayoutTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<SchLayoutTabPage>::Create(pParent, *rOutAttrs);
}

bool SchLayoutTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    int nShape = m_pGeometryResources ? m_pGeometryResources->get_selected_index() : -1;
    if (nShape != -1)
    {
        long nSegs=32;

        if (nShape==CHART_SHAPE3D_PYRAMID)
            nSegs=4;

        rOutAttrs->Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nShape));
        rOutAttrs->Put(makeSvx3DHorizontalSegmentsItem(nSegs));
    }
    return true;
}

void SchLayoutTabPage::Reset(const SfxItemSet* rInAttrs)
{
    const SfxPoolItem *pPoolItem = nullptr;

    if (rInAttrs->GetItemState(SCHATTR_STYLE_SHAPE,true, &pPoolItem) == SfxItemState::SET)
    {
        long nVal = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        if(m_pGeometryResources)
        {
            m_pGeometryResources->select(static_cast<sal_uInt16>(nVal));
            m_pGeometryResources->set_visible(true);
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
