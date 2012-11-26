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



// INCLUDE ---------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ("", off)
#endif

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/aeitem.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/dispatch.hxx>

#include "tabvwsh.hxx"
#include "drawsh.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"
#include "fuconrec.hxx"
#include "fuconpol.hxx"
#include "fuconarc.hxx"
#include "fuconuno.hxx"
#include "fusel.hxx"
#include "futext.hxx"
#include "fumark.hxx"
#include "fuinsert.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"

// #98185# Create default drawing objects via keyboard
#include <svx/svdpagv.hxx>
#include <svl/stritem.hxx>
#include <svx/svdpage.hxx>
#include <fuconcustomshape.hxx>

#define SvxDrawToolItem             SfxAllEnumItem

// -----------------------------------------------------------------------

SdrView* __EXPORT ScTabViewShell::GetDrawView() const
{
    return ((ScTabViewShell*)this)->GetScDrawView();    // GetScDrawView ist nicht-const
}

void ScTabViewShell::WindowChanged()
{
    Window* pWin = GetActiveWin();

    ScDrawView* pDrView = GetScDrawView();
    if (pDrView)
        pDrView->SetActualOutDev(pWin);

    FuPoor* pFunc = GetDrawFuncPtr();
    if (pFunc)
        pFunc->SetWindow(pWin);

    //  when font from InputContext is used,
    //  this must be moved to change of cursor position:
    UpdateInputContext();
}

