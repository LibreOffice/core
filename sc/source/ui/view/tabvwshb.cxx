/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <sfx2/app.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/pfiledlg.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdview.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/fontworkbar.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/soerr.hxx>
#include <svl/rectitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <sot/exchange.hxx>
#include <tools/diagnose_ex.h>

#include "tabvwsh.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "client.hxx"
#include "fuinsert.hxx"
#include "docsh.hxx"
#include "chartarr.hxx"
#include "drawview.hxx"
#include "ChartRangeSelectionListener.hxx"

#include <tools/urlobj.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace com::sun::star;



void ScTabViewShell::ConnectObject( SdrOle2Obj* pObj )
{
    

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    Window* pWin = GetActiveWin();

    

    SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
    if ( !pClient )
    {
        pClient = new ScClient( this, pWin, GetSdrView()->GetModel(), pObj );
        Rectangle aRect = pObj->GetLogicRect();
        Size aDrawSize = aRect.GetSize();

        Size aOleSize = pObj->GetOrigObjSize();

        Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       
        aScaleHeight.ReduceInaccurate(10);
        pClient->SetSizeScale(aScaleWidth,aScaleHeight);

        
        
        aRect.SetSize( aOleSize );
        pClient->SetObjArea( aRect );

        ((ScClient*)pClient)->SetGrafEdit( NULL );
    }
}

bool ScTabViewShell::ActivateObject( SdrOle2Obj* pObj, long nVerb )
{
    
    RemoveHintWindow();

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    Window* pWin = GetActiveWin();
    ErrCode nErr = ERRCODE_NONE;
    bool bErrorShown = false;

    {
        SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
        if ( !pClient )
            pClient = new ScClient( this, pWin, GetSdrView()->GetModel(), pObj );

        if ( !(nErr & ERRCODE_ERROR_MASK) && xObj.is() )
        {
            Rectangle aRect = pObj->GetLogicRect();

            {
                
                
                const Rectangle& rBoundRect = pObj->GetCurrentBoundRect();
                const Point aDelta(rBoundRect.Center() - aRect.Center());
                aRect.Move(aDelta.X(), aDelta.Y());
            }

            Size aDrawSize = aRect.GetSize();

            MapMode aMapMode( MAP_100TH_MM );
            Size aOleSize = pObj->GetOrigObjSize( &aMapMode );

            if ( pClient->GetAspect() != embed::Aspects::MSOLE_ICON
              && ( xObj->getStatus( pClient->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
            {
                

                if ( aDrawSize != aOleSize )
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( pClient->GetAspect() ) );
                    aOleSize = OutputDevice::LogicToLogic( aDrawSize,
                                            MAP_100TH_MM, aUnit );
                    awt::Size aSz( aOleSize.Width(), aOleSize.Height() );
                    xObj->setVisualAreaSize( pClient->GetAspect(), aSz );
                }
                Fraction aOne( 1, 1 );
                pClient->SetSizeScale( aOne, aOne );
            }
            else
            {
                

                Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
                Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
                aScaleWidth.ReduceInaccurate(10);       
                aScaleHeight.ReduceInaccurate(10);
                pClient->SetSizeScale(aScaleWidth,aScaleHeight);
            }

            
            
            aRect.SetSize( aOleSize );
            pClient->SetObjArea( aRect );

            ((ScClient*)pClient)->SetGrafEdit( NULL );

            nErr = pClient->DoVerb( nVerb );
            bErrorShown = true;
            

            
            
            
            if ( SvtModuleOptions().IsChart() )
            {
                SvGlobalName aObjClsId ( xObj->getClassID() );
                if (SotExchange::IsChart( aObjClsId ))
                {
                    try
                    {
                        uno::Reference < embed::XComponentSupplier > xSup( xObj, uno::UNO_QUERY_THROW );
                        uno::Reference< chart2::data::XDataReceiver > xDataReceiver(
                            xSup->getComponent(), uno::UNO_QUERY_THROW );
                        uno::Reference< chart2::data::XRangeHighlighter > xRangeHightlighter(
                            xDataReceiver->getRangeHighlighter());
                        if( xRangeHightlighter.is())
                        {
                            uno::Reference< view::XSelectionChangeListener > xListener(
                                new ScChartRangeSelectionListener( this ));
                            xRangeHightlighter->addSelectionChangeListener( xListener );
                        }
                    }
                    catch( const uno::Exception & )
                    {
                        OSL_FAIL( "Exception caught while querying chart" );
                    }
                }
            }
        }
    }
    if (nErr != ERRCODE_NONE && !bErrorShown)
        ErrorHandler::HandleError(nErr);

    
    if(GetSdrView())
    {
        GetSdrView()->AdjustMarkHdl();
    }
    
    
    

    return ( !(nErr & ERRCODE_ERROR_MASK) );
}

ErrCode ScTabViewShell::DoVerb(long nVerb)
{
    SdrView* pView = GetSdrView();
    if (!pView)
        return ERRCODE_SO_NOTIMPL;          

    SdrOle2Obj* pOle2Obj = NULL;
    ErrCode nErr = ERRCODE_NONE;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
            pOle2Obj = (SdrOle2Obj*) pObj;
    }

    if (pOle2Obj)
    {
        ActivateObject( pOle2Obj, nVerb );
    }
    else
    {
        OSL_FAIL("kein Objekt fuer Verb gefunden");
    }

    return nErr;
}

