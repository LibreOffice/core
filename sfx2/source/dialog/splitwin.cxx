/*************************************************************************
 *
 *  $RCSfile: splitwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:46 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#pragma hdrstop

#include "splitwin.hxx"
#include "workwin.hxx"
#include "dockwin.hxx"
#include "app.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "dialog.hrc"
#include "sfxresid.hxx"
#include "mnumgr.hxx"
#include "virtmenu.hxx"
#include "msgpool.hxx"
#include "viewfrm.hxx"

#define VERSION 1
#define nPixel 30L

struct SfxDock_Impl
{
    USHORT              nType;
    SfxDockingWindow*   pWin;           // SplitWindow hat dieses Fenster
    BOOL                bNewLine;
    BOOL                bHide;          // SplitWindow hatte dieses Fenster
    long                nSize;
};

typedef SfxDock_Impl* SfxDockPtr;
SV_DECL_PTRARR_DEL( SfxDockArr_Impl, SfxDockPtr, 4, 4);
SV_IMPL_PTRARR( SfxDockArr_Impl, SfxDockPtr);

class SfxEmptySplitWin_Impl : public SplitWindow
{
/*  [Beschreibung]

    Das SfxEmptySplitWin_Impldow ist ein leeres SplitWindow, das das SfxSplitWindow
    im AutoHide-Modus ersetzt. Es dient nur als Platzhalter, um MouseMoves
    zu empfangen und ggf. das eigentlichte SplitWindow einzublenden
*/
friend class SfxSplitWindow;

    SfxSplitWindow*     pOwner;
    BOOL                bFadeIn;
    BOOL                bAutoHide;
    BOOL                bSplit;
    BOOL                bEndAutoHide;
    Timer               aTimer;
    Point               aLastPos;
    USHORT              nState;

                        SfxEmptySplitWin_Impl( SfxSplitWindow *pParent )
                            : SplitWindow( pParent->GetParent(), WinBits( WB_BORDER | WB_3DLOOK ) )
                            , pOwner( pParent )
                            , bFadeIn( FALSE )
                            , bAutoHide( FALSE )
                            , bEndAutoHide( FALSE )
                            , bSplit( FALSE )
                            , nState( 1 )
                        {
                            aTimer.SetTimeoutHdl(
                                LINK(pOwner, SfxSplitWindow, TimerHdl ) );
                            aTimer.SetTimeout( 200 );
                            EnableDrop( TRUE );
                            SetAlign( pOwner->GetAlign() );
                            Actualize();
                            ShowAutoHideButton( pOwner->IsAutoHideButtonVisible() );
                            ShowFadeInHideButton( TRUE );
                        }

                        ~SfxEmptySplitWin_Impl()
                        {
                            aTimer.Stop();
                        }

    virtual void        MouseMove( const MouseEvent& );
    virtual BOOL        QueryDrop( DropEvent& rEvt );
    virtual void        AutoHide();
    virtual void        FadeIn();
    void                Actualize();
};

void SfxEmptySplitWin_Impl::Actualize()
{
    Size aSize( pOwner->GetSizePixel() );
    switch ( pOwner->GetAlign() )
    {
        case WINDOWALIGN_LEFT:
        case WINDOWALIGN_RIGHT:
            aSize.Width() = GetFadeInSize();
            break;
        case WINDOWALIGN_TOP:
        case WINDOWALIGN_BOTTOM:
            aSize.Height() = GetFadeInSize();
            break;
    }

    SetSizePixel( aSize );
}

void SfxEmptySplitWin_Impl::AutoHide()
{
    pOwner->SetPinned_Impl( !pOwner->bPinned );
    FadeIn();
    aLastPos = GetPointerPosPixel();
    aTimer.Start();
}

void SfxEmptySplitWin_Impl::FadeIn()
{
    if (!bAutoHide )
        bAutoHide = IsFadeNoButtonMode();
    pOwner->SetFadeIn_Impl( TRUE );
    pOwner->Show_Impl();
    if ( bAutoHide )
    {
        // Timer zum Schlie\sen aufsetzen; der Aufrufer mu\s selbst sicherstellen,
        // da\s das Window nicht gleich wieder zu geht ( z.B. durch Setzen des
        // Focus oder einen modal mode )
        aLastPos = GetPointerPosPixel();
        aTimer.Start();
    }
}

BOOL SfxEmptySplitWin_Impl::QueryDrop( DropEvent& rEvt )
{
#ifndef OS2
    bAutoHide = TRUE;
    FadeIn();
#endif

    return FALSE;
}


BOOL SfxSplitWindow::QueryDrop( DropEvent& rEvt )
{
    Point aMousePos( rEvt.GetPosPixel() );
    Rectangle aRect( GetFadeOutRect() );
    if ( aRect.IsInside( aMousePos ) && !bPinned )
    {
        FadeOut();
        return FALSE;
    }
    else
        return SplitWindow::QueryDrop( rEvt );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() != 2 )
        SplitWindow::MouseButtonDown( rMEvt );
}

