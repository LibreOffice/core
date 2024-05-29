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

#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <vcl/errinf.hxx>
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
#include <svtools/soerr.hxx>
#include <svl/rectitem.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>
#include <svl/whiter.hxx>
#include <svtools/strings.hrc>
#include <unotools/moduleoptions.hxx>
#include <sot/exchange.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <tabvwsh.hxx>
#include <scmod.hxx>
#include <document.hxx>
#include <sc.hrc>
#include <client.hxx>
#include <fuinsert.hxx>
#include <docsh.hxx>
#include <drawview.hxx>
#include <ChartRangeSelectionListener.hxx>
#include <gridwin.hxx>
#include <undomanager.hxx>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <svx/svdpagv.hxx>
#include <o3tl/temporary.hxx>
#include <officecfg/Office/Common.hxx>

#include <comphelper/lok.hxx>

using namespace com::sun::star;

void ScTabViewShell::ConnectObject( const SdrOle2Obj* pObj )
{
    // is called from paint

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    vcl::Window* pWin = GetActiveWin();

    // when already connected do not execute SetObjArea/SetSizeScale again

    SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
    if ( pClient )
        return;

    pClient = new ScClient( this, pWin, &GetScDrawView()->GetModel(), pObj );
    ScViewData& rViewData = GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    bool bNegativeX = comphelper::LibreOfficeKit::isActive() && rDoc.IsNegativePage(rViewData.GetTabNo());
    if (bNegativeX)
        pClient->SetNegativeX(true);

    tools::Rectangle aRect = pObj->GetLogicRect();
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
}

namespace {

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
        uno::Sequence<beans::PropertyValue> aProperties;
        if (!(aData >>= aProperties))
            return;

        awt::Rectangle xRectangle;
        sal_Int32 dimensionIndex = 0;
        OUString sPivotTableName("DataPilot1");

        for (beans::PropertyValue const& rProperty : aProperties)
        {
            if (rProperty.Name == "Rectangle")
                rProperty.Value >>= xRectangle;
            if (rProperty.Name == "DimensionIndex")
                rProperty.Value >>= dimensionIndex;
            if (rProperty.Name == "PivotTableName")
                rProperty.Value >>= sPivotTableName;
        }

        tools::Rectangle aChartRect = m_pObject->GetLogicRect();

        Point aPoint(xRectangle.X  + aChartRect.Left(), xRectangle.Y + aChartRect.Top());
        Size aSize(xRectangle.Width, xRectangle.Height);

        m_pViewShell->DoDPFieldPopup(sPivotTableName, dimensionIndex, aPoint, aSize);
    }
};

}

void ScTabViewShell::ActivateObject(SdrOle2Obj* pObj, sal_Int32 nVerb)
{
    // Do not leave the hint message box on top of the object
    RemoveHintWindow();

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    vcl::Window* pWin = GetActiveWin();
    ErrCodeMsg nErr = ERRCODE_NONE;
    bool bErrorShown = false;

    {
        ScViewData& rViewData = GetViewData();
        ScDocShell* pDocSh = rViewData.GetDocShell();
        ScDocument& rDoc = pDocSh->GetDocument();
        bool bNegativeX = comphelper::LibreOfficeKit::isActive() && rDoc.IsNegativePage(rViewData.GetTabNo());
        SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
        if ( !pClient )
            pClient = new ScClient( this, pWin, &GetScDrawView()->GetModel(), pObj );

        if (bNegativeX)
            pClient->SetNegativeX(true);

        if ( (sal_uInt32(nErr.GetCode()) & ERRCODE_ERROR_MASK) == 0 && xObj.is() )
        {
            tools::Rectangle aRect = pObj->GetLogicRect();

            {
                // #i118485# center on BoundRect for activation,
                // OLE may be sheared/rotated now
                const tools::Rectangle& rBoundRect = pObj->GetCurrentBoundRect();
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
                                MapMode(MapUnit::Map100thMM), MapMode(aUnit));
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
                        uno::Reference< chart2::data::XRangeHighlighter > xRangeHighlighter(
                            xDataReceiver->getRangeHighlighter());
                        if (xRangeHighlighter.is())
                        {
                            uno::Reference< view::XSelectionChangeListener > xListener(
                                new ScChartRangeSelectionListener( this ));
                            xRangeHighlighter->addSelectionChangeListener( xListener );
                        }
                        uno::Reference<awt::XRequestCallback> xPopupRequest(xDataReceiver->getPopupRequest());
                        if (xPopupRequest.is())
                        {
                            uno::Reference<awt::XCallback> xCallback(new PopupCallback(this, pObj));
                            uno::Any aAny;
                            xPopupRequest->addCallback(xCallback, aAny);
                        }
                    }
                    catch( const uno::Exception & )
                    {
                        TOOLS_WARN_EXCEPTION( "sc", "Exception caught while querying chart" );
                    }
                }
            }
        }
    }
    if (nErr != ERRCODE_NONE && !bErrorShown)
        ErrorHandler::HandleError(nErr);

    // #i118524# refresh handles to suppress for activated OLE
    if(GetScDrawView())
    {
        GetScDrawView()->AdjustMarkHdl();
    }
    //! SetDocumentName should already happen in Sfx ???
    //TODO/LATER: how "SetDocumentName"?
    //xIPObj->SetDocumentName( GetViewData().GetDocShell()->GetTitle() );
}

