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
#ifndef INCLUDED_SFX2_SOURCE_INC_SPLITWIN_HXX
#define INCLUDED_SFX2_SOURCE_INC_SPLITWIN_HXX

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
    bool                bLocked;
    bool                bPinned;
    SfxEmptySplitWin_Impl*  pEmptyWin;
    SfxDockingWindow*   pActive;

    void                InsertWindow_Impl( SfxDock_Impl* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            bool bNewLine=false );

    DECL_LINK(          TimerHdl, Timer* );
    bool                CursorIsOverRect( bool bForceAdding = false ) const;
    void                SetPinned_Impl( bool );
    void                SetFadeIn_Impl( bool );
    void                SaveConfig_Impl();
    void                FadeOut_Impl();

protected:

    virtual void        StartSplit() SAL_OVERRIDE;
    virtual void        SplitResize() SAL_OVERRIDE;
    virtual void        Split() SAL_OVERRIDE;
    virtual void        Command ( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void        MouseButtonDown ( const MouseEvent& ) SAL_OVERRIDE;

public:
                        SfxSplitWindow( Window* pParent, SfxChildAlignment eAl,
                            SfxWorkWindow *pW, bool bWithButtons,
                            WinBits nBits = WB_BORDER | WB_SIZEABLE | WB_3DLOOK );

                        virtual ~SfxSplitWindow();

    void                ReleaseWindow_Impl(SfxDockingWindow *pWin, bool bSaveConfig=true);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            bool bNewLine=false );

    void                MoveWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            bool bNewLine=false );

    void                RemoveWindow( SfxDockingWindow* pDockWin, bool bHide=true);

    void                Lock( bool bLock=true )
                        {
                            bLocked = bLock;
                            SetUpdateMode( !bLock );
                        }
        using Window::IsLocked;
    bool                IsLocked() const { return bLocked; }
    bool                GetWindowPos( const SfxDockingWindow* pWindow,
                                      sal_uInt16& rLine, sal_uInt16& rPos ) const;
    bool                GetWindowPos( const Point& rTestPos,
                                      sal_uInt16& rLine, sal_uInt16& rPos ) const;
    sal_uInt16              GetLineCount() const;
    long                GetLineSize( sal_uInt16 ) const;
    sal_uInt16              GetWindowCount(sal_uInt16 nLine) const;
    sal_uInt16              GetWindowCount() const;

    bool                IsPinned() const { return bPinned; }
    bool                IsFadeIn() const;
    bool                IsAutoHide( bool bSelf = false ) const;
    SplitWindow*        GetSplitWindow();

    virtual void        AutoHide() SAL_OVERRIDE;
    virtual void        FadeOut() SAL_OVERRIDE;
    virtual void        FadeIn() SAL_OVERRIDE;
    void                Show_Impl();
    void                Pin_Impl( bool bPinned );
    bool                ActivateNextChild_Impl( bool bForward = true );
    void                SetActiveWindow_Impl( SfxDockingWindow* pWin );
};

#endif // INCLUDED_SFX2_SOURCE_INC_SPLITWIN_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
