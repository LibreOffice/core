/*************************************************************************
 *
 *  $RCSfile: workwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-27 09:21:25 $
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

#pragma hdrstop

#include "workwin.hxx"
#include "topfrm.hxx"
#include "clientsh.hxx"
#include "arrdecl.hxx"
#include "viewfrm.hxx"
#include "module.hxx"
#include "dispatch.hxx"
#include "tbxmgr.hxx"
#include "tbxchild.hxx"
#include "tbxconf.hxx"
#include "imgmgr.hxx"
#include "dockwin.hxx"
#include "viewsh.hxx"
#include "ipenv.hxx"
#include "splitwin.hxx"
#include "msgpool.hxx"
#include "stbmgr.hxx"

DBG_NAME(SfxWorkWindow);

SV_IMPL_OBJARR( SfxObjectBarArr_Impl, SfxObjectBar_Impl );

//====================================================================
// Sortiert die Children nach ihrem Alignment
// Reihenfolge entspricht der im enum SfxChildAlignment (->CHILDWIN.HXX).
//

// Hilfe, um die "Anderungen am Alignment kompatibal zu machen!

BOOL IsAppWorkWinToolbox_Impl( USHORT nPos )
{
    switch ( nPos )
    {
        case SFX_OBJECTBAR_APPLICATION :
        case SFX_OBJECTBAR_MACRO:
        case SFX_OBJECTBAR_FULLSCREEN:
            return TRUE;
        default:
            return FALSE;
    }
}

USHORT TbxMatch( USHORT nPos )
{
    switch ( nPos )
    {
        case SFX_OBJECTBAR_APPLICATION :
            return 0;
            break;
        case SFX_OBJECTBAR_OPTIONS:
            return 1;
            break;
        case SFX_OBJECTBAR_MACRO:
            return 2;
            break;
        case SFX_OBJECTBAR_OBJECT:
            return 3;
            break;
        case SFX_OBJECTBAR_TOOLS:
            return 4;
            break;
        case SFX_OBJECTBAR_FULLSCREEN:
        case SFX_OBJECTBAR_COMMONTASK:
        case SFX_OBJECTBAR_RECORDING:
            return nPos+1;
            break;
        default:
            return nPos;
            break;
    }
}

USHORT ChildAlignValue(SfxChildAlignment eAlign)
{
    USHORT ret = 17;

    switch (eAlign)
    {
        case SFX_ALIGN_HIGHESTTOP:
            ret = 1;
            break;
        case SFX_ALIGN_LOWESTBOTTOM:
            ret = 2;
            break;
        case SFX_ALIGN_FIRSTLEFT:
            ret = 3;
            break;
        case SFX_ALIGN_LASTRIGHT:
            ret = 4;
            break;
        case SFX_ALIGN_LEFT:
            ret = 5;
            break;
        case SFX_ALIGN_RIGHT:
            ret = 6;
            break;
        case SFX_ALIGN_FIRSTRIGHT:
            ret = 7;
            break;
        case SFX_ALIGN_LASTLEFT:
            ret = 8;
            break;
        case SFX_ALIGN_TOP:
            ret = 9;
            break;
        case SFX_ALIGN_BOTTOM:
            ret = 10;
            break;
        case SFX_ALIGN_TOOLBOXTOP:
            ret = 11;
            break;
        case SFX_ALIGN_TOOLBOXBOTTOM:
            ret = 12;
            break;
        case SFX_ALIGN_LOWESTTOP:
            ret = 13;
            break;
        case SFX_ALIGN_HIGHESTBOTTOM:
            ret = 14;
            break;
        case SFX_ALIGN_TOOLBOXLEFT:
            ret = 15;
            break;
        case SFX_ALIGN_TOOLBOXRIGHT:
            ret = 16;
            break;
    }

    return ret;
}

USHORT ChildTravelValue( SfxChildAlignment eAlign )
{
    USHORT ret = 17;

    switch (eAlign)
    {
        case SFX_ALIGN_FIRSTLEFT:
            ret = 1;
            break;
        case SFX_ALIGN_LEFT:
            ret = 2;
            break;
        case SFX_ALIGN_LASTLEFT:
            ret = 3;
            break;
        case SFX_ALIGN_TOOLBOXLEFT:
            ret = 4;
            break;
        case SFX_ALIGN_HIGHESTTOP:
            ret = 5;
            break;
        case SFX_ALIGN_TOP:
            ret = 6;
            break;
        case SFX_ALIGN_TOOLBOXTOP:
            ret = 7;
            break;
        case SFX_ALIGN_LOWESTTOP:
            ret = 8;
            break;
        case SFX_ALIGN_HIGHESTBOTTOM:
            ret = 9;
            break;
        case SFX_ALIGN_TOOLBOXBOTTOM:
            ret = 10;
            break;
        case SFX_ALIGN_BOTTOM:
            ret = 11;
            break;
        case SFX_ALIGN_LOWESTBOTTOM:
            ret = 12;
            break;
        case SFX_ALIGN_TOOLBOXRIGHT:
            ret = 13;
            break;
        case SFX_ALIGN_FIRSTRIGHT:
            ret = 14;
            break;
        case SFX_ALIGN_RIGHT:
            ret = 15;
            break;
        case SFX_ALIGN_LASTRIGHT:
            ret = 16;
            break;
    }

    return ret;
}

void SfxWorkWindow::Sort_Impl()
{
    aSortedList.Remove(0, aSortedList.Count());
    for (USHORT i=0; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if (pCli)
        {
            USHORT k;
            for (k=0; k<aSortedList.Count(); k++)
//              if ( (*pChilds)[aSortedList[k]]->eAlign > pCli->eAlign )
                if (ChildAlignValue((*pChilds)[aSortedList[k]]->eAlign) >
                    ChildAlignValue(pCli->eAlign))
                    break;
            aSortedList.Insert (i,k);
        }
    }

    bSorted = TRUE;
}


//====================================================================
// ctor f"ur workwin eines Frames

SfxFrameWorkWin_Impl::SfxFrameWorkWin_Impl( Window *pWin, SfxFrame *pFrm )
    : SfxWorkWindow(
        pWin,
        pFrm->GetCurrentViewFrame()->GetBindings(),
        pFrm->GetParentFrame() ? pFrm->GetParentFrame()->GetWorkWindow_Impl() : NULL )
    , pFrame( pFrm )
{
    pConfigShell = pFrm->GetCurrentViewFrame();

    // Die ben"otigten SplitWindows (je eins f"ur jede Seite) werden erzeugt
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Die SplitWindows sind direkte ChildWindows des WorkWindows und enthalten
        // die angedockten Fenster.

        SfxChildAlignment eAlign =
                        ( n == SFX_SPLITWINDOWS_LEFT ? SFX_ALIGN_LEFT :
                            n == SFX_SPLITWINDOWS_RIGHT ? SFX_ALIGN_RIGHT :
                            n == SFX_SPLITWINDOWS_TOP ? SFX_ALIGN_TOP :
                                SFX_ALIGN_BOTTOM );
        SfxSplitWindow *pWin = new SfxSplitWindow(pWorkWin, eAlign, this, pParent==0 );
        pSplit[n] = pWin;
    }

    nOrigMode = SFX_VISIBILITY_CLIENT;
    nUpdateMode = SFX_VISIBILITY_STANDARD;
}

//====================================================================
// ctor f"ur workwin eines InPlaceObjects.

SfxIPWorkWin_Impl::SfxIPWorkWin_Impl( WorkWindow *pWin, SfxBindings& rB,
                        SfxInPlaceEnv_Impl *pE) :
    SfxWorkWindow(pWin, rB),
    pEnv(pE)
{
    // Die ben"otigten SplitWindows (je eins f"ur jede Seite) werden erzeugt
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Die SplitWindows sind direkte ChildWindows des WorkWindows und enthalten
        // die angedockten Fenster.

        SfxChildAlignment eAlign =
                        ( n == SFX_SPLITWINDOWS_LEFT ? SFX_ALIGN_LEFT :
                            n == SFX_SPLITWINDOWS_RIGHT ? SFX_ALIGN_RIGHT :
                            n == SFX_SPLITWINDOWS_TOP ? SFX_ALIGN_TOP :
                                SFX_ALIGN_BOTTOM );
        SfxSplitWindow *pWin = new SfxSplitWindow(pWorkWin, eAlign, this, FALSE);
        pSplit[n] = pWin;
    }

    nOrigMode = SFX_VISIBILITY_SERVER;
    nUpdateMode = SFX_VISIBILITY_SERVER;
    if ( !pParent )
        aStatBar.bOn = sal_True;
}

//====================================================================
// ctor der Basisklasse

SfxWorkWindow::SfxWorkWindow( Window *pWin, SfxBindings& rB, SfxWorkWindow* pParentWorkwin ) :
    pWorkWin (pWin),
    pParent( pParentWorkwin ),
    bSorted( TRUE ),
    pBindings(&rB),
    bDockingAllowed(TRUE),
    bAllChildsVisible(TRUE),
    nChilds( 0 ),
    nOrigMode( 0 ),
    pConfigShell( 0 ),
    pActiveChild( 0 )
{
    DBG_CTOR(SfxWorkWindow, 0);
    DBG_ASSERT (pBindings, "Keine Bindings!");

    pBindings->SetWorkWindow_Impl( this );

    pChildWins = new SfxChildWindows_Impl;
    pChilds = new SfxChildList_Impl;

    // F"ur die ObjectBars wird ein fester Platz in der ChildList reserviert,
    // damit sie immer in einer definierten Reihenfolge kommen.
    SfxChild_Impl* pChild=0;
    for (USHORT n=0; n < SFX_OBJECTBAR_MAX; ++n)
        pChilds->Insert(0,pChild);
}

//====================================================================
// dtor

SfxWorkWindow::~SfxWorkWindow()
{
    DBG_DTOR(SfxWorkWindow, 0);

    // SplitWindows l"oschen
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            ReleaseChild_Impl(*p);
        delete p;
    }

    // ObjectBars werden alle auf einmal released, da sie einen
    // festen zusammenh"angenden  Bereich im Array pChilds belegen
    pChilds->Remove(0, SFX_OBJECTBAR_MAX);

    // Hilfsstruktur f"ur Child-Windows l"oschen
    DBG_ASSERT( pChilds->Count() == 0, "dangling childs" );
    delete pChilds;
    delete pChildWins;
}

SystemWindow* SfxWorkWindow::GetTopWindow() const
{
    Window* pRet = pWorkWin;
    while ( pRet && !pRet->IsSystemWindow() )
        pRet = pRet->GetParent();
    return (SystemWindow*) pRet;
}

void SfxWorkWindow::ChangeWindow_Impl( Window *pNew )
{
    Window *pOld = pWorkWin;
    pWorkWin = pNew;
    for ( USHORT nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        SfxChild_Impl *pCli = (*pChilds)[nPos];
        if ( pCli && pCli->pWin && pCli->pWin->GetParent() == pOld )
        {
            pCli->pWin->SetParent( pNew );
        }
    }
}

//--------------------------------------------------------------------
// Hilfsmethode zum Freigeben der Childlisten. Wenn danach nicht der dtor
// aufgerufen wird, sondern weiter gearbeitet wird, mu\s wie im ctor von
// SfxWorkWindow noch Platz f"ur die Objectbars und SplitWindows reserviert
// werden.

void SfxWorkWindow::DeleteControllers_Impl()
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    // SplitWindows locken (d.h. Resize-Reaktion an den
    // DockingWindows unterdr"ucken)
    USHORT n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock();
    }

    // Child-Windows l"oschen
    USHORT nCount = pChildWins->Count();
    for ( n=0; n<nCount; n++ )
    {
        SfxChildWin_Impl* pCW = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            pChild->Hide();

            // Wenn das ChildWindow ein direktes Childfenster ist und nicht
            // in einem SplitWindow liegt, am WorkWindow abmelden.
            // Nach TH ist eine Abmeldung am Splitwindow nicht erforderlich,
            // wenn dieses auch gleich mit zerst"ort wird (s.u.).
            if (pCW->pCli)
                ReleaseChild_Impl(*pChild->GetWindow());
            pCW->pWin = 0;
            delete pChild;
        }
        delete pCW->pControl;
        delete pCW;
    }

    pChildWins->Remove((USHORT)0, nCount);

    // StatusBar l"oschen
    if ( aStatBar.pStatusBar )
    {
        StatusBar *pBar = aStatBar.pStatusBar->GetStatusBar();
        aStatBar.pStatusBar->StoreConfig();
        ReleaseChild_Impl( *pBar );
        DELETEZ( aStatBar.pStatusBar );
        delete pBar;
    }

    // ObjectBars l"oschen( zuletzt, damit pChilds nicht tote Pointer enh"alt )
    for (n=0; n<SFX_OBJECTBAR_MAX; n++)
    {
        delete (*pChilds)[TbxMatch(n)];

        // Nicht jede Position mu\s belegt sein
        SfxToolBoxManager *pTbx = aObjBars[n].pTbx;
        if (pTbx)
        {
            // Release siehe unten
            pTbx->StoreConfig();
            delete(pTbx);
        }
    }

    nChilds = 0;
}

//====================================================================
// Virtuelle Methode zum Anordnen der Childfenster.

void SfxWorkWindow::ArrangeChilds_Impl()
{
    aClientArea = GetTopRect_Impl();
    if ( nChilds )
        Arrange_Impl (aClientArea);
}

//====================================================================
// Virtuelle Methode zum Anordnen der Childfenster. Bei InPlaceObjects
// steht daf"ur als Fl"ache das Rectangle zur Verf"ugung, das am
// InPlaceClient als TopOuterRect abgefragt werden kann. Diese wird
// durch den von Arrange_Impl() errechneten SvBorder verringert.

void SfxIPWorkWin_Impl::ArrangeChilds_Impl()
{
    aClientArea = GetTopRect_Impl();

    SvBorder aBorder;
    if ( nChilds )
        aBorder = Arrange_Impl( aClientArea );

    // Wenn das zugeh"orige Objekt noch die UI-Kontrolle hat, mu\s der
    // AppBorder gesetzt werden; wenn hier allerdings die UITools entfernt
    // werden, tut das demn"achst der Container.
    // (->SfxContainerEnv_Impl::UIToolsShown())

    if ( pEnv->IsShowUITools() )
        pEnv->GetContainerEnv()->SetTopToolSpacePixel(aBorder);
    ArrangeAutoHideWindows( NULL );
}

//====================================================================
// Virtuelle Methode zum Anordnen der Childfenster. Bei einer Task
// steht daf"ur als Fl"ache die OutputSize des TaskWindows zur Verf"ugung.
// Sie wird durch den von Arrange_Impl() errechneten SvBorder verringert.

void SfxFrameWorkWin_Impl::ArrangeChilds_Impl()
{
    if ( pFrame->IsClosing_Impl() )
        return;

    aClientArea = GetTopRect_Impl();

    SvBorder aBorder;
    if ( nChilds )
        aBorder = Arrange_Impl (aClientArea);

    // Wenn das aktuelle Dokument der Applikation einen IPClient enth"alt, mu\s
    // dem dazugeh"origen Objekt durch SetTopToolFramePixel der zur Verf"ugung
    // stehende Platz zugeteilt werden. Das Objekt zeigt dann seine UITools an
    // und setzt den App-Border(->SfxInPlaceEnv_Impl::ArrangeChilds_Impl()).
    // Anderenfalls wird hier direkt der AppBorder gesetzt, um evtl. den Border
    // zu "uberschreiben, den bisher ein Objekt aus einem anderen Dokument
    // gesetzt hatte.
    // Das Objekt setzt, wenn es seine UI-Tools wegnimmt, den SetAppBorder nicht,
    // damit kein ObjectBar-Zappeln entsteht.
    // (->SfxInPlaceEnv_Impl::ArrangeChilds_Impl())
    SfxInPlaceClient *pClient = 0;
    SfxViewFrame *pF = pFrame->GetCurrentViewFrame();
    if ( pF && pF->GetViewShell() )
        pClient = pF->GetViewShell()->GetIPClient();

    if ( pClient )
        pClient->GetEnv()->SetTopToolFramePixel( aBorder );
    else
        pFrame->SetToolSpaceBorderPixel_Impl( aBorder );

    ArrangeAutoHideWindows( NULL );
}

//--------------------------------------------------------------------

SvBorder SfxWorkWindow::Arrange_Impl(Rectangle &rRect)

/*  [Beschreibung]

    Diese Methode ordnet alle sichtbaren ChildFenster so an, da\s die angedockten
    Fenster nach der Sorierreihenfolge von au\sen nach innen aneinander
    gesetzt werden. Wenn ein an sich sichtbares Fenster nicht mehr in die
    noch freie ClientArea pa\st, wird es auf "nicht sichtbar" gesetzt.

*/
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    Rectangle aTmp( rRect );

    if (!bSorted)
        Sort_Impl();

    SvBorder  aBorder;
    Point     aPos;
    Size      aSize;

    for ( USHORT n=0; n<aSortedList.Count(); ++n )
    {
        SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
        if ( !pCli->pWin )
            continue;

        // Zun"achst nehmen wir an, da\s das Fenster Platz hat
        pCli->nVisible |= CHILD_FITS_IN;

        // Nicht sichtbare Fenster "uberspringen
        if (pCli->nVisible != CHILD_VISIBLE)
            continue;

        if ( pCli->bResize )
            aSize = pCli->aSize;
        else
            aSize = pCli->pWin->GetSizePixel();

        SvBorder aTemp = aBorder;
        BOOL bAllowHiding = TRUE;
        switch ( pCli->eAlign )
        {
            case SFX_ALIGN_TOP:
            case SFX_ALIGN_TOOLBOXTOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_HIGHESTTOP:
                bAllowHiding = FALSE;
                aBorder.Top() += aSize.Height();
                aPos = aTmp.TopLeft();
                aTmp.Top() += aSize.Height();
                aSize.Width() = aTmp.GetWidth();
                break;

            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_TOOLBOXBOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                aBorder.Bottom() += aSize.Height();
                aPos = aTmp.BottomLeft();
                aPos.Y() -= (aSize.Height()-1);
                aTmp.Bottom() -= aSize.Height();
                aSize.Width() = aTmp.GetWidth();
                break;

            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_TOOLBOXLEFT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
                bAllowHiding = FALSE;
                aBorder.Left() += aSize.Width();
                aPos = aTmp.TopLeft();
                aTmp.Left() += aSize.Width();
                aSize.Height() = aTmp.GetHeight();
                break;

            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_TOOLBOXRIGHT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                aBorder.Right() += aSize.Width();
                aPos = aTmp.TopRight();
                aPos.X() -= (aSize.Width()-1);
                aTmp.Right() -= aSize.Width();
                aSize.Height() = aTmp.GetHeight();
                break;

            default:
                pCli->aSize = pCli->pWin->GetSizePixel();
                pCli->bResize = FALSE;
                continue;
                break;
        }

        pCli->pWin->SetPosSizePixel( aPos, aSize );
        pCli->bResize = FALSE;
        pCli->aSize = aSize;
        if( bAllowHiding && !RequestTopToolSpacePixel_Impl( aBorder ) )
        {
            pCli->nVisible ^= CHILD_FITS_IN;
            aBorder = aTemp;
        }
    }

    if ( rRect.GetWidth() >= aBorder.Left() + aBorder.Right() )
    {
        rRect.Left() += aBorder.Left();
        rRect.Right() -= aBorder.Right();
    }
    else
    {
        aBorder.Left() = rRect.Left();
        aBorder.Right() = rRect.Right();
        rRect.Right() = rRect.Left() = aTmp.Left();
    }

    if ( rRect.GetHeight() >= aBorder.Top() + aBorder.Bottom() )
    {
        rRect.Top() += aBorder.Top();
        rRect.Bottom() -= aBorder.Bottom();
    }
    else
    {
        aBorder.Top() = rRect.Top();
        aBorder.Bottom() = rRect.Bottom();
        rRect.Top() = rRect.Bottom() = aTmp.Top();
    }

    return IsDockingAllowed() ? aBorder : SvBorder();
}

