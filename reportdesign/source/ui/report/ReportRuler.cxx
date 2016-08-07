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


#include "precompiled_rptui.hxx"
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