ErrCode ScTabViewShell::DoVerb(sal_Int32 nVerb)
{
    SdrView* pView = GetScDrawView();
    if (!pView)
        return ERRCODE_SO_NOTIMPL;          // should not be

    SdrOle2Obj* pOle2Obj = nullptr;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() == SdrObjKind::OLE2)
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

    return ERRCODE_NONE;
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

void ScTabViewShell::SetInsertWizardUndoMark()
{
    assert(m_InsertWizardUndoMark == MARK_INVALID);
    m_InsertWizardUndoMark = GetUndoManager()->MarkTopUndoAction();
}

IMPL_LINK( ScTabViewShell, DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, pEvent, void )
{
    assert(m_InsertWizardUndoMark != MARK_INVALID);
    UndoStackMark nInsertWizardUndoMark = m_InsertWizardUndoMark;
    m_InsertWizardUndoMark = MARK_INVALID;
    if( pEvent->DialogResult == ui::dialogs::ExecutableDialogResults::CANCEL )
    {
        ScTabView* pTabView = GetViewData().GetView();
        ScDrawView* pView = pTabView->GetScDrawView();
        ScViewData& rData = GetViewData();
        ScDocShell* pScDocSh = rData.GetDocShell();
        ScDocument& rScDoc = pScDocSh->GetDocument();
        // leave OLE inplace mode and unmark
        OSL_ASSERT( pView );
        DeactivateOle();
        pView->UnMarkAll();

        auto pUndoManager = rScDoc.GetUndoManager();
        if (pUndoManager->GetRedoActionCount())
        {
            pUndoManager->RemoveMark(nInsertWizardUndoMark);
        }
        else
        {
            pUndoManager->UndoMark(nInsertWizardUndoMark);
            pUndoManager->ClearRedo();
        }

        // leave the draw shell
        SetDrawShell( false );

        // reset marked cell area
        ScMarkData aMark = GetViewData().GetMarkData();
        GetViewData().GetViewShell()->SetMarkData(aMark);
    }
    else
    {
        OSL_ASSERT( pEvent->DialogResult == ui::dialogs::ExecutableDialogResults::OK );
        //@todo maybe move chart to different table
    }
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

    ScTabView*   pTabView  = GetViewData().GetView();
    vcl::Window*      pWin      = pTabView->GetActiveWin();
    ScDrawView*  pView     = pTabView->GetScDrawView();
    ScDocShell*  pDocSh    = GetViewData().GetDocShell();
    ScDocument&  rDoc      = pDocSh->GetDocument();
    SdrModel& rModel = pView->GetModel();

    switch ( nSlot )
    {
        case SID_INSERT_GRAPHIC:
            FuInsertGraphic(*this, pWin, pView, &rModel, rReq);
            // shell is set in MarkListHasChanged
            break;

        case SID_INSERT_AVMEDIA:
            FuInsertMedia(*this, pWin, pView, &rModel, rReq);
            // shell is set in MarkListHasChanged
            break;

        case SID_INSERT_DIAGRAM:
            FuInsertChart(*this, pWin, pView, &rModel, rReq, LINK( this, ScTabViewShell, DialogClosedHdl ));
            if (comphelper::LibreOfficeKit::isActive())
                pDocSh->SetModified();
            break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_SMATH:
        case SID_INSERT_FLOATINGFRAME:
            FuInsertOLE(*this, pWin, pView, &rModel, rReq);
            break;

        case SID_INSERT_SIGNATURELINE:
        case SID_EDIT_SIGNATURELINE:
            {
                const uno::Reference<frame::XModel> xModel( GetViewData().GetDocShell()->GetBaseModel() );

                VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
                VclPtr<AbstractSignatureLineDialog> pDialog(pFact->CreateSignatureLineDialog(
                    pWin->GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_SIGNATURELINE));
                auto xRequest = std::make_shared<SfxRequest>(rReq);
                rReq.Ignore(); // the 'old' request is not relevant any more
                pDialog->StartExecuteAsync(
                    [pDialog, xRequest=std::move(xRequest)] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                            pDialog->Apply();
                        pDialog->disposeOnce();
                        xRequest->Done();
                    }
                );
                break;
            }

        case SID_SIGN_SIGNATURELINE:
            {
                const uno::Reference<frame::XModel> xModel(
                    GetViewData().GetDocShell()->GetBaseModel());

                VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
                VclPtr<AbstractSignSignatureLineDialog> pDialog(
                    pFact->CreateSignSignatureLineDialog(GetFrameWeld(), xModel));
                pDialog->StartExecuteAsync(
                    [pDialog] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                            pDialog->Apply();
                        pDialog->disposeOnce();
                    }
                );
                break;
            }

        case SID_INSERT_QRCODE:
        case SID_EDIT_QRCODE:
            {
                const uno::Reference<frame::XModel> xModel( GetViewData().GetDocShell()->GetBaseModel() );

                VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractQrCodeGenDialog> pDialog(pFact->CreateQrCodeGenDialog(
                    pWin->GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_QRCODE));
                pDialog->Execute();
                break;
            }

            case SID_ADDITIONS_DIALOG:
            {
                OUString sAdditionsTag = "";

                const SfxStringItem* pStringArg = rReq.GetArg<SfxStringItem>(FN_PARAM_ADDITIONS_TAG);
                if (pStringArg)
                    sAdditionsTag = pStringArg->GetValue();

                VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
                VclPtr<AbstractAdditionsDialog> pDialog(
                    pFact->CreateAdditionsDialog(pWin->GetFrameWeld(), sAdditionsTag));
                pDialog->StartExecuteAsync(
                    [pDialog] (sal_Int32 /*nResult*/)->void
                    {
                        pDialog->disposeOnce();
                    }
                );
                break;
            }

            case SID_OBJECTRESIZE:
            {
                //         the server would like to change the client size

                SfxInPlaceClient* pClient = GetIPClient();

                if ( pClient && pClient->IsObjectInPlaceActive() )
                {
                    const SfxRectangleItem& rRect = rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                    tools::Rectangle aRect( pWin->PixelToLogic( rRect.GetValue() ) );

                    if ( pView->AreObjectsMarked() )
                    {
                        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrMark* pMark = rMarkList.GetMark(0);
                            SdrObject* pObj = pMark->GetMarkedSdrObj();

                            SdrObjKind nSdrObjKind = pObj->GetObjIdentifier();

                            if (nSdrObjKind == SdrObjKind::OLE2)
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
                if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
                {
                    std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(
                        nullptr, VclMessageType::Warning, VclButtonsType::Ok,
                        SvtResId(STR_WARNING_EXTERNAL_LINK_EDIT_DISABLED)));
                    xError->run();
                    break;
                }

                VclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog(pWin->GetFrameWeld(), rDoc.GetLinkManager()));
                auto xRequest = std::make_shared<SfxRequest>(rReq);
                rReq.Ignore(); // the 'old' request is not relevant any more
                pDlg->StartExecuteAsync(
                    [this, pDlg, xRequest=std::move(xRequest)] (sal_Int32 /*nResult*/)->void
                    {
                        GetViewFrame().GetBindings().Invalidate( SID_LINKS );
                        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
                        pDlg->disposeOnce();
                        xRequest->Done();
                    }
                );
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
                        rtl::Reference<SdrObject> pNewDBField = pDrView->CreateFieldControl(aDescriptor);

                        if(pNewDBField)
                        {
                            tools::Rectangle aVisArea = pWin->PixelToLogic(tools::Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
                            Point aObjPos(aVisArea.Center());
                            Size aObjSize(pNewDBField->GetLogicRect().GetSize());
                            aObjPos.AdjustX( -(aObjSize.Width() / 2) );
                            aObjPos.AdjustY( -(aObjSize.Height() / 2) );
                            tools::Rectangle aNewObjectRectangle(aObjPos, aObjSize);

                            pNewDBField->SetLogicRect(aNewObjectRectangle);

                            // controls must be on control layer, groups on front layer
                            if ( dynamic_cast<const SdrUnoObj*>( pNewDBField.get() ) !=  nullptr )
                                pNewDBField->NbcSetLayer(SC_LAYER_CONTROLS);
                            else
                                pNewDBField->NbcSetLayer(SC_LAYER_FRONT);
                            if (dynamic_cast<const SdrObjGroup*>( pNewDBField.get() ) !=  nullptr)
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

                            pView->InsertObjectAtView(pNewDBField.get(), *pPageView);
                        }
                    }
                }
                rReq.Done();
            }
            break;

        case SID_FONTWORK_GALLERY_FLOATER:
            svx::FontworkBar::execute(*pView, rReq, GetViewFrame().GetBindings());
            rReq.Ignore();
            break;
    }
}

