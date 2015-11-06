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
#include <sfx2/sfxresid.hxx>
#include <sfx2/mnumgr.hxx>
#include "virtmenu.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>

#include <vector>
#include <utility>

using namespace ::com::sun::star::uno;

#define VERSION 1
#define nPixel  30L
#define USERITEM_NAME           "UserItem"

namespace {
    // helper class to deactivate UpdateMode, if needed, for the life time of an instance
    class DeactivateUpdateMode
    {
    public:
        explicit DeactivateUpdateMode( SfxSplitWindow& rSplitWindow )
            : mrSplitWindow( rSplitWindow )
            , mbUpdateMode( rSplitWindow.IsUpdateMode() )
        {
            if ( mbUpdateMode )
            {
                mrSplitWindow.SetUpdateMode( false );
            }
        }

        ~DeactivateUpdateMode()
        {
            if ( mbUpdateMode )
            {
                mrSplitWindow.SetUpdateMode( true );
            }
        }

    private:
        SfxSplitWindow& mrSplitWindow;
        const bool mbUpdateMode;
    };
}

class SfxEmptySplitWin_Impl : public SplitWindow
{
/*  [Description]

    The SfxEmptySplitWin_Impldow is an empty SplitWindow, that replaces the
    SfxSplitWindow AutoHide mode. It only serves as a placeholder to receive
    mouse moves and if possible blend in the true SplitWindow display.
*/
friend class SfxSplitWindow;

    VclPtr<SfxSplitWindow>  pOwner;
    bool                    bFadeIn;
    bool                    bAutoHide;
    bool                    bSplit;
    bool                    bEndAutoHide;
    Timer                   aTimer;
    Point                   aLastPos;
    sal_uInt16              nState;

public:
   explicit             SfxEmptySplitWin_Impl( SfxSplitWindow *pParent )
                            : SplitWindow( pParent->GetParent(), WinBits( WB_BORDER | WB_3DLOOK ) )
                            , pOwner( pParent )
                            , bFadeIn( false )
                            , bAutoHide( false )
                            , bSplit( false )
                            , bEndAutoHide( false )
                            , nState( 1 )
                        {
                            aTimer.SetTimeoutHdl(
                                LINK(pOwner, SfxSplitWindow, TimerHdl ) );
                            aTimer.SetTimeout( 200 );
                            SetAlign( pOwner->GetAlign() );
                            Actualize();
                            ShowAutoHideButton( pOwner->IsAutoHideButtonVisible() );
                            ShowFadeInHideButton();
                        }

                        virtual ~SfxEmptySplitWin_Impl()
                        { disposeOnce(); }
   virtual void         dispose() override
                        {
                            aTimer.Stop();
                            pOwner.clear();
                            SplitWindow::dispose();
                        }

    virtual void        MouseMove( const MouseEvent& ) override;
    virtual void        AutoHide() override;
    virtual void        FadeIn() override;
    void                Actualize();
};

void SfxEmptySplitWin_Impl::Actualize()
{
    Size aSize( pOwner->GetSizePixel() );
    switch ( pOwner->GetAlign() )
    {
        case WindowAlign::Left:
        case WindowAlign::Right:
            aSize.Width() = GetFadeInSize();
            break;
        case WindowAlign::Top:
        case WindowAlign::Bottom:
            aSize.Height() = GetFadeInSize();
            break;
    }

    SetSizePixel( aSize );
}

void SfxEmptySplitWin_Impl::AutoHide()
{
    pOwner->SetPinned_Impl( !pOwner->bPinned );
    pOwner->SaveConfig_Impl();
    bAutoHide = true;
    FadeIn();
}

void SfxEmptySplitWin_Impl::FadeIn()
{
    if (!bAutoHide )
        bAutoHide = IsFadeNoButtonMode();
    pOwner->SetFadeIn_Impl( true );
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



void SfxSplitWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() != 2 )
        SplitWindow::MouseButtonDown( rMEvt );
}

void SfxEmptySplitWin_Impl::MouseMove( const MouseEvent& rMEvt )
{
    SplitWindow::MouseMove( rMEvt );
}



