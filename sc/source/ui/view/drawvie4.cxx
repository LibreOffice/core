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

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sal/log.hxx>

#include <drawview.hxx>
#include <global.hxx>
#include <drwlayer.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <drwtrans.hxx>
#include <transobj.hxx>
#include <drawutil.hxx>
#include <scmod.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <chartarr.hxx>
#include <gridwin.hxx>
#include <userdat.hxx>
#include <tabvwsh.hxx>

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace com::sun::star;

Point aDragStartDiff;

void ScDrawView::BeginDrag( vcl::Window* pWindow, const Point& rStartPos )
{
    if ( AreObjectsMarked() )
    {
        BrkAction();

        tools::Rectangle aMarkedRect = GetAllMarkedRect();
        vcl::Region aRegion( aMarkedRect );

        aDragStartDiff = rStartPos - aMarkedRect.TopLeft();

        bool bAnyOle, bOneOle;
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        CheckOle( rMarkList, bAnyOle, bOneOle );

        ScDocShellRef aDragShellRef;
        if (bAnyOle)
        {
            aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
            aDragShellRef->DoInitNew();
        }
        ScDrawLayer::SetGlobalDrawPersist( aDragShellRef.get() );
        std::unique_ptr<SdrModel> pModel(CreateMarkedObjModel());
        ScDrawLayer::SetGlobalDrawPersist(nullptr);

        //  Charts now always copy their data in addition to the source reference, so
        //  there's no need to call SchDLL::Update for the charts in the clipboard doc.
        //  Update with the data (including NumberFormatter) from the live document would
        //  also store the NumberFormatter in the clipboard chart (#88749#)

        ScDocShell* pDocSh = pViewData->GetDocShell();

        TransferableObjectDescriptor aObjDesc;
        pDocSh->FillTransferableObjectDescriptor( aObjDesc );
        aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        // maSize is set in ScDrawTransferObj ctor

        rtl::Reference<ScDrawTransferObj> pTransferObj = new ScDrawTransferObj( std::move(pModel), pDocSh, aObjDesc );

        pTransferObj->SetDrawPersist( aDragShellRef.get() );    // keep persist for ole objects alive
        pTransferObj->SetDragSource( this );               // copies selection

        SC_MOD()->SetDragObject( nullptr, pTransferObj.get() );     // for internal D&D
        pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
    }
}

namespace {

void getRangeFromDataSource( uno::Reference< chart2::data::XDataSource > const & xDataSource, std::vector<OUString>& rRangeRep)
{
    uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence> > xSeqs = xDataSource->getDataSequences();
    for (sal_Int32 i = 0, n = xSeqs.getLength(); i < n; ++i)
    {
        uno::Reference<chart2::data::XLabeledDataSequence> xLS = xSeqs[i];
        uno::Reference<chart2::data::XDataSequence> xSeq = xLS->getValues();
        if (xSeq.is())
        {
            OUString aRep = xSeq->getSourceRangeRepresentation();
            rRangeRep.push_back(aRep);
        }
        xSeq = xLS->getLabel();
        if (xSeq.is())
        {
            OUString aRep = xSeq->getSourceRangeRepresentation();
            rRangeRep.push_back(aRep);
        }
    }
}

void getRangeFromErrorBar(const uno::Reference< chart2::XChartDocument >& rChartDoc, std::vector<OUString>& rRangeRep)
{
    uno::Reference <chart2::XDiagram > xDiagram = rChartDoc->getFirstDiagram();
    if(!xDiagram.is())
        return;

    uno::Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY);
    if(!xCooSysContainer.is())
        return;

    uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    for(sal_Int32 i = 0; i < xCooSysSequence.getLength(); ++i)
    {
        uno::Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[i], uno::UNO_QUERY);
        if(!xChartTypeContainer.is())
            continue;

