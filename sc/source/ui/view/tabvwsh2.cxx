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

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/aeitem.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/dispatch.hxx>

#include "tabvwsh.hxx"
#include "drawattr.hxx"
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


#include <svx/svdpagv.hxx>
#include <svl/stritem.hxx>
#include <svx/svdpage.hxx>
#include <fuconcustomshape.hxx>



SdrView* ScTabViewShell::GetDrawView() const
{
    return ((ScTabViewShell*)this)->GetScDrawView();    
}

void ScTabViewShell::WindowChanged()
{
    Window* pWin = GetActiveWin();

    ScDrawView* pDrView = GetScDrawView();
    if (pDrView)
        pDrView->SetActualWin(pWin);

    FuPoor* pFunc = GetDrawFuncPtr();
    if (pFunc)
        pFunc->SetWindow(pWin);

    
    
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
    SdrModel*   pDoc    = pView->GetModel();

    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16 nNewId = rReq.GetSlot();

    if ( nNewId == SID_DRAW_CHART )
    {
        
        FuInsertChart(this, pWin, pView, pDoc, rReq);
        return;
    }

    //
    
    
    //

    if (nNewId == SID_INSERT_DRAW && pArgs)
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_INSERT_DRAW, true, &pItem ) == SFX_ITEM_SET &&
             pItem->ISA( SvxDrawToolItem ) )
        {
            SvxDrawToolEnum eSel = (SvxDrawToolEnum)((const SvxDrawToolItem*)pItem)->GetValue();
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
        else                    
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
        if ( pArgs->GetItemState( SID_FM_CONTROL_IDENTIFIER, true, &pItem ) == SFX_ITEM_SET &&
             pItem->ISA( SfxUInt16Item ) )
            nNewFormId = ((const SfxUInt16Item*)pItem)->GetValue();
    }

    OUString sStringItemValue;
    if ( pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( nNewId, true, &pItem ) == SFX_ITEM_SET && pItem->ISA( SfxStringItem ) )
            sStringItemValue = static_cast<const SfxStringItem*>(pItem)->GetValue();
    }
    bool bSwitchCustom = ( !sStringItemValue.isEmpty() && !sDrawCustom.isEmpty() && sStringItemValue != sDrawCustom );

    if (nNewId == SID_INSERT_FRAME)                     
        nNewId = SID_DRAW_TEXT;

    
    
    
    bool bSelectFirst = ( nNewId == SID_OBJECT_SELECT && (rReq.GetModifier() & KEY_MOD1) );

    bool bEx = IsDrawSelMode();
    if ( rReq.GetModifier() & KEY_MOD1 )
    {
        
        
        
        bEx = true;
    }
    else if ( nNewId == nDrawSfxId && ( nNewId != SID_FM_CREATE_CONTROL ||
                                    nNewFormId == nFormSfxId || nNewFormId == 0 ) && !bSwitchCustom )
    {
        
        

        
        
        
        if ( nNewId == SID_FM_CREATE_CONTROL )
        {
            GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
            GetViewFrame()->GetBindings().InvalidateAll(false);
            
        }

        bEx = !bEx;
        nNewId = SID_OBJECT_SELECT;
    }
    else
        bEx = true;

    if ( nDrawSfxId == SID_FM_CREATE_CONTROL && nNewId != nDrawSfxId )
    {
        
        GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
        GetViewFrame()->GetBindings().InvalidateAll(false);
        
    }

    SetDrawSelMode(bEx);

    pView->LockBackgroundLayer( !bEx );

    if ( bSelectFirst )
    {
        
        if(!pView->AreObjectsMarked())
        {
            
            pView->UnmarkAllObj();
            pView->MarkNextObj(sal_True);

            
            if(pView->AreObjectsMarked())
                pView->MakeVisible(pView->GetAllMarkedRect(), *pWin);
        }
    }

    nDrawSfxId = nNewId;
    sDrawCustom = "";    

    if ( nNewId != SID_DRAW_CHART )             
    {
        if ( nNewId == SID_DRAW_TEXT || nNewId == SID_DRAW_TEXT_VERTICAL ||
                nNewId == SID_DRAW_TEXT_MARQUEE || nNewId == SID_DRAW_NOTEEDIT )
            SetDrawTextShell( true );
        else
        {
            if ( bEx || pView->GetMarkedObjectList().GetMarkCount() != 0 )
                SetDrawShellOrSub();
            else
                SetDrawShell( false );
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
            
            if(pView->GetMarkedObjectList().GetMarkCount() == 0) SetDrawShell(bEx);
            pTabView->SetDrawFuncPtr(new FuSelection(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_LINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            pTabView->SetDrawFuncPtr(new FuConstRectangle(this, pWin, pView, pDoc, aNewReq));
            pView->SetFrameDragSingles( false );
            rBindings.Invalidate( SID_BEZIER_EDIT );
            break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_FREELINE_NOFILL:
            pTabView->SetDrawFuncPtr(new FuConstPolygon(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
            pTabView->SetDrawFuncPtr(new FuConstArc(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAW_TEXT:
        case SID_DRAW_TEXT_VERTICAL:
        case SID_DRAW_TEXT_MARQUEE:
        case SID_DRAW_NOTEEDIT:
            pTabView->SetDrawFuncPtr(new FuText(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_FM_CREATE_CONTROL:
            SetDrawFormShell(true);
            pTabView->SetDrawFuncPtr(new FuConstUnoControl(this, pWin, pView, pDoc, aNewReq));
            nFormSfxId = nNewFormId;
            break;

        case SID_DRAW_CHART:
            pTabView->SetDrawFuncPtr(new FuMarkRect(this, pWin, pView, pDoc, aNewReq));
            break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        case SID_DRAW_CS_ID :
        {
            pTabView->SetDrawFuncPtr( new FuConstCustomShape( this, pWin, pView, pDoc, aNewReq ));
            if ( nNewId != SID_DRAW_CS_ID )
            {
                SFX_REQUEST_ARG( rReq, pEnumCommand, SfxStringItem, nNewId, false );
                if ( pEnumCommand )
                {
                    aCurrShapeEnumCommand[ nNewId - SID_DRAWTBX_CS_BASIC ] = pEnumCommand->GetValue();
                    SfxBindings& rBind = GetViewFrame()->GetBindings();
                    rBind.Invalidate( nNewId );
                    rBind.Update( nNewId );

                    sDrawCustom = pEnumCommand->GetValue();  
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

    
    
    FuPoor* pFuActual = GetDrawFuncPtr();

    if(pFuActual && (rReq.GetModifier() & KEY_MOD1))
    {
        
        const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
        sal_uInt32 nDefaultObjectSizeWidth = rAppOpt.GetDefaultObjectSizeWidth();
        sal_uInt32 nDefaultObjectSizeHeight = rAppOpt.GetDefaultObjectSizeHeight();

        
        Rectangle aVisArea = pWin->PixelToLogic(Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
        Point aPagePos = aVisArea.Center();
        aPagePos.X() -= nDefaultObjectSizeWidth / 2;
        aPagePos.Y() -= nDefaultObjectSizeHeight / 2;
        Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));

        ScDrawView* pDrView = GetScDrawView();

        if(pDrView)
        {
            SdrPageView* pPageView = pDrView->GetSdrPageView();

            if(pPageView)
            {
                
                SdrObject* pObj = pFuActual->CreateDefaultObject(nNewId, aNewObjectRectangle);

                if(pObj)
                {
                    
                    pView->InsertObjectAtView(pObj, *pPageView);

                    if ( nNewId == SID_DRAW_CAPTION || nNewId == SID_DRAW_CAPTION_VERTICAL )
                    {
                        
                        
                        

                        pFuActual->KeyInput( KeyEvent( 0, KeyCode( KEY_F2 ) ) );
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
            case SID_INSERT_DRAW:
                {
                    
                    sal_uInt16 nPutId = nDrawSfxId;
                    if ( nPutId == SID_OBJECT_SELECT && !IsDrawSelMode() )
                        nPutId = USHRT_MAX;
                    
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
                    bool bOle = GetViewFrame()->GetFrame().IsInPlace();
                    if ( bOle || !SvtModuleOptions().IsChart() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OBJECT_SELECT:     
                rSet.Put( SfxBoolItem( nWhich, nDrawSfxId == SID_OBJECT_SELECT && IsDrawSelMode() ) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

bool ScTabViewShell::SelectObject( const OUString& rName )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    if (!pView)
        return false;

    bool bFound = pView->SelectObject( rName );
    

    return bFound;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
