/*************************************************************************
 *
 *  $RCSfile: dockwin.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:30 $
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

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifdef VCL
#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#endif
#pragma hdrstop

#include "dockwin.hxx"
#include "bindings.hxx"
#include "viewfrm.hxx"
#include "dispatch.hxx"
#include "workwin.hxx"
#include "splitwin.hxx"
#include "viewsh.hxx"
#include "accmgr.hxx"

#define MAX_TOGGLEAREA_WIDTH        100  // max. 100 Pixel
#define MAX_TOGGLEAREA_HEIGHT       100  // max. 100 Pixel

class SfxDockingWindow_Impl
{
friend class SfxDockingWindow;

    SfxChildAlignment   eLastAlignment;
    SfxChildAlignment   eDockAlignment;
    BOOL                bConstructed;
    Size                aMinSize;
    SfxSplitWindow*     pSplitWin;
    BOOL                bSplitable;
//  BOOL                bAutoHide;

    // Folgende members sind nur in der Zeit von StartDocking bis EndDocking
    // g"ultig:
    BOOL                bEndDocked;
    Size                aSplitSize;
    USHORT              nLine;
    USHORT              nPos;
    USHORT              nDockLine;
    USHORT              nDockPos;
    BOOL                bNewLine;

    SfxChildAlignment   GetLastAlignment() const
                        { return eLastAlignment; }
    void                SetLastAlignment(SfxChildAlignment eAlign)
                        { eLastAlignment = eAlign; }
    SfxChildAlignment   GetDockAlignment() const
                        { return eDockAlignment; }
    void                SetDockAlignment(SfxChildAlignment eAlign)
                        { eDockAlignment = eAlign; }
};

//-------------------------------------------------------------------------

void SfxDockingWindow::Resize()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow merkt sich ggf. eine
    ver"anderte FloatingSize.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    auch SfxDockingWindow::Resize() gerufen werden.
*/
{
    DockingWindow::Resize();

    SfxSplitWindow *pSplit = pImp->pSplitWin;
    if ( pSplit )
    {
        // Die ItemSize des DockingWindows ist immer prozentual, kann sich
        // also nur "andern, wenn gesplittet wird (->SfxSplitWindow::Split())
/*
        long nWinSize = pSplit->GetItemSize( GetType() );
        if ( pSplit->IsHorizontal() )
            pImp->aSplitSize.Width()  = nWinSize;
        else
            pImp->aSplitSize.Height() = nWinSize;
*/
/*
        // Die ItemSize des ItemSets ist immer absolut in Pixeln
        if ( pSplit->IsHorizontal() )
            pImp->aSplitSize.Height() = GetOutputSizePixel().Height();
        else
            pImp->aSplitSize.Width()  = GetOutputSizePixel().Width();
*/
    }
    else
    {
        Invalidate();
        if ( IsFloatingMode() && !GetFloatingWindow()->IsRollUp() )
            SetFloatingSize(GetOutputSizePixel());
    }
}

//-------------------------------------------------------------------------

BOOL SfxDockingWindow::PrepareToggleFloatingMode()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow erm"oglicht ein Eingreifen
    in das Umschalten des floating mode.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    danach SfxDockingWindow::PrepareToggleFloatingMode() gerufen werden,
    wenn nicht FALSE zur"uckgegeben wird.
*/