//--------------------------------------------------------------------
// Close-Handler: die Konfiguration der ChildWindows wird gespeichert.
//

void SfxWorkWindow::Close_Impl()
{
    for (USHORT n=0; n<pChildWins->Count(); n++)
    {
        SfxChildWin_Impl *pCW  = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if (pChild)
        {
            BOOL bTask = ( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) != 0;
            pCW->aInfo = pChild->GetInfo();
            if ( bTask )
                pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
            SaveStatus_Impl(pChild, pCW->aInfo);
        }
    }
}

BOOL SfxWorkWindow::PrepareClose_Impl()
{
    for (USHORT n=0; n<pChildWins->Count(); n++)
    {
        SfxChildWin_Impl *pCW  = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        if ( pChild && !pChild->QueryClose() )
            return FALSE;
    }

    return TRUE;
}

//--------------------------------------------------------------------

SfxChild_Impl* SfxWorkWindow::RegisterChild_Impl( Window& rWindow,
                    SfxChildAlignment eAlign, BOOL bCanGetFocus )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
    DBG_ASSERT( pChilds->Count() < 255, "too many childs" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );
    DBG_ASSERT( !FindChild_Impl(rWindow), "child registered more than once" );


    if ( rWindow.GetParent() != pWorkWin )
        rWindow.SetParent( pWorkWin );

    SfxChild_Impl *pChild = new SfxChild_Impl(rWindow, rWindow.GetSizePixel(),
                                    eAlign, rWindow.IsVisible());
    pChild->bCanGetFocus = bCanGetFocus;

    pChilds->Insert(pChilds->Count(), pChild);
    bSorted = FALSE;
    nChilds++;
    return (*pChilds)[pChilds->Count()-1];
}

//--------------------------------------------------------------------

void SfxWorkWindow::AlignChild_Impl( Window& rWindow,
                                            const Size& rNewSize,
                                            SfxChildAlignment eAlign )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
    DBG_ASSERT( pChilds, "aligning unregistered child" );
    DBG_ASSERT( SfxChildAlignValid(eAlign), "invalid align" );

    SfxChild_Impl *pChild = FindChild_Impl(rWindow);
    if ( pChild )
    {
        if (pChild->eAlign != eAlign)
            bSorted = FALSE;

        pChild->eAlign = eAlign;
        pChild->aSize = rNewSize;
        pChild->bResize = TRUE;
    }
    else
        DBG_ERROR( "aligning unregistered child" );
}

//--------------------------------------------------------------------

void SfxWorkWindow::ReleaseChild_Impl( Window& rWindow )
{
    DBG_CHKTHIS(SfxWorkWindow, 0);
    DBG_ASSERT( pChilds, "releasing unregistered child" );

    SfxChild_Impl *pChild = 0;
    USHORT nCount = pChilds->Count();
    USHORT nPos;
    for ( nPos = 0; nPos < nCount; ++nPos )
    {
        pChild = (*pChilds)[nPos];
        if ( pChild )
          if ( pChild->pWin == &rWindow )
            break;
    }

    if ( nPos < nCount )
    {
        bSorted = FALSE;
        nChilds--;
        pChilds->Remove(nPos);
        delete pChild;
    }
    else
        DBG_ERROR( "releasing unregistered child" );
}

//--------------------------------------------------------------------

SfxChild_Impl* SfxWorkWindow::FindChild_Impl( const Window& rWindow ) const
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    SfxChild_Impl *pChild = 0;
    USHORT nCount = pChilds->Count();
    for ( USHORT nPos = 0; nPos < nCount; ++nPos )
    {
        pChild = (*pChilds)[nPos];
        if ( pChild )
          if ( pChild->pWin == &rWindow )
            return pChild;
    }

    return 0;
}

//--------------------------------------------------------------------