        uno::Sequence< uno::Reference< chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
        for(sal_Int32 nChartType = 0; nChartType < xChartTypeSequence.getLength(); ++nChartType)
        {
            uno::Reference< chart2::XDataSeriesContainer > xDataSequenceContainer( xChartTypeSequence[nChartType], uno::UNO_QUERY);
            if(!xDataSequenceContainer.is())
                continue;

            uno::Sequence< uno::Reference< chart2::XDataSeries > > xSeriesSequence( xDataSequenceContainer->getDataSeries() );
            for(sal_Int32 nDataSeries = 0; nDataSeries < xSeriesSequence.getLength(); ++nDataSeries)
            {
                uno::Reference< chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];
                uno::Reference< beans::XPropertySet > xPropSet( xSeries, uno::UNO_QUERY);
                uno::Reference< chart2::data::XDataSource > xErrorBarY;
                xPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarY;
                if(xErrorBarY.is())
                    getRangeFromDataSource(xErrorBarY, rRangeRep);
                uno::Reference< chart2::data::XDataSource > xErrorBarX;
                xPropSet->getPropertyValue("ErrorBarX") >>= xErrorBarX;
                if(xErrorBarX.is())
                    getRangeFromDataSource(xErrorBarX, rRangeRep);
            }
        }
    }
}

void getRangeFromOle2Object(const SdrOle2Obj& rObj, std::vector<OUString>& rRangeRep)
{
    if (!rObj.IsChart())
        // not a chart object.
        return;

    const uno::Reference<embed::XEmbeddedObject>& xObj = rObj.GetObjRef();
    if (!xObj.is())
        return;

    uno::Reference<embed::XComponentSupplier> xCompSupp(xObj, uno::UNO_QUERY);
    if (!xCompSupp.is())
        return;

    uno::Reference<chart2::XChartDocument> xChartDoc(xCompSupp->getComponent(), uno::UNO_QUERY);
    if (!xChartDoc.is())
        return;

    if(xChartDoc->hasInternalDataProvider())
        return;

    getRangeFromErrorBar(xChartDoc, rRangeRep);

    uno::Reference<chart2::data::XDataSource> xDataSource(xChartDoc, uno::UNO_QUERY);
    if (!xDataSource.is())
        return;

    // Get all data sources used in this chart.
    getRangeFromDataSource(xDataSource, rRangeRep);

    return;
}

// Get all cell ranges that are referenced by the selected chart objects.
void getOleSourceRanges(const SdrMarkList& rMarkList, bool& rAnyOle, bool& rOneOle, std::vector<ScRange>* pRanges = nullptr, const ScDocument* pDoc = nullptr )
{
    bool bCalcSourceRanges = pRanges && pDoc;
    std::vector<OUString> aRangeReps;
    rAnyOle = rOneOle = false;
    const size_t nCount = rMarkList.GetMarkCount();
    for (size_t i=0; i<nCount; ++i)
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        if ( !pMark )
            continue;

        SdrObject* pObj = pMark->GetMarkedSdrObj();
        if ( !pObj )
            continue;

        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();
        if (nSdrObjKind == OBJ_OLE2)
        {
            rAnyOle = true;
            rOneOle = (nCount == 1);
            if ( bCalcSourceRanges )
                getRangeFromOle2Object( static_cast<const SdrOle2Obj&>( *pObj ), aRangeReps );
            else
                break;
        }
        else if ( dynamic_cast<const SdrObjGroup*>( pObj) !=  nullptr )
        {
            SdrObjListIter aIter( *pObj, SdrIterMode::DeepNoGroups );
            SdrObject* pSubObj = aIter.Next();
            while (pSubObj)
            {
                if ( pSubObj->GetObjIdentifier() == OBJ_OLE2 )
                {
                    rAnyOle = true;
                    // rOneOle remains false - a group isn't treated like a single OLE object
                    if ( !bCalcSourceRanges )
                        return;

                    getRangeFromOle2Object( static_cast<const SdrOle2Obj&>( *pSubObj ), aRangeReps );
                }
                pSubObj = aIter.Next();
            }
        }
    }

    if (!bCalcSourceRanges)
        return;

    // Compile all range representation strings into ranges.
    for (const auto& rRangeRep : aRangeReps)
    {
        ScRangeList aRange;
        ScAddress aAddr;
        if (aRange.Parse(rRangeRep, pDoc, pDoc->GetAddressConvention()) & ScRefFlags::VALID)
        {
            for(size_t i = 0; i < aRange.size(); ++i)
                pRanges->push_back(aRange[i]);
        }
        else if (aAddr.Parse(rRangeRep, pDoc, pDoc->GetAddressConvention()) & ScRefFlags::VALID)
            pRanges->push_back(aAddr);
    }

    return;
}

