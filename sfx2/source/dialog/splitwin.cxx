/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifdef SOLARIS
#include <ctime>
#endif

#include <string>

#include <vcl/wrkwin.hxx>
#include <unotools/viewoptions.hxx>

#include <vcl/timer.hxx>

#include "splitwin.hxx"
#include "workwin.hxx"
#include <sfx2/dockwin.hxx>
#include <sfx2/app.hxx>
#include "dialog.hrc"
#include "sfx2/sfxresid.hxx"
#include <sfx2/mnumgr.hxx>
#include "virtmenu.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define VERSION 1
#define nPixel  30L
#define USERITEM_NAME           OUString(RTL_CONSTASCII_USTRINGPARAM("UserItem"))

struct SfxDock_Impl
{
    sal_uInt16              nType;
    SfxDockingWindow*   pWin;           // SplitWindow hat dieses Fenster
    sal_Bool                bNewLine;
    sal_Bool                bHide;          // SplitWindow hatte dieses Fenster
    long                nSize;
};

typedef SfxDock_Impl* SfxDockPtr;
SV_DECL_PTRARR_DEL( SfxDockArr_Impl, SfxDockPtr, 4, 4)
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
    sal_Bool                bFadeIn;
    sal_Bool                bAutoHide;
    sal_Bool                bSplit;
    sal_Bool                bEndAutoHide;
    Timer               aTimer;
    Point               aLastPos;
    sal_uInt16              nState;

                        SfxEmptySplitWin_Impl( SfxSplitWindow *pParent )
                            : SplitWindow( pParent->GetParent(), WinBits( WB_BORDER | WB_3DLOOK ) )
                            , pOwner( pParent )
                            , bFadeIn( sal_False )
                            , bAutoHide( sal_False )
                            , bSplit( sal_False )
                            , bEndAutoHide( sal_False )
                            , nState( 1 )
                        {
                            aTimer.SetTimeoutHdl(
                                LINK(pOwner, SfxSplitWindow, TimerHdl ) );
                            aTimer.SetTimeout( 200 );
                            SetAlign( pOwner->GetAlign() );
                            Actualize();
                            ShowAutoHideButton( pOwner->IsAutoHideButtonVisible() );
                            ShowFadeInHideButton( sal_True );
                        }

                        ~SfxEmptySplitWin_Impl()
                        {
                            aTimer.Stop();
                        }

    virtual void        MouseMove( const MouseEvent& );
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
    pOwner->SaveConfig_Impl();
    bAutoHide = sal_True;
    FadeIn();
}

