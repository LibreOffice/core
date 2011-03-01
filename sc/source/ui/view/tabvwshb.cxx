/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>



//------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ("", off)
#endif

// INCLUDE ---------------------------------------------------------------

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
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <sot/exchange.hxx>

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

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

void ScTabViewShell::ConnectObject( SdrOle2Obj* pObj )
{
    //  wird aus dem Paint gerufen

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    Window* pWin = GetActiveWin();

    //  #41412# wenn schon connected ist, nicht nochmal SetObjArea/SetSizeScale

    SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
    if ( !pClient )
    {
        pClient = new ScClient( this, pWin, GetSdrView()->GetModel(), pObj );
        Rectangle aRect = pObj->GetLogicRect();
        Size aDrawSize = aRect.GetSize();

        Size aOleSize = pObj->GetOrigObjSize();

        Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
        aScaleHeight.ReduceInaccurate(10);
        pClient->SetSizeScale(aScaleWidth,aScaleHeight);

        // sichtbarer Ausschnitt wird nur inplace veraendert!
        // the object area must be set after the scaling since it triggers the resizing
        aRect.SetSize( aOleSize );
        pClient->SetObjArea( aRect );

        ((ScClient*)pClient)->SetGrafEdit( NULL );
    }
}

BOOL ScTabViewShell::ActivateObject( SdrOle2Obj* pObj, long nVerb )
{
    // #41081# Gueltigkeits-Hinweisfenster nicht ueber dem Objekt stehenlassen
    RemoveHintWindow();

    uno::Reference < embed::XEmbeddedObject > xObj = pObj->GetObjRef();
    Window* pWin = GetActiveWin();
    ErrCode nErr = ERRCODE_NONE;
    BOOL bErrorShown = FALSE;

    {
        SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
        if ( !pClient )
            pClient = new ScClient( this, pWin, GetSdrView()->GetModel(), pObj );

        if ( !(nErr & ERRCODE_ERROR_MASK) && xObj.is() )
        {
            Rectangle aRect = pObj->GetLogicRect();
            Size aDrawSize = aRect.GetSize();

            MapMode aMapMode( MAP_100TH_MM );
            Size aOleSize = pObj->GetOrigObjSize( &aMapMode );

            if ( pClient->GetAspect() != embed::Aspects::MSOLE_ICON
              && ( xObj->getStatus( pClient->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE ) )
            {
                //  scale must always be 1 - change VisArea if different from client size

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
                //  calculate scale from client and VisArea size

                Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
                Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
                aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
                aScaleHeight.ReduceInaccurate(10);
                pClient->SetSizeScale(aScaleWidth,aScaleHeight);
            }

            // sichtbarer Ausschnitt wird nur inplace veraendert!
            // the object area must be set after the scaling since it triggers the resizing
            aRect.SetSize( aOleSize );
            pClient->SetObjArea( aRect );

            ((ScClient*)pClient)->SetGrafEdit( NULL );

            nErr = pClient->DoVerb( nVerb );
            bErrorShown = TRUE;
            // SfxViewShell::DoVerb zeigt seine Fehlermeldungen selber an

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

    //! SetDocumentName sollte schon im Sfx passieren ???
    //TODO/LATER: how "SetDocumentName"?
    //xIPObj->SetDocumentName( GetViewData()->GetDocShell()->GetTitle() );

    return ( !(nErr & ERRCODE_ERROR_MASK) );
}

ErrCode ScTabViewShell::DoVerb(long nVerb)
{
    SdrView* pView = GetSdrView();
    if (!pView)
        return ERRCODE_SO_NOTIMPL;          // soll nicht sein

    SdrOle2Obj* pOle2Obj = NULL;
    SdrObject* pObj = NULL;
    ErrCode nErr = ERRCODE_NONE;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
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
    // deactivate inplace editing if currently active

    ScModule* pScMod = SC_MOD();
    bool bUnoRefDialog = pScMod->IsRefDialogOpen() && pScMod->GetCurRefDlgId() == WID_SIMPLE_REF;

    ScClient* pClient = (ScClient*) GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() && !bUnoRefDialog )
        pClient->DeactivateObject();
}

void ScTabViewShell::ExecDrawIns(SfxRequest& rReq)
{
    USHORT nSlot = rReq.GetSlot();
    if (nSlot != SID_OBJECTRESIZE )
    {
        SC_MOD()->InputEnterHandler();
        UpdateInputHandler();
    }

    //  Rahmen fuer Chart einfuegen wird abgebrochen:
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
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_APPLET:
        case SID_INSERT_SMATH:
        case SID_INSERT_FLOATINGFRAME:
            FuInsertOLE(this, pWin, pView, pDrModel, rReq);
            break;

        case SID_OBJECTRESIZE:
            {
                //          Der Server moechte die Clientgrosse verandern

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

                            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

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
                    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
                    rReq.Done();
                }
            }
            break;

        // #98721#
        case SID_FM_CREATE_FIELDCONTROL:
            {
                SFX_REQUEST_ARG( rReq, pDescriptorItem, SfxUnoAnyItem, SID_FM_DATACCESS_DESCRIPTOR, sal_False );
                DBG_ASSERT( pDescriptorItem, "SID_FM_CREATE_FIELDCONTROL: invalid request args!" );

                if(pDescriptorItem)
                {
                    //! merge with ScViewFunc::PasteDataFormat (SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE)?

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

                            // controls must be on control layer, groups on front layer
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
    BOOL bOle = GetViewFrame()->GetFrame().IsInPlace();
    BOOL bTabProt = GetViewData()->GetDocument()->IsTabProtected(GetViewData()->GetTabNo());
    ScDocShell* pDocShell = ( GetViewData() ? GetViewData()->GetDocShell() : NULL );
    bool bShared = ( pDocShell ? pDocShell->IsDocShared() : false );

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
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

            case SID_INSERT_APPLET:
                //  wenn SOLAR_JAVA nicht definiert ist, immer disablen
#ifdef SOLAR_JAVA
                if (bOle || bTabProt)
#endif
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
                    if (GetViewData()->GetDocument()->GetLinkManager()->GetLinks().Count() == 0 )
                        rSet.DisableItem( SID_LINKS );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
