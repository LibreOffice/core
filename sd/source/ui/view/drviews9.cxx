/*************************************************************************
 *
 *  $RCSfile: drviews9.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:43 $
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

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX__XGRADIENT_HXX //autogen
#include <svx/xgrad.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX //autogen
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XFLHTIT_HXX //autogen
#include <svx/xflhtit.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX //autogen
#include <svx/xflgrit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX //autogen
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
#endif
#ifndef _EXPLORER_HXX //autogen
#include <sfx2/explorer.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#ifndef _GALLERY_HXX_ //autogen
#include <svx/gallery.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#pragma hdrstop

#define ITEMID_SIZE             0
#define ITEMID_LRSPACE          EE_PARA_LRSPACE
#define ITEMID_ULSPACE          EE_PARA_ULSPACE

#include "app.hrc"
#include "strings.hrc"

#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "drviewsh.hxx"
#include "docshell.hxx"
#include "sdresid.hxx"
#include "fupoor.hxx"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif


/*************************************************************************
|*
|* SfxRequests fuer Gallery bearbeiten
|*
\************************************************************************/

void SdDrawViewShell::ExecGallery(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if ( pArgs )
    {
        const UINT32        nFormats = ( (SfxUInt32Item&) pArgs->Get( SID_GALLERY_FORMATS ) ).GetValue();
        GalleryExplorer*    pGal = SVX_GALLERY();

        if ( pGal )
        {
            pDocSh->SetWaitCursor( TRUE );

            // Graphik einfuegen
            if (nFormats & SGA_FORMAT_GRAPHIC)
            {
                Graphic aGraphic = pGal->GetGraphic();

                // Ggf. Groesse reduzieren
                Window aWindow (pWindow);
                aWindow.SetMapMode(aGraphic.GetPrefMapMode());
                Size aSizePix = aWindow.LogicToPixel(aGraphic.GetPrefSize());
                aWindow.SetMapMode( MapMode(MAP_100TH_MM) );
                Size aSize = aWindow.PixelToLogic(aSizePix);

                // Groesse ggf. auf Seitengroesse begrenzen
                SdrPage* pPage = pDrView->GetPageViewPvNum(0)->GetPage();
                Size aPageSize = pPage->GetSize();
                aPageSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
                aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();


                // Falls Grafik zu gross, wird die Grafik
                // in die Seite eingepasst
                if ((aSize.Height() > aPageSize.Height()) ||
                    (aSize.Width()  > aPageSize.Width()) &&
                    aSize.Height() && aPageSize.Height())
                {
                    float fGrfWH =  (float)aSize.Width() /
                                    (float)aSize.Height();
                    float fWinWH =  (float)aPageSize.Width() /
                                    (float)aPageSize.Height();

                    // Grafik an Pagesize anpassen (skaliert)
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


                // Ausgaberechteck fuer Grafik setzen
                Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                            (aPageSize.Height() - aSize.Height()) / 2);
                aPnt += Point(pPage->GetLftBorder(), pPage->GetUppBorder());
                Rectangle aRect (aPnt, aSize);

                SdrGrafObj* pGrafObj = NULL;

                BOOL bInsertNewObject = TRUE;

                if ( pDrView->HasMarkedObj() )
                {
                    /******************************************************
                    * Ist ein leeres Graphik-Objekt vorhanden?
                    ******************************************************/
                    const SdrMarkList& rMarkList = pDrView->GetMarkList();

                    if (rMarkList.GetMarkCount() == 1)
                    {
                        SdrMark* pMark = rMarkList.GetMark(0);
                        SdrObject* pObj = pMark->GetObj();

                        if (pObj->GetObjInventor() == SdrInventor &&
                            pObj->GetObjIdentifier() == OBJ_GRAF)
                        {
                            pGrafObj = (SdrGrafObj*) pObj;

                            if ( pGrafObj->IsEmptyPresObj() )
                            {
                                /******************************************
                                * Das leere Graphik-Objekt bekommt eine neue
                                * Graphik
                                ******************************************/
                                bInsertNewObject = FALSE;

                                SdrGrafObj* pNewGrafObj = (SdrGrafObj*) pGrafObj->Clone();
                                pNewGrafObj->SetEmptyPresObj(FALSE);
                                pNewGrafObj->SetOutlinerParaObject(NULL);
                                pNewGrafObj->SetGraphic(aGraphic);

                                String aStr(pDrView->GetMarkDescription());
                                aStr += sal_Unicode(' ');
                                aStr += String(SdResId(STR_UNDO_REPLACE));
                                pDrView->BegUndo(aStr);
                                SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
                                pDrView->ReplaceObject(pGrafObj, *pPV, pNewGrafObj);
                                pDrView->EndUndo();
                            }
                        }
                    }
                }


                if (bInsertNewObject)
                {
                    pGrafObj = new SdrGrafObj(aGraphic, aRect);
                    SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
                    pDrView->InsertObject(pGrafObj, *pPV, SDRINSERT_SETDEFLAYER);
                }

                // Soll nur ein Link benutzt werden?
                if( pGrafObj && pGal->IsLinkage() )
                {
                    pGrafObj->SetGraphicLink( pGal->GetURL().PathToFileName(), pGal->GetFilterName() );
                }
            }
            // Sound als OLE-Objekt einfuegen
            else if (nFormats & SGA_FORMAT_SOUND)
            {
                const               String aFile( pGal->GetURL().PathToFileName() );
                SvInPlaceObjectRef  aIPObj;
                SvStorageRef        aStor = new SvStorage( String(), STREAM_STD_READWRITE );
                String              aName;

                aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(aFile, aStor);
                if (aIPObj.Is())
                {
                    Size        aSize(aIPObj->GetVisArea(ASPECT_CONTENT).GetSize());

                    if (!aSize.Width() || !aSize.Height())
                    {
                        aSize.Width()   = 1410;
                        aSize.Height()  = 1000;
                    }

                    SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
                    Size aPageSize = pPV->GetPage()->GetSize();
                    Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                                (aPageSize.Height() - aSize.Height()) / 2);

                    Rectangle aRect(aPnt, aSize);

                    aName = pDocSh->InsertObject(aIPObj, String())->GetObjName();

                    SdrOle2Obj* pOleObj = new SdrOle2Obj(aIPObj, aName, aRect);

                    pDrView->InsertObject(pOleObj, *pPV, SDRINSERT_SETDEFLAYER);
                    pOleObj->SetLogicRect(aRect);
                    aIPObj->SetVisAreaSize(aRect.GetSize());
                    ActivateObject(pOleObj, 0);
                }
            }

            pDocSh->SetWaitCursor( FALSE );
        }
    }
}


