/*************************************************************************
 *
 *  $RCSfile: splitwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2001-02-19 11:54:08 $
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
#ifndef _SFXSPLITWIN_HXX
#define _SFXSPLITWIN_HXX

#ifndef _SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#include "childwin.hxx"

class SfxWorkWindow;
class SfxDockingWindow;
class SfxDockArr_Impl;
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

protected:

    virtual void        StartSplit();
    virtual void        SplitResize();
    virtual void        Split();
    virtual void        Command ( const CommandEvent& rCEvt );
    virtual void        MouseButtonDown ( const MouseEvent& );
    virtual BOOL        QueryDrop( DropEvent& rEvt );

public:
                        SfxSplitWindow( Window* pParent, SfxChildAlignment eAl,
                            SfxWorkWindow *pW, BOOL bWithButtons,
                            WinBits nBits = WB_BORDER | WB_SIZEABLE | WB_3DLOOK );

                        ~SfxSplitWindow();

    void                ReleaseWindow_Impl(SfxDockingWindow *pWin);

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