void SfxEmptySplitWin_Impl::MouseMove( const MouseEvent& rMEvt )
{
    SplitWindow::MouseMove( rMEvt );
}

//-------------------------------------------------------------------------

SfxSplitWindow::SfxSplitWindow( Window* pParent, SfxChildAlignment eAl,
        SfxWorkWindow *pW, BOOL bWithButtons, WinBits nBits )

/*  [Beschreibung]

    Ein SfxSplitWindow verbirgt die rekursive Struktur des SV-Splitwindows
    nach au\sen, indem es einen tabellenartigen Aufbau mit Zeilen und Spalten
    ( also maximale Rekursionstiefe 2 ) simuliert.
    Au\erdem sichert es die Persistenz der Anordnung der SfxDockingWindows.
*/

:   SplitWindow ( pParent, nBits | WB_HIDE ),
    eAlign(eAl),
    pWorkWin(pW),
    pDockArr( new SfxDockArr_Impl ),
    bLocked(FALSE),
    bPinned(FALSE),
    pEmptyWin(NULL),
    pActive(NULL)
{
    if ( bWithButtons )
    {
        ShowAutoHideButton( TRUE );
        ShowFadeOutButton( TRUE );
    }

    // SV-Alignment setzen
    WindowAlign eTbxAlign;
    switch ( eAlign )
    {
        case SFX_ALIGN_LEFT:
            eTbxAlign = WINDOWALIGN_LEFT;
            break;
        case SFX_ALIGN_RIGHT:
            eTbxAlign = WINDOWALIGN_RIGHT;
            break;
        case SFX_ALIGN_TOP:
            eTbxAlign = WINDOWALIGN_TOP;
            break;
        case SFX_ALIGN_BOTTOM:
            eTbxAlign = WINDOWALIGN_BOTTOM;
            bPinned = TRUE;
            break;
    }

    SetAlign (eTbxAlign);
    pEmptyWin = new SfxEmptySplitWin_Impl( this );
    if ( bPinned )
    {
        pEmptyWin->bFadeIn = TRUE;
        pEmptyWin->nState = 2;
    }

    if ( bWithButtons )
    {
        // Konfiguration einlesen
#if SUPD<613//MUSTINI
    #ifndef ENABLE_DIALOGCFG//MUSTINI
            SfxIniManager *pAppIniMgr = SFX_APP()->GetAppIniManager();
            if ( pAppIniMgr )
            {
                String aWinData( pAppIniMgr->Get( SFX_KEY_SPLITWINDOW, (USHORT) eTbxAlign ) );
                if ( aWinData.Len() && aWinData.GetChar( (USHORT) 0 ) == 'V' )
                {
                    pEmptyWin->nState = aWinData.GetToken( 1, ',' ).ToInt32();
                    if ( pEmptyWin->nState & 2 )
                        pEmptyWin->bFadeIn = TRUE;
                    bPinned = !( pEmptyWin->nState & 1 );

                    USHORT i=2;
                    USHORT nCount = (USHORT) aWinData.GetToken(i++, ',').ToInt32();
                    for ( USHORT n=0; n<nCount; n++ )
                    {
                        SfxDock_Impl *pDock = new SfxDock_Impl;
                        pDock->pWin = 0;
                        pDock->bNewLine = FALSE;
                        pDock->bHide = TRUE;
                        pDock->nType = (USHORT) aWinData.GetToken(i++, ',').ToInt32();
                        if ( !pDock->nType )
                        {
                            // K"onnte NewLine bedeuten
                            pDock->nType = (USHORT) aWinData.GetToken(i++, ',').ToInt32();
                            if ( !pDock->nType )
                            {
                                // Lesefehler
                                delete pDock;
                                break;
                            }
                            else
                                pDock->bNewLine = TRUE;
                        }

                        pDockArr->Insert(pDock,n);
                    }
                }
            }
    #else
        #ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
                DBG_ASSERT(sal_False, "SfxSplitWindow::SfxSplitWindow()\nThere exist no config item for splitwindow!\n");
        #endif
    #endif
#endif
    }
    else
    {
        bPinned = TRUE;
        pEmptyWin->bFadeIn = TRUE;
        pEmptyWin->nState = 2;
    }

    SetAutoHideState( !bPinned );
    pEmptyWin->SetAutoHideState( !bPinned );
}

//-------------------------------------------------------------------------