/*************************************************************************
|*
|* Statuswerte fuer Gallery zurueckgeben
|*
\************************************************************************/

void SdDrawViewShell::GetGalleryState(SfxItemSet& rSet)
{
}

/*************************************************************************
|*
|* Makros fuer die Attributeinstellungen bearbeiten
|*
\************************************************************************/

//
// die vorgehensweise fuer die attributaenderung ist praktisch ueberall identisch
// 1. bisherige attribute auslesen
// 2. parameter aus dem basic-set auslesen
// 3. gewaehltes item aus dem attribut-set loeschen
// 4. neues attribut-item erzeugen
// 5. item in den set eintragen
//
void SdDrawViewShell::AttrExec (SfxRequest &rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->    GetSlotID() == SID_PRESENTATION)
        return;


    CheckLineTo (rReq);

    SfxItemSet* pAttr = new SfxItemSet ( pDoc->GetPool() );
    pView->GetAttributes( *pAttr );
    const SfxItemSet* pArgs = rReq.GetArgs();

    switch (rReq.GetSlot ())
    {
        // neuen fuellstil setzen
        case SID_SETFILLSTYLE :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pFillStyle, SfxUInt32Item, ID_VAL_STYLE, FALSE);
                    if (CHECK_RANGE (XFILL_NONE, pFillStyle->GetValue (), XFILL_BITMAP))
                    {
                        pAttr->ClearItem (XATTR_FILLSTYLE);
                        pAttr->Put (XFillStyleItem ((XFillStyle) pFillStyle->GetValue ()), XATTR_FILLSTYLE);
                        SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        // linienstil neu bestimmen
        case SID_SETLINESTYLE :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pLineStyle, SfxUInt32Item, ID_VAL_STYLE, FALSE);
                    if (CHECK_RANGE (XLINE_NONE, pLineStyle->GetValue (), XLINE_DASH))
                    {
                        pAttr->ClearItem (XATTR_LINESTYLE);
                        pAttr->Put (XLineStyleItem ((XLineStyle) pLineStyle->GetValue ()), XATTR_LINESTYLE);
                        SFX_BINDINGS ().Invalidate (SID_ATTR_LINE_STYLE);
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        // linienbreite setzen
        case SID_SETLINEWIDTH :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pLineWidth, SfxUInt32Item, ID_VAL_WIDTH, FALSE);
                    pAttr->ClearItem (XATTR_LINEWIDTH);
                    pAttr->Put (XLineWidthItem (pLineWidth->GetValue ()), XATTR_LINEWIDTH);
                    SFX_BINDINGS ().Invalidate (SID_ATTR_LINE_WIDTH);
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_SETFILLCOLOR :
            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, FALSE);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, FALSE);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, FALSE);

                    pAttr->ClearItem (XATTR_FILLCOLOR);
                    pAttr->ClearItem (XATTR_FILLSTYLE);
                    pAttr->Put (XFillColorItem (-1, Color ((short) pRed->GetValue (), (short) pGreen->GetValue (),
                                                           (short) pBlue->GetValue ())),
                                XATTR_FILLCOLOR);
                    pAttr->Put (XFillStyleItem (XFILL_SOLID), XATTR_FILLSTYLE);
                    SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_COLOR);
                    SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_SETLINECOLOR :
            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, FALSE);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, FALSE);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, FALSE);

                    pAttr->ClearItem (XATTR_LINECOLOR);
                    pAttr->Put (XLineColorItem (-1, Color ((short) pRed->GetValue (), (short) pGreen->GetValue (),
                                                           (short) pBlue->GetValue ())),
                                XATTR_LINECOLOR);
                    SFX_BINDINGS ().Invalidate (SID_ATTR_LINE_COLOR);
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_SETGRADSTARTCOLOR :
        case SID_SETGRADENDCOLOR :
            if (pArgs)
                if (pArgs->Count () == 4)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, FALSE);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, FALSE);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, FALSE);

                    XGradientList *pGradientList = pDoc->GetGradientList ();
                    long          nCounts        = pGradientList->Count ();
                    Color         aColor ((short) pRed->GetValue (), (short) pGreen->GetValue (),
                                          (short) pBlue->GetValue ());

                    pAttr->ClearItem (XATTR_FILLGRADIENT);
                    pAttr->ClearItem (XATTR_FILLSTYLE);

                    for (long i = 0;
                              i < nCounts;
                              i ++)
                    {
                        XGradientEntry *pEntry = pGradientList->Get (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            XGradient &rGradient = pEntry->GetGradient ();

                            if (rReq.GetSlot () == SID_SETGRADSTARTCOLOR) rGradient.SetStartColor (aColor);
                            else rGradient.SetEndColor (aColor);

                            pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                            pAttr->Put (XFillGradientItem (pName->GetValue (), rGradient), XATTR_FILLGRADIENT);
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

                        pDoc->GetGradientList ()->Insert (new XGradientEntry (aGradient, pName->GetValue ()));

                        pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                        pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
                    }

                    SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_GRADIENT);
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_SETHATCHCOLOR :
            if (pArgs)
                if (pArgs->Count () == 4)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);
                    SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, FALSE);
                    SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, FALSE);
                    SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, FALSE);

                    XHatchList *pHatchList = pDoc->GetHatchList ();
                    long       nCounts     = pHatchList->Count ();
                    Color      aColor ((short) pRed->GetValue (), (short) pGreen->GetValue (),
                                       (short) pBlue->GetValue ());

                    pAttr->ClearItem (XATTR_FILLHATCH);
                    pAttr->ClearItem (XATTR_FILLSTYLE);

                    for (long i = 0;
                              i < nCounts;
                              i ++)
                    {
                        XHatchEntry *pEntry = pHatchList->Get (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            XHatch &rHatch = pEntry->GetHatch ();

                            rHatch.SetColor (aColor);

                            pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                            pAttr->Put (XFillHatchItem (pName->GetValue (), rHatch), XATTR_FILLHATCH);
                            break;
                        }
                    }

                    if (i >= nCounts)
                    {
                        XHatch aHatch (aColor);

                        pDoc->GetHatchList ()->Insert (new XHatchEntry (aHatch, pName->GetValue ()));

                        pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                        pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
                    }

                    SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_HATCH);
                    SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        // einstellungen fuer liniendash
        case SID_DASH :
            if (pArgs)
                if (pArgs->Count () == 7)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);
                    SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, FALSE);
                    SFX_REQUEST_ARG (rReq, pDots, SfxUInt32Item, ID_VAL_DOTS, FALSE);
                    SFX_REQUEST_ARG (rReq, pDotLen, SfxUInt32Item, ID_VAL_DOTLEN, FALSE);
                    SFX_REQUEST_ARG (rReq, pDashes, SfxUInt32Item, ID_VAL_DASHES, FALSE);
                    SFX_REQUEST_ARG (rReq, pDashLen, SfxUInt32Item, ID_VAL_DASHLEN, FALSE);
                    SFX_REQUEST_ARG (rReq, pDistance, SfxUInt32Item, ID_VAL_DISTANCE, FALSE);

                    if (CHECK_RANGE (XDASH_RECT, pStyle->GetValue (), XDASH_ROUNDRELATIVE))
                    {
                        XDash aNewDash ((XDashStyle) pStyle->GetValue (), (short) pDots->GetValue (), pDotLen->GetValue (),
                                        (short) pDashes->GetValue (), pDashLen->GetValue (), pDistance->GetValue ());

                        pAttr->ClearItem (XATTR_LINEDASH);
                        pAttr->ClearItem (XATTR_LINESTYLE);

                        XDashList  *pDashList = pDoc->GetDashList ();
                        long       nCounts    = pDashList->Count ();
                        XDashEntry *pEntry    = new XDashEntry (aNewDash, pName->GetValue ());

                        for (long i = 0;
                                  i < nCounts;
                                  i ++)
                            if (pDashList->Get (i)->GetName () == pName->GetValue ()) break;

                        if (i < nCounts) pDashList->Replace (pEntry, i);
                        else pDashList->Insert (pEntry);

                        pAttr->Put (XLineDashItem (pName->GetValue (), aNewDash), XATTR_LINEDASH);
                        pAttr->Put (XLineStyleItem (XLINE_DASH), XATTR_LINESTYLE);
                        SFX_BINDINGS ().Invalidate (SID_ATTR_LINE_DASH);
                        SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        // einstellungen fuer farbverlauf
        case SID_GRADIENT :
            if (pArgs)
                if (pArgs->Count () == 8)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);
                    SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, FALSE);
                    SFX_REQUEST_ARG (rReq, pAngle, SfxUInt32Item, ID_VAL_ANGLE, FALSE);
                    SFX_REQUEST_ARG (rReq, pBorder, SfxUInt32Item, ID_VAL_BORDER, FALSE);
                    SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, FALSE);
                    SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, FALSE);
                    SFX_REQUEST_ARG (rReq, pStart, SfxUInt32Item, ID_VAL_STARTINTENS, FALSE);
                    SFX_REQUEST_ARG (rReq, pEnd, SfxUInt32Item, ID_VAL_ENDINTENS, FALSE);

                    if (CHECK_RANGE (XGRAD_LINEAR, pStyle->GetValue (), XGRAD_RECT) &&
                        CHECK_RANGE (0, pAngle->GetValue (), 360) &&
                        CHECK_RANGE (0, pBorder->GetValue (), 100) &&
                        CHECK_RANGE (0, pCenterX->GetValue (), 100) &&
                        CHECK_RANGE (0, pCenterY->GetValue (), 100) &&
                        CHECK_RANGE (0, pStart->GetValue (), 100) &&
                        CHECK_RANGE (0, pEnd->GetValue (), 100))
                    {
                        pAttr->ClearItem (XATTR_FILLGRADIENT);
                        pAttr->ClearItem (XATTR_FILLSTYLE);

                        XGradientList  *pGradientList = pDoc->GetGradientList ();
                        long           nCounts        = pGradientList->Count ();

                        for (long i = 0;
                                  i < nCounts;
                                  i ++)
                        {
                            XGradientEntry *pEntry = pGradientList->Get (i);

                            if (pEntry->GetName () == pName->GetValue ())
                            {
                                XGradient &rGradient = pEntry->GetGradient ();

                                rGradient.SetGradientStyle ((XGradientStyle) pStyle->GetValue ());
                                rGradient.SetAngle (pAngle->GetValue () * 10);
                                rGradient.SetBorder ((short) pBorder->GetValue ());
                                rGradient.SetXOffset ((short) pCenterX->GetValue ());
                                rGradient.SetYOffset ((short) pCenterY->GetValue ());
                                rGradient.SetStartIntens ((short) pStart->GetValue ());
                                rGradient.SetEndIntens ((short) pEnd->GetValue ());

                                pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                                pAttr->Put (XFillGradientItem (pName->GetValue (), rGradient), XATTR_FILLGRADIENT);
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

                        SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_GRADIENT);
                        SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        // einstellungen fuer schraffur
        case SID_HATCH :
            if (pArgs)
                if (pArgs->Count () == 4)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);
                    SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, FALSE);
                    SFX_REQUEST_ARG (rReq, pDistance, SfxUInt32Item, ID_VAL_DISTANCE, FALSE);
                    SFX_REQUEST_ARG (rReq, pAngle, SfxUInt32Item, ID_VAL_ANGLE, FALSE);

                    if (CHECK_RANGE (XHATCH_SINGLE, pStyle->GetValue (), XHATCH_TRIPLE) &&
                        CHECK_RANGE (0, pAngle->GetValue (), 360))
                    {
                        pAttr->ClearItem (XATTR_FILLHATCH);
                        pAttr->ClearItem (XATTR_FILLSTYLE);

                        XHatchList *pHatchList = pDoc->GetHatchList ();
                        long       nCounts     = pHatchList->Count ();

                        for (long i = 0;
                                  i < nCounts;
                                  i ++)
                        {
                            XHatchEntry *pEntry = pHatchList->Get (i);

                            if (pEntry->GetName () == pName->GetValue ())
                            {
                                XHatch &rHatch = pEntry->GetHatch ();

                                rHatch.SetHatchStyle ((XHatchStyle) pStyle->GetValue ());
                                rHatch.SetDistance (pDistance->GetValue ());
                                rHatch.SetAngle (pAngle->GetValue () * 10);

                                pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                                pAttr->Put (XFillHatchItem (pName->GetValue (), rHatch), XATTR_FILLHATCH);
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

                        SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_HATCH);
                        SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_SELECTGRADIENT :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);

                    XGradientList  *pGradientList = pDoc->GetGradientList ();
                    long           nCounts        = pGradientList->Count ();

                    for (long i = 0;
                              i < nCounts;
                              i ++)
                    {
                        XGradientEntry *pEntry = pGradientList->Get (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            pAttr->ClearItem (XATTR_FILLGRADIENT);
                            pAttr->ClearItem (XATTR_FILLSTYLE);
                            pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
                            pAttr->Put (XFillGradientItem (pName->GetValue (), pEntry->GetGradient ()), XATTR_FILLGRADIENT);

                            SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_GRADIENT);
                            SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                            break;
                        }
                    }

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_SELECTHATCH :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, FALSE);

                    XHatchList *pHatchList = pDoc->GetHatchList ();
                    long       nCounts     = pHatchList->Count ();

                    for (long i = 0;
                              i < nCounts;
                              i ++)
                    {
                        XHatchEntry *pEntry = pHatchList->Get (i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            pAttr->ClearItem (XATTR_FILLHATCH);
                            pAttr->ClearItem (XATTR_FILLSTYLE);
                            pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
                            pAttr->Put (XFillHatchItem (pName->GetValue (), pEntry->GetHatch ()), XATTR_FILLHATCH);

                            SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_HATCH);
                            SFX_BINDINGS ().Invalidate (SID_ATTR_FILL_STYLE);
                            break;
                        }
                    }

                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            break;

        case SID_UNSELECT :
            pDrView->UnmarkAll ();
            break;

        case SID_GETRED :
            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
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

    pDrView->SetAttributes (*(const SfxItemSet *) pAttr);
    rReq.Ignore ();
    delete pAttr;
}

