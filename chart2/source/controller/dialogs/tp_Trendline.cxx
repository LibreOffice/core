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

sal_Bool TrendlineTabPage::FillItemSet( SfxItemSet& rOutAttrs )
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
