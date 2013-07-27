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


#include "DrawViewShell.hxx"
#include <vcl/wrkwin.hxx>
#include <svx/xgrad.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <sfx2/bindings.hxx>

#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <svx/svxids.hrc>
#include <svx/xtable.hxx>
#include <svx/gallery.hxx>
#include <vcl/graph.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <svl/whiter.hxx>
#include <basic/sbstar.hxx>

#include <sfx2/viewfrm.hxx>


#include "app.hrc"
#include "strings.hrc"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "fupoor.hxx"

#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>

namespace sd {


void DrawViewShell::ExecGallery(SfxRequest& rReq)
{
    // nothing is executed during a slide show!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    const SfxItemSet* pArgs = rReq.GetArgs();

    SFX_ITEMSET_ARG( pArgs, pGalleryItem, SvxGalleryItem, SID_GALLERY_FORMATS, sal_False );
    if ( !pGalleryItem )
        return;

    GetDocSh()->SetWaitCursor( sal_True );

    sal_Int8 nType( pGalleryItem->GetType() );
    // insert graphic
    if (nType == com::sun::star::gallery::GalleryItemType::GRAPHIC)
    {
        Graphic aGraphic( pGalleryItem->GetGraphic() );

        // reduce size if necessary
        Window aWindow (GetActiveWindow());
        aWindow.SetMapMode(aGraphic.GetPrefMapMode());
        Size aSizePix = aWindow.LogicToPixel(aGraphic.GetPrefSize());
        aWindow.SetMapMode( MapMode(MAP_100TH_MM) );
        Size aSize = aWindow.PixelToLogic(aSizePix);

        // constrain size to page size if necessary
        SdrPage* pPage = mpDrawView->GetSdrPageView()->GetPage();
        Size aPageSize = pPage->GetSize();
        aPageSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();


        // Falls Grafik zu gross, wird die Grafik
        // in die Seite eingepasst
        if ( ( ( aSize.Height() > aPageSize.Height() ) || ( aSize.Width()   > aPageSize.Width() ) ) &&
            aSize.Height() && aPageSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aPageSize.Width() /
                            (float)aPageSize.Height();

            // constrain size to page size if necessary
            if ((fGrfWH != 0.F) && (fGrfWH < fWinWH))
            {
                aSize.Width() = (long)(aPageSize.Height() * fGrfWH);
                aSize.Height()= aPageSize.Height();
            }
            else
            {
                aSize.Width() = aPageSize.Width();
                aSize.Height()= (long)(aPageSize.Width() / fGrfWH);
            }
        }

        // set output rectangle for graphic
        Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                    (aPageSize.Height() - aSize.Height()) / 2);
        aPnt += Point(pPage->GetLftBorder(), pPage->GetUppBorder());
        Rectangle aRect (aPnt, aSize);

        SdrGrafObj* pGrafObj = NULL;

        sal_Bool bInsertNewObject = sal_True;

        if ( mpDrawView->AreObjectsMarked() )
        {
            // is there a empty graphic object?
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();

                if (pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF)
                {
                    pGrafObj = (SdrGrafObj*) pObj;

                    if( pGrafObj->IsEmptyPresObj() )
                    {
                        // the empty graphic object gets a new graphic
                        bInsertNewObject = sal_False;

                        SdrGrafObj* pNewGrafObj = (SdrGrafObj*) pGrafObj->Clone();
                        pNewGrafObj->SetEmptyPresObj(sal_False);
                        pNewGrafObj->SetOutlinerParaObject(NULL);
                        pNewGrafObj->SetGraphic(aGraphic);

                        String aStr(mpDrawView->GetDescriptionOfMarkedObjects());
                        aStr += sal_Unicode(' ');
                        aStr += String(SdResId(STR_UNDO_REPLACE));
                        mpDrawView->BegUndo(aStr);
                        SdrPageView* pPV = mpDrawView->GetSdrPageView();
                        mpDrawView->ReplaceObjectAtView(pGrafObj, *pPV, pNewGrafObj);
                        mpDrawView->EndUndo();
                    }
                }
            }
        }