{
    if (!pImp->bConstructed)
        return TRUE;

    if ( Application::IsInModalMode() && IsFloatingMode() || !pMgr )
        return FALSE;

    if (!IsFloatingMode())
    {
        // Testen, ob FloatingMode erlaubt ist
        if (CheckAlignment(GetAlignment(),SFX_ALIGN_NOALIGNMENT) !=
            SFX_ALIGN_NOALIGNMENT)
            return FALSE;

        if ( pImp->pSplitWin )
        {
            // Das DockingWindow sitzt in einem SplitWindow und wird abgerissen
            pImp->pSplitWin->RemoveWindow(this, FALSE);
            pImp->pSplitWin = 0;
        }
    }
    else if ( pMgr )
    {
        // Testen, ob es erlaubt ist, anzudocken
        if (CheckAlignment(GetAlignment(),pImp->GetLastAlignment()) ==
            SFX_ALIGN_NOALIGNMENT)
            return FALSE;

        // Testen, ob das Workwindow gerade ein Andocken erlaubt
        SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
        if ( !pWorkWin->IsDockingAllowed() )
            return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::ToggleFloatingMode()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow setzt die internen
    Daten des SfxDockingWindow und sorgt f"ur korrektes Alignment am
    parent window.
    Durch PrepareToggleFloatMode und Initialize ist sichergestellt, da\s
    pImp->GetLastAlignment() immer eine erlaubtes Alignment liefert.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    zuerst SfxDockingWindow::ToggleFloatingMode() gerufen werden.
*/
{
    if ( !pImp->bConstructed || !pMgr )
        return;                 // Kein Handler-Aufruf

    // Altes Alignment merken und dann umschalten.
    // Sv hat jetzt schon umgeschaltet, aber Alignment am SfxDockingWindow
    // ist noch das alte!
    // Was war ich bisher ?
    SfxChildAlignment eLastAlign = GetAlignment();

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;

    if (IsFloatingMode())
    {
        SetAlignment(SFX_ALIGN_NOALIGNMENT);
        SetOutputSizePixel( GetFloatingSize() );
        if ( pImp->bSplitable && !pImp->bEndDocked )
            // Wenn das Fenster vorher in einem SplitWindow lag, kommt von
            // Sv kein Show
            Show();
    }
    else
    {
        if (pImp->GetDockAlignment() == eLastAlign)
        {
            // Wenn ToggleFloatingMode aufgerufen wurde, das DockAlignment
            // aber noch unver"andert ist, mu\s das ein Toggeln durch DClick
            // gewesen sein, also LastAlignment verwenden
            SetAlignment (pImp->GetLastAlignment());
            if ( pImp->bSplitable )
            {
//              if ( GetFloatingSize().Height() < pImp->aSplitSize.Height() )
//                  pImp->aSplitSize.Height() = GetFloatingSize().Height();
//              if ( GetFloatingSize().Width() < pImp->aSplitSize.Width() )
//                  pImp->aSplitSize.Width() = GetFloatingSize().Width();
            }
            else
                SetSizePixel( CalcDockingSize(GetAlignment()) );
        }
        else
        {
            // Toggeln wurde durch Draggen ausgel"ost
            pImp->nLine = pImp->nDockLine;
            pImp->nPos = pImp->nDockPos;
            SetAlignment (pImp->GetDockAlignment());
        }

        if ( pImp->bSplitable )
        {
            // Das DockingWindow kommt jetzt in ein SplitWindow
            pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(GetAlignment());

            // Das LastAlignment ist jetzt immer noch das zuletzt angedockte
            SfxSplitWindow *pSplit =
                pWorkWin->GetSplitWindow_Impl(pImp->GetLastAlignment());
            DBG_ASSERT(pSplit, "LastAlignment kann nicht stimmen!");
            if ( pSplit && pSplit != pImp->pSplitWin )
                pSplit->ReleaseWindow_Impl(this);
            if ( pImp->GetDockAlignment() == eLastAlign )
                pImp->pSplitWin->InsertWindow(this, pImp->aSplitSize);
            else
                pImp->pSplitWin->InsertWindow(this, pImp->aSplitSize,
                      pImp->nLine,pImp->nPos,pImp->bNewLine);
            if ( !pImp->pSplitWin->IsFadeIn() )
                pImp->pSplitWin->FadeIn();
        }
    }

    // altes Alignment festhalten f"ur n"achstes Togglen; erst jetzt setzen
    // wg. Abmelden beim SplitWindow!
    pImp->SetLastAlignment(eLastAlign);

    // DockAlignment zur"ucksetzen, falls noch EndDocking gerufen wird
    pImp->SetDockAlignment(GetAlignment());

    // SfxChildWindow korrekt andocken bzw. entdocken
    if ( pMgr )
        pWorkWin->ConfigChild_Impl( eIdent, SFX_TOGGLEFLOATMODE, pMgr->GetType() );
}

//-------------------------------------------------------------------------

void SfxDockingWindow::StartDocking()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow holt vom parent window
    das innere und "au\sere docking rectangle.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    am Ende SfxDockingWindow::StartDocking() gerufen werden.
*/
{
    if ( !pImp->bConstructed || !pMgr )
        return;
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    pWorkWin->ConfigChild_Impl( eIdent, SFX_SETDOCKINGRECTS, pMgr->GetType() );
    pImp->SetDockAlignment(GetAlignment());

    if ( pImp->pSplitWin )
    {
        // Die aktuellen Docking-Daten besorgen
        pImp->pSplitWin->GetWindowPos(this, pImp->nLine, pImp->nPos);
        pImp->nDockLine = pImp->nLine;
        pImp->nDockPos = pImp->nPos;
        pImp->bNewLine = FALSE;
    }
}

//-------------------------------------------------------------------------

BOOL SfxDockingWindow::Docking( const Point& rPos, Rectangle& rRect )

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow berechnet das aktuelle
    tracking rectangle. Dazu benutzt sie die Methode CalcAlignment(rPos,rRect),
    deren Verhalten von abgeleiteten Klassen beeinflu\st werden kann (s.u.).
    Diese Methode sollte nach M"oglichkeit nicht "uberschrieben werden.
*/
{
    if ( Application::IsInModalMode() )
        return TRUE;

    if ( !pImp->bConstructed || !pMgr )
    {
        rRect.SetSize( Size() );
        return IsFloatingMode();
    }

    BOOL bFloatMode = FALSE;
    if ( GetOuterRect().IsInside( rPos ) && !IsDockingPrevented() )
    {
        // Maus innerhalb OuterRect : Alignment und Rectangle berechnen
        SfxChildAlignment eAlign = CalcAlignment(rPos, rRect);
        if (eAlign == SFX_ALIGN_NOALIGNMENT)
            bFloatMode = TRUE;
        pImp->SetDockAlignment(eAlign);
    }
    else
    {
        // Maus nicht innerhalb OuterRect : muss FloatingWindow sein
        // Ist das erlaubt ?
        if (CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT) !=
            SFX_ALIGN_NOALIGNMENT)
            return FALSE;
        bFloatMode = TRUE;
        if ( SFX_ALIGN_NOALIGNMENT != pImp->GetDockAlignment() )
        {
            // wg. SV-Bug darf rRect nur ver"andert werden, wenn sich das
            // Alignment "andert !
            pImp->SetDockAlignment(SFX_ALIGN_NOALIGNMENT);
            rRect.SetSize(CalcDockingSize(SFX_ALIGN_NOALIGNMENT));
        }
    }

    if ( SFX_ALIGN_NOALIGNMENT == pImp->GetDockAlignment() ||
                                                (pImp->bSplitable) )
    {
        // Falls durch ein ver"andertes tracking rectangle die Mausposition
        // nicht mehr darin liegt, mu\s die Position des rectangles noch
        // angepa\st werden

        Point aMouseOffset;
        aMouseOffset.X() = rRect.Left() - rPos.X();
        aMouseOffset.Y() = rRect.Top()  - rPos.Y();

        if ( (rPos.X() < rRect.Left()) || (rPos.X() > rRect.Right()) )
        {
            rRect.SetPos( rPos );
            rRect.Move( -5, aMouseOffset.Y() );
        }

        if ( (rPos.Y() < rRect.Top()) || (rPos.Y() > rRect.Bottom()) )
        {
            rRect.SetPos( rPos );
            rRect.Move( aMouseOffset.X(), -5 );
        }
    }
    else if ( !pImp->bSplitable )
    {
        // Bei individuell angedocktem Window wird die Position durch das
        // Alignment und die docking rects festgelegt.
        Size aSize = rRect.GetSize();
        Point aPos;

        switch ( pImp->GetDockAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                aPos = aInnerRect.TopLeft();
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.X() -= aSize.Width();
                break;

            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                aPos = Point(aOuterRect.Left(), aInnerRect.Top());
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.Y() -= aSize.Height();
                break;

            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                aPos = Point(aInnerRect.Right() - rRect.GetSize().Width(),
                            aInnerRect.Top());
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.X() += aSize.Width();
                break;

            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
                aPos = Point(aOuterRect.Left(),
                        aInnerRect.Bottom() - rRect.GetSize().Height());
                if ( pImp->GetDockAlignment() == GetAlignment() )
                    aPos.Y() += aSize.Height();
                break;
        }

        rRect.SetPos(aPos);
    }

    return bFloatMode;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::EndDocking( const Rectangle& rRect, BOOL bFloatMode )

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse DockingWindow sorgt f"ur das korrekte
    Alignment am parent window.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    zuerst SfxDockingWindow::EndDocking() gerufen werden.
*/
{
    if ( !pImp->bConstructed || IsDockingCanceled() || !pMgr )
        return;

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    BOOL bReArrange = FALSE;
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
    {
        eIdent = SFX_CHILDWIN_SPLITWINDOW;

        // Wenn sich das Alignment "andert und das Fenster befindet sich
        // im angedockten Zustand in einem SplitWindow, mu\s umgemeldet werden
        // Wenn neu angedockt wird, machen PrepareToggleFloatingMode()
        // und ToggleFloatingMode() das Ummelden.
        if ( !bFloatMode )
            bReArrange = TRUE;
    }

    if ( bReArrange )
    {
        if ( GetAlignment() != pImp->GetDockAlignment() )
        {
            // Vor dem Show() mu\s das Ummelden passiert sein, daher kann nicht
            // die Basisklasse gerufen werden
            if ( IsFloatingMode() || !pImp->bSplitable )
                Show( FALSE, SHOW_NOFOCUSCHANGE );

            // Die Gr"o\se f"urs Toggeln setzen
            pImp->aSplitSize = rRect.GetSize();
            if ( IsFloatingMode() )
            {
                SetFloatingMode( bFloatMode );
                if ( IsFloatingMode() || !pImp->bSplitable )
                Show( TRUE, SHOW_NOFOCUSCHANGE );
            }
            else
            {
                pImp->pSplitWin->RemoveWindow(this,FALSE);
                pImp->nLine = pImp->nDockLine;
                pImp->nPos = pImp->nDockPos;
                pImp->pSplitWin->ReleaseWindow_Impl(this);
                pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(pImp->GetDockAlignment());
                pImp->pSplitWin->InsertWindow(this, pImp->aSplitSize,
                            pImp->nDockLine, pImp->nDockPos,
                            pImp->bNewLine);
                if ( !pImp->pSplitWin->IsFadeIn() )
                    pImp->pSplitWin->FadeIn();
            }
        }
        else if ( pImp->nLine != pImp->nDockLine ||
                pImp->nPos != pImp->nDockPos || pImp->bNewLine )
        {
            // Ich wurde innerhalb meines Splitwindows verschoben.
            if ( pImp->nLine != pImp->nDockLine )
                pImp->aSplitSize = rRect.GetSize();
            pImp->pSplitWin->MoveWindow( this, pImp->aSplitSize,
                        pImp->nDockLine, pImp->nDockPos, pImp->bNewLine );
        }
    }
    else
    {
        pImp->bEndDocked = TRUE;
        DockingWindow::EndDocking(rRect, bFloatMode);
        pImp->bEndDocked = FALSE;
    }

    SetAlignment(pImp->GetDockAlignment());
    pWorkWin->ConfigChild_Impl( eIdent, SFX_ALIGNDOCKINGWINDOW, pMgr->GetType() );
}

//-------------------------------------------------------------------------

void SfxDockingWindow::Resizing( Size& rSize )

/*  [Beschreibung]

    Virtuelle Methode der Klasse DockingWindow.
    Hier kann das interaktive Umgr"o\sern im FloatingMode beeinflu\t werden,
    z.B. indem nur diskrete Werte f"ur Breite und/oder H"ohe zugelassen werden.
    Die Basisimplementation verhindert, da\s die OutputSize kleiner wird als
    eine mit SetMinOutputSizePixel() gesetzte Gr"o\se.
*/

{
    if(rSize.Width()   < pImp->aMinSize.Width())
        rSize.Width()  = pImp->aMinSize.Width();
    if(rSize.Height()  < pImp->aMinSize.Height())
        rSize.Height() = pImp->aMinSize.Height();
}

//-------------------------------------------------------------------------

SfxDockingWindow::SfxDockingWindow( SfxBindings *pBindinx, SfxChildWindow *pCW,
    Window* pParent, WinBits nWinBits) :
    pBindings(pBindinx),
    pMgr(pCW),
    DockingWindow (pParent, nWinBits)

/*  [Beschreibung]

    ctor der Klasse SfxDockingWindow. Es wird ein SfxChildWindow ben"otigt,
    da das Andocken im Sfx "uber SfxChildWindows realisiert wird.
*/

{
    ULONG nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = FALSE;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = FALSE;

    pImp->bSplitable = TRUE;
//  pImp->bAutoHide = FALSE;

    // Zun"achst auf Defaults setzen; das Alignment wird in der Subklasse gesetzt
    pImp->nLine = pImp->nDockLine = 0;
    pImp->nPos  = pImp->nDockPos = 0;
    pImp->bNewLine = FALSE;
    pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);

//  DBG_ASSERT(pMgr,"DockingWindow erfordert ein SfxChildWindow!");
}

//-------------------------------------------------------------------------

SfxDockingWindow::SfxDockingWindow( SfxBindings *pBindinx, SfxChildWindow *pCW,
    Window* pParent, const ResId& rResId) :
    pBindings(pBindinx),
    pMgr(pCW),
    DockingWindow(pParent, rResId)

/*  [Beschreibung]

    ctor der Klasse SfxDockingWindow. Es wird ein SfxChildWindow ben"otigt,
    da das Andocken im Sfx "uber SfxChildWindows realisiert wird.
*/

{
    ULONG nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = FALSE;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = FALSE;

    pImp->bSplitable = TRUE;
//  pImp->bAutoHide = FALSE;

    // Zun"achst auf Defaults setzen; das Alignment wird in der Subklasse gesetzt
    pImp->nLine = pImp->nDockLine = 0;
    pImp->nPos  = pImp->nDockPos = 0;
    pImp->bNewLine = FALSE;
    pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);

//  DBG_ASSERT(pMgr,"DockingWindow erfordert ein SfxChildWindow!");
}

