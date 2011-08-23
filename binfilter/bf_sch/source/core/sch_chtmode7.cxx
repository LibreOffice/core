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

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

#include "schattr.hxx"
#include "memchrt.hxx"

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTTEXTORDER   SCHATTR_TEXT_ORDER
#define ITEMID_CHARTTEXTORIENT	SCHATTR_TEXT_ORIENT
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#define ITEMID_CHARTDATADESCR   SCHATTR_DATADESCR_DESCR

#include <bf_svtools/eitem.hxx>

#endif


#include <bf_svx/eeitem.hxx>

#define ITEMID_FONT        EE_CHAR_FONTINFO
#define ITEMID_COLOR       EE_CHAR_COLOR
#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#include <bf_svx/svxids.hrc>
#include <globfunc.hxx>





#include "chaxis.hxx"
namespace binfilter {

// Hier abhaengig von der Reihe den Style zurückgeben, vorerst ist nur aufsplitten der
// Verbundcharts von nöten:
//TVM: bitte nicht benutzen! renovierungsbedürftig!


/*N*/ SchMemChart* ChartModel::GetChartData() const
/*N*/ {
/*N*/ 	return pChartData;
/*N*/ }


/*N*/ long ChartModel::GetColCount() const
/*N*/ {
/*N*/ 	if( !pChartData ) return 0;			// GPF via GetMenuState
/*N*/ 	
/*N*/ 	return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/ 			   ? pChartData->GetRowCount()
/*N*/ 			   : pChartData->GetColCount();
/*N*/ }


/*N*/ long ChartModel::GetRowCount() const
/*N*/ {
/*N*/ 	if( !pChartData ) return 0;			// GPF via GetMenuState
/*N*/ 
/*N*/ 	return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/ 			   ? pChartData->GetColCount()
/*N*/ 			   : pChartData->GetRowCount();
/*N*/ }






/*N*/ SvxChartStyle ChartModel::ChartStyle () const
/*N*/ {
/*N*/ 	return eChartStyle;
/*N*/ }


/*N*/ SvxChartStyle &ChartModel::ChartStyle()
/*N*/ {
/*N*/ 	return eChartStyle;
/*N*/ }



/*N*/ long ChartModel::PieSegOfs( long nCol ) const
/*N*/ {
/*N*/     DBG_ASSERT( nCol < nPieSegCount, "pie segment requested is out of bounds" );
/*N*/ 
/*N*/     if( IsPieChart() &&
/*N*/         nCol < nPieSegCount )
/*N*/     {
/*N*/         return pPieSegOfs[ nCol ];
/*N*/     }
/*N*/ 
/*N*/     return 0;
/*N*/ }



/*N*/ BOOL& ChartModel::ShowMainTitle ()
/*N*/ {
/*N*/ 	return bShowMainTitle;
/*N*/ }




/*N*/ String& ChartModel::MainTitle ()
/*N*/ {
/*N*/ 	return aMainTitle;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowSubTitle()
/*N*/ {
/*N*/ 	return bShowSubTitle;
/*N*/ }




/*N*/ String& ChartModel::SubTitle ()
/*N*/ {
/*N*/ 	return aSubTitle;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowXAxisTitle ()
/*N*/ {
/*N*/ 	return bShowXAxisTitle;
/*N*/ }


/*N*/ String& ChartModel::XAxisTitle ()
/*N*/ {
/*N*/ 	return aXAxisTitle;
/*N*/ }






/*N*/ BOOL& ChartModel::ShowYAxisTitle ()
/*N*/ {
/*N*/ 	return bShowYAxisTitle;
/*N*/ }
/*N*/ 



/*N*/ String& ChartModel::YAxisTitle ()
/*N*/ {
/*N*/ 	return aYAxisTitle;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowZAxisTitle ()
/*N*/ {
/*N*/ 	return bShowZAxisTitle;
/*N*/ }




/*N*/ String& ChartModel::ZAxisTitle()
/*N*/ {
/*N*/ 	return aZAxisTitle;
/*N*/ }


/*N*/ BOOL ChartModel::ShowXAxis () const
/*N*/ {
/*N*/ 	return pChartXAxis->IsVisible();
/*N*/ }


/*N*/ void ChartModel::ShowXAxis (BOOL b)
/*N*/ {
/*N*/ 	pChartXAxis->ShowAxis(b);
/*N*/ }




/*N*/ BOOL& ChartModel::ShowXGridMain ()
/*N*/ {
/*N*/ 	return bShowXGridMain;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowXGridHelp ()
/*N*/ {
/*N*/ 	return bShowXGridHelp;
/*N*/ }


/*N*/ BOOL ChartModel::ShowXDescr () const
/*N*/ {
/*N*/ 	return pChartXAxis->HasDescription();
/*N*/ }


/*N*/ void ChartModel::ShowXDescr(BOOL b)
/*N*/ {
/*N*/ 	pChartXAxis->ShowDescr(b);
/*N*/ }


/*N*/ BOOL ChartModel::ShowYAxis () const
/*N*/ {
/*N*/ 	return pChartYAxis->IsVisible();
/*N*/ }
/*N*/ void ChartModel::ShowYAxis (BOOL b)
/*N*/ {
/*N*/ 	pChartYAxis->ShowAxis(b);
/*N*/ }



/*N*/ BOOL& ChartModel::ShowYGridMain ()
/*N*/ {
/*N*/ 	return bShowYGridMain;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowYGridHelp ()
/*N*/ {
/*N*/ 	return bShowYGridHelp;
/*N*/ }


/*N*/ BOOL ChartModel::ShowYDescr () const
/*N*/ {
/*N*/ 	return pChartYAxis->HasDescription();
/*N*/ }


/*N*/ void ChartModel::ShowYDescr( BOOL b)
/*N*/ {
/*N*/ 	pChartYAxis->ShowDescr(b);
/*N*/ }


/*N*/ BOOL ChartModel::ShowZAxis () const
/*N*/ {
/*N*/ 	return pChartZAxis->IsVisible();
/*N*/ }


/*N*/ void ChartModel::ShowZAxis(BOOL b)
/*N*/ {
/*N*/ 	pChartZAxis->ShowAxis(b);
/*N*/ }




/*N*/ BOOL& ChartModel::ShowZGridMain ()
/*N*/ {
/*N*/ 	return bShowZGridMain;
/*N*/ }




/*N*/ BOOL& ChartModel::ShowZGridHelp ()
/*N*/ {
/*N*/ 	return bShowZGridHelp;
/*N*/ }


/*N*/ BOOL ChartModel::ShowZDescr () const
/*N*/ {
/*N*/ 	return pChartZAxis->HasDescription();
/*N*/ }


/*N*/ void ChartModel::ShowZDescr (BOOL b)
/*N*/ {
/*N*/ 	pChartZAxis->ShowDescr(b);
/*N*/ }


/*N*/ BOOL ChartModel::HasAxis( long nObjectId ) const
/*N*/ {
/*N*/     if( CHOBJID_ANY == nObjectId )
/*N*/         return IsAxisChart() &&
/*N*/             ( pChartXAxis->IsVisible() ||
/*N*/               pChartYAxis->IsVisible() ||
/*N*/               pChartAAxis->IsVisible() ||
/*N*/               pChartBAxis->IsVisible() ||
/*N*/               (Is3DChart() && pChartZAxis->IsVisible()) );
/*N*/ 
/*N*/     switch( nObjectId )
/*N*/     {
/*N*/         case CHOBJID_DIAGRAM_X_AXIS:
/*N*/             return  pChartXAxis->IsVisible();
/*N*/         case CHOBJID_DIAGRAM_Y_AXIS:
/*N*/             return  pChartYAxis->IsVisible();
/*N*/         case CHOBJID_DIAGRAM_Z_AXIS:
/*N*/             return  (Is3DChart() && pChartZAxis->IsVisible());
/*N*/         case CHOBJID_DIAGRAM_A_AXIS:
/*N*/             return  pChartAAxis->IsVisible();
/*N*/         case CHOBJID_DIAGRAM_B_AXIS:
/*N*/             return  pChartBAxis->IsVisible();
/*N*/     }
/*N*/ 
/*N*/     return FALSE;
/*N*/ }


/*N*/ const SfxItemSet& ChartModel::GetLegendAttr() const
/*N*/ {
/*N*/ 	return *pLegendAttr;
/*N*/ }









//ToDo: diese Fkt. überflüssig machen (Problem: Austausch der Achseneigenschaften nicht NUR über Attr (??? stimmt das ???)













































/*N*/ SdrOutliner* ChartModel::GetOutliner() const
/*N*/ {
/*N*/ 	return pOutliner;
/*N*/ }

/*
UINT32 ChartModel::ValFor mat () const
{
    return nValFo rmat;
}


UINT32& ChartModel::ValForm at()
{
    return nVal Format;
}


UINT32 ChartModel::PercentVa lFormat () const
{
    return nPercentV alFormat;
}


UINT32& ChartModel::Per centValFormat ()
{
    return nPercentValFo rmat;
}


UINT32 ChartModel::Des crFormat () const
{
    return nDescrFor mat;
}


UINT32& ChartModel::Desc rFormat()
{
    return nDes crFormat;
}


UINT32 ChartModel::PercentD escrFormat () const
{
    return nPercentDescrFo rmat;
}


UINT32& ChartModel::Percent DescrF ormat ()
{
    return nPercentDescr Format;
}

*/
/*N*/ BOOL ChartModel::IsInitialized() const
/*N*/ {
/*N*/ 	return mbIsInitialized;
/*N*/ }




/*N*/ BOOL& ChartModel::ReadError ()
/*N*/ {
/*N*/ 	return bReadError;
/*N*/ }


/*N*/ ChartScene* ChartModel::GetScene()
/*N*/ {
/*N*/ 	return pScene;
/*N*/ }

/*N*/ SvNumberFormatter* ChartModel::GetNumFormatter() const
/*N*/ {
/*N*/ 	return pNumFormatter;
/*N*/ }






























/*N*/ int& ChartModel::Granularity ()
/*N*/ {
/*N*/ 	return nGranularity;
/*N*/ }




/*N*/ Size& ChartModel::InitialSize ()
/*N*/ {
/*N*/ 	return aInitialSize;
/*N*/ }


















/*N*/ void ChartModel::SetPieSegOfs( long nCol, long nOfs )
/*N*/ {
/*N*/     DBG_ASSERT( pPieSegOfs, "Invalid Array" );
/*N*/     DBG_ASSERT( nCol < nPieSegCount, "trying to set pie offset out of bounds" );
/*N*/ 
/*N*/     if( nCol < nPieSegCount )
/*N*/         pPieSegOfs[ nCol ] = nOfs;
/*N*/ }


/*N*/ String& ChartModel::ColText( long nCol )
/*N*/ {
/*N*/ 	return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/ 			   ? (String&) pChartData->GetTransRowText(nCol)
/*N*/ 			   : (String&) pChartData->GetTransColText(nCol);
/*N*/ }


/*N*/ String& ChartModel::RowText( long nRow )
/*N*/ {
/*N*/ 	return IsDataSwitched()   //abhaengig vom Charttyp - statt bSwitchData
/*N*/ 			   ? (String&) pChartData->GetTransColText(nRow)
/*N*/ 			   : (String&) pChartData->GetTransRowText(nRow);
/*N*/ }




// FG: Ist eine Abfrage ob die Beschriftung eines Charts Zentriert unter einem Datenpunkt (Regelfall) steht
//     oder zentriert zwischen 2 Marken.
/*N*/ BOOL ChartModel::IsDescriptionCenteredUnderDataPoint ()
/*N*/ {
/*N*/ 	switch (eChartStyle)
/*N*/ 	{
/*N*/ 		case CHSTYLE_2D_STACKEDCOLUMN :
/*N*/ 		case CHSTYLE_2D_COLUMN :
/*N*/ 		case CHSTYLE_2D_PERCENTCOLUMN :
/*N*/ 			return FALSE;

//  		case CHSTYLE_2D_LINE :
//  		case CHSTYLE_2D_STACKEDLINE :
//  		case CHSTYLE_2D_PERCENTLINE :
//  		case CHSTYLE_2D_LINESYMBOLS :
//  		case CHSTYLE_2D_STACKEDLINESYM :
//  		case CHSTYLE_2D_PERCENTLINESYM :
//  		case CHSTYLE_2D_CUBIC_SPLINE :
//  		case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL :
//  		case CHSTYLE_2D_B_SPLINE :
//  		case CHSTYLE_2D_B_SPLINE_SYMBOL :

//  		case CHSTYLE_2D_XY :
//  		case CHSTYLE_2D_XYSYMBOLS :
//  		case CHSTYLE_2D_XY_LINE :
//  		case CHSTYLE_2D_CUBIC_SPLINE_XY :
//  		case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY :
//  		case CHSTYLE_2D_B_SPLINE_XY :
//  		case CHSTYLE_2D_B_SPLINE_SYMBOL_XY :

//  		case CHSTYLE_2D_BAR :
//  		case CHSTYLE_2D_STACKEDBAR:
//  		case CHSTYLE_2D_PERCENTBAR:

//  		case CHSTYLE_2D_AREA :
//  		case CHSTYLE_2D_PERCENTAREA :
//  		case CHSTYLE_2D_STACKEDAREA :

//  		case CHSTYLE_2D_STOCK_1:
//  		case CHSTYLE_2D_STOCK_2:
//  		case CHSTYLE_2D_STOCK_3:
//  		case CHSTYLE_2D_STOCK_4:
/*N*/ 		default :
/*N*/ 			return TRUE;
/*N*/ 	}
/*N*/ }
/*N*/ void ChartModel::LockBuild()
/*N*/ {
/*N*/ 	bNoBuildChart=TRUE;
/*N*/ }
/*N*/ void ChartModel::UnlockBuild()
/*N*/ {
/*N*/ 	bNoBuildChart=FALSE;
/*N*/ 	if(bShouldBuildChart)
/*N*/ 		BuildChart(FALSE); //evtl. TRUE, um Ranges zu prüfen???
/*N*/ 
/*N*/ 	// force broadcast SFX_HINT_DOCCHANGED
/*N*/ 	SfxObjectShell* pObjSh = GetObjectShell();
/*N*/ 	if( pObjSh )
/*N*/ 	{
/*N*/ 		pObjSh->SetModified( pObjSh->IsModified());
/*N*/ 	}
/*N*/ }
/*N*/ BOOL ChartModel::IsLockedBuild()
/*N*/ {
/*N*/ 	return bNoBuildChart;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