        if( bInsertNewObject )
        {
            pGrafObj = new SdrGrafObj(aGraphic, aRect);
            SdrPageView* pPV = mpDrawView->GetSdrPageView();
            mpDrawView->InsertObjectAtView(pGrafObj, *pPV, SDRINSERT_SETDEFLAYER);
        }
    }
    // insert sound
    else if( nType == com::sun::star::gallery::GalleryItemType::MEDIA )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, pGalleryItem->GetURL() );
        GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_AVMEDIA, SFX_CALLMODE_SYNCHRON, &aMediaURLItem, 0L );
    }

    GetDocSh()->SetWaitCursor( sal_False );
}



void DrawViewShell::GetGalleryState(SfxItemSet& )
{
}

/**
 * Edit macros for attribute configuration
 */

/* the work flow to adjust the attributes is nearly everywhere the same
   1. read existing attributes
   2. read parameter from the basic-set
   3. delete selected item from the attribute-set
   4. create new attribute-item
   5. insert item into set      */
void DrawViewShell::AttrExec (SfxRequest &rReq)
{
    // nothing is executed during a slide show!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    CheckLineTo (rReq);

    SfxBindings&    rBindings = GetViewFrame()->GetBindings();
    SfxItemSet*     pAttr = new SfxItemSet ( GetDoc()->GetPool() );

    GetView()->GetAttributes( *pAttr );
    const SfxItemSet* pArgs = rReq.GetArgs();

    switch (rReq.GetSlot ())
    {
        // set new fill-style
        case SID_SETFILLSTYLE :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pFillStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
                    if (CHECK_RANGE (XFILL_NONE, (sal_Int32)pFillStyle->GetValue (), XFILL_BITMAP))
                    {
                        pAttr->ClearItem (XATTR_FILLSTYLE);
                        pAttr->Put (XFillStyleItem ((XFillStyle) pFillStyle->GetValue ()), XATTR_FILLSTYLE);
                        rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    }
#ifndef DISABLE_SCRIPTING
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        // determine new line style
        case SID_SETLINESTYLE :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pLineStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
                    if (CHECK_RANGE (XLINE_NONE, (sal_Int32)pLineStyle->GetValue (), XLINE_DASH))
                    {
                        pAttr->ClearItem (XATTR_LINESTYLE);
                        pAttr->Put (XLineStyleItem ((XLineStyle) pLineStyle->GetValue ()), XATTR_LINESTYLE);
                        rBindings.Invalidate (SID_ATTR_LINE_STYLE);
                    }
#ifndef DISABLE_SCRIPTING
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        // set line width
        case SID_SETLINEWIDTH :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pLineWidth, SfxUInt32Item, ID_VAL_WIDTH, sal_False);
                    pAttr->ClearItem (XATTR_LINEWIDTH);
                    pAttr->Put (XLineWidthItem (pLineWidth->GetValue ()), XATTR_LINEWIDTH);
                    rBindings.Invalidate (SID_ATTR_LINE_WIDTH);
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_SETFILLCOLOR :
            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

                    pAttr->ClearItem (XATTR_FILLCOLOR);
                    pAttr->ClearItem (XATTR_FILLSTYLE);
                    pAttr->Put (XFillColorItem (-1, Color ((sal_uInt8) pRed->GetValue (),
                                                           (sal_uInt8) pGreen->GetValue (),
                                                           (sal_uInt8) pBlue->GetValue ())),
                                XATTR_FILLCOLOR);
                    pAttr->Put (XFillStyleItem (XFILL_SOLID), XATTR_FILLSTYLE);
                    rBindings.Invalidate (SID_ATTR_FILL_COLOR);
                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_SETLINECOLOR :
            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

                    pAttr->ClearItem (XATTR_LINECOLOR);
                    pAttr->Put (XLineColorItem (-1, Color ((sal_uInt8) pRed->GetValue (),
                                                           (sal_uInt8) pGreen->GetValue (),
                                                           (sal_uInt8) pBlue->GetValue ())),
                                XATTR_LINECOLOR);
                    rBindings.Invalidate (SID_ATTR_LINE_COLOR);
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_SETGRADSTARTCOLOR :
        case SID_SETGRADENDCOLOR :
            if (pArgs)
                if (pArgs->Count () == 4)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

                    XGradientListRef pGradientList = GetDoc()->GetGradientList ();
                    long          nCounts        = pGradientList->Count ();
                    Color         aColor ((sal_uInt8) pRed->GetValue (),
                                          (sal_uInt8) pGreen->GetValue (),
                                          (sal_uInt8) pBlue->GetValue ());
                    long i;

                    pAttr->ClearItem (XATTR_FILLGRADIENT);
                    pAttr->ClearItem (XATTR_FILLSTYLE);

                    for ( i = 0; i < nCounts; i ++)
                    {
                        XGradientEntry *pEntry = pGradientList->GetGradient (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            XGradient aGradient(pEntry->GetGradient());

                            if (rReq.GetSlot () == SID_SETGRADSTARTCOLOR) aGradient.SetStartColor (aColor);
                            else aGradient.SetEndColor (aColor);

                            pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                            pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
                            break;
                        }
                    }

                    if (i >= nCounts)
                    {
                        Color aBlack (0, 0, 0);
                        XGradient aGradient ((rReq.GetSlot () == SID_SETGRADSTARTCOLOR)
                                                 ? aColor
                                                 : aBlack,
                                             (rReq.GetSlot () == SID_SETGRADENDCOLOR)
                                                 ? aColor
                                                 : aBlack);

                        GetDoc()->GetGradientList ()->Insert (new XGradientEntry (aGradient, pName->GetValue ()));

                        pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                        pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
                    }

                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_SETHATCHCOLOR :
            if (pArgs)
                if (pArgs->Count () == 4)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

                    XHatchListRef pHatchList = GetDoc()->GetHatchList ();
                    long       nCounts     = pHatchList->Count ();
                    Color      aColor ((sal_uInt8) pRed->GetValue (),
                                       (sal_uInt8) pGreen->GetValue (),
                                       (sal_uInt8) pBlue->GetValue ());
                    long i;

                    pAttr->ClearItem (XATTR_FILLHATCH);
                    pAttr->ClearItem (XATTR_FILLSTYLE);

                    for ( i = 0; i < nCounts; i ++)
                    {
                        XHatchEntry *pEntry = pHatchList->GetHatch (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            XHatch aHatch(pEntry->GetHatch());

                            aHatch.SetColor (aColor);

                            pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                            pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
                            break;
                        }
                    }

                    if (i >= nCounts)
                    {
                        XHatch aHatch (aColor);

                        GetDoc()->GetHatchList ()->Insert (new XHatchEntry (aHatch, pName->GetValue ()));

                        pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                        pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
                    }

                    rBindings.Invalidate (SID_ATTR_FILL_HATCH);
                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        // einstellungen fuer liniendash
        case SID_DASH :
            if (pArgs)
                if (pArgs->Count () == 7)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
                    SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
                    SFX_REQUEST_ARG (rReq, pDots, SfxUInt32Item, ID_VAL_DOTS, sal_False);
                    SFX_REQUEST_ARG (rReq, pDotLen, SfxUInt32Item, ID_VAL_DOTLEN, sal_False);
                    SFX_REQUEST_ARG (rReq, pDashes, SfxUInt32Item, ID_VAL_DASHES, sal_False);
                    SFX_REQUEST_ARG (rReq, pDashLen, SfxUInt32Item, ID_VAL_DASHLEN, sal_False);
                    SFX_REQUEST_ARG (rReq, pDistance, SfxUInt32Item, ID_VAL_DISTANCE, sal_False);

                    if (CHECK_RANGE (XDASH_RECT, (sal_Int32)pStyle->GetValue (), XDASH_ROUNDRELATIVE))
                    {
                        XDash aNewDash ((XDashStyle) pStyle->GetValue (), (short) pDots->GetValue (), pDotLen->GetValue (),
                                        (short) pDashes->GetValue (), pDashLen->GetValue (), pDistance->GetValue ());

                        pAttr->ClearItem (XATTR_LINEDASH);
                        pAttr->ClearItem (XATTR_LINESTYLE);

                        XDashListRef pDashList = GetDoc()->GetDashList();
                        long       nCounts    = pDashList->Count ();
                        XDashEntry *pEntry    = new XDashEntry (aNewDash, pName->GetValue ());
                        long i;

                        for ( i = 0; i < nCounts; i++ )
                            if (pDashList->GetDash (i)->GetName () == pName->GetValue ())
                                break;

                        if (i < nCounts)
                            pDashList->Replace (pEntry, i);
                        else
                            pDashList->Insert (pEntry);

                        pAttr->Put (XLineDashItem (pName->GetValue (), aNewDash), XATTR_LINEDASH);
                        pAttr->Put (XLineStyleItem (XLINE_DASH), XATTR_LINESTYLE);
                        rBindings.Invalidate (SID_ATTR_LINE_DASH);
                        rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    }
#ifndef DISABLE_SCRIPTING
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        // configuration for gradients
        case SID_GRADIENT :
            if (pArgs)
                if (pArgs->Count () == 8)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
                    SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
                    SFX_REQUEST_ARG (rReq, pAngle, SfxUInt32Item, ID_VAL_ANGLE, sal_False);
                    SFX_REQUEST_ARG (rReq, pBorder, SfxUInt32Item, ID_VAL_BORDER, sal_False);
                    SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, sal_False);
                    SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, sal_False);
                    SFX_REQUEST_ARG (rReq, pStart, SfxUInt32Item, ID_VAL_STARTINTENS, sal_False);
                    SFX_REQUEST_ARG (rReq, pEnd, SfxUInt32Item, ID_VAL_ENDINTENS, sal_False);

                    if (CHECK_RANGE (XGRAD_LINEAR, (sal_Int32)pStyle->GetValue (), XGRAD_RECT) &&
                        CHECK_RANGE (0, (sal_Int32)pAngle->GetValue (), 360) &&
                        CHECK_RANGE (0, (sal_Int32)pBorder->GetValue (), 100) &&
                        CHECK_RANGE (0, (sal_Int32)pCenterX->GetValue (), 100) &&
                        CHECK_RANGE (0, (sal_Int32)pCenterY->GetValue (), 100) &&
                        CHECK_RANGE (0, (sal_Int32)pStart->GetValue (), 100) &&
                        CHECK_RANGE (0, (sal_Int32)pEnd->GetValue (), 100))
                    {
                        pAttr->ClearItem (XATTR_FILLGRADIENT);
                        pAttr->ClearItem (XATTR_FILLSTYLE);

                        XGradientListRef pGradientList = GetDoc()->GetGradientList ();
                        long           nCounts        = pGradientList->Count ();
                        long i;

                        for ( i = 0; i < nCounts; i++ )
                        {
                            XGradientEntry *pEntry = pGradientList->GetGradient (i);

                            if (pEntry->GetName () == pName->GetValue ())
                            {
                                XGradient aGradient(pEntry->GetGradient());

                                aGradient.SetGradientStyle ((XGradientStyle) pStyle->GetValue ());
                                aGradient.SetAngle (pAngle->GetValue () * 10);
                                aGradient.SetBorder ((short) pBorder->GetValue ());
                                aGradient.SetXOffset ((short) pCenterX->GetValue ());
                                aGradient.SetYOffset ((short) pCenterY->GetValue ());
                                aGradient.SetStartIntens ((short) pStart->GetValue ());
                                aGradient.SetEndIntens ((short) pEnd->GetValue ());

                                pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                                pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
                                break;
                            }
                        }

                        if (i >= nCounts)
                        {
                            Color aBlack (0, 0, 0);
                            XGradient aGradient (aBlack, aBlack, (XGradientStyle) pStyle->GetValue (),
                                                 pAngle->GetValue () * 10, (short) pCenterX->GetValue (),
                                                 (short) pCenterY->GetValue (), (short) pBorder->GetValue (),
                                                 (short) pStart->GetValue (), (short) pEnd->GetValue ());

                            pGradientList->Insert (new XGradientEntry (aGradient, pName->GetValue ()));
                            pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                            pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
                        }

                        rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
                        rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    }
