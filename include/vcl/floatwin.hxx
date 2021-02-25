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

#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>
#include <memory>
#include <o3tl/typed_flags_set.hxx>

class ToolBox;

enum class FloatWinPopupEndFlags
{
    NONE              = 0x00,
    Cancel            = 0x01,
    TearOff           = 0x02,
    DontCallHdl       = 0x04,
    CloseAll          = 0x08,
    NoCloseChildren   = 0x10,
};
namespace o3tl
{
    template<> struct typed_flags<FloatWinPopupEndFlags> : is_typed_flags<FloatWinPopupEndFlags, 0x1f> {};
}

enum class FloatWinTitleType
{
    Unknown                  = 0,
    Normal                   = 1,
    TearOff                  = 2,
    Popup                    = 3,
    NONE                     = 4,
};

class VCL_DLLPUBLIC FloatingWindow : public SystemWindow
{
    class SAL_DLLPRIVATE ImplData;
private:
    VclPtr<FloatingWindow>  mpNextFloat;
    VclPtr<vcl::Window>     mpFirstPopupModeWin;
    VclPtr<vcl::Window>     mxPrevFocusWin;
    std::unique_ptr<ImplData> mpImplData;
    tools::Rectangle       maFloatRect;
    ImplSVEvent *   mnPostId;
    FloatWinPopupFlags   mnPopupModeFlags;
    FloatWinTitleType    mnTitle;
    FloatWinTitleType    mnOldTitle;
    bool            mbInPopupMode;
    bool            mbPopupMode;
    bool            mbPopupModeCanceled;
    bool            mbPopupModeTearOff;
    bool            mbMouseDown;
    bool            mbGrabFocus;    // act as key input window, although focus is not set
    bool            mbInCleanUp;
    Link<FloatingWindow*,void> maPopupModeEndHdl;

    SAL_DLLPRIVATE void    ImplCallPopupModeEnd();
    DECL_DLLPRIVATE_LINK(  ImplEndPopupModeHdl, void*, void );

                           FloatingWindow (const FloatingWindow &) = delete;
                           FloatingWindow & operator= (const FloatingWindow &) = delete;

protected:
    SAL_DLLPRIVATE void    ImplInitFloating( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

public:
    SAL_DLLPRIVATE FloatingWindow*  ImplFloatHitTest( vcl::Window* pReference, const Point& rPos, bool& rbHitTestInsideRect );
    SAL_DLLPRIVATE FloatingWindow*  ImplFindLastLevelFloat();
    SAL_DLLPRIVATE bool             ImplIsFloatPopupModeWindow( const vcl::Window* pWindow );
    SAL_DLLPRIVATE void             ImplSetMouseDown() { mbMouseDown = true; }
    SAL_DLLPRIVATE bool             ImplIsMouseDown() const  { return mbMouseDown; }
                   static Point     ImplCalcPos(vcl::Window* pWindow,
                                                const tools::Rectangle& rRect, FloatWinPopupFlags nFlags,
                                                sal_uInt16& rArrangeIndex, Point* pLOKTwipsPos = nullptr);
                   static Point     ImplConvertToAbsPos(vcl::Window* pReference, const Point& rPos);
                   static tools::Rectangle ImplConvertToAbsPos(vcl::Window* pReference, const tools::Rectangle& rRect);
                   static tools::Rectangle ImplConvertToRelPos(vcl::Window* pReference, const tools::Rectangle& rRect);
    SAL_DLLPRIVATE void             ImplEndPopupMode( FloatWinPopupEndFlags nFlags, const VclPtr<vcl::Window>& xFocusId );
    SAL_DLLPRIVATE tools::Rectangle&       ImplGetItemEdgeClipRect();
    SAL_DLLPRIVATE bool             ImplIsInPrivatePopupMode() const { return mbInPopupMode; }
    virtual        void             doDeferredInit(WinBits nBits) override;
                   void             PixelInvalidate(const tools::Rectangle* pRectangle) override;

public:
    explicit        FloatingWindow(vcl::Window* pParent, WinBits nStyle);
    explicit        FloatingWindow(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                                   const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
    virtual         ~FloatingWindow() override;
    virtual void    dispose() override;

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void    PopupModeEnd();

    void            SetTitleType( FloatWinTitleType nTitle );
    FloatWinTitleType GetTitleType() const { return mnTitle; }

    void            StartPopupMode( const tools::Rectangle& rRect, FloatWinPopupFlags nFlags );
    void            StartPopupMode( ToolBox* pBox, FloatWinPopupFlags nFlags  );
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
    bool            UpdatePositionData();

    static Point    CalcFloatingPosition( vcl::Window* pWindow, const tools::Rectangle& rRect, FloatWinPopupFlags nFlags, sal_uInt16& rArrangeIndex );
};

#endif // INCLUDED_VCL_FLOATWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