void SfxWorkWindow::ShowChilds_Impl()
{
    DBG_CHKTHIS(SfxWorkWindow, 0);

    SfxChild_Impl *pCli = 0;
    for ( USHORT nPos = 0; nPos < pChilds->Count(); ++nPos )
    {
        pCli = (*pChilds)[nPos];
        if (pCli && pCli->pWin)
        {
            if ( CHILD_VISIBLE == (pCli->nVisible & CHILD_VISIBLE) )
            {
                switch ( pCli->pWin->GetType() )
                {
                    case RSC_DOCKINGWINDOW :
                        ((DockingWindow*)pCli->pWin)->Show();
                        break;
                    case RSC_TOOLBOX :
                        ((ToolBox*)pCli->pWin)->Show();
                        break;
                    case RSC_SPLITWINDOW :
                        ((SplitWindow*)pCli->pWin)->Show();
                        break;
                    default:
                        pCli->pWin->Show();
                        break;
                }
            }
            else
            {
                switch ( pCli->pWin->GetType() )
                {
                    case RSC_DOCKINGWINDOW :
                    case RSC_TOOLBOX :
                        ((DockingWindow*)pCli->pWin)->Hide();
                        break;
                    default:
                        pCli->pWin->Hide();
                        break;
                }
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxWorkWindow::HideChilds_Impl()
{
    SfxChild_Impl *pChild = 0;
    for ( USHORT nPos = pChilds->Count(); nPos > 0; --nPos )
    {
        pChild = (*pChilds)[nPos-1];
        if (pChild && pChild->pWin)
        {
            switch ( pChild->pWin->GetType() )
            {
                case RSC_DOCKINGWINDOW :
                case RSC_TOOLBOX :
                    ((DockingWindow*)pChild->pWin)->Hide();
                    break;
                default:
                    pChild->pWin->Hide();
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------

void SfxWorkWindow::ResetObjectBars_Impl()
{
    USHORT n;
    for ( n = 0; n < SFX_OBJECTBAR_MAX; ++n )
    {
        aObjBars[n].nId = 0;
        aObjBarLists[n].aArr.Remove(0, aObjBarLists[n].aArr.Count() );
        aObjBarLists[n].nAct = 0;
    }

    for ( n = 0; n < pChildWins->Count(); ++n )
        (*pChildWins)[n]->nId = 0;

//  if ( pParent )
//      pParent->ResetObjectBars_Impl();
}

void SfxWorkWindow::NextObjectBar_Impl( USHORT nPos )
{
    USHORT nCount = aObjBarLists[nPos].aArr.Count();
    if ( nCount > 1 )
    {
        if ( ++(aObjBarLists[nPos].nAct) == nCount )
            aObjBarLists[nPos].nAct = 0;

        aObjBars[nPos] = aObjBarLists[nPos].Actual();
        UpdateObjectBars_Impl();
    }
}

USHORT SfxWorkWindow::HasNextObjectBar_Impl( USHORT nPos, String *pStr )
{
    USHORT nCount = aObjBarLists[nPos].aArr.Count();
    if ( nCount > 1 )
    {
        USHORT nAct = aObjBarLists[nPos].nAct;
        if ( ++nAct == nCount )
            nAct = 0;
        const SfxObjectBar_Impl& rObjbar = aObjBarLists[nPos].aArr[nAct];
        if ( pStr )
            *pStr = rObjbar.aName;
        return rObjbar.nId;
    }

    return 0;
}

//------------------------------------------------------------------------

void SfxWorkWindow::SetObjectBar_Impl( USHORT nPos, const ResId& rResId,
            SfxInterface* pIFace, const String *pName)
{
    DBG_ASSERT( (nPos & SFX_POSITION_MASK) < SFX_OBJECTBAR_MAX,
                "object bar position overflow" );

    USHORT nRealPos = nPos & SFX_POSITION_MASK;
    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
    {
        pParent->SetObjectBar_Impl( nPos, rResId, pIFace, pName );
        return;
    }

    SfxObjectBar_Impl& rObjBar = aObjBars[nRealPos];
    rObjBar.pIFace = pIFace;
    rObjBar.nId = rResId.GetId();
    rObjBar.pResMgr = rResId.GetResMgr();
    rObjBar.nMode = (nPos & SFX_VISIBILITY_MASK);
    if (pName)
        rObjBar.aName = *pName;
    else
        rObjBar.aName.Erase();

    SfxObjectBarArr_Impl& rArr = aObjBarLists[nRealPos].aArr;
    for ( USHORT n=0; n<rArr.Count(); n++ )
    {
        if ( rArr[n].nId == rObjBar.nId )
        {
            aObjBarLists[nRealPos].nAct = n;
            return;
        }
    }

    rArr.Insert( rObjBar, 0 );
}

//------------------------------------------------------------------------

FASTBOOL SfxWorkWindow::KnowsObjectBar_Impl( USHORT nPos ) const

/*  [Beschreibung]

    Stellt fest, ob an der betreffenden Position "uberhaupt eine
    Objektleiste zur Verf"ugung stehen w"urde. Ist unabh"agig davon,
    ob diese tats"achlich ein- oder ausgeschaltet ist.
*/

{
    USHORT nRealPos = nPos & SFX_POSITION_MASK;
    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
        return pParent->KnowsObjectBar_Impl( nPos );

    return 0 != aObjBars[nPos & SFX_POSITION_MASK].nId;
}

//------------------------------------------------------------------------

BOOL SfxWorkWindow::IsVisible_Impl( USHORT nMode ) const
{
    switch( nUpdateMode )
    {
        case SFX_VISIBILITY_STANDARD:
            return TRUE;
        case SFX_VISIBILITY_UNVISIBLE:
            return FALSE;
        case SFX_VISIBILITY_PLUGSERVER:
        case SFX_VISIBILITY_PLUGCLIENT:
        case SFX_VISIBILITY_CLIENT:
        case SFX_VISIBILITY_SERVER:
            return !!(nMode & nUpdateMode);
        default:
            return !!(nMode & nOrigMode ) ||
                nOrigMode == SFX_VISIBILITY_STANDARD;
    }
}

Window* SfxWorkWindow::GetObjectBar_Impl( USHORT nPos, ResId& rResId )
{
    USHORT nRealPos = nPos & SFX_POSITION_MASK;
    DBG_ASSERT( nRealPos < SFX_OBJECTBAR_MAX,
                "object bar position overflow" );

    if ( pParent && IsAppWorkWinToolbox_Impl( nRealPos ) )
        return pParent->GetObjectBar_Impl( nPos, rResId );

    rResId = ResId(aObjBars[nPos & SFX_POSITION_MASK].nId);
    rResId.SetResMgr(aObjBars[nPos & SFX_POSITION_MASK].pResMgr);

    return &aObjBars[nPos].pTbx->GetToolBox();
}

//------------------------------------------------------------------------
void SfxFrameWorkWin_Impl::UpdateObjectBars_Impl()
{
    if ( pFrame->IsClosing_Impl() )
        return;

    SfxWorkWindow *pWork = pParent;
    while ( pWork )
    {
        pWork->SfxWorkWindow::UpdateObjectBars_Impl();
        pWork = pWork->GetParent_Impl();
    }

    SfxWorkWindow::UpdateObjectBars_Impl();

//  if ( pTask->IsActive() )
    {
        pWork = pParent;
        while ( pWork )
        {
            pWork->ArrangeChilds_Impl();
            pWork = pWork->GetParent_Impl();
        }

        ArrangeChilds_Impl();

        pWork = pParent;
        while ( pWork )
        {
            pWork->ShowChilds_Impl();
            pWork = pWork->GetParent_Impl();
        }

        ShowChilds_Impl();
    }

    if ( aStatBar.pStatusBar )
    {
        StatusBar *pBar = aStatBar.pStatusBar->GetStatusBar();
//        pBar->SetBottomBorder( FALSE );
        pBar->SetPosSizePixel( 0, 0, 0, pBar->CalcWindowSizePixel().Height(), WINDOW_POSSIZE_HEIGHT );
        ArrangeChilds_Impl();
    }

    ShowChilds_Impl();
}

void SfxIPWorkWin_Impl::UpdateObjectBars_Impl()
{
    SfxWorkWindow::UpdateObjectBars_Impl();
    if ( pWorkWin->IsVisible() )
    {
        ArrangeChilds_Impl();
        ShowChilds_Impl();
    }
}

SfxStatusBarManager* SfxWorkWindow::GetStatusBarManager_Impl()
{
    return aStatBar.pStatusBar;
}

//------------------------------------------------------------------------

void SfxWorkWindow::UpdateObjectBars_Impl()
{
    // SplitWindows locken (d.h. Resize-Reaktion an den
    // DockingWindows unterdr"ucken)
    USHORT n;
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock();
    }

    // was man so "ofters braucht, merkt man sich (spart Code und Laufzeit)
    SfxApplication *pSfxApp = SFX_APP();
    SfxImageManager *pImgMgr = SFX_IMAGEMANAGER();
    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();

    // die Modi bestimmen, die im Kontext gelten
    // Geht InPlace nur bein internem InPlace, was aber egal ist, da bei
    // externem InPlace es schon wegen OLE nicht gehen kann !
    FASTBOOL bIsFullScreen = FALSE;
    Window* pSysWin = GetTopWindow();
    if ( pSysWin )
    {
        while ( pSysWin && pSysWin->GetType() != WINDOW_WORKWINDOW )
            pSysWin = pSysWin->GetParent();

        if ( pSysWin )
            bIsFullScreen = ((WorkWindow*)pSysWin)->IsFullScreenMode();
    }

    // "uber alle Toolbox-Positionen iterieren
    for ( n = 0; n < SFX_OBJECTBAR_MAX; ++n )
    {
        SfxChild_Impl *&rpCli = (*pChilds)[TbxMatch(n)];

        // Toolbox und/oder Id an der Position 'n'
        USHORT nId = aObjBars[n].nId;
        SfxToolBoxManager *&rpTbx = aObjBars[n].pTbx;
        FASTBOOL bToolBoxVisible = pTbxCfg->IsToolBoxPositionVisible(n);

        // die Modi bestimmen, f"ur die die ToolBox gilt
        USHORT nTbxMode = aObjBars[n].nMode;
        FASTBOOL bFullScreenTbx = SFX_VISIBILITY_FULLSCREEN ==
                    ( nTbxMode & SFX_VISIBILITY_FULLSCREEN );
        nTbxMode &= ~SFX_VISIBILITY_FULLSCREEN;

        // wird in diesem Kontext eine ToolBox gefordert?
        FASTBOOL bModesMatching = nUpdateMode && ( nTbxMode & nUpdateMode) == nUpdateMode;
        if ( nId != 0 && bToolBoxVisible == TRUE &&
             ( ( bModesMatching && !bIsFullScreen ) ||
               ( bIsFullScreen && bFullScreenTbx ) ) )
        {
            // keine oder falsche Toolbox an der Position 'n' vorhanden?
            if ( !rpTbx || ( rpTbx && rpTbx->GetType() != nId ) )
            {
                // (falsche) Toolbox vorhanden?
                if (rpTbx)
                {
                    // dann Toolbox-Inhalt autauschen
                    rpTbx = new SfxToolBoxManager(
                                    ResId( aObjBars[n].nId,
                                           aObjBars[n].pResMgr ),
                                    aObjBars[n].pIFace,
                                    aObjBars[n].aName,
                                    rpTbx, pConfigShell);
                    rpCli->pWin = &rpTbx->GetToolBox();
                }
                else
                {
                    // sonst Toolbox neu erzeugen
                    rpTbx = new SfxToolBoxManager( pWorkWin,
                                    GetBindings(),
                                    ResId( aObjBars[n].nId,
                                           aObjBars[n].pResMgr ),
                                    aObjBars[n].pIFace,
                                    n, pConfigShell );

                    rpTbx->Initialize();
                    SfxToolbox& rTbx = (SfxToolbox&) rpTbx->GetToolBox();
                    if ( !rpCli )
                    {
                        rpCli = new SfxChild_Impl( rTbx, Size(),
                                rTbx.IsFloatingMode() ? SFX_ALIGN_NOALIGNMENT : rTbx.GetAlignment(), TRUE );
                    }
                    else
                    {
                        rpCli->pWin = &rTbx;
                    }

                    bSorted = FALSE;
                    nChilds++;
                }

                ToolBox& rTbx = rpTbx->GetToolBox();
                rTbx.SetText(aObjBars[n].aName);
                if ( rTbx.IsFloatingMode() )
                    rpCli->aSize = rTbx.GetFloatingWindow()->GetSizePixel();
                else
                {
                    rpCli->bResize = TRUE;
                    rpCli->aSize = rTbx.GetSizePixel();
                }

                // Schnelles Statusupdate wirkt optisch besser
                rpTbx->UpdateControls_Impl();
            }
            else
            {
                ToolBox& rTbx = rpTbx->GetToolBox();
                BOOL bWasFloating = rTbx.IsFloatingMode();
                rpTbx->Reconfigure_Impl( pTbxCfg->GetCfgItem_Impl( n ) );
                rTbx.SetText(aObjBars[n].aName);
                String aNext;
                rpTbx->GetNextToolBox_Impl( &aNext );
                rTbx.SetNextToolBox( aNext );

                if ( bWasFloating && !rTbx.IsFloatingMode() )
                {
                    // Toolbox einer nicht aktiven Task wurde angedockt und
                    // jetzt wurde sie wieder aktiviert.
                    // Falls angedockte Toolboxen sichtbar sein sollen, mu\s
                    // sie geshowed werden, da sie vorher in HidePopups
                    // gehidet wurde
                    if ( IsDockingAllowed() && bAllChildsVisible )
                        rpCli->nVisible |= CHILD_ACTIVE;
                    if ( CHILD_VISIBLE == (rpCli->nVisible & CHILD_VISIBLE) )
                        rTbx.Show();
                }
            }
        }
        else
        {
            // ggf. Toolbox an dieser Position l"oschen
            if (rpTbx)
            {
                rpTbx->StoreConfig();
                rpCli->pWin = 0;
                SfxToolBoxManager *p = rpTbx;
                rpTbx = 0;
                p->Delete();
                bSorted = FALSE;
                nChilds--;
            }
        }

        if ( rpTbx && !IsDockingAllowed() )
        {
            // Presentation mode
            if ( !rpTbx->GetToolBox().IsFloatingMode() )
            {
                // keine angedockten Toolboxen
                rpCli->pWin = 0;
                SfxToolBoxManager *p = rpTbx;
                rpTbx = 0;
                p->Delete();
                bSorted = FALSE;
                nChilds--;
            }
            else if( !bAllChildsVisible )
                // angedockte Toolboxen bis zum Livemodus hiden
                rpCli->nVisible &= ~CHILD_ACTIVE;
        }
    }

    UpdateChildWindows_Impl();

    // SplitWindows wieder ent-locken
    for ( n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if (p->GetWindowCount())
            p->Lock(FALSE);
    }

    UpdateStatusBar_Impl();
}

void SfxWorkWindow::UpdateChildWindows_Impl()
{
    FASTBOOL bIsFullScreen = FALSE;
    Window* pSysWin = GetTopWindow();
    if ( pSysWin )
    {
        while ( pSysWin && pSysWin->GetType() != WINDOW_WORKWINDOW )
            pSysWin = pSysWin->GetParent();

        if ( pSysWin )
            bIsFullScreen = ((WorkWindow*)pSysWin)->IsFullScreenMode();
    }

    // alle vorhandenen oder in den Kontext gekommenen ChildWindows
    for ( USHORT n=0; n<pChildWins->Count(); n++ )
    {
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
        SfxChildWindow *pChildWin = pCW->pWin;
        BOOL bCreate = FALSE;
        if ( pCW->nId && !pCW->bDisabled  && IsVisible_Impl( pCW->nVisibility ) )
        {
            // Im Kontext ist ein geeignetes ChildWindow erlaubt;
            // ist es auch eingeschaltet ?
            if ( pChildWin == 0 && pCW->bCreate )
            {
                if ( !IsDockingAllowed() || bIsFullScreen )
                {
                    // im PresentationMode oder FullScreen nur FloatingWindows
                    SfxChildAlignment eAlign;
                    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
                        bCreate = ( eAlign == SFX_ALIGN_NOALIGNMENT );
                }
                else
                    bCreate = TRUE;

                // Momentan kein Fenster da, aber es ist eingeschaltet; Fenster
                // und ggf. Context erzeugen
                if ( bCreate )
                    CreateChildWin_Impl( pCW );

                if ( !bAllChildsVisible )
                {
                    if ( pCW->pCli )
                        pCW->pCli->nVisible &= ~CHILD_ACTIVE;
                }
            }
            else if ( pChildWin )
            {
                // Fenster existiert schon; soll es auch sichtbar sein ?
                if ( ( !bIsFullScreen || pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT ) && bAllChildsVisible )
                {
                    // Updatemode ist kompatibel; auf jeden Fall wieder einschalten
                    bCreate = TRUE;
                    if ( pCW->pCli )
                    {
                        // Fenster ist direktes Child
                        if ( bAllChildsVisible && ( IsDockingAllowed() || pCW->pCli->eAlign == SFX_ALIGN_NOALIGNMENT ) )
                            pCW->pCli->nVisible |= CHILD_NOT_HIDDEN;
                    }
                    else if ( pCW->bCreate && IsDockingAllowed() )
                        // Fenster liegt in einem SplitWindow
                        ((SfxDockingWindow*)pChildWin->GetWindow())->Reappear_Impl();

                    if ( pCW->nInterfaceId != pChildWin->GetContextId() )
                        pChildWin->CreateContext( pCW->nInterfaceId, GetBindings() );
                }
            }
        }

        if ( pChildWin && !bCreate )
        {
            Window *pWindow = pChildWin->GetWindow();
            if ( pWindow && ( pWindow->HasFocus() || pWindow->HasChildPathFocus( TRUE ) ) )
            {
                // Wenn der Focus auf dem Childwindow sitzt, mu\s er auf das aktuelle
                // Dokument gesetzt werden
                // Nein !! Das führt gerne mal zu Rekursionen, da im GetFocus-Notify erneut ein Update ausgel"ost wird!
                // ->#66426#
                // SfxViewFrame *pView = SfxViewFrame::Current();
                // if ( pView )
                //  pView->GetFrame()->GrabFocusOnComponent_Impl();
            }

            if ( !pChildWin->QueryClose() || pChildWin->IsHideNotDelete() || Application::IsUICaptured() )
            {
                if ( pCW->pCli )
                {
                    if ( pCW->pCli->nVisible & CHILD_NOT_HIDDEN )
                        pCW->pCli->nVisible ^= CHILD_NOT_HIDDEN;
                }
                else
                    ((SfxDockingWindow*)pChildWin->GetWindow())->Disappear_Impl();
            }
            else
                RemoveChildWin_Impl( pCW );
        }
    }
}

void SfxWorkWindow::CreateChildWin_Impl( SfxChildWin_Impl *pCW )
{
    if ( pCW->aInfo.bVisible != 42 )
        pCW->aInfo.bVisible = TRUE;

    SfxChildWindow *pChildWin = SfxChildWindow::CreateChildWindow( pCW->nId, pWorkWin, &GetBindings(), pCW->aInfo);
    if (pChildWin)
    {
        pChildWin->SetWorkWindow_Impl( this );
#if 0
        // Enable-Status richtig setzen
        pChildWin->GetWindow()->EnableInput( pCW->bEnable &&
            ( pWorkWin->IsInputEnabled() /* || pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT */ ) );
#endif
        // Zumindest der ExtraString wird beim Auswerten ver"andert, also neu holen
        BOOL bTask = ( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) != 0;
        pCW->aInfo = pChildWin->GetInfo();
        if ( bTask )
            pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;

        // Nein !! Sonst kann man keine Fenster defaultmaessig ausschalten ( Partwindow! )
//      pCW->aInfo.bVisible = TRUE;

        // Erzeugung war erfolgreich
        GetBindings().Invalidate(pCW->nId);

        USHORT nPos = pChildWin->GetPosition();
        if (nPos != CHILDWIN_NOPOS)
        {
            DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
            if ((*pChilds)[TbxMatch(nPos)])// &&
//                            pChildWin->GetAlignment() == (*pChilds)[nPos]->eAlign )
            {
                // ChildWindow ersetzt ObjectBar
                (*pChilds)[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
            }
        }

        pCW->pWin = pChildWin;

        if ( pChildWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT ||
                pChildWin->GetWindow()->GetParent() == pWorkWin)
        {
            // Das Fenster ist entweder nicht angedockt oder au\serhalb
            // eines SplitWindows angedockt und mu\s daher explizit als
            // Child registriert werden
            pCW->pCli = RegisterChild_Impl(*(pChildWin->GetWindow()),
                            pChildWin->GetAlignment(), pChildWin->CanGetFocus());
            pCW->pCli->nVisible = CHILD_VISIBLE;
            FASTBOOL bIsFullScreen = FALSE;
            Window* pSysWin = GetTopWindow();
            if ( pSysWin )
            {
                while ( pSysWin && pSysWin->GetType() != WINDOW_WORKWINDOW )
                    pSysWin = pSysWin->GetParent();

                if ( pSysWin )
                    bIsFullScreen = ((WorkWindow*)pSysWin)->IsFullScreenMode();
            }

            if ( pChildWin->GetAlignment() != SFX_ALIGN_NOALIGNMENT && bIsFullScreen )
                    pCW->pCli->nVisible ^= CHILD_ACTIVE;
        }
        else
        {
            // Ein angedocktes Fenster, dessen Parent nicht das WorkWindow ist,
            // mu\s in einem SplitWindow liegen und daher nicht explizit
            // registriert werden.
            // Das passiert aber schon bei der Initialisierung des
            // SfxDockingWindows!
        }

        if ( pCW->nInterfaceId != pChildWin->GetContextId() )
            pChildWin->CreateContext( pCW->nInterfaceId, GetBindings() );

        // Information in der INI-Datei sichern
        SaveStatus_Impl(pChildWin, pCW->aInfo);
    }
}

void SfxWorkWindow::RemoveChildWin_Impl( SfxChildWin_Impl *pCW )
{
    USHORT nId = pCW->nSaveId;
    SfxChildWindow *pChildWin = pCW->pWin;

    // vorhandenes Fenster geht aus dem Kontext und wird daher entfernt
    USHORT nPos = pChildWin->GetPosition();
    if (nPos != CHILDWIN_NOPOS)
    {
        // ChildWindow "uberlagert einen ObjectBar
        DBG_ASSERT(nPos < SFX_OBJECTBAR_MAX, "Illegal objectbar position!");
        if ((*pChilds)[TbxMatch(nPos)] &&
            (aObjBars[nPos].nMode & nUpdateMode) ) //&&
//                         pChildWin->GetAlignment() == (*pChilds)[nPos]->eAlign )
        {
            // ObjectBar war "uberlagert; jetzt wieder anzeigen
            (*pChilds)[TbxMatch(nPos)]->nVisible ^= CHILD_NOT_HIDDEN;
        }
    }

    // Information in der INI-Datei sichern
    BOOL bTask = ( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) != 0;
    pCW->aInfo = pChildWin->GetInfo();
    if ( bTask )
        pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
    SaveStatus_Impl(pChildWin, pCW->aInfo);

    pChildWin->Hide();

    if ( pCW->pCli )
    {
        // ChildWindow ist ein direktes ChildWindow und mu\s sich daher
        // beim WorkWindow abmelden
        pCW->pCli = 0;
        ReleaseChild_Impl(*pChildWin->GetWindow());
    }
    else
    {
        // ChildWindow liegt in einem SplitWindow und meldet sich
        // selbst im dtor dort ab
    }

    pCW->pWin = 0;
    delete pChildWin;

    GetBindings().Invalidate( nId );
}

void SfxWorkWindow::ResetStatusBar_Impl()
{
    aStatBar.nId = 0;
    aStatBar.pShell = NULL;
    aStatBar.pBindings = NULL;
}

//--------------------------------------------------------------------
void SfxWorkWindow::SetStatusBar_Impl( const ResId& rResId, SfxShell *pSh, SfxBindings& rBindings )
{
    if ( rResId.GetId() )
    {
        aStatBar.nId = rResId.GetId();
        aStatBar.pShell = pSh;
        aStatBar.pBindings = &rBindings;
    }
}

void SfxWorkWindow::SetTempStatusBar_Impl( BOOL bSet )
{
    if ( aStatBar.bTemp != bSet )
    {
        BOOL bOn = FALSE;
        SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();
        FASTBOOL bIsFullScreen = FALSE;
        Window* pSysWin = GetTopWindow();
        if ( pSysWin )
        {
            while ( pSysWin && pSysWin->GetType() != WINDOW_WORKWINDOW )
                pSysWin = pSysWin->GetParent();

            if ( pSysWin )
                bIsFullScreen = ((WorkWindow*)pSysWin)->IsFullScreenMode();
        }

        if ( aStatBar.nId && aStatBar.bOn && !bIsFullScreen && pTbxCfg->IsStatusBarVisible() )
            bOn = TRUE;

        aStatBar.bTemp = bSet;
        if ( !bOn )
        {
            if ( aStatBar.pStatusBar && !aStatBar.pStatusBar->GetBindings_Impl() )
            {
                DBG_ERROR( "Kaputte Statusbar-Bindings!" );
                return;
            }

            // Nur was tun, wenn die Temp-Einstellung wirklich was bewirkt
            UpdateStatusBar_Impl();
            ArrangeChilds_Impl();

            if ( aStatBar.pStatusBar )
            {
                StatusBar *pBar = aStatBar.pStatusBar->GetStatusBar();
                pBar->SetBottomBorder( TRUE );
                pBar->SetPosSizePixel( 0, 0, 0, pBar->CalcWindowSizePixel().Height(), WINDOW_POSSIZE_HEIGHT );
                ArrangeChilds_Impl();
            }

            ShowChilds_Impl();
        }
    }
}

//------------------------------------------------------------------------
void SfxWorkWindow::UpdateStatusBar_Impl()
{
    // Die aktuelle Statusleiste
    StatusBar *pBar = NULL;
    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();

    // Falls schon eine StatusBar existiert: ist es die gew"unschte?
    USHORT nActId = 0;
    if ( aStatBar.pStatusBar )
    {
        pBar = aStatBar.pStatusBar->GetStatusBar();
        nActId = aStatBar.pStatusBar->GetType();
    }

    // keine Statusleiste, wenn keine Id gew"unscht oder bei FullScreenView
    // oder wenn ausgeschaltet
    FASTBOOL bIsFullScreen = FALSE;
    Window* pSysWin = GetTopWindow();
    if ( pSysWin )
    {
        while ( pSysWin && pSysWin->GetType() != WINDOW_WORKWINDOW )
            pSysWin = pSysWin->GetParent();

        if ( pSysWin )
            bIsFullScreen = ((WorkWindow*)pSysWin)->IsFullScreenMode();
    }

    if ( aStatBar.nId && IsDockingAllowed() &&
            ( aStatBar.bOn && !bIsFullScreen && pTbxCfg->IsStatusBarVisible() || aStatBar.bTemp ) )
    {
        if ( aStatBar.nId != nActId || aStatBar.pStatusBar && aStatBar.pStatusBar->GetBindings_Impl() != aStatBar.pBindings )
        {
            // Id hat sich ge"andert, also passenden Statusbarmanager erzeugen,
            // dieser "ubernimmt die aktuelle Statusleiste;
            if ( aStatBar.pStatusBar )
            {
                // Der Manager erzeugt die Statusleiste nur, er zerst"ort sie
                // nicht !
                aStatBar.pStatusBar->StoreConfig();
                DELETEZ( aStatBar.pStatusBar );
            }

            aStatBar.pStatusBar = new SfxStatusBarManager(
                GetWindow(),
                *aStatBar.pBindings,
                aStatBar.pShell,
                aStatBar.nId, pBar, pConfigShell );

            if ( !pBar )
            {
                pBar = aStatBar.pStatusBar->GetStatusBar();
                RegisterChild_Impl( *pBar, SFX_ALIGN_LOWESTBOTTOM );
            }
        }
    }
    else if ( nActId )
    {
        // Aktuelle StatusBar vernichten
        // Der Manager erzeugt die Statusleiste nur, er zerst"ort sie
        // nicht !
        aStatBar.pStatusBar->StoreConfig();
        ReleaseChild_Impl( *pBar );
        DELETEZ( aStatBar.pStatusBar );
        DELETEZ( pBar );
    }
}

//------------------------------------------------------------------------

void SfxWorkWindow::SetObjectBarVisibility_Impl( USHORT nMask )
{
    switch( nMask )
    {
        case SFX_VISIBILITY_UNVISIBLE:
        case SFX_VISIBILITY_STANDARD:
        case SFX_VISIBILITY_PLUGSERVER:
        case SFX_VISIBILITY_PLUGCLIENT:
        case SFX_VISIBILITY_CLIENT:
        case SFX_VISIBILITY_SERVER:
            nOrigMode = nMask;
    }
    if (nMask != nUpdateMode)
        nUpdateMode = nMask;
}

BOOL SfxWorkWindow::IsContainer_Impl() const
{
    return nUpdateMode == SFX_VISIBILITY_CLIENT;
}

//------------------------------------------------------------------------
void SfxWorkWindow::HidePopups_Impl(BOOL bHide, BOOL bParent, USHORT nId )
{
    if ( nId )
    {
        for ( USHORT n = 0; n < SFX_OBJECTBAR_MAX; ++n )
        {
            SfxChild_Impl *pCli = (*pChilds)[TbxMatch(n)];
            if ( pCli && pCli->eAlign == SFX_ALIGN_NOALIGNMENT )
            {
                BOOL bHasTbx = ( aObjBars[n].pTbx != 0 );
                if (bHide)
                {
                    pCli->nVisible &= ~CHILD_ACTIVE;
                    if ( bHasTbx )
                        aObjBars[n].pTbx->GetToolBox().Hide();
                }
                else
                {
                    pCli->nVisible |= CHILD_ACTIVE;
                    if ( bHasTbx && CHILD_VISIBLE == (pCli->nVisible & CHILD_VISIBLE) )
                        aObjBars[n].pTbx->GetToolBox().Show();
                }
            }
        }
    }

    for ( USHORT n = 0; n < pChildWins->Count(); ++n )
    {
        SfxChildWindow *pCW = (*pChildWins)[n]->pWin;
        if (pCW && pCW->GetAlignment() == SFX_ALIGN_NOALIGNMENT && pCW->GetType() != nId)
        {
            Window *pWin = pCW->GetWindow();
            SfxChild_Impl *pChild = FindChild_Impl(*pWin);
            if (bHide)
            {
                pChild->nVisible &= ~CHILD_ACTIVE;
                pCW->Hide();
            }
            else
            {
                pChild->nVisible |= CHILD_ACTIVE;
                if ( CHILD_VISIBLE == (pChild->nVisible & CHILD_VISIBLE) )
                    pCW->Show();
            }
        }
    }

    if ( bParent && pParent )
        pParent->HidePopups_Impl( bHide, bParent, nId );
}

//------------------------------------------------------------------------

void SfxWorkWindow::ConfigChild_Impl(SfxChildIdentifier eChild,
            SfxDockingConfig eConfig, USHORT nId)
{
    SfxDockingWindow* pDockWin=0;
    SfxToolbox *pBox=0;
    USHORT nPos = USHRT_MAX;
    Window *pWin=0;
    SfxChildWin_Impl *pCW = 0;

    if (eChild == SFX_CHILDWIN_OBJECTBAR)
    {
        // Es soll eine Toolbox konfiguriert werden
        USHORT n;
        for (n=0; n<SFX_OBJECTBAR_MAX; n++)
        {
            if (aObjBars[n].nId == nId && aObjBars[n].pTbx)
                break;
        }

        DBG_ASSERT( pParent || n<SFX_OBJECTBAR_MAX, "Unbekannte ToolBox!" );
        if (n>=SFX_OBJECTBAR_MAX)
        {
            if (pParent)
                pParent->ConfigChild_Impl( eChild, eConfig, nId );
            return;
        }

        pWin = pBox = (SfxToolbox*) &aObjBars[n].pTbx->GetToolBox();
    }
    else
    {
        // Es soll ein DockingWindow als direktes Child konfiguriert werden
        for (USHORT n=0; n<pChildWins->Count(); n++)
        {
            pCW = (*pChildWins)[n];
            SfxChildWindow *pChild = pCW->pWin;
            if (pChild)
            {
                if (pChild->GetType() == nId)
                {
                    pDockWin = (SfxDockingWindow*) pChild->GetWindow();
                    break;
                }
            }
        }

        if ( pDockWin )
        {
            if ( eChild == SFX_CHILDWIN_DOCKINGWINDOW ||
                pDockWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT )
            {
                if ( eChild == SFX_CHILDWIN_SPLITWINDOW && eConfig == SFX_TOGGLEFLOATMODE)
                {
                    // Ein DockingWindow wurde aus einem SplitWindow rausgezogen
                    pCW->pCli =
                        RegisterChild_Impl(*pDockWin, pDockWin->GetAlignment(), pCW->pWin->CanGetFocus());
                    pCW->pCli->nVisible = CHILD_VISIBLE;
                }

                pWin = pDockWin;
            }
            else
            {
                SfxSplitWindow *pSplitWin = GetSplitWindow_Impl(pDockWin->GetAlignment());

                // Es soll ein DockingWindow in einem SplitWindow konfiguriert werden
                if ( eConfig == SFX_TOGGLEFLOATMODE)
                {
                    // Ein DockingWindow wurde in ein SplitWindow hineingezogen
                    pCW->pCli = 0;
                    ReleaseChild_Impl(*pDockWin);
                }

                pWin = pSplitWin->GetSplitWindow();
                if ( pSplitWin->GetWindowCount() == 1 )
                    ((SplitWindow*)pWin)->Show();
            }
        }

        DBG_ASSERT( pDockWin, "Unbekanntes DockingWindow!" );
        if ( !pDockWin && pParent )
        {
            pParent->ConfigChild_Impl( eChild, eConfig, nId );
            return;
        }
    }

    // Eventuell sind Childs an - oder abgemeldet worden
    if (!bSorted)
        Sort_Impl();

    SfxChild_Impl *pChild = 0;
    USHORT n;
    for ( n=0; n<aSortedList.Count(); ++n )
    {
        pChild = (*pChilds)[aSortedList[n]];
        if ( pChild )
            if ( pChild->pWin == pWin )
            break;
    }

    nPos = aSortedList[n];

    switch ( eConfig )
    {
        case SFX_SETDOCKINGRECTS :
        {
            SfxChild_Impl *pChild = (*pChilds)[nPos];
            Rectangle aOuterRect( GetTopRect_Impl() );
            aOuterRect.SetPos(
                pWorkWin->OutputToScreenPixel( aOuterRect.TopLeft() ));
            Rectangle aInnerRect(aOuterRect);
            BOOL bTbx = (eChild == SFX_CHILDWIN_OBJECTBAR);

            // Das gerade betroffene Fenster wird bei der Berechnung des
            // inneren Rechtecks mit eingeschlossen!
            for ( USHORT m=0; m<aSortedList.Count(); ++m )
            {
                USHORT n=aSortedList[m];
                SfxChild_Impl* pCli = (*pChilds)[n];

                if ( pCli && pCli->nVisible == CHILD_VISIBLE && pCli->pWin )
                {
                    switch ( pCli->eAlign )
                    {
                        case SFX_ALIGN_TOP:
                            // Objekt-Toolboxen kommen immer zuletzt
                            if ( bTbx || n <= nPos)
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_TOOLBOXTOP:
                            // Toolbox geht nur vor, wenn nicht h"ohere Position
                            if ( bTbx && n <= nPos)
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_HIGHESTTOP:
                            // Geht immer vor
                            aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LOWESTTOP:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if ( n == nPos )
                                aInnerRect.Top() += pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_BOTTOM:
                            // Objekt-Toolboxen kommen immer zuletzt
                            if ( bTbx || n <= nPos)
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_TOOLBOXBOTTOM:
                            // Toolbox geht nur vor, wenn nicht h"ohere Position
                            if ( bTbx && n <= nPos)
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LOWESTBOTTOM:
                            // Geht immer vor
                            aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_HIGHESTBOTTOM:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if ( n == nPos )
                                aInnerRect.Bottom() -= pCli->aSize.Height();
                            break;

                        case SFX_ALIGN_LEFT:
                            // Toolboxen kommen immer zuletzt
                            if (bTbx || n <= nPos)
                                aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_TOOLBOXLEFT:
                            // Toolboxen kommen immer zuletzt
                            if (bTbx && n <= nPos)
                                aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_FIRSTLEFT:
                            // Geht immer vor
                            aInnerRect.Left() += pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_LASTLEFT:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if (n == nPos)
                                aInnerRect.Left() += pCli->aSize.Width();

                        case SFX_ALIGN_RIGHT:
                            // Toolboxen kommen immer zuletzt
                            if (bTbx || n <= nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_TOOLBOXRIGHT:
                            // Toolboxen kommen immer zuletzt
                            if (bTbx && n <= nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_FIRSTRIGHT:
                            // Wird nur mitgez"ahlt, wenn es das aktuelle Fenster ist
                            if (n == nPos)
                                aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        case SFX_ALIGN_LASTRIGHT:
                            // Geht immer vor
                            aInnerRect.Right() -= pCli->aSize.Width();
                            break;

                        default:
                            break;
                    }
                }
            }

            if (pBox)
                pBox->SetDockingRects(aOuterRect, aInnerRect);
            else
                pDockWin->SetDockingRects(aOuterRect, aInnerRect);
            break;
        }

        case SFX_MOVEDOCKINGWINDOW :
        case SFX_ALIGNDOCKINGWINDOW :
        case SFX_TOGGLEFLOATMODE:
        {
            SfxChildAlignment eAlign;
            SfxChild_Impl *pCli = (*pChilds)[nPos];
            if (pBox)
            {
                if ( pBox->IsFloatingMode() )
                {
                    eAlign = SFX_ALIGN_NOALIGNMENT;
                    pCli->aSize = pBox->GetFloatingWindow()->GetSizePixel();
                }
                else
                {
                    eAlign = pBox->GetAlignment();
                    pCli->bResize = TRUE;
                    Size aActSize( pBox->GetSizePixel() );
                    pCli->aSize = pBox->CalcWindowSizePixel();
                    if ( pBox->IsHorizontal() )
                        pCli->aSize.Width() = aActSize.Width();
                    else
                        pCli->aSize.Height() = aActSize.Height();
                }
            }
            else
            {
                eAlign = pDockWin->GetAlignment();
                if ( eChild == SFX_CHILDWIN_DOCKINGWINDOW ||
                    eAlign == SFX_ALIGN_NOALIGNMENT)
                {
                    // wenn innerhalb eines SplitWindows umkonfiguriert
                    // wurde, "andert sich am SplitWindow selbst nichts
                    pCli->bResize = TRUE;
                    pCli->aSize = pDockWin->GetSizePixel();
                }
            }

            if ( pCli->eAlign != eAlign )
                bSorted = FALSE;
            pCli->eAlign = eAlign;
            ArrangeChilds_Impl();
            ShowChilds_Impl();

            // INI schreiben
            if ( pCW )
            {
                BOOL bTask = ( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) != 0;
                pCW->aInfo = pCW->pWin->GetInfo();
                if ( bTask )
                    pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
                if ( eConfig != SFX_MOVEDOCKINGWINDOW )
                    SaveStatus_Impl( pCW->pWin, pCW->aInfo);
            }
            break;
        }
    }
}


//--------------------------------------------------------------------

void SfxWorkWindow::SetChildWindowVisible_Impl( ULONG lId, BOOL bEnabled, USHORT nMode )
{
    USHORT nInter = (USHORT) ( lId >> 16 );
    USHORT nId = (USHORT) ( lId & 0xFFFF );

    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Den obersten parent nehmen; ChildWindows werden immer am WorkWindow
    // der Task bzw. des Frames oder am AppWorkWindow angemeldet
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // Dem Parent schon bekannt ?
        USHORT nCount = pWork->pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Kein Parent oder dem Parent noch unbekannt, dann bei mir suchen
        USHORT nCount = pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Ist neu, also initialisieren; je nach Flag beim Parent oder bei
        // mir eintragen
        pCW = new SfxChildWin_Impl( lId );
        pCW->nId = nId;
        InitializeChild_Impl( pCW );
        if ( pWork && !( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) )
            pWork->pChildWins->Insert( pWork->pChildWins->Count(), pCW );
        else
            pChildWins->Insert( pChildWins->Count(), pCW );
    }

    pCW->nId = nId;
    if ( nInter )
        pCW->nInterfaceId = nInter;
    pCW->nVisibility = nMode;
    pCW->bEnable = bEnabled;
#if 0
    if ( pCW->pWin )
        pCW->pWin->GetWindow()->EnableInput( bEnabled &&
            ( pWorkWin->IsInputEnabled() /* || pCW->pWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT */ ) );
#endif
    pCW->nVisibility = nMode;
}

//--------------------------------------------------------------------
// Der An/Aus-Status eines ChildWindows wird umgeschaltet.

void SfxWorkWindow::ToggleChildWindow_Impl(USHORT nId)
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nId == nId)
            break;

    if ( n<nCount )
    {
        // Das Fenster ist schon bekannt
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;

        if ( pChild && pCW->bCreate )
        {
            if ( pChild->QueryClose() )
            {
                Window *pWindow = pChild->GetWindow();
                if ( pWindow && ( pWindow->HasFocus() || pWindow->HasChildPathFocus( TRUE ) ) )
                {
                    // Wenn der Focus auf dem Childwindow sitzt, mu\s er auf das aktuelle
                    // Dokument gesetzt werden
                    // Nein !! Das führt gerne mal zu Rekursionen, da im GetFocus-Notify erneut ein Update ausgel"ost wird!
                // ->#66426#
                    // SfxViewFrame *pView = SfxViewFrame::Current();
                    // if ( pView )
                    //  pView->GetFrame()->GrabFocusOnComponent_Impl();
                }

                if ( pChild && pChild->IsHideAtToggle() )
                {
                    pCW->bCreate = !pCW->bCreate;
                    ShowChildWindow_Impl( nId, pCW->bCreate );
                }
                else
                {
                    // Fenster soll ausgeschaltet werdem
                    pCW->bCreate = FALSE;
                    pChild->SetVisible_Impl( FALSE );
                    RemoveChildWin_Impl( pCW );
                }
            }
        }
        else if ( pCW->bCreate )
        {
            pCW->bCreate = FALSE;
        }
        else
        {
            pCW->bCreate = TRUE;

            if ( pChild )
            {
                ShowChildWindow_Impl( nId, pCW->bCreate );
            }
            else
            {
                // Fenster erzeugen
                CreateChildWin_Impl( pCW );
                if ( !pCW->pWin )
                    // Erzeugung war erfolglos
                    pCW->bCreate = FALSE;
            }
        }

        ArrangeChilds_Impl();
        ShowChilds_Impl();

        if ( pCW->bCreate )
        {
            if ( !pCW->pCli )
            {
                SfxDockingWindow *pDock =
                    (SfxDockingWindow*) pCW->pWin->GetWindow();
                if ( nId == SID_BROWSER )
                    SfxFrame::BeamerSet_Impl();
                else if ( pDock->IsAutoHide_Impl() )
                    pDock->AutoShow_Impl();
            }
        }

        return;
    }
    else if ( pParent )
    {
        pParent->ToggleChildWindow_Impl( nId );
        return;
    }

#ifdef DBG_UTIL
    nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if ( n < nCount )
    {
        DBG_ERROR("ChildWindow ist nicht im Kontext!");
    }
    else
    {
        DBG_ERROR("ChildWindow ist nicht registriert!");
    }
#endif
}

//--------------------------------------------------------------------

BOOL SfxWorkWindow::HasChildWindow_Impl(USHORT nId)
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if (n<nCount)
    {
        SfxChildWin_Impl *pCW = (*pChildWins)[n];
        SfxChildWindow *pChild = pCW->pWin;
        return ( pChild && pCW->bCreate );
    }

    if ( pParent )
        return pParent->HasChildWindow_Impl( nId );

    return FALSE;
}

BOOL SfxWorkWindow::IsFloating( USHORT nId )
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Den obersten parent nehmen; ChildWindows werden immer am WorkWindow
    // der Task bzw. des Frames oder am AppWorkWindow angemeldet
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // Dem Parent schon bekannt ?
        USHORT nCount = pWork->pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Kein Parent oder dem Parent noch unbekannt, dann bei mir suchen
        USHORT nCount = pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Ist neu, also initialisieren; je nach Flag beim Parent oder bei
        // mir eintragen
        pCW = new SfxChildWin_Impl( nId );
        pCW->bEnable = FALSE;
        pCW->nId = 0;
        pCW->nVisibility = 0;
        InitializeChild_Impl( pCW );
        if ( pWork && !( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) )
            pWork->pChildWins->Insert( pWork->pChildWins->Count(), pCW );
        else
            pChildWins->Insert( pChildWins->Count(), pCW );
    }

    SfxChildAlignment eAlign;
    if ( pCW->aInfo.GetExtraData_Impl( &eAlign ) )
        return( eAlign == SFX_ALIGN_NOALIGNMENT );
    else
        return TRUE;
}

//--------------------------------------------------------------------

BOOL SfxWorkWindow::KnowsChildWindow_Impl(USHORT nId)
{
    SfxChildWin_Impl *pCW=0;
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
    {
        pCW = (*pChildWins)[n];
        if ( pCW->nSaveId == nId)
             break;
    }

    if (n<nCount)
    {
        if ( !IsVisible_Impl(  pCW->nVisibility ) )
            return FALSE;
        return pCW->bEnable;
    }
    else if ( pParent )
        return pParent->KnowsChildWindow_Impl( nId );
    else
        return FALSE;
}

//--------------------------------------------------------------------

void SfxWorkWindow::SetChildWindow_Impl(USHORT nId, BOOL bOn)
{
    SfxChildWin_Impl *pCW=NULL;
    SfxWorkWindow *pWork = pParent;

    // Den obersten parent nehmen; ChildWindows werden immer am WorkWindow
    // der Task bzw. des Frames oder am AppWorkWindow angemeldet
    while ( pWork && pWork->pParent )
        pWork = pWork->pParent;

    if ( pWork )
    {
        // Dem Parent schon bekannt ?
        USHORT nCount = pWork->pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pWork->pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pWork->pChildWins)[n];
                break;
            }
    }

    if ( !pCW )
    {
        // Kein Parent oder dem Parent noch unbekannt, dann bei mir suchen
        USHORT nCount = pChildWins->Count();
        for (USHORT n=0; n<nCount; n++)
            if ((*pChildWins)[n]->nSaveId == nId)
            {
                pCW = (*pChildWins)[n];
                pWork = this;
                break;
            }
    }

    if ( !pCW )
    {
        // Ist neu, also initialisieren; je nach Flag beim Parent oder bei
        // mir eintragen
        pCW = new SfxChildWin_Impl( nId );
        InitializeChild_Impl( pCW );
        if ( !pWork || pCW->aInfo.nFlags & SFX_CHILDWIN_TASK )
            pWork = this;
        pWork->pChildWins->Insert( pWork->pChildWins->Count(), pCW );
    }

    if ( pCW->bCreate != bOn )
        pWork->ToggleChildWindow_Impl(nId);
}

//--------------------------------------------------------------------

void SfxWorkWindow::ShowChildWindow_Impl(USHORT nId, BOOL bVisible)
{
    USHORT nCount = pChildWins->Count();
    SfxChildWin_Impl* pCW=0;
    USHORT n;
    for (n=0; n<nCount; n++)
    {
        pCW = (*pChildWins)[n];
        if (pCW->nId == nId)
            break;
    }

    if ( n<nCount )
    {
        SfxChildWindow *pChildWin = pCW->pWin;
        if ( pChildWin )
        {
//          pCW->bCreate = bVisible;
            if ( bVisible )
            {
                if ( pCW->pCli )
                {
                    pCW->pCli->nVisible = CHILD_VISIBLE;
                    pChildWin->Show();
                }
                else
                    ((SfxDockingWindow*)pChildWin->GetWindow())->Reappear_Impl();

            }
            else
            {
                if ( pCW->pCli )
                {
                    pCW->pCli->nVisible = CHILD_VISIBLE ^ CHILD_NOT_HIDDEN;
                    pCW->pWin->Hide();
                }
                else
                    ((SfxDockingWindow*)pChildWin->GetWindow())->Disappear_Impl();

            }

            ArrangeChilds_Impl();
            ShowChilds_Impl();
        }
        else if ( bVisible )
        {
            SetChildWindow_Impl( nId, TRUE );
            pChildWin = pCW->pWin;
        }

        if ( pChildWin )
        {
            pChildWin->SetVisible_Impl( bVisible );
            BOOL bTask = ( pCW->aInfo.nFlags & SFX_CHILDWIN_TASK ) != 0;
            pCW->aInfo = pChildWin->GetInfo();
            if ( bTask )
                pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
            if ( !pCW->bCreate )
                SaveStatus_Impl( pChildWin, pCW->aInfo );
        }

        return;
    }

    if ( pParent )
    {
        pParent->ShowChildWindow_Impl( nId, bVisible );
        return;
    }

#ifdef DBG_UTIL
    nCount = pChildWins->Count();
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
            break;

    if ( n<nCount )
    {
        DBG_ERROR("ChildWindow ist nicht im Kontext!");
    }
    else
    {
        DBG_ERROR("ChildWindow ist nicht registriert!");
    }
#endif
}

//--------------------------------------------------------------------

SfxChildWindow* SfxWorkWindow::GetChildWindow_Impl(USHORT nId)
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
             break;

    if (n<nCount)
        return (*pChildWins)[n]->pWin;
    else if ( pParent )
        return pParent->GetChildWindow_Impl( nId );
    return 0;
}

//------------------------------------------------------------------------

void SfxWorkWindow::ResetChildWindows_Impl()
{
//  if ( pParent )
//      pParent->ResetChildWindows_Impl();

    for ( USHORT n = 0; n < pChildWins->Count(); ++n )
    {
        (*pChildWins)[n]->nId = 0;
        (*pChildWins)[n]->bEnable = FALSE;
    }
}

//------------------------------------------------------------------------
// Virtuelle Methode, die die Gr"o\se der Fl"ache (client area) des parent
// windows liefert, in der Child-Fenster angeordnet werden k"onnen.
// in der ClientArea des parent findet.

Rectangle SfxWorkWindow::GetTopRect_Impl()
{
    return Rectangle (Point(), pWorkWin->GetOutputSizePixel() );
}

//------------------------------------------------------------------------
// Virtuelle Methode, die die Gr"o\se der Fl"ache (client area) des parent
// windows liefert, in der Child-Fenster angeordnet werden k"onnen.
// in der ClientArea des parent findet.

Rectangle SfxFrameWorkWin_Impl::GetTopRect_Impl()
{
    return pFrame->GetTopOuterRectPixel_Impl();
}

//------------------------------------------------------------------------
// Virtuelle Methode, die die Gr"o\se der Fl"ache (client area) des parent
// windows liefert, in der Child-Fenster angeordnet werden k"onnen.
// in der ClientArea des parent findet.

Rectangle SfxIPWorkWin_Impl::GetTopRect_Impl()
{
    return pEnv->GetContainerEnv()->GetTopOuterRectPixel();
}

//------------------------------------------------------------------------
// Virtuelle Methode, um herauszufinden, ob ein Child-Fenster noch Platz
// in der ClientArea des parent findet.

BOOL SfxWorkWindow::RequestTopToolSpacePixel_Impl( SvBorder aBorder )
{
    if ( !IsDockingAllowed() ||
            aClientArea.GetWidth() < aBorder.Left() + aBorder.Right() ||
            aClientArea.GetHeight() < aBorder.Top() + aBorder.Bottom() )
        return FALSE;
    else
        return TRUE;;
}

//------------------------------------------------------------------------
// Implementation am InPlaceObject: benutzt die SO-Methode, um am InPlaceClient
// um Platz nachzusuchen.

BOOL SfxIPWorkWin_Impl::RequestTopToolSpacePixel_Impl( SvBorder aBorder )
{
    if ( !IsDockingAllowed() )
        return FALSE;
    else
        return pEnv->GetContainerEnv()->RequestTopToolSpacePixel( aBorder );
}


void SfxIPWorkWin_Impl::SaveStatus_Impl(SfxChildWindow *pChild, const SfxChildWinInfo &rInfo)
{
    if ( pEnv->IsInternalInPlace() )
        SfxWorkWindow::SaveStatus_Impl( pChild, rInfo );
}

void SfxWorkWindow::SaveStatus_Impl(SfxChildWindow *pChild, const SfxChildWinInfo &rInfo)
{
    // Den Status vom Presentation mode wollen wir nicht sichern
    if ( IsDockingAllowed() )
        pChild->SaveStatus(rInfo);
}

void SfxWorkWindow::InitializeChild_Impl(SfxChildWin_Impl *pCW)
{
    SfxChildWinFactory* pFact=0;
    SfxApplication *pApp = SFX_APP();
    SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
    for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
    {
        pFact = rFactories[nFactory];
        if ( pFact->nId == pCW->nSaveId )
        {
            pCW->aInfo   = pFact->aInfo;
            SfxChildWindow::InitializeChildWinFactory_Impl(
                                        pCW->nSaveId, pCW->aInfo);
            pCW->bCreate = pCW->aInfo.bVisible;
            USHORT nFlags = pFact->aInfo.nFlags;
            if ( nFlags & SFX_CHILDWIN_TASK )
                pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
            if ( nFlags & SFX_CHILDWIN_CANTGETFOCUS )
                pCW->aInfo.nFlags |= SFX_CHILDWIN_CANTGETFOCUS;
            pFact->aInfo = pCW->aInfo;
            return;
        }
    }

    SfxDispatcher *pDisp = pBindings->GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? pApp->GetActiveModule( pDisp->GetFrame() ) :0;
    if ( pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == pCW->nSaveId )
                {
                    pCW->aInfo   = pFact->aInfo;
                    SfxChildWindow::InitializeChildWinFactory_Impl(
                                                pCW->nSaveId, pCW->aInfo);
                    pCW->bCreate = pCW->aInfo.bVisible;
                    USHORT nFlags = pFact->aInfo.nFlags;
                    if ( nFlags & SFX_CHILDWIN_TASK )
                        pCW->aInfo.nFlags |= SFX_CHILDWIN_TASK;
                    if ( nFlags & SFX_CHILDWIN_CANTGETFOCUS )
                        pCW->aInfo.nFlags |= SFX_CHILDWIN_CANTGETFOCUS;
                    pFact->aInfo = pCW->aInfo;
                    return;
                }
            }
        }
    }
}