//-------------------------------------------------------------------------

void SfxDockingWindow::Initialize(SfxChildWinInfo *pInfo)

/*  [Beschreibung]

    Initialisierung der Klasse SfxDockingWindow "uber ein SfxChildWinInfo.
    Die Initialisierung erfolgt erst in einem 2.Schritt nach dem ctor und sollte
    vom ctor der abgeleiteten Klasse oder vom ctor des SfxChildWindows
    aufgerufen werden.
*/

{
    if ( !pMgr )
    {
        // Bugfix #39771
        pImp->SetDockAlignment( SFX_ALIGN_NOALIGNMENT );
        pImp->bConstructed = TRUE;
        return;
    }

    BOOL bFloatMode = IsFloatingMode();
    Point aPos;
    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        // Uebergebene Size und Pos ist immer FloatingSize und FloatingPos.
        // Bei FloatingWindow ist das auch die aktuelle Size;
        Size aMinSize( GetMinOutputSizePixel() );
        if ( pInfo->aSize.Width() < aMinSize.Width() )
            pInfo->aSize.Width() = aMinSize.Width();
        if ( pInfo->aSize.Height() < aMinSize.Height() )
            pInfo->aSize.Height() = aMinSize.Height();

        if ( GetFloatStyle() & WB_SIZEABLE )
            SetFloatingSize( pInfo->aSize );
        else
            SetFloatingSize( GetSizePixel() );
        pImp->aSplitSize = GetFloatingSize();

        // Falls von FloatingWindow auf DockingWindow umgestellt wurde, gibt
        // es keinen ExtraString ( Abw"artskompatibel ) und das Alignment
        // ist SFX_ALIGN_NOALIGNMENT (im ctor zu setzen).
        if ( pInfo->aExtraString.Len() )
        {
            String aStr;
            USHORT nPos = pInfo->aExtraString.SearchAscii("AL:");

            // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
            // er nicht vorhanden ist, liegt eine "altere Version vor
            if ( nPos != STRING_NOTFOUND )
            {
                USHORT n1 = pInfo->aExtraString.Search('(', nPos);
                if ( n1 != STRING_NOTFOUND )
                {
                    USHORT n2 = pInfo->aExtraString.Search(')', n1);
                    if ( n2 != STRING_NOTFOUND )
                    {
                        // Alignment-String herausschneiden
                        aStr = pInfo->aExtraString.Copy(nPos, n2 - nPos + 1);
                        pInfo->aExtraString.Erase(nPos, n2 - nPos + 1);
                        aStr.Erase(nPos, n1-nPos+1);
                    }
                }
            }

            if ( aStr.Len() )
            {
                // Zuerst das Alignment extrahieren
                SetAlignment( (SfxChildAlignment) (USHORT) aStr.ToInt32() );

                // Um Fehler bei Manipulationen an der INI-Datei zu vermeiden,
                // wird das Alignment validiert
                SfxChildAlignment eAlign =
                    CheckAlignment(GetAlignment(),GetAlignment());
                if ( eAlign != GetAlignment() )
                {
                    DBG_ERROR("Unsinniges Alignment!");
                    SetAlignment( eAlign );
                    aStr = String();
                }

                // Dann das LastAlignment
                nPos = aStr.Search(',');
                if ( nPos != STRING_NOTFOUND )
                {
                    aStr.Erase(0, nPos+1);
                    pImp->SetLastAlignment( (SfxChildAlignment) (USHORT) aStr.ToInt32() );
                }

                // Dann die Splitting-Informationen
                nPos = aStr.Search(',');
                if ( nPos != STRING_NOTFOUND )
                {
                    aStr.Erase(0, nPos+1);
                    SfxIniManager *pAppIniMgr = SFX_APP()->GetAppIniManager();
                    if ( pAppIniMgr->GetPosSize( aStr, aPos, pImp->aSplitSize ) )
                    {
                        pImp->nLine = pImp->nDockLine = (USHORT) aPos.X();
                        pImp->nPos  = pImp->nDockPos  = (USHORT) aPos.Y();
                    }
                }
            }
        }

        aPos = pInfo->aPos;
    }
    else
    {
        Size aFloatSize = GetFloatingSize();
        if ( !aFloatSize.Width() || !aFloatSize.Height())
        {
            // Wenn FloatingSize nicht explizit gesetzt ist, wird die aktuelle
            // Fenstergr"o\se genommen !
            SetFloatingSize(GetOutputSizePixel());
            aFloatSize = GetFloatingSize();
        }

        // Der default f"ur die SplitSize ist die FloatingSize
        pImp->aSplitSize = aFloatSize;
        Size aSize = GetParent()->GetOutputSizePixel();
        aPos = GetFloatingPos();
        if ( !aPos.X() && !aPos.Y() )
        {
            // Wenn nichts gesetzt, FloatingWindow zentrieren
#ifndef VCL
            aPos = GetParent()->GetPosPixel();
#endif
            aPos.X() += (aSize.Width() - aFloatSize.Width()) / 2;
            aPos.Y() += (aSize.Height() - aFloatSize.Height()) / 2;
        }
    }