SfxSplitWindow::~SfxSplitWindow()
{
    if ( !pWorkWin->GetParent_Impl() )
    {
        // Konfiguration abspeichern
        String aWinData('V');
        aWinData += String::CreateFromInt32( VERSION );
        aWinData += ',';
        aWinData += String::CreateFromInt32( pEmptyWin->nState );
        aWinData += ',';

        USHORT nCount = 0;
        USHORT n;
        for ( n=0; n<pDockArr->Count(); n++ )
        {
            SfxDock_Impl *pDock = (*pDockArr)[n];
            if ( pDock->bHide )
                nCount++;
        }

        aWinData += String::CreateFromInt32( nCount );

        for ( n=0; n<pDockArr->Count(); n++ )
        {
            SfxDock_Impl *pDock = (*pDockArr)[n];
            if ( !pDock->bHide )
                continue;
            if ( pDock->bNewLine )
                aWinData += DEFINE_CONST_UNICODE(",0");
            aWinData += ',';
            aWinData += String::CreateFromInt32( pDock->nType);
        }
#if SUPD<613//MUSTINI
        SfxIniManager *pIniMgr = SFX_INIMANAGER();
        pIniMgr->Set( aWinData, SFX_KEY_SPLITWINDOW, GetAlign() );
#endif
    }

    if ( pEmptyWin )
    {
        // pOwner auf NULL setzen, sonst versucht pEmptyWin, nochmal zu
        // l"oschen; es wird n"amlich von au\sen immer das Fenster deleted,
        // das gerade angedockt ist
        pEmptyWin->pOwner = NULL;
        delete pEmptyWin;
    }

    delete pDockArr;
}

//-------------------------------------------------------------------------

void SfxSplitWindow::StartSplit()
{
    long nSize;
    Size aSize = GetSizePixel();

    if ( pEmptyWin )
    {
        pEmptyWin->bFadeIn = TRUE;
        pEmptyWin->bSplit = TRUE;
    }

    Rectangle aRect = pWorkWin->GetFreeArea( !bPinned );
    switch ( GetAlign() )
    {
        case WINDOWALIGN_LEFT:
        case WINDOWALIGN_RIGHT:
            nSize = aSize.Width() + aRect.GetWidth();
            break;
        case WINDOWALIGN_TOP:
        case WINDOWALIGN_BOTTOM:
            nSize = aSize.Height() + aRect.GetHeight();
            break;
    }

    SetMaxSizePixel( nSize );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::SplitResize()
{
    if ( bPinned )
    {
        pWorkWin->ArrangeChilds_Impl();
        pWorkWin->ShowChilds_Impl();
    }
    else
        pWorkWin->ArrangeAutoHideWindows( this );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::Split()
{
    if ( pEmptyWin )
        pEmptyWin->bSplit = FALSE;

    SplitWindow::Split();

    USHORT nCount = pDockArr->Count();
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pD = (*pDockArr)[n];
        if ( pD->pWin )
        {
            USHORT nId = pD->nType;
            long nSize    = GetItemSize( nId );
            long nSetSize = GetItemSize( GetSet( nId ) );
            Size aSize;

            if ( IsHorizontal() )
            {
                aSize.Width()  = nSize;
                aSize.Height() = nSetSize;
            }
            else
            {
                aSize.Width()  = nSetSize;
                aSize.Height() = nSize;
            }

            pD->pWin->SetItemSize_Impl( aSize );
        }
    }
}

//-------------------------------------------------------------------------

void SfxSplitWindow::InsertWindow( SfxDockingWindow* pDockWin, const Size& rSize)

/*  [Beschreibung]

    Zum Einf"ugen von SfxDockingWindows kann auch keine Position "ubergeben
    werden. Das SfxSplitWindow sucht dann die zuletzt gemerkte zu dem
    "ubergebenen SfxDockingWindow heraus oder h"angt es als letztes neu an.

*/
{
    short nLine = -1;       // damit erstes Fenster nLine auf 0 hochsetzen kann
    USHORT nL;
    USHORT nPos = 0;
    BOOL bNewLine = TRUE;

    SfxDock_Impl *pFoundDock=0;
    USHORT nCount = pDockArr->Count();
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pDock = (*pDockArr)[n];
        if ( pDock->bNewLine )
        {
            // Das Fenster er"offnet eine neue Zeile
            if ( pFoundDock )
                // Aber hinter dem gerade eingef"ugten Fenster
                break;

            // Neue Zeile
            nPos = 0;
            bNewLine = TRUE;
        }

        if ( pDock->pWin )
        {
            // Es gibt an dieser Stelle gerade ein Fenster
            if ( bNewLine && !pFoundDock )
            {
                // Bisher ist nicht bekannt, in welcher realen Zeile es liegt
                GetWindowPos( pDock->pWin, nL, nPos );
                nLine = (short) nL;
            }

            if ( !pFoundDock )
            {
                // Fenster liegt vor dem eingef"ugten
                nPos++;
            }

            // Zeile ist schon er"offnet
            bNewLine = FALSE;
            if ( pFoundDock )
                break;
        }

        if ( pDock->nType == pDockWin->GetType() )
        {
            DBG_ASSERT( !pFoundDock && !pDock->pWin, "Fenster ist schon vorhanden!");
            pFoundDock = pDock;
            if ( !bNewLine )
                break;
            else
            {
                // Es wurde zuletzt eine neue Reihe gestartet, aber noch kein
                // darin liegendes Fenster gefunden; daher weitersuchen, ob noch
                // ein Fenster in dieser Zeile folgt, um bNewLine korrekt zu setzen.
                // Dabei darf aber nLine oder nPos nicht mehr ver"andert werden!
                nLine++;
            }
        }
    }

    if ( !pFoundDock )
    {
        // Nicht gefunden, am Ende einf"ugen
        pFoundDock = new SfxDock_Impl;
        pFoundDock->bHide = TRUE;
        pDockArr->Insert( pFoundDock, nCount );
        pFoundDock->nType = pDockWin->GetType();
        nLine++;
        nPos = 0;
        bNewLine = TRUE;
        pFoundDock->bNewLine = bNewLine;
    }

    pFoundDock->pWin = pDockWin;
    pFoundDock->bHide = FALSE;
    InsertWindow_Impl( pFoundDock, rSize, nLine, nPos, bNewLine );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::ReleaseWindow_Impl(SfxDockingWindow *pDockWin)

/*  [Beschreibung]

    Das DockingWindow wird nicht mehr in den internen Daten gespeichert.
*/

{
    SfxDock_Impl *pDock=0;
    USHORT nCount = pDockArr->Count();
    BOOL bFound = FALSE;
    for ( USHORT n=0; n<nCount; n++ )
    {
        pDock = (*pDockArr)[n];
        if ( pDock->nType == pDockWin->GetType() )
        {
            if ( pDock->bNewLine && n<nCount-1 )
                (*pDockArr)[n+1]->bNewLine = TRUE;

            // Fenster hat schon eine Position, die vergessen wir
            bFound = TRUE;
            pDockArr->Remove(n);
            break;
        }
    }

    if ( bFound )
        delete pDock;
}

//-------------------------------------------------------------------------

void SfxSplitWindow::MoveWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        USHORT nLine, USHORT nPos, BOOL bNewLine)

