/*************************************************************************
 *
 *  $RCSfile: dockwin.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:27:58 $
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
#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
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
#include "sfxhelp.hxx"

#define MAX_TOGGLEAREA_WIDTH        20
#define MAX_TOGGLEAREA_HEIGHT       20

// implemented in 'sfx2/source/appl/childwin.cxx'
extern sal_Bool GetPosSizeFromString( const String& rStr, Point& rPos, Size& rSize );

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
    long                nHorizontalSize;
    long                nVerticalSize;
    USHORT              nLine;
    USHORT              nPos;
    USHORT              nDockLine;
    USHORT              nDockPos;
    BOOL                bNewLine;
    BOOL                bDockingPrevented;
    ByteString          aWinState;

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
    Invalidate();
    if ( pImp->bConstructed && pMgr )
    {
        if ( IsFloatingMode() )
        {
            if( !GetFloatingWindow()->IsRollUp() )
                SetFloatingSize( GetOutputSizePixel() );
            pImp->aWinState = GetFloatingWindow()->GetWindowState();
            SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
            SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
            if ( pImp->bSplitable )
                eIdent = SFX_CHILDWIN_SPLITWINDOW;
            pWorkWin->ConfigChild_Impl( eIdent, SFX_ALIGNDOCKINGWINDOW, pMgr->GetType() );
        }
        else
        {
            Size aSize( GetSizePixel() );
            switch ( pImp->GetDockAlignment() )
            {
                case SFX_ALIGN_LEFT:
                case SFX_ALIGN_FIRSTLEFT:
                case SFX_ALIGN_LASTLEFT:
                case SFX_ALIGN_RIGHT:
                case SFX_ALIGN_FIRSTRIGHT:
                case SFX_ALIGN_LASTRIGHT:
                    pImp->nHorizontalSize = aSize.Width();
                    break;
                case SFX_ALIGN_TOP:
                case SFX_ALIGN_LOWESTTOP:
                case SFX_ALIGN_HIGHESTTOP:
                case SFX_ALIGN_BOTTOM:
                case SFX_ALIGN_HIGHESTBOTTOM:
                case SFX_ALIGN_LOWESTBOTTOM:
                    pImp->nVerticalSize = aSize.Height();
                    break;
            }
        }
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

    if ( pImp->bDockingPrevented )
        return FALSE;

    if (!IsFloatingMode())
    {
        // Testen, ob FloatingMode erlaubt ist
        if ( CheckAlignment(GetAlignment(),SFX_ALIGN_NOALIGNMENT) != SFX_ALIGN_NOALIGNMENT )
            return FALSE;

        if ( pImp->pSplitWin )
        {
            // Das DockingWindow sitzt in einem SplitWindow und wird abgerissen
            pImp->pSplitWin->RemoveWindow(this/*, FALSE*/);
            pImp->pSplitWin = 0;
        }
    }
    else if ( pMgr )
    {
        pImp->aWinState = GetFloatingWindow()->GetWindowState();

        // Testen, ob es erlaubt ist, anzudocken
        if (CheckAlignment(GetAlignment(),pImp->GetLastAlignment()) == SFX_ALIGN_NOALIGNMENT)
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
        if ( pImp->aWinState.Len() )
            GetFloatingWindow()->SetWindowState( pImp->aWinState );
        else
            GetFloatingWindow()->SetOutputSizePixel( GetFloatingSize() );
/*
        if ( pImp->bSplitable && !pImp->bEndDocked )
            // Wenn das Fenster vorher in einem SplitWindow lag, kommt von
            // Sv kein Show
            Show();
*/
    }
    else
    {
        if (pImp->GetDockAlignment() == eLastAlign)
        {
            // Wenn ToggleFloatingMode aufgerufen wurde, das DockAlignment
            // aber noch unver"andert ist, mu\s das ein Toggeln durch DClick
            // gewesen sein, also LastAlignment verwenden
            SetAlignment (pImp->GetLastAlignment());
            if ( !pImp->bSplitable )
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
            SfxSplitWindow *pSplit = pWorkWin->GetSplitWindow_Impl(pImp->GetLastAlignment());

            DBG_ASSERT( pSplit, "LastAlignment kann nicht stimmen!" );
            if ( pSplit && pSplit != pImp->pSplitWin )
                pSplit->ReleaseWindow_Impl(this);
            if ( pImp->GetDockAlignment() == eLastAlign )
                pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize );
            else
                pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize, pImp->nLine, pImp->nPos, pImp->bNewLine );
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

    if ( pImp->bDockingPrevented )
        return FALSE;

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
        if (CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT) != SFX_ALIGN_NOALIGNMENT)
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

    if ( !pImp->bSplitable )
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
                pImp->pSplitWin->InsertWindow( this, pImp->aSplitSize, pImp->nDockLine, pImp->nDockPos, pImp->bNewLine );
                if ( !pImp->pSplitWin->IsFadeIn() )
                    pImp->pSplitWin->FadeIn();
            }
        }
        else if ( pImp->nLine != pImp->nDockLine || pImp->nPos != pImp->nDockPos || pImp->bNewLine )
        {
            // Ich wurde innerhalb meines Splitwindows verschoben.
            if ( pImp->nLine != pImp->nDockLine )
                pImp->aSplitSize = rRect.GetSize();
            pImp->pSplitWin->MoveWindow( this, pImp->aSplitSize, pImp->nDockLine, pImp->nDockPos, pImp->bNewLine );
        }
    }
    else
    {
        pImp->bEndDocked = TRUE;
        DockingWindow::EndDocking(rRect, bFloatMode);
        pImp->bEndDocked = FALSE;
    }

    SetAlignment( IsFloatingMode() ? SFX_ALIGN_NOALIGNMENT : pImp->GetDockAlignment() );
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
/*
    if(rSize.Width()   < pImp->aMinSize.Width())
        rSize.Width()  = pImp->aMinSize.Width();
    if(rSize.Height()  < pImp->aMinSize.Height())
        rSize.Height() = pImp->aMinSize.Height();
*/
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
    if ( !nId && pCW )
        nId = pCW->GetType();
    SetHelpId( 0 );
    SetUniqueId( nId );

    pImp = new SfxDockingWindow_Impl;
    pImp->bConstructed = FALSE;
    pImp->pSplitWin = 0;
    pImp->bEndDocked = FALSE;
    pImp->bDockingPrevented = FALSE;

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
    pImp->bDockingPrevented = FALSE;

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

    if ( pInfo->nFlags & SFX_CHILDWIN_FORCEDOCK )
        pImp->bDockingPrevented = TRUE;

    pImp->aSplitSize = GetOutputSizePixel();
    if ( !GetFloatingSize().Width() )
    {
        Size aMinSize( GetMinOutputSizePixel() );
        SetFloatingSize( pImp->aSplitSize );
        if ( pImp->aSplitSize.Width() < aMinSize.Width() )
            pImp->aSplitSize.Width() = aMinSize.Width();
        if ( pImp->aSplitSize.Height() < aMinSize.Height() )
            pImp->aSplitSize.Height() = aMinSize.Height();
    }

    if ( pInfo->aExtraString.Len() )
    {
        // get information about alignment, split size and position in SplitWindow
        String aStr;
        USHORT nPos = pInfo->aExtraString.SearchAscii("AL:");
        if ( nPos != STRING_NOTFOUND )
        {
            // alignment information
            USHORT n1 = pInfo->aExtraString.Search('(', nPos);
            if ( n1 != STRING_NOTFOUND )
            {
                USHORT n2 = pInfo->aExtraString.Search(')', n1);
                if ( n2 != STRING_NOTFOUND )
                {
                    // extract alignment information from extrastring
                    aStr = pInfo->aExtraString.Copy(nPos, n2 - nPos + 1);
                    pInfo->aExtraString.Erase(nPos, n2 - nPos + 1);
                    aStr.Erase(nPos, n1-nPos+1);
                }
            }
        }

        if ( aStr.Len() )
        {
            // accept window state only if alignment is also set
            pImp->aWinState = pInfo->aWinState;

            // check for valid alignment
            SfxChildAlignment eLocalAlignment = (SfxChildAlignment) (USHORT) aStr.ToInt32();
            if ( pImp->bDockingPrevented )
                // docking prevented, ignore old configuration and take alignment from default
                aStr.Erase();
            else
                SetAlignment( eLocalAlignment );

            SfxChildAlignment eAlign = CheckAlignment(GetAlignment(),GetAlignment());
            if ( eAlign != GetAlignment() )
            {
                DBG_ERROR("Invalid Alignment!");
                SetAlignment( eAlign );
                aStr.Erase();
            }

            // get last alignment (for toggeling)
            nPos = aStr.Search(',');
            if ( nPos != STRING_NOTFOUND )
            {
                aStr.Erase(0, nPos+1);
                pImp->SetLastAlignment( (SfxChildAlignment) (USHORT) aStr.ToInt32() );
            }

            nPos = aStr.Search(',');
            if ( nPos != STRING_NOTFOUND )
            {
                // get split size and position in SplitWindow
                Point aPos;
                aStr.Erase(0, nPos+1);
                if ( GetPosSizeFromString( aStr, aPos, pImp->aSplitSize ) )
                {
                    pImp->nLine = pImp->nDockLine = (USHORT) aPos.X();
                    pImp->nPos  = pImp->nDockPos  = (USHORT) aPos.Y();
                }
            }
        }
        else
            DBG_ERROR( "Information is missing!" );
    }

    pImp->nVerticalSize = pImp->aSplitSize.Height();
    pImp->nHorizontalSize = pImp->aSplitSize.Width();

    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( GetAlignment() != SFX_ALIGN_NOALIGNMENT )
    {
        // check if SfxWorkWindow is able to allow docking at its border
        if ( !pWorkWin->IsDockingAllowed() || ( GetFloatStyle() & WB_STANDALONE )
            && Application::IsInModalMode() )
            SetAlignment( SFX_ALIGN_NOALIGNMENT );
    }

    // detect floating mode
    // toggeling mode will not execute code in handlers, because pImp->bConstructed is not set yet
    BOOL bFloatMode = IsFloatingMode();
    if ( bFloatMode != ((GetAlignment() == SFX_ALIGN_NOALIGNMENT)) )
    {
        bFloatMode = !bFloatMode;
        SetFloatingMode( bFloatMode );
        if ( bFloatMode )
        {
            if ( pImp->aWinState.Len() )
                GetFloatingWindow()->SetWindowState( pImp->aWinState );
            else
                GetFloatingWindow()->SetOutputSizePixel( GetFloatingSize() );
        }
    }

    if ( IsFloatingMode() )
    {
        // validate last alignment
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
        // docked window must have NOALIGNMENT as last alignment
        pImp->SetLastAlignment(SFX_ALIGN_NOALIGNMENT);

        if ( pImp->bSplitable )
        {
//          pImp->bAutoHide = ( pInfo->nFlags & SFX_CHILDWIN_AUTOHIDE) != 0;
            pImp->pSplitWin = pWorkWin->GetSplitWindow_Impl(GetAlignment());
            pImp->pSplitWin->InsertWindow(this, pImp->aSplitSize);
        }
        else
        {
            //?????? Currently not supported
            // Fenster ist individuell angedockt; Gr"o\se berechnen.
            // Dazu mu\s sie mit der FloatingSize initialisiert werden, falls
            // irgendwer sich darauf verl"a\st, da\s eine vern"unftige Gr"o\se
            // gesetzt ist
            SetSizePixel(GetFloatingSize());
            SetSizePixel(CalcDockingSize(GetAlignment()));
        }
    }

    // save alignment
    pImp->SetDockAlignment( GetAlignment() );
}

