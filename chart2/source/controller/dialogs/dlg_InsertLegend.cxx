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
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertLegend.hrc"
#include "res_LegendPosition.hxx"
#include "ObjectNameProvider.hxx"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

// header for enum SvxChartLegendPos
#include <svx/chrtitem.hxx>
// header for class SfxItemPool
#include <svl/itempool.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;

SchLegendDlg::SchLegendDlg(Window* pWindow, const uno::Reference< uno::XComponentContext>& xCC )
    : ModalDialog(pWindow, SchResId(DLG_LEGEND))
    , m_apLegendPositionResources( new LegendPositionResources(this,xCC) )
    , aBtnOK(this, SchResId(BTN_OK))
    , aBtnCancel(this, SchResId(BTN_CANCEL))
    , aBtnHelp(this, SchResId(BTN_HELP))
{
    FreeResource();
    this->SetText( ObjectNameProvider::getName(OBJECTTYPE_LEGEND) );
}

SchLegendDlg::~SchLegendDlg()
{
}

void SchLegendDlg::init( const uno::Reference< frame::XModel >& xChartModel )
{
    m_apLegendPositionResources->writeToResources( xChartModel );
}

bool SchLegendDlg::writeToModel( const uno::Reference< frame::XModel >& xChartModel ) const
{
    m_apLegendPositionResources->writeToModel( xChartModel );
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
