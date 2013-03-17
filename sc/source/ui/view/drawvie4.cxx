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

#include "drawview.hxx"
#include "global.hxx"
#include "drwlayer.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "drwtrans.hxx"
#include "transobj.hxx"     // SetDrawClipDoc
#include "drawutil.hxx"
#include "scmod.hxx"
#include "globstr.hrc"
#include "chartarr.hxx"

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

Point aDragStartDiff;

// -----------------------------------------------------------------------

void ScDrawView::CheckOle( const SdrMarkList& rMarkList, sal_Bool& rAnyOle, sal_Bool& rOneOle )
{
    rAnyOle = rOneOle = false;
    sal_uLong nCount = rMarkList.GetMarkCount();
    for (sal_uLong i=0; i<nCount; i++)
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        SdrObject* pObj = pMark->GetMarkedSdrObj();
        sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();
        if (nSdrObjKind == OBJ_OLE2)
        {
            rAnyOle = sal_True;
            rOneOle = (nCount == 1);
            break;
        }
        else if ( pObj->ISA(SdrObjGroup) )
        {
            SdrObjListIter aIter( *pObj, IM_DEEPNOGROUPS );
            SdrObject* pSubObj = aIter.Next();
            while (pSubObj)
            {
                if ( pSubObj->GetObjIdentifier() == OBJ_OLE2 )
                {
                    rAnyOle = sal_True;
                    // rOneOle remains sal_False - a group isn't treated like a single OLE object
                    return;
                }
                pSubObj = aIter.Next();
            }
        }
    }
}

sal_Bool ScDrawView::BeginDrag( Window* pWindow, const Point& rStartPos )
{
    sal_Bool bReturn = false;

    if ( AreObjectsMarked() )
    {
        BrkAction();

        Rectangle aMarkedRect = GetAllMarkedRect();
        Region aRegion( aMarkedRect );

        aDragStartDiff = rStartPos - aMarkedRect.TopLeft();

        sal_Bool bAnyOle, bOneOle;
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        CheckOle( rMarkList, bAnyOle, bOneOle );

        ScDocShellRef aDragShellRef;
        if (bAnyOle)
        {
            aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
            aDragShellRef->DoInitNew(NULL);
        }
        ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);
        SdrModel* pModel = GetMarkedObjModel();
        ScDrawLayer::SetGlobalDrawPersist(NULL);

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

        ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
        uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

        pTransferObj->SetDrawPersist( &aDragShellRef );    // keep persist for ole objects alive
        pTransferObj->SetDragSource( this );            // copies selection

        SC_MOD()->SetDragObject( NULL, pTransferObj );      // for internal D&D
        pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
    }

    return bReturn;
}