SfxSplitWindow* SfxWorkWindow::GetSplitWindow_Impl( SfxChildAlignment eAlign )
{
    switch ( eAlign )
    {
        case SFX_ALIGN_TOP:
            return pSplit[2];
            break;

        case SFX_ALIGN_BOTTOM:
            return pSplit[3];
            break;

        case SFX_ALIGN_LEFT:
            return pSplit[0];
            break;

        case SFX_ALIGN_RIGHT:
            return pSplit[1];
            break;

        default:
            return 0;
            break;
    }
}

void SfxWorkWindow::MakeChildsVisible_Impl( BOOL bVis )
{
    if ( pParent )
        pParent->MakeChildsVisible_Impl( bVis );

    bAllChildsVisible = bVis;
    if ( bVis )
    {
        if ( !bSorted )
            Sort_Impl();
        for ( USHORT n=0; n<aSortedList.Count(); ++n )
        {
            SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
            if ( pCli->eAlign == SFX_ALIGN_NOALIGNMENT || IsDockingAllowed() )
                pCli->nVisible |= CHILD_ACTIVE;
        }
    }
    else
    {
        if ( !bSorted )
            Sort_Impl();
        for ( USHORT n=0; n<aSortedList.Count(); ++n )
        {
            SfxChild_Impl* pCli = (*pChilds)[aSortedList[n]];
            pCli->nVisible &= ~CHILD_ACTIVE;
        }
    }
}