void SfxEmptySplitWin_Impl::FadeIn()
{
    if (!bAutoHide )
        bAutoHide = IsFadeNoButtonMode();
    pOwner->SetFadeIn_Impl( sal_True );
    pOwner->Show_Impl();
    if ( bAutoHide )
    {
        // Timer zum Schlie\sen aufsetzen; der Aufrufer mu\s selbst sicherstellen,
        // da\s das Window nicht gleich wieder zu geht ( z.B. durch Setzen des
        // Focus oder einen modal mode )
        aLastPos = GetPointerPosPixel();
        aTimer.Start();
    }
    else
        pOwner->SaveConfig_Impl();
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
        SfxWorkWindow *pW, sal_Bool bWithButtons, WinBits nBits )

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
    bLocked(sal_False),
    bPinned(sal_True),
    pEmptyWin(NULL),
    pActive(NULL)
{
    if ( bWithButtons )
    {
        ShowAutoHideButton( sal_False );    // no autohide button (pin) anymore
        ShowFadeOutButton( sal_True );
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
            bPinned = sal_True;
            break;
        default:
            eTbxAlign = WINDOWALIGN_TOP;  // some sort of default...
            break;  // -Wall lots not handled..
    }

    SetAlign (eTbxAlign);
    pEmptyWin = new SfxEmptySplitWin_Impl( this );
    if ( bPinned )
    {
        pEmptyWin->bFadeIn = sal_True;
        pEmptyWin->nState = 2;
    }

    if ( bWithButtons )
    {
        // Konfiguration einlesen
        String aWindowId = String::CreateFromAscii("SplitWindow");
        aWindowId += String::CreateFromInt32( (sal_Int32) eTbxAlign );
        SvtViewOptions aWinOpt( E_WINDOW, aWindowId );
        String aWinData;
        Any aUserItem = aWinOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aWinData = String( aTemp );
        if ( aWinData.Len() && aWinData.GetChar( (sal_uInt16) 0 ) == 'V' )
        {
            pEmptyWin->nState = (sal_uInt16) aWinData.GetToken( 1, ',' ).ToInt32();
            if ( pEmptyWin->nState & 2 )
                pEmptyWin->bFadeIn = sal_True;
            bPinned = sal_True; // always assume pinned - floating mode not used anymore

            sal_uInt16 i=2;
            sal_uInt16 nCount = (sal_uInt16) aWinData.GetToken(i++, ',').ToInt32();
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                SfxDock_Impl *pDock = new SfxDock_Impl;
                pDock->pWin = 0;
                pDock->bNewLine = sal_False;
                pDock->bHide = sal_True;
                pDock->nType = (sal_uInt16) aWinData.GetToken(i++, ',').ToInt32();
                if ( !pDock->nType )
                {
                    // K"onnte NewLine bedeuten
                    pDock->nType = (sal_uInt16) aWinData.GetToken(i++, ',').ToInt32();
                    if ( !pDock->nType )
                    {
                        // Lesefehler
                        delete pDock;
                        break;
                    }
                    else
                        pDock->bNewLine = sal_True;
                }

                pDockArr->Insert(pDock,n);
            }
        }
    }
    else
    {
        bPinned = sal_True;
        pEmptyWin->bFadeIn = sal_True;
        pEmptyWin->nState = 2;
    }

    SetAutoHideState( !bPinned );
    pEmptyWin->SetAutoHideState( !bPinned );
}

//-------------------------------------------------------------------------

SfxSplitWindow::~SfxSplitWindow()
{
    if ( !pWorkWin->GetParent_Impl() )
        SaveConfig_Impl();

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

void SfxSplitWindow::SaveConfig_Impl()
{
    // Konfiguration abspeichern
    String aWinData('V');
    aWinData += String::CreateFromInt32( VERSION );
    aWinData += ',';
    aWinData += String::CreateFromInt32( pEmptyWin->nState );
    aWinData += ',';

    sal_uInt16 nCount = 0;
    sal_uInt16 n;
    for ( n=0; n<pDockArr->Count(); n++ )
    {
        SfxDock_Impl *pDock = (*pDockArr)[n];
        if ( pDock->bHide || pDock->pWin )
            nCount++;
    }

    aWinData += String::CreateFromInt32( nCount );

    for ( n=0; n<pDockArr->Count(); n++ )
    {
        SfxDock_Impl *pDock = (*pDockArr)[n];
        if ( !pDock->bHide && !pDock->pWin )
            continue;
        if ( pDock->bNewLine )
            aWinData += DEFINE_CONST_UNICODE(",0");
        aWinData += ',';
        aWinData += String::CreateFromInt32( pDock->nType);
    }

    String aWindowId = String::CreateFromAscii("SplitWindow");
    aWindowId += String::CreateFromInt32( (sal_Int32) GetAlign() );
    SvtViewOptions aWinOpt( E_WINDOW, aWindowId );
    aWinOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aWinData ) ) );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::StartSplit()
{
    long nSize = 0;
    Size aSize = GetSizePixel();

    if ( pEmptyWin )
    {
        pEmptyWin->bFadeIn = sal_True;
        pEmptyWin->bSplit = sal_True;
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
        pEmptyWin->bSplit = sal_False;

    SplitWindow::Split();

    sal_uInt16 nCount = pDockArr->Count();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pD = (*pDockArr)[n];
        if ( pD->pWin )
        {
            sal_uInt16 nId = pD->nType;
            long nSize    = GetItemSize( nId, SWIB_FIXED );
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

    SaveConfig_Impl();
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
    sal_uInt16 nL;
    sal_uInt16 nPos = 0;
    sal_Bool bNewLine = sal_True;
    sal_Bool bSaveConfig = sal_False;
    SfxDock_Impl *pFoundDock=0;
    sal_uInt16 nCount = pDockArr->Count();
    for ( sal_uInt16 n=0; n<nCount; n++ )
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
            bNewLine = sal_True;
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
            bNewLine = sal_False;
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
        pFoundDock->bHide = sal_True;
        pDockArr->Insert( pFoundDock, nCount );
        pFoundDock->nType = pDockWin->GetType();
        nLine++;
        nPos = 0;
        bNewLine = sal_True;
        pFoundDock->bNewLine = bNewLine;
        bSaveConfig = sal_True;
    }

    pFoundDock->pWin = pDockWin;
    pFoundDock->bHide = sal_False;
    InsertWindow_Impl( pFoundDock, rSize, nLine, nPos, bNewLine );
    if ( bSaveConfig )
        SaveConfig_Impl();
}

//-------------------------------------------------------------------------

void SfxSplitWindow::ReleaseWindow_Impl(SfxDockingWindow *pDockWin, sal_Bool bSave)

/*  [Beschreibung]

    Das DockingWindow wird nicht mehr in den internen Daten gespeichert.
*/

{
    SfxDock_Impl *pDock=0;
    sal_uInt16 nCount = pDockArr->Count();
    sal_Bool bFound = sal_False;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        pDock = (*pDockArr)[n];
        if ( pDock->nType == pDockWin->GetType() )
        {
            if ( pDock->bNewLine && n<nCount-1 )
                (*pDockArr)[n+1]->bNewLine = sal_True;

            // Fenster hat schon eine Position, die vergessen wir
            bFound = sal_True;
            pDockArr->Remove(n);
            break;
        }
    }

    if ( bFound )
        delete pDock;

    if ( bSave )
        SaveConfig_Impl();
}

