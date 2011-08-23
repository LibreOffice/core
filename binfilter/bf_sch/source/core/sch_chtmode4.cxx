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

#include "pairs.hxx"

#include <bf_svx/eeitem.hxx>
#include <bf_sfx2/sfxsids.hrc>
#ifdef _MSC_VER
#pragma hdrstop
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <bf_svx/svdopath.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <bf_svtools/itempool.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <bf_svx/svdorect.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <bf_sfx2/printer.hxx>
#endif

#include <bf_svx/editdata.hxx>
#ifndef _SCHATTR_HXX
#include "schattr.hxx"
#endif

#ifndef _SVX_CHRTITEM_HXX //autogen
#define ITEMID_DOUBLE	        0
#define ITEMID_CHARTDATADESCR	SCHATTR_DATADESCR_DESCR
#define ITEMID_CHARTSTYLE       CHATTR_DIAGRAM_STYLE
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS

#ifndef _SFXENUMITEM_HXX
#include <bf_svtools/eitem.hxx>
#endif

#include <bf_svx/chrtitem.hxx>
#endif

#define ITEMID_FONTHEIGHT  EE_CHAR_FONTHEIGHT
#define ITEMID_FONTWIDTH   EE_CHAR_FONTWIDTH
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/fwdtitem.hxx>
#ifndef _CHTMODEL_HXX
#include <globfunc.hxx>
#include <chtmodel.hxx>
#endif
#include <bf_svx/svdoutl.hxx>	// wg. SdrOutliner
#ifndef __SVDPAGE_HXX
#include <bf_svx/svdpage.hxx>
#endif
#ifndef _SCH_OBJADJ_HXX
#include  "objadj.hxx"
#endif
#ifndef _SCH_SCHRESID_HXX
#include "schresid.hxx"
#endif
#ifndef _SCH_OBJID_HXX
#include "objid.hxx"
#endif
#ifndef _SCH_DATAROW_HXX
#include "datarow.hxx"
#endif
#ifndef _SVX_DLGUTIL_HXX
#include  <bf_svx/dlgutil.hxx>
#endif

#include "chtscene.hxx"
#include "glob.hrc"
#include <math.h>
#include <float.h>

#include "chmod3d.hxx"
#include "docshell.hxx"

#ifndef _SFXAPP_HXX //autogen
#include <bf_sfx2/app.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef	_RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include "chaxis.hxx"
#include "ChXChartDocument.hxx"

#include "ReBuildHint.hxx"
namespace binfilter {

/************************************************************************/

/*************************************************************************
|*
|* Achsen & Gitter aendern;
|* Liefert bei Aenderung TRUE.
|*
\************************************************************************/





/*************************************************************************
|*
|* Datenbeschriftung aendern;
|* FG: 24.4.97 nRow ist ein Parameter der ermoeglicht die Datenbeschriftung
|*     einer einzigen Reihe zu aendern, falls er -1 ist werden alle
|*     Datenreihen mit dem uebergebenen Attributen belegt. (default ist alle)
|*
\************************************************************************/

/*N*/ void ChartModel::ChangeDataDescr(SvxChartDataDescr eDescr, BOOL bSym,
/*N*/ 								 long nRowToChange, BOOL bBuildChart)
/*N*/ {
/*N*/     if( nRowToChange == -1 ||
/*N*/         IsPieChart())
/*N*/     {
/*N*/         // change setting globally
/*N*/         eDataDescr = eDescr;
/*N*/         bShowSym = bSym;
/*N*/ 
/*N*/         // clear individual settings
/*N*/         long nEndRow = GetRowCount();
/*N*/         for( long nRow = 0; nRow < nEndRow; nRow++ )
/*N*/         {
/*N*/             SfxItemSet &rAttr = SAL_CONST_CAST( SfxItemSet&, GetDataRowAttr( (short)nRow ));
/*N*/ 
/*N*/             rAttr.Put( SvxChartDataDescrItem( eDescr ));
/*N*/             rAttr.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYM, bSym ));
/*N*/ 
/*N*/             // the following is the preferred method, but the items are not evaluated appropriately
/*N*/ //              rAttr.ClearItem( ITEMID_CHARTDATADESCR );
/*N*/ //              rAttr.ClearItem( SCHATTR_DATADESCR_SHOW_SYM );
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         SfxItemSet &rAttr = SAL_CONST_CAST( SfxItemSet&, GetDataRowAttr( (short)nRowToChange ));
/*N*/ 
/*N*/  		rAttr.Put( SvxChartDataDescrItem( eDescr ));
/*N*/  		rAttr.Put( SfxBoolItem( SCHATTR_DATADESCR_SHOW_SYM, bSym ));
/*N*/     }
/*N*/ 
/*N*/ 	if( bBuildChart )
/*?*/ 		BuildChart( FALSE );
/*N*/ }

/*************************************************************************
|*
|* Seitengroesse aendern;
|* Liefert bei Aenderung TRUE.
|*
\************************************************************************/

/*N*/ BOOL ChartModel::ResizePage(const Size& rNewSize)
/*N*/ {
/*N*/ 	SdrPage* pPage = GetPage(0);
/*N*/ 
/*N*/ 	if (!pPage || pPage->GetSize() == rNewSize) return FALSE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bWasChanged = IsChanged();
/*N*/ 
/*N*/ 		pPage->SetSize(rNewSize);
/*N*/ 		eOldChartStyle = eChartStyle;
/*N*/ 		if (rNewSize.Width () != 0 && rNewSize.Height () != 0) BuildChart(FALSE);
/*N*/ 
/*N*/ 		if (!bWasChanged) SetChanged(FALSE);
/*N*/ 
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ }


/************************************************************************
|*
|* skaliere Texte neu um
|*
\************************************************************************/

/*N*/ void ChartModel::ResizeText (SfxItemSet *pTextAttr, Size aPageSize, BOOL bResizePage )
/*N*/ {
/*N*/ 	double fRatio;
/*N*/ 	if( bResizePage )
/*N*/ 		fRatio = (double)aPageSize.Height() / (double)aInitialSize.Height();
/*N*/ 	else
/*?*/ 		fRatio = (double)aDiagramRectangle.GetHeight() / (double)aLastDiagramRectangle.GetHeight();
/*N*/ 
/*N*/ 	// resize all three fonts
/*N*/ 	static const USHORT nWhichIds[ 3 ] = {
/*N*/ 		EE_CHAR_FONTHEIGHT,
/*N*/ 		EE_CHAR_FONTHEIGHT_CJK,
/*N*/ 		EE_CHAR_FONTHEIGHT_CTL
/*N*/ 	};
/*N*/ 
/*N*/ 	for( int i=0; i < 3; i++ )
/*N*/ 	{
/*N*/ 		long nHeight = ((SvxFontHeightItem &)pTextAttr->Get( nWhichIds[ i ] )).GetHeight();
/*N*/ 
/*N*/         //	Scale and round height.
/*N*/ 		nHeight = (long)( fRatio * (double)nHeight + 0.5 );
/*N*/ 
/*N*/ 		float fPoints = (float)CalcToPoint( nHeight, SFX_MAPUNIT_100TH_MM, 10 );
/*N*/ 		if( fPoints < 2.0 ) fPoints = 2.0;
/*N*/ 
/*N*/ 		pTextAttr->Put( SvxFontHeightItem( CalcToUnit( fPoints / (float)10.0, SFX_MAPUNIT_100TH_MM ),
/*N*/ 										   100, nWhichIds[ i ] ));
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Textobjekt fuer Editieren vorbereiten
|*
\************************************************************************/


/*************************************************************************
|*
|* Editieren abschlieáen
|*
\************************************************************************/


/*************************************************************************
|*
|* Attribute lesen
|*
\************************************************************************/

//SfxItemSet ChartModel::GetAttr() const
/*N*/ void ChartModel::GetAttr(SfxItemSet& rAttr)
/*N*/ {
/*N*/ 	CHART_TRACE( "ChartModel::GetAttr(SfxItemSet& rAttr)" );
/*N*/ 
/*N*/ 	rAttr.Put(SvxChartStyleItem(ChartStyle()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_TITLE_SHOW_MAIN, ShowMainTitle()));
/*N*/ 	rAttr.Put(SfxStringItem(CHATTR_TITLE_MAIN, MainTitle()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_TITLE_SHOW_SUB, ShowSubTitle()));
/*N*/ 	rAttr.Put(SfxStringItem(CHATTR_TITLE_SUB, SubTitle()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_TITLE_SHOW_X_AXIS, ShowXAxisTitle()));
/*N*/ 	rAttr.Put(SfxStringItem(CHATTR_TITLE_X_AXIS, XAxisTitle()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_TITLE_SHOW_Y_AXIS, ShowYAxisTitle()));
/*N*/ 	rAttr.Put(SfxStringItem(CHATTR_TITLE_Y_AXIS, YAxisTitle()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_TITLE_SHOW_Z_AXIS, ShowZAxisTitle()));
/*N*/ 	rAttr.Put(SfxStringItem(CHATTR_TITLE_Z_AXIS, ZAxisTitle()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_X_AXIS, ShowXAxis()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_X_MAIN, ShowXGridMain()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_X_HELP, ShowXGridHelp()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_X_DESCR, ShowXDescr()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Y_AXIS, ShowYAxis()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Y_MAIN, ShowYGridMain()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Y_HELP, ShowYGridHelp()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Y_DESCR, ShowYDescr()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Z_AXIS, ShowZAxis()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Z_MAIN, ShowZGridMain()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Z_HELP, ShowZGridHelp()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_Z_DESCR, ShowZDescr()));
/*N*/ 
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_2Y_AXIS, GetAxisByUID(CHART_AXIS_SECONDARY_Y)->IsVisible()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_2Y_DESCR, GetAxisByUID(CHART_AXIS_SECONDARY_Y)->HasDescription()));
/*N*/ 
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_2X_AXIS, GetAxisByUID(CHART_AXIS_SECONDARY_X)->IsVisible()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_AXISGRID_SHOW_2X_DESCR, GetAxisByUID(CHART_AXIS_SECONDARY_X)->HasDescription()));
/*N*/ 
/*N*/ 	const SfxPoolItem *pPoolItem = NULL;
/*N*/ 	if (pLegendAttr->GetItemState(SCHATTR_LEGEND_POS, TRUE, &pPoolItem)
/*N*/ 							== SFX_ITEM_SET)
/*N*/ 		rAttr.Put(SvxChartLegendPosItem
/*N*/ 					(((const SvxChartLegendPosItem*)pPoolItem)->GetValue(),
/*N*/ 					pItemPool->GetSlotId(SCHATTR_LEGEND_POS)));
/*N*/ 
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_DATA_SWITCH, IsSwitchData()));
/*N*/ 
/*N*/ 
/*N*/ 	rAttr.Put (SfxBoolItem (SCHATTR_STAT_AVERAGE,bShowAverage));
/*N*/ 	rAttr.Put (SfxInt32Item (SCHATTR_STAT_KIND_ERROR, (INT32) eErrorKind));
/*N*/ 	rAttr.Put (SfxInt32Item (SCHATTR_STAT_INDICATE, (INT32) eIndicate));
/*N*/ 	rAttr.Put (SvxDoubleItem (fIndicatePercent, SCHATTR_STAT_PERCENT));
/*N*/ 	rAttr.Put (SvxDoubleItem (fIndicateBigError, SCHATTR_STAT_BIGERROR));
/*N*/ 	rAttr.Put (SvxDoubleItem (fIndicatePlus, SCHATTR_STAT_CONSTPLUS));
/*N*/ 	rAttr.Put (SvxDoubleItem (fIndicateMinus, SCHATTR_STAT_CONSTMINUS));
/*N*/ 	rAttr.Put (SfxInt32Item (SCHATTR_STAT_REGRESSTYPE, (INT32) eRegression));
/*N*/ 
/*N*/ 	//Datenbeschriftung
/*N*/ 	rAttr.Put(SfxBoolItem(SCHATTR_DATADESCR_SHOW_SYM,bShowSym));
/*N*/ 	rAttr.Put(SvxChartDataDescrItem(eDataDescr));
/*N*/ 
/*N*/ 	rAttr.Put(SfxInt32Item(CHATTR_BARWIDTH,GetBarPercentWidth()));
/*N*/ 	rAttr.Put(SfxBoolItem(CHATTR_BARCONNECT, (BOOL)(m_nDefaultColorSet&CHSPECIAL_TRACELINES)));
/*N*/ 
/*N*/ 	if (pChartAttr->GetItemState(SCHATTR_STYLE_SHAPE, TRUE, &pPoolItem)== SFX_ITEM_SET)
/*N*/ 	{
/*?*/ 		long nTmp= (long)((const SfxInt32Item*)pPoolItem)->GetValue();
/*?*/ 		rAttr.Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nTmp));
/*N*/ 	}
/*N*/ 
/*N*/ 	ChartScene *pScene=GetScene();
/*N*/ 	if(pScene)
/*N*/ //-/		pScene->TakeAttributes(rAttr,TRUE,FALSE);
/*N*/ 		rAttr.Put(pScene->GetItemSet());
/*N*/ 
/*N*/     if( pChartAttr->GetItemState( SCHATTR_USER_DEFINED_ATTR, TRUE, &pPoolItem ) == SFX_ITEM_SET )
/*N*/     {
/*?*/         rAttr.Put( *pPoolItem );
/*N*/     }
/*N*/ }

