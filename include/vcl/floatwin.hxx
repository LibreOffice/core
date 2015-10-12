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

#ifndef INCLUDED_VCL_FLOATWIN_HXX
#define INCLUDED_VCL_FLOATWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>
#include <o3tl/typed_flags_set.hxx>

class ToolBox;


// - FloatingWindow-Types -


enum class FloatWinPopupFlags
{
    NONE                 = 0x000000,
    AllowTearOff         = 0x000001,
    AnimationSlide       = 0x000002,
    NoAutoArrange        = 0x000004,
    NoAnimation          = 0x000008,
    Down                 = 0x000010,
    Up                   = 0x000020,
    Left                 = 0x000040,
    Right                = 0x000080,
    NoFocusClose         = 0x000100,
    NoKeyClose           = 0x000200,
    NoMouseClose         = 0x000400,
    NoMouseRectClose     = 0x000800,
    AllMouseButtonClose  = 0x001000,
    NoAppFocusClose      = 0x002000,
    NewLevel             = 0x004000,
    NoMouseUpClose       = 0x008000,
    GrabFocus            = 0x010000,
    NoHorzPlacement      = 0x020000,
};
namespace o3tl
{
    template<> struct typed_flags<FloatWinPopupFlags> : is_typed_flags<FloatWinPopupFlags, 0x03ffff> {};
}

enum class FloatWinPopupEndFlags
{
    NONE              = 0x00,
    Cancel            = 0x01,
    TearOff           = 0x02,
    DontCallHdl       = 0x04,
    CloseAll          = 0x08,
};
namespace o3tl
{
    template<> struct typed_flags<FloatWinPopupEndFlags> : is_typed_flags<FloatWinPopupEndFlags, 0x0f> {};
}

enum class FloatWinTitleType
{
    Unknown                  = 0,
    Normal                   = 1,
    TearOff                  = 2,
    Popup                    = 3,
    NONE                     = 4,
};

enum HitTest
{
    HITTEST_OUTSIDE,
    HITTEST_WINDOW,
    HITTEST_RECT
};

// - FloatingWindow -

class VCL_DLLPUBLIC FloatingWindow : public SystemWindow
{
    class   ImplData;
private:
    VclPtr<FloatingWindow>  mpNextFloat;
    VclPtr<vcl::Window>     mpFirstPopupModeWin;
    ImplData*       mpImplData;
    Rectangle       maFloatRect;
    ImplSVEvent *   mnPostId;
    FloatWinPopupFlags   mnPopupModeFlags;
    FloatWinTitleType    mnTitle;
    FloatWinTitleType    mnOldTitle;
    bool            mbInPopupMode;
    bool            mbPopupMode;
    bool            mbPopupModeCanceled;
    bool            mbPopupModeTearOff;
    bool            mbMouseDown;
    bool            mbOldSaveBackMode;
    bool            mbGrabFocus;    // act as key input window, although focus is not set
    bool            mbInCleanUp;
    Link<FloatingWindow*,void> maPopupModeEndHdl;

    SAL_DLLPRIVATE void    ImplCallPopupModeEnd();
    DECL_DLLPRIVATE_LINK_TYPED(  ImplEndPopupModeHdl, void*, void );
    virtual void setPosSizeOnContainee(Size aSize, Window &rBox) SAL_OVERRIDE;

                           FloatingWindow (const FloatingWindow &) = delete;
                           FloatingWindow & operator= (const FloatingWindow &) = delete;

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) SAL_OVERRIDE;

public:
    SAL_DLLPRIVATE FloatingWindow*  ImplFloatHitTest( vcl::Window* pReference, const Point& rPos, HitTest& rHitTest );
    SAL_DLLPRIVATE FloatingWindow*  ImplFindLastLevelFloat();
    SAL_DLLPRIVATE bool             ImplIsFloatPopupModeWindow( const vcl::Window* pWindow );
    SAL_DLLPRIVATE void             ImplSetMouseDown() { mbMouseDown = true; }
    SAL_DLLPRIVATE bool             ImplIsMouseDown() const  { return mbMouseDown; }
    SAL_DLLPRIVATE static Point     ImplCalcPos( vcl::Window* pWindow,
                                                 const Rectangle& rRect, FloatWinPopupFlags nFlags,
                                                 sal_uInt16& rArrangeIndex );
    SAL_DLLPRIVATE void             ImplEndPopupMode( FloatWinPopupEndFlags nFlags = FloatWinPopupEndFlags::NONE, sal_uLong nFocusId = 0 );
    SAL_DLLPRIVATE Rectangle&       ImplGetItemEdgeClipRect();
    SAL_DLLPRIVATE bool             ImplIsInPrivatePopupMode() const { return mbInPopupMode; }
    virtual        void             doDeferredInit(WinBits nBits) SAL_OVERRIDE;

public:
    explicit        FloatingWindow(vcl::Window* pParent, WinBits nStyle = WB_STDFLOATWIN);
    explicit        FloatingWindow(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                                   const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
    virtual         ~FloatingWindow();
    virtual void    dispose() SAL_OVERRIDE;

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void    PopupModeEnd();

    void            SetTitleType( FloatWinTitleType nTitle );
    FloatWinTitleType GetTitleType() const { return mnTitle; }

    void            StartPopupMode( const Rectangle& rRect, FloatWinPopupFlags nFlags = FloatWinPopupFlags::NONE );
    void            StartPopupMode( ToolBox* pBox, FloatWinPopupFlags nFlags = FloatWinPopupFlags::NONE  );
    void            EndPopupMode( FloatWinPopupEndFlags nFlags = FloatWinPopupEndFlags::NONE );
    void            AddPopupModeWindow( vcl::Window* pWindow );
    FloatWinPopupFlags GetPopupModeFlags() const { return mnPopupModeFlags; }
    void            SetPopupModeFlags( FloatWinPopupFlags nFlags ) { mnPopupModeFlags = nFlags; }
    bool            IsInPopupMode() const { return mbPopupMode; }
    bool            IsInCleanUp() const { return mbInCleanUp; }
    bool            IsPopupModeCanceled() const { return mbPopupModeCanceled; }
    bool            IsPopupModeTearOff() const { return mbPopupModeTearOff; }

    void            SetPopupModeEndHdl( const Link<FloatingWindow*,void>& rLink ) { maPopupModeEndHdl = rLink; }

    bool            GrabsFocus() const { return mbGrabFocus; }

    static Point    CalcFloatingPosition( vcl::Window* pWindow, const Rectangle& rRect, FloatWinPopupFlags nFlags, sal_uInt16& rArrangeIndex );
};

#endif // INCLUDED_VCL_FLOATWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
