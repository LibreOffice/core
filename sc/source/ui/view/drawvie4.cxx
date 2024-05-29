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
#include <gridwin.hxx>
#include <userdat.hxx>

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace com::sun::star;

Point aDragStartDiff;

void ScDrawView::BeginDrag( vcl::Window* pWindow, const Point& rStartPos )
{
    if ( GetMarkedObjectList().GetMarkCount() == 0 )
        return;

    BrkAction();

    tools::Rectangle aMarkedRect = GetAllMarkedRect();

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

    rtl::Reference<ScDrawTransferObj> pTransferObj = new ScDrawTransferObj( std::move(pModel), pDocSh, std::move(aObjDesc) );

    pTransferObj->SetDrawPersist(aDragShellRef); // keep persist for ole objects alive
    pTransferObj->SetDragSource( this );               // copies selection

    SC_MOD()->SetDragObject( nullptr, pTransferObj.get() );     // for internal D&D
    pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
}

namespace {

void getRangeFromDataSource( uno::Reference< chart2::data::XDataSource > const & xDataSource, std::vector<OUString>& rRangeRep)
{
    const uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence> > xSeqs = xDataSource->getDataSequences();
    for (const uno::Reference<chart2::data::XLabeledDataSequence>& xLS : xSeqs)
    {
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

    const uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    for(const auto& rCooSys : xCooSysSequence)
    {
        uno::Reference< chart2::XChartTypeContainer > xChartTypeContainer( rCooSys, uno::UNO_QUERY);
        if(!xChartTypeContainer.is())
            continue;

        const uno::Sequence< uno::Reference< chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
        for(const auto& rChartType : xChartTypeSequence)
        {
            uno::Reference< chart2::XDataSeriesContainer > xDataSequenceContainer( rChartType, uno::UNO_QUERY);
            if(!xDataSequenceContainer.is())
                continue;

            const uno::Sequence< uno::Reference< chart2::XDataSeries > > xSeriesSequence( xDataSequenceContainer->getDataSeries() );
            for(const uno::Reference<chart2::XDataSeries>& xSeries : xSeriesSequence)
            {
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

    uno::Reference<chart2::XChartDocument> xChartDoc(xObj->getComponent(), uno::UNO_QUERY);
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

        SdrObjKind nSdrObjKind = pObj->GetObjIdentifier();
        if (nSdrObjKind == SdrObjKind::OLE2)
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
                if ( pSubObj->GetObjIdentifier() == SdrObjKind::OLE2 )
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
        if (aRange.Parse(rRangeRep, *pDoc, pDoc->GetAddressConvention()) & ScRefFlags::VALID)
        {
            pRanges->insert(pRanges->end(), aRange.begin(), aRange.end());
        }
        else if (aAddr.Parse(rRangeRep, *pDoc, pDoc->GetAddressConvention()) & ScRefFlags::VALID)
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
    ScDocument& mrSrc;
    ScDocument& mrDest;
public:
    CopyRangeData(ScDocument& rSrc, ScDocument& rDest) : mrSrc(rSrc), mrDest(rDest) {}

    void operator() (const ScRange& rRange)
    {
        OUString aTabName;
        mrSrc.GetName(rRange.aStart.Tab(), aTabName);

        SCTAB nTab;
        if (!mrDest.GetTable(aTabName, nTab))
            // Sheet by this name doesn't exist.
            return;

        mrSrc.CopyStaticToDocument(rRange, nTab, mrDest);
    }
};

void copyChartRefDataToClipDoc(ScDocument& rSrcDoc, ScDocument& rClipDoc, const std::vector<ScRange>& rRanges)
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
    if (!rSrcDoc.GetName(*it, aName))
        return;

    rClipDoc.SetTabNameOnLoad(0, aName); // document initially has one sheet.

    for (++it; it != itEnd; ++it)
    {
        if (!rSrcDoc.GetName(*it, aName))
            return;

        rClipDoc.AppendTabOnLoad(aName);
    }

    std::for_each(rRanges.begin(), rRanges.end(), CopyRangeData(rSrcDoc, rClipDoc));
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
    getOleSourceRanges( rMarkList, bAnyOle, bOneOle, &aRanges, &rDoc );

    // update ScGlobal::xDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    if (ScGlobal::xDrawClipDocShellRef.is() && !aRanges.empty())
    {
        // Copy data referenced by the chart objects to the draw clip
        // document. We need to do this before CreateMarkedObjModel() below.
        ScDocShellRef xDocSh = ScGlobal::xDrawClipDocShellRef;
        ScDocument& rClipDoc = xDocSh->GetDocument();
        copyChartRefDataToClipDoc(rDoc, rClipDoc, aRanges);
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

    rtl::Reference<ScDrawTransferObj> pTransferObj(new ScDrawTransferObj( std::move(pModel), pDocSh, std::move(aObjDesc) ));

    if ( ScGlobal::xDrawClipDocShellRef.is() )
    {
        pTransferObj->SetDrawPersist( ScGlobal::xDrawClipDocShellRef );    // keep persist for ole objects alive
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

    rtl::Reference<ScDrawTransferObj> pTransferObj = new ScDrawTransferObj( std::move(pModel), pDocSh, std::move(aObjDesc) );

    if ( ScGlobal::xDrawClipDocShellRef.is() )
    {
        pTransferObj->SetDrawPersist( ScGlobal::xDrawClipDocShellRef );    // keep persist for ole objects alive
    }

    return pTransferObj;
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
    rDoc.GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 1000;

    Fraction aZoom(1,1);
    ScDrawUtil::CalcScale( rDoc, nTab, 0,0, nEndCol,nEndRow, pDev, aZoom,aZoom,
                           nPPTX, nPPTY, rFractX,rFractY );
}

void ScDrawView::SetMarkedOriginalSize()
{
    std::unique_ptr<SdrUndoGroup> pUndoGroup(new SdrUndoGroup(GetModel()));

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    tools::Long nDone = 0;
    const size_t nCount = rMarkList.GetMarkCount();
    for (size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        SdrObjKind nIdent = pObj->GetObjIdentifier();
        bool bDo = false;
        Size aOriginalSize;
        if (nIdent == SdrObjKind::OLE2)
        {
            // TODO/LEAN: working with visual area can switch object to running state
            uno::Reference < embed::XEmbeddedObject > xObj = static_cast<SdrOle2Obj*>(pObj)->GetObjRef();
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
                    try
                    {
                        awt::Size aSz = xObj->getVisualAreaSize( static_cast<SdrOle2Obj*>(pObj)->GetAspect() );
                        aOriginalSize = OutputDevice::LogicToLogic(
                                            Size( aSz.Width, aSz.Height ),
                                            MapMode(aUnit),
                                            MapMode(MapUnit::Map100thMM));
                        bDo = true;
                    } catch( embed::NoVisualAreaSizeException& )
                    {
                        TOOLS_WARN_EXCEPTION("sc.ui", "Can't get the original size of the object!" );
                    }
                }
            }
        }
        else if (nIdent == SdrObjKind::Graphic)
        {
            const SdrGrafObj* pSdrGrafObj = static_cast<const SdrGrafObj*>(pObj);

            MapMode aSourceMap = pSdrGrafObj->GetGraphic().GetPrefMapMode();
            MapMode aDestMap( MapUnit::Map100thMM );
            if (aSourceMap.GetMapUnit() == MapUnit::MapPixel)
            {
                // consider pixel correction, so that the bitmap is correct on the screen
                Fraction aNormScaleX, aNormScaleY;
                CalcNormScale( aNormScaleX, aNormScaleY );
                aDestMap.SetScaleX(aNormScaleX);
                aDestMap.SetScaleY(aNormScaleY);
            }
            aOriginalSize = pSdrGrafObj->getOriginalSize();
            bDo = true;
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

    std::unique_ptr<SdrUndoGroup> pUndoGroup(new SdrUndoGroup(GetModel()));
    tools::Rectangle aGraphicRect = pObj->GetSnapRect();
    tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( rDoc, pObjData->maStart, true);

    // For graphic objects, we want to keep the aspect ratio
    if (pObj->shouldKeepAspectRatio())
    {
        tools::Long nWidth = aGraphicRect.GetWidth();
        assert(nWidth && "div-by-zero");
        double fScaleX = static_cast<double>(aCellRect.GetWidth()) / static_cast<double>(nWidth);
        tools::Long nHeight = aGraphicRect.GetHeight();
        assert(nHeight && "div-by-zero");
        double fScaleY = static_cast<double>(aCellRect.GetHeight()) / static_cast<double>(nHeight);
        double fScaleMin = std::min(fScaleX, fScaleY);

        aCellRect.setWidth(static_cast<double>(aGraphicRect.GetWidth()) * fScaleMin);
        aCellRect.setHeight(static_cast<double>(aGraphicRect.GetHeight()) * fScaleMin);
    }

    pUndoGroup->AddAction( std::make_unique<SdrUndoGeoObj>( *pObj ) );
    if (pObj->GetObjIdentifier() == SdrObjKind::CustomShape)
        pObj->AdjustToMaxRect(aCellRect);
    else
        pObj->SetSnapRect(aCellRect);

    pUndoGroup->SetComment(ScResId( STR_UNDO_FITCELLSIZE ));
    ScDocShell* pDocSh = pViewData->GetDocShell();
    pDocSh->GetUndoManager()->AddUndoAction(std::move(pUndoGroup));

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