/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

/*N*/ void ChartModel::PutAttr(const SfxItemSet& rAttr)
/*N*/ {
/*N*/ 	const SfxPoolItem *pPoolItem = NULL;
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_SHOW_MAIN, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowMainTitle () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_MAIN, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		MainTitle () = ((const SfxStringItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_SHOW_SUB, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowSubTitle () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_SUB, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		SubTitle () = ((const SfxStringItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_SHOW_X_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowXAxisTitle () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_X_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		XAxisTitle () = ((const SfxStringItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_SHOW_Y_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowYAxisTitle () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_Y_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		YAxisTitle () = ((const SfxStringItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_SHOW_Z_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowZAxisTitle () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_TITLE_Z_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		ZAxisTitle () = ((const SfxStringItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_X_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowXAxis (((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_X_MAIN, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowXGridMain () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_X_HELP, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		ShowXGridHelp () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_X_DESCR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowXDescr (((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Y_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowYAxis (((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Y_MAIN, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowYGridMain () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Y_HELP, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		ShowYGridHelp () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Y_DESCR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowYDescr (((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Z_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowZAxis (((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Z_MAIN, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		ShowZGridMain () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Z_HELP, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		ShowZGridHelp () = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_Z_DESCR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		ShowZDescr (((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_2Y_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		BOOL b = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 		pChartBAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_SHOWAXIS,b));
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_2Y_DESCR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		BOOL b = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 		pChartBAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,b));
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_2X_AXIS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		BOOL b = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 		pChartAAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_SHOWAXIS,b));
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_AXISGRID_SHOW_2X_DESCR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		BOOL b = ((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 		pChartAAxis->GetItemSet()->Put(SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,b));
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_LEGEND_POS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		pLegendAttr->Put(SvxChartLegendPosItem
/*N*/ 			(((const SvxChartLegendPosItem*)pPoolItem)->GetValue(),
/*?*/ 			 pItemPool->GetWhich(CHATTR_LEGEND_POS)));
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_DATA_SWITCH, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		ChangeSwitchData(((const SfxBoolItem*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ //Statistik
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_CONSTPLUS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		fIndicatePlus=((const SvxDoubleItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_BIGERROR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		fIndicateBigError=((const SvxDoubleItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_PERCENT, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		fIndicatePercent=((const SvxDoubleItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_INDICATE, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*?*/ 		eIndicate=(SvxChartIndicate)((const SfxInt32Item*)pPoolItem)->GetValue();
/*N*/ 
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_KIND_ERROR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		eErrorKind=(SvxChartKindError)((const SfxInt32Item*)pPoolItem)->GetValue();
/*N*/ 
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_AVERAGE, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		bShowAverage=((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_CONSTMINUS, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		fIndicateMinus=((const SvxDoubleItem*)pPoolItem)->GetValue();
/*N*/    if (rAttr.GetItemState(SCHATTR_STAT_REGRESSTYPE, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		eRegression=(SvxChartRegress)((const SfxInt32Item*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 
/*N*/ 	   //Datenbeschriftung
/*N*/ 	   SvxChartDataDescr eNewDataDescr,eOldDataDescr;
/*N*/ 	   BOOL bNewShowSym, bOldShowSym;
/*N*/ 
/*N*/ 	   eNewDataDescr = eOldDataDescr = eDataDescr;
/*N*/ 	   bNewShowSym = bOldShowSym = bShowSym;
/*N*/ 
/*N*/ 	  if (rAttr.GetItemState(SCHATTR_DATADESCR_DESCR, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		eNewDataDescr=(SvxChartDataDescr)((const SvxChartDataDescrItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	  if (rAttr.GetItemState(SCHATTR_DATADESCR_SHOW_SYM, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/ 		bNewShowSym=((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 
/*N*/ 	  if( bOldShowSym != bNewShowSym || eOldDataDescr != eNewDataDescr )
/*N*/ 	  {
/*?*/ 		  ChangeDataDescr(eNewDataDescr,bNewShowSym);
/*N*/ 	  }
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_BARWIDTH, TRUE, &pPoolItem)
/*N*/ 				== SFX_ITEM_SET)
/*?*/ 		SetBarPercentWidth((long)((const SfxInt32Item*)pPoolItem)->GetValue());
/*N*/ 
/*N*/ 
/*N*/    if (rAttr.GetItemState(CHATTR_BARCONNECT, TRUE, &pPoolItem)
/*N*/ 					== SFX_ITEM_SET)
/*N*/    {
/*N*/ 	   BOOL bTmp=((const SfxBoolItem*)pPoolItem)->GetValue();
/*N*/ 	   m_nDefaultColorSet=(bTmp)
/*N*/ 		   ?(m_nDefaultColorSet|CHSPECIAL_TRACELINES)
/*N*/ 		   :(m_nDefaultColorSet|CHSPECIAL_TRACELINES)-CHSPECIAL_TRACELINES;
/*N*/    }
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(SCHATTR_STYLE_SHAPE, TRUE, &pPoolItem)== SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		long nTmp= (long)((const SfxInt32Item*)pPoolItem)->GetValue();
/*N*/ 		SfxItemSet aS(*pItemPool,SCHATTR_STYLE_SHAPE,SCHATTR_STYLE_SHAPE,0);
/*N*/ 		aS.Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nTmp));
/*N*/ 		PutDataRowAttrAll(aS);
/*N*/ 	}
/*N*/ 
/*N*/ 	// Autopilot mit CHATTR_
/*N*/ 	if (rAttr.GetItemState(CHATTR_STYLE_SHAPE, TRUE, &pPoolItem)== SFX_ITEM_SET)
/*N*/ 	{
/*?*/ 		long nTmp= (long)((const SfxInt32Item*)pPoolItem)->GetValue();
/*?*/ 		SfxItemSet aS(*pItemPool,SCHATTR_STYLE_SHAPE,SCHATTR_STYLE_SHAPE,0);
/*?*/ 		aS.Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nTmp));
/*?*/ 		PutDataRowAttrAll(aS);
/*N*/ 	}
/*N*/ 	if( rAttr.GetItemState(CHATTR_AXIS_AUTO_ORIGIN, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 	{
/*?*/ 		BOOL bTmp= (BOOL)((const SfxBoolItem*)pPoolItem)->GetValue();
/*?*/ 		SfxItemSet aS( *pItemPool, SCHATTR_AXIS_AUTO_ORIGIN, SCHATTR_AXIS_AUTO_ORIGIN);
/*?*/ 		aS.Put( SfxBoolItem( SCHATTR_AXIS_AUTO_ORIGIN, bTmp ));
/*?*/ 		if( pChartYAxis )
/*?*/ 		{
/*?*/ 			pChartYAxis->SetAttributes( aS );
/*?*/ 		}
/*?*/ 		if( pChartBAxis )
/*?*/ 		{
/*?*/ 			pChartBAxis->SetAttributes( aS );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ChartScene *pScene=GetScene();
/*N*/ 	if(pScene) //ToDo: Ist das hier nötig??? warum nicht direkt rAttr? #63904#
/*N*/ 	{
/*N*/ 		SfxItemSet aSceneSet(*pItemPool,nRowWhichPairs);
/*N*/ 		aSceneSet.Put(rAttr);
/*N*/ 
/*N*/ //-/		pScene->NbcSetAttributes(aSceneSet,FALSE);
/*N*/ 		pScene->SetItemSet(aSceneSet);
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/     if( rAttr.GetItemState( SCHATTR_USER_DEFINED_ATTR, TRUE, &pPoolItem ) == SFX_ITEM_SET )
/*N*/     {
/*?*/         pChartAttr->Put( *pPoolItem );
/*N*/     }
/*N*/ 
/*N*/ 
/*N*/ 	if (rAttr.GetItemState(CHATTR_DIAGRAM_STYLE, TRUE, &pPoolItem)
/*N*/ 				== SFX_ITEM_SET)
/*?*/ 		ChangeChart( ((const SvxChartStyleItem*)pPoolItem)->GetValue());
/*N*/ }

/*N*/ long ChartModel::GetAxisUID(long nRow)
/*N*/ {
/*N*/ 	return ((const SfxInt32Item &)GetDataRowAttr(nRow).Get(SCHATTR_AXIS)).GetValue();
/*N*/ };




/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/


/*************************************************************************
|*
|* gesamte Grafik initialisieren
|*
\************************************************************************/

/*N*/ void ChartModel::Initialize()
/*N*/ {
/*N*/ 	mbIsInitialized = TRUE;
/*N*/ 
/*N*/ 	if (!(aInitialSize.Height () && aInitialSize.Width ()))
/*N*/ 	{
/*?*/ 		SdrPage* pPage = GetPage(0);
/*?*/ 		if (pPage) aInitialSize = pPage->GetSize();
/*?*/ 
/*?*/ 		if (!(aInitialSize.Height () && aInitialSize.Width ())) aInitialSize = Size (7000, 8000);
/*?*/ //        13000, 8666
/*N*/ 	}
/*N*/ 
/*N*/ 	ScaleText( 0, aInitialSize );
/*N*/ 
/*N*/ 	if( bShouldBuildChart )
/*N*/ 	{
/*N*/ 		//	Force build chart.  If it has been loaded from an XML file, than that is
/*N*/ 		//	done by unlocking it.
/*N*/ 		bNoBuildChart = FALSE;
/*N*/ 
/*N*/ 		if( pDocShell &&
/*N*/ 			pDocShell->IsEnableSetModified())
/*N*/ 		{
/*?*/ 			pDocShell->EnableSetModified( FALSE );
/*?*/ 			if (IsLockedBuild())
/*?*/ 				UnlockBuild();
/*?*/ 			else
/*?*/ 				BuildChart( FALSE );
/*?*/ 			pDocShell->EnableSetModified( TRUE );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			if (IsLockedBuild())
/*?*/ 				UnlockBuild();
/*N*/ 			else
/*N*/ 				BuildChart( FALSE );
/*N*/ 	}
/*N*/ 
/*N*/ 	SetDiagramRectangle( GetChartRect() );
/*N*/ }


/*************************************************************************
|*
|* Sub-Methode von BuildChart()
|*
\************************************************************************/
/*N*/ void ChartModel::ScaleText( long nTitle, const Size& rPageSize )
/*N*/ {
/*N*/ 	// #67459#
/*N*/ 	// if the page size is unchanged ...
/*N*/ 	if( aInitialSize == rPageSize )
/*N*/ 	{
/*N*/ 		// ... and the diagram (possibly) has been resized
/*N*/ 		if( GetDiagramHasBeenMovedOrResized() && (aLastDiagramRectangle != aDiagramRectangle) )
/*N*/ 		{
/*?*/ 			if( nTitle != CHOBJID_DIAGRAM_X_AXIS )
/*?*/ 				ResizeText( &GetAttr(CHOBJID_DIAGRAM_X_AXIS), rPageSize, FALSE );
/*?*/ 			if( nTitle != CHOBJID_DIAGRAM_Y_AXIS )
/*?*/ 				ResizeText( &GetAttr(CHOBJID_DIAGRAM_Y_AXIS), rPageSize, FALSE );
/*?*/ 			if( nTitle != CHOBJID_DIAGRAM_A_AXIS )
/*?*/ 				ResizeText( &GetAttr(CHOBJID_DIAGRAM_A_AXIS), rPageSize, FALSE );
/*?*/ 			if( nTitle != CHOBJID_DIAGRAM_B_AXIS )
/*?*/ 				ResizeText( &GetAttr(CHOBJID_DIAGRAM_B_AXIS), rPageSize, FALSE );
/*?*/ 			if( nTitle != CHOBJID_DIAGRAM_Z_AXIS )
/*?*/ 				ResizeText( &GetAttr(CHOBJID_DIAGRAM_Z_AXIS), rPageSize, FALSE );
/*?*/ 
/*?*/ 			for( ULONG i=0; i<aDataRowAttrList.Count(); i++ )
/*?*/ 				ResizeText( aDataRowAttrList.GetObject(i), rPageSize, FALSE );
/*?*/ 			ResizeText(pDummyAttr,rPageSize);	// also resize crash test dummy to normal for next test ?
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else	// page size has changed
/*N*/ 	{
/*N*/ 		if(nTitle != CHOBJID_TITLE_MAIN)
/*N*/ 			ResizeText (pMainTitleAttr ,rPageSize);
/*N*/ 		if(nTitle != CHOBJID_TITLE_SUB)
/*N*/ 			ResizeText (pSubTitleAttr  ,rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_TITLE_X_AXIS)
/*N*/ 			ResizeText (pXAxisTitleAttr,rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_TITLE_Y_AXIS)
/*N*/ 			ResizeText (pYAxisTitleAttr,rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_TITLE_Z_AXIS)
/*N*/ 			ResizeText (pZAxisTitleAttr,rPageSize);
/*N*/ 
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_X_AXIS)
/*N*/ 			ResizeText(&GetAttr(CHOBJID_DIAGRAM_X_AXIS),rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_Y_AXIS)
/*N*/ 			ResizeText(&GetAttr(CHOBJID_DIAGRAM_Y_AXIS),rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_A_AXIS)
/*N*/ 			ResizeText(&GetAttr(CHOBJID_DIAGRAM_A_AXIS),rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_B_AXIS)
/*N*/ 			ResizeText(&GetAttr(CHOBJID_DIAGRAM_B_AXIS),rPageSize);
/*N*/ 		if(nTitle != CHOBJID_DIAGRAM_Z_AXIS)
/*N*/ 			ResizeText(&GetAttr(CHOBJID_DIAGRAM_Z_AXIS),rPageSize);
/*N*/ 		if(nTitle != CHOBJID_LEGEND)
/*N*/ 			ResizeText(pLegendAttr, rPageSize);
/*N*/ 		for( ULONG i=0; i<aDataRowAttrList.Count(); i++ )
/*N*/ 			ResizeText(aDataRowAttrList.GetObject(i),rPageSize);
/*N*/ 		ResizeText(pDummyAttr,rPageSize);	// also resize crash test dummy to normal for next test ?
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Sub-Methode von BuildChart(), Anzeige des Haupttitels
|*
\************************************************************************/
/*N*/ void ChartModel::DoShowMainTitle(USHORT& rIndex,const long nYOfs)
/*N*/ {
/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/ 	const Size& rPageSize=pPage->GetSize();
/*N*/ 
/*N*/ 	Point aTitlePosition;
/*N*/ 	if (bUseRelativePositionsForChartGroups && (aTitleTopCenter.X() >= 0) &&
/*N*/ 	   (aTitleTopCenter.Y() >= 0) && GetMainTitleHasBeenMoved())
/*N*/ 	{
/*N*/ 		double fRelativXPosition = ((double) aTitleTopCenter.X()) / aInitialSize.Width();
/*N*/ 		double fRelativYPosition = ((double) aTitleTopCenter.Y()) / aInitialSize.Height();
/*N*/ 		aTitlePosition.X() = (long)((double)rPageSize.Width()  * fRelativXPosition );
/*N*/ 		aTitlePosition.Y() = (long)((double)rPageSize.Height() * fRelativYPosition );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aTitlePosition.X() = aChartRect.Left() + aChartRect.GetWidth() / 2;
/*N*/ 		aTitlePosition.Y() = aChartRect.Top();
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrTextObj *pObj = CreateTextObj(CHOBJID_TITLE_MAIN, aTitlePosition,
/*N*/ 									 aMainTitle,
/*N*/ 									 *pMainTitleAttr, TRUE, CHADJUST_TOP_CENTER);
/*N*/ 
/*N*/ 	aChartRect.Top() += GetOutputSize(*pObj).Height() + nYOfs;
/*N*/ 	pObj->SetResizeProtect(TRUE);
/*N*/ 	pPage->NbcInsertObject(pObj, rIndex++);
/*N*/ }

/*************************************************************************
|*
|* Sub-Methode von BuildChart(), man koennte vermutlich DoShowMainTitle und
|* DoShowSubTitle zu DoShowTitle zusammenfassen
|*
\************************************************************************/
/*N*/ void ChartModel::DoShowSubTitle(USHORT& rIndex,const long nYOfs)
/*N*/ {
/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/ 	const Size& rPageSize=pPage->GetSize();
/*N*/ 
/*N*/ 	Point aSubTitlePosition;
/*N*/ 	if (bUseRelativePositionsForChartGroups && (aSubTitleTopCenter.X() >= 0) &&
/*N*/ 	  (aSubTitleTopCenter.Y() >= 0) && GetSubTitleHasBeenMoved())
/*N*/ 	{
/*N*/ 	   double fRelativeXPosition = ((double) aSubTitleTopCenter.X()) / aInitialSize.Width();
/*N*/ 	   double fRelativeYPosition = ((double) aSubTitleTopCenter.Y()) / aInitialSize.Height();
/*N*/ 	   aSubTitlePosition.X() = (long)((double)rPageSize.Width() *  fRelativeXPosition );
/*N*/ 	   aSubTitlePosition.Y() = (long)((double)rPageSize.Height() * fRelativeYPosition );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 	   aSubTitlePosition.X() = aChartRect.Left() + aChartRect.GetWidth() / 2;
/*N*/ 	   aSubTitlePosition.Y() = aChartRect.Top();
/*N*/ 	}
/*N*/ 	SdrTextObj *pObj = CreateTextObj(CHOBJID_TITLE_SUB, aSubTitlePosition,
/*N*/ 									aSubTitle,
/*N*/ 									*pSubTitleAttr, TRUE, CHADJUST_TOP_CENTER);
/*N*/ 
/*N*/ 	aChartRect.Top() += GetOutputSize(*pObj).Height() + nYOfs;
/*N*/ 	pObj->SetResizeProtect(TRUE);
/*N*/ 	pPage->NbcInsertObject(pObj, rIndex++);
/*N*/ }

/*************************************************************************
|*
|* Sub-Methode von BuildChart(), Anzeige der Legende
|*
\************************************************************************/
/*N*/ void ChartModel::DoShowLegend(const Rectangle& rWholeRect,
/*N*/ 							  const long nXOfs,const long nYOfs,USHORT& rIndex)
/*N*/ {
/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/ 	const Size& rPageSize=pPage->GetSize();
/*N*/ 
/*N*/ 	SvxChartLegendPos eLegendPos = ((const SvxChartLegendPosItem&) pLegendAttr->Get(SCHATTR_LEGEND_POS)).
/*N*/ 							   GetValue();
/*N*/ 
/*N*/ 	if (eLegendPos != CHLEGEND_NONE)
/*N*/ 	{
/*N*/ 		SdrObject   *pObj = CreateLegend(rWholeRect);
/*N*/ 
/*N*/ 		if (pObj)
/*N*/ 		{
/*N*/ 			Rectangle   aRect = pObj->GetLogicRect();
/*N*/ 			ChartAdjust eAdjust;
/*N*/ 
/*N*/ 			Point aLegendPosition;
/*N*/ 			if (bUseRelativePositionsForChartGroups && (aLegendTopLeft.X() >= 0) &&
/*N*/ 			   (aLegendTopLeft.Y() >= 0) && GetLegendHasBeenMoved())
/*N*/ 			{
/*N*/                 double fRatioX =
/*N*/                     static_cast< double >( rPageSize.Width() ) /
/*N*/                     static_cast< double >( aInitialSize.Width() );
/*N*/                 double fRatioY =
/*N*/                     static_cast< double >( rPageSize.Height() ) /
/*N*/                     static_cast< double >( aInitialSize.Height() );
/*N*/ 
/*N*/ 				aLegendPosition.X() = static_cast< long >(
/*N*/                     static_cast< double >( aLegendTopLeft.X() ) * fRatioX );
/*N*/ 				aLegendPosition.Y() = static_cast< long >(
/*N*/                     static_cast< double >( aLegendTopLeft.Y() ) * fRatioY );
/*N*/ 
/*N*/ 				if( bAdjustMarginsForLegend )
/*N*/ 				{
/*N*/ 					switch (eLegendPos)
/*N*/ 					{
/*N*/ 						case CHLEGEND_LEFT:
/*?*/ 							eAdjust = CHADJUST_TOP_LEFT;
/*?*/ 							aChartRect.Left() += aRect.GetWidth() + nXOfs;
/*?*/ 							break;
/*?*/ 
/*?*/ 						case CHLEGEND_TOP:
/*?*/ 							eAdjust = CHADJUST_TOP_LEFT;
/*?*/ 							aChartRect.Top() += aRect.GetHeight() + nYOfs;
/*?*/ 							break;
/*N*/ 
/*N*/ 						case CHLEGEND_RIGHT:
/*N*/ 							eAdjust = CHADJUST_TOP_LEFT;
/*N*/ 							aChartRect.Right() -= aRect.GetWidth() + nXOfs;
/*N*/ 							break;
/*N*/ 
/*?*/ 						case CHLEGEND_BOTTOM:
/*?*/ 							eAdjust = CHADJUST_TOP_LEFT;
/*?*/ 							aChartRect.Bottom() -= aRect.GetHeight() + nYOfs;
/*?*/ 							break;
/*N*/ 					}
/*N*/ 				}
/*N*/                 // #96418# why shifting the legend nXOfs/nYOfs away from the
/*N*/                 // edge, if it moved manually; it may well reach the edge in
/*N*/                 // that case.  So just check if it laps out of the edge.  Apart
/*N*/                 // from that the default position did not satisfy this condition
/*N*/                 // and was therefore always shifted inside
/*N*/ 				if (aLegendPosition.X() + aRect.GetWidth() > rPageSize.Width())
/*N*/ 				{
/*?*/ 					aLegendPosition.X() =  rPageSize.Width() - aRect.GetWidth();
/*N*/ 				}
/*N*/ 				if (aLegendPosition.Y() + aRect.GetHeight() > rPageSize.Height())
/*N*/ 				{
/*?*/ 					aLegendPosition.Y() =  rPageSize.Height() - aRect.GetHeight();
/*N*/ 				}
/*N*/             }
/*N*/ 			else
/*N*/ 			{
/*N*/ 				switch (eLegendPos)
/*N*/ 				{
/*N*/ 					case CHLEGEND_LEFT:
/*?*/ 						aLegendPosition.X() = rWholeRect.Left();
/*?*/ 						aLegendPosition.Y() = rWholeRect.Top() + rWholeRect.GetHeight() / 2;
/*?*/ 						eAdjust = CHADJUST_CENTER_LEFT;
/*?*/ 						aChartRect.Left() += aRect.GetWidth() + nXOfs;
/*?*/ 						break;
/*?*/ 
/*?*/ 					case CHLEGEND_TOP:
/*?*/ 						aLegendPosition.X() = rWholeRect.Left() + rWholeRect.GetWidth() / 2;
/*?*/ 						aLegendPosition.Y() = aChartRect.Top();
/*?*/ 						eAdjust = CHADJUST_TOP_CENTER;
/*?*/ 						aChartRect.Top() += aRect.GetHeight() + nYOfs;
/*?*/ 						break;
/*N*/ 
/*N*/ 					case CHLEGEND_RIGHT:
/*N*/ 						aLegendPosition.X() = rWholeRect.Right();
/*N*/ 						aLegendPosition.Y() = rWholeRect.Top() + rWholeRect.GetHeight() / 2;
/*N*/ 						eAdjust = CHADJUST_CENTER_RIGHT;
/*N*/ 						aChartRect.Right() -= aRect.GetWidth() + nXOfs;
/*N*/ 						break;
/*N*/ 
/*N*/ 					case CHLEGEND_BOTTOM:
/*?*/ 						aLegendPosition.X() = rWholeRect.Left() + rWholeRect.GetWidth() / 2;
/*?*/ 						aLegendPosition.Y() = rWholeRect.Bottom();
/*?*/ 						eAdjust = CHADJUST_BOTTOM_CENTER;
/*?*/ 						aChartRect.Bottom() -= aRect.GetHeight() + nYOfs;
/*?*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 	
/*N*/ 			//	For 3D pie charts that whose position and size have not been altered
/*N*/ 			//	manually, the calculated size in aChartRect is 
/*N*/ 			//	set to aDiagramRectangle after a possible adaption to half
/*N*/ 			//	it's height.  This adaption mimics the behaviour from BuildChart
/*N*/ 			//	and takes place only if the diagram is not more than twice as wide
/*N*/ 			//	as it is high.
/*N*/ 			if (IsPieChart() && IsReal3D() && ! GetDiagramHasBeenMovedOrResized())
/*N*/ 			{
/*N*/ 				long nWidth=aChartRect.GetWidth();
/*N*/ 				long nHeight=aChartRect.GetHeight();
/*N*/ 				if( ((double)nHeight/(double)nWidth) > 0.5 )
/*N*/ 				{
/*N*/ 					aChartRect.Top()+=nHeight/4;
/*N*/ 					aChartRect.Bottom()-=nHeight/4;
/*N*/ 				}
/*N*/ 				SetDiagramRectangle (aChartRect);
/*N*/ 			}
/*N*/ 
/*N*/ 			aRect.SetPos(aLegendPosition);
/*N*/ 			AdjustRect(aRect, eAdjust);
/*N*/ 			pObj->NbcSetLogicRect(aRect);
/*N*/ 			pObj->SetResizeProtect(TRUE);
/*N*/ 			if (pObj->ISA(SchObjGroup))
/*N*/ 			{
/*N*/ 				((SchObjGroup *)pObj)->SetGroupType(SchObjGroup::LEGEND);
/*N*/ 				((SchObjGroup *)pObj)->SetModel(this);
/*N*/ 			}
/*N*/ 			pPage->InsertObject(pObj, rIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//Übertragung der Page-Objekt-Attribute in die Modeleigenen AttrSets
/*N*/ void ChartModel::Rescue3DObjAttr(SdrObjList* pList)//#52277#
/*N*/ {
/*N*/ 	CHART_TRACE( "ChartModel::Rescue3DObjAttr" );
/*N*/ 	if(pList)
/*N*/ 	{
/*N*/ 		SdrObject* pObj;
/*N*/ 		long nRow, nCol;
/*N*/ //-/		SfxItemSet aSet( *pItemPool, SID_ATTR_3D_START, SID_ATTR_3D_END );
/*N*/ 		SfxItemSet aSet( *pItemPool, SDRATTR_3D_FIRST, SDRATTR_3D_LAST);
/*N*/ 		aSet.ClearItem();
/*N*/ 
/*N*/ 		SdrObjListIter aIterator( *pList, IM_DEEPWITHGROUPS );
/*N*/ 
/*N*/ 		while( aIterator.IsMore() )
/*N*/ 		{
/*N*/ 			pObj = aIterator.Next();
/*N*/ 			SchDataRow*     pDataRow   = GetDataRow( *pObj );
/*N*/ 			SchDataPoint*   pDataPoint = GetDataPoint( *pObj );
/*N*/ 			if( pDataRow )
/*N*/ 			{
/*N*/ 				nRow = pDataRow->GetRow();
/*N*/ //-/				pObj->TakeAttributes( aSet, FALSE, TRUE );
/*N*/ 				aSet.Put(pObj->GetItemSet());
/*N*/ 
/*N*/ 				aSet.ClearInvalidItems();
/*N*/ 				aDataRowAttrList.GetObject( nRow )->Put( aSet );
/*N*/ 				aSet.ClearItem();
/*N*/ 			}
/*N*/ 			else if( pDataPoint )
/*N*/ 			{
/*N*/ 				nCol = pDataPoint->GetCol();
/*N*/ 				nRow = pDataPoint->GetRow();
/*N*/ //-/				pObj->TakeAttributes( aSet, FALSE, TRUE );
/*N*/ 				aSet.Put(pObj->GetItemSet());
/*N*/ 
/*N*/ 				aSet.ClearInvalidItems();
/*N*/ 				PutDataPointAttr( nCol, nRow, aSet );
/*N*/ 				aSet.ClearItem();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SchObjectId *pId = GetObjectId( *pObj );
/*N*/ 				if( pId )
/*N*/ 				{
/*N*/ 					long nId = pId->GetObjId();
/*N*/ //-/					pObj->TakeAttributes( aSet, FALSE, TRUE );
/*N*/ 					aSet.Put(pObj->GetItemSet());
/*N*/ 					if( aSet.Count() )
/*N*/ 					{
/*N*/ 						aSet.ClearInvalidItems();
/*N*/ 						GetAttr( nId ).Put( aSet, TRUE );
/*N*/ 						aSet.ClearItem();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* Sub-Methode von BuildChart(), Loeschen aller Chartobjecte
|*
\************************************************************************/
/*N*/ void ChartModel::DeleteChartObjects()
/*N*/ {
/*N*/ 	SdrPage* pPage=GetPage(0);
/*N*/     BOOL bResize = (aInitialSize != pPage->GetSize());
/*N*/ 
/*N*/ 	// FG: Bevor die Objekte geloescht und neu aufgebaut werden, merkt man sich deren
/*N*/ 	//     Position. Da in InitalSize die urspruengliche Seitengroesse steht, kann
/*N*/ 	//     man nun die relative Position auf der Seite bestimmen.
/*N*/ 	//     Diese relative Position bleibt bei einem Resize erhalten,
/*N*/ 	//     falls der  Schalter "bUseRelativePositionsForChartGroups" auf TRUE steht.
/*N*/ 
/*N*/ 	SdrObject* pGroupObject = GetObjWithId( CHOBJID_DIAGRAM_AREA, *pPage );
/*N*/ 	if( pGroupObject != NULL )
/*N*/ 		DeleteObject( pGroupObject );
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId( CHOBJID_TITLE_MAIN, *pPage );
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		aTitleTopCenter = pGroupObject->GetBoundRect().TopCenter();
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId(CHOBJID_TITLE_SUB, *pPage);
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		aSubTitleTopCenter = pGroupObject->GetBoundRect().TopCenter();
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId(CHOBJID_DIAGRAM, *pPage);
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		// FG: 11.3.97 Wenn die Grenzen des Objekts woanders gesetzt worden sind, duerfen sie hier
/*N*/ 		//             nicht mehr gesetzt werden.
/*N*/ 		if (pGroupObject->ISA(SchObjGroup))
/*N*/ 		{
/*N*/ 			if (GetDiagramHasBeenMovedOrResized() &&
/*N*/ 			   ((SchObjGroup *) pGroupObject)->GetAskForLogicRect())
/*N*/ 			{
/*N*/ 				// FG: So leider nicht das Chart-Rect ist fehlerhafterweise nicht das Bounding-Rectangle
/*N*/ 				// aDiagramRectangle = pGroupObject->GetLogicRect();
/*N*/ 				SetDiagramRectangle( GetChartRect() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if (pGroupObject->ISA(ChartScene))
/*N*/ 		{
/*N*/ 			if (GetDiagramHasBeenMovedOrResized() &&
/*N*/ 			   ((ChartScene *) pGroupObject)->GetAskForLogicRect())
/*N*/ 			{
/*N*/ 				// FG: So leider nicht das Chart-Rect ist fehlerhafterweise nicht das Bounding-Rectangle
/*N*/ 				// aDiagramRectangle = pGroupObject->GetLogicRect();
/*N*/ 				SetDiagramRectangle( GetChartRect() );
/*N*/ 			}
/*N*/ 
/*N*/ 
/*N*/ 			if(	pGroupObject == pScene )
/*N*/ 				pScene = NULL;
/*N*/ 		}
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId(CHOBJID_LEGEND, *pPage);
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		// FG: 11.3.97 Wenn die Grenzen des Objekts woanders gesetzt worden sind, duerfen sie hier
/*N*/ 		//             nicht mehr gesetzt werden.
/*N*/ 		if (GetLegendHasBeenMoved()&&
/*N*/             ((SchObjGroup *) pGroupObject)->GetAskForLogicRect() )
/*N*/             // #107305# I don't see where aLegendTopLeft should have changed at
/*N*/             // another place, if not via resizing.  Besides, why should it for
/*N*/             // the legend and not the titles?
/*N*/ //             && ! bResize )
/*N*/ 		{
/*N*/   			aLegendTopLeft = pGroupObject->GetLogicRect().TopLeft();
/*N*/ 		}
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId(CHOBJID_DIAGRAM_TITLE_X_AXIS, *pPage);
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		aTitleXAxisPosition = SetPointOfRectangle(pGroupObject->GetBoundRect(), eAdjustXAxesTitle);
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId(CHOBJID_DIAGRAM_TITLE_Y_AXIS, *pPage);
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		Rectangle Test1 = pGroupObject->GetBoundRect();
/*N*/ 		Rectangle Test2 = pGroupObject->GetSnapRect();
/*N*/ 		aTitleYAxisPosition = SetPointOfRectangle(pGroupObject->GetBoundRect(), eAdjustYAxesTitle);
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ 
/*N*/ 	pGroupObject = GetObjWithId(CHOBJID_DIAGRAM_TITLE_Z_AXIS, *pPage);
/*N*/ 	if (pGroupObject != NULL)
/*N*/ 	{
/*N*/ 		aTitleZAxisPosition = SetPointOfRectangle(pGroupObject->GetBoundRect(), eAdjustXAxesTitle);
/*N*/ 		DeleteObject(pGroupObject);
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* Sub-Methode von BuildChart(), Wertebereichueberpruefung mit MsgBox
|*
\************************************************************************/
/*N*/ BOOL ChartModel::CheckRanges(BOOL bCheckAlways)
/*N*/ {
/*N*/ 	//ToDo:CHOBJID_DIAGRAM_B_AXIS
/*N*/ 	BOOL bOK=TRUE;
/*N*/ 	if ((eChartStyle != eOldChartStyle) || (bCheckAlways))
/*N*/ 	{
/*N*/ 		if ((pChartYAxis->GetMin() < 0.0) && (pChartYAxis->GetMax() > 0.0) && (!IsSignedChart()))
/*N*/ 		{
/*?*/ 			// FG: Falls ein Pointer auf ein Fenster uebergeben wird ist die Dialogbox nicht-modal
/*?*/ 			//     Nutzt man dies fuer Veraenderungen am Chart aus, stuertzt das Programm ab.
/*?*/ 			// InfoBox aInfoBox(SFX_APP()->GetAppWindow(), String(SchResId(STR_ONLY_ABS_VALUES)));
/*?*/ 			InfoBox aInfoBox(NULL, String(SchResId(STR_ONLY_ABS_VALUES)));
/*?*/ 			aInfoBox.Execute();
/*?*/ 			eOldChartStyle = eChartStyle;
/*?*/ 			bOK=FALSE;
/*N*/ 		}
/*N*/ 		else if ((pChartYAxis->GetMin() < 0.0) && (!IsNegativeChart ()))
/*N*/ 			{
/*N*/ 				// FG: Falls ein Pointer auf ein Fenster uebergeben wird ist die Dialogbox nicht-modal
/*N*/ 				//     Nutzt man dies fuer Veraenderungen am Chart aus, stuertzt das Programm ab.
/*N*/ 				// InfoBox aInfoBox(SFX_APP()->GetAppWindow(), String(SchResId(STR_NO_MIN_VALUES)));
/*?*/ 				InfoBox aInfoBox(NULL, String(SchResId(STR_NO_MIN_VALUES)));
/*?*/ 				aInfoBox.Execute();
/*?*/ 				eOldChartStyle = eChartStyle;
/*?*/ 				bOK=FALSE;
/*N*/ 			}
/*N*/ 	}
    /*
    if (bCheckAlways)
        if ((pChartYAxis->GetMin() <= 0.0)
            && ((const SfxBoolItem&)GetAttr(CHOBJID_DIAGRAM_Y_AXIS).Get(SCHATTR_AXIS_LOGARITHM)).GetValue())
        {
            // FG: Falls ein Pointer auf ein Fenster uebergeben wird ist die Dialogbox nicht-modal
            //     Nutzt man dies fuer Veraenderungen am Chart aus, stuertzt das Programm ab.
            // InfoBox aInfoBox(SFX_APP()->GetAppWindow(), String(SchResId(STR_NO_LOGARITHMIC_MIN_VALUES)));
            InfoBox aInfoBox(NULL, String(SchResId(STR_NO_LOGARITHMIC_MIN_VALUES)));
            aInfoBox.Execute();
            eOldChartStyle = eChartStyle;
            bOK=FALSE;
        }
*/
/*N*/ 	return bOK;
/*N*/ }
/*************************************************************************
|*
|* Sub-Methode von BuildChart(), Chart-ReSizing
|*
\************************************************************************/
/*N*/ void ChartModel::ResizeChart(const Size& rPageSize)
/*N*/ {
/*N*/ 	if (GetUseRelativePositions() && GetDiagramHasBeenMovedOrResized())
/*N*/ 	{
/*N*/ 		if ((rPageSize.Width() == aInitialSize.Width()) && (rPageSize.Height() == aInitialSize.Height()))
/*N*/ 		{  // FG: Es war also ein Resize nur des Diagramms (also nur der Grafik und nicht der Seite)
/*N*/ 			aChartRect= aDiagramRectangle;
/*N*/ 		}
/*N*/ 		else  // FG: 11.3.97 Dann war es ein Resize des gesamten Charts
/*N*/ 		{
            //	The whole chart has been resized.
            //	The old size aDiagramRectangle of the diagram has to fullfill the constraint
            //	the it has to have a positive extent both in the horizontal and the vertical
            //	direction.
            //
            //	Previously (before 24.07.2001) the constraints to fullfill have been tighter:
            //	All four border lines had to lie inside the chart rectangle.
            //	If the new solution, that solves error #88404# proves to work, then remove this
            //	paragraph and the commented code below.
/*N*/ 			if (	(aDiagramRectangle.nLeft < aDiagramRectangle.nRight)
/*N*/ 				&&	(aDiagramRectangle.nTop < aDiagramRectangle.nBottom) )
//              if ((aDiagramRectangle.nLeft >= 0) && (aDiagramRectangle.nTop >= 0) &&
//                  (aDiagramRectangle.nRight >= 0) && (aDiagramRectangle.nBottom >= 0))
/*N*/ 			{
/*?*/ 				double fRelativeXPosition = ((double) aDiagramRectangle.Left()) / aInitialSize.Width();
/*?*/ 				double fRelativeYPosition = ((double) aDiagramRectangle.Top()) / aInitialSize.Height();
//				if ((fRelativeXPosition <= 1.0) && (fRelativeYPosition <= 1.0))
//				{
/*?*/ 					aChartRect.nLeft = (long)((double)rPageSize.Width() *  fRelativeXPosition );
/*?*/ 					aChartRect.nTop =  (long)((double)rPageSize.Height() * fRelativeYPosition );
//				}
/*?*/ 				fRelativeXPosition = ((double) aDiagramRectangle.Right()) / aInitialSize.Width();
/*?*/ 				fRelativeYPosition = ((double) aDiagramRectangle.Bottom()) / aInitialSize.Height();
//				if ((fRelativeXPosition <= 1.0) && (fRelativeYPosition <= 1.0))
//				{
/*?*/ 					aChartRect.nRight =  (long)((double)rPageSize.Width() *  fRelativeXPosition );
/*?*/ 					aChartRect.nBottom = (long)((double)rPageSize.Height() * fRelativeYPosition );
//				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ }
/*************************************************************************
|*
|* Sub-Methode von BuildChart(),
|*
|* FG: Hier wird der Platz berechnet, den man als oberen Rand freilassen
|*     soll,falls keine Titel dargestellt werden.
|*
\************************************************************************/
/*N*/ void ChartModel::CalculateUpperBorder()
/*N*/ {
/*N*/ 	if ((!bShowMainTitle && !bShowSubTitle))
/*N*/ 	{
/*N*/ 		BOOL bSwitchColRow = eChartStyle == CHSTYLE_2D_BAR ||
/*N*/ 							 eChartStyle == CHSTYLE_2D_STACKEDBAR ||
/*N*/ 							 eChartStyle == CHSTYLE_2D_PERCENTBAR ||
/*N*/ 							 eChartStyle == CHSTYLE_3D_BAR ||
/*N*/ 							 eChartStyle == CHSTYLE_3D_FLATBAR ||
/*N*/ 							 eChartStyle == CHSTYLE_3D_STACKEDFLATBAR ||
/*N*/ 							 eChartStyle == CHSTYLE_3D_PERCENTFLATBAR;
/*N*/ 
/*N*/ 		SfxItemSet aTextAttr((const SfxItemSet &)GetAttr(
/*N*/ 					bSwitchColRow ?
/*N*/ 						CHOBJID_DIAGRAM_X_AXIS
/*N*/ 					  : CHOBJID_DIAGRAM_Y_AXIS));
/*N*/ 
/*N*/ 		Size aTextSize (((SvxFontWidthItem &) aTextAttr.Get (EE_CHAR_FONTWIDTH)).GetWidth (),
/*N*/ 						((SvxFontHeightItem &) aTextAttr.Get (EE_CHAR_FONTHEIGHT)).GetHeight ());
/*N*/ 
/*N*/ 		aChartRect.Top () += aTextSize.Height () / 2;
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|* Sub-Methode von BuildChart(), Chart-ReSizing
|*
\************************************************************************/
/*N*/ void ChartModel::CreateRectsAndTitles(long whatTitle)
/*N*/ {
/*N*/ 		// FG: Abhaengig von der Groesse der Page, also der Ausgabe
/*N*/ 		// werden die Raender erst mal auf 2% der jeweiligen Richtungsausdehnung gesetzt
/*N*/ 		// abgespeichert wird das im ChartModel. Sollte ein Text uber den  den linken
/*N*/ 		// Rand zu weit hinausragen, also linker Rand OuterRect(SdrTextObj) < 0,
/*N*/ 		// So wird entweder der Offset neu berechnet oder der Text veraendert.
/*N*/ 		// Im Moment ist es nur moeglich den linken Rand nachzuregeln.
/*N*/ 		SdrPage* pPage=GetPage(0);
/*N*/ 		const Size& rPageSize=pPage->GetSize();
/*N*/ 
/*N*/ 		long nYOfs = (rPageSize.Height() / 100) * 2;
/*N*/ 		long nXOfs = (rPageSize.Width() /100) * 2;
/*N*/ 		USHORT nIndex = 0;
/*N*/ 
/*N*/         // #97159# the chart rect was set one unit too large in either direction
/*N*/         aChartRect.SetPos( Point( 0, 0 ) );
/*N*/         aChartRect.SetSize( rPageSize );
/*N*/ 
/*N*/  		SchRectObj* pRect = new SchRectObj(aChartRect);
/*N*/ 		pRect->SetModel( this );
/*N*/ 		pPage->InsertObject(SetObjectAttr( pRect, CHOBJID_DIAGRAM_AREA,
/*N*/ 											  FALSE, TRUE, pDiagramAreaAttr), nIndex++);
/*N*/ 
/*N*/ 		aChartRect.Left()   += nXOfs;  // FG: Dies ist das Rechteck in das das Chart (also die Balken ...)
/*N*/ 		aChartRect.Top()    += nYOfs;  //     rein muss. Erst ist es maximal gross und dann wird immer
/*N*/ 		aChartRect.Right()  -= nXOfs;  //     mehr abgezwackt.
/*N*/ 		aChartRect.Bottom() -= nYOfs;
/*N*/ 
/*N*/ 		Rectangle aWholeRect (aChartRect);
/*N*/ 
/*N*/ 			// Falls aInitialSize Null ist dann wird es auf die Seitengroesse gesetzt
/*N*/ 		if( !aInitialSize.Width ()) aInitialSize.Width () = rPageSize.Width ();
/*N*/ 		if( !aInitialSize.Height ()) aInitialSize.Height () = rPageSize.Height ();
/*N*/ 
/*N*/ 			// FG: Dieser Offset wird bei der Legende und den Diagrammtiteln gebraucht.
/*N*/ 			// Er wird aus aesthetischen Gruenden verdoppelt.
/*N*/ 			// Also ab jetzt oberer Rand nYOfs + Titel + 2*nYOfs + Unteritel + 2*nYOfs + Diagramm
/*N*/ 			// Fuer die Legende gilt das fuer den X-Abstand.
/*N*/ 		nXOfs *= 2; nYOfs *=2;
/*N*/ 
/*N*/ 	   if (bTextScalable)
/*N*/ 		  ScaleText(whatTitle,rPageSize);
/*N*/ 
/*N*/ 	   if (bShowMainTitle)
/*N*/ 		   DoShowMainTitle(nIndex,nYOfs);
/*N*/ 
/*N*/ 	   if (bShowSubTitle)
/*N*/ 		   DoShowSubTitle(nIndex,nYOfs);
/*N*/ 
/*N*/ 		CalculateUpperBorder();
/*N*/ 		DoShowLegend(aWholeRect,nXOfs,nYOfs,nIndex);
/*N*/ 		ResizeChart(rPageSize);
/*N*/ }
/*************************************************************************
|*
|* gesamte Grafik erzeugen
|*
\************************************************************************/

namespace
{
class lcl_MapModeResetGuard
{
public:
    lcl_MapModeResetGuard( OutputDevice * pOutDev ) :
            m_pOutDev( pOutDev ),
            m_aMapMode( pOutDev ? pOutDev->GetMapMode() : MapMode() )
    {}

    ~lcl_MapModeResetGuard()
    {
        if( m_pOutDev )
            m_pOutDev->SetMapMode( m_aMapMode );
    }

private:
    OutputDevice *   m_pOutDev;
    MapMode          m_aMapMode;
};

}

/*N*/ void ChartModel::BuildChart(BOOL bCheckRanges,long whatTitle)
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR (context, "sch", "af119097", "::ChartModel::BuildChart");
/*N*/ 	
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ByteString aBStr( aMainTitle, RTL_TEXTENCODING_ASCII_US );
/*N*/ 	CHART_TRACE1( "ChartModel::BuildChart (%s)", aBStr.GetBuffer());
/*N*/ #endif
/*N*/ 
/*N*/ 	//kein setzen der p*Attr durch die Objekte
/*N*/ 	SetAttrAutoStorage(FALSE);
/*N*/ 
/*N*/ 	if (bNoBuildChart)
/*N*/ 	{
/*N*/ 		bShouldBuildChart = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		RTL_LOGFILE_CONTEXT_AUTHOR (context2,
/*N*/ 			"sch", "af119097", "::ChartModel::BuildChart not locked");
/*N*/ 		//nicht ganz so schön, aber vorteilhaft. Hiermit wird ein Itemset mit
/*N*/ 		//Attributen gefüllt, die beim Einfügen etc. als default benutzt werden
/*N*/ 		//(SetDefAttrRow). Da an dieser Stelle ein Defaultset erstellt wird, sind
/*N*/ 		//auch Chart-globale Items gleich gesetzt (Shape3D, Fehlerbalken,Datenbeschriftung)
/*N*/ 		//lediglich die Linien oder Flächenfarbe muss generiert werden!
/*N*/ 		//Alternative: nur in ChangeChart defaults erstellen
/*N*/ 		if( pChartData )
/*N*/ 		{
/*N*/ 			// in scatter charts the first series is for x values
/*N*/ 			// so use first 'y-series' to copy attributes to new series
/*N*/ 			if( IsXYChart())
/*N*/ 			{
/*N*/ 				if( GetRowCount() > 1 )
/*N*/ 					pDummyAttr->Put( GetDataRowAttr( 1 ));
/*N*/ 			}
/*N*/ 			else
/*N*/ 				if( GetRowCount() > 0 )
/*N*/ 					pDummyAttr->Put( GetDataRowAttr( 0 ));
/*N*/ 		}
/*N*/ 
/*N*/ 		//Hotfix für OLE-Charts #54884# Die Wurzel des Übels liegt möglicherweise irgendwo anders!
/*N*/ 		if(!pRefOutDev)
/*N*/ 		{
/*?*/ 			if(pChartRefOutDev)
/*?*/ 			{
/*?*/ 				DBG_ERROR("ChartModel::BuildChart: restoring lost pRefOutDev... something strange happend!");
/*?*/ 				SetRefDevice(pChartRefOutDev);
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				CHART_TRACE( "ChartModel::BuildChart no pRefOutDev!" );
/*?*/ 				if( pDocShell )
/*?*/ 				{
/*?*/                     // this is a valid upcast, because we 'know' that the
/*?*/                     // pDocShell member is always a SchChartDocShell
/*?*/                     OutputDevice * pOutDev = static_cast< SchChartDocShell * >( pDocShell )->GetRefDevice();
/*?*/                     SetRefDevice( pOutDev );
/*?*/                     GetOutliner()->SetRefDevice( pOutDev );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					OutputDevice* pOut = Application::GetDefaultDevice();
/*?*/ 					// this is no error: if a ChartModel is copied via clipboard there is no docshell
/*?*/ 					// DBG_ERROR("ChartModel::BuildChart : no Docshell! (OutputDevice)");
/*?*/ 					pChartRefOutDev = new VirtualDevice( *pOut );
/*?*/ 					MapMode aMapMode = pChartRefOutDev->GetMapMode();
/*?*/ 					aMapMode.SetMapUnit(MAP_100TH_MM);
/*?*/ 					pChartRefOutDev->SetMapMode(aMapMode);
/*?*/ 					SetRefDevice(pChartRefOutDev);
/*?*/ 					GetOutliner()->SetRefDevice(pChartRefOutDev);
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		CheckRanges(bCheckRanges);
/*N*/ 
/*N*/ 		if( !pChartData )		// create default chart if no chart exists so far
/*N*/ 			InitChartData();
/*N*/ 
/*N*/ 		if (GetColCount() && GetRowCount())
/*N*/ 		{
/*N*/ 			SdrPage* pPage = GetPage(0);
/*N*/ 			if (pPage)
/*N*/ 			{
/*N*/                 //#61431# MapMode speichern, am Ende BuildCharts restaurieren
/*N*/ 
/*N*/                 // remember the MapMode of the Printer and restore it at the end
/*N*/                 // of the current block, because it may be changed later in this
/*N*/                 // method (AdjustPrinter)
/*N*/                 lcl_MapModeResetGuard aPrinterMapModeGuard(
/*N*/                     static_cast< SchChartDocShell * >( GetObjectShell() )->GetPrinter() );
/*N*/ 
/*N*/                 AdjustPrinter();
/*N*/ 
/*N*/ 				// FG : DAs hier braucht es weil bei 3D-Charts der Bildchsimraufbau von der ChartScene
/*N*/ 				//      erst beim ersten Paint initiiert wird. Erst dann werden die Achsentitel ausgegeben
/*N*/ 				//      da ist aber BuildChart, aInitialSize=aPageSize setzt schon gelaufen. Deshalb diese
/*N*/ 				//      "Chart-Globale" Variable;
/*N*/ 				aInitialSizefor3d = aInitialSize;
/*N*/ 
/*N*/ 				// ItemSet mit entspr. Bereich anlegen
/*N*/ //-/				SfxItemSet aSet( GetItemPool(), SID_ATTR_3D_RANGE_SCENE, 0);
/*N*/ 				SfxItemSet aSet( GetItemPool(), 
/*N*/ 					SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
/*N*/ 				BOOL bItemSetUsed = FALSE;
/*N*/ 
/*N*/ 				if( pScene )
/*N*/ 				{
/*N*/ //-/					pScene->TakeAttributes(aSet, TRUE, TRUE);
/*N*/ 					aSet.Put(pScene->GetItemSet());
/*N*/ 					bItemSetUsed = TRUE;
/*N*/ 					aSceneMatrix = pScene->GetTransform();
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					SdrObject* pLoadedScene = GetObjWithId(CHOBJID_DIAGRAM, *pPage);
/*N*/ 					if (pLoadedScene && pLoadedScene->ISA(E3dPolyScene))
/*N*/ 					{
/*N*/ //-/						((E3dObject*)pLoadedScene)->SetAttrUseSubObjects(FALSE);
/*N*/ //-/						pLoadedScene->TakeAttributes(aSet, TRUE, TRUE);
/*N*/ 						aSet.Put(pLoadedScene->GetItemSet());
/*N*/ 						bItemSetUsed = TRUE;
/*N*/ 						aSceneMatrix = ((E3dPolyScene*) pLoadedScene)->GetTransform();
/*N*/ 						Rescue3DObjAttr(pLoadedScene->GetSubList()); //#52277#
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				DeleteChartObjects();
/*N*/ 				Size aPageSize = pPage->GetSize();
/*N*/ 
/*N*/ 				SdrObjGroup *pObj = NULL;
/*N*/ 				BOOL bNeedSetDiagram=FALSE;
/*N*/ 
/*N*/ 				// if type addin is set the core CreateChart shoudln't be called
/*N*/ 				// if an addin is used together with a base diagram type the base
/*N*/ 				// type should be set as chart type here (!= CHSTYLE_ADDIN)
/*N*/ 				if( eChartStyle != CHSTYLE_ADDIN )
/*N*/ 				{
/*N*/ 					// core CreateChart
/*N*/ 					CreateRectsAndTitles(whatTitle);
/*N*/ 
/*N*/ 					if(bResizePie && IsPieChart() && IsReal3D())
/*N*/ 					{
/*N*/ 						long nWidth=aChartRect.GetWidth();
/*N*/ 						long nHeight=aChartRect.GetHeight();
/*N*/ 						if( ((double)nHeight/(double)nWidth) > 0.5 )
/*N*/ 						{
/*?*/ 							bNeedSetDiagram=TRUE;
/*?*/ 							aChartRect.Top()+=nHeight/4;
/*?*/ 							aChartRect.Bottom()-=nHeight/4;
/*?*/   							SetDiagramHasBeenMovedOrResized(TRUE);
/*?*/ 							SetUseRelativePositions(TRUE);
/*?*/ 							bResizePie=FALSE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					pObj = CreateChart(aChartRect);
/*N*/ 
/*N*/ 					if (pObj->ISA(SchObjGroup))
/*N*/ 					{
/*N*/ 						((SchObjGroup *)pObj)->SetModel(this);
/*N*/ 						((SchObjGroup *)pObj)->SetGroupIsChart();
/*N*/ 					}
/*N*/ 
/*N*/ 					pPage->NbcInsertObject(pObj, 1);
/*N*/ 
/*N*/ 				}
/*N*/ 
/*N*/ 				// ====================
/*N*/ 				// call AddIn if required
/*N*/ 				// ====================
/*N*/ 				if( mbIsInitialized &&
/*N*/                     mxChartAddIn.is())
/*N*/ 				{
/*?*/ 					// avoid recursion
/*?*/ 					if( ! GetChartStatusFlag( CHS_NO_ADDIN_REFRESH ))
/*?*/ 					{
/*?*/ 						SetChartStatusFlag( CHS_NO_ADDIN_REFRESH );
/*?*/ 						try
/*?*/ 						{
/*?*/ 							mxChartAddIn->refresh();
/*?*/ 						}
/*?*/ 						catch( ::com::sun::star::uno::Exception aEx )
/*?*/ 						{
/*?*/ #ifdef DBG_UTIL
/*?*/ 							// convert ::rtl::OUString => tools String => ByteString
/*?*/ 							String aStr( aEx.Message );
/*?*/ 							ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
/*?*/ 							DBG_ERROR1( "AddIn threw exception during refresh(): %s", aBStr.GetBuffer());
/*?*/ #endif
/*?*/ 						}
/*?*/ 						ResetChartStatusFlag( CHS_NO_ADDIN_REFRESH );
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				aInitialSize = aPageSize;
/*N*/ 
/*N*/ 				if(pScene && IsReal3D() )
/*N*/ 				{
/*N*/ 
/*N*/ #ifndef NO_56798_FIX //#56798# QuickFix 5.0-Final
/*N*/ 
/*N*/ 					if(bClearDepth)
/*N*/ 					{
/*N*/ //-/						aSet.ClearItem(SID_ATTR_3D_DEPTH);
/*N*/ 						aSet.ClearItem(SDRATTR_3DOBJ_DEPTH);
/*N*/ 						bClearDepth=FALSE;
/*N*/ 					}
/*N*/ #endif
/*N*/ 
/*N*/ //-/					aSet.ClearItem(SID_ATTR_3D_DOUBLE_SIDED);	//#56941#
/*N*/ //-/					aSet.ClearItem(SID_ATTR_3D_HORZ_SEGS);
/*?*/ 					aSet.ClearItem(SDRATTR_3DOBJ_DOUBLE_SIDED);	//#56941#
/*?*/ 					aSet.ClearItem(SDRATTR_3DOBJ_HORZ_SEGS);
/*N*/ 
/*N*/ 					if(bItemSetUsed)
/*N*/ 					{
/*N*/ 						//funktioniert nur, wenn nicht durch vorherigen Aufruf (CheckRanges) eine MsgBox entstand
/*N*/ 						if(eChartStyle != eOldChartStyle)
/*N*/ 						{
/*N*/ 							// Falls sich der ChartStyle aendert, die Extrude-Tiefe
/*N*/ 							// auf unfueltig setzen
/*N*/ 							const SfxPoolItem* pPoolItem = NULL;
/*N*/ //-/							SfxItemState eState = aSet.GetItemState(SID_ATTR_3D_DEPTH, FALSE, &pPoolItem);
/*N*/ 							SfxItemState eState = aSet.GetItemState(SDRATTR_3DOBJ_DEPTH, FALSE, &pPoolItem);
/*N*/ 							if(eState == SFX_ITEM_SET)
/*N*/ 							{
/*N*/ 								// Ist gesetzt, invalidiere
/*N*/ //-/								aSet.InvalidateItem(SID_ATTR_3D_DEPTH);
/*N*/ 								aSet.InvalidateItem(SDRATTR_3DOBJ_DEPTH);
/*N*/ 								CHART_TRACE( "Chart: Invalidating saveable item ..." );
/*N*/ 							}
/*N*/ 						}
/*N*/ 
/*N*/ 						// Attribute an neuer Szene setzen
/*N*/ 
/*N*/ //-/						pScene->NbcSetAttributes(aSet, TRUE);
/*N*/ 						pScene->SetItemSet(aSet);
/*N*/ 
/*N*/ 					}
/*N*/ 
/*N*/ 					pScene->NbcSetTransform(aSceneMatrix);
/*N*/ 
/*N*/ 					const SfxPoolItem *pPoolItem = NULL;
/*N*/ 					if(!IsPieChart())
/*N*/ 					{
/*N*/ 						if(pChartXAxis->HasDescription())
/*N*/ 						{
/*N*/ 							if(GetAttr(CHOBJID_DIAGRAM_X_AXIS).GetItemState( SCHATTR_TEXT_OVERLAP, FALSE, &pPoolItem ) >= SFX_ITEM_AVAILABLE )
/*N*/ 							{
/*N*/ 								if(!( (const SfxBoolItem*) pPoolItem)->GetValue())
/*N*/ 									pScene->ReduceDescrList(aXDescrList);
/*N*/ 							}
/*N*/ 						}
/*N*/ 
/*N*/ 						if(pChartYAxis->HasDescription())
/*N*/ 						{
/*N*/ 							if(GetAttr(CHOBJID_DIAGRAM_Y_AXIS).GetItemState( SCHATTR_TEXT_OVERLAP, FALSE, &pPoolItem ) >= SFX_ITEM_AVAILABLE )
/*N*/ 							{
/*N*/ 								if(!( (const SfxBoolItem*) pPoolItem)->GetValue())
/*N*/ 									pScene->ReduceDescrList(aYDescrList);
/*N*/ 							}
/*N*/ 						}
/*N*/ 
/*N*/ 						if(pChartZAxis->HasDescription())
/*N*/ 						{
/*N*/ 							if(GetAttr(CHOBJID_DIAGRAM_Z_AXIS).GetItemState( SCHATTR_TEXT_OVERLAP, FALSE, &pPoolItem ) >= SFX_ITEM_AVAILABLE )
/*N*/ 							{
/*N*/ 								if(!( (const SfxBoolItem*) pPoolItem)->GetValue())
/*N*/ 									pScene->ReduceDescrList(aZDescrList);
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 
/*N*/ 				if(bNeedSetDiagram && pScene)
/*N*/ 				{
/*?*/ 					Rectangle aRectangle;
/*?*/ 					aRectangle = pScene->GetSnapRect();
/*?*/ 					((ChartScene *)pObj)->SetAskForLogicRect(FALSE);
/*?*/ 					SetDiagramRectangle(aRectangle);
/*?*/ 					SetDiagramHasBeenMovedOrResized(TRUE);
/*N*/ 				}
/*N*/ 
/*N*/ 				//#61431# MapMode speichern, am Ende BuildCharts restaurieren
/*N*/ 
/*N*/                 // the MapMode of the RefDevice is restored by the
/*N*/                 // lcl_MapModeResetGuard
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ 	//Jetzt sollen sich die Objekte wieder drum kümmern, da?das Model die Attribute erhält
/*N*/ 	SetAttrAutoStorage(TRUE);
/*N*/ 	SetDiagramRectangle( aChartRect );
/*N*/ 	eOldChartStyle = eChartStyle;
/*N*/ 
/*N*/     // notify build to listeners
/*N*/     Broadcast( SchReBuildHint());
/*N*/ }



/*N*/ void ChartModel::SetUseRelativePositions (BOOL value)
/*N*/ {
/*N*/ 	if (value == FALSE)
/*N*/ 	{
/*N*/ 		SetDiagramHasBeenMovedOrResized(FALSE);
/*N*/ 		SetMainTitleHasBeenMoved(FALSE);
/*N*/ 		SetSubTitleHasBeenMoved(FALSE);
/*N*/ 		SetLegendHasBeenMoved(FALSE);
/*N*/ 		SetXAxisTitleHasBeenMoved(FALSE);
/*N*/ 		SetYAxisTitleHasBeenMoved(FALSE);
/*N*/ 		SetZAxisTitleHasBeenMoved(FALSE);
/*N*/ 		if((IsReal3D() && IsPieChart()))
/*N*/ 		{
/*?*/ 			Matrix4D aMatrix; //Reset der Matrix.... könnte man auch grundsätzlich machen?
/*?*/ 			aMatrix.RotateX(-F_PI/3);
/*?*/ 			aSceneMatrix=aMatrix;
/*?*/ 			if(pScene)
/*?*/ 				pScene->NbcSetTransform(aSceneMatrix);
/*?*/ 			bResizePie=TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	bUseRelativePositionsForChartGroups = value;
/*N*/ }

/*************************************************************************
|*
|* bestimme Mittelwert
|*
\************************************************************************/



/*************************************************************************
|*
|* erzeuge mittelwertlinie
|*
\************************************************************************/



/*************************************************************************
|*
|* bestimme Varianz
|*
\************************************************************************/


/*N*/ double ChartModel::GetVariantY (long nRow)
/*N*/ {
/*N*/ 	long nColCnt = GetColCount();
/*N*/ 
/*N*/ 	if (nColCnt)
/*N*/ 	{
/*N*/ 		double fSum     = 0.0;
/*N*/ 		double fQuadSum = 0.0;
/*N*/ 		long nValidCols = nColCnt;
/*N*/ 
/*N*/ 		for (long nCol = 0;
/*N*/ 				  nCol < nColCnt;
/*N*/ 				  nCol ++)
/*N*/ 		{
/*N*/ 			double fData = GetData ((short) nCol, (short) nRow, FALSE);
/*N*/ 
/*N*/ 			if( fData == DBL_MIN )
/*N*/ 			{
/*N*/ 				nValidCols--;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				fSum     += fData;
/*N*/ 				fQuadSum += fData * fData;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		return nValidCols? (fQuadSum - fSum*fSum / (double)nValidCols) / (double)nValidCols: DBL_MIN;
/*N*/ 	}
/*N*/ 	else return 0;
/*N*/ }

/*************************************************************************
|*
|* bestimme Varianz
|*
\************************************************************************/


/*N*/ double ChartModel::GetSigmaY (long nRow)
/*N*/ {
/*N*/ 	return sqrt (GetVariantY (nRow));
/*N*/ }

/*************************************************************************
|*
|* bestimme Fehler des groessten Wertes
|*
\************************************************************************/


/*N*/ double ChartModel::GetBigErrorY (long   nRow,
/*N*/ 								 double fError)
/*N*/ {
/*N*/ 	double     fMax    = 0.0;
/*N*/ 	long       nColCnt = GetColCount();
/*N*/ 
/*N*/ 	for (long nCol = 0;
/*N*/ 			  nCol < nColCnt;
/*N*/ 			  nCol ++)
/*N*/ 	{
/*N*/ 		double fData = GetData ((short) nCol, (short) nRow, FALSE);
/*N*/ 
/*N*/ 		if (fData != DBL_MIN)
/*N*/ 			if (fData > fMax) fMax = fData;
/*N*/ 	}
/*N*/ 
/*N*/ 	return fMax * fError / 100.0;
/*N*/ }

/*************************************************************************
|*
|* bestimme lineare regression
|*
\************************************************************************/



}