/*  [Beschreibung]

    Das DockingWindow wird innerhalb des Splitwindows verschoben.

*/

{
    USHORT nL, nP;
    GetWindowPos( pDockWin, nL, nP );

    if ( nLine > nL && GetItemCount( GetItemId( nL, 0 ) ) == 1 )
    {
        // Wenn das letzte Fenster aus seiner Zeile entfernt wird, rutscht
        // alles eine Zeile nach vorne!
        nLine--;
    }
    else if ( nLine == nL && nPos > nP )
    {
        nPos--;
    }

    RemoveWindow( pDockWin );
    InsertWindow( pDockWin, rSize, nLine, nPos, bNewLine );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::InsertWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        USHORT nLine, USHORT nPos, BOOL bNewLine)

/*  [Beschreibung]

    Das DockingWindow wird in dieses Splitwindow geschoben und soll die
    "ubergebene Position und Gr"o\se haben.

*/
{
    ReleaseWindow_Impl( pDockWin );
    SfxDock_Impl *pDock = new SfxDock_Impl;
    pDock->bHide = FALSE;
    pDock->nType = pDockWin->GetType();
    pDock->bNewLine = bNewLine;
    pDock->pWin = pDockWin;

    DBG_ASSERT( nPos==0 || !bNewLine, "Falsche Paramenter!");
    if ( bNewLine )
        nPos = 0;

    // Das Fenster mu\s vor dem ersten Fenster eingef"ugt werden, das die
    // gleiche oder eine gr"o\sere Position hat als pDockWin.
    USHORT nCount = pDockArr->Count();

    // Wenn gar kein Fenster gefunden wird, wird als erstes eingef"ugt
    USHORT nInsertPos = 0;
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pD = (*pDockArr)[n];

        if (pD->pWin)
        {
            // Ein angedocktes Fenster wurde gefunden
            // Wenn kein geeignetes Fenster hinter der gew"unschten Einf"ugeposition
            // gefunden wird, wird am Ende eingef"ugt
            nInsertPos = nCount;
            USHORT nL=0, nP=0;
            GetWindowPos( pD->pWin, nL, nP );

            if ( nL == nLine && nP == nPos || nL > nLine )
            {
                DBG_ASSERT( nL == nLine || bNewLine || nPos > 0, "Falsche Parameter!" );
                if ( nL == nLine && nPos == 0 && !bNewLine )
                {
                    DBG_ASSERT(pD->bNewLine, "Keine neue Zeile?");

                    // Das Fenster wird auf nPos==0 eingeschoben
                    pD->bNewLine = FALSE;
                    pDock->bNewLine = TRUE;
                }

                nInsertPos = n;
                break;
            }
        }
    }

    pDockArr->Insert(pDock, nInsertPos);
    InsertWindow_Impl( pDock, rSize, nLine, nPos, bNewLine );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::InsertWindow_Impl( SfxDock_Impl* pDock,
                        const Size& rSize,
                        USHORT nLine, USHORT nPos, BOOL bNewLine)

/*  [Beschreibung]

    F"ugt ein DockingWindow ein und veranla\st die Neuberechnung der Gr"o\se
    des Splitwindows.
*/

