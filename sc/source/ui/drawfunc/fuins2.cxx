/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <sot/exchange.hxx>
#include <svl/globalnameitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svx/svdoole2.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/insdlg.hxx>
#include <svtools/embedhlp.hxx>
#include <svx/svxdlg.hxx>
#include <comphelper/classids.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/msgpool.hxx>
#include <scmod.hxx>
#include <sal/log.hxx>

#include <comphelper/storagehelper.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <cppuhelper/bootstrap.hxx>

#include <PivotTableDataProvider.hxx>
#include <chart2uno.hxx>
#include <fuinsert.hxx>
#include <tabvwsh.hxx>
#include <sc.hrc>
#include <chartpos.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <undotab.hxx>
#include <uiitems.hxx>
#include <drawview.hxx>
#include <markdata.hxx>
#include <dpobject.hxx>
#include <memory>

using namespace css;

namespace
{

void lcl_ChartInit(const uno::Reference <embed::XEmbeddedObject>& xObj, ScViewData* pViewData,
                   const OUString& rRangeParam, bool bRangeIsPivotTable)
{
    ScDocShell* pDocShell = pViewData->GetDocShell();
    ScDocument& rScDoc = pDocShell->GetDocument();

    OUString aRangeString(rRangeParam);

    if (aRangeString.isEmpty() && !bRangeIsPivotTable)
    {
        SCCOL nCol1 = 0;
        SCROW nRow1 = 0;
        SCTAB nTab1 = 0;
        SCCOL nCol2 = 0;
        SCROW nRow2 = 0;
        SCTAB nTab2 = 0;

        ScMarkData& rMark = pViewData->GetMarkData();
        if ( !rMark.IsMarked() )
            pViewData->GetView()->MarkDataArea();

        if ( pViewData->GetSimpleArea( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 ) == SC_MARK_SIMPLE )
        {
            PutInOrder( nCol1, nCol2 );
            PutInOrder( nRow1, nRow2 );
            if (nCol2 >= nCol1 || nRow2 >= nRow1)
            {
                ScDocument* pDoc = pViewData->GetDocument();
                pDoc->LimitChartArea( nTab1, nCol1,nRow1, nCol2,nRow2 );

                ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                aRangeString = aRange.Format(ScRefFlags::RANGE_ABS_3D, &rScDoc, rScDoc.GetAddressConvention());
            }
        }
    }

    if (!aRangeString.isEmpty())
    {
        // connect to Calc data (if no range string, leave chart alone, with its own data)

        uno::Reference< css::chart2::data::XDataReceiver > xReceiver;
        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is())
            xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
        OSL_ASSERT( xReceiver.is());
        if( xReceiver.is() )
        {
            uno::Reference<chart2::data::XDataProvider> xDataProvider;
            if (bRangeIsPivotTable)
            {
                std::unique_ptr<sc::PivotTableDataProvider> pPivotTableDataProvider(new sc::PivotTableDataProvider(&rScDoc));
                pPivotTableDataProvider->setPivotTableName(aRangeString);
                xDataProvider.set(pPivotTableDataProvider.release());
            }
            else
            {
                xDataProvider.set(new ScChart2DataProvider(&rScDoc));
            }

            xReceiver->attachDataProvider(xDataProvider);

            uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( pDocShell->GetModel(), uno::UNO_QUERY );
            xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

            // Same behavior as with old chart: Always assume data series in columns
            chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
            bool bHasCategories = false;
            bool bFirstCellAsLabel = false;

            // use ScChartPositioner to auto-detect column/row headers (like ScChartArray in old version)
            ScRangeListRef aRangeListRef( new ScRangeList );
            aRangeListRef->Parse( aRangeString, &rScDoc, rScDoc.GetAddressConvention() );
            if ( !aRangeListRef->empty() )
            {
                rScDoc.LimitChartIfAll( aRangeListRef );               // limit whole columns/rows to used area

                // update string from modified ranges.  The ranges must be in the current formula syntax.
                OUString aTmpStr;
                aRangeListRef->Format( aTmpStr, ScRefFlags::RANGE_ABS_3D, &rScDoc, rScDoc.GetAddressConvention() );
                aRangeString = aTmpStr;

                ScChartPositioner aChartPositioner( &rScDoc, aRangeListRef );
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
                "CellRangeRepresentation", -1,
                uno::makeAny( aRangeString ), beans::PropertyState_DIRECT_VALUE );
            aArgs[1] = beans::PropertyValue(
                "HasCategories", -1,
                uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
            aArgs[2] = beans::PropertyValue(
                "FirstCellAsLabel", -1,
                uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
            aArgs[3] = beans::PropertyValue(
                "DataRowSource", -1,
                uno::makeAny( eDataRowSource ), beans::PropertyState_DIRECT_VALUE );

            try
            {
                xReceiver->setArguments( aArgs );
            }
            catch (const lang::IllegalArgumentException& e)
            {
                // Can happen for invalid aRangeString, in which case a Chart
                // will be created nevertheless and the range string can be
                // edited.
                SAL_WARN("sc.ui",
                        "lcl_ChartInit - caught IllegalArgumentException with message \"" << e << "\","
                        " might be due to aRangeString: " << aRangeString);
            }

            // don't create chart listener here (range may be modified in chart dialog)
        }
    }
}

}

FuInsertOLE::FuInsertOLE(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
           SdrModel* pDoc, SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pViewP, pDoc, rReq)
{
    if( ! rReq.IsAPI() )
        rReq.Done();

    //! initialize DLLs here, so that the factories exist?

    uno::Reference < embed::XEmbeddedObject > xObj;
    uno::Reference < embed::XStorage > xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
    bool bIsFromFile = false;
    OUString aName;

    sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
    OUString aIconMediaType;
    uno::Reference< io::XInputStream > xIconMetaFile;

    const sal_uInt16 nSlot = rReq.GetSlot();
    const SfxGlobalNameItem* pNameItem = rReq.GetArg<SfxGlobalNameItem>(SID_INSERT_OBJECT);
    if ( nSlot == SID_INSERT_OBJECT && pNameItem )
    {
        const SvGlobalName& aClassName = pNameItem->GetValue();
        xObj = rViewShell.GetViewFrame()->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( aClassName.GetByteSequence(), aName );
    }
    else if ( nSlot == SID_INSERT_SMATH )
    {
        if ( SvtModuleOptions().IsMath() )
        {
            xObj = rViewShell.GetViewFrame()->GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SM_CLASSID_60 ).GetByteSequence(), aName );
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
                aServerLst.Remove( ScDocShell::Factory().GetClassId() );   // Do not show Starcalc
                //TODO/LATER: currently no inserting of ClassId into SfxRequest!
                [[fallthrough]]; //TODO ???
            case SID_INSERT_FLOATINGFRAME :
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<SfxAbstractInsertObjectDialog> pDlg(
                        pFact->CreateInsertObjectDialog( rViewShell.GetFrameWeld(), SC_MOD()->GetSlotPool()->GetSlot(nSlot)->GetCommandString(),
                        xStorage, &aServerLst ));
                if ( pDlg )
                {
                    pDlg->Execute();
                    xObj = pDlg->GetObject();

                    xIconMetaFile = pDlg->GetIconIfIconified( &aIconMediaType );
                    if ( xIconMetaFile.is() )
                        nAspect = embed::Aspects::MSOLE_ICON;

                    if ( xObj.is() )
                        rViewSh.GetObjectShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );
                    // to activate DrawShell (no need to activate Object)
                    bIsFromFile = !pDlg->IsCreateNew();
                }