#ifndef DISABLE_SCRIPTING
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        // configuration for hatch
        case SID_HATCH :
            if (pArgs)
                if (pArgs->Count () == 4)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
                    SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
                    SFX_REQUEST_ARG (rReq, pDistance, SfxUInt32Item, ID_VAL_DISTANCE, sal_False);
                    SFX_REQUEST_ARG (rReq, pAngle, SfxUInt32Item, ID_VAL_ANGLE, sal_False);

                    if (CHECK_RANGE (XHATCH_SINGLE, (sal_Int32)pStyle->GetValue (), XHATCH_TRIPLE) &&
                        CHECK_RANGE (0, (sal_Int32)pAngle->GetValue (), 360))
                    {
                        pAttr->ClearItem (XATTR_FILLHATCH);
                        pAttr->ClearItem (XATTR_FILLSTYLE);

                        XHatchListRef pHatchList = GetDoc()->GetHatchList ();
                        long       nCounts     = pHatchList->Count ();
                        long i;

                        for ( i = 0; i < nCounts; i++ )
                        {
                            XHatchEntry *pEntry = pHatchList->GetHatch (i);

                            if (pEntry->GetName () == pName->GetValue ())
                            {
                                XHatch aHatch(pEntry->GetHatch());

                                aHatch.SetHatchStyle ((XHatchStyle) pStyle->GetValue ());
                                aHatch.SetDistance (pDistance->GetValue ());
                                aHatch.SetAngle (pAngle->GetValue () * 10);

                                pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                                pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
                                break;
                            }
                        }

                        if (i >= nCounts)
                        {
                            Color  aBlack (0, 0, 0);
                            XHatch aHatch (aBlack, (XHatchStyle) pStyle->GetValue (), pDistance->GetValue (),
                                           pAngle->GetValue () * 10);

                            pHatchList->Insert (new XHatchEntry (aHatch, pName->GetValue ()));
                            pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                            pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
                        }

                        rBindings.Invalidate (SID_ATTR_FILL_HATCH);
                        rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    }