BOOL SfxWorkWindow::IsAutoHideMode( const SfxSplitWindow *pSplitWin )
{
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        if ( pSplit[n] != pSplitWin && pSplit[n]->IsAutoHide( TRUE ) )
            return TRUE;
    }
    return FALSE;
}


void SfxWorkWindow::EndAutoShow_Impl()
{
    if ( pParent )
        pParent->EndAutoShow_Impl();

    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        SfxSplitWindow *p = pSplit[n];
        if ( p && p->IsAutoHide() )
            p->FadeOut();
    }
}

void SfxWorkWindow::ArrangeAutoHideWindows( SfxSplitWindow *pActSplitWin )
{
    if ( pParent )
        pParent->ArrangeAutoHideWindows( pActSplitWin );

    Rectangle aArea( aClientArea );
    for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
    {
        // Es werden entweder Dummyfenster oder Fenster im AutoShow-Modus
        // ( nicht gepinned, FadeIn ) behandelt.
        // Nur das "ubergebene Fenster darf unsichtbar sein, denn vielleicht
        // soll daf"ur gerade die Gr"o\se berechnet werden, bevor es angezeigt
        // wird.
        SfxSplitWindow* pSplitWin = pSplit[n];
        BOOL bDummyWindow = !pSplitWin->IsFadeIn();
        Window *pDummy = pSplitWin->GetSplitWindow();
        Window *pWin = bDummyWindow ? pDummy : pSplitWin;
        if ( pSplitWin->IsPinned() && !bDummyWindow || !pWin->IsVisible() && pActSplitWin != pSplitWin )
            continue;

        // Breite und Position des Dummy-Fensters als Ausgangspunkt
        Size aSize = pDummy->GetSizePixel();
        Point aPos = pDummy->GetPosPixel();

        switch ( n )
        {
            case ( 0 ) :
            {
                // Linkes SplitWindow
                // Breite vom Fenster selbst holen, wenn nicht das DummyWindow
                if ( !bDummyWindow )
                    aSize.Width() = pSplitWin->GetSizePixel().Width();

                // Wenn links ein Window sichtbar ist, beginnt der freie
                // Bereich rechts davon bzw. bei der Client area
                long nLeft = aPos.X() + aSize.Width();
                if ( nLeft > aArea.Left() )
                    aArea.Left() = nLeft;
                break;
            }
            case ( 1 ) :
            {
                // Rechtes SplitWindow
                // Position um Differenz der Breiten korrigieren
                aPos.X() += aSize.Width();

                // Breite vom Fenster selbst holen, wenn nicht das DummyWindow
                if ( !bDummyWindow )
                    aSize.Width() = pSplitWin->GetSizePixel().Width();

                aPos.X() -= aSize.Width();

                // Wenn links schon ein Fenster aufgeklappt ist, darf
                // das rechte nicht dar"uber gehen
                if ( aPos.X() < aArea.Left() )
                {
                    aPos.X() = aArea.Left();
                    aSize.Width() = aArea.GetWidth();
                }

                // Wenn rechts ein Window sichtbar ist, endet der freie
                // Bereich links davon bzw. bei der Client area
                long nRight = aPos.X();
                if ( nRight < aArea.Right() )
                    aArea.Right() = nRight;
                break;
            }
            case ( 2 ) :
            {
                // Oberes SplitWindow
                // H"ohe vom Fenster selbst holen, wenn nicht das DummyWindow
                if ( !bDummyWindow )
                    aSize.Height() = pSplitWin->GetSizePixel().Height();

                // Breite anpassen, je nachdem ob links oder rechts
                // schon ein Fenster aufgeklappt ist
                aPos.X() = aArea.Left();
                aSize.Width() = aArea.GetWidth();

                // Wenn oben ein Window sichtbar ist, beginnt der freie
                // Bereich darunter bzw. bei der Client Area
                long nTop = aPos.Y() + aSize.Height();
                if ( nTop > aArea.Top() )
                    aArea.Top() = nTop;
                break;
            }
            case ( 3 ) :
            {
                // Das untere SplitWindow
                // Position um Differenz der H"ohen korrigieren
                aPos.Y() += aSize.Height();

                // H"ohe vom Fenster selbst holen, wenn nicht das DummmyWindow
                if ( !bDummyWindow )
                    aSize.Height() = pSplitWin->GetSizePixel().Height();

                aPos.Y() -= aSize.Height();

                // Breite anpassen, je nachdem ob links oder rechts
                // schon ein Fenster aufgeklappt ist
                aPos.X() = aArea.Left();
                aSize.Width() = aArea.GetWidth();

                // Wenn oben schon ein Fenster aufgeklappt ist, darf
                // das untere nicht dar"uber gehen
                if ( aPos.Y() < aArea.Top() )
                {
                    aPos.Y() = aArea.Top();
                    aSize.Height() = aArea.GetHeight();
                }

                break;
            }
        }

        if ( !bDummyWindow )
            // Das FadeIn-Window ist ein Float, dessen Koordinaten in
            // Screenkoordinaten gesetzt werden
            pSplitWin->SetPosSizePixel( pWorkWin->OutputToScreenPixel(aPos), aSize );
        else
            // Das angedockte DummyWindow
            pDummy->SetPosSizePixel( aPos, aSize );
    }
}

