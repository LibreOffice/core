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
#include "sfxresid.hxx"
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
    USHORT              nType;
    SfxDockingWindow*  pWin;      // SplitWindow has this window
    BOOL                bNewLine;
    BOOL               bHide;     // SplitWindow had this window
    long                nSize;
};

typedef SfxDock_Impl* SfxDockPtr;
SV_DECL_PTRARR_DEL( SfxDockArr_Impl, SfxDockPtr, 4, 4)
SV_IMPL_PTRARR( SfxDockArr_Impl, SfxDockPtr);

class SfxEmptySplitWin_Impl : public SplitWindow
{
/*  [Description]

    The SfxEmptySplitWin_Impldow is an empty SplitWindow, that replaces the
    SfxSplitWindow AutoHide mode. It only serves as a placeholder to receive
    mouse moves and if possible blend in the true SplitWindow display.
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
                            , bSplit( FALSE )
                            , bEndAutoHide( FALSE )
                            , nState( 1 )
                        {
                            aTimer.SetTimeoutHdl(
                                LINK(pOwner, SfxSplitWindow, TimerHdl ) );
                            aTimer.SetTimeout( 200 );
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
    bAutoHide = TRUE;
    FadeIn();
}

void SfxEmptySplitWin_Impl::FadeIn()
{
    if (!bAutoHide )
        bAutoHide = IsFadeNoButtonMode();
    pOwner->SetFadeIn_Impl( TRUE );
    pOwner->Show_Impl();
    if ( bAutoHide )
    {
        // Set Timer to close; the caller has to ensure themselves that the
        // Window is not closed instantly (eg by setting the focus or a modal
        // mode.
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
        SfxWorkWindow *pW, BOOL bWithButtons, WinBits nBits )

/*  [Description]

    A SfxSplitWindow brings the recursive structure of the SV-SplitWindows to
    the outside by simulating a table-like structure with rows and columns
    (maximum recursion depth 2). Furthermore, it ensures the persistence of
    the arrangement of the SfxDockingWindows.
*/

:   SplitWindow ( pParent, nBits | WB_HIDE ),
    eAlign(eAl),
    pWorkWin(pW),
    pDockArr( new SfxDockArr_Impl ),
    bLocked(FALSE),
    bPinned(TRUE),
    pEmptyWin(NULL),
    pActive(NULL)
{
    if ( bWithButtons )
    {
        ShowAutoHideButton( FALSE );    // no autohide button (pin) anymore
        ShowFadeOutButton( TRUE );
    }

    // Set SV-Alignment
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
        default:
            eTbxAlign = WINDOWALIGN_TOP;  // some sort of default...
            break;  // -Wall lots not handled..
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
        //  Read Configuration
        String aWindowId = String::CreateFromAscii("SplitWindow");
        aWindowId += String::CreateFromInt32( (sal_Int32) eTbxAlign );
        SvtViewOptions aWinOpt( E_WINDOW, aWindowId );
        String aWinData;
        Any aUserItem = aWinOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aWinData = String( aTemp );
        if ( aWinData.Len() && aWinData.GetChar( (USHORT) 0 ) == 'V' )
        {
            pEmptyWin->nState = (USHORT) aWinData.GetToken( 1, ',' ).ToInt32();
            if ( pEmptyWin->nState & 2 )
                pEmptyWin->bFadeIn = TRUE;
            bPinned = TRUE; // always assume pinned - floating mode not used anymore

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
                  // could mean NewLine
                    pDock->nType = (USHORT) aWinData.GetToken(i++, ',').ToInt32();
                    if ( !pDock->nType )
                    {
                        // Read error
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
        SaveConfig_Impl();

    if ( pEmptyWin )
    {
        // Set pOwner to NULL, otherwise try to delete pEmptyWin once more. The
        // window that is just beeing docked is always deleted from the outside.
        pEmptyWin->pOwner = NULL;
        delete pEmptyWin;
    }

    delete pDockArr;
}

void SfxSplitWindow::SaveConfig_Impl()
{
    // Save configuration
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

/*  [Description]

    To insert SfxDockingWindows can no position be passed. The SfxSplitWindow
    searches the last marked one to the passed SfxDockingWindow or appends a
    new one at the end.
*/
{
    short nLine = -1;  // so that the first window cab set nline to 0
    USHORT nL;
    USHORT nPos = 0;
    BOOL bNewLine = TRUE;
    BOOL bSaveConfig = FALSE;
    SfxDock_Impl *pFoundDock=0;
    USHORT nCount = pDockArr->Count();
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pDock = (*pDockArr)[n];
        if ( pDock->bNewLine )
        {
            // The window opens a new line
            if ( pFoundDock )
                // But after the just inserted window
                break;

            // New line
            nPos = 0;
            bNewLine = TRUE;
        }

        if ( pDock->pWin )
        {
            // Does there exist a window now at this position
            if ( bNewLine && !pFoundDock )
            {
                // Not known until now in which real line it is located
                GetWindowPos( pDock->pWin, nL, nPos );
                nLine = (short) nL;
            }

            if ( !pFoundDock )
            {
                // The window is located before the inserted one
                nPos++;
            }

            // Line is opened
            bNewLine = FALSE;
            if ( pFoundDock )
                break;
        }

        if ( pDock->nType == pDockWin->GetType() )
        {
            DBG_ASSERT( !pFoundDock && !pDock->pWin, "Window does already exist!");
            pFoundDock = pDock;
            if ( !bNewLine )
                break;
            else
            {
                // A new line has been created but no window was fond there;
                // continue searching for a window in this line in-order to set
                // bNewLine correctly. While doing so nline or nPos are not
                // to be changed!
                nLine++;
            }
        }
    }

    if ( !pFoundDock )
    {
        // Not found, insert at end
        pFoundDock = new SfxDock_Impl;
        pFoundDock->bHide = TRUE;
        pDockArr->Insert( pFoundDock, nCount );
        pFoundDock->nType = pDockWin->GetType();
        nLine++;
        nPos = 0;
        bNewLine = TRUE;
        pFoundDock->bNewLine = bNewLine;
        bSaveConfig = TRUE;
    }

    pFoundDock->pWin = pDockWin;
    pFoundDock->bHide = FALSE;
    InsertWindow_Impl( pFoundDock, rSize, nLine, nPos, bNewLine );
    if ( bSaveConfig )
        SaveConfig_Impl();
}

//-------------------------------------------------------------------------

void SfxSplitWindow::ReleaseWindow_Impl(SfxDockingWindow *pDockWin, BOOL bSave)

/*  [Description]

    The docking window is no longer stored in the internal data.
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

            // Window has a position, this we forget
            bFound = TRUE;
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
                        USHORT nLine, USHORT nPos, BOOL bNewLine)

/*  [Description]

    The docking window is moved within the SplitWindows.
*/

{
    USHORT nL, nP;
    GetWindowPos( pDockWin, nL, nP );

    if ( nLine > nL && GetItemCount( GetItemId( nL, 0 ) ) == 1 )
    {
        // If the last window is removed from its line, then everything slips
        // one line to the front!
        nLine--;
    }
    RemoveWindow( pDockWin );
    InsertWindow( pDockWin, rSize, nLine, nPos, bNewLine );
}

//-------------------------------------------------------------------------

void SfxSplitWindow::InsertWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        USHORT nLine, USHORT nPos, BOOL bNewLine)

/*  [Description]

    The DockingWindow that is pushed on this SplitWindow and shall hold the
    given position and size.
*/
{
    ReleaseWindow_Impl( pDockWin, FALSE );
    SfxDock_Impl *pDock = new SfxDock_Impl;
    pDock->bHide = FALSE;
    pDock->nType = pDockWin->GetType();
    pDock->bNewLine = bNewLine;
    pDock->pWin = pDockWin;

    DBG_ASSERT( nPos==0 || !bNewLine, "Wrong Paramenter!");
    if ( bNewLine )
        nPos = 0;

    // The window must be inserted before the first window so that it has the
    // same or a greater position than pDockWin.
    USHORT nCount = pDockArr->Count();
    USHORT nLastWindowIdx(0);

    // If no window is found, a first window is inserted
    USHORT nInsertPos = 0;
    for ( USHORT n=0; n<nCount; n++ )
    {
        SfxDock_Impl *pD = (*pDockArr)[n];

        if (pD->pWin)
        {
            // A docked window has been found. If no suitable window behind the
            // the desired insertion point s found, then insertion is done at
            // the end.
            nInsertPos = nCount;
            nLastWindowIdx = n;
            USHORT nL=0, nP=0;
            GetWindowPos( pD->pWin, nL, nP );

            if ( (nL == nLine && nP == nPos) || nL > nLine )
            {
                DBG_ASSERT( nL == nLine || bNewLine || nPos > 0, "Wrong Parameter!" );
                if ( nL == nLine && nPos == 0 && !bNewLine )
                {
                    DBG_ASSERT(pD->bNewLine, "No new line?");

                    // The posption is pushed to nPos==0
                    pD->bNewLine = FALSE;
                    pDock->bNewLine = TRUE;
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
                        USHORT nLine, USHORT nPos, BOOL bNewLine)

/*  [Description]

    Adds a DockingWindow, and causes the recalculation of the size of
    the SplitWindows.
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
        // An existing row should not be inserted, instead a new one
        // will be created

        USHORT nId = 1;
        for ( USHORT n=0; n<GetItemCount(0); n++ )
        {
            if ( GetItemId(n) >= nId )
                nId = GetItemId(n)+1;
        }

        // Create a new nLine:th line
        USHORT nBits = nItemBits;
        if ( GetAlign() == WINDOWALIGN_TOP || GetAlign() == WINDOWALIGN_BOTTOM )
            nBits |= SWIB_COLSET;
        InsertItem( nId, nSetSize, nLine, 0, nBits );
    }

    // Insert the window at line with the position nline. ItemWindowSize set to
    // "percentage" share since the SV then does the re-sizing as expected,
    // "pixel" actually only makes sense if also items with percentage or
    // relative sizes are present.
    nItemBits |= SWIB_PERCENTSIZE;
    bLocked = TRUE;
    USHORT nSet = GetItemId( nLine );
    InsertItem( pDockWin->GetType(), pDockWin, nWinSize, nPos, nSet, nItemBits );

    // SplitWindows are once created in SFX and when inserting the first
    // DockingWindows is made visable.
    if ( GetItemCount( 0 ) == 1 && GetItemCount( 1 ) == 1 )
    {
        // The Rearranging in WorkWindow and a Show() on the SplitWindow is
        // caues by SfxDockingwindow (->SfxWorkWindow::ConfigChild_Impl)
        if ( !bPinned && !IsFloatingMode() )
        {
            bPinned = TRUE;
            BOOL bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = FALSE;
            SetPinned_Impl( FALSE );
            pEmptyWin->Actualize();
            DBG_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering empty Splitwindow" );
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

/*  [Description]

    Removes a DockingWindow. If it was the last one, then the SplitWindow is
    beeing hidden.
*/
{
    USHORT nSet = GetSet( pDockWin->GetType() );

    // SplitWindows are once created in SFX and is made invisible after
    // removing the last DockingWindows.
    if ( GetItemCount( nSet ) == 1 && GetItemCount( 0 ) == 1 )
    {
        // The Rearranging in WorkWindow is caues by SfxDockingwindow
        Hide();
        pEmptyWin->aTimer.Stop();
        USHORT nRealState = pEmptyWin->nState;
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
    USHORT nCount = pDockArr->Count();
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

    // Remove Windows, and if it was the last of the line, then also remove
    // the line (line = itemset)
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
/*  [Description]

    Returns the ID of the item sets and items for the DockingWindow in
    the position passed on the old row / column-name.
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
/*  [Description]

    Returns the ID of the item sets and items for the DockingWindow in
    the position passed on the old row / column-name.
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

/*  [Description]

    Returns the number of rows = number of sub-itemsets in the root set.
*/
{
    return GetItemCount( 0 );
}

//-------------------------------------------------------------------------

long SfxSplitWindow::GetLineSize( USHORT nLine ) const

/*  [Description]

    Returns the Row Height of nline itemset.
*/
{
    USHORT nId = GetItemId( nLine );
    return GetItemSize( nId );
}

//-------------------------------------------------------------------------

USHORT SfxSplitWindow::GetWindowCount( USHORT nLine ) const

/*  [Description]

    Returns the total number of windows
*/
{
    USHORT nId = GetItemId( nLine );
    return GetItemCount( nId );
}

//-------------------------------------------------------------------------

USHORT SfxSplitWindow::GetWindowCount() const

/*  [Description]

    Returns the total number of windows
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

    if ( CursorIsOverRect( FALSE ) || !pTimer )
    {
        // If the cursor is within the window, display the SplitWindow and set
        // up the timer for close
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
            // The mouse has moved within the running time of the timer, thus
            // do nothing
            pEmptyWin->aLastPos = GetPointerPosPixel();
            pEmptyWin->aTimer.Start();
            return 0L;
        }

        // Especially for TF_AUTOSHOW_ON_MOUSEMOVE :
        // If the window is not visible, there is nothing to do
        // (user has simply moved the mouse over pEmptyWin)
        if ( IsVisible() )
        {
            pEmptyWin->bEndAutoHide = FALSE;
            if ( !Application::IsInModalMode() &&
                  !PopupMenu::IsInExecute() &&
                  !pEmptyWin->bSplit && !HasChildPathFocus( TRUE ) )
            {
                // While a modal dialog or a popup menu is open or while the
                // Splitting is done, in any case, do not close. Even as long
                // as one of the Children has the focus, the window remains
                // open.
                pEmptyWin->bEndAutoHide = TRUE;
            }

            if ( pEmptyWin->bEndAutoHide )
            {
               // As far as I am concered this can be the end of AutoShow
               // But maybe some other SfxSplitWindow will remain open,
               // then all others remain open too.
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

BOOL SfxSplitWindow::CursorIsOverRect( BOOL bForceAdding ) const
{
    BOOL bVisible = IsVisible();

    // Also, take the collapsed SplitWindow into account
    Point aPos = pEmptyWin->GetParent()->OutputToScreenPixel( pEmptyWin->GetPosPixel() );
    Size aSize = pEmptyWin->GetSizePixel();

    if ( bForceAdding )
    {
        // Extend with +/- a few pixels, otherwise it is too nervous
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

        // Extend with +/- a few pixels, otherwise it is too nervous
        aVisPos.X() -= nPixel;
        aVisPos.Y() -= nPixel;
        aVisSize.Width() += 2 * nPixel;
        aVisSize.Height() += 2 * nPixel;

        Rectangle aVisRect( aVisPos, aVisSize );
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
            // Unregister replacement windows
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - releasing real Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - registering empty Splitwindow" );
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
            // Unregister replacement windows
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - releasing empty Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            DBG_TRACE( "SfxSplitWindow::SetPinned_Impl - registering real Splitwindow" );
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
            // FloatingWindow is not visable, thus display it
            pWorkWin->ArrangeAutoHideWindows( this );
            Show();
        }
        else
        {
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - releasing empty Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - registering real Splitwindow" );
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
            // The window is not "floating", should be hidden
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - releasing real Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            DBG_TRACE( "SfxSplitWindow::SetFadeIn_Impl - registering empty Splitwindow" );
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

void SfxSplitWindow::AutoHide()
{
    // If this handler is called in the "real" SplitWindow, it is
    // either docked and should be displayed as floating, or vice versa
    if ( !bPinned )
    {
        // It "floats", thus dock it again
        SetPinned_Impl( TRUE );
        pWorkWin->ArrangeChilds_Impl();
    }
    else
    {
        // In "limbo"
        SetPinned_Impl( FALSE );
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
        pEmptyWin->bAutoHide = FALSE;
        pEmptyWin->aTimer.Stop();
    }

    SetFadeIn_Impl( FALSE );
    Show_Impl();
}

void SfxSplitWindow::FadeOut()
{
    FadeOut_Impl();
    SaveConfig_Impl();
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

BOOL SfxSplitWindow::ActivateNextChild_Impl( BOOL bForward )
{
    // If no pActive, go to first and last window (!bForward is first
    // decremented in the loop)
    USHORT nCount = pDockArr->Count();
    USHORT n = bForward ? 0 : nCount;

    // if Focus is within, then move to a window forward or backwards
    // if possible
    if ( pActive )
    {
        // Determine the active window
        for ( n=0; n<nCount; n++ )
        {
            SfxDock_Impl *pD = (*pDockArr)[n];
            if ( pD->pWin && pD->pWin->HasChildPathFocus() )
                break;
        }

        if ( bForward )
            // up window counter (then when n>nCount, the loop below is
            // not entered)
            n++;
    }

    if ( bForward )
    {
        // Search for next window
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
        // Search for previous window
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
