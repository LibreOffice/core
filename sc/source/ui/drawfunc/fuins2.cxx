/*************************************************************************
 *
 *  $RCSfile: fuins2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-14 15:33:37 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <sfx2/frameobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/stritem.hxx>
#include <so3/insdlg.hxx>
#include <so3/svstor.hxx>
#include <so3/plugin.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <sch/schdll0.hxx>
#include <starmath/smdll0.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/pfiledlg.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <svtools/moduleoptions.hxx>

#include "fuinsert.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "chartarr.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "undotab.hxx"
#include "chartlis.hxx"
#include "uiitems.hxx"
#include "globstr.hrc"

extern SdrObject* pSkipPaintObj;            // output.cxx - dieses Objekt nicht zeichnen

//------------------------------------------------------------------------

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

void lcl_ChartInit( SvInPlaceObjectRef aIPObj, ScViewData* pViewData, Window* pWin )
{
    USHORT nCol1 = 0;
    USHORT nRow1 = 0;
    USHORT nTab1 = 0;
    USHORT nCol2 = 0;
    USHORT nRow2 = 0;
    USHORT nTab2 = 0;

    ScMarkData& rMark = pViewData->GetMarkData();
    if ( !rMark.IsMarked() )
        pViewData->GetView()->MarkDataArea( TRUE );

    if ( pViewData->GetSimpleArea( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 ) )
    {
        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );
        if ( nCol2>nCol1 || nRow2>nRow1 )
        {
            String aChartName;
            SvInfoObject* pInfoObj = pViewData->GetDocShell()->Find( aIPObj );
            if ( pInfoObj )
                aChartName = pInfoObj->GetObjName();
            else
                DBG_ERROR( "IP-Object not found :-/" );

            ScDocument* pDoc = pViewData->GetDocument();
            pDoc->LimitChartArea( nTab1, nCol1,nRow1, nCol2,nRow2 );

                //  Chart-Collection gibt's nicht mehr

            ScRange aRange( nCol1,nRow1,nTab1, nCol2,nRow2,nTab1 );
            ScChartListener* pCL = new ScChartListener( aChartName, pDoc, aRange );
            pDoc->GetChartListenerCollection()->Insert( pCL );
            pCL->StartListeningTo();

            ScChartArray aParam( pDoc, nTab1, nCol1,nRow1, nCol2,nRow2, String() );
            SchMemChart* pMemChart = aParam.CreateMemChart();
            SchDLL::Update( aIPObj, pMemChart, pWin );
            delete pMemChart;
        }
    }
}

#ifdef WNT
#pragma optimize("",off)
#endif

void lcl_ChartInit2( SvInPlaceObjectRef aIPObj, ScViewData* pViewData, Window* pWin,
                        const SfxItemSet* pReqArgs, const String& rChartName )
{
    ScDocument* pDoc = pViewData->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();

    const SfxPoolItem* pItem;
    BOOL    bRowHeader = TRUE;
    BOOL    bColHeader = TRUE;

    if( IS_AVAILABLE( SID_ATTR_ROWHEADERS, &pItem ) )
        bRowHeader = ((const SfxBoolItem*)pItem)->GetValue();

    if( IS_AVAILABLE( SID_ATTR_COLHEADERS, &pItem ) )
        bColHeader = ((const SfxBoolItem*)pItem)->GetValue();

    SchMemChart* pMemChart;
    ScChartListener* pChartListener;
    BOOL bMulti;
    if ( IS_AVAILABLE( FN_PARAM_5, &pItem ) )
    {
        String aStr( ((const SfxStringItem*)pItem)->GetValue() );
        ScRangeListRef aRangeListRef( new ScRangeList() );
        aRangeListRef->Parse( aStr, pDoc );

        if ( aRangeListRef->Count() )
        {
            ScChartArray aParam( pDoc, aRangeListRef, String() );
            aParam.SetHeaders( bColHeader, bRowHeader );
            pMemChart = aParam.CreateMemChart();

            pChartListener = new ScChartListener( rChartName, pDoc, aRangeListRef );
            bMulti = TRUE;
        }
        else
            bMulti = FALSE;
    }
    else
        bMulti = FALSE;
    if ( !bMulti )
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        USHORT nColStart = aMarkRange.aStart.Col();
        USHORT nRowStart = aMarkRange.aStart.Row();
        USHORT nColEnd = aMarkRange.aEnd.Col();
        USHORT nRowEnd = aMarkRange.aEnd.Row();
        USHORT nTab = aMarkRange.aStart.Tab();

        pDoc->LimitChartArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );

            //  Chart-Collection gibt's nicht mehr

        ScChartArray aParam( pDoc, nTab, nColStart, nRowStart, nColEnd, nRowEnd, String() );
        aParam.SetHeaders( bColHeader, bRowHeader );
        pMemChart = aParam.CreateMemChart();

        pChartListener = new ScChartListener( rChartName, pDoc,
            ScRange( nColStart, nRowStart, nTab, nColEnd, nRowEnd, nTab ) );
    }
    pDoc->GetChartListenerCollection()->Insert( pChartListener );
    pChartListener->StartListeningTo();
    SchDLL::Update( aIPObj, pMemChart, *pReqArgs, pWin );
    delete pMemChart;
}

#ifdef WNT
#pragma optimize("",on)
#endif

/*************************************************************************
|*
|* FuInsertOLE::Konstruktor
|*
\************************************************************************/

