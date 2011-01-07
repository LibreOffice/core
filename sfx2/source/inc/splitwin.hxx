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

#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
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
    sal_Bool                bLocked;
    sal_Bool                bPinned;
    SfxEmptySplitWin_Impl*  pEmptyWin;
    SfxDockingWindow*   pActive;

    void                InsertWindow_Impl( SfxDock_Impl* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            sal_Bool bNewLine=sal_False );

    DECL_LINK(          TimerHdl, Timer* );
    sal_Bool                CursorIsOverRect( sal_Bool bForceAdding = sal_False ) const;
    void                SetPinned_Impl( sal_Bool );
    void                SetFadeIn_Impl( sal_Bool );
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
                            SfxWorkWindow *pW, sal_Bool bWithButtons,
                            WinBits nBits = WB_BORDER | WB_SIZEABLE | WB_3DLOOK );

                        ~SfxSplitWindow();

    void                ReleaseWindow_Impl(SfxDockingWindow *pWin, sal_Bool bSaveConfig=sal_True);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            sal_Bool bNewLine=sal_False );

    void                MoveWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            sal_Bool bNewLine=sal_False );

    void                RemoveWindow( SfxDockingWindow* pDockWin, sal_Bool bHide=sal_True);

    void                Lock( sal_Bool bLock=sal_True )
                        {
                            bLocked = bLock;
                            SetUpdateMode( !bLock );
                        }
        using Window::IsLocked;
    sal_Bool                IsLocked() const { return bLocked; }
    sal_Bool                GetWindowPos( const SfxDockingWindow* pWindow,
                                      sal_uInt16& rLine, sal_uInt16& rPos ) const;
    sal_Bool                GetWindowPos( const Point& rTestPos,
                                      sal_uInt16& rLine, sal_uInt16& rPos ) const;
    sal_uInt16              GetLineCount() const;
    long                GetLineSize( sal_uInt16 ) const;
    sal_uInt16              GetWindowCount(sal_uInt16 nLine) const;
    sal_uInt16              GetWindowCount() const;

    sal_Bool                IsPinned() const { return bPinned; }
    sal_Bool                IsFadeIn() const;
    sal_Bool                IsAutoHide( sal_Bool bSelf = sal_False ) const;
    SplitWindow*        GetSplitWindow();

    virtual void        AutoHide();
    virtual void        FadeOut();
    virtual void        FadeIn();
    void                Show_Impl();
    void                Pin_Impl( sal_Bool bPinned );
    sal_Bool                ActivateNextChild_Impl( sal_Bool bForward = sal_True );
    void                SetActiveWindow_Impl( SfxDockingWindow* pWin );
};

#endif // #ifndef _SFXSPLITWIN_HXX


