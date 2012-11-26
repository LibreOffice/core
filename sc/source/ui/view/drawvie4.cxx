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



// INCLUDE ---------------------------------------------------------------
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svdlegacy.hxx>

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

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

basegfx::B2DPoint aDragStartDiff;

// -----------------------------------------------------------------------

//! welche Funktionen aus drawview/drawvie4 muessen wirklich ohne Optimierung sein?

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

// -----------------------------------------------------------------------

void lcl_CheckOle( const SdrObjectVector& rSelection, sal_Bool& rAnyOle, sal_Bool& rOneOle )
{
    rAnyOle = rOneOle = sal_False;

    for (sal_uInt32 i(0); i < rSelection.size(); i++)
    {
        const SdrObject* pObj = rSelection[i];
        const sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_OLE2)
        {
            rAnyOle = sal_True;
            rOneOle = (1 == rSelection.size());
            break;
        }
        else if ( dynamic_cast< const SdrObjGroup* >(pObj) )
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

#if 0
void lcl_RefreshChartData( SdrModel* pModel, ScDocument* pSourceDoc )
{
    const sal_uInt32 nPages(pModel->GetPageCount());
    for (SCTAB nTab=0; nTab<nPages; nTab++)
    {
        SdrPage* pPage = pModel->GetPage(nTab);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                if ( aIPObj.Is() && SotExchange::IsChart( aIPObj->GetStorage()->GetClassName() ) )
                {
                    SchMemChart* pOldData = SchDLL::GetChartData(aIPObj);
                    if ( pOldData )
                    {
                        //  create data from source document
                        ScChartArray aArray( pSourceDoc, *pOldData );
                        if ( aArray.IsValid() )
                        {
                            SchMemChart* pNewData = aArray.CreateMemChart();
                            SchDLL::Update( aIPObj, pNewData );
                            delete pNewData;
                            ((SdrOle2Obj*)pObject)->GetNewReplacement();
                        }
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}
#endif


sal_Bool ScDrawView::BeginDrag( Window* pWindow, const basegfx::B2DPoint& rStartPos )
{
    sal_Bool bReturn = sal_False;

    if ( areSdrObjectsSelected() )
    {
        BrkAction();

        const basegfx::B2DRange aMarkedRange(getMarkedObjectSnapRange());

        aDragStartDiff = rStartPos - aMarkedRange.getMinimum();

        sal_Bool bAnyOle, bOneOle;
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        lcl_CheckOle( aSelection, bAnyOle, bOneOle );

        ScDocShellRef aDragShellRef;
        if (bAnyOle)
        {
            aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
            aDragShellRef->DoInitNew(NULL);
        }
        ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);
        SdrModel* pModel = GetAllMarkedModel();
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

void ScDrawView::DoCopy()
{
    sal_Bool bAnyOle, bOneOle;
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    lcl_CheckOle( aSelection, bAnyOle, bOneOle );

    // update ScGlobal::pDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    SdrModel* pModel = GetAllMarkedModel();
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

    pTransferObj->CopyToClipboard( pViewData->GetActiveWin() );     // system clipboard
    SC_MOD()->SetClipObject( NULL, pTransferObj );                  // internal clipboard
}

uno::Reference<datatransfer::XTransferable> ScDrawView::CopyToTransferable()
{
    sal_Bool bAnyOle, bOneOle;
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    lcl_CheckOle( aSelection, bAnyOle, bOneOle );

    // update ScGlobal::pDrawClipDocShellRef
    ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    SdrModel* pModel = GetAllMarkedModel();
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
    Point aLogic = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
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
        nEndRow = 20;

    Fraction aZoom(1,1);
    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev, aZoom,aZoom,
                            nPPTX, nPPTY, rFractX,rFractY );
}

void ScDrawView::SetMarkedOriginalSize()
{
    SdrUndoGroup* pUndoGroup = new SdrUndoGroup(getSdrModelFromSdrView());
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    long nDone = 0;

    for (sal_uInt32 i(0); i < aSelection.size(); i++)
    {
        SdrObject* pObj = aSelection[i];
        sal_Bool bDo = sal_False;
        Size aOriginalSize;

        if(SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pObj))
        {
            // TODO/LEAN: working with visual area can switch object to running state
            uno::Reference < embed::XEmbeddedObject > xObj( pSdrOle2Obj->GetObjRef(), uno::UNO_QUERY );
            if ( xObj.is() )    // #121612# NULL for an invalid object that couldn't be loaded
            {
                sal_Int64 nAspect = pSdrOle2Obj->GetAspect();

                if ( nAspect == embed::Aspects::MSOLE_ICON )
                {
                    MapMode aMapMode( MAP_100TH_MM );
                    aOriginalSize = pSdrOle2Obj->GetOrigObjSize( &aMapMode );
                    bDo = sal_True;
                }
                else
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( pSdrOle2Obj->GetAspect() ) );
                    awt::Size aSz;
                    try
                    {
                        aSz = xObj->getVisualAreaSize( pSdrOle2Obj->GetAspect() );
                        aOriginalSize = OutputDevice::LogicToLogic(
                                            Size( aSz.Width, aSz.Height ),
                                            aUnit, MAP_100TH_MM );
                        bDo = sal_True;
                    } catch( embed::NoVisualAreaSizeException& )
                    {
                        OSL_ENSURE( sal_False, "Can't get the original size of the object!" );
                    }
                }
            }
        }
        else if (SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pObj))
        {
            const Graphic& rGraphic = pSdrGrafObj->GetGraphic();

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
            const Rectangle aDrawRect(sdr::legacy::GetLogicRect(*pObj));

            pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
            sdr::legacy::ResizeSdrObject(*pObj, aDrawRect.TopLeft(),
                Fraction( aOriginalSize.Width(), aDrawRect.GetWidth() ),
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


#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif




