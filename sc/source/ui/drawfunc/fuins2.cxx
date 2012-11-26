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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//------------------------------------------------------------------------

#include <toolkit/helper/vclunohelper.hxx>
#include <sot/exchange.hxx>
#include <svl/globalnameitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <svl/stritem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/pfiledlg.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/insdlg.hxx>
#include <svtools/soerr.hxx>
#include <svx/svxdlg.hxx>
#include <sot/clsids.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdlegacy.hxx>
#include <sfx2/msgpool.hxx>
#include <scmod.hxx>

// BM/IHA --
#include <cppuhelper/component_context.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <cppuhelper/bootstrap.hxx>

using namespace ::com::sun::star;
// BM/IHA --

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
#include "drawview.hxx"

extern SdrObject* pSkipPaintObj;            // output.cxx - dieses Objekt nicht zeichnen

//------------------------------------------------------------------------

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), sal_True, ppItem ) == SFX_ITEM_SET)

void lcl_ChartInit( const uno::Reference < embed::XEmbeddedObject >& xObj, ScViewData* pViewData,
                    const rtl::OUString& rRangeParam )
{
    ScDocShell* pDocShell = pViewData->GetDocShell();
    ScDocument* pScDoc = pDocShell->GetDocument();

    rtl::OUString aRangeString( rRangeParam );
    if ( !aRangeString.getLength() )
    {
        SCCOL nCol1 = 0;
        SCROW nRow1 = 0;
        SCTAB nTab1 = 0;
        SCCOL nCol2 = 0;
        SCROW nRow2 = 0;
        SCTAB nTab2 = 0;

        ScMarkData& rMark = pViewData->GetMarkData();
        if ( !rMark.IsMarked() )
            pViewData->GetView()->MarkDataArea( sal_True );

        if ( pViewData->GetSimpleArea( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 ) == SC_MARK_SIMPLE )
        {
            PutInOrder( nCol1, nCol2 );
            PutInOrder( nRow1, nRow2 );
            if ( nCol2>nCol1 || nRow2>nRow1 )
            {
                ScDocument* pDoc = pViewData->GetDocument();
                pDoc->LimitChartArea( nTab1, nCol1,nRow1, nCol2,nRow2 );

                String aStr;
                ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                aRange.Format( aStr, SCR_ABS_3D, pScDoc );
                aRangeString = aStr;
            }
        }
    }

    if ( aRangeString.getLength() )
    {
        // connect to Calc data (if no range string, leave chart alone, with its own data)

        uno::Reference< ::com::sun::star::chart2::data::XDataReceiver > xReceiver;
        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is())
            xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
        OSL_ASSERT( xReceiver.is());
        if( xReceiver.is() )
        {
            uno::Reference< chart2::data::XDataProvider > xDataProvider = new ScChart2DataProvider( pScDoc );
            xReceiver->attachDataProvider( xDataProvider );

            uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( pDocShell->GetModel(), uno::UNO_QUERY );
            xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

            // Same behavior as with old chart: Always assume data series in columns
            chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
            bool bHasCategories = false;
            bool bFirstCellAsLabel = false;

            // use ScChartPositioner to auto-detect column/row headers (like ScChartArray in old version)
            ScRangeListRef aRangeListRef( new ScRangeList );
            aRangeListRef->Parse( aRangeString, pScDoc );
            if ( aRangeListRef->Count() )
            {
                pScDoc->LimitChartIfAll( aRangeListRef );               // limit whole columns/rows to used area

                // update string from modified ranges.  The ranges must be in the current formula syntax.
                String aTmpStr;
                aRangeListRef->Format( aTmpStr, SCR_ABS_3D, pScDoc, pScDoc->GetAddressConvention() );
                aRangeString = aTmpStr;

                ScChartPositioner aChartPositioner( pScDoc, aRangeListRef );
                const ScChartPositionMap* pPositionMap( aChartPositioner.GetPositionMap() );
                if( pPositionMap )
                {
                    SCSIZE nRowCount = pPositionMap->GetRowCount();
                    if( 1==nRowCount )
                        eDataRowSource = chart::ChartDataRowSource_ROWS;
                }
                if ( eDataRowSource == chart::ChartDataRowSource_COLUMNS )
                {
                    bHasCategories = aChartPositioner.HasRowHeaders();
                    bFirstCellAsLabel = aChartPositioner.HasColHeaders();
                }
                else    // in case the default is changed
                {
                    bHasCategories = aChartPositioner.HasColHeaders();
                    bFirstCellAsLabel = aChartPositioner.HasRowHeaders();
                }
            }

            uno::Sequence< beans::PropertyValue > aArgs( 4 );
            aArgs[0] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
                uno::makeAny( aRangeString ), beans::PropertyState_DIRECT_VALUE );
            aArgs[1] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("HasCategories"), -1,
                uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
            aArgs[2] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
                uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
            aArgs[3] = beans::PropertyValue(
                ::rtl::OUString::createFromAscii("DataRowSource"), -1,
                uno::makeAny( eDataRowSource ), beans::PropertyState_DIRECT_VALUE );
            xReceiver->setArguments( aArgs );

            // don't create chart listener here (range may be modified in chart dialog)
        }
    }
}