void ScTabViewShell::ExecDraw(SfxRequest& rReq)
{
    SC_MOD()->InputEnterHandler();
    UpdateInputHandler();

    MakeDrawLayer();

    ScTabView* pTabView = GetViewData()->GetView();
    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    Window*     pWin    = pTabView->GetActiveWin();
    ScDrawView* pView   = pTabView->GetScDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nNewId = rReq.GetSlot();

    if ( nNewId == SID_DRAW_CHART )
    {
        // #i71254# directly insert a chart instead of drawing its output rectangle
        FuInsertChart(this, pWin, pView, &pView->getSdrModelFromSdrView(), rReq);
        return;
    }

    //
    //  Pseudo-Slots von Draw-Toolbox auswerten
    //! wird das ueberhaupt noch gebraucht ?????
    //

    if (nNewId == SID_INSERT_DRAW && pArgs)
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_INSERT_DRAW, sal_True, &pItem ) == SFX_ITEM_SET &&
             dynamic_cast< const SvxDrawToolItem* >(pItem) )
        {
            SvxDrawToolEnum eSel = (SvxDrawToolEnum)(static_cast< const SvxDrawToolItem* >(pItem))->GetValue();
            switch (eSel)
            {
                case SVX_SNAP_DRAW_SELECT:          nNewId = SID_OBJECT_SELECT;         break;
                case SVX_SNAP_DRAW_LINE:            nNewId = SID_DRAW_LINE;             break;
                case SVX_SNAP_DRAW_RECT:            nNewId = SID_DRAW_RECT;             break;
                case SVX_SNAP_DRAW_ELLIPSE:         nNewId = SID_DRAW_ELLIPSE;          break;
                case SVX_SNAP_DRAW_POLYGON_NOFILL:  nNewId = SID_DRAW_POLYGON_NOFILL;   break;
                case SVX_SNAP_DRAW_BEZIER_NOFILL:   nNewId = SID_DRAW_BEZIER_NOFILL;    break;
                case SVX_SNAP_DRAW_FREELINE_NOFILL: nNewId = SID_DRAW_FREELINE_NOFILL;  break;
                case SVX_SNAP_DRAW_ARC:             nNewId = SID_DRAW_ARC;              break;
                case SVX_SNAP_DRAW_PIE:             nNewId = SID_DRAW_PIE;              break;
                case SVX_SNAP_DRAW_CIRCLECUT:       nNewId = SID_DRAW_CIRCLECUT;        break;
                case SVX_SNAP_DRAW_TEXT:            nNewId = SID_DRAW_TEXT;             break;
                case SVX_SNAP_DRAW_TEXT_VERTICAL:   nNewId = SID_DRAW_TEXT_VERTICAL;    break;
                case SVX_SNAP_DRAW_TEXT_MARQUEE:    nNewId = SID_DRAW_TEXT_MARQUEE;     break;
                case SVX_SNAP_DRAW_CAPTION:         nNewId = SID_DRAW_CAPTION;          break;
                case SVX_SNAP_DRAW_CAPTION_VERTICAL: nNewId = SID_DRAW_CAPTION_VERTICAL; break;
            }
        }
        else                    // sal_uInt16-Item vom Controller
        {
            rReq.Done();
            return;
        }
    }

    if ( nNewId == SID_DRAW_SELECT )
        nNewId = SID_OBJECT_SELECT;

    sal_uInt16 nNewFormId = 0;
    if ( nNewId == SID_FM_CREATE_CONTROL && pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_FM_CONTROL_IDENTIFIER, sal_True, &pItem ) == SFX_ITEM_SET &&
             dynamic_cast< const SfxUInt16Item* >(pItem) )
        {
            nNewFormId = ((const SfxUInt16Item*)pItem)->GetValue();
        }
    }

    String sStringItemValue;
    if ( pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( nNewId, sal_True, &pItem ) == SFX_ITEM_SET && dynamic_cast< const SfxStringItem* >(pItem) )
        {
            sStringItemValue = static_cast<const SfxStringItem*>(pItem)->GetValue();
        }
    }
    bool bSwitchCustom = ( sStringItemValue.Len() && sDrawCustom.Len() && sStringItemValue != sDrawCustom );

    if (nNewId == SID_INSERT_FRAME)                     // vom Tbx-Button
        nNewId = SID_DRAW_TEXT;

    //  #97016# CTRL-SID_OBJECT_SELECT is used to select the first object,
    //  but not if SID_OBJECT_SELECT is the result of clicking a create function again,
    //  so this must be tested before changing nNewId below.
    sal_Bool bSelectFirst = ( nNewId == SID_OBJECT_SELECT && (rReq.GetModifier() & KEY_MOD1) );

    sal_Bool bEx = IsDrawSelMode();
    if ( rReq.GetModifier() & KEY_MOD1 )
    {
        //  #97016# always allow keyboard selection also on background layer
        //  #98185# also allow creation of default objects if the same object type
        //  was already active
        bEx = sal_True;
    }
    else if ( nNewId == nDrawSfxId && ( nNewId != SID_FM_CREATE_CONTROL ||
                                    nNewFormId == nFormSfxId || nNewFormId == 0 ) && !bSwitchCustom )
    {
        //  #i52871# if a different custom shape is selected, the slot id can be the same,
        //  so the custom shape type string has to be compared, too.

        //  SID_FM_CREATE_CONTROL mit nNewFormId==0 (ohne Parameter) kommt beim Deaktivieren
        //  aus FuConstruct::SimpleMouseButtonUp
        //  #59280# Execute fuer die Form-Shell, um im Controller zu deselektieren
        if ( nNewId == SID_FM_CREATE_CONTROL )
        {
            GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
            GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            //! was fuer einen Slot braucht der komische Controller wirklich, um das anzuzeigen????
        }

        bEx = !bEx;
        nNewId = SID_OBJECT_SELECT;
    }
    else
        bEx = sal_True;

    if ( nDrawSfxId == SID_FM_CREATE_CONTROL && nNewId != nDrawSfxId )
    {
        //  Wechsel von Control- zu Zeichenfunktion -> im Control-Controller deselektieren
        GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
        GetViewFrame()->GetBindings().InvalidateAll(sal_False);
        //! was fuer einen Slot braucht der komische Controller wirklich, um das anzuzeigen????
    }

    SetDrawSelMode(bEx);

    pView->LockBackgroundLayer( !bEx );

    if ( bSelectFirst )
    {
        //  #97016# select first draw object if none is selected yet
        if(!pView->areSdrObjectsSelected())
        {
            // select first object
            pView->UnmarkAllObj();
            pView->MarkNextObj(true);

            // ...and make it visible
            if(pView->areSdrObjectsSelected())
            {
                pView->MakeVisibleAtView(pView->getMarkedObjectSnapRange(), *pWin);
            }
        }
    }

    nDrawSfxId = nNewId;
    sDrawCustom.Erase();    // value is set below for custom shapes

    if ( nNewId != SID_DRAW_CHART )             // Chart nicht mit DrawShell
    {
        if ( nNewId == SID_DRAW_TEXT || nNewId == SID_DRAW_TEXT_VERTICAL ||
                nNewId == SID_DRAW_TEXT_MARQUEE || nNewId == SID_DRAW_NOTEEDIT )
            SetDrawTextShell( sal_True );
        else
        {
            if ( bEx || pView->areSdrObjectsSelected() )
                SetDrawShellOrSub();
            else
                SetDrawShell( sal_False );
        }
    }

    if (pTabView->GetDrawFuncPtr())
    {
        if (pTabView->GetDrawFuncOldPtr() != pTabView->GetDrawFuncPtr())
            delete pTabView->GetDrawFuncOldPtr();

        pTabView->GetDrawFuncPtr()->Deactivate();
        pTabView->SetDrawFuncOldPtr(pTabView->GetDrawFuncPtr());
        pTabView->SetDrawFuncPtr(NULL);
    }

    SfxRequest aNewReq(rReq);
    aNewReq.SetSlot(nDrawSfxId);

    switch (nNewId)
    {
        case SID_OBJECT_SELECT:
            //@#70206# Nicht immer zurueckschalten
            if(!pView->areSdrObjectsSelected())
                SetDrawShell(bEx);
            pTabView->SetDrawFuncPtr(new FuSelection(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            break;

        case SID_DRAW_LINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            pView->SetFrameHandles(false);
            rBindings.Invalidate( SID_BEZIER_EDIT );
            break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_FREELINE_NOFILL:
            pTabView->SetDrawFuncPtr(new FuConstPolygon(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            break;

        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
            pTabView->SetDrawFuncPtr(new FuConstArc(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            break;

        case SID_DRAW_TEXT:
        case SID_DRAW_TEXT_VERTICAL:
        case SID_DRAW_TEXT_MARQUEE:
        case SID_DRAW_NOTEEDIT:
            pTabView->SetDrawFuncPtr(new FuText(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            break;

        case SID_FM_CREATE_CONTROL:
            SetDrawFormShell(sal_True);
            pTabView->SetDrawFuncPtr(new FuConstUnoControl(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            nFormSfxId = nNewFormId;
            break;

        case SID_DRAW_CHART:
//UNUSED2008-05  bChartDlgIsEdit = FALSE;
            pTabView->SetDrawFuncPtr(new FuMarkRect(this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq));
            break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        case SID_DRAW_CS_ID :
        {
            pTabView->SetDrawFuncPtr( new FuConstCustomShape( this, pWin, pView, &pView->getSdrModelFromSdrView(), aNewReq ));
            if ( nNewId != SID_DRAW_CS_ID )
            {
                SFX_REQUEST_ARG( rReq, pEnumCommand, SfxStringItem, nNewId );
                if ( pEnumCommand )
                {
                    aCurrShapeEnumCommand[ nNewId - SID_DRAWTBX_CS_BASIC ] = pEnumCommand->GetValue();
                    SfxBindings& rBind = GetViewFrame()->GetBindings();
                    rBind.Invalidate( nNewId );
                    rBind.Update( nNewId );

                    sDrawCustom = pEnumCommand->GetValue();  // to detect when a different shape type is selected
                }
            }
        }
        break;

        default:
            break;
    }

    if (pTabView->GetDrawFuncPtr())
        pTabView->GetDrawFuncPtr()->Activate();

    rReq.Done();

    rBindings.Invalidate( SID_INSERT_DRAW );
    rBindings.Update( SID_INSERT_DRAW );

    // #98185# Create default drawing objects via keyboard
    // with qualifier construct directly
    FuPoor* pFuActual = GetDrawFuncPtr();

    if(pFuActual && (rReq.GetModifier() & KEY_MOD1))
    {
        ScDrawView* pDrView = GetScDrawView();

        if(pDrView)
        {
            // #98185# Create default drawing objects via keyboard
            const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
            const sal_uInt32 nDefaultObjectSizeWidth(rAppOpt.GetDefaultObjectSizeWidth());
            const sal_uInt32 nDefaultObjectSizeHeight(rAppOpt.GetDefaultObjectSizeHeight());

            // calc position and size
            const basegfx::B2DRange aVisArea(pWin->GetLogicRange());
            const basegfx::B2DVector aObjectSize(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight);
            const basegfx::B2DPoint aObjectPos(aVisArea.getCenter() - (aObjectSize * 0.5));
            const basegfx::B2DRange aNewObjectRange(aObjectPos, aObjectPos + aObjectSize);

                // create the default object
            SdrObject* pObj = pFuActual->CreateDefaultObject(nNewId, aNewObjectRange);

            if(pObj)
            {
                // insert into page
                pView->InsertObjectAtView(*pObj);

                if ( nNewId == SID_DRAW_CAPTION || nNewId == SID_DRAW_CAPTION_VERTICAL )
                {
                    //  #105815# use KeyInput to start edit mode (FuText is created).
                    //  For FuText objects, edit mode is handled within CreateDefaultObject.
                    //  KEY_F2 is handled in FuDraw::KeyInput.

                    pFuActual->KeyInput( KeyEvent( 0, KeyCode( KEY_F2 ) ) );
                }
            }
        }
    }
}

void ScTabViewShell::GetDrawState(SfxItemSet &rSet)
{
    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_DRAW:
                {
                    //  SID_OBJECT_SELECT nur, wenn "harter" Selektionsmodus
                    sal_uInt16 nPutId = nDrawSfxId;
                    if ( nPutId == SID_OBJECT_SELECT && !IsDrawSelMode() )
                        nPutId = USHRT_MAX;
                    // nur die Images, die auch auf dem Controller liegen
                    if ( nPutId != SID_OBJECT_SELECT &&
                         nPutId != SID_DRAW_LINE &&
                         nPutId != SID_DRAW_RECT &&
                         nPutId != SID_DRAW_ELLIPSE &&
                         nPutId != SID_DRAW_POLYGON_NOFILL &&
                         nPutId != SID_DRAW_BEZIER_NOFILL &&
                         nPutId != SID_DRAW_FREELINE_NOFILL &&
                         nPutId != SID_DRAW_ARC &&
                         nPutId != SID_DRAW_PIE &&
                         nPutId != SID_DRAW_CIRCLECUT &&
                         nPutId != SID_DRAW_TEXT &&
                         nPutId != SID_DRAW_TEXT_VERTICAL &&
                         nPutId != SID_DRAW_TEXT_MARQUEE &&
                         nPutId != SID_DRAW_CAPTION &&
                         nPutId != SID_DRAW_CAPTION_VERTICAL )
                        nPutId = USHRT_MAX;
                    SfxAllEnumItem aItem( nWhich, nPutId );
                    if ( !SvtLanguageOptions().IsVerticalTextEnabled() )
                    {
                        aItem.DisableValue( SID_DRAW_TEXT_VERTICAL );
                        aItem.DisableValue( SID_DRAW_CAPTION_VERTICAL );
                    }
                    rSet.Put( aItem );
                }
                break;

            case SID_DRAW_CHART:
                {
                    sal_Bool bOle = GetViewFrame()->GetFrame().IsInPlace();
                    if ( bOle || !SvtModuleOptions().IsChart() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OBJECT_SELECT:     // wichtig fuer den ollen Control-Controller
                rSet.Put( SfxBoolItem( nWhich, nDrawSfxId == SID_OBJECT_SELECT && IsDrawSelMode() ) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

sal_Bool ScTabViewShell::SelectObject( const String& rName )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    if (!pView)
        return sal_False;

    sal_Bool bFound = pView->SelectObject( rName );

    return bFound;
}