                break;
            }
        }
    }

    //  SvInsertObjectDialog (everything in one Dialog) are not used anymore
    if (xObj.is())
    {
        pView->UnmarkAll();

        try
        {
            ::svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
            Size aSize;
            MapMode aMap100( MapUnit::Map100thMM );
            MapUnit aMapUnit = MapUnit::Map100thMM;

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
                    // rectangle with balanced edge ratio
                    aSize.setWidth( 5000 );
                    aSize.setHeight( 5000 );
                    Size aTmp = OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
                    aSz.Width = aTmp.Width();
                    aSz.Height = aTmp.Height();
                    xObj->setVisualAreaSize( nAspect, aSz );

                    //  re-convert aSize to 1/100th mm to avoid rounding errors in comparison below
                    aSize = vcl::Window::LogicToLogic( aTmp,
                                    MapMode( aMapUnit ), aMap100 );
                }
                else
                    aSize = vcl::Window::LogicToLogic( aSize,
                                    MapMode( aMapUnit ), aMap100 );
            }

            //  initialize chart ?
            if ( SvtModuleOptions().IsChart() && SotExchange::IsChart( SvGlobalName( xObj->getClassID() ) ) )
                lcl_ChartInit(xObj, &rViewSh.GetViewData(), OUString(), false);

            ScViewData& rData = rViewSh.GetViewData();

            Point aPnt = rViewSh.GetInsertPos();
            if ( rData.GetDocument()->IsNegativePage( rData.GetTabNo() ) )
                aPnt.AdjustX( -(aSize.Width()) );      // move position to left edge
            tools::Rectangle aRect (aPnt, aSize);
            SdrOle2Obj* pObj = new SdrOle2Obj(
                *pDoc, // TTTT should be reference
                aObjRef,
                aName,
                aRect);
            SdrPageView* pPV = pView->GetSdrPageView();
            pView->InsertObjectAtView(pObj, *pPV);

            if ( nAspect != embed::Aspects::MSOLE_ICON )
            {
                //  Math objects change their object size during InsertObject.
                //  New size must be set in SdrObject, or a wrong scale will be set at
                //  ActivateObject.

                try
                {
                    awt::Size aSz = xObj->getVisualAreaSize( nAspect );

                    Size aNewSize( aSz.Width, aSz.Height );
                    aNewSize = OutputDevice::LogicToLogic(aNewSize, MapMode(aMapUnit), MapMode(MapUnit::Map100thMM));

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
                // XXX Activate from macro is deadly !!! ???
                if (bIsFromFile)
                {
                    // Object selected, activate Draw-Shell
                    rViewShell.SetDrawShell( true );
                }
                else
                {
                    rViewShell.ActivateObject(pObj, embed::EmbedVerbs::MS_OLEVERB_SHOW);
                }
            }

            rReq.Done();
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "May need error handling here!" );
        }
    }
    else
        rReq.Ignore();
}

