/*************************************************************************
 *
 *  $RCSfile: client.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:17:27 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/sotref.hxx>
#include <so3/ipobj.hxx>
#include <so3/persist.hxx>
#include <so3/embobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/svdograf.hxx>

#include "client.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

//------------------------------------------------------------------------

ScClient::ScClient( ScTabViewShell* pViewShell, Window* pDraw, SdrModel* pSdrModel ) :
    SfxInPlaceClient( pViewShell, pDraw ),
    pModel( pSdrModel ),
    pGrafEdit( 0 )
{
}

__EXPORT ScClient::~ScClient()
{
}

SdrOle2Obj* ScClient::GetDrawObj()
{
    SvEmbeddedObject* pMyObj = GetEmbedObj();
    SvInfoObject* pInfoObj = GetViewShell()->GetViewFrame()->GetObjectShell()->Find( pMyObj );
    SdrOle2Obj* pOle2Obj = NULL;
//  String aName = pMyObj->GetName()->GetName();

    String aName;

    if ( pInfoObj )
        aName = pInfoObj->GetObjName();
    else
        DBG_ERROR( "IP-Object not found :-/" );

    USHORT nPages = pModel->GetPageCount();
    for (USHORT nPNr=0; nPNr<nPages && !pOle2Obj; nPNr++)
    {
        SdrPage* pPage = pModel->GetPage(nPNr);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject && !pOle2Obj)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                // name from InfoObject is PersistName
                if ( ((SdrOle2Obj*)pObject)->GetPersistName() == aName )
                    pOle2Obj = (SdrOle2Obj*)pObject;
            }
            pObject = aIter.Next();
        }
    }
    return pOle2Obj;
}

void __EXPORT ScClient::RequestObjAreaPixel( const Rectangle& rObjRect )
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
    if (!pViewSh)
    {
        DBG_ERROR("RequestObjAreaPixel: das ist nicht meine ViewShell");
        SfxInPlaceClient::RequestObjAreaPixel( rObjRect );
        return;
    }

    //  Position anpassen

    Rectangle aRect = rObjRect;
    Window* pWin = pViewSh->GetActiveWin();
    Rectangle aLogicRect( pWin->PixelToLogic( aRect.TopLeft() ),
                            pWin->PixelToLogic( aRect.GetSize() ) );

    BOOL bChange = FALSE;
    SCTAB nTab = pViewSh->GetViewData()->GetTabNo();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(static_cast<sal_Int16>(nTab)));
    if (pPage)
    {
        Point aPos;
        Size aSize = pPage->GetSize();
        if ( aSize.Width() < 0 )
        {
            aPos.X() = aSize.Width() + 1;       // negative
            aSize.Width() = -aSize.Width();     // positive
        }
        Rectangle aPageRect( aPos, aSize );

        if (aLogicRect.Right() > aPageRect.Right())
        {
            long nDiff = aLogicRect.Right() - aPageRect.Right();
            aLogicRect.Left() -= nDiff;
            aLogicRect.Right() -= nDiff;
            bChange = TRUE;
        }
        if (aLogicRect.Bottom() > aPageRect.Bottom())
        {
            long nDiff = aLogicRect.Bottom() - aPageRect.Bottom();
            aLogicRect.Top() -= nDiff;
            aLogicRect.Bottom() -= nDiff;
            bChange = TRUE;
        }

        if (aLogicRect.Left() < aPageRect.Left())
        {
            long nDiff = aLogicRect.Left() - aPageRect.Left();
            aLogicRect.Right() -= nDiff;
            aLogicRect.Left() -= nDiff;
            bChange = TRUE;
        }
        if (aLogicRect.Top() < aPageRect.Top())
        {
            long nDiff = aLogicRect.Top() - aPageRect.Top();
            aLogicRect.Bottom() -= nDiff;
            aLogicRect.Top() -= nDiff;
            bChange = TRUE;
        }
    }
    else
        DBG_ERROR("RequestObjAreaPixel: Page ist weg");

        //  wieder in Pixel umrechnen

    if (bChange)
        aRect = Rectangle( pWin->LogicToPixel( aLogicRect.TopLeft() ),
                            pWin->LogicToPixel( aLogicRect.GetSize() ) );

    //  Basisklasse

    SfxInPlaceClient::RequestObjAreaPixel( aRect );

    //  Position und Groesse ins Dokument uebernehmen

    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        //  das ist schon das skalierte Rechteck (wie im Draw-Model)

        Rectangle aOld = pDrawObj->GetLogicRect();
        if ( aLogicRect != aOld )
        {
            //  #56590# Rundungsfehler vermeiden - nur, wenn mindestens 1 Pixel Unterschied
            //  (getrennt fuer Position und Groesse)

            Size aOnePixel = pWin->PixelToLogic( Size(1,1) );
            Size aLogicSize = aLogicRect.GetSize();
            Rectangle aNew = aOld;
            Size aNewSize = aNew.GetSize();

            if ( Abs( aLogicRect.Left() - aOld.Left() ) >= aOnePixel.Width() )
                aNew.SetPos( Point( aLogicRect.Left(), aNew.Top() ) );
            if ( Abs( aLogicRect.Top() - aOld.Top() ) >= aOnePixel.Height() )
                aNew.SetPos( Point( aNew.Left(), aLogicRect.Top() ) );

            if ( Abs( aLogicSize.Width() - aNewSize.Width() ) >= aOnePixel.Width() )
                aNewSize.Width() = aLogicSize.Width();
            if ( Abs( aLogicSize.Height() - aNewSize.Height() ) >= aOnePixel.Height() )
                aNewSize.Height() = aLogicSize.Height();
            aNew.SetSize( aNewSize );

            if ( aNew != aOld )                     // veraendert nur, wenn mindestens 1 Pixel
            {
                pDrawObj->SetLogicRect( aNew );

                //  set document modified (SdrModel::SetChanged is not used)
                SfxViewShell* pSfxViewSh = GetViewShell();
                ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
                if (pViewSh)
                    pViewSh->GetViewData()->GetDocShell()->SetDrawModified();
            }
        }
    }

    //
    //  evtl. scrollen, um Objekt sichtbar zu halten
    //

    if (pDrawObj)
        pViewSh->ScrollToObject( pDrawObj );
}

void __EXPORT ScClient::ViewChanged( USHORT nAspect )
{
    SfxInPlaceClient::ViewChanged( nAspect );

    SvEmbeddedObject* pObj = GetEmbedObj();
    Rectangle aObjVisArea = OutputDevice::LogicToLogic( pObj->GetVisArea(),
                                                pObj->GetMapUnit(), MAP_100TH_MM );
    Size aVisSize = aObjVisArea.GetSize();

    //  Groesse ins Dokument uebernehmen

    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        Rectangle aLogicRect = pDrawObj->GetLogicRect();

//      SvClientData* pClientData = GetEnv();
        SvClientData* pClientData = GetClientData();
        if (pClientData)
        {
            Fraction aFractX = pClientData->GetScaleWidth();
            Fraction aFractY = pClientData->GetScaleHeight();
            aFractX *= aVisSize.Width();
            aFractY *= aVisSize.Height();
            aVisSize = Size( (long) aFractX, (long) aFractY );      // skaliert fuer Draw-Model

                //  pClientData->SetObjArea vor pDrawObj->SetLogicRect, damit keine
                //  falschen Skalierungen ausgerechnet werden:

            Rectangle aObjArea = aLogicRect;
            aObjArea.SetSize( aObjVisArea.GetSize() );          // Dokument-Groesse vom Server
            pClientData->SetObjArea( aObjArea );
        }

        //  Set size in draw model (scale is recalculated) - only if there was a visible
        //  change (in pixels)

        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if ( pViewSh )
        {
            Window* pWin = pViewSh->GetActiveWin();
            if ( pWin->LogicToPixel( aVisSize ) != pWin->LogicToPixel( aLogicRect.GetSize() ) )
            {
                aLogicRect.SetSize( aVisSize );
                pDrawObj->SetLogicRect( aLogicRect );

                //  set document modified (SdrModel::SetChanged is not used)
                pViewSh->GetViewData()->GetDocShell()->SetDrawModified();
            }
        }
    }
}

void __EXPORT ScClient::MakeViewData()
{
    SfxInPlaceClient::MakeViewData();

    SvClientData* pClientData = GetClientData();
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pClientData && pDrawObj)
    {
        //  Groesse und Scale wie in tabvwshb

        Rectangle aRect = pDrawObj->GetLogicRect();
        Size aDrawSize = aRect.GetSize();
        Size aOleSize = GetEmbedObj()->GetVisArea().GetSize();

            // sichtbarer Ausschnitt wird nur inplace veraendert!
        aRect.SetSize( aOleSize );
        pClientData->SetObjArea( aRect );

        Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
        aScaleHeight.ReduceInaccurate(10);
        pClientData->SetSizeScale(aScaleWidth,aScaleHeight);
    }
}

void __EXPORT ScClient::MakeVisible()
{
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if (pViewSh)
            pViewSh->ScrollToObject( pDrawObj );
    }
}

#if 0

//  Mit Optimierung gibt es Abstuerze beim Deaktivieren von Grafik-Image-Objekten

#ifdef WNT
#pragma optimize ( "", off )
#endif

void lcl_ReplaceObject( SdrOle2Obj* pGrafOle, SdrGrafObj* pGrafObj,
                        SfxViewShell* pSfxViewSh )
{
    //  das Ole-Objekt wieder durch das Grafikobjekt ersetzen

    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
    if (pViewSh)
    {
        SdrView* pSdrView = pViewSh->GetSdrView();
        SdrPageView* pPV = pSdrView->GetPageViewPvNum(0);

        if ( pGrafOle )
        {
            SvInPlaceObjectRef aIPObj = pGrafOle->GetObjRef();
            Rectangle          aRect  = pGrafOle->GetLogicRect();

            // #41302# ReplaceObject ohne zu markieren, weil's aus dem
            // MarkListHasChanged heraus gerufen wird
            pSdrView->ReplaceObject( pGrafOle, *pPV, pGrafObj, FALSE );
            pSdrView->EndUndo();
            pGrafObj->SetLogicRect(aRect);
            pGrafObj->SetGraphic(SimDLL::GetGraphic(aIPObj));
        }
        else                            // zwischendurch geloescht ?
        {
            DBG_ERROR("Grafik-Ole-Objekt ist weg");

            pSdrView->EndUndo();        //  damit das Undo nicht offen bleibt
                                        //! anderen Text fuer Undo setzen ???
        }
    }
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

#endif

void __EXPORT ScClient::UIActivate( BOOL bActivate )
{
    SvInPlaceClientRef aIPClient( this );   // nicht aus versehen zwischendrin loeschen

    SfxInPlaceClient::UIActivate(bActivate);

#if 0
    //! remove this along with the pGrafEdit member!

    if ( !bActivate && pGrafEdit )          // wurde eine Grafik bearbeitet?
    {
        SdrOle2Obj* pGrafOle = GetDrawObj();
        SdrGrafObj* pGrafObj = pGrafEdit;
        pGrafEdit = NULL;                   // vor lcl_ReplaceObject

        lcl_ReplaceObject( pGrafOle, pGrafObj, GetViewShell() );
    }
#endif

    if ( !bActivate )       //  Chart-Daten-Hervorhebung aufheben
    {
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if (pViewSh)
        {
            pViewSh->ClearHighlightRanges();

            //  Move an der ViewShell soll eigentlich vom Sfx gerufen werden, wenn sich
            //  das Frame-Window wegen unterschiedlicher Toolboxen o.ae. verschiebt
            //  (um nicht aus Versehen z.B. Zeichenobjekte zu verschieben, #56515#).
            //  Dieser Mechanismus funktioniert aber momentan nicht, darum hier der Aufruf
            //  per Hand (im Move wird verglichen, ob die Position wirklich geaendert ist).
            pViewSh->ForceMove();
        }
    }
}