{
    SfxDockingWindow* pDockWin = pDock->pWin;

    USHORT nItemBits = pDockWin->GetWinBits_Impl();

    long nWinSize, nSetSize;
    if ( IsHorizontal() )
    {
        nWinSize = rSize.Width();
        nSetSize = rSize.Height();
    }
    else
    {
        nSetSize = rSize.Width();
        nWinSize = rSize.Height();
    }

    pDock->nSize = nWinSize;

    BOOL bUpdateMode = IsUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( FALSE );

    if ( bNewLine || nLine == GetItemCount( 0 ) )
    {
        // Es soll nicht in eine vorhandene Zeile eingef"ugt werden, sondern
        // eine neue erzeugt werden

        USHORT nId = 1;
        for ( USHORT n=0; n<GetItemCount(0); n++ )
        {
            if ( GetItemId(n) >= nId )
                nId = GetItemId(n)+1;
        }

        // Eine neue nLine-te Zeile erzeugen
        USHORT nBits = nItemBits;
        if ( GetAlign() == WINDOWALIGN_TOP || GetAlign() == WINDOWALIGN_BOTTOM )
            nBits |= SWIB_COLSET;
        InsertItem( nId, nSetSize, nLine, 0, nBits );
    }

    // In Zeile mit Position nLine das Fenster einf"ugen
    // ItemWindowSize auf "Prozentual" setzen, da SV dann das Umgr"o\sern
    // so macht, wie man erwartet; "Pixel" macht eigentlich nur Sinn, wenn
    // auch Items mit prozentualen oder relativen Gr"o\sen dabei sind.
    nItemBits |= SWIB_PERCENTSIZE;
    bLocked = TRUE;
    USHORT nSet = GetItemId( nLine );
    InsertItem( pDockWin->GetType(), pDockWin, nWinSize, nPos, nSet, nItemBits );

    // Splitwindows werden im SFX einmal angelegt und beim Einf"ugen des ersten
    // DockingWindows sichtbar gemacht.
    if ( GetItemCount( 0 ) == 1 && GetItemCount( 1 ) == 1 )
    {
        // Das Neuarrangieren am WorkWindow und ein Show() auf das SplitWindow
        // wird vom SfxDockingwindow veranla\st (->SfxWorkWindow::ConfigChild_Impl)
        if ( !bPinned && !IsFloatingMode() )
        {
            bPinned = TRUE;
            BOOL bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = FALSE;
            SetPinned_Impl( FALSE );
            pEmptyWin->Actualize();
            pWorkWin->RegisterChild_Impl( *GetSplitWindow(), eAlign, TRUE )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            if ( bFadeIn )
                FadeIn();
        }
        else
        {
            BOOL bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = FALSE;
            pEmptyWin->Actualize();
            pWorkWin->RegisterChild_Impl( *GetSplitWindow(), eAlign, TRUE )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            if ( bFadeIn )
                FadeIn();
        }

        pWorkWin->ShowChilds_Impl();
    }

    if ( bUpdateMode )
        SetUpdateMode( TRUE );
    bLocked = FALSE;
}

//-------------------------------------------------------------------------

void SfxSplitWindow::RemoveWindow( SfxDockingWindow* pDockWin, BOOL bHide )

/*  [Beschreibung]

    Entfernt ein DockingWindow. Wenn es das letzte war, wird das SplitWindow
    gehidet.
*/
{
    USHORT nSet = GetSet( pDockWin->GetType() );

    // Splitwindows werden im SFX einmal angelegt und nach dem Entfernen
    // des letzten DockingWindows unsichtbar gemacht.
    if ( GetItemCount( nSet ) == 1 && GetItemCount( 0 ) == 1 )
    {
        // Das Neuarrangieren am WorkWindow wird vom SfxDockingwindow
        // veranla\st!
        Hide();
        pEmptyWin->aTimer.Stop();
        long nRealState = pEmptyWin->nState;
        FadeOut();
        pEmptyWin->Hide();
        pWorkWin->ReleaseChild_Impl( *GetSplitWindow() );
        pEmptyWin->nState = nRealState;
        pWorkWin->ArrangeAutoHideWindows( this );
    }

    SfxDock_Impl *pDock=0;
    USHORT nCount = pDockArr->Count();
    BOOL bFound = FALSE;
    for ( USHORT n=0; n<nCount; n++ )
    {
        pDock = (*pDockArr)[n];
        if ( pDock->nType == pDockWin->GetType() )
        {
            pDock->pWin = 0;
            pDock->bHide = bHide;
            break;
        }
    }

    // Fenster removen, und wenn es das letzte der Zeile war, auch die Zeile
    // ( Zeile = ItemSet )
    BOOL bUpdateMode = IsUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( FALSE );
    bLocked = TRUE;

    RemoveItem( pDockWin->GetType() );

    if ( nSet && !GetItemCount( nSet ) )
        RemoveItem( nSet );

    if ( bUpdateMode )
        SetUpdateMode( TRUE );
    bLocked = FALSE;
};