void SfxDockingWindow::Initialize_Impl()
{
    if ( !pMgr )
    {
        // Bugfix #39771
        pImp->bConstructed = TRUE;
        return;
    }

    FloatingWindow* pFloatWin = GetFloatingWindow();
    BOOL bSet = FALSE;
    if ( pFloatWin )
    {
        bSet = !pFloatWin->IsDefaultPos();
    }
    else
    {
        Point aPos = GetFloatingPos();
        if ( aPos != Point() )
            bSet = TRUE;
    }

    if ( !bSet)
    {
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        Window* pEditWin = pFrame->GetViewShell()->GetWindow();
        Point aPos = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPos = GetParent()->ScreenToOutputPixel( aPos );
        SetFloatingPos( aPos );
    }

    if ( pFloatWin )
    {
        // initialize floating window
        if ( !pImp->aWinState.Len() )
            // window state never set before, get if from defaults
            pImp->aWinState = pFloatWin->GetWindowState();

        // trick: use VCL method SetWindowState to adjust position and size
        pFloatWin->SetWindowState( pImp->aWinState );

        // remember floating size for calculating alignment and tracking rectangle
        SetFloatingSize( pFloatWin->GetSizePixel() );

        // some versions of VCL didn't call resize in the current situation
        //Resize();
    }

    // allow calling of docking handlers
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

    if ( GetFloatingWindow() && pImp->bConstructed )
        pImp->aWinState = GetFloatingWindow()->GetWindowState();

    rInfo.aWinState = pImp->aWinState;
    rInfo.aExtraString += DEFINE_CONST_UNICODE("AL:(");
    rInfo.aExtraString += String::CreateFromInt32((USHORT) GetAlignment());
    rInfo.aExtraString += ',';
    rInfo.aExtraString += String::CreateFromInt32 ((USHORT) pImp->GetLastAlignment());
    if ( pImp->bSplitable )
    {
        Point aPos(pImp->nLine, pImp->nPos);
        rInfo.aExtraString += ',';
        rInfo.aExtraString += String::CreateFromInt32( aPos.X() );
        rInfo.aExtraString += '/';
        rInfo.aExtraString += String::CreateFromInt32( aPos.Y() );
        rInfo.aExtraString += '/';
        rInfo.aExtraString += String::CreateFromInt32( pImp->nHorizontalSize );
        rInfo.aExtraString += '/';
        rInfo.aExtraString += String::CreateFromInt32( pImp->nVerticalSize );
    }

    rInfo.aExtraString += ')';
}

