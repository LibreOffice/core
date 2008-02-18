/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_Trendline.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:52:27 $
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

#include "tp_Trendline.hxx"
#include "ResId.hxx"
#include "TabPages.hrc"

//.............................................................................
namespace chart
{
//.............................................................................

TrendlineTabPage::TrendlineTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, SchResId( TP_TRENDLINE ), rInAttrs ),
        m_aTrendlineResources( this, rInAttrs, false )
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

BOOL TrendlineTabPage::FillItemSet( SfxItemSet& rOutAttrs )
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

//.............................................................................
} //namespace chart
//.............................................................................
