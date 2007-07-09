/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportRuler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:32 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_RULER_HXX
#include "ReportRuler.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif
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
void OReportRuler::SetGridVisible(BOOL _bVisible)
{
    m_pSection->SetGridVisible( _bVisible );
}
//------------------------------------------------------------------------------
//==============================================================================
} // rptui
//==============================================================================