/*************************************************************************
|*
|* FuInsertOLE::Konstruktor
|*
\************************************************************************/

FuInsertOLE::FuInsertOLE(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
           SdrModel* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    if( ! rReq.IsAPI() )
        rReq.Done();

    //! hier DLL's initalisieren, damit die Factories existieren?

    uno::Reference < embed::XEmbeddedObject > xObj;
    uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    sal_Bool bIsFromFile = sal_False;
    ::rtl::OUString aName;

    sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
    ::rtl::OUString aIconMediaType;
    uno::Reference< io::XInputStream > xIconMetaFile;


    sal_uInt16 nSlot = rReq.GetSlot();
    SFX_REQUEST_ARG( rReq, pNameItem, SfxGlobalNameItem, SID_INSERT_OBJECT );
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
            case SID_INSERT_FLOATINGFRAME :
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                SfxAbstractInsertObjectDialog* pDlg =
                        pFact->CreateInsertObjectDialog( pViewShell->GetWindow(), SC_MOD()->GetSlotPool()->GetSlot(nSlot)->GetCommandString(),
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
                        ::rtl::OUString aObjName;
                        SvGlobalName aClassId( SO3_PLUGIN_CLASSID );
                        comphelper::EmbeddedObjectContainer aCnt( xStorage );
                        xObj = aCnt.CreateEmbeddedObject( aClassId.GetByteSequence(), aObjName );
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
            basegfx::B2DVector aScale;
            MapUnit aMapUnit(MAP_100TH_MM);

            if ( nAspect == embed::Aspects::MSOLE_ICON )
            {
                aObjRef.SetGraphicStream( xIconMetaFile, aIconMediaType );
                MapMode aMap100( MAP_100TH_MM );
                const Size aSize(aObjRef.GetSize( &aMap100 ));
                aScale = basegfx::B2DVector(aSize.Width(), aSize.Height());
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

                aScale = basegfx::B2DVector( aSz.Width, aSz.Height );
                aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

                if(basegfx::fTools::equalZero(aScale.getY()) || basegfx::fTools::equalZero(aScale.getX()))
                {
                    // Rechteck mit ausgewogenem Kantenverhaeltnis
                    aScale = basegfx::B2DVector(5000.0, 5000.0);

                    basegfx::B2DVector aTmp(aScale * OutputDevice::GetFactorLogicToLogic(MAP_100TH_MM, aMapUnit));
                    aSz.Width = basegfx::fround(aTmp.getX());
                    aSz.Height = basegfx::fround(aTmp.getY());
                    xObj->setVisualAreaSize( nAspect, aSz );
                }

                    //  re-convert aSize to 1/100th mm to avoid rounding errors in comparison below
                aScale *= Window::GetFactorLogicToLogic(aMapUnit, MAP_100TH_MM);
            }

            //  Chart initialisieren ?
            if ( SvtModuleOptions().IsChart() && SotExchange::IsChart( SvGlobalName( xObj->getClassID() ) ) )
                lcl_ChartInit( xObj, pViewSh->GetViewData(), rtl::OUString() );

            ScViewData* pData = pViewSh->GetViewData();

            basegfx::B2DPoint aPnt(pViewSh->GetInsertPos());
            if ( pData->GetDocument()->IsNegativePage( pData->GetTabNo() ) )
                aPnt.setX(aPnt.getX() - aScale.getX());     // move position to left edge

            SdrOle2Obj* pObj = new SdrOle2Obj(
                pView->getSdrModelFromSdrView(),
                aObjRef,
                aName,
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aScale,
                    aPnt));

                // Dieses Objekt nicht vor dem Aktivieren zeichnen
                // (in MarkListHasChanged kommt ein Update)
            if (!bIsFromFile)
                pSkipPaintObj = pObj;

            pView->InsertObjectAtView(*pObj);

            if ( nAspect != embed::Aspects::MSOLE_ICON )
            {
                //  #73279# Math objects change their object size during InsertObjectToSdrObjList.
                //  New size must be set in SdrObject, or a wrong scale will be set at
                //  ActivateObject.

                try
                {
                    const awt::Size aSz(xObj->getVisualAreaSize( nAspect ));
                    const basegfx::B2DVector aNewSize(basegfx::B2DVector(aSz.Width, aSz.Height) * OutputDevice::GetFactorLogicToLogic(aMapUnit, MAP_100TH_MM));

                    if ( !aNewSize.equal(aScale) )
                    {
                        pObj->setSdrObjectTransformation(
                            basegfx::tools::createScaleTranslateB2DHomMatrix(
                                aNewSize,
                                aPnt));
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
                    pViewShell->SetDrawShell( sal_True );
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

FuInsertChart::FuInsertChart(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
           SdrModel* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();

    if( ! rReq.IsAPI() )
        rReq.Done();

    if( SvtModuleOptions().IsChart() )
    {
        // ----------------------------------------
        // BM/IHA --

        // get range
        ::rtl::OUString aRangeString;
        ScRange aPositionRange;             // cell range for chart positioning
        if( pReqArgs )
        {
            const SfxPoolItem* pItem;
            if( IS_AVAILABLE( FN_PARAM_5, &pItem ) )
            aRangeString = ::rtl::OUString( ((const SfxStringItem*)pItem)->GetValue());

            aPositionRange = pViewSh->GetViewData()->GetCurPos();
        }
        else
        {
            ScMarkData& rMark = pViewSh->GetViewData()->GetMarkData();
            bool bAutomaticMark = false;
            if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
            {
                pViewSh->GetViewData()->GetView()->MarkDataArea( sal_True );
                bAutomaticMark = true;
            }

            ScMarkData aMultiMark( rMark );
            aMultiMark.MarkToMulti();

            ScRangeList aRanges;
            aMultiMark.FillRangeListWithMarks( &aRanges, sal_False );
            String aStr;
            ScDocument* pDocument = pViewSh->GetViewData()->GetDocument();
            aRanges.Format( aStr, SCR_ABS_3D, pDocument, pDocument->GetAddressConvention() );
            aRangeString = aStr;

            // get "total" range for positioning
            sal_uLong nCount = aRanges.Count();
            if ( nCount > 0 )
            {
                aPositionRange = *aRanges.GetObject(0);
                for (sal_uLong i=1; i<nCount; i++)
                    aPositionRange.ExtendTo( *aRanges.GetObject(i) );
            }

            if(bAutomaticMark)
                pViewSh->GetViewData()->GetView()->Unmark();
        }

        // ----------------------------------------
        // adapted old code
        pView->UnmarkAll();

        ::rtl::OUString aName;
        const sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

        uno::Reference < embed::XEmbeddedObject > xObj =
            pViewShell->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID_60 ).GetByteSequence(), aName );

        uno::Reference< ::com::sun::star::chart2::data::XDataReceiver > xReceiver;
        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is())
            xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );

        // lock the model to suppress any internal updates
        uno::Reference< frame::XModel > xChartModel( xReceiver, uno::UNO_QUERY );
        if( xChartModel.is() )
            xChartModel->lockControllers();

        ScRangeListRef aDummy;
        basegfx::B2DRange aMarkDest;
        SCTAB nMarkTab;
        sal_Bool bDrawRect = pViewShell->GetChartArea( aDummy, aMarkDest, nMarkTab );

        //  Objekt-Groesse
        awt::Size aSz = xObj->getVisualAreaSize( nAspect );
        basegfx::B2DVector aScale( aSz.Width, aSz.Height );

        MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

        sal_Bool bSizeCh = sal_False;
        if (bDrawRect && !aMarkDest.isEmpty())
        {
            aScale = aMarkDest.getRange();
            bSizeCh = sal_True;
        }
        if (aScale.getY() <= 0.0 || aScale.getX() <= 0.0)
        {
            aScale = basegfx::B2DVector(5000.0, 5000.0);
            bSizeCh = sal_True;
        }
        if (bSizeCh)
        {
            aScale = Window::GetFactorLogicToLogic(MAP_100TH_MM, aMapUnit) * aScale;
            aSz.Width = basegfx::fround(aScale.getX());
            aSz.Height = basegfx::fround(aScale.getY());
            xObj->setVisualAreaSize( nAspect, aSz );
        }

        ScViewData* pData = pViewSh->GetViewData();
        ScDocShell* pScDocSh = pData->GetDocShell();
        ScDocument* pScDoc   = pScDocSh->GetDocument();
        sal_Bool bUndo (pScDoc->IsUndoEnabled());

        if( pReqArgs )
        {
            const SfxPoolItem* pItem;
            sal_uInt16 nToTable = 0;

            if( IS_AVAILABLE( FN_PARAM_4, &pItem ) )
            {
                if ( dynamic_cast< const SfxUInt16Item* >(pItem) )
                {
                    nToTable = ((const SfxUInt16Item*)pItem)->GetValue();
                }
                else if ( dynamic_cast< const SfxBoolItem* >(pItem) )
                {
                    //  #46033# in der idl fuer Basic steht FN_PARAM_4 als SfxBoolItem
                    //  -> wenn gesetzt, neue Tabelle, sonst aktuelle Tabelle

                    if ( ((const SfxBoolItem*)pItem)->GetValue() )
                        nToTable = static_cast<sal_uInt16>(pScDoc->GetTableCount());
                    else
                        nToTable = static_cast<sal_uInt16>(pData->GetTabNo());
                }
            }
            else
            {
                if (bDrawRect)
                    nToTable = static_cast<sal_uInt16>(nMarkTab);
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
                    sal_Bool bAppend = sal_True;

                    if (bUndo)
                    {
                        pScDocSh->GetUndoManager()->AddUndoAction(
                            new ScUndoInsertTab( pScDocSh, nNewTab,
                                                 bAppend, aTabName ) );
                    }

                    pScDocSh->Broadcast( ScTablesHint( SC_TAB_INSERTED, nNewTab ) );
                    pViewSh->SetTabNo( nNewTab, sal_True );
                    pScDocSh->PostPaintExtras();            //! erst hinterher ???
                }
                else
                {
                    DBG_ERROR( "Could not create new table :-/" );
                }
            }
            else if ( nToTable != pData->GetTabNo() )
            {
                pViewSh->SetTabNo( nToTable, sal_True );
            }
        }

        lcl_ChartInit( xObj, pData, aRangeString );         // set source range, auto-detect column/row headers

        //  Objekt-Position

        basegfx::B2DPoint aStart(0.0, 0.0);
        if ( bDrawRect )
        {
            aStart = aMarkDest.getMinimum(); // marked by hand
        }
        else
        {
            // get chart position (from window size and data range)
            aStart = pViewSh->GetChartInsertPos( aScale, aPositionRange );
        }

        SdrOle2Obj* pObj = new SdrOle2Obj(
            pView->getSdrModelFromSdrView(),
            svt::EmbeddedObjectRef( xObj, nAspect ),
            aName,
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aScale,
                aStart));

        // Dieses Objekt nicht vor dem Aktivieren zeichnen
        pSkipPaintObj = pObj;

        SdrPageView* pPV = pView->GetSdrPageView();

        if(pPV)
        {
            // use the page instead of the view to insert, so no undo action is created yet
            SdrPage& rInsPage = pPV->getSdrPageFromSdrPageView();
            rInsPage.InsertObjectToSdrObjList(*pObj);
            pView->UnmarkAllObj();
            pView->MarkObj( *pObj );
            bool bAddUndo = true;               // add undo action later, unless the dialog is canceled

            if (rReq.IsAPI())
            {
                if( xChartModel.is() )
                    xChartModel->unlockControllers();
            }
            else
            {
                //the controller will be unlocked by the dialog when the dialog is told to do so

                // only activate object if not called via API (e.g. macro)
                pViewShell->ActivateObject( (SdrOle2Obj*) pObj, SVVERB_SHOW );

                //open wizard
                //@todo get context from calc if that has one
                uno::Reference< uno::XComponentContext > xContext(
                    ::cppu::defaultBootstrap_InitialComponentContext() );
                if(xContext.is())
                {
                    uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
                    if(xMCF.is())
                    {
                        uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
                            xMCF->createInstanceWithContext(
                                rtl::OUString::createFromAscii("com.sun.star.comp.chart2.WizardDialog")
                                , xContext), uno::UNO_QUERY);
                        uno::Reference< lang::XInitialization > xInit( xDialog, uno::UNO_QUERY );
                        if( xChartModel.is() && xInit.is() )
                        {
                            uno::Reference< awt::XWindow > xDialogParentWindow(0);
                            //  initialize dialog
                            uno::Sequence<uno::Any> aSeq(2);
                            uno::Any* pArray = aSeq.getArray();
                            beans::PropertyValue aParam1;
                            aParam1.Name = rtl::OUString::createFromAscii("ParentWindow");
                            aParam1.Value <<= uno::makeAny(xDialogParentWindow);
                            beans::PropertyValue aParam2;
                            aParam2.Name = rtl::OUString::createFromAscii("ChartModel");
                            aParam2.Value <<= uno::makeAny(xChartModel);
                            pArray[0] <<= uno::makeAny(aParam1);
                            pArray[1] <<= uno::makeAny(aParam2);
                            xInit->initialize( aSeq );

                            // try to set the dialog's position so it doesn't hide the chart
                            uno::Reference < beans::XPropertySet > xDialogProps( xDialog, uno::UNO_QUERY );
                            if ( xDialogProps.is() )
                            {
                                try
                                {
                                    //get dialog size:
                                    awt::Size aDialogAWTSize;
                                    if( xDialogProps->getPropertyValue( ::rtl::OUString::createFromAscii("Size") )
                                        >>= aDialogAWTSize )
                                    {
                                        const basegfx::B2DVector aDialogScale(aDialogAWTSize.Width, aDialogAWTSize.Height);

                                        if( aDialogScale.getX() > 0 && aDialogScale.getY() > 0 )
                                        {
                                            //calculate and set new position
                                            const basegfx::B2DRange aOldObjRange(sdr::legacy::GetLogicRange(*pObj));
                                            const basegfx::B2DPoint aDialogPos(pViewShell->GetChartDialogPos(aDialogScale, aOldObjRange));

                                            xDialogProps->setPropertyValue( ::rtl::OUString::createFromAscii("Position"),
                                                    uno::makeAny(
                                                        awt::Point(
                                                            basegfx::fround(aDialogPos.getX()),
                                                            basegfx::fround(aDialogPos.getY())) ) );
                                        }
                                    }
                                    //tell the dialog to unlock controller
                                    xDialogProps->setPropertyValue( ::rtl::OUString::createFromAscii("UnlockControllersOnExecute"),
                                                uno::makeAny( sal_True ) );

                                }
                                catch( uno::Exception& )
                                {
                                    OSL_ASSERT( "Chart wizard couldn't be positioned automatically\n" );
                                }
                            }

                            sal_Int16 nDialogRet = xDialog->execute();
                            if( nDialogRet == ui::dialogs::ExecutableDialogResults::CANCEL )
                            {
                                // leave OLE inplace mode and unmark
                                OSL_ASSERT( pViewShell );
                                OSL_ASSERT( pView );
                                pViewShell->DeactivateOle();
                                pView->UnmarkAll();

                                // old page view pointer is invalid after switching sheets
                                pPV = pView->GetSdrPageView();

                                // remove the chart
                                OSL_ASSERT( pPV );
                                    SdrPage& rPage = pPV->getSdrPageFromSdrPageView();
                                OSL_ASSERT( pObj );
                                    rPage.RemoveObjectFromSdrObjList(pObj->GetNavigationPosition());

                                bAddUndo = false;       // don't create the undo action for inserting

                                // leave the draw shell
                                pViewShell->SetDrawShell( sal_False );
                            }
                            else
                            {
                                OSL_ASSERT( nDialogRet == ui::dialogs::ExecutableDialogResults::OK );
                                //@todo maybe move chart to different table
                            }
                        }
                        uno::Reference< lang::XComponent > xComponent( xDialog, uno::UNO_QUERY );
                        if( xComponent.is())
                            xComponent->dispose();
                    }
                }
            }

            if ( bAddUndo )
            {
                // add undo action the same way as in SdrEditView::InsertObjectAtView
                // (using UndoActionHdl etc.)
                pView->AddUndo(pDoc->GetSdrUndoFactory().CreateUndoNewObject(*pObj));
            }
        }

        // BM/IHA --
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