//-------------------------------------------------------------------------

void SfxSplitWindow::MoveWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        sal_uInt16 nLine, sal_uInt16 nPos, sal_Bool bNewLine)

/*  [Beschreibung]

    Das DockingWindow wird innerhalb des Splitwindows verschoben.

*/

{
    sal_uInt16 nL, nP;
    GetWindowPos( pDockWin, nL, nP );

    if ( nLine > nL && GetItemCount( GetItemId( nL, 0 ) ) == 1 )
    {
        // Wenn das letzte Fenster aus seiner Zeile entfernt wird, rutscht
        // alles eine Zeile nach vorne!
        nLine--;
    }
    RemoveWindow( pDockWin );
    InsertWindow( pDockWin, rSize, nLine, nPos, bNewLine );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::InsertWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        sal_uInt16 nLine, sal_uInt16 nPos, sal_Bool bNewLine)

/*  [Beschreibung]

    Das DockingWindow wird in dieses Splitwindow geschoben und soll die
    "ubergebene Position und Gr"o\se haben.

*/
{
    ReleaseWindow_Impl( pDockWin, sal_False );
    SfxDock_Impl *pDock = new SfxDock_Impl;
    pDock->bHide = sal_False;
    pDock->nType = pDockWin->GetType();
    pDock->bNewLine = bNewLine;
    pDock->pWin = pDockWin;

    DBG_ASSERT( nPos==0 || !bNewLine, "Falsche Paramenter!");
    if ( bNewLine )
        nPos = 0;

    // Das Fenster mu\s vor dem ersten Fenster eingef"ugt werden, das die
    // gleiche oder eine gr"o\sere Position hat als pDockWin.
    sal_uInt16 nCount = pDockArr->Count();
    USHORT nLastWindowIdx(0);

    // Wenn gar kein Fenster gefunden wird, wird als erstes eingef"ugt
    sal_uInt16 nInsertPos = 0;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pD = (*pDockArr)[n];

        if (pD->pWin)
        {
            // Ein angedocktes Fenster wurde gefunden
            // Wenn kein geeignetes Fenster hinter der gew"unschten Einf"ugeposition
            // gefunden wird, wird am Ende eingef"ugt
            nInsertPos = nCount;
            nLastWindowIdx = n;
            sal_uInt16 nL=0, nP=0;
            GetWindowPos( pD->pWin, nL, nP );

            if ( (nL == nLine && nP == nPos) || nL > nLine )
            {
                DBG_ASSERT( nL == nLine || bNewLine || nPos > 0, "Falsche Parameter!" );
                if ( nL == nLine && nPos == 0 && !bNewLine )
                {
                    DBG_ASSERT(pD->bNewLine, "Keine neue Zeile?");

                    // Das Fenster wird auf nPos==0 eingeschoben
                    pD->bNewLine = sal_False;
                    pDock->bNewLine = sal_True;
                }

                nInsertPos = n != 0 ? nLastWindowIdx + 1 : 0;    // ignore all non-windows after the last window
                break;
            }
        }
    }
    if (nCount != 0 && nInsertPos == nCount && nLastWindowIdx != nCount - 1)
    {
        nInsertPos = nLastWindowIdx + 1;    // ignore all non-windows after the last window
    }

    pDockArr->Insert(pDock, nInsertPos);
    InsertWindow_Impl( pDock, rSize, nLine, nPos, bNewLine );
    SaveConfig_Impl();
}

