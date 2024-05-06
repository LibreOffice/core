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
#include <res_BarGeometry.hxx>

#include <chartview/ChartSfxItemIds.hxx>

#include <svl/intitem.hxx>
#include <svx/svx3ditems.hxx>

namespace chart
{

SchLayoutTabPage::SchLayoutTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
     : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_ChartType.ui"_ustr, u"tp_ChartType"_ustr, &rInAttrs)
{
    m_pGeometryResources.reset(new BarGeometryResources(m_xBuilder.get()));
}

SchLayoutTabPage::~SchLayoutTabPage()
{
    m_pGeometryResources.reset();
}

std::unique_ptr<SfxTabPage> SchLayoutTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<SchLayoutTabPage>(pPage, pController, *rOutAttrs);
}

bool SchLayoutTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    int nShape = m_pGeometryResources ? m_pGeometryResources->get_selected_index() : -1;
    if (nShape != -1)
    {
        tools::Long nSegs=32;

        if (nShape==CHART_SHAPE3D_PYRAMID)
            nSegs=4;

        rOutAttrs->Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nShape));
        rOutAttrs->Put(makeSvx3DHorizontalSegmentsItem(nSegs));
    }
    return true;
}

void SchLayoutTabPage::Reset(const SfxItemSet* rInAttrs)
{
    if (const SfxInt32Item* pShapeItem = rInAttrs->GetItemIfSet(SCHATTR_STYLE_SHAPE))
    {
        tools::Long nVal = pShapeItem->GetValue();
        if(m_pGeometryResources)
        {
            m_pGeometryResources->select(static_cast<sal_uInt16>(nVal));
            m_pGeometryResources->set_visible(true);
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
