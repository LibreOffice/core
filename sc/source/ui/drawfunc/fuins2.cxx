/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuins2.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:51:46 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//------------------------------------------------------------------------

#include <toolkit/helper/vclunohelper.hxx>
#include <sot/exchange.hxx>
#include <svtools/globalnameitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/stritem.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/pfiledlg.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <svtools/moduleoptions.hxx>
#include <svtools/insdlg.hxx>
#include <svtools/soerr.hxx>
#include <svx/svxdlg.hxx>
#include <sot/clsids.hxx>

// BM --
#include <cppuhelper/component_context.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/XDataProvider.hpp>
#include <com/sun/star/chart2/XDataReceiver.hpp>

#include "ScDevChart.hxx"

using namespace ::com::sun::star;
// BM --

// erAck
#include "chart2uno.hxx"
// erAck

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

void lcl_ChartInit( const uno::Reference < embed::XEmbeddedObject >& aIPObj, ScViewData* pViewData, Window* pWin )
{
    SCCOL nCol1 = 0;
    SCROW nRow1 = 0;
    SCTAB nTab1 = 0;
    SCCOL nCol2 = 0;
    SCROW nRow2 = 0;
    SCTAB nTab2 = 0;

    ScMarkData& rMark = pViewData->GetMarkData();
    if ( !rMark.IsMarked() )
        pViewData->GetView()->MarkDataArea( TRUE );

    if ( pViewData->GetSimpleArea( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 ) )
    {
        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );
        if ( nCol2>nCol1 || nRow2>nRow1 )
        {
            String aChartName = pViewData->GetDocShell()->GetEmbeddedObjectContainer().GetEmbeddedObjectName( aIPObj );

            ScDocument* pDoc = pViewData->GetDocument();
            pDoc->LimitChartArea( nTab1, nCol1,nRow1, nCol2,nRow2 );

                //  Chart-Collection gibt's nicht mehr

            ScRange aRange( nCol1,nRow1,nTab1, nCol2,nRow2,nTab1 );
            ScChartListener* pCL = new ScChartListener( aChartName, pDoc, aRange );
            pDoc->GetChartListenerCollection()->Insert( pCL );
            pCL->StartListeningTo();

            ScChartArray aParam( pDoc, nTab1, nCol1,nRow1, nCol2,nRow2, String() );
            SchMemChart* pMemChart = aParam.CreateMemChart();
            // TODO/LATER: looks like there is no need to update replacement, but it should be checked.
            SchDLL::Update( aIPObj, pMemChart, pWin );
            delete pMemChart;
        }
    }
}

#ifdef WNT
#pragma optimize("",off)
#endif

