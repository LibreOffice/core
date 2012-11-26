/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "tp_PointGeometry.hxx"
#include "TabPages.hrc"
#include "res_BarGeometry.hxx"
#include "ResId.hxx"

#include "chartview/ChartSfxItemIds.hxx"
#include <svl/intitem.hxx>
#include <svx/svddef.hxx>

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
        rOutAttrs.Put(SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, nSegs));
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