FuInsertOLE::FuInsertOLE(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    if( ! rReq.IsAPI() )
        rReq.Done();

    //! hier DLL's initalisieren, damit die Factories existieren?

    SvInPlaceObjectRef aIPObj;
    SvStorageRef aStor = new SvStorage( String() );
    BOOL bIsFromFile = FALSE;

    USHORT nSlot = rReq.GetSlot();
    if (nSlot == SID_INSERT_FLOATINGFRAME)
    {
        SfxInsertFloatingFrameDialog aDlg( pWin );
//      aDlg.SetHelpId(nSlot);
        aIPObj = aDlg.Execute( aStor );

        // damit DrawShell eingeschaltet wird (Objekt aktivieren ist unnoetig):
        bIsFromFile = TRUE;
    }
    else if (nSlot == SID_INSERT_SMATH)
    {
        if ( SvtModuleOptions().IsMath() )
            aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
//                                      *OFF_APP()->GetSmDLL()->pSmDocShellFactory,
                                        *SM_MOD()->pSmDocShellFactory,
                                        aStor );
    }
    else if (nSlot == SID_INSERT_PLUGIN)
    {
        SvInsertPlugInDialog aDlg;
        aDlg.SetHelpId(nSlot);
        aIPObj = aDlg.Execute( pWin, aStor );
        bIsFromFile = TRUE;                                 // nicht aktivieren
    }
    else if (nSlot == SID_INSERT_SOUND || nSlot == SID_INSERT_VIDEO)
    {
        // create special filedialog for plugins
        SvxPluginFileDlg aPluginFileDialog(pWin, nSlot);

        // open filedlg
        if (RET_OK == aPluginFileDialog.Execute())
        {
            // get URL
            String aStrURL(aPluginFileDialog.GetPath());
            aStrURL = URIHelper::SmartRelToAbs( aStrURL );

            INetURLObject aURL;
            aURL.SetSmartProtocol( INET_PROT_FILE );

            if ( aURL.SetURL( aStrURL ) )
            {
                // create plugin, initialize, etc.
                SvFactory * pPlugIn = SvFactory::GetDefaultPlugInFactory();
                SvStorageRef aStor = new SvStorage( EMPTY_STRING, STREAM_STD_READWRITE );
                SvPlugInObjectRef xObj = &pPlugIn->CreateAndInit( *pPlugIn, aStor );
                xObj->SetPlugInMode( (USHORT)PLUGIN_EMBEDED );
                xObj->SetURL( aURL );
                aIPObj = (SvInPlaceObject*)&xObj;
            }
            else
            {
                //! error message
                //! can this happen???
            }
        }
    }
    else if (nSlot == SID_INSERT_APPLET)
    {
        SvInsertAppletDialog aDlg;
        aDlg.SetHelpId(nSlot);
        aIPObj = aDlg.Execute( pWin, aStor );
        bIsFromFile = TRUE;                                 // nicht aktivieren
    }
    else    // SID_INSERT_OBJECT
    {
        SvInsertOleObjectDialog aDlg;
        aDlg.SetHelpId(nSlot);
        SvObjectServerList aServerLst;
        aDlg.FillObjectServerList(&aServerLst);
        aServerLst.Remove( *ScDocShell::ClassFactory() );   // Starcalc nicht anzeigen
        aIPObj = aDlg.Execute(pWin, aStor, &aServerLst );
        bIsFromFile = !aDlg.IsCreateNew();
    }

    //  SvInsertObjectDialog (alles in einem Dialog) wird nicht mehr benutzt

    if( aIPObj.Is() )
    {
        pView->UnmarkAll();

        SvEmbeddedInfoObject* pInfoObj = pViewSh->GetViewFrame()->GetObjectShell()->
                                            InsertObject(aIPObj, String());
        if ( !pInfoObj )
            pViewSh->ErrorMessage( STR_ERR_INSERTOBJ );
        else
        {
            String aName = pInfoObj->GetObjName();

            //  aSize immer in 1/100mm, egal was das Ole-Objekt hat

            Size aSize = aIPObj->GetVisArea().GetSize();
            if (aSize.Height() == 0 || aSize.Width() == 0)
            {
                aSize.Width() = 5000;
                aSize.Height() = 5000;
                Size aObjSize = Window::LogicToLogic( aSize,
                                MapMode( MAP_100TH_MM ), MapMode( aIPObj->GetMapUnit() ) );
                aIPObj->SetVisAreaSize(aObjSize);

                //  re-convert aSize to 1/100th mm to avoid rounding errors in comparison below
                aSize = Window::LogicToLogic( aObjSize,
                                MapMode( aIPObj->GetMapUnit() ), MapMode( MAP_100TH_MM ) );
            }
            else
                aSize = Window::LogicToLogic( aSize,
                                MapMode( aIPObj->GetMapUnit() ), MapMode( MAP_100TH_MM ) );

            //  Chart initialisieren ?

            if ( SvtModuleOptions().IsChart() &&
                    aIPObj->GetClassName() == *SCH_MOD()->pSchChartDocShellFactory )
                lcl_ChartInit( aIPObj, pViewSh->GetViewData(), pWin );

            Point aPnt = pViewSh->GetInsertPos();
            Rectangle aRect (aPnt, aSize);
            SdrOle2Obj* pObj = new SdrOle2Obj(aIPObj, aName, aRect);

                // Dieses Objekt nicht vor dem Aktivieren zeichnen
                // (in MarkListHasChanged kommt ein Update)
            if (!bIsFromFile)
                pSkipPaintObj = pObj;

            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            pView->InsertObject(pObj, *pPV);
            pObj->SetOleLink(pInfoObj);

            //  #73279# Math objects change their object size during InsertObject.
            //  New size must be set in SdrObject, or a wrong scale will be set at
            //  ActivateObject.

            Size aNewSize = Window::LogicToLogic( aIPObj->GetVisArea().GetSize(),
                            MapMode( aIPObj->GetMapUnit() ), MapMode( MAP_100TH_MM ) );
            if ( aNewSize != aSize )
            {
                aRect.SetSize( aNewSize );
                pObj->SetLogicRect( aRect );
            }

            if ( !rReq.IsAPI() )
            {
                // XXX Activate aus Makro ist toedlich !!! ???
                if (bIsFromFile)
                {
                    // #45012# Objekt ist selektiert, also Draw-Shell aktivieren
                    pViewShell->SetDrawShell( TRUE );
                }
                else
                {
                    pViewShell->ActivateObject( (SdrOle2Obj*) pObj, SVVERB_SHOW );
                    pSkipPaintObj = NULL;
                }
            }
        }
    }
}

