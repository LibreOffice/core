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
#include "ResourceIds.hrc"
#include "res_BarGeometry.hxx"
#include "ResId.hxx"

#include "chartview/ChartSfxItemIds.hxx"

#include <svl/intitem.hxx>
#include <svx/svx3ditems.hxx>

namespace chart
{

SchLayoutTabPage::SchLayoutTabPage(vcl::Window* pWindow,const SfxItemSet& rInAttrs)
     : SfxTabPage(pWindow, "tp_ChartType", "modules/schart/ui/tp_ChartType.ui", &rInAttrs)
     , m_pGeometryResources(nullptr)
{
    m_pGeometryResources = new BarGeometryResources( this );
}

SchLayoutTabPage::~SchLayoutTabPage()
{
    disposeOnce();
}

void SchLayoutTabPage::dispose()
{
    delete m_pGeometryResources;
    m_pGeometryResources = nullptr;
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SchLayoutTabPage::Create(vcl::Window* pWindow,
                                            const SfxItemSet* rOutAttrs)
{
    return VclPtr<SchLayoutTabPage>::Create(pWindow, *rOutAttrs);
}

bool SchLayoutTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{

    if(m_pGeometryResources && m_pGeometryResources->GetSelectEntryCount())
    {
        long nSegs=32;

        long nShape = m_pGeometryResources->GetSelectEntryPos();
        if(nShape==CHART_SHAPE3D_PYRAMID)
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
            m_pGeometryResources->SelectEntryPos(static_cast<sal_uInt16>(nVal));
            m_pGeometryResources->Show( true );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
