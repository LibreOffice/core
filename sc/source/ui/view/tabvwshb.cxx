/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabvwshb.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 15:16:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif



//------------------------------------------------------------------

#ifdef WNT
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
#include <svx/linkmgr.hxx>
#include <svx/fontworkbar.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/soerr.hxx>
#include <svtools/rectitem.hxx>
#include <svtools/whiter.hxx>
#include <svtools/moduleoptions.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
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

        awt::Size aSz;
        try
        {
            aSz = xObj->getVisualAreaSize( pClient->GetAspect() );
        }
        catch ( embed::NoVisualAreaSizeException& )
        {
            DBG_ERROR( "Can't get visual area size from the object!\n" );
            aSz.Width = 5000;
            aSz.Height = 5000;
        }

        Size aOleSize( aSz.Width, aSz.Height );

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

    // linked objects aren't supported
//  if ( xIPObj->IsLink() )
//      nErr = xIPObj->DoVerb(nVerb);           // gelinkt -> ohne Client etc.
//  else
    {
        SfxInPlaceClient* pClient = FindIPClient( xObj, pWin );
        if ( !pClient )
            pClient = new ScClient( this, pWin, GetSdrView()->GetModel(), pObj );

        if ( !(nErr & ERRCODE_ERROR_MASK) && xObj.is() )
        {
            Rectangle aRect = pObj->GetLogicRect();
            Size aDrawSize = aRect.GetSize();

            awt::Size aSz;
            try
            {
                aSz = xObj->getVisualAreaSize( pClient->GetAspect() );
            }
            catch ( embed::NoVisualAreaSizeException& )
            {
                DBG_ERROR( "Can't get visual area size from the object!\n" );
                aSz.Width = 5000;
                aSz.Height = 5000;
            }

            Size aOleSize( aSz.Width, aSz.Height );
            MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( pClient->GetAspect() ) );

            aOleSize = OutputDevice::LogicToLogic( aOleSize,
                                                   aUnit, MAP_100TH_MM );

            if ( xObj->getStatus( pClient->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE )
            {
                //  scale must always be 1 - change VisArea if different from client size

                if ( aDrawSize != aOleSize )
                {
                    aOleSize = OutputDevice::LogicToLogic( aDrawSize,
                                            MAP_100TH_MM, aUnit );
                    aSz.Width = aOleSize.Width();
                    aSz.Height = aOleSize.Height();
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

            //  Link fuer Daten-Highlighting im Chart setzen
            if ( SvtModuleOptions().IsChart() )
            {
                SvGlobalName aObjClsId ( xObj->getClassID() );
                if (SotExchange::IsChart( aObjClsId ))
                {
                    SchMemChart* pMemChart = SchDLL::GetChartData(xObj);
                    if (pMemChart)
                    {
                        // set handler for highlighting cell ranges
                        // for selection inside the chart
                        pMemChart->SetSelectionHdl( LINK( GetViewData()->GetDocShell(),
                                                          ScDocShell, ChartSelectionHdl ) );

                        //  #96148# if an unmodified chart in a 5.2-document is edited
                        //  after swapping out and in again, the ChartRange has to be set
                        //  (from SomeData strings) again, or SetReadOnly doesn't work
                        if ( pMemChart->SomeData1().Len() && pMemChart->GetChartRange().maRanges.size() == 0 )
                        {
                            ScChartArray aArray( GetViewData()->GetDocument(), *pMemChart );
                            if ( aArray.IsValid() )
                                aArray.SetExtraStrings( *pMemChart );
                        }

                        // #102706# After loaded, a chart doesn't know our
                        // number formatter, therefor new formats added in the
                        // meantime aren't available in the chart's number
                        // formatter dialog. Set the formatter here. The
                        // original bug was the SourceFormat checkbox not being
                        // available, but that is a problem of the chart.
                        pMemChart->SetNumberFormatter( GetViewData()->GetDocument()->GetFormatTable() );

                        // disable DataBrowseBox for editing chart data
                        pMemChart->SetReadOnly( TRUE );

                        // #102706# The new NumberFormatter is set at the
                        // SchMemChart, but not at the corresponding
                        // ChartModel. Therefore, an Update is needed, because
                        // the MemChart doesn't know its ChartModel.

                        // TODO/LATER: Looks like there is no need to update the replacement, since the object will be activated.
                        SchDLL::Update( xObj, pMemChart );
                    }
                }
            }

            nErr = pClient->DoVerb( nVerb );
            bErrorShown = TRUE;
            // SfxViewShell::DoVerb zeigt seine Fehlermeldungen selber an
        }
    }
    if (nErr != ERRCODE_NONE && !bErrorShown)
        ErrorHandler::HandleError(nErr);

    //! SetDocumentName sollte schon im Sfx passieren ???
    //TODO/LATER: how "SetDocumentName"?
    //xIPObj->SetDocumentName( GetViewData()->GetDocShell()->GetTitle() );

    return ( !(nErr & ERRCODE_ERROR_MASK) );
}

ErrCode __EXPORT ScTabViewShell::DoVerb(long nVerb)
{
    SdrView* pView = GetSdrView();
    if (!pView)
        return ERRCODE_SO_NOTIMPL;          // soll nicht sein

    SdrOle2Obj* pOle2Obj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    SdrObject* pObj = NULL;
    ErrCode nErr = ERRCODE_NONE;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        pObj = rMarkList.GetMark(0)->GetObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
            pOle2Obj = (SdrOle2Obj*) pObj;
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = (SdrGrafObj*) pObj;
        }
    }

    if (pOle2Obj)
    {
        ActivateObject( pOle2Obj, nVerb );
    }
    else
        DBG_ERROR("kein Objekt fuer Verb gefunden");

    return nErr;
}