class InsertTabIndex
{
    std::vector<SCTAB>& mrTabs;
public:
    explicit InsertTabIndex(std::vector<SCTAB>& rTabs) : mrTabs(rTabs) {}
    void operator() (const ScRange& rRange)
    {
        mrTabs.push_back(rRange.aStart.Tab());
    }
};

class CopyRangeData
{
    ScDocument* mpSrc;
    ScDocument* const mpDest;
public:
    CopyRangeData(ScDocument* pSrc, ScDocument* pDest) : mpSrc(pSrc), mpDest(pDest) {}

    void operator() (const ScRange& rRange)
    {
        OUString aTabName;
        mpSrc->GetName(rRange.aStart.Tab(), aTabName);

        SCTAB nTab;
        if (!mpDest->GetTable(aTabName, nTab))
            // Sheet by this name doesn't exist.
            return;

        mpSrc->CopyStaticToDocument(rRange, nTab, mpDest);
    }
};

void copyChartRefDataToClipDoc(ScDocument* pSrcDoc, ScDocument* pClipDoc, const std::vector<ScRange>& rRanges)
{
    // Get a list of referenced table indices.
    std::vector<SCTAB> aTabs;
    std::for_each(rRanges.begin(), rRanges.end(), InsertTabIndex(aTabs));
    std::sort(aTabs.begin(), aTabs.end());
    aTabs.erase(std::unique(aTabs.begin(), aTabs.end()), aTabs.end());

    // Get table names.
    if (aTabs.empty())
        return;

    // Create sheets only for referenced source sheets.
    OUString aName;
    std::vector<SCTAB>::const_iterator it = aTabs.begin(), itEnd = aTabs.end();
    if (!pSrcDoc->GetName(*it, aName))
        return;

    pClipDoc->SetTabNameOnLoad(0, aName); // document initially has one sheet.

    for (++it; it != itEnd; ++it)
    {
        if (!pSrcDoc->GetName(*it, aName))
            return;

        pClipDoc->AppendTabOnLoad(aName);
    }

    std::for_each(rRanges.begin(), rRanges.end(), CopyRangeData(pSrcDoc, pClipDoc));
}

}

void ScDrawView::CheckOle( const SdrMarkList& rMarkList, bool& rAnyOle, bool& rOneOle )
{
    getOleSourceRanges( rMarkList, rAnyOle, rOneOle );
}

void ScDrawView::DoCopy()
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    std::vector<ScRange> aRanges;
    bool bAnyOle = false, bOneOle = false;
    getOleSourceRanges( rMarkList, bAnyOle, bOneOle, &aRanges, pDoc );

    // update ScGlobal::xDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    if (ScGlobal::xDrawClipDocShellRef.is() && !aRanges.empty())
    {
        // Copy data referenced by the chart objects to the draw clip
        // document. We need to do this before CreateMarkedObjModel() below.
        ScDocShellRef xDocSh = ScGlobal::xDrawClipDocShellRef;
        ScDocument& rClipDoc = xDocSh->GetDocument();
        copyChartRefDataToClipDoc(pDoc, &rClipDoc, aRanges);
    }
    std::unique_ptr<SdrModel> pModel(CreateMarkedObjModel());
    ScDrawLayer::SetGlobalDrawPersist(nullptr);

    //  Charts now always copy their data in addition to the source reference, so
    //  there's no need to call SchDLL::Update for the charts in the clipboard doc.
    //  Update with the data (including NumberFormatter) from the live document would
    //  also store the NumberFormatter in the clipboard chart (#88749#)

    ScDocShell* pDocSh = pViewData->GetDocShell();

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScDrawTransferObj ctor

    ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( std::move(pModel), pDocSh, aObjDesc );
    uno::Reference<css::datatransfer::XTransferable2> xTransferObj = pTransferObj;

    if ( ScGlobal::xDrawClipDocShellRef.is() )
    {
        pTransferObj->SetDrawPersist( ScGlobal::xDrawClipDocShellRef.get() );    // keep persist for ole objects alive
    }

    pTransferObj->CopyToClipboard( pViewData->GetActiveWin() );     // system clipboard
}

