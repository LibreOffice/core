/*************************************************************************
 *
 *  $RCSfile: tabvwsh2.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:32:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifdef WNT
#pragma optimize ("", off)
#endif

#include <sfx2/bindings.hxx>
#include <sfx2/ipfrm.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/whiter.hxx>
#include <svtools/moduleoptions.hxx>
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
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
#include "global.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"

// #98185# Create default drawing objects via keyboard
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

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

    ScTabView* pTabView = GetViewData()->GetView();
    SCTAB nTab = GetViewData()->GetTabNo();
    SfxBindings& rBindings = GetViewFrame()->GetBindings();

    Window*     pWin    = pTabView->GetActiveWin();
    SdrView*    pView   = pTabView->GetSdrView();
    SdrModel*   pDoc    = pView->GetModel();

    const SfxItemSet *pArgs = rReq.GetArgs();
    USHORT nNewId = rReq.GetSlot();

    //
    //  Pseudo-Slots von Draw-Toolbox auswerten
    //! wird das ueberhaupt noch gebraucht ?????
    //

    if (nNewId == SID_INSERT_DRAW && pArgs)
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_INSERT_DRAW, TRUE, &pItem ) == SFX_ITEM_SET &&
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
        else                    // USHORT-Item vom Controller
        {
            rReq.Done();
            return;
        }
    }

    if ( nNewId == SID_DRAW_SELECT )
        nNewId = SID_OBJECT_SELECT;

    USHORT nNewFormId = 0;
    if ( nNewId == SID_FM_CREATE_CONTROL && pArgs )
    {
        const SfxPoolItem* pItem;
        if ( pArgs->GetItemState( SID_FM_CONTROL_IDENTIFIER, TRUE, &pItem ) == SFX_ITEM_SET &&
             pItem->ISA( SfxUInt16Item ) )
            nNewFormId = ((const SfxUInt16Item*)pItem)->GetValue();
    }

    if (nNewId == SID_INSERT_FRAME)                     // vom Tbx-Button
        nNewId = SID_DRAW_TEXT;

    //  #97016# CTRL-SID_OBJECT_SELECT is used to select the first object,
    //  but not if SID_OBJECT_SELECT is the result of clicking a create function again,
    //  so this must be tested before changing nNewId below.
    BOOL bSelectFirst = ( nNewId == SID_OBJECT_SELECT && (rReq.GetModifier() & KEY_MOD1) );

    BOOL bEx = IsDrawSelMode();
    if ( rReq.GetModifier() & KEY_MOD1 )
    {
        //  #97016# always allow keyboard selection also on background layer
        //  #98185# also allow creation of default objects if the same object type
        //  was already active
        bEx = TRUE;
    }
    else if ( nNewId == nDrawSfxId && ( nNewId != SID_FM_CREATE_CONTROL ||
                                    nNewFormId == nFormSfxId || nNewFormId == 0 ) )
    {
        //  SID_FM_CREATE_CONTROL mit nNewFormId==0 (ohne Parameter) kommt beim Deaktivieren
        //  aus FuConstruct::SimpleMouseButtonUp
        //  #59280# Execute fuer die Form-Shell, um im Controller zu deselektieren
        if ( nNewId == SID_FM_CREATE_CONTROL )
        {
            GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
            GetViewFrame()->GetBindings().InvalidateAll(FALSE);
            //! was fuer einen Slot braucht der komische Controller wirklich, um das anzuzeigen????
        }

        bEx = !bEx;
        nNewId = SID_OBJECT_SELECT;
    }
    else
        bEx = TRUE;

    if ( nDrawSfxId == SID_FM_CREATE_CONTROL && nNewId != nDrawSfxId )
    {
        //  Wechsel von Control- zu Zeichenfunktion -> im Control-Controller deselektieren
        GetViewData()->GetDispatcher().Execute(SID_FM_LEAVE_CREATE);
        GetViewFrame()->GetBindings().InvalidateAll(FALSE);
        //! was fuer einen Slot braucht der komische Controller wirklich, um das anzuzeigen????
    }

    SetDrawSelMode(bEx);

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
    if (pLayer)
        pView->SetLayerLocked( pLayer->GetName(), !bEx );

    if ( bSelectFirst )
    {
        //  #97016# select first draw object if none is selected yet
        if(!pView->AreObjectsMarked())
        {
            // select first object
            pView->UnmarkAllObj();
            pView->MarkNextObj(TRUE);

            // ...and make it visible
            if(pView->AreObjectsMarked())
                pView->MakeVisible(pView->GetAllMarkedRect(), *pWin);
        }
    }

    nDrawSfxId = nNewId;

    if ( nNewId != SID_DRAW_CHART )             // Chart nicht mit DrawShell
    {
        if ( nNewId == SID_DRAW_TEXT || nNewId == SID_DRAW_TEXT_VERTICAL ||
                nNewId == SID_DRAW_TEXT_MARQUEE || nNewId == SID_DRAW_NOTEEDIT )
            SetDrawTextShell( TRUE );
        else
        {
            if ( bEx || pView->GetMarkedObjectList().GetMarkCount() != 0 )
                SetDrawShellOrSub();
            else
                SetDrawShell( FALSE );
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
            pView->SetFrameDragSingles( FALSE );
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
            SetDrawFormShell(TRUE);
            pTabView->SetDrawFuncPtr(new FuConstUnoControl(this, pWin, pView, pDoc, aNewReq));
            nFormSfxId = nNewFormId;
            break;

        case SID_DRAW_CHART:
            bChartDlgIsEdit = FALSE;
            pTabView->SetDrawFuncPtr(new FuMarkRect(this, pWin, pView, pDoc, aNewReq));
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
        // #98185# Create default drawing objects via keyboard
        const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
        sal_uInt32 nDefaultObjectSizeWidth = rAppOpt.GetDefaultObjectSizeWidth();
        sal_uInt32 nDefaultObjectSizeHeight = rAppOpt.GetDefaultObjectSizeHeight();

        // calc position and size
        Rectangle aVisArea = pWin->PixelToLogic(Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
        Point aPagePos = aVisArea.Center();
        aPagePos.X() -= nDefaultObjectSizeWidth / 2;
        aPagePos.Y() -= nDefaultObjectSizeHeight / 2;
        Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));

        ScDrawView* pDrView = GetScDrawView();

        if(pDrView)
        {
            SdrPageView* pPageView = pDrView->GetPageViewPvNum(0);

            if(pPageView)
            {
                // create the default object
                SdrObject* pObj = pFuActual->CreateDefaultObject(nNewId, aNewObjectRectangle);

                if(pObj)
                {
                    // insert into page
                    pView->InsertObject(pObj, *pPageView, pView->IsSolidDraggingNow() ? SDRINSERT_NOBROADCAST : 0);

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
}

void ScTabViewShell::GetDrawState(SfxItemSet &rSet)
{
    SfxWhichIter    aIter(rSet);
    USHORT          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_DRAW:
                {
                    //  SID_OBJECT_SELECT nur, wenn "harter" Selektionsmodus
                    USHORT nPutId = nDrawSfxId;
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
                    BOOL bOle = GetViewFrame()->ISA(SfxInPlaceFrame);
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

BOOL ScTabViewShell::SelectObject( const String& rName )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();
    if (!pView)
        return FALSE;

    BOOL bFound = pView->SelectObject( rName );
    // DrawShell etc. is handled in MarkListHasChanged

    return bFound;
}



