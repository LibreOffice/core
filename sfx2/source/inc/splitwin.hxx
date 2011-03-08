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
#ifndef _SFXSPLITWIN_HXX
#define _SFXSPLITWIN_HXX

#include <vcl/splitwin.hxx>
#include <sfx2/childwin.hxx>

class SfxWorkWindow;
class SfxDockingWindow;
class SfxDockArr_Impl;
class SfxEmptySplitWin_Impl;
struct SfxDock_Impl;

class SfxSplitWindow : public SplitWindow
{
friend class SfxEmptySplitWin_Impl;

private:
    SfxChildAlignment   eAlign;
    SfxWorkWindow*      pWorkWin;
    SfxDockArr_Impl*    pDockArr;
    BOOL                bLocked;
    BOOL                bPinned;
    SfxEmptySplitWin_Impl*  pEmptyWin;
    SfxDockingWindow*   pActive;

    void                InsertWindow_Impl( SfxDock_Impl* pDockWin,
                            const Size& rSize,
                            USHORT nLine,
                            USHORT nPos,
                            BOOL bNewLine=FALSE );

    DECL_LINK(          TimerHdl, Timer* );
    BOOL                CursorIsOverRect( BOOL bForceAdding = FALSE ) const;
    void                SetPinned_Impl( BOOL );
    void                SetFadeIn_Impl( BOOL );
    void                SaveConfig_Impl();
    void                FadeOut_Impl();

protected:

    virtual void        StartSplit();
    virtual void        SplitResize();
    virtual void        Split();
    virtual void        Command ( const CommandEvent& rCEvt );
    virtual void        MouseButtonDown ( const MouseEvent& );

public:
                        SfxSplitWindow( Window* pParent, SfxChildAlignment eAl,
                            SfxWorkWindow *pW, BOOL bWithButtons,
                            WinBits nBits = WB_BORDER | WB_SIZEABLE | WB_3DLOOK );

                        ~SfxSplitWindow();

    void                ReleaseWindow_Impl(SfxDockingWindow *pWin, BOOL bSaveConfig=TRUE);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            USHORT nLine,
                            USHORT nPos,
                            BOOL bNewLine=FALSE );

    void                MoveWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            USHORT nLine,
                            USHORT nPos,
                            BOOL bNewLine=FALSE );

    void                RemoveWindow( SfxDockingWindow* pDockWin, BOOL bHide=TRUE);

    void                Lock( BOOL bLock=TRUE )
                        {
                            bLocked = bLock;
                            SetUpdateMode( !bLock );
                        }
        using Window::IsLocked;
    BOOL                IsLocked() const { return bLocked; }
    BOOL                GetWindowPos( const SfxDockingWindow* pWindow,
                                      USHORT& rLine, USHORT& rPos ) const;
    BOOL                GetWindowPos( const Point& rTestPos,
                                      USHORT& rLine, USHORT& rPos ) const;
    USHORT              GetLineCount() const;
    long                GetLineSize( USHORT ) const;
    USHORT              GetWindowCount(USHORT nLine) const;
    USHORT              GetWindowCount() const;

    BOOL                IsPinned() const { return bPinned; }
    BOOL                IsFadeIn() const;
    BOOL                IsAutoHide( BOOL bSelf = FALSE ) const;
    SplitWindow*        GetSplitWindow();

    virtual void        AutoHide();
    virtual void        FadeOut();
    virtual void        FadeIn();
    void                Show_Impl();
    void                Pin_Impl( BOOL bPinned );
    BOOL                ActivateNextChild_Impl( BOOL bForward = TRUE );
    void                SetActiveWindow_Impl( SfxDockingWindow* pWin );
};

#endif // #ifndef _SFXSPLITWIN_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