//-------------------------------------------------------------------------

SfxDockingWindow::~SfxDockingWindow()
{
    ReleaseChildWindow_Impl();
    delete pImp;
}

void SfxDockingWindow::ReleaseChildWindow_Impl()
{
    if ( pMgr && pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( NULL );

    if ( pMgr && pImp->pSplitWin && pImp->pSplitWin->IsItemValid( GetType() ) )
        pImp->pSplitWin->RemoveWindow(this);

    pMgr=NULL;
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
    // calculate hypothetical sizes for different modes
    Size aFloatingSize(CalcDockingSize(SFX_ALIGN_NOALIGNMENT));
    Size aVerticalSize(CalcDockingSize(SFX_ALIGN_LEFT));
    Size aHorizontalSize(CalcDockingSize(SFX_ALIGN_TOP));

    // check if docking is permitted
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    if ( !pWorkWin->IsDockingAllowed() )
    {
        rRect.SetSize( aFloatingSize );
        return pImp->GetDockAlignment();
    }

    // calculate borders to shrink inner area before checking for intersection with tracking rectangle
    long nLRBorder, nTBBorder;
    if ( pImp->bSplitable )
    {
        // take the smaller size of docked and floating mode
        Size aSize = pImp->aSplitSize;
        if ( GetFloatingSize().Height() < aSize.Height() )
            aSize.Height() = GetFloatingSize().Height();
        if ( GetFloatingSize().Width() < aSize.Width() )
            aSize.Width() = GetFloatingSize().Width();

        nLRBorder = aSize.Width();
        nTBBorder = aSize.Height();
    }
    else
    {
        nLRBorder = aVerticalSize.Width();
        nTBBorder = aHorizontalSize.Height();
    }

    // limit border to predefined constant values
    if ( nLRBorder > MAX_TOGGLEAREA_WIDTH )
        nLRBorder = MAX_TOGGLEAREA_WIDTH;
    if ( nTBBorder > MAX_TOGGLEAREA_WIDTH )
        nTBBorder = MAX_TOGGLEAREA_WIDTH;

    // shrink area for floating mode if possible
    Rectangle aInRect = GetInnerRect();
    if ( aInRect.GetWidth() > nLRBorder )
        aInRect.Left()   += nLRBorder/2;
    if ( aInRect.GetWidth() > nLRBorder )
        aInRect.Right()  -= nLRBorder/2;
    if ( aInRect.GetHeight() > nTBBorder )
        aInRect.Top()    += nTBBorder/2;
    if ( aInRect.GetHeight() > nTBBorder )
        aInRect.Bottom() -= nTBBorder/2;

    // calculate alignment resulting from docking rectangle
    BOOL bBecomesFloating = FALSE;
    SfxChildAlignment eDockAlign = pImp->GetDockAlignment();
    Rectangle aDockingRect( rRect );
    if ( !IsFloatingMode() )
    {
        // don't use tracking rectangle for alignment check, because it will be too large
        // to get a floating mode as result - switch to floating size
        // so the calculation only depends on the position of the rectangle, not the current
        // docking state of the window
        aDockingRect.SetSize( GetFloatingSize() );

        // in this mode docking is never done by keyboard, so it's OK to use the mouse position
        aDockingRect.SetPos( pWorkWin->GetWindow()->GetPointerPosPixel() );
    }

    Point aPos = aDockingRect.TopLeft();
    Rectangle aIntersect = GetOuterRect().GetIntersection( aDockingRect );
    if ( aIntersect.IsEmpty() )
        // docking rectangle completely outside docking area -> floating mode
        bBecomesFloating = TRUE;
    else
    {
        Rectangle aIntersect = aInRect.GetIntersection( aDockingRect );
        if ( aIntersect == aDockingRect )
            // docking rectangle completely inside (shrinked) inner area -> floating mode
            bBecomesFloating = TRUE;
    }

    if ( bBecomesFloating )
    {
        eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT);
    }
    else
    {
        // docking rectangle is in the "sensible area"
        Point aInPosTL( aPos.X()-aInRect.Left(), aPos.Y()-aInRect.Top() );
        Point aInPosBR( aPos.X()-aInRect.Left() + aDockingRect.GetWidth(), aPos.Y()-aInRect.Top() + aDockingRect.GetHeight() );
        Size  aInSize = aInRect.GetSize();
        BOOL  bNoChange = FALSE;

        // check if alignment is still unchanged
        switch ( GetAlignment() )
        {
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                if (aInPosTL.X() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                if ( aInPosTL.Y() <= 0)
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                if ( aInPosBR.X() >= aInSize.Width())
                {
                    eDockAlign = GetAlignment();
                    bNoChange = TRUE;
                }
                break;
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                if ( aInPosBR.Y() >= aInSize.Height())
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
            // alignment will change, test alignment according to distance of the docking rectangles edges
            BOOL bForbidden = TRUE;
            if ( aInPosTL.X() <= 0)
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_LEFT);
                bForbidden = ( eDockAlign != SFX_ALIGN_LEFT &&
                               eDockAlign != SFX_ALIGN_FIRSTLEFT &&
                               eDockAlign != SFX_ALIGN_LASTLEFT );
            }

            if ( bForbidden && aInPosTL.Y() <= 0)
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_TOP);
                bForbidden = ( eDockAlign != SFX_ALIGN_TOP &&
                               eDockAlign != SFX_ALIGN_HIGHESTTOP &&
                               eDockAlign != SFX_ALIGN_LOWESTTOP );
            }

            if ( bForbidden && aInPosBR.X() >= aInSize.Width())
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_RIGHT);
                bForbidden = ( eDockAlign != SFX_ALIGN_RIGHT &&
                               eDockAlign != SFX_ALIGN_FIRSTRIGHT &&
                               eDockAlign != SFX_ALIGN_LASTRIGHT );
            }

            if ( bForbidden && aInPosBR.Y() >= aInSize.Height())
            {
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_BOTTOM);
                bForbidden = ( eDockAlign != SFX_ALIGN_BOTTOM &&
                               eDockAlign != SFX_ALIGN_HIGHESTBOTTOM &&
                               eDockAlign != SFX_ALIGN_LOWESTBOTTOM );
            }

            // the calculated alignment was rejected by the window -> take floating mode
            if ( bForbidden )
                eDockAlign = CheckAlignment(pImp->GetDockAlignment(),SFX_ALIGN_NOALIGNMENT);
        }
    }

    if ( eDockAlign == SFX_ALIGN_NOALIGNMENT )
    {
        //Im FloatingMode erh"alt das tracking rectangle die floating size
        // wg. SV-Bug darf rRect nur ver"andert werden, wenn sich das
        // Alignment "andert !
        if ( eDockAlign != pImp->GetDockAlignment() )
            aDockingRect.SetSize( aFloatingSize );
    }
    else if ( pImp->bSplitable )
    {
        USHORT nLine, nPos;
        SfxSplitWindow *pSplitWin = pWorkWin->GetSplitWindow_Impl(eDockAlign);
        aPos = pSplitWin->ScreenToOutputPixel( aPos );
        if ( pSplitWin->GetWindowPos( aPos, nLine, nPos ) )
        {
            // mouse over splitwindow, get line and position
            pImp->nDockLine = nLine;
            pImp->nDockPos = nPos;
            pImp->bNewLine = FALSE;
        }
        else
        {
            if ( 0 )
            {
                // mouse touches outer border -> treated as floating mode
                eDockAlign = SFX_ALIGN_NOALIGNMENT;
                aDockingRect.SetSize( aFloatingSize );
                rRect = aDockingRect;
                return eDockAlign;
            }

            // mouse touches inner border -> create new line
            if ( eDockAlign == GetAlignment() && pImp->pSplitWin &&
                 pImp->nLine == pImp->pSplitWin->GetLineCount()-1 && pImp->pSplitWin->GetWindowCount(pImp->nLine) == 1 )
            {
                // if this window is the only one in the last line, it can't be docked as new line in the same splitwindow
                pImp->nDockLine = pImp->nLine;
                pImp->nDockPos = pImp->nPos;
                pImp->bNewLine = FALSE;
            }
            else
            {
                // create new line
                pImp->nDockLine = pSplitWin->GetLineCount();
                pImp->nDockPos = 0;
                pImp->bNewLine = TRUE;
            }
        }

        BOOL bChanged = pImp->nLine != pImp->nDockLine || pImp->nPos != pImp->nDockPos || eDockAlign != GetAlignment();
        if ( !bChanged && !IsFloatingMode() )
        {
            // window only sightly moved, no change of any property
            rRect.SetSize( pImp->aSplitSize );
            rRect.SetPos( aDockingRect.TopLeft() );
            return eDockAlign;
        }

        // calculate new size and position
        Size aSize;
        Point aPoint = aDockingRect.TopLeft();
        Size aInnerSize = GetInnerRect().GetSize();
        if ( eDockAlign == SFX_ALIGN_LEFT || eDockAlign == SFX_ALIGN_RIGHT )
        {
            if ( pImp->bNewLine )
            {
                // set height to height of free area
                aSize.Height() = aInnerSize.Height();
                aSize.Width() = pImp->nHorizontalSize;
                if ( eDockAlign == SFX_ALIGN_LEFT )
                {
                    aPoint = aInnerRect.TopLeft();
                }
                else
                {
                    aPoint = aInnerRect.TopRight();
                    aPoint.X() -= aSize.Width();
                }
            }
            else
            {
                // get width from splitwindow
                aSize.Width() = pSplitWin->GetLineSize(nLine);
                aSize.Height() = pImp->aSplitSize.Height();
            }
        }
        else
        {
            if ( pImp->bNewLine )
            {
                // set width to width of free area
                aSize.Width() = aInnerSize.Width();
                aSize.Height() = pImp->nVerticalSize;
                if ( eDockAlign == SFX_ALIGN_TOP )
                {
                    aPoint = aInnerRect.TopLeft();
                }
                else
                {
                    aPoint = aInnerRect.BottomLeft();
                    aPoint.Y() -= aSize.Height();
                }
            }
            else
            {
                // get height from splitwindow
                aSize.Height() = pSplitWin->GetLineSize(nLine);
                aSize.Width() = pImp->aSplitSize.Width();
            }
        }

        aDockingRect.SetSize( aSize );
        aDockingRect.SetPos( aPoint );
    }
    else
    {
        // window can be docked, but outside our splitwindows
        // tracking rectangle only needs to be modified if alignment was changed
        if ( eDockAlign != pImp->GetDockAlignment() )
        {
            switch ( eDockAlign )
            {
                case SFX_ALIGN_LEFT:
                case SFX_ALIGN_RIGHT:
                case SFX_ALIGN_FIRSTLEFT:
                    aDockingRect.SetPos( aInnerRect.TopLeft() );
                    aDockingRect.SetSize( aVerticalSize );
                    break;
                case SFX_ALIGN_LASTLEFT:
                case SFX_ALIGN_FIRSTRIGHT:
                case SFX_ALIGN_LASTRIGHT:
                {
                    Point aPt( aInnerRect.TopRight() );
                    aPt.X() -= aDockingRect.GetWidth();
                    aDockingRect.SetPos( aPt );
                    aDockingRect.SetSize( aVerticalSize );
                    break;
                }

                case SFX_ALIGN_TOP:
                case SFX_ALIGN_BOTTOM:
                case SFX_ALIGN_LOWESTTOP:
                    aDockingRect.SetPos( aInnerRect.TopLeft() );
                    aDockingRect.SetSize( aHorizontalSize );
                    break;
                case SFX_ALIGN_HIGHESTTOP:
                case SFX_ALIGN_LOWESTBOTTOM:
                case SFX_ALIGN_HIGHESTBOTTOM:
                {
                    Point aPt( aInnerRect.BottomLeft() );
                    aPt.Y() -= aDockingRect.GetHeight();
                    aDockingRect.SetPos( aPt );
                    aDockingRect.SetSize( aHorizontalSize );
                    break;
                }
            }
        }
    }

    rRect = aDockingRect;
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
    DockingWindow::SetMinOutputSizePixel( rSize );
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