#ifdef VCL
    Point aPoint;
    Rectangle aRect = GetDesktopRectPixel();
    Size aSize( GetFloatingSize() );

    aPoint.X() = aRect.Right() - aSize.Width();
    aPoint.Y() = aRect.Bottom() - aSize.Height();

    aPoint = OutputToScreenPixel( aPoint );

    if ( aPos.X() > aPoint.X() )
        aPos.X() = aPoint.X() ;
    if ( aPos.Y() > aPoint.Y() )
        aPos.Y() = aPoint.Y();

    if ( aPos.X() < 0 ) aPos.X() = 0;
    if ( aPos.Y() < 0 ) aPos.Y() = 0;

#endif

    SetFloatingPos( aPos );

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( GetAlignment() != SFX_ALIGN_NOALIGNMENT )
    {
        // Testen, ob das Workwindow gerade ein Andocken erlaubt
        if ( !pWorkWin->IsDockingAllowed() || ( GetFloatStyle() & WB_STANDALONE )
            && Application::IsInModalMode() )
            SetAlignment( SFX_ALIGN_NOALIGNMENT );
    }

    // ggf. FloatingMode korrekt setzen (ohne Aufruf der Handler, da
    // pImp->bConstructed noch nicht TRUE ist!)
    if ( bFloatMode != (GetAlignment() == SFX_ALIGN_NOALIGNMENT) )
        SetFloatingMode(!bFloatMode);

    if ( pInfo->nFlags & SFX_CHILDWIN_FORCEDOCK )
    {
        SetFloatingMode( FALSE );
    }


    if (IsFloatingMode())
    {
        GetFloatingWindow()->SetPosPixel(GetFloatingPos());
        GetFloatingWindow()->SetOutputSizePixel(GetFloatingSize());

        // Leider gibt es von SV kein Resize, also selber machen
        Resize();

        // Soll das FloatingWindow eingezoomt werden ?
        if (pInfo && (pInfo->nFlags & SFX_CHILDWIN_ZOOMIN))
            GetFloatingWindow()->RollUp();

        // LastAlignment "uberpr"ufen
        SfxChildAlignment eLastAlign = pImp->GetLastAlignment();

        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_LEFT);
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_RIGHT);
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_TOP);
        if ( eLastAlign == SFX_ALIGN_NOALIGNMENT)
            eLastAlign = CheckAlignment(eLastAlign, SFX_ALIGN_BOTTOM);
        pImp->SetLastAlignment(eLastAlign);
    }
    else
    {
        pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);

        // Angedockte Fenster werden, sofern sie resizable sind, in ein
        // SplitWindow eingesetzt
        if ( pImp->bSplitable && !(pInfo->nFlags & SFX_CHILDWIN_FORCEDOCK) )
        {
//          pImp->bAutoHide = ( pInfo->nFlags & SFX_CHILDWIN_AUTOHIDE) != 0;
            SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
            pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(GetAlignment());
            pImp->pSplitWin->InsertWindow(this, pImp->aSplitSize);
        }
        else
        {
            // Fenster ist individuell angedockt; Gr"o\se berechnen.
            // Dazu mu\s sie mit der FloatingSize initialisiert werden, falls
            // irgendwer sich darauf verl"a\st, da\s eine vern"unftige Gr"o\se
            // gesetz ist
            SetSizePixel(GetFloatingSize());
            SetSizePixel(CalcDockingSize(GetAlignment()));
        }
    }

    // Ab jetzt d"urfen die DockingHandler aufgerufen werden
    pImp->SetDockAlignment( GetAlignment() );
    pImp->bConstructed = TRUE;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Beschreibung]

    F"ullt ein SfxChildWinInfo mit f"ur SfxDockingWindow spezifischen Daten,
    damit sie in die INI-Datei geschrieben werden koennen.
    Es wird angenommen, da\s rInfo alle anderen evt. relevanten Daten in
    der ChildWindow-Klasse erh"alt.
    Eingetragen werden hier gemerkten Gr"o\sen, das ZoomIn-Flag und die
    f"ur das Docking relevanten Informationen.
    Wird diese Methode "uberschrieben, mu\s zuerst die Basisimplementierung
    gerufen werden.