namespace {

void getRangeFromOle2Object(const SdrOle2Obj& rObj, std::vector<OUString>& rRangeRep)
{
    if (!rObj.IsChart())
        // not a chart object.
        return;

    uno::Reference<embed::XEmbeddedObject> xObj = rObj.GetObjRef();
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

    uno::Reference<chart2::data::XDataSource> xDataSource(xChartDoc, uno::UNO_QUERY);
    if (!xDataSource.is())
        return;

    // Get all data sources used in this chart.
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

/**
 * Get all cell ranges that are referenced by the selected chart objects.
 */
void getChartSourceRanges(ScDocument* pDoc, const SdrMarkList& rObjs, std::vector<ScRange>& rRanges)
{
    std::vector<OUString> aRangeReps;
    for (size_t i = 0, n = rObjs.GetMarkCount(); i < n; ++i)
    {
        const SdrMark* pMark = rObjs.GetMark(i);
        if (!pMark)
            continue;

        const SdrObject* pObj = pMark->GetMarkedSdrObj();
        if (!pObj)
            continue;

        switch (pObj->GetObjIdentifier())
        {
            case OBJ_OLE2:
                getRangeFromOle2Object(static_cast<const SdrOle2Obj&>(*pObj), aRangeReps);
            break;
            case OBJ_GRUP:
            {
                SdrObjListIter aIter(*pObj, IM_DEEPNOGROUPS);
                for (SdrObject* pSubObj = aIter.Next(); pSubObj; pSubObj = aIter.Next())
                {
                    if (pSubObj->GetObjIdentifier() != OBJ_OLE2)
                        continue;

                    getRangeFromOle2Object(static_cast<const SdrOle2Obj&>(*pSubObj), aRangeReps);
                }

            }
            break;
            default:
                ;
        }
    }

    // Compile all range representation strings into ranges.
    std::vector<OUString>::const_iterator it = aRangeReps.begin(), itEnd = aRangeReps.end();
    for (; it != itEnd; ++it)
    {
        ScRange aRange;
        ScAddress aAddr;
        if (aRange.Parse(*it, pDoc, pDoc->GetAddressConvention()) & SCA_VALID)
            rRanges.push_back(aRange);
        else if (aAddr.Parse(*it, pDoc, pDoc->GetAddressConvention()) & SCA_VALID)
            rRanges.push_back(aAddr);
    }
}

class InsertTabIndex : std::unary_function<ScRange, void>
{
    std::vector<SCTAB>& mrTabs;
public:
    InsertTabIndex(std::vector<SCTAB>& rTabs) : mrTabs(rTabs) {}
    void operator() (const ScRange& rRange)
    {
        mrTabs.push_back(rRange.aStart.Tab());
    }
};

class CopyRangeData : std::unary_function<ScRange, void>
{
    ScDocument* mpSrc;
    ScDocument* mpDest;
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

void ScDrawView::DoCopy()
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    std::vector<ScRange> aRanges;
    getChartSourceRanges(pDoc, rMarkList, aRanges);

    // update ScGlobal::pDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc(!aRanges.empty()) );
    if (ScGlobal::pDrawClipDocShellRef)
    {
        // Copy data referenced by the chart objects to the draw clip
        // document. We need to do this before GetMarkedObjModel() below.
        ScDocShellRef xDocSh = *ScGlobal::pDrawClipDocShellRef;
        ScDocument* pClipDoc = xDocSh->GetDocument();
        copyChartRefDataToClipDoc(pDoc, pClipDoc, aRanges);
    }
    SdrModel* pModel = GetMarkedObjModel();
    ScDrawLayer::SetGlobalDrawPersist(NULL);

    //  Charts now always copy their data in addition to the source reference, so
    //  there's no need to call SchDLL::Update for the charts in the clipboard doc.
    //  Update with the data (including NumberFormatter) from the live document would
    //  also store the NumberFormatter in the clipboard chart (#88749#)

    ScDocShell* pDocSh = pViewData->GetDocShell();

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScDrawTransferObj ctor

    ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
    uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

    if ( ScGlobal::pDrawClipDocShellRef )
    {
        pTransferObj->SetDrawPersist( &(*ScGlobal::pDrawClipDocShellRef) );    // keep persist for ole objects alive
    }

    pTransferObj->CopyToClipboard( pViewData->GetActiveWin() );     // system clipboard
    SC_MOD()->SetClipObject( NULL, pTransferObj );                  // internal clipboard
}

uno::Reference<datatransfer::XTransferable> ScDrawView::CopyToTransferable()
{
    sal_Bool bAnyOle, bOneOle;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    CheckOle( rMarkList, bAnyOle, bOneOle );

    // update ScGlobal::pDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    SdrModel* pModel = GetMarkedObjModel();
    ScDrawLayer::SetGlobalDrawPersist(NULL);

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

    ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
    uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

    if ( ScGlobal::pDrawClipDocShellRef )
    {
        pTransferObj->SetDrawPersist( &(*ScGlobal::pDrawClipDocShellRef) );    // keep persist for ole objects alive
    }

    return xTransferable;
}

//  Korrektur fuer 100% berechnen, unabhaengig von momentanen Einstellungen

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
    SdrUndoGroup* pUndoGroup = new SdrUndoGroup(*GetModel());

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    long nDone = 0;
    sal_uLong nCount = rMarkList.GetMarkCount();
    for (sal_uLong i=0; i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        sal_uInt16 nIdent = pObj->GetObjIdentifier();
        sal_Bool bDo = false;
        Size aOriginalSize;
        if (nIdent == OBJ_OLE2)
        {
            // TODO/LEAN: working with visual area can switch object to running state
            uno::Reference < embed::XEmbeddedObject > xObj( ((SdrOle2Obj*)pObj)->GetObjRef(), uno::UNO_QUERY );
            if ( xObj.is() )    // NULL for an invalid object that couldn't be loaded
            {
                sal_Int64 nAspect = ((SdrOle2Obj*)pObj)->GetAspect();

                if ( nAspect == embed::Aspects::MSOLE_ICON )
                {
                    MapMode aMapMode( MAP_100TH_MM );
                    aOriginalSize = ((SdrOle2Obj*)pObj)->GetOrigObjSize( &aMapMode );
                    bDo = sal_True;
                }
                else
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( ((SdrOle2Obj*)pObj)->GetAspect() ) );
                    awt::Size aSz;
                    try
                    {
                        aSz = xObj->getVisualAreaSize( ((SdrOle2Obj*)pObj)->GetAspect() );
                        aOriginalSize = OutputDevice::LogicToLogic(
                                            Size( aSz.Width, aSz.Height ),
                                            aUnit, MAP_100TH_MM );
                        bDo = sal_True;
                    } catch( embed::NoVisualAreaSizeException& )
                    {
                        OSL_ENSURE( false, "Can't get the original size of the object!" );
                    }
                }
            }
        }
        else if (nIdent == OBJ_GRAF)
        {
            const Graphic& rGraphic = ((SdrGrafObj*)pObj)->GetGraphic();

            MapMode aSourceMap = rGraphic.GetPrefMapMode();
            MapMode aDestMap( MAP_100TH_MM );
            if (aSourceMap.GetMapUnit() == MAP_PIXEL)
            {
                //  Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

                Fraction aNormScaleX, aNormScaleY;
                CalcNormScale( aNormScaleX, aNormScaleY );
                aDestMap.SetScaleX(aNormScaleX);
                aDestMap.SetScaleY(aNormScaleY);
            }
            if (pViewData)
            {
                Window* pActWin = pViewData->GetActiveWin();
                if (pActWin)
                {
                    aOriginalSize = pActWin->LogicToLogic(
                                    rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
                    bDo = sal_True;
                }
            }
        }

        if ( bDo )
        {
            Rectangle aDrawRect = pObj->GetLogicRect();

            pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
            pObj->Resize( aDrawRect.TopLeft(), Fraction( aOriginalSize.Width(), aDrawRect.GetWidth() ),
                                                 Fraction( aOriginalSize.Height(), aDrawRect.GetHeight() ) );
            ++nDone;
        }
    }

    if (nDone)
    {
        pUndoGroup->SetComment(ScGlobal::GetRscString( STR_UNDO_ORIGINALSIZE ));
        ScDocShell* pDocSh = pViewData->GetDocShell();
        pDocSh->GetUndoManager()->AddUndoAction(pUndoGroup);
        pDocSh->SetDrawModified();
    }
    else
        delete pUndoGroup;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