Rectangle SfxWorkWindow::GetFreeArea( BOOL bAutoHide ) const
{
    if ( bAutoHide )
    {
        Rectangle aArea( aClientArea );
        for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
        {
            if ( pSplit[n]->IsPinned() || !pSplit[n]->IsVisible() )
                continue;

            Size aSize = pSplit[n]->GetSizePixel();
            switch ( n )
            {
                case ( 0 ) :
                    aArea.Left() += aSize.Width();
                    break;
                case ( 1 ) :
                    aArea.Right() -= aSize.Width();
                    break;
                case ( 2 ) :
                    aArea.Top() += aSize.Height();
                    break;
                case ( 3 ) :
                    aArea.Bottom() -= aSize.Height();
                    break;
            }
        }

        return aArea;
    }
    else
        return aClientArea;
}

SfxChildWinController_Impl::SfxChildWinController_Impl( USHORT nId, SfxWorkWindow *pWork )
    : SfxControllerItem( nId, pWork->GetBindings() )
    , pWorkwin( pWork )
{}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SfxWorkWindow::CreateDispatch( const String& )
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >();
}

void SfxChildWinController_Impl::StateChanged( USHORT nSID, SfxItemState eState,
                            const SfxPoolItem* pState )
{
    pWorkwin->DisableChildWindow_Impl( nSID, eState == SFX_ITEM_DISABLED );
}