*/

{
    if ( !pMgr )
        return;

    rInfo.aPos   = GetFloatingPos();
    rInfo.aSize  = GetFloatingSize();
    rInfo.aExtraString += DEFINE_CONST_UNICODE("AL:(");
    rInfo.aExtraString += String::CreateFromInt32((USHORT) GetAlignment());
    rInfo.aExtraString += ',';
    rInfo.aExtraString += String::CreateFromInt32 ((USHORT) pImp->GetLastAlignment());
    if ( pImp->bSplitable )
    {
        rInfo.aExtraString += ',';
        SfxIniManager *pIniMgr = SFX_INIMANAGER();
        Point aPos(pImp->nLine, pImp->nPos);
        rInfo.aExtraString += pIniMgr->GetString( aPos, pImp->aSplitSize );
    }

    rInfo.aExtraString += ')';
    if (IsFloatingMode() )
    {
        if ( GetFloatingWindow()->IsRollUp() )
            rInfo.nFlags |= SFX_CHILDWIN_ZOOMIN;
    }
}

//-------------------------------------------------------------------------

SfxDockingWindow::~SfxDockingWindow()
{
    if ( pMgr && pImp->pSplitWin && pImp->pSplitWin->IsItemValid( GetType() ) )
        pImp->pSplitWin->RemoveWindow(this);
    delete pImp;
}

//-------------------------------------------------------------------------

SfxChildAlignment SfxDockingWindow::CalcAlignment(const Point& rPos, Rectangle& rRect)

/*  [Beschreibung]

    Diese Methode berechnet f"ur gegebene Mausposition und tracking rectangle,
    welches Alignment sich daraus ergeben w"urde. Beim Wechsel des Alignments
    kann sich auch das tracking rectangle "andern, so dass ein ver"andertes
    rectangle zur"uckgegeben wird.

    Der Klassenbenutzer kann das Verhalten dieser Methode und damit das Verhalten
    seiner DockinWindow-Klasse beim Docken beeinflussen, indem er die hier
    aufgerufene virtuelle Methode

        SfxDockingWindow::CalcDockingSize(SfxChildAlignment eAlign)

    "uberschreibt (s.u.).
*/

