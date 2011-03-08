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
#include "ObjectNameProvider.hxx"
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>

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
    this->SetText( ObjectNameProvider::getName_ObjectForAllSeries( OBJECTTYPE_DATA_CURVE ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