void SfxWorkWindow::DisableChildWindow_Impl( USHORT nId, BOOL bDisable )
{
    USHORT nCount = pChildWins->Count();
    USHORT n;
    for (n=0; n<nCount; n++)
        if ((*pChildWins)[n]->nSaveId == nId)
             break;
    if ( n<nCount && (*pChildWins)[n]->bDisabled != bDisable )
    {
        (*pChildWins)[n]->bDisabled = bDisable;
        UpdateChildWindows_Impl();
        ArrangeChilds_Impl();
        ShowChilds_Impl();
    }
}

void SfxWorkWindow::SetActiveChild_Impl( Window *pChild )
{
    pActiveChild = pChild;
}

Window* SfxWorkWindow::GetActiveChild_Impl()
{
    return pActiveChild;
}

BOOL SfxWorkWindow::ActivateNextChild_Impl( BOOL bForward )
{
    // Alle Kinder gem"a\s Liste sortieren
    SvUShorts aList;
    USHORT i;
    for (i=SFX_OBJECTBAR_MAX; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if ( pCli && pCli->bCanGetFocus && pCli->pWin )
        {
            USHORT k;
            for (k=0; k<aList.Count(); k++)
                if ( ChildTravelValue((*pChilds)[aList[k]]->eAlign) > ChildTravelValue(pCli->eAlign) )
                    break;
            aList.Insert(i,k);
        }
    }

    // Wenn der Focus in der Task liegt, ist pActiveChild zu ignorieren
    SfxFrame *pTask = SfxViewFrame::Current()->GetFrame();
    BOOL bTaskActive = pTask->GetWindow().HasChildPathFocus();

    // F"ur das Taskfenster einen Dummy-Eintrag in die Liste
    USHORT nDummy =0xFFFF;
    USHORT nTopValue  = ChildTravelValue( SFX_ALIGN_LOWESTTOP );
    for ( i=0; i<aList.Count(); i++ )
    {
        SfxChild_Impl* pCli = (*pChilds)[aList[i]];
        if ( pCli->pWin && ChildTravelValue( pCli->eAlign ) > nTopValue )
            break;
    }

    aList.Insert( nDummy, i );

    USHORT n = bForward ? 0 : aList.Count()-1;
    if ( bTaskActive )
        n = i;

    SfxChild_Impl *pAct=NULL;
    if ( pActiveChild && !bTaskActive )
    {
        // Das aktive Fenster suchen
        for ( n=0; n<aList.Count(); n++ )
        {
            if ( aList[n] == nDummy )
                continue;

            SfxChild_Impl* pCli = (*pChilds)[aList[n]];
            if ( pCli && pCli->pWin && ( pCli->pWin == pActiveChild || !pActiveChild ) )
            {
                pAct = pCli;
                break;
            }
        }
    }

    if ( pAct )
    {
        for ( USHORT i=0; i<SFX_SPLITWINDOWS_MAX; i++ )
        {
            // Eventuell ist pAct ein Splitwindow
            SfxSplitWindow *p = pSplit[i];
            if ( pAct->pWin == p )
            {
                if( p->ActivateNextChild_Impl( bForward ) )
                    return TRUE;
                break;
            }
        }

        // pAct ist ein direktes ChildWindow
        // mit dem Nachfolger bzw. Vorg"anger des aktiven Fensters weitermachen
        if ( bForward )
            n = n+1<aList.Count() ? n+1 : 0;
        else
            n = n>0 ? n-1 : aList.Count() - 1;
    }

    USHORT nStart = n;
    for( ;; )
    {
        if ( aList[n] == nDummy )
        {
            // Das n"achste Fenster ist das Taskfenster
            SfxWorkWindow* pTaskWin = pTask->GetWorkWindow_Impl();
            if ( pActiveChild && !bTaskActive )
                pTaskWin->pActiveChild = NULL;
            if ( pTaskWin->ActivateNextChild_Impl( bForward ) )
                return TRUE;
        }
        else
        {
            SfxChild_Impl* pCli = (*pChilds)[aList[n]];
            if ( pCli == pAct )
                // Alle durchgesehen
                break;
            else if ( pCli->pWin )
            {
                SfxChild_Impl* pNext = pCli;
                for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
                {
                    // Eventuell ist pNext ein Splitwindow
                    SfxSplitWindow *p = pSplit[n];
                    if ( pNext->pWin == p )
                    {
                        // Das erste/letzte Fenster dort aktivieren
                        p->SetActiveWindow_Impl( NULL );
                        pNext = NULL;
                        if( p->ActivateNextChild_Impl( bForward ) )
                            return TRUE;
                        break;
                    }
                }

                if ( pNext )
                {
                    pNext->pWin->GrabFocus();
                    pActiveChild = pNext->pWin;
                    return TRUE;
                }
            }
        }

        if ( bForward )
            n = n+1<aList.Count() ? n+1 : 0;
        else
            n = n>0 ? n-1 : aList.Count() - 1;

        if ( n == nStart )
            // Alle durchgesehen
            break;
    }

    return FALSE;
}