{
    // Hypothetische Gr"o\sen ausrechnen
    Size aFloatingSize(CalcDockingSize(SFX_ALIGN_NOALIGNMENT));
    Size aVerticalSize(CalcDockingSize(SFX_ALIGN_LEFT));
    Size aHorizontalSize(CalcDockingSize(SFX_ALIGN_TOP));

    // Testen, ob das Workwindow gerade ein Andocken erlaubt
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( !pWorkWin->IsDockingAllowed() )
    {
//      rRect.SetSize( aFloatingSize );
        return pImp->GetDockAlignment();
    }

    long nLeft, nRight, nTop, nBottom;
    if ( pImp->bSplitable )
    {
        Size aSize = pImp->aSplitSize;
        if ( GetFloatingSize().Height() < aSize.Height() )
            aSize.Height() = GetFloatingSize().Height();
        if ( GetFloatingSize().Width() < aSize.Width() )
            aSize.Width() = GetFloatingSize().Width();

        // die Gr"o\se des Umschaltbereiches sollte begrenzt sein
        if ( aSize.Width() > MAX_TOGGLEAREA_WIDTH )
            aSize.Width() = MAX_TOGGLEAREA_WIDTH;
        if ( aSize.Height() > MAX_TOGGLEAREA_WIDTH )
            aSize.Height() = MAX_TOGGLEAREA_WIDTH;

        nLeft = nRight = aSize.Width();
        nTop = nBottom = aSize.Height();
    }
    else
    {
        nLeft = nRight = aVerticalSize.Width();
        nTop = nBottom = aHorizontalSize.Height();
    }

    Rectangle aInRect = GetInnerRect();

    // Inneres Rechteck etwas verkleinern, wenn m"oglich, au\ser wenn das
    // Fenster an der Kante steht, wo es angedockt ist
    if ( aInRect.GetWidth() > nLeft )
        aInRect.Left()   += nLeft/2;
    if ( aInRect.GetWidth() > nRight )
        aInRect.Right()  -= nRight/2;
    if ( aInRect.GetHeight() > nTop )
        aInRect.Top()    += nTop/2;
    if ( aInRect.GetHeight() > nBottom )
        aInRect.Bottom() -= nBottom/2;

    if ( !pImp->pSplitWin ||
        pImp->nLine == pImp->pSplitWin->GetLineCount()-1 &&
        pImp->pSplitWin->GetWindowCount(pImp->nLine) == 1)
    {
        // Beim Docken mit SplitWindows darf auch au\serhalb angedockt werden,
        // um eine neue Zeile aufzumachen, es sei denn, das aktuelle Fenster
        // ist das einzige in der letzten Zeile
        switch ( GetAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                aInRect.Left() -= nLeft/2;
                break;
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                aInRect.Top() -= nTop/2;
                break;
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                aInRect.Right() += nRight/2;
                break;
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                aInRect.Bottom() += nBottom/2;
                break;
        }
    }

    // Nun das neue Alignment berechnen
    SfxChildAlignment eDockAlign = pImp->GetDockAlignment();

    if ( aInRect.IsInside( rPos ) )
    {
        // Maus im inneren Rechteck: wird/bleibt FloatingMode
        eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT);
    }
    else
    {
        // Wir befinden uns im Dock-Bereich. Jetzt m"u\sen wir feststellen,
        // an welcher Kante angedockt werden soll

        Point aInPos( rPos.X()-aInRect.Left(), rPos.Y()-aInRect.Top() );
        Size  aInSize = aInRect.GetSize();
        BOOL  bNoChange = FALSE;

        // Zuerst feststellen, ob das Alignment unver"andert bleiben kann
        switch ( GetAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                if (aInPos.X() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                if ( aInPos.Y() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                if ( aInPos.X() >= aInSize.Width())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                if ( aInPos.Y() >= aInSize.Height())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            default:
                break;
        }

        if ( !bNoChange )
        {
            // Wenn dem nicht so ist, links/oben/rechts/unten in dieser Reihenfolge
            // testen
            BOOL bForbidden = TRUE;
            if ( aInPos.X() <= 0)
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_LEFT);
                bForbidden = ( eDockAlign != SFX_ALIGN_LEFT &&
                               eDockAlign != SFX_ALIGN_FIRSTLEFT &&
                               eDockAlign != SFX_ALIGN_LASTLEFT );
            }

            if ( bForbidden && aInPos.Y() <= 0)
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_TOP);
                bForbidden = ( eDockAlign != SFX_ALIGN_TOP &&
                               eDockAlign != SFX_ALIGN_HIGHESTTOP &&
                               eDockAlign != SFX_ALIGN_LOWESTTOP );
            }

            if ( bForbidden && aInPos.X() >= aInSize.Width())
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_RIGHT);
                bForbidden = ( eDockAlign != SFX_ALIGN_RIGHT &&
                               eDockAlign != SFX_ALIGN_FIRSTRIGHT &&
                               eDockAlign != SFX_ALIGN_LASTRIGHT );
            }

            if ( bForbidden && aInPos.Y() >= aInSize.Height())
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_BOTTOM);
                bForbidden = ( eDockAlign != SFX_ALIGN_BOTTOM &&
                               eDockAlign != SFX_ALIGN_HIGHESTBOTTOM &&
                               eDockAlign != SFX_ALIGN_LOWESTBOTTOM );
            }

            // Wenn wir zwar im Dockbereich sind, da\s einzig m"ogliche Alignment
            // aber verboten ist, mu\s in den FloatingMode geschaltet werden
            if ( bForbidden )
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT);
            }
        }
    }

    if ( eDockAlign == SFX_ALIGN_NOALIGNMENT )
    {
        //Im FloatingMode erh"alt das tracking rectangle die floating size
        // wg. SV-Bug darf rRect nur ver"andert werden, wenn sich das
        // Alignment "andert !
        if ( eDockAlign != pImp->GetDockAlignment() )
            rRect.SetSize( aFloatingSize );
    }
    else if ( pImp->bSplitable )
    {
        // Bei DockingWindows in SplitWindows sind Position und Gr"o\se zu
        // berechnen, auch wenn sich das Alignment nicht ge"andert hat, da
        // sich die Zeile ge"andert haben k"onnte.
        CalcSplitPosition(rPos, rRect, eDockAlign);
    }
    else
    {
        // Bei individuell angedockten Fenstern mu\s das tracking rectangle
        // nur ge"andert werden, wenn sich das Dock-Alignment ge"andert hat.
        if ( eDockAlign != pImp->GetDockAlignment() )
        {
            switch ( eDockAlign )
            {
                case SFX_ALIGN_LEFT:
                case SFX_ALIGN_RIGHT:
                case SFX_ALIGN_FIRSTLEFT:
                case SFX_ALIGN_LASTLEFT:
                case SFX_ALIGN_FIRSTRIGHT:
                case SFX_ALIGN_LASTRIGHT:
                    rRect.SetSize( aVerticalSize );
                    break;

                case SFX_ALIGN_TOP:
                case SFX_ALIGN_BOTTOM:
                case SFX_ALIGN_LOWESTTOP:
                case SFX_ALIGN_HIGHESTTOP:
                case SFX_ALIGN_LOWESTBOTTOM:
                case SFX_ALIGN_HIGHESTBOTTOM:
                    rRect.SetSize( aHorizontalSize );
                    break;
            }
        }
    }

    return eDockAlign;
}

