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
#include "TabPages.hrc"
#include "res_BarGeometry.hxx"
#include "ResId.hxx"

#include "chartview/ChartSfxItemIds.hxx"

// header for SfxInt32Item
#include <svl/intitem.hxx>
// header for class Svx3DHorizontalSegmentsItem
#include <svx/svx3ditems.hxx>

namespace chart
{

SchLayoutTabPage::SchLayoutTabPage(Window* pWindow,const SfxItemSet& rInAttrs)
                 : SfxTabPage(pWindow, SchResId(TP_LAYOUT), rInAttrs)
                 , m_pGeometryResources(0)
{
    Point aPos( this->LogicToPixel( Point(6,6), MapMode(MAP_APPFONT) ) );
    m_pGeometryResources = new BarGeometryResources( this );
    m_pGeometryResources->SetPosPixel( aPos );
}

SchLayoutTabPage::~SchLayoutTabPage()
{
    if( m_pGeometryResources )
        delete m_pGeometryResources;
}

SfxTabPage* SchLayoutTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchLayoutTabPage(pWindow, rOutAttrs);
}

sal_Bool SchLayoutTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{

    if(m_pGeometryResources && m_pGeometryResources->GetSelectEntryCount())
    {
        long nSegs=32;

        long nShape = m_pGeometryResources->GetSelectEntryPos();
        if(nShape==CHART_SHAPE3D_PYRAMID)
            nSegs=4;

        rOutAttrs.Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nShape));
        rOutAttrs.Put(Svx3DHorizontalSegmentsItem(nSegs));
    }
    return sal_True;
}

void SchLayoutTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    if (rInAttrs.GetItemState(SCHATTR_STYLE_SHAPE,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
        if(m_pGeometryResources)
        {
            m_pGeometryResources->SelectEntryPos(static_cast<sal_uInt16>(nVal));
            m_pGeometryResources->Show( true );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
