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
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <sfx2/app.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dataaccessdescriptor.hxx>
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
#include <sfx2/filedlghelper.hxx>
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
#include <gridwin.hxx>

#include <tools/urlobj.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

using namespace com::sun::star;

void ScTabViewShell::ConnectObject( SdrOle2Obj* pObj )
{
    // is called from paint

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    vcl::Window* pWin = GetActiveWin();

    // when already connected do not execute SetObjArea/SetSizeScale again

    SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
    if ( !pClient )
    {
        pClient = new ScClient( this, pWin, GetSdrView()->GetModel(), pObj );
        Rectangle aRect = pObj->GetLogicRect();
        Size aDrawSize = aRect.GetSize();

        Size aOleSize = pObj->GetOrigObjSize();

        Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       // compatible with SdrOle2Obj
        aScaleHeight.ReduceInaccurate(10);
        pClient->SetSizeScale(aScaleWidth,aScaleHeight);

        // visible section is only changed inplace!
        // the object area must be set after the scaling since it triggers the resizing
        aRect.SetSize( aOleSize );
        pClient->SetObjArea( aRect );

        static_cast<ScClient*>(pClient)->SetGrafEdit( nullptr );
    }
}

class PopupCallback : public cppu::WeakImplHelper<css::awt::XCallback>
{
    ScTabViewShell* m_pViewShell;
    SdrOle2Obj* m_pObject;

public:
    explicit PopupCallback(ScTabViewShell* pViewShell, SdrOle2Obj* pObject)
        : m_pViewShell(pViewShell)
        , m_pObject(pObject)
    {}

    // XCallback
    virtual void SAL_CALL notify(const css::uno::Any& aData) override
    {
        Rectangle aRect = m_pObject->GetLogicRect();
        m_pViewShell->DoDPFieldPopup(aRect.TopLeft(), aRect.GetSize());
    }

    virtual void SAL_CALL disposing()
    {
        m_pViewShell = nullptr;
    }
};

