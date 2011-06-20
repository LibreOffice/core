/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "tp_PointGeometry.hxx"
#include "TabPages.hrc"
#include "res_BarGeometry.hxx"
#include "ResId.hxx"

#include "chartview/ChartSfxItemIds.hxx"

// header for SfxInt32Item
#include <svl/intitem.hxx>
// header for class Svx3DHorizontalSegmentsItem
#include <svx/svx3ditems.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

SchLayoutTabPage::SchLayoutTabPage(Window* pWindow,const SfxItemSet& rInAttrs)
                 : SfxTabPage(pWindow, SchResId(TP_LAYOUT), rInAttrs)
                 , m_pGeometryResources(0)
{
    Size aPageSize( this->GetSizePixel() );
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
        long nShape=CHART_SHAPE3D_SQUARE;
        long nSegs=32;

        nShape = m_pGeometryResources->GetSelectEntryPos();
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

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
