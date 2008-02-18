/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertTrendline.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:44:12 $
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

#include "dlg_InsertTrendline.hxx"
#include "dlg_InsertTrendline.hrc"
#include "res_Trendline.hxx"
#include "ResourceIds.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "chartview/ExplicitValueProvider.hxx"
#include "ChartModelHelper.hxx"
#include "ObjectIdentifier.hxx"
#include "DiagramHelper.hxx"
#include "AxisHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif

#include <svtools/controldims.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{
void lcl_SetControlXPos( Control & rControl, long nPosPixel )
{
    Point aPos( rControl.GetPosPixel());
    aPos.setX( nPosPixel );
    rControl.SetPosPixel( aPos );
}
} // anonymous namespace

//.............................................................................
namespace chart
{
//.............................................................................

InsertTrendlineDialog::InsertTrendlineDialog( Window* pParent, const SfxItemSet& rMyAttrs ) :
        ModalDialog( pParent, SchResId( DLG_DATA_TRENDLINE )),
        rInAttrs( rMyAttrs ),
        aBtnOK( this, SchResId( BTN_OK )),
        aBtnCancel( this, SchResId( BTN_CANCEL )),
        aBtnHelp( this, SchResId( BTN_HELP )),
        m_apTrendlineResources( new TrendlineResources( this, rInAttrs, true ))
{
    FreeResource();
    this->SetText( String( SchResId( STR_OBJECT_CURVES )));
}

InsertTrendlineDialog::~InsertTrendlineDialog()
{
}

void InsertTrendlineDialog::adjustSize()
{
    long nControlsRightEdge = m_apTrendlineResources->adjustControlSizes();
    long nButtonXPos = nControlsRightEdge +
        LogicToPixel( Size( RSC_SP_CTRL_X, 10 ), MapMode( MAP_APPFONT )).getWidth();
    lcl_SetControlXPos( aBtnOK, nButtonXPos );
    lcl_SetControlXPos( aBtnCancel, nButtonXPos );
    lcl_SetControlXPos( aBtnHelp, nButtonXPos );

    Size aDialogSize( GetSizePixel());
    aDialogSize.setWidth(
        nControlsRightEdge +
        LogicToPixel( Size( RSC_SP_CTRL_X + RSC_CD_PUSHBUTTON_WIDTH + RSC_SP_DLG_INNERBORDER_RIGHT, 0 ),
                      MapMode( MAP_APPFONT )).getWidth());
    SetSizePixel( aDialogSize );
}

void InsertTrendlineDialog::Reset()
{
    m_apTrendlineResources->Reset(rInAttrs);
}

void InsertTrendlineDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apTrendlineResources->FillItemSet(rOutAttrs);
}

void InsertTrendlineDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    ModalDialog::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        m_apTrendlineResources->FillValueSets();
}

//.............................................................................
} //namespace chart
//.............................................................................