SfxSplitWindow::SfxSplitWindow( vcl::Window* pParent, SfxChildAlignment eAl,
        SfxWorkWindow *pW, bool bWithButtons, WinBits nBits )

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
    bLocked(false),
    bPinned(true),
    pEmptyWin(NULL),
    pActive(NULL)
{
    if ( bWithButtons )
    {
        ShowAutoHideButton( false );    // no autohide button (pin) anymore
        ShowFadeOutButton();
    }

    // Set SV-Alignment
    WindowAlign eTbxAlign;
    switch ( eAlign )
    {
        case SfxChildAlignment::LEFT:
            eTbxAlign = WindowAlign::Left;
            break;
        case SfxChildAlignment::RIGHT:
            eTbxAlign = WindowAlign::Right;
            break;
        case SfxChildAlignment::TOP:
            eTbxAlign = WindowAlign::Top;
            break;
        case SfxChildAlignment::BOTTOM:
            eTbxAlign = WindowAlign::Bottom;
            bPinned = true;
            break;
        default:
            eTbxAlign = WindowAlign::Top;  // some sort of default...
            break;  // -Wall lots not handled..
    }

    SetAlign (eTbxAlign);
    pEmptyWin = VclPtr<SfxEmptySplitWin_Impl>::Create( this );
    if ( bPinned )
    {
        pEmptyWin->bFadeIn = true;
        pEmptyWin->nState = 2;
    }

    if ( bWithButtons )
    {
        //  Read Configuration
        OUString aWindowId("SplitWindow");
        aWindowId += OUString::number( (sal_Int32) eTbxAlign );
        SvtViewOptions aWinOpt( E_WINDOW, aWindowId );
        OUString aWinData;
        Any aUserItem = aWinOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aWinData = aTemp;
        if ( aWinData.startsWith("V") )
        {
            pEmptyWin->nState = (sal_uInt16) aWinData.getToken( 1, ',' ).toInt32();
            if ( pEmptyWin->nState & 2 )
                pEmptyWin->bFadeIn = true;
            bPinned = true; // always assume pinned - floating mode not used anymore

            sal_uInt16 i=2;
            sal_uInt16 nCount = (sal_uInt16) aWinData.getToken(i++, ',').toInt32();
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                SfxDock_Impl *pDock = new SfxDock_Impl;
                pDock->pWin = 0;
                pDock->bNewLine = false;
                pDock->bHide = true;
                pDock->nType = (sal_uInt16) aWinData.getToken(i++, ',').toInt32();
                if ( !pDock->nType )
                {
                    // could mean NewLine
                    pDock->nType = (sal_uInt16) aWinData.getToken(i++, ',').toInt32();
                    if ( !pDock->nType )
                    {
                        // Read error
                        delete pDock;
                        break;
                    }
                    else
                        pDock->bNewLine = true;
                }

                pDockArr->insert(pDockArr->begin() + n, pDock);
            }
        }
    }
    else
    {
        bPinned = true;
        pEmptyWin->bFadeIn = true;
        pEmptyWin->nState = 2;
    }

    SetAutoHideState( !bPinned );
    pEmptyWin->SetAutoHideState( !bPinned );
}



SfxSplitWindow::~SfxSplitWindow()
{
    disposeOnce();
}

void SfxSplitWindow::dispose()
{
    if ( !pWorkWin->GetParent_Impl() )
        SaveConfig_Impl();

    if ( pEmptyWin )
    {
        // Set pOwner to NULL, otherwise try to delete pEmptyWin once more. The
        // window that is just being docked is always deleted from the outside.
        pEmptyWin->pOwner = NULL;
    }
    pEmptyWin.disposeAndClear();

    delete pDockArr;
    pActive.clear();
    SplitWindow::dispose();
}

