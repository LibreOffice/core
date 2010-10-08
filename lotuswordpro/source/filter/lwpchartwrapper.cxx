/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "lwpchartwrapper.hxx"
#include <sschart/charttools.hxx>
using namespace NAME_CHART;

LtcLwpChartWrapper::LtcLwpChartWrapper(SvStream* pStream,UINT32 nStrmLen)
{
    m_pImpChart.reset(new LtcImpChart(pStream,nStrmLen) );

    m_spDatahandler.reset(new ltcDataWindowHandler());
    m_pImpChart->SetData(m_spDatahandler);
}

sal_Bool LtcLwpChartWrapper::Read()
{
    sal_Bool bRet = sal_False;
    m_pImpChart->SetStrmOffset(0);
    chtFltError eRet = m_pImpChart->Read();
    if (eChtERR_OK==eRet)
    {
        bRet = sal_True;
    }
    return bRet;
}

void LtcLwpChartWrapper::CreateChart(Rectangle aChartRect,SvInPlaceObjectRef& xIPObj,SvStorageRef& xStorage)
{
    m_pImpChart->CreateChartObj(aChartRect, xIPObj, xStorage);
}

//////////////////////////////////////////////////////////////////////////
void LtcLwpChartDataHandler::ApplyAreaStyle()
{
    CHART_TYPE eChartType = GetChartType();
    if ( eChartType==RADAR || eChartType==RADAR_AREA )
    {
        LtcChartTools::ApplyRadarDefaultArea(this,m_xChartDoc);
    }
}