void ScTabViewShell::ActivateObject( SdrOle2Obj* pObj, long nVerb )
{
    // Do not leave the hint message box on top of the object
    RemoveHintWindow();

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    vcl::Window* pWin = GetActiveWin();
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
                // #i118485# center on BoundRect for activation,
                // OLE may be sheared/rotated now
                const Rectangle& rBoundRect = pObj->GetCurrentBoundRect();
                const Point aDelta(rBoundRect.Center() - aRect.Center());
                aRect.Move(aDelta.X(), aDelta.Y());
            }

            Size aDrawSize = aRect.GetSize();

            MapMode aMapMode( MapUnit::Map100thMM );
            Size aOleSize = pObj->GetOrigObjSize( &aMapMode );

            if ( pClient->GetAspect() != embed::Aspects::MSOLE_ICON
              && ( xObj->getStatus( pClient->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
            {
                //  scale must always be 1 - change VisArea if different from client size

                if ( aDrawSize != aOleSize )
                {
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( pClient->GetAspect() ) );
                    aOleSize = OutputDevice::LogicToLogic( aDrawSize,
                                            MapUnit::Map100thMM, aUnit );
                    awt::Size aSz( aOleSize.Width(), aOleSize.Height() );
                    xObj->setVisualAreaSize( pClient->GetAspect(), aSz );
                }
                Fraction aOne( 1, 1 );
                pClient->SetSizeScale( aOne, aOne );
            }
            else
            {
                //  calculate scale from client and VisArea size

                Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
                Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
                aScaleWidth.ReduceInaccurate(10);       // compatible with SdrOle2Obj
                aScaleHeight.ReduceInaccurate(10);
                pClient->SetSizeScale(aScaleWidth,aScaleHeight);
            }

            // visible section is only changed inplace!
            // the object area must be set after the scaling since it triggers the resizing
            aRect.SetSize( aOleSize );
            pClient->SetObjArea( aRect );

            static_cast<ScClient*>(pClient)->SetGrafEdit( nullptr );

            nErr = pClient->DoVerb( nVerb );
            bErrorShown = true;
            // SfxViewShell::DoVerb shows its error messages

            // attach listener to selection changes in chart that affect cell
            // ranges, so those can be highlighted
            // note: do that after DoVerb, so that the chart controller exists
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
                        if (xRangeHightlighter.is())
                        {
                            uno::Reference< view::XSelectionChangeListener > xListener(
                                new ScChartRangeSelectionListener( this ));
                            xRangeHightlighter->addSelectionChangeListener( xListener );
                        }
                        uno::Reference<chart2::data::XPopupRequest> xPopupRequest(xDataReceiver->getPopupRequest());
                        if (xPopupRequest.is())
                        {
                            uno::Reference<awt::XCallback> xCallback(new PopupCallback(this, pObj));
                            uno::Any aAny;
                            xPopupRequest->addCallback(xCallback, aAny);
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

    // #i118524# refresh handles to suppress for activated OLE
    if(GetSdrView())
    {
        GetSdrView()->AdjustMarkHdl();
    }
    //! SetDocumentName should already happen in Sfx ???
    //TODO/LATER: how "SetDocumentName"?
    //xIPObj->SetDocumentName( GetViewData().GetDocShell()->GetTitle() );
}

ErrCode ScTabViewShell::DoVerb(long nVerb)
{
    SdrView* pView = GetSdrView();
    if (!pView)
        return ERRCODE_SO_NOTIMPL;          // should not be

    SdrOle2Obj* pOle2Obj = nullptr;
    ErrCode nErr = ERRCODE_NONE;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
            pOle2Obj = static_cast<SdrOle2Obj*>(pObj);
    }

    if (pOle2Obj)
    {
        ActivateObject( pOle2Obj, nVerb );
    }
    else
    {
        OSL_FAIL("no object for Verb found");
    }

    return nErr;
}

void ScTabViewShell::DeactivateOle()
{
    // deactivate inplace editing if currently active

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = static_cast<ScClient*>(GetIPClient());
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

    // insertion of border for Chart is cancelled:
    FuPoor* pPoor = GetDrawFuncPtr();
    if ( pPoor && pPoor->GetSlotID() == SID_DRAW_CHART )
        GetViewData().GetDispatcher().Execute(SID_DRAW_CHART, SfxCallMode::SLOT | SfxCallMode::RECORD);

    MakeDrawLayer();

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    ScTabView*   pTabView  = GetViewData().GetView();
    vcl::Window*      pWin      = pTabView->GetActiveWin();
    ScDrawView*  pView     = pTabView->GetScDrawView();
    ScDocShell*  pDocSh    = GetViewData().GetDocShell();
    ScDocument&  rDoc      = pDocSh->GetDocument();
    SdrModel*    pDrModel  = pView->GetModel();

    switch ( nSlot )
    {
        case SID_INSERT_GRAPHIC:
            FuInsertGraphic(this, pWin, pView, pDrModel, rReq);
            // shell is set in MarkListHasChanged
            break;

        case SID_INSERT_AVMEDIA:
            FuInsertMedia(this, pWin, pView, pDrModel, rReq);
            // shell is set in MarkListHasChanged
            break;

        case SID_INSERT_DIAGRAM:
            FuInsertChart(this, pWin, pView, pDrModel, rReq);
            break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_SMATH:
        case SID_INSERT_FLOATINGFRAME:
            FuInsertOLE(this, pWin, pView, pDrModel, rReq);
            break;

        case SID_INSERT_DIAGRAM_FROM_FILE:
            try
            {
                sfx2::FileDialogHelper aDlg(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                        FileDialogFlags::NONE, OUString("com.sun.star.chart2.ChartDocument"));
                if(aDlg.Execute() == ERRCODE_NONE )
                {
                    INetURLObject aURLObj( aDlg.GetPath() );
                    OUString aURL = aURLObj.GetURLNoPass();
                    FuInsertChartFromFile(this, pWin, pView, pDrModel, rReq, aURL);
                }
            }
            catch (const uno::Exception& e)
            {
                SAL_WARN( "sc", "Cannot Insert Chart: " << e.Message);
            }
            break;

        case SID_OBJECTRESIZE:
            {
                //         the server would like to change the client size

                SfxInPlaceClient* pClient = GetIPClient();

                if ( pClient && pClient->IsObjectInPlaceActive() )
                {
                    const SfxRectangleItem& rRect =
                        static_cast<const SfxRectangleItem&>(rReq.GetArgs()->Get(SID_OBJECTRESIZE));
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
                                if ( static_cast<SdrOle2Obj*>(pObj)->GetObjRef().is() )
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
                ScopedVclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog( pWin, rDoc.GetLinkManager() ));
                if ( pDlg )
                {
                    pDlg->Execute();
                    rBindings.Invalidate( nSlot );
                    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
                    rReq.Done();
                }
            }
            break;

        case SID_FM_CREATE_FIELDCONTROL:
            {
                const SfxUnoAnyItem* pDescriptorItem = rReq.GetArg<SfxUnoAnyItem>(SID_FM_DATACCESS_DESCRIPTOR);
                OSL_ENSURE( pDescriptorItem, "SID_FM_CREATE_FIELDCONTROL: invalid request args!" );

                if(pDescriptorItem)
                {
                    //! merge with ScViewFunc::PasteDataFormat (SotClipboardFormatId::SBA_FIELDDATAEXCHANGE)?

                    ScDrawView* pDrView = GetScDrawView();
                    SdrPageView* pPageView = pDrView ? pDrView->GetSdrPageView() : nullptr;
                    if(pPageView)
                    {
                        svx::ODataAccessDescriptor aDescriptor(pDescriptorItem->GetValue());
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

                            // controls must be on control layer, groups on front layer
                            if ( dynamic_cast<const SdrUnoObj*>( pNewDBField) !=  nullptr )
                                pNewDBField->NbcSetLayer(SC_LAYER_CONTROLS);
                            else
                                pNewDBField->NbcSetLayer(SC_LAYER_FRONT);
                            if (dynamic_cast<const SdrObjGroup*>( pNewDBField) !=  nullptr)
                            {
                                SdrObjListIter aIter( *pNewDBField, SdrIterMode::DeepWithGroups );
                                SdrObject* pSubObj = aIter.Next();
                                while (pSubObj)
                                {
                                    if ( dynamic_cast<const SdrUnoObj*>( pSubObj) !=  nullptr )
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
    bool bTabProt = GetViewData().GetDocument()->IsTabProtected(GetViewData().GetTabNo());
    ScDocShell* pDocShell = GetViewData().GetDocShell();
    bool bShared = pDocShell && pDocShell->IsDocShared();

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
            case SID_INSERT_FLOATINGFRAME:
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
                    if (GetViewData().GetDocument()->GetLinkManager()->GetLinks().empty())
                        rSet.DisableItem( SID_LINKS );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScTabViewShell::ExecuteUndo(SfxRequest& rReq)
{
    SfxShell* pSh = GetViewData().GetDispatcher().GetShell(0);
    ::svl::IUndoManager* pUndoManager = pSh->GetUndoManager();

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    ScDocShell* pDocSh = GetViewData().GetDocShell();

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
                if ( pReqArgs && pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                    nCount = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                // lock paint for more than one cell undo action (not for editing within a cell)
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
                    // no need to handle. By definition, the UndoManager handled this by clearing the
                    // Undo/Redo stacks
                }

                if ( bLockPaint )
                    pDocSh->UnlockPaint();

                GetViewFrame()->GetBindings().InvalidateAll(false);
            }
            break;
//      default:
//          GetViewFrame()->ExecuteSlot( rReq );
    }
}

void ScTabViewShell::GetUndoState(SfxItemSet &rSet)
{
    SfxShell* pSh = GetViewData().GetDispatcher().GetShell(0);
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
                // get state from sfx view frame
                GetViewFrame()->GetSlotState( nWhich, nullptr, &rSet );
        }

        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
