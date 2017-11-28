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

#include <vector>
#include <memory>

class SfxWorkWindow;
class SfxDockingWindow;
class SfxEmptySplitWin_Impl;

struct SfxDock_Impl
{
    sal_uInt16        nType;
    VclPtr<SfxDockingWindow> pWin;      // SplitWindow has this window
    bool              bNewLine;
    bool              bHide;     // SplitWindow had this window
};

class SfxSplitWindow : public SplitWindow
{
friend class SfxEmptySplitWin_Impl;

private:
    SfxChildAlignment   eAlign;
    SfxWorkWindow*      pWorkWin;
    std::vector<std::unique_ptr<SfxDock_Impl> >
                        maDockArr;
    bool                bPinned;
    VclPtr<SfxEmptySplitWin_Impl>  pEmptyWin;
    VclPtr<SfxDockingWindow>       pActive;

    void                InsertWindow_Impl( SfxDock_Impl const * pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            bool bNewLine );

    DECL_LINK(    TimerHdl, Timer*, void );
    bool                CursorIsOverRect() const;
    void                SetPinned_Impl( bool );
    void                SetFadeIn_Impl( bool );
    void                SaveConfig_Impl();
    void                FadeOut_Impl();

protected:

    virtual void        StartSplit() override;
    virtual void        SplitResize() override;
    virtual void        Split() override;
    virtual void        MouseButtonDown ( const MouseEvent& ) override;

public:
                        SfxSplitWindow( vcl::Window* pParent, SfxChildAlignment eAl,
                            SfxWorkWindow *pW, bool bWithButtons );

                        virtual ~SfxSplitWindow() override;
    virtual void        dispose() override;

    void                ReleaseWindow_Impl(SfxDockingWindow const *pWin, bool bSaveConfig=true);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize);

    void                InsertWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            bool bNewLine );

    void                MoveWindow( SfxDockingWindow* pDockWin,
                            const Size& rSize,
                            sal_uInt16 nLine,
                            sal_uInt16 nPos,
                            bool bNewLine );

    void                RemoveWindow( SfxDockingWindow const * pDockWin, bool bHide=true);

    void                Lock( bool bLock=true )
                        {
                            SetUpdateMode( !bLock );
                        }

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
    bool                IsAutoHide( bool bSelf ) const;
    SplitWindow*        GetSplitWindow();

    virtual void        FadeOut() override;
    virtual void        FadeIn() override;
    void                SetActiveWindow_Impl( SfxDockingWindow* pWin );
};

#endif // INCLUDED_SFX2_SOURCE_INC_SPLITWIN_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