long SfxDockingWindow::Notify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pMgr->GetFrame() );

        if ( pImp->pSplitWin )
            pImp->pSplitWin->SetActiveWindow_Impl( this );
        else
            pMgr->Activate_Impl();

        Window* pWindow = rEvt.GetWindow();
        ULONG nHelpId  = 0;
        while ( !nHelpId && pWindow )
        {
            nHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( nHelpId )
            SfxHelp::OpenHelpAgent( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), nHelpId );
/*
        // Nur wg. PlugIn
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        if ( pFrame )
            pFrame->MakeActive_Impl();
*/
        // In VCL geht Notify zun"achst an das Fenster selbst,
        // also base class rufen, sonst erf"ahrt der parent nichts
        if ( rEvt.GetWindow() == this )
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
    SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
    SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
    if ( pImp->bSplitable )
        eIdent = SFX_CHILDWIN_SPLITWINDOW;
    pWorkWin->ConfigChild_Impl( eIdent, SFX_ALIGNDOCKINGWINDOW, pMgr->GetType() );
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

/*
void SfxDockingWindow::Pin_Impl( BOOL bPinned )
{
    if ( pImp->pSplitWin )
        pImp->pSplitWin->Pin_Impl( bPinned );
}
*/

SfxSplitWindow* SfxDockingWindow::GetSplitWindow_Impl() const
{
    return pImp->pSplitWin;
}

void SfxDockingWindow::FadeIn( BOOL bFadeIn )
{
}

void SfxDockingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
        Initialize_Impl();

    DockingWindow::StateChanged( nStateChange );
}

void SfxDockingWindow::Move()
{
    if ( IsReallyVisible() && IsFloatingMode() )
    {
        SfxChildIdentifier eIdent = SFX_CHILDWIN_DOCKINGWINDOW;
        if ( pImp->bSplitable )
            eIdent = SFX_CHILDWIN_SPLITWINDOW;
        SfxWorkWindow *pWorkWin = pBindings->GetWorkWindow_Impl();
        pWorkWin->ConfigChild_Impl( eIdent, SFX_ALIGNDOCKINGWINDOW, pMgr->GetType() );
    }
}