#ifndef DISABLE_SCRIPTING
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_SELECTGRADIENT :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);

                    XGradientListRef pGradientList = GetDoc()->GetGradientList ();
                    long           nCounts        = pGradientList->Count ();

                    for (long i = 0;
                              i < nCounts;
                              i ++)
                    {
                        XGradientEntry *pEntry = pGradientList->GetGradient (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            pAttr->ClearItem (XATTR_FILLGRADIENT);
                            pAttr->ClearItem (XATTR_FILLSTYLE);
                            pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                            pAttr->Put (XFillGradientItem (pName->GetValue (), pEntry->GetGradient ()), XATTR_FILLGRADIENT);

                            rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
                            rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                            break;
                        }
                    }

                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_SELECTHATCH :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);

                    XHatchListRef pHatchList = GetDoc()->GetHatchList ();
                    long       nCounts     = pHatchList->Count ();

                    for (long i = 0;
                              i < nCounts;
                              i ++)
                    {
                        XHatchEntry *pEntry = pHatchList->GetHatch (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            pAttr->ClearItem (XATTR_FILLHATCH);
                            pAttr->ClearItem (XATTR_FILLSTYLE);
                            pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                            pAttr->Put (XFillHatchItem (pName->GetValue (), pEntry->GetHatch ()), XATTR_FILLHATCH);

                            rBindings.Invalidate (SID_ATTR_FILL_HATCH);
                            rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                            break;
                        }
                    }

                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

        case SID_UNSELECT :
            mpDrawView->UnmarkAll ();
            break;

        case SID_GETRED :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    break;
                }
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
            break;