/*************************************************************************
|*
|* Makros fuer die Attributeinstellungen bearbeiten
|*
\************************************************************************/

void SdDrawViewShell::AttrState (SfxItemSet& rSet)
{
    SfxWhichIter     aIter (rSet);
    USHORT           nWhich = aIter.FirstWhich ();
    SfxItemSet aAttr( pDoc->GetPool() );
    pDrView->GetAttributes( aAttr );

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

                        aColor = rLineColorItem.GetValue ();
                        break;
                    }

                    case 2 :
                    {
                        const XFillColorItem &rFillColorItem = (const XFillColorItem &) aAttr.Get (XATTR_FILLCOLOR);

                        aColor = rFillColorItem.GetValue ();
                        break;
                    }

                    case 3 :
                    case 4 :
                    {
                        const XFillGradientItem &rFillGradientItem = (const XFillGradientItem &) aAttr.Get (XATTR_FILLGRADIENT);
                        const XGradient         &rGradient         = rFillGradientItem.GetValue ();

                        aColor = (rWhatKind.GetValue () == 3)
                                    ? rGradient.GetStartColor ()
                                    : rGradient.GetEndColor ();
                        break;
                    }

                    case 5:
                    {
                        const XFillHatchItem &rFillHatchItem = (const XFillHatchItem &) aAttr.Get (XATTR_FILLHATCH);
                        const XHatch         &rHatch         = rFillHatchItem.GetValue ();

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