//-------------------------------------------------------------------------

BOOL SfxSplitWindow::GetWindowPos( const SfxDockingWindow* pWindow,
                                        USHORT& rLine, USHORT& rPos ) const
/*  [Beschreibung]

    Liefert die Id des Itemsets und die des Items f"ur das "ubergebene
    DockingWindow in der alten Zeilen/Spalten-Bezeichnung zur"uck.
*/

{
    USHORT nSet = GetSet ( pWindow->GetType() );
    if ( nSet == SPLITWINDOW_ITEM_NOTFOUND )
        return FALSE;

    rPos  = GetItemPos( pWindow->GetType(), nSet );
    rLine = GetItemPos( nSet );
    return TRUE;
}

//-------------------------------------------------------------------------

BOOL SfxSplitWindow::GetWindowPos( const Point& rTestPos,
                                      USHORT& rLine, USHORT& rPos ) const
/*  [Beschreibung]

    Liefert die Id des Itemsets und die des Items f"ur das DockingWindow
    an der "ubergebenen Position in der alten Zeilen/Spalten-Bezeichnung
    zur"uck.
*/

{
    USHORT nId = GetItemId( rTestPos );
    if ( nId == 0 )
        return FALSE;

    USHORT nSet = GetSet ( nId );
    rPos  = GetItemPos( nId, nSet );
    rLine = GetItemPos( nSet );
    return TRUE;
}

//-------------------------------------------------------------------------

USHORT SfxSplitWindow::GetLineCount() const

/*  [Beschreibung]

    Liefert die Zeilenzahl = Zahl der Sub-Itemsets im Root-Set.
*/
{
    return GetItemCount( 0 );
}

//-------------------------------------------------------------------------

long SfxSplitWindow::GetLineSize( USHORT nLine ) const

/*  [Beschreibung]

    Liefert die "Zeilenh"ohe" des nLine-ten Itemsets.
*/
{
    USHORT nId = GetItemId( nLine );
    return GetItemSize( nId );
}

//-------------------------------------------------------------------------

USHORT SfxSplitWindow::GetWindowCount( USHORT nLine ) const

/*  [Beschreibung]

    Liefert die
*/
{
    USHORT nId = GetItemId( nLine );
    return GetItemCount( nId );
}

//-------------------------------------------------------------------------

USHORT SfxSplitWindow::GetWindowCount() const

/*  [Beschreibung]

    Liefert die Gesamtzahl aller Fenstert
*/
{
    return GetItemCount( 0 );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::Command( const CommandEvent& rCEvt )
{
#ifdef TF_AUTOSHOW_ON_MOUSEMOVE
    if ( rCEvt.GetCommand() != COMMAND_CONTEXTMENU )
        return;

    SfxPopupMenuManager aPop( SfxResId( RID_AUTOHIDE ), pWorkWin->GetBindings() );
    aPop.StartInsert();

    if ( !bPinned )
    {
        aPop.RemoveItem( 0 );
        String aText( SfxResId(RID_AUTOHIDE ) );
        aPop.InsertItem( SID_AUTOHIDE, aText, MIB_CHECKABLE ) ;
    }

    aPop.InsertSeparator();

    USHORT nCount = pDockArr->Count();
    SfxSlotPool &rSlotPool = SFX_SLOTPOOL();
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pD = (*pDockArr)[n];
        if ( pD->pWin || pD->bHide )
        {
            String aHelpText;
            String aMenuText = rSlotPool.GetSlotName_Impl( pD->nType, &aHelpText );
            aPop.InsertItem( pD->nType, aMenuText, MIB_CHECKABLE );
        }
    }

    aPop.EndInsert();

    if ( aPop.Execute( GetPointerPosPixel() ) == SID_AUTOHIDE )
    {
        // ChildWindows neu anordnen
        SetPinned_Impl( !bPinned );
        if ( bPinned && pWorkWin->IsAutoHideMode( this ) )
            pWorkWin->ArrangeAutoHideWindows( this );
        pWorkWin->ArrangeChilds_Impl();
        pWorkWin->ShowChilds_Impl();
    }
#else
    SplitWindow::Command( rCEvt );
#endif
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxSplitWindow, TimerHdl, Timer*, pTimer)
{
    if ( pTimer )
        pTimer->Stop();

    if ( CursorIsOverRect( FALSE ) || !pTimer )
    {
        // Wenn der Mauszeiger innerhalb des Fensters liegt, SplitWindow anzeigen
        // und Timer zum Schlie\sen aufsetzen
        pEmptyWin->bAutoHide = TRUE;
        if ( !IsVisible() )
            pEmptyWin->FadeIn();

        pEmptyWin->aLastPos = GetPointerPosPixel();
        pEmptyWin->aTimer.Start();
    }
    else if ( pEmptyWin->bAutoHide )
    {
        if ( GetPointerPosPixel() != pEmptyWin->aLastPos )
        {
            // Die Maus wurd innerhalb der Timerlaugzeit bewegt, also erst einmal
            // nichts tun
            pEmptyWin->aLastPos = GetPointerPosPixel();
            pEmptyWin->aTimer.Start();
            return 0L;
        }

        // Speziell f"ur TF_AUTOSHOW_ON_MOUSEMOVE :
        // Wenn das Fenster nicht sichtbar ist, gibt es nichts zu tun
        // (Benutzer ist einfach mit der Maus "uber pEmptyWin gefahren)
        if ( IsVisible() )
        {
            pEmptyWin->bEndAutoHide = FALSE;
            if ( !Application::IsInModalMode() &&
                  !PopupMenu::IsInExecute() &&
                  !pEmptyWin->bSplit && !HasChildPathFocus( TRUE ) )
            {
                // W"ahrend ein modaler Dialog oder ein Popupmenu offen sind
                // oder w"ahrend des Splittens auf keinen Fall zumachen; auch
                // solange eines der Children den Focus hat, bleibt das
                // das Fenster offen
                pEmptyWin->bEndAutoHide = TRUE;
            }

            if ( pEmptyWin->bEndAutoHide )
            {
                // Von mir aus kann Schlu\s sein mit AutoShow
                // Aber vielleicht will noch ein anderes SfxSplitWindow offen bleiben,
                // dann bleiben auch alle anderen offen
                if ( !pWorkWin->IsAutoHideMode( this ) )
                {
                    FadeOut();
                    pWorkWin->ArrangeAutoHideWindows( this );
                }
                else
                {
                    pEmptyWin->aLastPos = GetPointerPosPixel();
                    pEmptyWin->aTimer.Start();
                }
            }
            else
            {
                pEmptyWin->aLastPos = GetPointerPosPixel();
                pEmptyWin->aTimer.Start();
            }
        }
    }

    return 0L;
}