void SfxSplitWindow::SaveConfig_Impl()
{
    // Save configuration
    OUStringBuffer aWinData;
    aWinData.append('V');
    aWinData.append(static_cast<sal_Int32>(VERSION));
    aWinData.append(',');
    aWinData.append(static_cast<sal_Int32>(pEmptyWin->nState));
    aWinData.append(',');

    sal_uInt16 nCount = 0;
    sal_uInt16 n;
    for ( n=0; n<pDockArr->size(); n++ )
    {
        const SfxDock_Impl& rDock = (*pDockArr)[n];
        if ( rDock.bHide || rDock.pWin )
            nCount++;
    }

    aWinData.append(static_cast<sal_Int32>(nCount));

    for ( n=0; n<pDockArr->size(); n++ )
    {
        const SfxDock_Impl& rDock = (*pDockArr)[n];
        if ( !rDock.bHide && !rDock.pWin )
            continue;
        if ( rDock.bNewLine )
            aWinData.append(",0");
        aWinData.append(',');
        aWinData.append(static_cast<sal_Int32>(rDock.nType));
    }

    OUString aWindowId("SplitWindow");
    aWindowId += OUString::number( (sal_Int32) GetAlign() );
    SvtViewOptions aWinOpt( E_WINDOW, aWindowId );
    aWinOpt.SetUserItem( USERITEM_NAME, makeAny( aWinData.makeStringAndClear() ) );
}



void SfxSplitWindow::StartSplit()
{
    long nSize = 0;
    Size aSize = GetSizePixel();

    if ( pEmptyWin )
    {
        pEmptyWin->bFadeIn = true;
        pEmptyWin->bSplit = true;
    }

    Rectangle aRect = pWorkWin->GetFreeArea( !bPinned );
    switch ( GetAlign() )
    {
        case WindowAlign::Left:
        case WindowAlign::Right:
            nSize = aSize.Width() + aRect.GetWidth();
            break;
        case WindowAlign::Top:
        case WindowAlign::Bottom:
            nSize = aSize.Height() + aRect.GetHeight();
            break;
    }

    SetMaxSizePixel( nSize );
}



void SfxSplitWindow::SplitResize()
{
    if ( bPinned )
    {
        pWorkWin->ArrangeChildren_Impl();
        pWorkWin->ShowChildren_Impl();
    }
    else
        pWorkWin->ArrangeAutoHideWindows( this );
}



void SfxSplitWindow::Split()
{
    if ( pEmptyWin )
        pEmptyWin->bSplit = false;

    SplitWindow::Split();

    std::vector< std::pair< sal_uInt16, long > > aNewOrgSizes;

    sal_uInt16 nCount = pDockArr->size();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        const SfxDock_Impl& rD = (*pDockArr)[n];
        if ( rD.pWin )
        {
            const sal_uInt16 nId = rD.nType;
            const long nSize    = GetItemSize( nId, SplitWindowItemFlags::Fixed );
            const long nSetSize = GetItemSize( GetSet( nId ) );
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

            rD.pWin->SetItemSize_Impl( aSize );

            aNewOrgSizes.push_back( std::pair< sal_uInt16, long >( nId, nSize ) );
        }
    }

    // workaround insuffiency of <SplitWindow> regarding dock layouting:
    // apply FIXED item size as 'original' item size to improve layouting of undock-dock-cycle of a window
    {
        DeactivateUpdateMode aDeactivateUpdateMode( *this );
        for ( size_t i = 0; i < aNewOrgSizes.size(); ++i )
        {
            SetItemSize( aNewOrgSizes[i].first, aNewOrgSizes[i].second );
        }
    }

    SaveConfig_Impl();
}



void SfxSplitWindow::InsertWindow( SfxDockingWindow* pDockWin, const Size& rSize)