void ScTabViewShell::GetDrawInsState(SfxItemSet &rSet)
{
    bool bOle = GetViewFrame().GetFrame().IsInPlace();
    bool bTabProt = GetViewData().GetDocument().IsTabProtected(GetViewData().GetTabNo());
    ScDocShell* pDocShell = GetViewData().GetDocShell();
    bool bShared = pDocShell && pDocShell->IsDocShared();
    SdrView* pSdrView = GetScDrawView();

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

            case SID_INSERT_AVMEDIA:
            case SID_FONTWORK_GALLERY_FLOATER:
                if ( bTabProt || bShared )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SIGNATURELINE:
                if ( bTabProt || bShared || (pSdrView && pSdrView->GetMarkedObjectList().GetMarkCount() != 0))
                    rSet.DisableItem( nWhich );
                break;
            case SID_EDIT_SIGNATURELINE:
            case SID_SIGN_SIGNATURELINE:
                if (!IsSignatureLineSelected() || IsSignatureLineSigned())
                    rSet.DisableItem(nWhich);
                break;

            case SID_INSERT_QRCODE:
                if ( bTabProt || bShared || (pSdrView && pSdrView->GetMarkedObjectList().GetMarkCount() != 0))
                    rSet.DisableItem( nWhich );
                break;
            case SID_EDIT_QRCODE:
                if (!IsQRCodeSelected())
                    rSet.DisableItem(nWhich);
                break;

            case SID_INSERT_GRAPHIC:
                if (bTabProt || bShared)
                {
                    // do not disable 'insert graphic' item if the currently marked area is editable (not protected)
                    // if there is no marked area, check the current cell
                    bool bDisableInsertImage = true;
                    ScMarkData& rMark = GetViewData().GetMarkData();
                    if (!rMark.GetMarkedRanges().empty() && GetViewData().GetDocument().IsSelectionEditable(rMark))
                        bDisableInsertImage = false;
                    else
                    {
                        if (GetViewData().GetDocument().IsBlockEditable
                            (GetViewData().GetTabNo(), GetViewData().GetCurX(), GetViewData().GetCurY(), GetViewData().GetCurX(), GetViewData().GetCurY()))
                        {
                            bDisableInsertImage = false;
                        }
                    }

                    if (bDisableInsertImage)
                        rSet.DisableItem(nWhich);
                }
                break;

            case SID_LINKS:
                {
                    if (GetViewData().GetDocument().GetLinkManager()->GetLinks().empty())
                        rSet.DisableItem( SID_LINKS );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

bool ScTabViewShell::IsSignatureLineSelected()
{
    SdrView* pSdrView = GetScDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    return pGraphic->isSignatureLine();
}

bool ScTabViewShell::IsQRCodeSelected()
{
    SdrView* pSdrView = GetScDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    if(pGraphic->getQrCode())
    {
        return true;
    }
    else{
        return false;
    }
}

bool ScTabViewShell::IsSignatureLineSigned()
{
    SdrView* pSdrView = GetScDrawView();
    if (!pSdrView)
        return false;

    if (pSdrView->GetMarkedObjectList().GetMarkCount() != 1)
        return false;

    SdrObject* pPickObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    if (!pPickObj)
        return false;

    SdrGrafObj* pGraphic = dynamic_cast<SdrGrafObj*>(pPickObj);
    if (!pGraphic)
        return false;

    return pGraphic->isSignatureLineSigned();
}

void ScTabViewShell::ExecuteUndo(SfxRequest& rReq)
{
    SfxShell* pSh = GetViewData().GetDispatcher().GetShell(0);
    if (!pSh)
        return;

    ScUndoManager* pUndoManager = static_cast<ScUndoManager*>(pSh->GetUndoManager());

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

                // Repair mode: allow undo/redo of all undo actions, even if access would
                // be limited based on the view shell ID.
                bool bRepair = false;
                if (pReqArgs && pReqArgs->GetItemState(SID_REPAIRPACKAGE, false, &pItem) == SfxItemState::SET)
                    bRepair = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                sal_uInt16 nUndoOffset = 0;
                if (comphelper::LibreOfficeKit::isActive() && !bRepair)
                {
                    SfxUndoAction* pAction = nullptr;
                    if (bIsUndo)
                    {
                        if (pUndoManager->GetUndoActionCount() != 0)
                            pAction = pUndoManager->GetUndoAction();
                    }
                    else
                    {
                        if (pUndoManager->GetRedoActionCount() != 0)
                            pAction = pUndoManager->GetRedoAction();
                    }
                    if (pAction)
                    {
                        // If another view created the undo action, prevent undoing it from this view.
                        // Unless we know that the other view's undo action is independent from us.
                        ViewShellId nViewShellId = GetViewShellId();
                        if (pAction->GetViewShellId() != nViewShellId)
                        {
                            sal_uInt16 nOffset = 0;
                            if (pUndoManager->IsViewUndoActionIndependent(this, nOffset))
                            {
                                // Execute the undo with an offset: don't undo the top action, but an
                                // earlier one, since it's independent and that belongs to our view.
                                nUndoOffset += nOffset;
                            }
                            else
                            {
                                rReq.SetReturnValue(SfxUInt32Item(SID_UNDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
                                return;
                            }
                        }
                    }
                }

                // lock paint for more than one cell undo action (not for editing within a cell)
                bool bLockPaint = ( nCount > 1 && pUndoManager == GetUndoManager() );
                if ( bLockPaint )
                    pDocSh->LockPaint();

                try
                {
                    ScUndoRedoContext aUndoRedoContext;
                    aUndoRedoContext.SetUndoOffset(nUndoOffset);

                    for (sal_uInt16 i=0; i<nCount; i++)
                    {
                        if ( bIsUndo )
                            pUndoManager->UndoWithContext(aUndoRedoContext);
                        else
                            pUndoManager->RedoWithContext(aUndoRedoContext);
                    }
                }
                catch ( const uno::Exception& )
                {
                    // no need to handle. By definition, the UndoManager handled this by clearing the
                    // Undo/Redo stacks
                }

                if ( bLockPaint )
                    pDocSh->UnlockPaint();

                GetViewFrame().GetBindings().InvalidateAll(false);
            }
            break;
//      default:
//          GetViewFrame().ExecuteSlot( rReq );
    }
}

void ScTabViewShell::GetUndoState(SfxItemSet &rSet)
{
    SfxShell* pSh = GetViewData().GetDispatcher().GetShell(0);
    if (!pSh)
        return;

    SfxUndoManager* pUndoManager = pSh->GetUndoManager();
    ScUndoManager* pScUndoManager = dynamic_cast<ScUndoManager*>(pUndoManager);

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

            case SID_UNDO:
            {
                if (pScUndoManager)
                {
                    if (pScUndoManager->GetUndoActionCount())
                    {
                        const SfxUndoAction* pAction = pScUndoManager->GetUndoAction();
                        SfxViewShell *pViewSh = GetViewShell();
                        if (pViewSh && pAction->GetViewShellId() != pViewSh->GetViewShellId()
                            && !pScUndoManager->IsViewUndoActionIndependent(this, o3tl::temporary(sal_uInt16())))
                        {
                            rSet.Put(SfxUInt32Item(SID_UNDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
                        }
                        else
                        {
                            rSet.Put( SfxStringItem( SID_UNDO, SvtResId(STR_UNDO)+pScUndoManager->GetUndoActionComment() ) );
                        }
                    }
                    else
                        rSet.DisableItem( SID_UNDO );
                }
                else
                    // get state from sfx view frame
                    GetViewFrame().GetSlotState( nWhich, nullptr, &rSet );
                break;
            }
            case SID_REDO:
            {
                if (pScUndoManager)
                {
                    if (pScUndoManager->GetRedoActionCount())
                    {
                        const SfxUndoAction* pAction = pScUndoManager->GetRedoAction();
                        SfxViewShell *pViewSh = GetViewShell();
                        if (pViewSh && pAction->GetViewShellId() != pViewSh->GetViewShellId())
                        {
                            rSet.Put(SfxUInt32Item(SID_REDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
                        }
                        else
                        {
                            rSet.Put(SfxStringItem(SID_REDO, SvtResId(STR_REDO) + pScUndoManager->GetRedoActionComment()));
                        }
                    }
                    else
                        rSet.DisableItem( SID_REDO );
                }
                else
                    // get state from sfx view frame
                    GetViewFrame().GetSlotState( nWhich, nullptr, &rSet );
                break;
            }
            default:
                // get state from sfx view frame
                GetViewFrame().GetSlotState( nWhich, nullptr, &rSet );
        }

        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