//-------------------------------------------------------------------------

BOOL SfxSplitWindow::CursorIsOverRect( BOOL bForceAdding ) const
{
    BOOL bVisible = IsVisible();

    // Auch das kollabierte SplitWindow ber"ucksichtigen
    Point aPos = pEmptyWin->GetParent()->OutputToScreenPixel( pEmptyWin->GetPosPixel() );
    Size aSize = pEmptyWin->GetSizePixel();

    if ( bForceAdding )
    {
        // Um +/- ein paar Pixel erweitern, sonst ist es zu nerv"os
        aPos.X() -= nPixel;
        aPos.Y() -= nPixel;
        aSize.Width() += 2 * nPixel;
        aSize.Height() += 2 * nPixel;
    }

    Rectangle aRect( aPos, aSize );

    if ( bVisible )
    {
        Point aPos = GetPosPixel();
        Size aSize = GetSizePixel();

        // Um +/- ein paar Pixel erweitern, sonst ist es zu nerv"os
        aPos.X() -= nPixel;
        aPos.Y() -= nPixel;
        aSize.Width() += 2 * nPixel;
        aSize.Height() += 2 * nPixel;

        Rectangle aVisRect( aPos, aSize );
        aRect = aRect.GetUnion( aVisRect );
    }

    if ( aRect.IsInside( OutputToScreenPixel( ((Window*)this)->GetPointerPosPixel() ) ) )
        return TRUE;
    return FALSE;
}

//-------------------------------------------------------------------------

SplitWindow* SfxSplitWindow::GetSplitWindow()
{
    if ( !bPinned || !pEmptyWin->bFadeIn )
        return pEmptyWin;
    return this;
}

//-------------------------------------------------------------------------
BOOL SfxSplitWindow::IsFadeIn() const
{
    return pEmptyWin->bFadeIn;
}

BOOL SfxSplitWindow::IsAutoHide( BOOL bSelf ) const
{
    return bSelf ? pEmptyWin->bAutoHide && !pEmptyWin->bEndAutoHide : pEmptyWin->bAutoHide;
}

//-------------------------------------------------------------------------