//-------------------------------------------------------------------------

Size SfxDockingWindow::CalcDockingSize(SfxChildAlignment eAlign)

/*  [Beschreibung]

    Virtuelle Methode der Klasse SfxDockingWindow.
    Hier wird festgelegt, wie sich die Gr"o\se des DockingWindows abh"angig vom
    Alignment "andert.
    Die Basisimplementation setzt im Floating Mode die Gr"o\se auf die gemerkte
    Floating Size.
    Bei horizontalem Alignment wird die Breite auf die Breite des "au\seren
    DockingRects, bei vertikalem Alignment die H"ohe auf die H"ohe des inneren
    DockingRects (ergibt sich aus der Reihenfolge, in der im SFX ChildWindows
    ausgegeben werden). Die jeweils andere Gr"o\se wird auf die aktuelle
    Floating Size gesetzt, hier k"onnte eine abgeleitete Klasse "andernd
    eingreifen.
    Die DockingSize mu\s f"ur Left/Right und Top/Bottom jeweils gleich sein.
*/

{
    // Achtung: falls das Resizing auch im angedockten Zustand geht, mu\s dabei
    // auch die Floating Size angepa\st werden !?

    Size aSize = GetFloatingSize();
    switch (eAlign)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
            aSize.Width() = aOuterRect.Right() - aOuterRect.Left();
            break;
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
            aSize.Height() = aInnerRect.Bottom() - aInnerRect.Top();
            break;
        case SFX_ALIGN_NOALIGNMENT:
            break;
    }

    return aSize;
}

//-------------------------------------------------------------------------

SfxChildAlignment SfxDockingWindow::CheckAlignment(SfxChildAlignment,
    SfxChildAlignment eAlign)

/*  [Beschreibung]

    Virtuelle Methode der Klasse SfxDockingWindow.
    Hier kann eine abgeleitete Klasse bestimmte Alignments verbieten.
    Die Basisimplementation verbietet kein Alignment.
*/

{
    return eAlign;
}

//-------------------------------------------------------------------------

BOOL SfxDockingWindow::Close()