//-------------------------------------------------------------------------

void SfxSplitWindow::InsertWindow_Impl( SfxDock_Impl* pDock,
                        const Size& rSize,
                        sal_uInt16 nLine, sal_uInt16 nPos, sal_Bool bNewLine)

/*  [Beschreibung]

    F"ugt ein DockingWindow ein und veranla\st die Neuberechnung der Gr"o\se
    des Splitwindows.
*/

{
    SfxDockingWindow* pDockWin = pDock->pWin;

    sal_uInt16 nItemBits = pDockWin->GetWinBits_Impl();

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

    sal_Bool bUpdateMode = IsUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( sal_False );

    if ( bNewLine || nLine == GetItemCount( 0 ) )
    {
        // Es soll nicht in eine vorhandene Zeile eingef"ugt werden, sondern
        // eine neue erzeugt werden

        sal_uInt16 nId = 1;
        for ( sal_uInt16 n=0; n<GetItemCount(0); n++ )
        {
            if ( GetItemId(n) >= nId )
                nId = GetItemId(n)+1;
        }

        // Eine neue nLine-te Zeile erzeugen
        sal_uInt16 nBits = nItemBits;
        if ( GetAlign() == WINDOWALIGN_TOP || GetAlign() == WINDOWALIGN_BOTTOM )
            nBits |= SWIB_COLSET;
        InsertItem( nId, nSetSize, nLine, 0, nBits );
    }

    // In Zeile mit Position nLine das Fenster einf"ugen
    // ItemWindowSize auf "Prozentual" setzen, da SV dann das Umgr"o\sern
    // so macht, wie man erwartet; "Pixel" macht eigentlich nur Sinn, wenn
    // auch Items mit prozentualen oder relativen Gr"o\sen dabei sind.
    nItemBits |= SWIB_PERCENTSIZE;
    bLocked = sal_True;
    sal_uInt16 nSet = GetItemId( nLine );
    InsertItem( pDockWin->GetType(), pDockWin, nWinSize, nPos, nSet, nItemBits );

    // Splitwindows werden im SFX einmal angelegt und beim Einf"ugen des ersten
    // DockingWindows sichtbar gemacht.
    if ( GetItemCount( 0 ) == 1 && GetItemCount( 1 ) == 1 )
    {
        // Das Neuarrangieren am WorkWindow und ein Show() auf das SplitWindow
        // wird vom SfxDockingwindow veranla\st (->SfxWorkWindow::ConfigChild_Impl)
        if ( !bPinned && !IsFloatingMode() )
        {
            bPinned = sal_True;
            sal_Bool bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = sal_False;
            SetPinned_Impl( sal_False );
            pEmptyWin->Actualize();
            DBG_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering empty Splitwindow" );
            pWorkWin->RegisterChild_Impl( *GetSplitWindow(), eAlign, sal_True )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            if ( bFadeIn )
                FadeIn();
        }
        else
        {
            sal_Bool bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = sal_False;
            pEmptyWin->Actualize();
#ifdef DBG_UTIL
            if ( !bPinned || !pEmptyWin->bFadeIn )
            {
                DBG_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering empty Splitwindow" );
            }
            else
            {
                DBG_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering real Splitwindow" );
            }
#endif
            pWorkWin->RegisterChild_Impl( *GetSplitWindow(), eAlign, sal_True )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            if ( bFadeIn )
                FadeIn();
        }

        pWorkWin->ShowChilds_Impl();
    }

    if ( bUpdateMode )
        SetUpdateMode( sal_True );
    bLocked = sal_False;
}

