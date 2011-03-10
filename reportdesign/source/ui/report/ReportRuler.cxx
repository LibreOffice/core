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
#include "precompiled_reportdesign.hxx"
#include "ReportRuler.hxx"
#include "ReportSection.hxx"
#include "ReportWindow.hxx"
namespace rptui
{
using namespace ::com::sun::star;
DBG_NAME( rpt_OReportRuler )
OReportRuler::OReportRuler(Window* _pParent,OReportWindow* _pReportWindow,const uno::Reference< report::XSection >& _xSection)
:Window(_pParent)
,m_pParent(_pReportWindow)
{
    DBG_CTOR( rpt_OReportRuler,NULL);
    SetMapMode( MapMode( MAP_100TH_MM ) );

    Show();
}
//------------------------------------------------------------------------------
OReportRuler::~OReportRuler()
{
    DBG_DTOR( rpt_OReportRuler,NULL);
    delete m_pSection;
}
//------------------------------------------------------------------------------
void OReportRuler::Resize()
{
    m_pSection->SetPosSizePixel(Point(0,0),GetOutputSize());
}
//------------------------------------------------------------------------------
void OReportRuler::SetGridVisible(sal_Bool _bVisible)
{
    m_pSection->SetGridVisible( _bVisible );
}
//------------------------------------------------------------------------------
//==============================================================================
} // rptui
//==============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