/*
BOOL SfxTaskWorkWin_Impl::ActivateNextChild_Impl( BOOL bForward )
{
    // Alle Kinder gem"a\s Liste sortieren
    SvUShorts aList;
    USHORT i;
    for (i=SFX_OBJECTBAR_MAX; i<pChilds->Count(); i++)
    {
        SfxChild_Impl *pCli = (*pChilds)[i];
        if ( pCli && pCli->bCanGetFocus && pCli->pWin )
        {
            USHORT k;
            for (k=0; k<aList.Count(); k++)
                if ( ChildTravelValue((*pChilds)[aList[k]]->eAlign) > ChildTravelValue(pCli->eAlign) )
                    break;
            aList.Insert(i,k);
        }
    }

    BOOL bFrameActive = pTask->GetActiveFrame()->GetWindow().HasChildPathFocus();

    // F"ur das Taskfenster einen Dummy-Eintrag in die Liste
    USHORT nDummy =0xFFFF;
    USHORT nTopValue  = ChildTravelValue( SFX_ALIGN_LOWESTTOP );
    for ( i=0; i<aList.Count(); i++ )
    {
        SfxChild_Impl* pCli = (*pChilds)[aList[i]];
        if ( pCli->pWin && ChildTravelValue( pCli->eAlign ) > nTopValue )
            break;
    }

    aList.Insert( nDummy, i );

    // Index des aktiven Fensters
    USHORT n = bForward ? 0 : aList.Count()-1;
    if ( bFrameActive )
        n = i;

    SfxChild_Impl *pAct=NULL;
    if ( pActiveChild && !bFrameActive )
    {
        // Das aktive Fenster suchen
        for ( n=0; n<aList.Count(); n++ )
        {
            if ( aList[n] == nDummy )
                continue;

            SfxChild_Impl* pCli = (*pChilds)[aList[n]];
            if ( pCli && pCli->pWin && ( pCli->pWin == pActiveChild || !pActiveChild ) )
            {
                pAct = pCli;
                break;
            }
        }
    }

    if ( pAct )
    {
        for ( USHORT i=0; i<SFX_SPLITWINDOWS_MAX; i++ )
        {
            // Eventuell ist pAct ein Splitwindow
            SfxSplitWindow *p = pSplit[i];
            if ( pAct->pWin == p )
            {
                if( p->ActivateNextChild_Impl( bForward ) )
                    return TRUE;
                break;
            }
        }
    }

    if ( pAct || bFrameActive )
    {
        // mit dem Nachfolger bzw. Vorg"anger des aktiven Fensters weitermachen
        if ( bForward )
        {
            n = n+1;
            if ( n == aList.Count() )
                return FALSE;
        }
        else
        {
            if ( n == 0 )
                return FALSE;
            n = n-1;
        }
    }

    for( ;; )
    {
        if ( aList[n] == nDummy )
        {
            SfxFrame *pFrame = pTask->GetFrame_Impl( bForward ? 0 : ( pTask->GetFrameCount_Impl() - 1 ) );
            if ( !pFrame->ActivateNextChildWindow_Impl( bForward ) )
                pFrame->GetFrameInterface()->activate();
            return TRUE;
        }
        else
        {
            SfxChild_Impl* pCli = (*pChilds)[aList[n]];
            if ( pCli == pAct )
                // Alle durchgesehen
                break;
            else if ( pCli->pWin )
            {
                SfxChild_Impl* pNext = pCli;
                for ( USHORT n=0; n<SFX_SPLITWINDOWS_MAX; n++ )
                {
                    // Eventuell ist pNext ein Splitwindow
                    SfxSplitWindow *p = pSplit[n];
                    if ( pNext->pWin == p )
                    {
                        // Das erste/letzte Fenster dort aktivieren
                        p->SetActiveWindow_Impl( NULL );
                        pNext = NULL;
                        if( p->ActivateNextChild_Impl( bForward ) )
                            return TRUE;
                        break;
                    }
                }

                if ( pNext )
                {
                    pNext->pWin->GrabFocus();
                    pActiveChild = pNext->pWin;
                    return TRUE;
                }
            }
        }

        if ( bForward )
        {
            n = n+1;
            if ( n == aList.Count() )
                break;
        }
        else
        {
            if ( n == 0 )
                break;
            n = n-1;
        }

    }

    return FALSE;
}
 */