/*  [Beschreibung]

    Das Fenster wird geschlossen, indem das ChildWindow durch Ausf"uhren des
    ChildWindow-Slots zerst"ort wird.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    danach SfxDockingWindow::Close() gerufen werden, wenn nicht das Close()
    mit "return FALSE" abgebrochen wird.

*/
{
    // Execute mit Parametern, da Toggle von einigen ChildWindows ignoriert
    // werden kann
    if ( !pMgr )
        return TRUE;

    SfxBoolItem aValue( pMgr->GetType(), FALSE);
    pBindings->GetDispatcher_Impl()->Execute(
        pMgr->GetType(), SFX_CALLMODE_RECORD | SFX_CALLMODE_ASYNCHRON, &aValue, 0L );
    return TRUE;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::Paint(const Rectangle& rRect)

/*  [Beschreibung]

    Es wird im angedockten Zustand eine Begrenzungslinie an der angedockten
    Kante und ein Rahmen ausgegeben. Dabei wird SVLOOK ber"ucksichtigt.
*/

{
    if ( pImp->bSplitable || IsFloatingMode() )
        return;

    Rectangle aRect = Rectangle(Point(0, 0),
                                GetOutputSizePixel());
    switch (GetAlignment())
    {
        case SFX_ALIGN_TOP:
        {
            DrawLine(aRect.BottomLeft(), aRect.BottomRight());
            aRect.Bottom()--;
            break;
        }

        case SFX_ALIGN_BOTTOM:
        {
            DrawLine(aRect.TopLeft(), aRect.TopRight());
            aRect.Top()++;
            break;
        }

        case SFX_ALIGN_LEFT:
        {
            DrawLine(aRect.TopRight(), aRect.BottomRight());
            aRect.Right()--;
            break;
        }

        case SFX_ALIGN_RIGHT:
        {
            DrawLine(aRect.TopLeft(), aRect.BottomLeft());
            aRect.Left()++;
            break;
        }
    }

    DecorationView aView( this );
    aView.DrawFrame( aRect, FRAME_DRAW_OUT );
}

//-------------------------------------------------------------------------

void SfxDockingWindow::SetMinOutputSizePixel( const Size& rSize )

/*  [Beschreibung]

    Mit dieser Methode kann eine minimale OutpuSize gesetzt werden, die
    im Resizing()-Handler abgefragt wird.
*/

{
    pImp->aMinSize = rSize;
}

//-------------------------------------------------------------------------

Size SfxDockingWindow::GetMinOutputSizePixel() const

/*  [Beschreibung]

    Die gesetzte minimale Gr"o\se wird zur"uckgegeben.
*/

{
    return pImp->aMinSize;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::CalcSplitPosition(const Point rPos, Rectangle& rRect,
                            SfxChildAlignment eAlign)

/*  [Beschreibung]

    Diese Methode berechnet f"ur gegebene Mausposition und aktuelles tracking
    rectangle, welches tats"achliche tracking rectangle sich bei einem im
    SplitWindow angedockten Fenster ergibt (unter Ber"ucksichtigung von Zeile
    und Spalte des SplitWindows, an der sich die Maus befindet).
*/

{
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    SfxSplitWindow *pSplitWin = pWorkWin->GetSplitWindow_Impl(eAlign);
    USHORT nLine, nPos;

    // Mausposition in Koordinaten des Splitwindows
    Point aPos(pSplitWin->ScreenToOutputPixel(rPos));
    if ( pSplitWin->GetWindowPos(aPos, nLine, nPos ) )
    {
        // Maus innerhalb des Splitwindows
        pImp->nDockLine = nLine;
        pImp->nDockPos = nPos;
    }
    else
    {
        // Maus au\serhalb, aber au\serhalb des InnerRects (sonst w"are diese
        // Methode gar nicht gerufen worden ), also soll eine neue Zeile
        // aufgemacht werden
        pImp->nDockLine = pSplitWin->GetLineCount();
        pImp->nDockPos = 0;
    }

    // Neue Zeile aufmachen ?
    pImp->bNewLine = (pImp->nDockLine >= pSplitWin->GetLineCount());

    // Tracking rectangle auf gemerkte Splitsize setzen
    Size aSize = pImp->aSplitSize;

    // Gr"o\se der noch freien client area
    Size aInnerSize = GetInnerRect().GetSize();

    // Wenn neue Zeile, je nach Alignment Breite oder H"ohe auf die der
    // freien client area setzen, sonst auf die LineSize der Zeile im
    // Splitwindow, "uber der die Maus steht
    if ( eAlign == SFX_ALIGN_TOP || eAlign == SFX_ALIGN_BOTTOM )
    {
        if ( pImp->bNewLine )
        {
            aSize.Width()  = aInnerSize.Width();
            if ( aSize.Height() > aInnerSize.Height() / 2 &&
                GetFloatingSize().Height() < aSize.Height() )
                aSize.Height() = GetFloatingSize().Height();
        }
        else
        {
            aSize.Height() = pSplitWin->GetLineSize(nLine);
            if ( aSize.Width() > aInnerSize.Width() / 2 &&
                GetFloatingSize().Width() < aSize.Width() )
                aSize.Width() = GetFloatingSize().Width();
        }
    }
    else
    {
        if ( pImp->bNewLine )
        {
            aSize.Height() = aInnerSize.Height();
            if ( aSize.Width() > aInnerSize.Width() / 2 &&
                GetFloatingSize().Width() < aSize.Width() )
                aSize.Width() = GetFloatingSize().Width();
        }
        else
        {
            aSize.Width()  = pSplitWin->GetLineSize(nLine);
            if ( aSize.Height() > aInnerSize.Height() / 2 &&
                GetFloatingSize().Height() < aSize.Height() )
                aSize.Height() = GetFloatingSize().Height();
        }
    }

    rRect.SetSize(aSize);
}

//-------------------------------------------------------------------------

void SfxDockingWindow::EnableSplitting(BOOL bEnable)
{
    pImp->bSplitable = bEnable;
}

// -----------------------------------------------------------------------

long SfxDockingWindow::Notify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_GETFOCUS || rEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        pBindings->SetActiveFrame( pMgr->GetFrame() );

        if ( pImp->pSplitWin )
            pImp->pSplitWin->SetActiveWindow_Impl( this );
        else
            pMgr->Activate_Impl();
/*
        // Nur wg. PlugIn
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        if ( pFrame )
            pFrame->MakeActive_Impl();
*/
        // In VCL geht Notify zun"achst an das Fenster selbst,
        // also base class rufen, sonst erf"ahrt der parent nichts
        if ( rEvt.GetType() == EVENT_MOUSEBUTTONDOWN || rEvt.GetWindow() == this )
            DockingWindow::Notify( rEvt );
        return TRUE;
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // KeyInput zuerst f"ur Dialogfunktionen zulassen
        if ( !DockingWindow::Notify( rEvt ) )
            // dann auch global g"ultige Acceleratoren verwenden
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return TRUE;
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
    {
//        pBindings->SetActiveFrame( XFrameRef() );
        pMgr->Deactivate_Impl();
    }

    return DockingWindow::Notify( rEvt );
}


USHORT SfxDockingWindow::GetWinBits_Impl() const
{
    USHORT nBits = 0;
//  if ( pImp->bAutoHide )
//      nBits |= SWIB_AUTOHIDE;
    return nBits;
}

//-------------------------------------------------------------------------

void SfxDockingWindow::SetItemSize_Impl( const Size& rSize )
{
    pImp->aSplitSize = rSize;
}

void SfxDockingWindow::Disappear_Impl()
{
    if ( pImp->pSplitWin && pImp->pSplitWin->IsItemValid( GetType() ) )
        pImp->pSplitWin->RemoveWindow(this);
}

void SfxDockingWindow::Reappear_Impl()
{
    if ( pImp->pSplitWin && !pImp->pSplitWin->IsItemValid( GetType() ) )
    {
        pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize );
    }
}

BOOL SfxDockingWindow::IsAutoHide_Impl() const
{
    if ( pImp->pSplitWin )
        return !pImp->pSplitWin->IsFadeIn();
    else
        return FALSE;
}

BOOL SfxDockingWindow::IsPinned_Impl() const
{
    if ( pImp->pSplitWin )
        return pImp->pSplitWin->IsPinned();
    else
        return TRUE;
}

void SfxDockingWindow::AutoShow_Impl( BOOL bShow )
{
    if ( pImp->pSplitWin )
    {
        if ( bShow )
            pImp->pSplitWin->FadeIn();
        else
            pImp->pSplitWin->FadeOut();
    }
}

void SfxDockingWindow::Pin_Impl( BOOL bPinned )
{
    if ( pImp->pSplitWin )
        pImp->pSplitWin->Pin_Impl( bPinned );
}

SfxSplitWindow* SfxDockingWindow::GetSplitWindow_Impl() const
{
    return pImp->pSplitWin;
}

void SfxDockingWindow::FadeIn( BOOL bFadeIn )
{
}