/*
    To insert SfxDockingWindows just pass no position. The SfxSplitWindow
    searches the last marked one to the passed SfxDockingWindow or appends a
    new one at the end.
*/
{
    short nLine = -1;  // so that the first window cab set nline to 0
    sal_uInt16 nL;
    sal_uInt16 nPos = 0;
    bool bNewLine = true;
    bool bSaveConfig = false;
    SfxDock_Impl *pFoundDock=0;
    sal_uInt16 nCount = pDockArr->size();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxDock_Impl& rDock = (*pDockArr)[n];
        if ( rDock.bNewLine )
        {
            // The window opens a new line
            if ( pFoundDock )
                // But after the just inserted window
                break;

            // New line
            nPos = 0;
            bNewLine = true;
        }

        if ( rDock.pWin )
        {
            // Does there exist a window now at this position
            if ( bNewLine && !pFoundDock )
            {
                // Not known until now in which real line it is located
                GetWindowPos( rDock.pWin, nL, nPos );
                nLine = (short) nL;
            }

            if ( !pFoundDock )
            {
                // The window is located before the inserted one
                nPos++;
            }

            // Line is opened
            bNewLine = false;
            if ( pFoundDock )
                break;
        }

        if ( rDock.nType == pDockWin->GetType() )
        {
            DBG_ASSERT( !pFoundDock && !rDock.pWin, "Window already exists!");
            pFoundDock = &rDock;
            if ( !bNewLine )
                break;
            else
            {
                // A new line has been created but no window was found there;
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
        pFoundDock->bHide = true;
        pDockArr->push_back( pFoundDock );
        pFoundDock->nType = pDockWin->GetType();
        nLine++;
        nPos = 0;
        bNewLine = true;
        pFoundDock->bNewLine = bNewLine;
        bSaveConfig = true;
    }

    pFoundDock->pWin = pDockWin;
    pFoundDock->bHide = false;
    InsertWindow_Impl( pFoundDock, rSize, nLine, nPos, bNewLine );
    if ( bSaveConfig )
        SaveConfig_Impl();
}



void SfxSplitWindow::ReleaseWindow_Impl(SfxDockingWindow *pDockWin, bool bSave)
{
//  The docking window is no longer stored in the internal data.
    sal_uInt16 nCount = pDockArr->size();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        const SfxDock_Impl& rDock = (*pDockArr)[n];
        if ( rDock.nType == pDockWin->GetType() )
        {
            if ( rDock.bNewLine && n<nCount-1 )
                (*pDockArr)[n+1].bNewLine = true;

            // Window has a position, this we forget
            pDockArr->erase(pDockArr->begin() + n);
            break;
        }
    }

    if ( bSave )
        SaveConfig_Impl();
}



void SfxSplitWindow::MoveWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        sal_uInt16 nLine, sal_uInt16 nPos, bool bNewLine)

/*  [Description]

    The docking window is moved within the SplitWindows.
*/

{
    sal_uInt16 nL, nP;
    GetWindowPos( pDockWin, nL, nP );

    if ( nLine > nL && GetItemCount( GetItemId( nL ) ) == 1 )
    {
        // If the last window is removed from its line, then everything slips
        // one line to the front!
        nLine--;
    }
    RemoveWindow( pDockWin );
    InsertWindow( pDockWin, rSize, nLine, nPos, bNewLine );
}



void SfxSplitWindow::InsertWindow( SfxDockingWindow* pDockWin, const Size& rSize,
                        sal_uInt16 nLine, sal_uInt16 nPos, bool bNewLine)