FuInsertChart::FuInsertChart(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
           SdrModel* pDoc, SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pViewP, pDoc, rReq)
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    if( ! rReq.IsAPI() )
        rReq.Done();

    if (!SvtModuleOptions().IsChart())
        return;

    // BM/IHA --

    // get range
    OUString aRangeString;
    bool bRangeIsPivotTable = false;
    ScRange aPositionRange;             // cell range for chart positioning
    ScMarkData aMark = rViewSh.GetViewData().GetMarkData();
    if( pReqArgs )
    {
        const SfxPoolItem* pItem;
        if( pReqArgs->HasItem( FN_PARAM_5, &pItem ) )
            aRangeString = static_cast<const SfxStringItem*>(pItem)->GetValue();

        aPositionRange = rViewSh.GetViewData().GetCurPos();
    }
    else
    {
        ScDocument* pDocument = rViewSh.GetViewData().GetDocument();
        ScDPObject* pObject = pDocument->GetDPAtCursor(rViewSh.GetViewData().GetCurX(),
                                                       rViewSh.GetViewData().GetCurY(),
                                                       rViewSh.GetViewData().GetTabNo());
        if (pObject)
        {
            aRangeString = pObject->GetName();
            bRangeIsPivotTable = true;
        }
        else
        {
            bool bAutomaticMark = false;
            if ( !aMark.IsMarked() && !aMark.IsMultiMarked() )
            {
                rViewSh.GetViewData().GetView()->MarkDataArea();
                bAutomaticMark = true;
            }

            ScMarkData aMultiMark( aMark );
            aMultiMark.MarkToMulti();

            ScRangeList aRanges;
            aMultiMark.FillRangeListWithMarks( &aRanges, false );
            OUString aStr;
            aRanges.Format( aStr, ScRefFlags::RANGE_ABS_3D, pDocument, pDocument->GetAddressConvention() );
            aRangeString = aStr;

            // get "total" range for positioning
            if ( !aRanges.empty() )
            {
                aPositionRange = aRanges[ 0 ];
                for ( size_t i = 1, nCount = aRanges.size(); i < nCount; ++i )
                {
                    aPositionRange.ExtendTo( aRanges[ i ] );
                }
            }

            if(bAutomaticMark)
                rViewSh.GetViewData().GetView()->Unmark();
        }
    }

    // adapted old code
    pView->UnmarkAll();

    OUString aName;
    const sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

    uno::Reference < embed::XEmbeddedObject > xObj =
        rViewShell.GetObjectShell()->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID_60 ).GetByteSequence(), aName );

    uno::Reference< css::chart2::data::XDataReceiver > xReceiver;
    uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
    if( xCompSupp.is())
        xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );

    uno::Reference<chart2::XChartDocument> xChartDoc(xReceiver, uno::UNO_QUERY);
    if (xChartDoc.is())
        xChartDoc->createDefaultChart();

    // lock the model to suppress any internal updates
    uno::Reference< frame::XModel > xChartModel( xReceiver, uno::UNO_QUERY );
    if( xChartModel.is() )
        xChartModel->lockControllers();

    // object size
    awt::Size aSz = xObj->getVisualAreaSize( nAspect );
    Size aSize( aSz.Width, aSz.Height );

    MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );

    bool bSizeCh = false;
    if (aSize.Height() <= 0 || aSize.Width() <= 0)
    {
        aSize.setWidth( 5000 );
        aSize.setHeight( 5000 );
        bSizeCh = true;
    }
    if (bSizeCh)
    {
        aSize = vcl::Window::LogicToLogic( aSize, MapMode( MapUnit::Map100thMM ), MapMode( aMapUnit ) );
        aSz.Width = aSize.Width();
        aSz.Height = aSize.Height();
        xObj->setVisualAreaSize( nAspect, aSz );
    }

    ScViewData& rData = rViewSh.GetViewData();
    ScDocShell* pScDocSh = rData.GetDocShell();
    ScDocument& rScDoc   = pScDocSh->GetDocument();
    bool bUndo (rScDoc.IsUndoEnabled());

    if( pReqArgs )
    {
        const SfxPoolItem* pItem;
        sal_uInt16 nToTable = 0;

        if( pReqArgs->HasItem( FN_PARAM_4, &pItem ) )
        {
            if ( auto pUInt16Item = dynamic_cast<const SfxUInt16Item*>( pItem) )
                nToTable = pUInt16Item->GetValue();
            else if ( auto pBoolItem = dynamic_cast<const SfxBoolItem*>( pItem) )
            {
                //  In IDL for Basic FN_PARAM_4 means SfxBoolItem
                //  -> if set new table, else current table

                if ( pBoolItem->GetValue() )
                    nToTable = static_cast<sal_uInt16>(rScDoc.GetTableCount());
                else
                    nToTable = static_cast<sal_uInt16>(rData.GetTabNo());
            }
        }
        else
        {
            rReq.AppendItem( SfxUInt16Item( FN_PARAM_4, nToTable ) );
        }

        // Output on new table?
        if ( nToTable == rScDoc.GetTableCount() )
        {
            // Let's go...
            OUString      aTabName;
            SCTAB       nNewTab = rScDoc.GetTableCount();

            rScDoc.CreateValidTabName( aTabName );

            if ( rScDoc.InsertTab( nNewTab, aTabName ) )
            {
                if (bUndo)
                {
                    pScDocSh->GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoInsertTab>( pScDocSh, nNewTab,
                                             true/*bAppend*/, aTabName ) );
                }

                pScDocSh->Broadcast( ScTablesHint( SC_TAB_INSERTED, nNewTab ) );
                rViewSh.SetTabNo( nNewTab, true );
                pScDocSh->PostPaintExtras();            //! done afterwards ???
            }
            else
            {
                OSL_FAIL( "Could not create new table :-/" );
            }
        }
        else if ( nToTable != rData.GetTabNo() )
        {
            rViewSh.SetTabNo( nToTable, true );
        }
    }

    lcl_ChartInit(xObj, &rData, aRangeString, bRangeIsPivotTable);         // set source range, auto-detect column/row headers

    //  object position

    // get chart position (from window size and data range)
    Point aStart = rViewSh.GetChartInsertPos( aSize, aPositionRange );

    tools::Rectangle aRect (aStart, aSize);
    SdrOle2Obj* pObj = new SdrOle2Obj(
        *pDoc, // TTTT should be reference
        svt::EmbeddedObjectRef(xObj, nAspect),
        aName,
        aRect);
    SdrPageView* pPV = pView->GetSdrPageView();

    // #i121334# This call will change the chart's default background fill from white to transparent.
    // Add here again if this is wanted (see task description for details)
    // ChartHelper::AdaptDefaultsForChart( xObj );