//-------------------------------------------------------------------------

void SfxSplitWindow::RemoveWindow( SfxDockingWindow* pDockWin, sal_Bool bHide )

/*  [Beschreibung]

    Entfernt ein DockingWindow. Wenn es das letzte war, wird das SplitWindow
    gehidet.
*/
{
    sal_uInt16 nSet = GetSet( pDockWin->GetType() );

    // Splitwindows werden im SFX einmal angelegt und nach dem Entfernen
    // des letzten DockingWindows unsichtbar gemacht.
    if ( GetItemCount( nSet ) == 1 && GetItemCount( 0 ) == 1 )
    {
        // Das Neuarrangieren am WorkWindow wird vom SfxDockingwindow
        // veranla\st!
        Hide();
        pEmptyWin->aTimer.Stop();
        sal_uInt16 nRealState = pEmptyWin->nState;
        FadeOut_Impl();
        pEmptyWin->Hide();
#ifdef DBG_UTIL
        if ( !bPinned || !pEmptyWin->bFadeIn )
        {
            DBG_TRACE( "SfxSplitWindow::RemoveWindow - releasing empty Splitwindow" );
        }
        else
        {
            DBG_TRACE( "SfxSplitWindow::RemoveWindow - releasing real Splitwindow" );
        }
#endif
        pWorkWin->ReleaseChild_Impl( *GetSplitWindow() );
        pEmptyWin->nState = nRealState;
        pWorkWin->ArrangeAutoHideWindows( this );
    }

    SfxDock_Impl *pDock=0;
    sal_uInt16 nCount = pDockArr->Count();
    for ( sal_uInt16 n=0; n<nCount; n++ )
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
    sal_Bool bUpdateMode = IsUpdateMode();
    if ( bUpdateMode )
        SetUpdateMode( sal_False );
    bLocked = sal_True;

    RemoveItem( pDockWin->GetType() );

    if ( nSet && !GetItemCount( nSet ) )
        RemoveItem( nSet );

    if ( bUpdateMode )
        SetUpdateMode( sal_True );
    bLocked = sal_False;
};

//-------------------------------------------------------------------------

sal_Bool SfxSplitWindow::GetWindowPos( const SfxDockingWindow* pWindow,
                                        sal_uInt16& rLine, sal_uInt16& rPos ) const
/*  [Beschreibung]

    Liefert die Id des Itemsets und die des Items f"ur das "ubergebene
    DockingWindow in der alten Zeilen/Spalten-Bezeichnung zur"uck.
*/

