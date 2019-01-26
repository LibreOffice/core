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

#include <comphelper/lok.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/aeitem.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/dispatch.hxx>

#include <tabvwsh.hxx>
#include <drawsh.hxx>
#include <drawview.hxx>
#include <fupoor.hxx>
#include <fuconrec.hxx>
#include <fuconpol.hxx>
#include <fuconarc.hxx>
#include <fuconuno.hxx>
#include <fusel.hxx>
#include <futext.hxx>
#include <fuinsert.hxx>
#include <global.hxx>
#include <sc.hrc>
#include <scmod.hxx>
#include <appoptio.hxx>
#include <gridwin.hxx>

// Create default drawing objects via keyboard
#include <svx/svdpagv.hxx>
#include <svl/stritem.hxx>
#include <svx/svdpage.hxx>
#include <fuconcustomshape.hxx>

SdrView* ScTabViewShell::GetDrawView() const
{
    return const_cast<ScTabViewShell*>(this)->GetScDrawView();    // GetScDrawView is non-const
}

void ScTabViewShell::WindowChanged()
{
    vcl::Window* pWin = GetActiveWin();

    ScDrawView* pDrView = GetScDrawView();
    if (pDrView)
        pDrView->SetActualWin(pWin);

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

    ScTabView* pTabView = GetViewData().GetView();
    SfxBindings& rBindings = GetViewFrame()->GetBindings();

    vcl::Window*     pWin    = pTabView->GetActiveWin();
    ScDrawView* pView   = pTabView->GetScDrawView();
    SdrModel*   pDoc    = pView->GetModel();

    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nNewId = rReq.GetSlot();

    if ( nNewId == SID_DRAW_CHART )
    {
        // #i71254# directly insert a chart instead of drawing its output rectangle
        FuInsertChart(*this, pWin, pView, pDoc, rReq);
        return;
    }

    if ( nNewId == SID_DRAW_SELECT )
        nNewId = SID_OBJECT_SELECT;

    sal_uInt16 nNewFormId = 0;
    if ( nNewId == SID_FM_CREATE_CONTROL && pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_FM_CONTROL_IDENTIFIER, true, &pItem ) == SfxItemState::SET &&
             dynamic_cast<const SfxUInt16Item*>( pItem) !=  nullptr )
            nNewFormId = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    }

    OUString sStringItemValue;
    if ( pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( nNewId, true, &pItem ) == SfxItemState::SET && dynamic_cast<const SfxStringItem*>( pItem) !=  nullptr )
            sStringItemValue = static_cast<const SfxStringItem*>(pItem)->GetValue();
    }
    bool bSwitchCustom = ( !sStringItemValue.isEmpty() && !sDrawCustom.isEmpty() && sStringItemValue != sDrawCustom );

    if (nNewId == SID_INSERT_FRAME)                     // from Tbx button
        nNewId = SID_DRAW_TEXT;

    //  CTRL-SID_OBJECT_SELECT is used to select the first object,
    //  but not if SID_OBJECT_SELECT is the result of clicking a create function again,
    //  so this must be tested before changing nNewId below.
    bool bSelectFirst = ( nNewId == SID_OBJECT_SELECT && (rReq.GetModifier() & KEY_MOD1) );

    bool bEx = IsDrawSelMode();
    if ( rReq.GetModifier() & KEY_MOD1 )
    {
        //  always allow keyboard selection also on background layer
        //  also allow creation of default objects if the same object type
        //  was already active
        bEx = true;
    }
    else if ( nNewId == nDrawSfxId && ( nNewId != SID_FM_CREATE_CONTROL ||
                                    nNewFormId == nFormSfxId || nNewFormId == 0 ) && !bSwitchCustom )
    {
        // #i52871# if a different custom shape is selected, the slot id can be the same,
        // so the custom shape type string has to be compared, too.

        // SID_FM_CREATE_CONTROL with nNewFormId==0 (without parameter) comes
        // from FuConstruct::SimpleMouseButtonUp when deactivating
        // Execute for the form shell, to deselect the controller
        if ( nNewId == SID_FM_CREATE_CONTROL )
        {
            GetViewData().GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
            GetViewFrame()->GetBindings().InvalidateAll(false);
            //! what kind of slot does the weird controller really need to display this????
        }

        bEx = !bEx;
        nNewId = SID_OBJECT_SELECT;
    }
    else
        bEx = true;

    if ( nDrawSfxId == SID_FM_CREATE_CONTROL && nNewId != nDrawSfxId )
    {
        // switching from control- to paint function -> deselect in control-controller
        GetViewData().GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
        GetViewFrame()->GetBindings().InvalidateAll(false);
        //! what kind of slot does the weird controller really need to display this????
    }

    SetDrawSelMode(bEx);

    pView->LockBackgroundLayer( !bEx );

    if ( bSelectFirst )
    {
        // select first draw object if none is selected yet
        if(!pView->AreObjectsMarked())
        {
            // select first object
            pView->UnmarkAllObj();
            pView->MarkNextObj(true);

            // ...and make it visible
            if(pView->AreObjectsMarked())
                pView->MakeVisible(pView->GetAllMarkedRect(), *pWin);
        }
    }

    nDrawSfxId = nNewId;
    sDrawCustom.clear();    // value is set below for custom shapes

    if (nNewId == SID_DRAW_TEXT || nNewId == SID_DRAW_TEXT_VERTICAL
        || nNewId == SID_DRAW_TEXT_MARQUEE || nNewId == SID_DRAW_NOTEEDIT)
        SetDrawTextShell(true);
    else
    {
        if (bEx || pView->GetMarkedObjectList().GetMarkCount() != 0)
            SetDrawShellOrSub();
        else
            SetDrawShell(false);
    }

    if (pTabView->GetDrawFuncPtr())
    {
        if (pTabView->GetDrawFuncOldPtr() != pTabView->GetDrawFuncPtr())
            delete pTabView->GetDrawFuncOldPtr();

        pTabView->GetDrawFuncPtr()->Deactivate();
        pTabView->SetDrawFuncOldPtr(pTabView->GetDrawFuncPtr());
        pTabView->SetDrawFuncPtr(nullptr);
    }

    SfxRequest aNewReq(rReq);
    aNewReq.SetSlot(nDrawSfxId);

    assert(nNewId != SID_DRAW_CHART); //#i71254# handled already above

    // for LibreOfficeKit - choosing a shape should construct it directly
    bool bCreateDirectly = false;

    switch (nNewId)
    {
        case SID_OBJECT_SELECT:
            // not always switch back
            if(pView->GetMarkedObjectList().GetMarkCount() == 0) SetDrawShell(bEx);
            pTabView->SetDrawFuncPtr(new FuSelection(*this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_ARROW_START:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_SQUARE_ARROW:
        case SID_LINE_ARROWS:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_MEASURELINE:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(*this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(*this, pWin, pView, pDoc, aNewReq));
            pView->SetFrameDragSingles( false );
            rBindings.Invalidate( SID_BEZIER_EDIT );
            break;

        case SID_DRAW_XPOLYGON:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_FREELINE:
        case SID_DRAW_FREELINE_NOFILL:
            pTabView->SetDrawFuncPtr(new FuConstPolygon(*this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
            pTabView->SetDrawFuncPtr(new FuConstArc(*this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_TEXT:
        case SID_DRAW_TEXT_VERTICAL:
        case SID_DRAW_TEXT_MARQUEE:
        case SID_DRAW_NOTEEDIT:
            pTabView->SetDrawFuncPtr(new FuText(*this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_FM_CREATE_CONTROL:
            SetDrawFormShell(true);
            pTabView->SetDrawFuncPtr(new FuConstUnoControl(*this, pWin, pView, pDoc, aNewReq));
            nFormSfxId = nNewFormId;
            break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        case SID_DRAW_CS_ID :
        {
            pTabView->SetDrawFuncPtr(new FuConstCustomShape(*this, pWin, pView, pDoc, aNewReq));

            bCreateDirectly = comphelper::LibreOfficeKit::isActive();

            if ( nNewId != SID_DRAW_CS_ID )
            {
                const SfxStringItem* pEnumCommand = rReq.GetArg<SfxStringItem>(nNewId);
                if ( pEnumCommand )
                {
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

    Invalidate();

    // Create default drawing objects via keyboard
    // with qualifier construct directly
    FuPoor* pFuActual = GetDrawFuncPtr();

    if(pFuActual && ((rReq.GetModifier() & KEY_MOD1) || bCreateDirectly))
    {
        // Create default drawing objects via keyboard
        const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
        sal_uInt32 nDefaultObjectSizeWidth = rAppOpt.GetDefaultObjectSizeWidth();
        sal_uInt32 nDefaultObjectSizeHeight = rAppOpt.GetDefaultObjectSizeHeight();

        // calc position and size
        bool bLOKIsActive = comphelper::LibreOfficeKit::isActive();
        Point aInsertPos;
        if(!bLOKIsActive)
        {
            tools::Rectangle aVisArea = pWin->PixelToLogic(tools::Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
            aInsertPos = aVisArea.Center();
            aInsertPos.AdjustX( -sal_Int32(nDefaultObjectSizeWidth / 2) );
            aInsertPos.AdjustY( -sal_Int32(nDefaultObjectSizeHeight / 2) );
        }
        else
        {
            aInsertPos = GetInsertPos();
        }

        tools::Rectangle aNewObjectRectangle(aInsertPos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));

        ScDrawView* pDrView = GetScDrawView();

        if(pDrView)
        {
            SdrPageView* pPageView = pDrView->GetSdrPageView();

            if(pPageView)
            {
                // create the default object
                SdrObjectUniquePtr pObj = pFuActual->CreateDefaultObject(nNewId, aNewObjectRectangle);

                if(pObj)
                {
                    // insert into page
                    pView->InsertObjectAtView(pObj.release(), *pPageView);

                    if ( nNewId == SID_DRAW_CAPTION || nNewId == SID_DRAW_CAPTION_VERTICAL )
                    {
                        //  use KeyInput to start edit mode (FuText is created).
                        //  For FuText objects, edit mode is handled within CreateDefaultObject.
                        //  KEY_F2 is handled in FuDraw::KeyInput.

                        pFuActual->KeyInput( KeyEvent( 0, vcl::KeyCode( KEY_F2 ) ) );
                    }
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
            case SID_DRAW_CHART:
                {
                    bool bOle = GetViewFrame()->GetFrame().IsInPlace();
                    if ( bOle || !SvtModuleOptions().IsChart() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DRAW_LINE:
            case SID_DRAW_XLINE:
            case SID_LINE_ARROW_END:
            case SID_LINE_ARROW_CIRCLE:
            case SID_LINE_ARROW_SQUARE:
            case SID_LINE_ARROW_START:
            case SID_LINE_CIRCLE_ARROW:
            case SID_LINE_SQUARE_ARROW:
            case SID_LINE_ARROWS:
            case SID_DRAW_MEASURELINE:
            case SID_DRAW_RECT:
            case SID_DRAW_ELLIPSE:
            case SID_DRAW_POLYGON:
            case SID_DRAW_POLYGON_NOFILL:
            case SID_DRAW_XPOLYGON:
            case SID_DRAW_XPOLYGON_NOFILL:
            case SID_DRAW_BEZIER_FILL:
            case SID_DRAW_BEZIER_NOFILL:
            case SID_DRAW_FREELINE:
            case SID_DRAW_FREELINE_NOFILL:
            case SID_DRAW_ARC:
            case SID_DRAW_PIE:
            case SID_DRAW_CIRCLECUT:
            case SID_DRAW_TEXT:
            case SID_DRAW_TEXT_MARQUEE:
            case SID_DRAW_CAPTION:
                rSet.Put( SfxBoolItem( nWhich, nDrawSfxId == nWhich ) );
                break;

            case SID_DRAW_TEXT_VERTICAL:
            case SID_DRAW_CAPTION_VERTICAL:
                if ( !SvtLanguageOptions().IsVerticalTextEnabled() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, nDrawSfxId == nWhich ) );
                break;

            case SID_OBJECT_SELECT:     // important for the old control-controller
                rSet.Put( SfxBoolItem( nWhich, nDrawSfxId == SID_OBJECT_SELECT && IsDrawSelMode() ) );
                break;

            case SID_DRAWTBX_CS_BASIC:
            case SID_DRAWTBX_CS_SYMBOL:
            case SID_DRAWTBX_CS_ARROW:
            case SID_DRAWTBX_CS_FLOWCHART:
            case SID_DRAWTBX_CS_CALLOUT:
            case SID_DRAWTBX_CS_STAR:
                rSet.Put( SfxStringItem( nWhich, nDrawSfxId == nWhich ? sDrawCustom : OUString() ) );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

bool ScTabViewShell::SelectObject( const OUString& rName )
{
    ScDrawView* pView = GetViewData().GetScDrawView();
    if (!pView)
        return false;

    bool bFound = pView->SelectObject( rName );
    // DrawShell etc. is handled in MarkListHasChanged

    return bFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