/*************************************************************************
|*
|* FuInsertOLE::Destruktor
|*
\************************************************************************/

FuInsertOLE::~FuInsertOLE()
{
}

/*************************************************************************
|*
|* FuInsertOLE::Function aktivieren
|*
\************************************************************************/

void FuInsertOLE::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* FuInsertOLE::Function deaktivieren
|*
\************************************************************************/

void FuInsertOLE::Deactivate()
{
    FuPoor::Deactivate();
}

/*************************************************************************
|*
|* FuInsertChart::Konstruktor
|*
\************************************************************************/

FuInsertChart::FuInsertChart(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();

    if( ! rReq.IsAPI() )
        rReq.Done();

    SvStorageRef aStor = new SvStorage( String() );
    SvInPlaceObjectRef aIPObj;
    if ( SvtModuleOptions().IsChart() )
        aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                                        *SCH_MOD()->pSchChartDocShellFactory,
                                        aStor );

    if( aIPObj.Is() )
    {
        pView->UnmarkAll();

        SvEmbeddedInfoObject* pInfoObj = pViewSh->GetViewFrame()->GetObjectShell()->
                                            InsertObject(aIPObj, String());
        if ( !pInfoObj )
            pViewSh->ErrorMessage( STR_ERR_INSERTOBJ );
        else
        {
            String aName = pInfoObj->GetObjName();

            ScRangeListRef aDummy;
            Rectangle aMarkDest;
            USHORT nMarkTab;
            BOOL bDrawRect = pViewShell->GetChartArea( aDummy, aMarkDest, nMarkTab );

                //  Objekt-Groesse

            Size aSize = aIPObj->GetVisArea().GetSize();
            BOOL bSizeCh = FALSE;
            if (bDrawRect && !aMarkDest.IsEmpty())
            {
                aSize = aMarkDest.GetSize();
                bSizeCh = TRUE;
            }
            if (aSize.Height() <= 0 || aSize.Width() <= 0)
            {
                aSize.Width() = 5000;
                aSize.Height() = 5000;
                bSizeCh = TRUE;
            }
            if (bSizeCh)
            {
                aSize = Window::LogicToLogic( aSize, MapMode( MAP_100TH_MM ), MapMode( aIPObj->GetMapUnit() ) );
                aIPObj->SetVisAreaSize(aSize);
            }

            ScViewData* pData = pViewSh->GetViewData();
            ScDocShell* pScDocSh = pData->GetDocShell();
            ScDocument* pScDoc   = pScDocSh->GetDocument();
            BOOL bUndo (pScDoc->IsUndoEnabled());

            if( pReqArgs )
            {
                lcl_ChartInit2( aIPObj, pData, pWin, pReqArgs, aName );
                const SfxPoolItem* pItem;
                UINT16 nToTable = 0;

                if( IS_AVAILABLE( FN_PARAM_4, &pItem ) )
                {
                    if ( pItem->ISA( SfxUInt16Item ) )
                        nToTable = ((const SfxUInt16Item*)pItem)->GetValue();
                    else if ( pItem->ISA( SfxBoolItem ) )
                    {
                        //  #46033# in der idl fuer Basic steht FN_PARAM_4 als SfxBoolItem
                        //  -> wenn gesetzt, neue Tabelle, sonst aktuelle Tabelle

                        if ( ((const SfxBoolItem*)pItem)->GetValue() )
                            nToTable = pScDoc->GetTableCount();
                        else
                            nToTable = pData->GetTabNo();
                    }
                }
                else
                {
                    if (bDrawRect)
                        nToTable = nMarkTab;
                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_4, nToTable ) );
                }

                // auf neue Tabelle ausgeben?
                if ( nToTable == pScDoc->GetTableCount() )
                {
                    // dann los...
                    String      aTabName;
                    USHORT      nNewTab = pScDoc->GetTableCount();

                    pScDoc->CreateValidTabName( aTabName );

                    if ( pScDoc->InsertTab( nNewTab, aTabName ) )
                    {
                        BOOL bAppend = TRUE;

                        if (bUndo)
                        {
                            pScDocSh->GetUndoManager()->AddUndoAction(
                                        new ScUndoInsertTab( pScDocSh, nNewTab,
                                                             bAppend, aTabName ) );
                        }

                        pScDocSh->Broadcast( ScTablesHint( SC_TAB_INSERTED, nNewTab ) );
                        pViewSh->SetTabNo( nNewTab, TRUE );
                        pScDocSh->PostPaintExtras();            //! erst hinterher ???
                    }
                    else
                    {
                        DBG_ERROR( "Could not create new table :-/" );
                    }
                }
                else if ( nToTable != pData->GetTabNo() )
                {
                    pViewSh->SetTabNo( nToTable, TRUE );
                }
            }
            else
                lcl_ChartInit( aIPObj, pData, pWin );

                //  Objekt-Position

            Point aStart;
            if ( bDrawRect )
                aStart = aMarkDest.TopLeft();                       // per Hand markiert
            else
            {
                USHORT nC0, nR0, nT0, nC1, nR1, nT1;
                if( pData->GetSimpleArea( nC0, nR0, nT0, nC1, nR1, nT1 )
                    && ( nT0 == nT1 ) )
                {
                    // Einfache Selektion in der gleichen Tabelle:
                    // Positionieren 1/2 Spalte rechts
                    // und 1 1/2 Zeilen unterhalb des Starts
                    ScDocument* pScDoc = pData->GetDocument();
                    ULONG x = 0, y = 0;
                    for( USHORT i = 0; i <= nC1; i++ )
                        x += pScDoc->GetColWidth( i, nT0 );
                    while( ++i <= MAXCOL )
                    {
                        USHORT n = pScDoc->GetColWidth( i, nT0 );
                        if( n )
                        {
                            x += n / 2; break;
                        }
                    }
                    for( i = 0; i <= nR0; i++ )
                        y += pScDoc->FastGetRowHeight( i, nT0 );
                    while( ++i <= MAXROW )
                    {
                        USHORT n = pScDoc->FastGetRowHeight( i, nT0 );
                        if( n )
                        {
                            y += n / 2; break;
                        }
                    }
                    // Das ganze von Twips nach 1/100 mm
                    x = (ULONG) ((double) x * HMM_PER_TWIPS);
                    y = (ULONG) ((double) y * HMM_PER_TWIPS);
                    aStart = Point( x, y );
                }
                else
                    aStart = pViewSh->GetInsertPos();
            }

            Rectangle aRect (aStart, aSize);
            SdrOle2Obj* pObj = new SdrOle2Obj(aIPObj, aName, aRect);

                // Dieses Objekt nicht vor dem Aktivieren zeichnen
                // (in MarkListHasChanged kommt ein Update)
            pSkipPaintObj = pObj;

            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            pView->InsertObject(pObj, *pPV);
            pObj->SetOleLink(pInfoObj);

            // Dies veranlaesst Chart zum sofortigen Update
            SvData aEmpty;

            aIPObj->SendDataChanged( aEmpty );
            aIPObj->SendViewChanged();

            if (!rReq.IsAPI())
            {
                // XXX Activate aus Makro ist toedlich !!! ???
                pViewShell->ActivateObject( (SdrOle2Obj*) pObj, SVVERB_SHOW );
            }

            pSkipPaintObj = NULL;
        }
    }

    pViewShell->ResetChartArea();       // Einstellungen nur einmal auslesen
}

/*************************************************************************
|*
|* FuInsertChart::Destruktor
|*
\************************************************************************/

FuInsertChart::~FuInsertChart()
{
}

/*************************************************************************
|*
|* FuInsertChart::Function aktivieren
|*
\************************************************************************/

void FuInsertChart::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* FuInsertChart::Function deaktivieren
|*
\************************************************************************/

void FuInsertChart::Deactivate()
{
    FuPoor::Deactivate();
}