uno::Reference<datatransfer::XTransferable> ScDrawView::CopyToTransferable()
{
    bool bAnyOle, bOneOle;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    CheckOle( rMarkList, bAnyOle, bOneOle );

    // update ScGlobal::xDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    std::unique_ptr<SdrModel> pModel( CreateMarkedObjModel() );
    ScDrawLayer::SetGlobalDrawPersist(nullptr);

    //  Charts now always copy their data in addition to the source reference, so
    //  there's no need to call SchDLL::Update for the charts in the clipboard doc.
    //  Update with the data (including NumberFormatter) from the live document would
    //  also store the NumberFormatter in the clipboard chart (#88749#)
    // lcl_RefreshChartData( pModel, pViewData->GetDocument() );

    ScDocShell* pDocSh = pViewData->GetDocShell();

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScDrawTransferObj ctor

    ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( std::move(pModel), pDocSh, aObjDesc );
    uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

    if ( ScGlobal::xDrawClipDocShellRef.is() )
    {
        pTransferObj->SetDrawPersist( ScGlobal::xDrawClipDocShellRef.get() );    // keep persist for ole objects alive
    }

    return xTransferable;
}

// Calculate correction for 100%, regardless of current settings

void ScDrawView::CalcNormScale( Fraction& rFractX, Fraction& rFractY ) const
{
    double nPPTX = ScGlobal::nScreenPPTX;
    double nPPTY = ScGlobal::nScreenPPTY;

    if (pViewData)
        nPPTX /= pViewData->GetDocShell()->GetOutputFactor();

    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    pDoc->GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 1000;

    Fraction aZoom(1,1);
    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev, aZoom,aZoom,
                            nPPTX, nPPTY, rFractX,rFractY );
}