/*        case SID_SETFONTFAMILYNAME :
        case SID_SETFONTSTYLENAME :
        case SID_SETFONTFAMILY :
        case SID_SETFONTPITCH :
        case SID_SETFONTCHARSET :
        case SID_SETFONTPOSTURE :
        case SID_SETFONTWEIGHT :
        case SID_SETFONTUNDERLINE :
        case SID_SETFONTCROSSEDOUT :
        case SID_SETFONTSHADOWED :
        case SID_SETFONTCONTOUR :
        case SID_SETFONTCOLOR :
        case SID_SETFONTLANGUAGE :
        case SID_SETFONTWORDLINE :
        case SID_SETFONTCASEMAP :
        case SID_SETFONTESCAPE :
        case SID_SETFONTKERNING :
            break;*/

        default :
            ;
    }

    mpDrawView->SetAttributes (*(const SfxItemSet *) pAttr);
    rReq.Ignore ();
    delete pAttr;
}

/**
 * Edit macros for attribute configuration
 */
void DrawViewShell::AttrState (SfxItemSet& rSet)
{
    SfxWhichIter     aIter (rSet);
    sal_uInt16           nWhich = aIter.FirstWhich ();
    SfxItemSet aAttr( GetDoc()->GetPool() );
    mpDrawView->GetAttributes( aAttr );

    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_GETFILLSTYLE :
            {
                const XFillStyleItem &rFillStyleItem = (const XFillStyleItem &) aAttr.Get (XATTR_FILLSTYLE);

                rSet.Put (SfxUInt32Item (nWhich, (long) rFillStyleItem.GetValue ()));
                break;
            }

            case SID_GETLINESTYLE :
            {
                const XLineStyleItem &rLineStyleItem = (const XLineStyleItem &) aAttr.Get (XATTR_LINESTYLE);

                rSet.Put (SfxUInt32Item (nWhich, (long) rLineStyleItem.GetValue ()));
                break;
            }

            case SID_GETLINEWIDTH :
            {
                const XLineWidthItem &rLineWidthItem = (const XLineWidthItem &) aAttr.Get (XATTR_LINEWIDTH);

                rSet.Put (SfxUInt32Item (nWhich, (long) rLineWidthItem.GetValue ()));
                break;
            }

            case SID_GETGREEN :
            case SID_GETRED :
            case SID_GETBLUE :
            {
                const SfxUInt32Item &rWhatKind = (const SfxUInt32Item &) rSet.Get (ID_VAL_WHATKIND);
                Color               aColor;

                switch (rWhatKind.GetValue ())
                {
                    case 1 :
                    {
                        const XLineColorItem &rLineColorItem = (const XLineColorItem &) aAttr.Get (XATTR_LINECOLOR);

                        aColor = rLineColorItem.GetColorValue ();
                        break;
                    }

                    case 2 :
                    {
                        const XFillColorItem &rFillColorItem = (const XFillColorItem &) aAttr.Get (XATTR_FILLCOLOR);

                        aColor = rFillColorItem.GetColorValue ();
                        break;
                    }

                    case 3 :
                    case 4 :
                    {
                        const XFillGradientItem &rFillGradientItem = (const XFillGradientItem &) aAttr.Get (XATTR_FILLGRADIENT);
                        const XGradient         &rGradient         = rFillGradientItem.GetGradientValue ();

                        aColor = (rWhatKind.GetValue () == 3)
                                    ? rGradient.GetStartColor ()
                                    : rGradient.GetEndColor ();
                        break;
                    }

                    case 5:
                    {
                        const XFillHatchItem &rFillHatchItem = (const XFillHatchItem &) aAttr.Get (XATTR_FILLHATCH);
                        const XHatch         &rHatch         = rFillHatchItem.GetHatchValue ();

                        aColor = rHatch.GetColor ();
                        break;
                    }

                    default :
                        ;
                }

                rSet.Put (SfxUInt32Item (nWhich, (long) ((nWhich == SID_GETRED)
                                                             ? aColor.GetRed ()
                                                             : (nWhich == SID_GETGREEN)
                                                                   ? aColor.GetGreen ()
                                                                   : aColor.GetBlue ())));
                break;
            }

            default :
                ;
        }

        nWhich = aIter.NextWhich ();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
