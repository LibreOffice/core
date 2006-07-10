/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbfunc2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:09:45 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------


#include "dbfunc.hxx"
#include "docsh.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "globstr.hrc"


// STATIC DATA -----------------------------------------------------------


#ifdef WNT
#pragma optimize ( "", off )
#endif

//==================================================================

class ScDrawLayer;
class ScChartCollection;

void ScDBFunc::UpdateCharts( BOOL bAllCharts )
{
    USHORT nFound = 0;
    ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();

    if ( pDoc->GetDrawLayer() )
        nFound = DoUpdateCharts( ScAddress( pViewData->GetCurX(),
                                            pViewData->GetCurY(),
                                            pViewData->GetTabNo()),
                                 pDoc,
                                 GetActiveWin(),
                                 bAllCharts );

    if ( !nFound && !bAllCharts )
        ErrorMessage(STR_NOCHARTATCURSOR);
}






