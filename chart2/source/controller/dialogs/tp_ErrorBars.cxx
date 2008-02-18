/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_ErrorBars.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:49:09 $
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

#include "tp_ErrorBars.hxx"
#include "ResId.hxx"
#include "TabPages.hrc"

//.............................................................................
namespace chart
{
//.............................................................................

ErrorBarsTabPage::ErrorBarsTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, SchResId( TP_YERRORBAR ), rInAttrs ),
        m_aErrorBarResources( this, rInAttrs )
{
    FreeResource();
}

ErrorBarsTabPage::~ErrorBarsTabPage()
{
}

SfxTabPage* ErrorBarsTabPage::Create(
    Window* pParent, const SfxItemSet& rOutAttrs )
{
    return new ErrorBarsTabPage( pParent, rOutAttrs );
}

BOOL ErrorBarsTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    return m_aErrorBarResources.FillItemSet( rOutAttrs );
}

void ErrorBarsTabPage::Reset( const SfxItemSet& rInAttrs )
{
    m_aErrorBarResources.Reset( rInAttrs );
}

void ErrorBarsTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        m_aErrorBarResources.FillValueSets();
}

void ErrorBarsTabPage::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    m_aErrorBarResources.SetAxisMinorStepWidthForErrorBarDecimals( fMinorStepWidth );
}

void ErrorBarsTabPage::SetErrorBarType( ErrorBarResources::tErrorBarType eNewType )
{
    m_aErrorBarResources.SetErrorBarType( eNewType );
}

//.............................................................................
} //namespace chart
//.............................................................................