void lcl_ChartInit2( const uno::Reference < embed::XEmbeddedObject >& aIPObj, ScViewData* pViewData, Window* pWin,
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

    SchMemChart* pMemChart = 0;
    ScChartListener* pChartListener = 0;
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
        SCCOL nColStart = aMarkRange.aStart.Col();
        SCROW nRowStart = aMarkRange.aStart.Row();
        SCCOL nColEnd = aMarkRange.aEnd.Col();
        SCROW nRowEnd = aMarkRange.aEnd.Row();
        SCTAB nTab = aMarkRange.aStart.Tab();

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
    // TODO/LATER: looks like there is no need to update replacement, but it should be checked.
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

    uno::Reference < embed::XEmbeddedObject > xObj;
    uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    BOOL bIsFromFile = FALSE;
    ::rtl::OUString aName;

    sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
    ::rtl::OUString aIconMediaType;
    uno::Reference< io::XInputStream > xIconMetaFile;


    USHORT nSlot = rReq.GetSlot();
    SFX_REQUEST_ARG( rReq, pNameItem, SfxGlobalNameItem, SID_INSERT_OBJECT, sal_False );
    if ( nSlot == SID_INSERT_OBJECT && pNameItem )
    {
        SvGlobalName aClassName = pNameItem->GetValue();
        xObj = pViewShell->GetViewFrame()->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( aClassName.GetByteSequence(), aName );
    }
    else if ( nSlot == SID_INSERT_SMATH )
    {
        if ( SvtModuleOptions().IsMath() )
        {
            nSlot = SID_INSERT_OBJECT;
            xObj = pViewShell->GetViewFrame()->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SM_CLASSID_60 ).GetByteSequence(), aName );
            rReq.AppendItem( SfxGlobalNameItem( SID_INSERT_OBJECT, SvGlobalName( SO3_SM_CLASSID_60 ) ) );
        }
    }
    else
    {
        SvObjectServerList aServerLst;
        switch ( nSlot )
        {
            case SID_INSERT_OBJECT :
                aServerLst.FillInsertObjects();
                aServerLst.Remove( ScDocShell::Factory().GetClassId() );   // Starcalc nicht anzeigen
                //TODO/LATER: currently no inserting of ClassId into SfxRequest!
            case SID_INSERT_PLUGIN :
            case SID_INSERT_APPLET :
            case SID_INSERT_FLOATINGFRAME :
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                SfxAbstractInsertObjectDialog* pDlg =
                        pFact->CreateInsertObjectDialog( pViewShell->GetWindow(), nSlot,
                        xStorage, &aServerLst );
                if ( pDlg )
                {
                    pDlg->Execute();
                    xObj = pDlg->GetObject();

                    xIconMetaFile = pDlg->GetIconIfIconified( &aIconMediaType );
                    if ( xIconMetaFile.is() )
                        nAspect = embed::Aspects::MSOLE_ICON;

                    if ( xObj.is() )
                        pViewSh->GetObjectShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );
                    // damit DrawShell eingeschaltet wird (Objekt aktivieren ist unnoetig):
                    bIsFromFile = !pDlg->IsCreateNew();
                    DELETEZ( pDlg );
                }

                break;
            }
            case SID_INSERT_SOUND :
            case SID_INSERT_VIDEO :
            {
                // create special filedialog for plugins
                SvxPluginFileDlg aPluginFileDialog(pWin, nSlot);

                // open filedlg
                if ( ERRCODE_NONE == aPluginFileDialog.Execute() )
                {
                    // get URL
                    INetURLObject aURL;
                    aURL.SetSmartProtocol( INET_PROT_FILE );
                    if ( aURL.SetURL( aPluginFileDialog.GetPath() ) )
                    {
                        // create a plugin object
                        ::rtl::OUString aName;
                        SvGlobalName aClassId( SO3_PLUGIN_CLASSID );
                        comphelper::EmbeddedObjectContainer aCnt( xStorage );
                        xObj = aCnt.CreateEmbeddedObject( aClassId.GetByteSequence(), aName );
                        if ( xObj.is() && svt::EmbeddedObjectRef::TryRunningState( xObj ) )
                        {
                            // set properties from dialog
                            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                            if ( xSet.is() )
                            {
                                xSet->setPropertyValue( ::rtl::OUString::createFromAscii("PluginURL"),
                                        uno::makeAny( ::rtl::OUString( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) ) );
                            }
                        }
                    }
                    else
                    {
                        DBG_ERROR("Invalid URL!");
                        //! error message
                        //! can this happen???
                    }
                }
            }
        }
    }

    //  SvInsertObjectDialog (alles in einem Dialog) wird nicht mehr benutzt
    if (xObj.is())
    {
        pView->UnmarkAll();

        try
        {
            ::svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
            Size aSize;
            MapMode aMap100( MAP_100TH_MM );
            MapUnit aMapUnit = MAP_100TH_MM;

            if ( nAspect == embed::Aspects::MSOLE_ICON )
            {
                aObjRef.SetGraphicStream( xIconMetaFile, aIconMediaType );
                aSize = aObjRef.GetSize( &aMap100 );
            }
            else
            {
                awt::Size aSz;
                try
                {
                    aSz = xObj->getVisualAreaSize( nAspect );
                }
                catch( embed::NoVisualAreaSizeException& )
                {
                    // the default size will be set later
                }

                aSize = Size( aSz.Width, aSz.Height );

                aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
                if (aSize.Height() == 0 || aSize.Width() == 0)
                {
                    // Rechteck mit ausgewogenem Kantenverhaeltnis
                    aSize.Width() = 5000;
                    aSize.Height() = 5000;
                    Size aTmp = OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aMapUnit );
                    aSz.Width = aTmp.Width();
                    aSz.Height = aTmp.Height();
                    xObj->setVisualAreaSize( nAspect, aSz );

                    //  re-convert aSize to 1/100th mm to avoid rounding errors in comparison below
                    aSize = Window::LogicToLogic( aTmp,
                                    MapMode( aMapUnit ), aMap100 );
                }
                else
                    aSize = Window::LogicToLogic( aSize,
                                    MapMode( aMapUnit ), aMap100 );
            }

            //  Chart initialisieren ?
            if ( SvtModuleOptions().IsChart() && SotExchange::IsChart( SvGlobalName( xObj->getClassID() ) ) )
                lcl_ChartInit( xObj, pViewSh->GetViewData(), pWin );

            ScViewData* pData = pViewSh->GetViewData();

            Point aPnt = pViewSh->GetInsertPos();
            if ( pData->GetDocument()->IsNegativePage( pData->GetTabNo() ) )
                aPnt.X() -= aSize.Width();      // move position to left edge
            Rectangle aRect (aPnt, aSize);
            SdrOle2Obj* pObj = new SdrOle2Obj( aObjRef, aName, aRect);

                // Dieses Objekt nicht vor dem Aktivieren zeichnen
                // (in MarkListHasChanged kommt ein Update)
            if (!bIsFromFile)
                pSkipPaintObj = pObj;

            SdrPageView* pPV = pView->GetSdrPageView();
            pView->InsertObjectAtView(pObj, *pPV);

            if ( nAspect != embed::Aspects::MSOLE_ICON )
            {
                //  #73279# Math objects change their object size during InsertObject.
                //  New size must be set in SdrObject, or a wrong scale will be set at
                //  ActivateObject.

                try
                {
                    awt::Size aSz = xObj->getVisualAreaSize( nAspect );

                    Size aNewSize( aSz.Width, aSz.Height );
                    aNewSize = OutputDevice::LogicToLogic( aNewSize, aMapUnit, MAP_100TH_MM );

                    if ( aNewSize != aSize )
                    {
                        aRect.SetSize( aNewSize );
                        pObj->SetLogicRect( aRect );
                    }
                }
                catch( embed::NoVisualAreaSizeException& )
                {}
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

            rReq.Done();
        }
        catch( uno::Exception& )
        {
            OSL_ASSERT( "May need error handling here!\n" );
        }
    }
    else
        rReq.Ignore();
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

    if( ScDevChart::UseDevChart())
    {
        // BM --

        uno::Reference< chart2::XDataProvider > xDataProvider = new
            ScChart2DataProvider( pViewSh->GetViewData()->GetDocShell());

        // get range
        ::rtl::OUString aRangeString;
        const SfxPoolItem* pItem;
        if ( IS_AVAILABLE( FN_PARAM_5, &pItem ) )
            aRangeString = ::rtl::OUString( ((const SfxStringItem*)pItem)->GetValue());

        uno::Reference< lang::XMultiServiceFactory > xMultServFac( ::comphelper::getProcessServiceFactory());
        OSL_ASSERT( xMultServFac.is());
        if( xMultServFac.is())
        {
            uno::Reference< frame::XComponentLoader > xLoader(
                xMultServFac->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.frame.Desktop" ))),
                uno::UNO_QUERY );

            OSL_ASSERT( xLoader.is());
            if( xLoader.is())
            {
                uno::Sequence< beans::PropertyValue > aArgs;

                uno::Reference< ::com::sun::star::chart2::XDataReceiver > xReceiver(
                    xLoader->loadComponentFromURL(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/chart" )),
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" )),
                        0, // flags
                        aArgs ),
                    uno::UNO_QUERY );

                OSL_ASSERT( xReceiver.is());
                if( xReceiver.is())
                {
                    xReceiver->attachDataProvider( xDataProvider );
                    xReceiver->setRangeRepresentation( aRangeString );
                }
            }
        }
        else
            DBG_ERROR( "Couldn't get XMultiComponentFactory" );

        return;
        // BM --
    }
    else
    {
        if ( SvtModuleOptions().IsChart() )
        {
            pView->UnmarkAll();
            ::rtl::OUString aName;

            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
            uno::Reference < embed::XEmbeddedObject > xObj =
                pViewShell->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID_60 ).GetByteSequence(), aName );
            if ( !xObj.is() )
                pViewSh->ErrorMessage( STR_ERR_INSERTOBJ );
            else
            {
                ScRangeListRef aDummy;
                Rectangle aMarkDest;
                SCTAB nMarkTab;
                BOOL bDrawRect = pViewShell->GetChartArea( aDummy, aMarkDest, nMarkTab );

                //  Objekt-Groesse
                awt::Size aSz = xObj->getVisualAreaSize( nAspect );
                Size aSize( aSz.Width, aSz.Height );

                MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

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
                    aSize = Window::LogicToLogic( aSize, MapMode( MAP_100TH_MM ), MapMode( aMapUnit ) );
                    aSz.Width = aSize.Width();
                    aSz.Height = aSize.Height();
                    xObj->setVisualAreaSize( nAspect, aSz );
                }

                ScViewData* pData = pViewSh->GetViewData();
                ScDocShell* pScDocSh = pData->GetDocShell();
                ScDocument* pScDoc   = pScDocSh->GetDocument();
                BOOL bUndo (pScDoc->IsUndoEnabled());

                if( pReqArgs )
                {
                    lcl_ChartInit2( xObj, pData, pWin, pReqArgs, aName );
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
                                nToTable = static_cast<UINT16>(pScDoc->GetTableCount());
                            else
                                nToTable = static_cast<UINT16>(pData->GetTabNo());
                        }
                    }
                    else
                    {
                        if (bDrawRect)
                            nToTable = static_cast<UINT16>(nMarkTab);
                        rReq.AppendItem( SfxUInt16Item( FN_PARAM_4, nToTable ) );
                    }

                    // auf neue Tabelle ausgeben?
                    if ( nToTable == pScDoc->GetTableCount() )
                    {
                        // dann los...
                        String      aTabName;
                        SCTAB       nNewTab = pScDoc->GetTableCount();

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
                    lcl_ChartInit( xObj, pData, pWin );

                //  Objekt-Position

                Point aStart;
                if ( bDrawRect )
                    aStart = aMarkDest.TopLeft();                       // marked by hand
                else
                {
                    SCCOL nC0, nC1;
                    SCROW nR0, nR1;
                    SCTAB nT0, nT1;
                    if( pData->GetSimpleArea( nC0, nR0, nT0, nC1, nR1, nT1 )
                        && ( nT0 == nT1 ) )
                    {
                        // Einfache Selektion in der gleichen Tabelle:
                        // Positionieren 1/2 Spalte rechts
                        // und 1 1/2 Zeilen unterhalb des Starts
                        ScDocument* pScDoc = pData->GetDocument();
                        long x = 0, y = 0;
                        SCCOL i;
                        for( i = 0; i <= nC1; i++ )
                            x += pScDoc->GetColWidth( i, nT0 );
                        while( ++i <= MAXCOL )
                        {
                            USHORT n = pScDoc->GetColWidth( i, nT0 );
                            if( n )
                            {
                                x += n / 2; break;
                            }
                        }
                        y += pScDoc->FastGetRowHeight( 0, nR0, nT0);
                        // Find the first non-hidden row with row height >0 and
                        // add the half of it's height.
                        SCROW j = pScDoc->FastGetFirstNonHiddenRow( nR0 + 1, nT0 );
                        if (j <= MAXROW)
                        {
                            USHORT n = pScDoc->FastGetOriginalRowHeight( j, nT0);
                            if (n)
                                y += n / 2;
                            else
                            {   // bad luck, not hidden but height 0
                                for( ++j; j <= MAXROW; ++j )
                                {
                                    n = pScDoc->FastGetRowHeight( j, nT0 );
                                    if( n )
                                    {
                                        y += n / 2;
                                        break;  // for
                                    }
                                }
                            }
                        }
                        // Das ganze von Twips nach 1/100 mm
                        x = (long) ((double) x * HMM_PER_TWIPS);
                        y = (long) ((double) y * HMM_PER_TWIPS);
                        if ( pScDoc->IsNegativePage( nT0 ) )
                            x = -x;
                        aStart = Point( x, y );
                    }
                    else
                        aStart = pViewSh->GetInsertPos();

                    // if not from marked rectangle, move position to left edge
                    if ( pScDoc->IsNegativePage( nT0 ) )
                        aStart.X() -= aSize.Width();
                }

                Rectangle aRect (aStart, aSize);
                SdrOle2Obj* pObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xObj, nAspect ), aName, aRect);

                // Dieses Objekt nicht vor dem Aktivieren zeichnen
                // (in MarkListHasChanged kommt ein Update)
                pSkipPaintObj = pObj;

                SdrPageView* pPV = pView->GetSdrPageView();
                pView->InsertObjectAtView(pObj, *pPV);

                // Dies veranlaesst Chart zum sofortigen Update
                //SvData aEmpty;

                //aIPObj->SendDataChanged( aEmpty );

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