void ScTabViewShell::DeactivateOle()
{
    

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = (ScClient*) GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() && !bUnoRefDialog )
        pClient->DeactivateObject();
}

void ScTabViewShell::ExecDrawIns(SfxRequest& rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    if (nSlot != SID_OBJECTRESIZE )
    {
        SC_MOD()->InputEnterHandler();
        UpdateInputHandler();
    }

    
    FuPoor* pPoor = GetDrawFuncPtr();
    if ( pPoor && pPoor->GetSlotID() == SID_DRAW_CHART )
        GetViewData()->GetDispatcher().Execute(SID_DRAW_CHART, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

    MakeDrawLayer();

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    ScTabView*   pTabView  = GetViewData()->GetView();
    Window*      pWin      = pTabView->GetActiveWin();
    ScDrawView*  pView     = pTabView->GetScDrawView();
    ScDocShell*  pDocSh    = GetViewData()->GetDocShell();
    ScDocument*  pDoc      = pDocSh->GetDocument();
    SdrModel*    pDrModel  = pView->GetModel();

    switch ( nSlot )
    {
        case SID_INSERT_GRAPHIC:
            FuInsertGraphic(this, pWin, pView, pDrModel, rReq);
            
            break;

        case SID_INSERT_AVMEDIA:
            FuInsertMedia(this, pWin, pView, pDrModel, rReq);
            
            break;

        case SID_INSERT_DIAGRAM:
            FuInsertChart(this, pWin, pView, pDrModel, rReq);
            break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_SMATH:
        case SID_INSERT_FLOATINGFRAME:
            FuInsertOLE(this, pWin, pView, pDrModel, rReq);
            break;

        case SID_INSERT_DIAGRAM_FROM_FILE:
            {
                sfx2::FileDialogHelper aDlg(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                        0, OUString("com.sun.star.chart2.ChartDocument"));
                if(aDlg.Execute() == ERRCODE_NONE )
                {
                    INetURLObject aURLObj( aDlg.GetPath() );
                    OUString aURL = aURLObj.GetURLNoPass();
                    FuInsertChartFromFile(this, pWin, pView, pDrModel, rReq, aURL);
                }
            }
            break;

        case SID_OBJECTRESIZE:
            {
                

                SfxInPlaceClient* pClient = GetIPClient();

                if ( pClient && pClient->IsObjectInPlaceActive() )
                {
                    const SfxRectangleItem& rRect =
                        (SfxRectangleItem&)rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                    Rectangle aRect( pWin->PixelToLogic( rRect.GetValue() ) );

                    if ( pView->AreObjectsMarked() )
                    {
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrMark* pMark = rMarkList.GetMark(0);
                            SdrObject* pObj = pMark->GetMarkedSdrObj();

                            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

                            if (nSdrObjKind == OBJ_OLE2)
                            {
                                if ( ( (SdrOle2Obj*) pObj)->GetObjRef().is() )
                                {
                                    pObj->SetLogicRect(aRect);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case SID_LINKS:
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                SfxAbstractLinksDialog* pDlg = pFact->CreateLinksDialog( pWin, pDoc->GetLinkManager() );
                if ( pDlg )
                {
                    pDlg->Execute();
                    rBindings.Invalidate( nSlot );
                    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     
                    rReq.Done();
                }
            }
            break;

        
        case SID_FM_CREATE_FIELDCONTROL:
            {
                SFX_REQUEST_ARG( rReq, pDescriptorItem, SfxUnoAnyItem, SID_FM_DATACCESS_DESCRIPTOR, false );
                OSL_ENSURE( pDescriptorItem, "SID_FM_CREATE_FIELDCONTROL: invalid request args!" );

                if(pDescriptorItem)
                {
                    

                    ScDrawView* pDrView = GetScDrawView();
                    SdrPageView* pPageView = pDrView ? pDrView->GetSdrPageView() : NULL;
                    if(pPageView)
                    {
                        ::svx::ODataAccessDescriptor aDescriptor(pDescriptorItem->GetValue());
                        SdrObject* pNewDBField = pDrView->CreateFieldControl(aDescriptor);

                        if(pNewDBField)
                        {
                            Rectangle aVisArea = pWin->PixelToLogic(Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
                            Point aObjPos(aVisArea.Center());
                            Size aObjSize(pNewDBField->GetLogicRect().GetSize());
                            aObjPos.X() -= aObjSize.Width() / 2;
                            aObjPos.Y() -= aObjSize.Height() / 2;
                            Rectangle aNewObjectRectangle(aObjPos, aObjSize);

                            pNewDBField->SetLogicRect(aNewObjectRectangle);

                            
                            if ( pNewDBField->ISA(SdrUnoObj) )
                                pNewDBField->NbcSetLayer(SC_LAYER_CONTROLS);
                            else
                                pNewDBField->NbcSetLayer(SC_LAYER_FRONT);
                            if (pNewDBField->ISA(SdrObjGroup))
                            {
                                SdrObjListIter aIter( *pNewDBField, IM_DEEPWITHGROUPS );
                                SdrObject* pSubObj = aIter.Next();
                                while (pSubObj)
                                {
                                    if ( pSubObj->ISA(SdrUnoObj) )
                                        pSubObj->NbcSetLayer(SC_LAYER_CONTROLS);
                                    else
                                        pSubObj->NbcSetLayer(SC_LAYER_FRONT);
                                    pSubObj = aIter.Next();
                                }
                            }

                            pView->InsertObjectAtView(pNewDBField, *pPageView);
                        }
                    }
                }
                rReq.Done();
            }
            break;

        case SID_FONTWORK_GALLERY_FLOATER:
            svx::FontworkBar::execute( pView, rReq, GetViewFrame()->GetBindings() );
            rReq.Ignore();
            break;
    }
}

void ScTabViewShell::GetDrawInsState(SfxItemSet &rSet)
{
    bool bOle = GetViewFrame()->GetFrame().IsInPlace();
    bool bTabProt = GetViewData()->GetDocument()->IsTabProtected(GetViewData()->GetTabNo());
    ScDocShell* pDocShell = ( GetViewData() ? GetViewData()->GetDocShell() : NULL );
    bool bShared = ( pDocShell ? pDocShell->IsDocShared() : false );

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_DIAGRAM:
                if ( bOle || bTabProt || !SvtModuleOptions().IsChart() || bShared )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SMATH:
                if ( bOle || bTabProt || !SvtModuleOptions().IsMath() || bShared )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_OBJECT:
            case SID_INSERT_PLUGIN:
            case SID_INSERT_FLOATINGFRAME:
                if ( bOle || bTabProt || bShared )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SOUND:
            case SID_INSERT_VIDEO:
                 /* #i102735# discussed with NN: removed for performance reasons
                 || !SvxPluginFileDlg::IsAvailable(nWhich)
                 */
                if ( bOle || bTabProt || bShared )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_GRAPHIC:
            case SID_INSERT_AVMEDIA:
            case SID_FONTWORK_GALLERY_FLOATER:
                if ( bTabProt || bShared )
                    rSet.DisableItem( nWhich );
                break;

            case SID_LINKS:
                {
                    if (GetViewData()->GetDocument()->GetLinkManager()->GetLinks().empty())
                        rSet.DisableItem( SID_LINKS );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




void ScTabViewShell::ExecuteUndo(SfxRequest& rReq)
{
    SfxShell* pSh = GetViewData()->GetDispatcher().GetShell(0);
    ::svl::IUndoManager* pUndoManager = pSh->GetUndoManager();

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_UNDO:
        case SID_REDO:
            if ( pUndoManager )
            {
                bool bIsUndo = ( nSlot == SID_UNDO );

                sal_uInt16 nCount = 1;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlot, true, &pItem ) == SFX_ITEM_SET )
                    nCount = ((const SfxUInt16Item*)pItem)->GetValue();

                
                bool bLockPaint = ( nCount > 1 && pUndoManager == GetUndoManager() );
                if ( bLockPaint )
                    pDocSh->LockPaint();

                try
                {
                    for (sal_uInt16 i=0; i<nCount; i++)
                    {
                        if ( bIsUndo )
                            pUndoManager->Undo();
                        else
                            pUndoManager->Redo();
                    }
                }
                catch ( const uno::Exception& )
                {
                    
                    
                }

                if ( bLockPaint )
                    pDocSh->UnlockPaint();

                GetViewFrame()->GetBindings().InvalidateAll(false);
            }
            break;


    }
}

void ScTabViewShell::GetUndoState(SfxItemSet &rSet)
{
    SfxShell* pSh = GetViewData()->GetDispatcher().GetShell(0);
    ::svl::IUndoManager* pUndoManager = pSh->GetUndoManager();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_GETUNDOSTRINGS:
            case SID_GETREDOSTRINGS:
                {
                    SfxStringListItem aStrLst( nWhich );
                    if ( pUndoManager )
                    {
                        std::vector<OUString> &aList = aStrLst.GetList();
                        bool bIsUndo = ( nWhich == SID_GETUNDOSTRINGS );
                        size_t nCount = bIsUndo ? pUndoManager->GetUndoActionCount() : pUndoManager->GetRedoActionCount();
                        for (size_t i=0; i<nCount; ++i)
                        {
                            aList.push_back( bIsUndo ? pUndoManager->GetUndoActionComment(i) :
                                                       pUndoManager->GetRedoActionComment(i) );
                        }
                    }
                    rSet.Put( aStrLst );
                }
                break;
            default:
                
                GetViewFrame()->GetSlotState( nWhich, NULL, &rSet );
        }

        nWhich = aIter.NextWhich();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