/*  [Description]

    The DockingWindow that is pushed on this SplitWindow and shall hold the
    given position and size.
*/
{
    ReleaseWindow_Impl( pDockWin, false );
    SfxDock_Impl *pDock = new SfxDock_Impl;
    pDock->bHide = false;
    pDock->nType = pDockWin->GetType();
    pDock->bNewLine = bNewLine;
    pDock->pWin = pDockWin;

    DBG_ASSERT( nPos==0 || !bNewLine, "Wrong Paramenter!");
    if ( bNewLine )
        nPos = 0;

    // The window must be inserted before the first window so that it has the
    // same or a greater position than pDockWin.
    sal_uInt16 nCount = pDockArr->size();
    sal_uInt16 nLastWindowIdx(0);

    // If no window is found, a first window is inserted
    sal_uInt16 nInsertPos = 0;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxDock_Impl& rD = (*pDockArr)[n];

        if (rD.pWin)
        {
            // A docked window has been found. If no suitable window behind the
            // the desired insertion point s found, then insertion is done at
            // the end.
            nInsertPos = nCount;
            nLastWindowIdx = n;
            sal_uInt16 nL=0, nP=0;
            GetWindowPos( rD.pWin, nL, nP );

            if ( (nL == nLine && nP == nPos) || nL > nLine )
            {
                DBG_ASSERT( nL == nLine || bNewLine || nPos > 0, "Wrong Parameter!" );
                if ( nL == nLine && nPos == 0 && !bNewLine )
                {
                    DBG_ASSERT(rD.bNewLine, "No new line?");

                    // The position is pushed to nPos==0
                    rD.bNewLine = false;
                    pDock->bNewLine = true;
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

    pDockArr->insert(pDockArr->begin() + nInsertPos, pDock);
    InsertWindow_Impl( pDock, rSize, nLine, nPos, bNewLine );
    SaveConfig_Impl();
}



void SfxSplitWindow::InsertWindow_Impl( SfxDock_Impl* pDock,
                        const Size& rSize,
                        sal_uInt16 nLine, sal_uInt16 nPos, bool bNewLine)

/*  [Description]

    Adds a DockingWindow, and causes the recalculation of the size of
    the SplitWindows.
*/

{
    SfxDockingWindow* pDockWin = pDock->pWin;

    SplitWindowItemFlags nItemBits = pDockWin->GetWinBits_Impl();

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

    DeactivateUpdateMode* pDeactivateUpdateMode = new DeactivateUpdateMode( *this );

    if ( bNewLine || nLine == GetItemCount() )
    {
        // An existing row should not be inserted, instead a new one
        // will be created

        sal_uInt16 nId = 1;
        for ( sal_uInt16 n=0; n<GetItemCount(); n++ )
        {
            if ( GetItemId(n) >= nId )
                nId = GetItemId(n)+1;
        }

        // Create a new nLine:th line
        SplitWindowItemFlags nBits = nItemBits;
        if ( GetAlign() == WindowAlign::Top || GetAlign() == WindowAlign::Bottom )
            nBits |= SplitWindowItemFlags::ColSet;
        InsertItem( nId, nSetSize, nLine, 0, nBits );
    }

    // Insert the window at line with the position nline. ItemWindowSize set to
    // "percentage" share since the SV then does the re-sizing as expected,
    // "pixel" actually only makes sense if also items with percentage or
    // relative sizes are present.
    nItemBits |= SplitWindowItemFlags::PercentSize;
    bLocked = true;
    sal_uInt16 nSet = GetItemId( nLine );
    InsertItem( pDockWin->GetType(), pDockWin, nWinSize, nPos, nSet, nItemBits );

    // SplitWindows are once created in SFX and when inserting the first
    // DockingWindows is made visible.
    if ( GetItemCount() == 1 && GetItemCount( 1 ) == 1 )
    {
        // The Rearranging in WorkWindow and a Show() on the SplitWindow is
        // caused by SfxDockingwindow (->SfxWorkWindow::ConfigChild_Impl)
        if ( !bPinned && !IsFloatingMode() )
        {
            bPinned = true;
            bool bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = false;
            SetPinned_Impl( false );
            pEmptyWin->Actualize();
            OSL_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering empty Splitwindow" );
            pWorkWin->RegisterChild_Impl( *GetSplitWindow(), eAlign, true )->nVisible = SfxChildVisibility::VISIBLE;
            pWorkWin->ArrangeChildren_Impl();
            if ( bFadeIn )
                FadeIn();
        }
        else
        {
            bool bFadeIn = ( pEmptyWin->nState & 2 ) != 0;
            pEmptyWin->bFadeIn = false;
            pEmptyWin->Actualize();
#ifdef DBG_UTIL
            if ( !bPinned || !pEmptyWin->bFadeIn )
            {
                OSL_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering empty Splitwindow" );
            }
            else
            {
                OSL_TRACE( "SfxSplitWindow::InsertWindow_Impl - registering real Splitwindow" );
            }
#endif
            pWorkWin->RegisterChild_Impl( *GetSplitWindow(), eAlign, true )->nVisible = SfxChildVisibility::VISIBLE;
            pWorkWin->ArrangeChildren_Impl();
            if ( bFadeIn )
                FadeIn();
        }

        pWorkWin->ShowChildren_Impl();
    }

    delete pDeactivateUpdateMode;
    bLocked = false;

    // workaround insuffiency of <SplitWindow> regarding dock layouting:
    // apply FIXED item size as 'original' item size to improve layouting of undock-dock-cycle of a window
    {
        std::vector< std::pair< sal_uInt16, long > > aNewOrgSizes;
        // get FIXED item sizes
        sal_uInt16 nCount = pDockArr->size();
        for ( sal_uInt16 n=0; n<nCount; ++n )
        {
            const SfxDock_Impl& rD = (*pDockArr)[n];
            if ( rD.pWin )
            {
                const sal_uInt16 nId = rD.nType;
                const long nSize    = GetItemSize( nId, SplitWindowItemFlags::Fixed );
                aNewOrgSizes.push_back( std::pair< sal_uInt16, long >( nId, nSize ) );
            }
        }
        // apply new item sizes
        DeactivateUpdateMode aDeactivateUpdateMode( *this );
        for ( size_t i = 0; i < aNewOrgSizes.size(); ++i )
        {
            SetItemSize( aNewOrgSizes[i].first, aNewOrgSizes[i].second );
        }
    }
}



void SfxSplitWindow::RemoveWindow( SfxDockingWindow* pDockWin, bool bHide )

/*  [Description]

    Removes a DockingWindow. If it was the last one, then the SplitWindow is
    being hidden.
*/
{
    sal_uInt16 nSet = GetSet( pDockWin->GetType() );

    // SplitWindows are once created in SFX and is made invisible after
    // removing the last DockingWindows.
    if ( GetItemCount( nSet ) == 1 && GetItemCount() == 1 )
    {
        // The Rearranging in WorkWindow is caused by SfxDockingwindow
        Hide();
        pEmptyWin->aTimer.Stop();
        sal_uInt16 nRealState = pEmptyWin->nState;
        FadeOut_Impl();
        pEmptyWin->Hide();
#ifdef DBG_UTIL
        if ( !bPinned || !pEmptyWin->bFadeIn )
        {
            OSL_TRACE( "SfxSplitWindow::RemoveWindow - releasing empty Splitwindow" );
        }
        else
        {
            OSL_TRACE( "SfxSplitWindow::RemoveWindow - releasing real Splitwindow" );
        }
#endif
        pWorkWin->ReleaseChild_Impl( *GetSplitWindow() );
        pEmptyWin->nState = nRealState;
        pWorkWin->ArrangeAutoHideWindows( this );
    }

    sal_uInt16 nCount = pDockArr->size();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        SfxDock_Impl& rDock = (*pDockArr)[n];
        if ( rDock.nType == pDockWin->GetType() )
        {
            rDock.pWin = 0;
            rDock.bHide = bHide;
            break;
        }
    }

    // Remove Windows, and if it was the last of the line, then also remove
    // the line (line = itemset)
    DeactivateUpdateMode* pDeactivateUpdateMode = new DeactivateUpdateMode( *this );
    bLocked = true;

    RemoveItem( pDockWin->GetType() );

    if ( nSet && !GetItemCount( nSet ) )
        RemoveItem( nSet );

    delete pDeactivateUpdateMode;
    bLocked = false;
};



bool SfxSplitWindow::GetWindowPos( const SfxDockingWindow* pWindow,
                                        sal_uInt16& rLine, sal_uInt16& rPos ) const
/*  [Description]

    Returns the ID of the item sets and items for the DockingWindow in
    the position passed on the old row / column-name.
*/

{
    sal_uInt16 nSet = GetSet ( pWindow->GetType() );
    if ( nSet == SPLITWINDOW_ITEM_NOTFOUND )
        return false;

    rPos  = GetItemPos( pWindow->GetType(), nSet );
    rLine = GetItemPos( nSet );
    return true;
}



bool SfxSplitWindow::GetWindowPos( const Point& rTestPos,
                                      sal_uInt16& rLine, sal_uInt16& rPos ) const
/*  [Description]

    Returns the ID of the item sets and items for the DockingWindow in
    the position passed on the old row / column-name.
*/

{
    sal_uInt16 nId = GetItemId( rTestPos );
    if ( nId == 0 )
        return false;

    sal_uInt16 nSet = GetSet ( nId );
    rPos  = GetItemPos( nId, nSet );
    rLine = GetItemPos( nSet );
    return true;
}



sal_uInt16 SfxSplitWindow::GetLineCount() const

/*  [Description]

    Returns the number of rows = number of sub-itemsets in the root set.
*/
{
    return GetItemCount();
}



long SfxSplitWindow::GetLineSize( sal_uInt16 nLine ) const

/*  [Description]

    Returns the Row Height of nline itemset.
*/
{
    sal_uInt16 nId = GetItemId( nLine );
    return GetItemSize( nId );
}



sal_uInt16 SfxSplitWindow::GetWindowCount( sal_uInt16 nLine ) const

/*  [Description]

    Returns the total number of windows
*/
{
    sal_uInt16 nId = GetItemId( nLine );
    return GetItemCount( nId );
}



sal_uInt16 SfxSplitWindow::GetWindowCount() const

/*  [Description]

    Returns the total number of windows
*/
{
    return GetItemCount();
}



void SfxSplitWindow::Command( const CommandEvent& rCEvt )
{
    SplitWindow::Command( rCEvt );
}



IMPL_LINK_TYPED( SfxSplitWindow, TimerHdl, Timer*, pTimer, void)
{
    if ( pTimer )
        pTimer->Stop();

    if ( CursorIsOverRect() || !pTimer )
    {
        // If the cursor is within the window, display the SplitWindow and set
        // up the timer for close
        pEmptyWin->bAutoHide = true;
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
            return;
        }

        // Especially for TF_AUTOSHOW_ON_MOUSEMOVE :
        // If the window is not visible, there is nothing to do
        // (user has simply moved the mouse over pEmptyWin)
        if ( IsVisible() )
        {
            pEmptyWin->bEndAutoHide = false;
            if ( !Application::IsInModalMode() &&
                  !PopupMenu::IsInExecute() &&
                  !pEmptyWin->bSplit && !HasChildPathFocus( true ) )
            {
                // While a modal dialog or a popup menu is open or while the
                // Splitting is done, in any case, do not close. Even as long
                // as one of the Children has the focus, the window remains
                // open.
                pEmptyWin->bEndAutoHide = true;
            }

            if ( pEmptyWin->bEndAutoHide )
            {
               // As far as I am concerned this can be the end of AutoShow
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
}



bool SfxSplitWindow::CursorIsOverRect( bool bForceAdding ) const
{
    bool bVisible = IsVisible();

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

    if ( aRect.IsInside( OutputToScreenPixel( static_cast<vcl::Window*>(const_cast<SfxSplitWindow *>(this))->GetPointerPosPixel() ) ) )
        return true;
    return false;
}



SplitWindow* SfxSplitWindow::GetSplitWindow()
{
    if ( !bPinned || !pEmptyWin->bFadeIn )
        return pEmptyWin;
    return this;
}


bool SfxSplitWindow::IsFadeIn() const
{
    return pEmptyWin->bFadeIn;
}

bool SfxSplitWindow::IsAutoHide( bool bSelf ) const
{
    return bSelf ? pEmptyWin->bAutoHide && !pEmptyWin->bEndAutoHide : pEmptyWin->bAutoHide;
}



void SfxSplitWindow::SetPinned_Impl( bool bOn )
{
    if ( bPinned == bOn )
        return;

    bPinned = bOn;
    if ( GetItemCount() == 0 )
        return;

    if ( !bOn )
    {
        pEmptyWin->nState |= 1;
        if ( pEmptyWin->bFadeIn )
        {
            // Unregister replacement windows
            OSL_TRACE( "SfxSplitWindow::SetPinned_Impl - releasing real Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            OSL_TRACE( "SfxSplitWindow::SetPinned_Impl - registering empty Splitwindow" );
            pWorkWin->RegisterChild_Impl( *pEmptyWin, eAlign, true )->nVisible = SfxChildVisibility::VISIBLE;
        }

        Point aPos( GetPosPixel() );
        aPos = GetParent()->OutputToScreenPixel( aPos );
        SetFloatingPos( aPos );
        SetFloatingMode( true );
        GetFloatingWindow()->SetOutputSizePixel( GetOutputSizePixel() );

        if ( pEmptyWin->bFadeIn )
            Show();
    }
    else
    {
        pEmptyWin->nState &= ~1;
        SetOutputSizePixel( GetFloatingWindow()->GetOutputSizePixel() );
        SetFloatingMode(false);

        if ( pEmptyWin->bFadeIn )
        {
            // Unregister replacement windows
            OSL_TRACE( "SfxSplitWindow::SetPinned_Impl - releasing empty Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            OSL_TRACE( "SfxSplitWindow::SetPinned_Impl - registering real Splitwindow" );
            pWorkWin->RegisterChild_Impl( *this, eAlign, true )->nVisible = SfxChildVisibility::VISIBLE;
        }
    }

    SetAutoHideState( !bPinned );
    pEmptyWin->SetAutoHideState( !bPinned );
}



void SfxSplitWindow::SetFadeIn_Impl( bool bOn )
{
    if ( bOn == pEmptyWin->bFadeIn )
        return;

    if ( GetItemCount() == 0 )
        return;

    pEmptyWin->bFadeIn = bOn;
    if ( bOn )
    {
        pEmptyWin->nState |= 2;
        if ( IsFloatingMode() )
        {
            // FloatingWindow is not visible, thus display it
            pWorkWin->ArrangeAutoHideWindows( this );
            Show();
        }
        else
        {
            OSL_TRACE( "SfxSplitWindow::SetFadeIn_Impl - releasing empty Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *pEmptyWin );
            pEmptyWin->Hide();
            OSL_TRACE( "SfxSplitWindow::SetFadeIn_Impl - registering real Splitwindow" );
            pWorkWin->RegisterChild_Impl( *this, eAlign, true )->nVisible = SfxChildVisibility::VISIBLE;
            pWorkWin->ArrangeChildren_Impl();
            pWorkWin->ShowChildren_Impl();
        }
    }
    else
    {
        pEmptyWin->bAutoHide = false;
        pEmptyWin->nState &= ~2;
        if ( !IsFloatingMode() )
        {
            // The window is not "floating", should be hidden
            OSL_TRACE( "SfxSplitWindow::SetFadeIn_Impl - releasing real Splitwindow" );
            pWorkWin->ReleaseChild_Impl( *this );
            Hide();
            pEmptyWin->Actualize();
            OSL_TRACE( "SfxSplitWindow::SetFadeIn_Impl - registering empty Splitwindow" );
            pWorkWin->RegisterChild_Impl( *pEmptyWin, eAlign, true )->nVisible = SfxChildVisibility::VISIBLE;
            pWorkWin->ArrangeChildren_Impl();
            pWorkWin->ShowChildren_Impl();
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
        SetPinned_Impl( true );
        pWorkWin->ArrangeChildren_Impl();
    }
    else
    {
        // In "limbo"
        SetPinned_Impl( false );
        pWorkWin->ArrangeChildren_Impl();
        pWorkWin->ArrangeAutoHideWindows( this );
    }

    pWorkWin->ShowChildren_Impl();
    SaveConfig_Impl();
}

void SfxSplitWindow::FadeOut_Impl()
{
    if ( pEmptyWin->aTimer.IsActive() )
    {
        pEmptyWin->bAutoHide = false;
        pEmptyWin->aTimer.Stop();
    }

    SetFadeIn_Impl( false );
}

void SfxSplitWindow::FadeOut()
{
    FadeOut_Impl();
    SaveConfig_Impl();
}

void SfxSplitWindow::FadeIn()
{
    SetFadeIn_Impl( true );
}

void SfxSplitWindow::SetActiveWindow_Impl( SfxDockingWindow* pWin )
{
    pActive = pWin;
    pWorkWin->SetActiveChild_Impl( this );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
