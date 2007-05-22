/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_PointGeometry.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:47:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
// header for class Svx3DHorizontalSegmentsItem
#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif

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

BOOL SchLayoutTabPage::FillItemSet(SfxItemSet& rOutAttrs)
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
    return TRUE;
}

void SchLayoutTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    if (rInAttrs.GetItemState(SCHATTR_STYLE_SHAPE,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
        if(m_pGeometryResources)
        {
            m_pGeometryResources->SelectEntryPos(static_cast<USHORT>(nVal));
            m_pGeometryResources->Show( true );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
