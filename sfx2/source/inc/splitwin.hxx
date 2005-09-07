/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: splitwin.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:12:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXSPLITWIN_HXX
#define _SFXSPLITWIN_HXX

#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#include "childwin.hxx"

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