void SfxSplitWindow::SetPinned_Impl( BOOL bOn )
{
    if ( bPinned == bOn )
        return;

    bPinned = bOn;
    if ( GetItemCount( 0 ) == 0 )
        return;

    if ( !bOn )
    {
        pEmptyWin->nState |= 1;
        if ( pEmptyWin->bFadeIn )
        {
            // Ersatzfenster anmelden
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            pWorkWin->RegisterChild_Impl( *pEmptyWin, eAlign, TRUE )->nVisible = CHILD_VISIBLE;
        }

        Point aPos( GetPosPixel() );
        aPos = GetParent()->OutputToScreenPixel( aPos );
        SetFloatingPos( aPos );
        SetFloatingMode( TRUE );
        GetFloatingWindow()->SetOutputSizePixel( GetOutputSizePixel() );

        if ( pEmptyWin->bFadeIn )
            Show();
    }
    else
    {
        pEmptyWin->nState &= ~1;
        SetOutputSizePixel( GetFloatingWindow()->GetOutputSizePixel() );
        SetFloatingMode( FALSE );

        if ( pEmptyWin->bFadeIn )
        {
            // Ersatzfenster abmelden
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            pWorkWin->RegisterChild_Impl( *this, eAlign, TRUE )->nVisible = CHILD_VISIBLE;
        }
    }

    SetAutoHideState( !bPinned );
    pEmptyWin->SetAutoHideState( !bPinned );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::SetFadeIn_Impl( BOOL bOn )
{
    if ( bOn == pEmptyWin->bFadeIn )
        return;

    if ( GetItemCount( 0 ) == 0 )
        return;

    pEmptyWin->bFadeIn = bOn;
    if ( bOn )
    {
        pEmptyWin->nState |= 2;
        if ( IsFloatingMode() )
        {
            // FloatingWindow ist nicht sichtbar, also anzeigen
            pWorkWin->ArrangeAutoHideWindows( this );
            Show();
        }
        else
        {
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            pWorkWin->RegisterChild_Impl( *this, eAlign, TRUE )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            pWorkWin->ShowChilds_Impl();
        }
    }
    else
    {
        pEmptyWin->bAutoHide = FALSE;
        pEmptyWin->nState &= ~2;
        if ( !IsFloatingMode() )
        {
            // Das Fenster "schwebt" nicht, soll aber ausgeblendet werden,
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            pWorkWin->RegisterChild_Impl( *pEmptyWin, eAlign, TRUE )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            pWorkWin->ShowChilds_Impl();
            pWorkWin->ArrangeAutoHideWindows( this );
        }
        else
        {
            Hide();
            pWorkWin->ArrangeAutoHideWindows( this );
        }
    }
}

void SfxSplitWindow::EndAutoShow_Impl()
{
    SfxWorkWindow *pWorkWin = SFX_APP()->GetWorkWindow_Impl(SfxViewFrame::Current());
    pWorkWin->EndAutoShow_Impl();
}

void SfxSplitWindow::AutoHide()
{
    // Wenn dieser Handler am "echten" SplitWindow aufgerufen wird, ist es
    // entweder angedockt und soll "schwebend" angezeigt werden oder umgekehrt
    if ( !bPinned )
    {
        // Es "schwebt", also wieder andocken
        SetPinned_Impl( TRUE );
        pWorkWin->ArrangeChilds_Impl();
    }
    else
    {
        // In den "Schwebezustand" bringen
        SetPinned_Impl( FALSE );
        pWorkWin->ArrangeChilds_Impl();
        pWorkWin->ArrangeAutoHideWindows( this );
    }

    pWorkWin->ShowChilds_Impl();
}

void SfxSplitWindow::FadeOut()
{
    SetFadeIn_Impl( FALSE );
    Show_Impl();
}

void SfxSplitWindow::FadeIn()
{
    SetFadeIn_Impl( TRUE );
    Show_Impl();
}

void SfxSplitWindow::Show_Impl()
{
    USHORT nCount = pDockArr->Count();
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pDock = (*pDockArr)[n];
        if ( pDock->pWin )
            pDock->pWin->FadeIn( pEmptyWin->bFadeIn );
    }
}

void SfxSplitWindow::Pin_Impl( BOOL bPin )
{
    if ( bPinned != bPin )
        AutoHide();
}

BOOL SfxSplitWindow::ActivateNextChild_Impl( BOOL bForward )
{
    // Wenn kein pActive, auf erstes bzw. letztes Fenster gehen ( bei !bForward wird erst in der loop dekrementiert )
    USHORT nCount = pDockArr->Count();
    USHORT n = bForward ? 0 : nCount;

    // Wenn Focus innerhalb, dann ein Fenster vor oder zur"uck, wenn m"oglich
    if ( pActive )
    {
        // Aktives Fenster ermitteln
        for ( n=0; n<nCount; n++ )
        {
            SfxDock_Impl *pD = (*pDockArr)[n];
            if ( pD->pWin && pD->pWin->HasChildPathFocus() )
                break;
        }

        if ( bForward )
            // ein Fenster weiter ( wenn dann n>nCount, wird die Schleife unten gar nicht durchlaufen )
            n++;
    }

    if ( bForward )
    {
        // N"achstes Fenster suchen
        for ( USHORT nNext=n; nNext<nCount; nNext++ )
        {
            SfxDock_Impl *pD = (*pDockArr)[nNext];
            if ( pD->pWin )
            {
                pD->pWin->GrabFocus();
                return TRUE;
            }
        }
    }
    else
    {
        // Vorheriges Fenster suchen
        for ( USHORT nNext=n; nNext--; )
        {
            SfxDock_Impl *pD = (*pDockArr)[nNext];
            if ( pD->pWin )
            {
                pD->pWin->GrabFocus();
                return TRUE;
            }
        }
    }

    return FALSE;
}

void SfxSplitWindow::SetActiveWindow_Impl( SfxDockingWindow* pWin )
{
    pActive = pWin;
    pWorkWin->SetActiveChild_Impl( this );
}