{
    sal_uInt16 nSet = GetSet ( pWindow->GetType() );
    if ( nSet == SPLITWINDOW_ITEM_NOTFOUND )
        return sal_False;

    rPos  = GetItemPos( pWindow->GetType(), nSet );
    rLine = GetItemPos( nSet );
    return sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxSplitWindow::GetWindowPos( const Point& rTestPos,
                                      sal_uInt16& rLine, sal_uInt16& rPos ) const
/*  [Beschreibung]

    Liefert die Id des Itemsets und die des Items f"ur das DockingWindow
    an der "ubergebenen Position in der alten Zeilen/Spalten-Bezeichnung
    zur"uck.
*/

{
    sal_uInt16 nId = GetItemId( rTestPos );
    if ( nId == 0 )
        return sal_False;

    sal_uInt16 nSet = GetSet ( nId );
    rPos  = GetItemPos( nId, nSet );
    rLine = GetItemPos( nSet );
    return sal_True;
}

//-------------------------------------------------------------------------

sal_uInt16 SfxSplitWindow::GetLineCount() const

/*  [Beschreibung]

    Liefert die Zeilenzahl = Zahl der Sub-Itemsets im Root-Set.
*/
{
    return GetItemCount( 0 );
}

//-------------------------------------------------------------------------

long SfxSplitWindow::GetLineSize( sal_uInt16 nLine ) const

/*  [Beschreibung]

    Liefert die "Zeilenh"ohe" des nLine-ten Itemsets.
*/
{
    sal_uInt16 nId = GetItemId( nLine );
    return GetItemSize( nId );
}

//-------------------------------------------------------------------------

sal_uInt16 SfxSplitWindow::GetWindowCount( sal_uInt16 nLine ) const

/*  [Beschreibung]

    Liefert die
*/
{
    sal_uInt16 nId = GetItemId( nLine );
    return GetItemCount( nId );
}

//-------------------------------------------------------------------------

sal_uInt16 SfxSplitWindow::GetWindowCount() const

/*  [Beschreibung]

    Liefert die Gesamtzahl aller Fenstert
*/
{
    return GetItemCount( 0 );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::Command( const CommandEvent& rCEvt )
{
    SplitWindow::Command( rCEvt );
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxSplitWindow, TimerHdl, Timer*, pTimer)
{
    if ( pTimer )
        pTimer->Stop();

    if ( CursorIsOverRect( sal_False ) || !pTimer )
    {
        // Wenn der Mauszeiger innerhalb des Fensters liegt, SplitWindow anzeigen
        // und Timer zum Schlie\sen aufsetzen
        pEmptyWin->bAutoHide = sal_True;
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
            pEmptyWin->bEndAutoHide = sal_False;
            if ( !Application::IsInModalMode() &&
                  !PopupMenu::IsInExecute() &&
                  !pEmptyWin->bSplit && !HasChildPathFocus( sal_True ) )
            {
                // W"ahrend ein modaler Dialog oder ein Popupmenu offen sind
                // oder w"ahrend des Splittens auf keinen Fall zumachen; auch
                // solange eines der Children den Focus hat, bleibt das
                // das Fenster offen
                pEmptyWin->bEndAutoHide = sal_True;
            }

            if ( pEmptyWin->bEndAutoHide )
            {
                // Von mir aus kann Schlu\s sein mit AutoShow
                // Aber vielleicht will noch ein anderes SfxSplitWindow offen bleiben,
                // dann bleiben auch alle anderen offen
                if ( !pWorkWin->IsAutoHideMode( this ) )
                {
                    FadeOut_Impl();
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

sal_Bool SfxSplitWindow::CursorIsOverRect( sal_Bool bForceAdding ) const
{
    sal_Bool bVisible = IsVisible();

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
        Point aVisPos = GetPosPixel();
        Size aVisSize = GetSizePixel();

        // Um +/- ein paar Pixel erweitern, sonst ist es zu nerv"os
        aVisPos.X() -= nPixel;
        aVisPos.Y() -= nPixel;
        aVisSize.Width() += 2 * nPixel;
        aVisSize.Height() += 2 * nPixel;

        Rectangle aVisRect( aVisPos, aVisSize );
        aRect = aRect.GetUnion( aVisRect );
    }

    if ( aRect.IsInside( OutputToScreenPixel( ((Window*)this)->GetPointerPosPixel() ) ) )
        return sal_True;
    return sal_False;
}

//-------------------------------------------------------------------------

SplitWindow* SfxSplitWindow::GetSplitWindow()
{
    if ( !bPinned || !pEmptyWin->bFadeIn )
        return pEmptyWin;
    return this;
}

//-------------------------------------------------------------------------
sal_Bool SfxSplitWindow::IsFadeIn() const
{
    return pEmptyWin->bFadeIn;
}

sal_Bool SfxSplitWindow::IsAutoHide( sal_Bool bSelf ) const
{
    return bSelf ? pEmptyWin->bAutoHide && !pEmptyWin->bEndAutoHide : pEmptyWin->bAutoHide;
}

//-------------------------------------------------------------------------

void SfxSplitWindow::SetPinned_Impl( sal_Bool bOn )
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
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - releasing real Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - registering empty Splitwindow" );
            pWorkWin->RegisterChild_Impl( *pEmptyWin, eAlign, sal_True )->nVisible = CHILD_VISIBLE;
        }

        Point aPos( GetPosPixel() );
        aPos = GetParent()->OutputToScreenPixel( aPos );
        SetFloatingPos( aPos );
        SetFloatingMode( sal_True );
        GetFloatingWindow()->SetOutputSizePixel( GetOutputSizePixel() );

        if ( pEmptyWin->bFadeIn )
            Show();
    }
    else
    {
        pEmptyWin->nState &= ~1;
        SetOutputSizePixel( GetFloatingWindow()->GetOutputSizePixel() );
        SetFloatingMode( sal_False );

        if ( pEmptyWin->bFadeIn )
        {
            // Ersatzfenster abmelden
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - releasing empty Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - registering real Splitwindow" );
            pWorkWin->RegisterChild_Impl( *this, eAlign, sal_True )->nVisible = CHILD_VISIBLE;
        }
    }

    SetAutoHideState( !bPinned );
    pEmptyWin->SetAutoHideState( !bPinned );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::SetFadeIn_Impl( sal_Bool bOn )
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
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - releasing empty Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - registering real Splitwindow" );
            pWorkWin->RegisterChild_Impl( *this, eAlign, sal_True )->nVisible = CHILD_VISIBLE;
            pWorkWin->ArrangeChilds_Impl();
            pWorkWin->ShowChilds_Impl();
        }
    }
    else
    {
        pEmptyWin->bAutoHide = sal_False;
        pEmptyWin->nState &= ~2;
        if ( !IsFloatingMode() )
        {
            // Das Fenster "schwebt" nicht, soll aber ausgeblendet werden,
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - releasing real Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - registering empty Splitwindow" );
            pWorkWin->RegisterChild_Impl( *pEmptyWin, eAlign, sal_True )->nVisible = CHILD_VISIBLE;
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

void SfxSplitWindow::AutoHide()
{
    // Wenn dieser Handler am "echten" SplitWindow aufgerufen wird, ist es
    // entweder angedockt und soll "schwebend" angezeigt werden oder umgekehrt
    if ( !bPinned )
    {
        // Es "schwebt", also wieder andocken
        SetPinned_Impl( sal_True );
        pWorkWin->ArrangeChilds_Impl();
    }
    else
    {
        // In den "Schwebezustand" bringen
        SetPinned_Impl( sal_False );
        pWorkWin->ArrangeChilds_Impl();
        pWorkWin->ArrangeAutoHideWindows( this );
    }

    pWorkWin->ShowChilds_Impl();
    SaveConfig_Impl();
}

void SfxSplitWindow::FadeOut_Impl()
{
    if ( pEmptyWin->aTimer.IsActive() )
    {
        pEmptyWin->bAutoHide = sal_False;
        pEmptyWin->aTimer.Stop();
    }

    SetFadeIn_Impl( sal_False );
    Show_Impl();
}

void SfxSplitWindow::FadeOut()
{
    FadeOut_Impl();
    SaveConfig_Impl();
}

void SfxSplitWindow::FadeIn()
{
    SetFadeIn_Impl( sal_True );
    Show_Impl();
}

void SfxSplitWindow::Show_Impl()
{
    sal_uInt16 nCount = pDockArr->Count();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pDock = (*pDockArr)[n];
        if ( pDock->pWin )
            pDock->pWin->FadeIn( pEmptyWin->bFadeIn );
    }
}

sal_Bool SfxSplitWindow::ActivateNextChild_Impl( sal_Bool bForward )
{
    // Wenn kein pActive, auf erstes bzw. letztes Fenster gehen ( bei !bForward wird erst in der loop dekrementiert )
    sal_uInt16 nCount = pDockArr->Count();
    sal_uInt16 n = bForward ? 0 : nCount;

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
        for ( sal_uInt16 nNext=n; nNext<nCount; nNext++ )
        {
            SfxDock_Impl *pD = (*pDockArr)[nNext];
            if ( pD->pWin )
            {
                pD->pWin->GrabFocus();
                return sal_True;
            }
        }
    }
    else
    {
        // Vorheriges Fenster suchen
        for ( sal_uInt16 nNext=n; nNext--; )
        {
            SfxDock_Impl *pD = (*pDockArr)[nNext];
            if ( pD->pWin )
            {
                pD->pWin->GrabFocus();
                return sal_True;
            }
        }
    }

    return sal_False;
}

void SfxSplitWindow::SetActiveWindow_Impl( SfxDockingWindow* pWin )
{
    pActive = pWin;
    pWorkWin->SetActiveChild_Impl( this );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