void ScDrawView::SetMarkedOriginalSize()
{
    std::unique_ptr<SdrUndoGroup> pUndoGroup(new SdrUndoGroup(*GetModel()));

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    long nDone = 0;
    const size_t nCount = rMarkList.GetMarkCount();
    for (size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        sal_uInt16 nIdent = pObj->GetObjIdentifier();
        bool bDo = false;
        Size aOriginalSize;
        if (nIdent == OBJ_OLE2)
        {
            // TODO/LEAN: working with visual area can switch object to running state
            uno::Reference < embed::XEmbeddedObject > xObj( static_cast<SdrOle2Obj*>(pObj)->GetObjRef(), uno::UNO_QUERY );
            if ( xObj.is() )    // NULL for an invalid object that couldn't be loaded
            {
                sal_Int64 nAspect = static_cast<SdrOle2Obj*>(pObj)->GetAspect();

                if ( nAspect == embed::Aspects::MSOLE_ICON )
                {
                    MapMode aMapMode( MapUnit::Map100thMM );
                    aOriginalSize = static_cast<SdrOle2Obj*>(pObj)->GetOrigObjSize( &aMapMode );
                    bDo = true;
                }
                else
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( static_cast<SdrOle2Obj*>(pObj)->GetAspect() ) );
                    awt::Size aSz;
                    try
                    {
                        aSz = xObj->getVisualAreaSize( static_cast<SdrOle2Obj*>(pObj)->GetAspect() );
                        aOriginalSize = OutputDevice::LogicToLogic(
                                            Size( aSz.Width, aSz.Height ),
                                            MapMode(aUnit),
                                            MapMode(MapUnit::Map100thMM));
                        bDo = true;
                    } catch( embed::NoVisualAreaSizeException& )
                    {
                        OSL_ENSURE( false, "Can't get the original size of the object!" );
                    }
                }
            }
        }
        else if (nIdent == OBJ_GRAF)
        {
            const Graphic& rGraphic = static_cast<SdrGrafObj*>(pObj)->GetGraphic();

            MapMode aSourceMap = rGraphic.GetPrefMapMode();
            MapMode aDestMap( MapUnit::Map100thMM );
            if (aSourceMap.GetMapUnit() == MapUnit::MapPixel)
            {
                // consider pixel correction, so that the bitmap is correct on the screen
                Fraction aNormScaleX, aNormScaleY;
                CalcNormScale( aNormScaleX, aNormScaleY );
                aDestMap.SetScaleX(aNormScaleX);
                aDestMap.SetScaleY(aNormScaleY);
            }
            if (pViewData)
            {
                vcl::Window* pActWin = pViewData->GetActiveWin();
                if (pActWin)
                {
                    aOriginalSize = pActWin->LogicToLogic(
                                    rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
                    bDo = true;
                }
            }
        }

        if ( bDo )
        {
            tools::Rectangle aDrawRect = pObj->GetLogicRect();

            pUndoGroup->AddAction( std::make_unique<SdrUndoGeoObj>( *pObj ) );
            pObj->Resize( aDrawRect.TopLeft(), Fraction( aOriginalSize.Width(), aDrawRect.GetWidth() ),
                                                 Fraction( aOriginalSize.Height(), aDrawRect.GetHeight() ) );
            ++nDone;
        }
    }

    if (nDone && pViewData)
    {
        pUndoGroup->SetComment(ScResId( STR_UNDO_ORIGINALSIZE ));
        ScDocShell* pDocSh = pViewData->GetDocShell();
        pDocSh->GetUndoManager()->AddUndoAction(std::move(pUndoGroup));
        pDocSh->SetDrawModified();
    }
}

void ScDrawView::FitToCellSize()
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();

    if (rMarkList.GetMarkCount() != 1)
    {
        SAL_WARN("sc.ui", "Fit to cell only works with one graphic!");
        return;
    }

    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    ScAnchorType aAnchorType = ScDrawLayer::GetAnchorType(*pObj);
    if (aAnchorType != SCA_CELL && aAnchorType != SCA_CELL_RESIZE)
    {
        SAL_WARN("sc.ui", "Fit to cell only works with cell anchored graphics!");
        return;
    }

    ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObj);
    if (!pObjData)
    {
        SAL_WARN("sc.ui", "Missing ScDrawObjData!");
        return;
    }

    std::unique_ptr<SdrUndoGroup> pUndoGroup(new SdrUndoGroup(*GetModel()));
    tools::Rectangle aGraphicRect = pObj->GetSnapRect();
    tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( *pDoc, pObjData->maStart, true);

    // For graphic objects, we want to keep the aspect ratio
    if (pObj->shouldKeepAspectRatio())
    {
        long nWidth = aGraphicRect.GetWidth();
        assert(nWidth && "div-by-zero");
        double fScaleX = static_cast<double>(aCellRect.GetWidth()) / static_cast<double>(nWidth);
        long nHeight = aGraphicRect.GetHeight();
        assert(nHeight && "div-by-zero");
        double fScaleY = static_cast<double>(aCellRect.GetHeight()) / static_cast<double>(nHeight);
        double fScaleMin = std::min(fScaleX, fScaleY);

        aCellRect.setWidth(static_cast<double>(aGraphicRect.GetWidth()) * fScaleMin);
        aCellRect.setHeight(static_cast<double>(aGraphicRect.GetHeight()) * fScaleMin);
    }

    pUndoGroup->AddAction( std::make_unique<SdrUndoGeoObj>( *pObj ) );

    pObj->SetSnapRect(aCellRect);

    pUndoGroup->SetComment(ScResId( STR_UNDO_FITCELLSIZE ));
    ScDocShell* pDocSh = pViewData->GetDocShell();
    pDocSh->GetUndoManager()->AddUndoAction(std::move(pUndoGroup));

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