//        pView->InsertObjectAtView(pObj, *pPV);//this call leads to an immediate redraw and asks the chart for a visual representation

    // use the page instead of the view to insert, so no undo action is created yet
    SdrPage* pInsPage = pPV->GetPage();
    pInsPage->InsertObject( pObj );
    pView->UnmarkAllObj();
    pView->MarkObj( pObj, pPV );
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
        rViewShell.ActivateObject(pObj, embed::EmbedVerbs::MS_OLEVERB_SHOW);

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
                        "com.sun.star.comp.chart2.WizardDialog"
                        , xContext), uno::UNO_QUERY);
                uno::Reference< lang::XInitialization > xInit( xDialog, uno::UNO_QUERY );
                if( xChartModel.is() && xInit.is() )
                {
                    uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
                    {
                        {"ParentWindow", uno::Any(uno::Reference< awt::XWindow >())},
                        {"ChartModel", uno::Any(xChartModel)}
                    }));
                    xInit->initialize( aSeq );

                    // try to set the dialog's position so it doesn't hide the chart
                    uno::Reference < beans::XPropertySet > xDialogProps( xDialog, uno::UNO_QUERY );
                    if ( xDialogProps.is() )
                    {
                        try
                        {
                            //get dialog size:
                            awt::Size aDialogAWTSize;
                            if( xDialogProps->getPropertyValue("Size")
                                >>= aDialogAWTSize )
                            {
                                Size aDialogSize( aDialogAWTSize.Width, aDialogAWTSize.Height );
                                if ( aDialogSize.Width() > 0 && aDialogSize.Height() > 0 )
                                {
                                    //calculate and set new position
                                    Point aDialogPos = rViewShell.GetChartDialogPos( aDialogSize, aRect );
                                    xDialogProps->setPropertyValue("Position",
                                        uno::makeAny( awt::Point(aDialogPos.getX(),aDialogPos.getY()) ) );
                                }
                            }
                            //tell the dialog to unlock controller
                            xDialogProps->setPropertyValue("UnlockControllersOnExecute",
                                        uno::makeAny( true ) );

                        }
                        catch( uno::Exception& )
                        {
                            OSL_FAIL( "Chart wizard couldn't be positioned automatically" );
                        }
                    }

                    sal_Int16 nDialogRet = xDialog->execute();
                    if( nDialogRet == ui::dialogs::ExecutableDialogResults::CANCEL )
                    {
                        // leave OLE inplace mode and unmark
                        OSL_ASSERT( pView );
                        rViewShell.DeactivateOle();
                        pView->UnmarkAll();

                        // old page view pointer is invalid after switching sheets
                        pPV = pView->GetSdrPageView();

                        // remove the chart
                        OSL_ASSERT( pPV );
                        SdrPage * pPage( pPV->GetPage());
                        OSL_ASSERT( pPage );
                        OSL_ASSERT( pObj );
                        if( pPage )
                        {
                            // Remove the OLE2 object from the sdr page.
                            SdrObject* pRemoved = pPage->RemoveObject(pObj->GetOrdNum());
                            OSL_ASSERT(pRemoved == pObj);
                            SdrObject::Free(pRemoved); // Don't forget to free it.
                        }

                        bAddUndo = false;       // don't create the undo action for inserting

                        // leave the draw shell
                        rViewShell.SetDrawShell( false );

                        // reset marked cell area

                        rViewSh.GetViewData().GetViewShell()->SetMarkData(aMark);
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
        pView->AddUndo(std::make_unique<SdrUndoNewObj>(*pObj));
    }

    // BM/IHA --
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