void ScTabViewShell::DeactivateOle()
{
    // deactivate inplace editing if currently active

    ScClient* pClient = (ScClient*) GetIPClient();
    if ( pClient && pClient->IsObjectInPlaceActive() )
    {
            //TODO/CLEANUP
            //nur im SFX Viewframe setzen
        pClient->GetObject()->changeState( embed::EmbedStates::RUNNING );
        SfxViewFrame::SetViewFrame(GetViewFrame());
    }
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
    SdrView*     pView     = pTabView->GetSdrView();
    ScDocShell*  pDocSh    = GetViewData()->GetDocShell();
    ScDocument*  pDoc      = pDocSh->GetDocument();
//  SdrModel*    pDrModel  = pDocSh->MakeDrawLayer();
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
//?         SC_MOD()->SetFunctionDlg( NULL );//XXX
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

                SfxInPlaceClient* pIPClient = GetIPClient();

                if ( pIPClient && pIPClient->IsObjectInPlaceActive() )
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
                            SdrObject* pObj = pMark->GetObj();

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
                    SdrPageView* pPageView = pDrView ? pDrView->GetPageViewPvNum(0) : NULL;
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

                            pView->InsertObject(pNewDBField, *pPageView, pView->IsSolidDraggingNow() ? SDRINSERT_NOBROADCAST : 0);
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
    BOOL bOle = GetViewFrame()->GetFrame()->IsInPlace();
    BOOL bTabProt = GetViewData()->GetDocument()->IsTabProtected(GetViewData()->GetTabNo());
    SfxApplication* pSfxApp = SFX_APP();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_DIAGRAM:
            case SID_OPENDLG_CHART:
                if ( bOle || bTabProt || !SvtModuleOptions().IsChart() )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OPENDLG_MODCHART:
                if ( bTabProt || !GetSelectedChartName().Len() )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SMATH:
                if ( bOle || bTabProt || !SvtModuleOptions().IsMath() )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_OBJECT:
            case SID_INSERT_PLUGIN:
            case SID_INSERT_FLOATINGFRAME:
                if (bOle || bTabProt)
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SOUND:
            case SID_INSERT_VIDEO:
                if (bOle || bTabProt || !SvxPluginFileDlg::IsAvailable(nWhich))
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
                if (bTabProt)
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




