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

#ifndef INCLUDED_VCL_INC_BRDWIN_HXX
#define INCLUDED_VCL_INC_BRDWIN_HXX

#include <vcl/window.hxx>

class ImplBorderWindowView;
enum class DrawButtonFlags;

#define BORDERWINDOW_STYLE_OVERLAP          ((sal_uInt16)0x0001)
#define BORDERWINDOW_STYLE_BORDER           ((sal_uInt16)0x0002)
#define BORDERWINDOW_STYLE_FLOAT            ((sal_uInt16)0x0004)
#define BORDERWINDOW_STYLE_FRAME            ((sal_uInt16)0x0008)
#define BORDERWINDOW_STYLE_APP              ((sal_uInt16)0x0010)

#define BORDERWINDOW_HITTEST_TITLE          ((sal_uInt16)0x0001)
#define BORDERWINDOW_HITTEST_LEFT           ((sal_uInt16)0x0002)
#define BORDERWINDOW_HITTEST_MENU           ((sal_uInt16)0x0004)
#define BORDERWINDOW_HITTEST_TOP            ((sal_uInt16)0x0008)
#define BORDERWINDOW_HITTEST_RIGHT          ((sal_uInt16)0x0010)
#define BORDERWINDOW_HITTEST_BOTTOM         ((sal_uInt16)0x0020)
#define BORDERWINDOW_HITTEST_TOPLEFT        ((sal_uInt16)0x0040)
#define BORDERWINDOW_HITTEST_TOPRIGHT       ((sal_uInt16)0x0080)
#define BORDERWINDOW_HITTEST_BOTTOMLEFT     ((sal_uInt16)0x0100)
#define BORDERWINDOW_HITTEST_BOTTOMRIGHT    ((sal_uInt16)0x0200)
#define BORDERWINDOW_HITTEST_CLOSE          ((sal_uInt16)0x0400)
#define BORDERWINDOW_HITTEST_ROLL           ((sal_uInt16)0x0800)
#define BORDERWINDOW_HITTEST_DOCK           ((sal_uInt16)0x1000)
#define BORDERWINDOW_HITTEST_HIDE           ((sal_uInt16)0x2000)
#define BORDERWINDOW_HITTEST_HELP           ((sal_uInt16)0x4000)
#define BORDERWINDOW_HITTEST_PIN            ((sal_uInt16)0x8000)

#define BORDERWINDOW_DRAW_TITLE             ((sal_uInt16)0x0001)
#define BORDERWINDOW_DRAW_BORDER            ((sal_uInt16)0x0002)
#define BORDERWINDOW_DRAW_FRAME             ((sal_uInt16)0x0004)
#define BORDERWINDOW_DRAW_CLOSE             ((sal_uInt16)0x0008)
#define BORDERWINDOW_DRAW_ROLL              ((sal_uInt16)0x0010)
#define BORDERWINDOW_DRAW_DOCK              ((sal_uInt16)0x0020)
#define BORDERWINDOW_DRAW_HIDE              ((sal_uInt16)0x0040)
#define BORDERWINDOW_DRAW_HELP              ((sal_uInt16)0x0080)
#define BORDERWINDOW_DRAW_PIN               ((sal_uInt16)0x0100)
#define BORDERWINDOW_DRAW_MENU              ((sal_uInt16)0x0200)
#define BORDERWINDOW_DRAW_ALL               (BORDERWINDOW_DRAW_TITLE |      \
                                             BORDERWINDOW_DRAW_BORDER |     \
                                             BORDERWINDOW_DRAW_FRAME |      \
                                             BORDERWINDOW_DRAW_CLOSE |      \
                                             BORDERWINDOW_DRAW_ROLL |       \
                                             BORDERWINDOW_DRAW_DOCK |       \
                                             BORDERWINDOW_DRAW_HIDE |       \
                                             BORDERWINDOW_DRAW_HELP |       \
                                             BORDERWINDOW_DRAW_PIN  |       \
                                             BORDERWINDOW_DRAW_MENU)

#define BORDERWINDOW_TITLE_NORMAL           ((sal_uInt16)0x0001)
#define BORDERWINDOW_TITLE_SMALL            ((sal_uInt16)0x0002)
#define BORDERWINDOW_TITLE_TEAROFF          ((sal_uInt16)0x0004)
#define BORDERWINDOW_TITLE_POPUP            ((sal_uInt16)0x0008)
#define BORDERWINDOW_TITLE_NONE             ((sal_uInt16)0x0010)

class ImplBorderWindow : public vcl::Window
{
    friend class vcl::Window;
    friend class ImplBorderWindowView;
    friend class ImplSmallBorderWindowView;
    friend class ImplStdBorderWindowView;

private:
    ImplBorderWindowView*   mpBorderView;
    VclPtr<vcl::Window>     mpMenuBarWindow;
    long                    mnMinWidth;
    long                    mnMinHeight;
    long                    mnMaxWidth;
    long                    mnMaxHeight;
    long                    mnRollHeight;
    long                    mnOrgMenuHeight;
    sal_uInt16              mnTitleType;
    WindowBorderStyle       mnBorderStyle;
    bool                    mbFloatWindow;
    bool                    mbSmallOutBorder;
    bool                    mbFrameBorder;
    bool                    mbPinned;
    bool                    mbRollUp;
    bool                    mbMenuHide;
    bool                    mbDockBtn;
    bool                    mbHideBtn;
    bool                    mbMenuBtn;
    bool                    mbDisplayActive;

    using Window::ImplInit;
    void                    ImplInit( vcl::Window* pParent,
                                      WinBits nStyle, sal_uInt16 nTypeStyle,
                                      SystemParentData* pParentData );
    void                    ImplInit( vcl::Window* pParent,
                                      WinBits nStyle, sal_uInt16 nTypeStyle,
                                      const css::uno::Any& );

                            ImplBorderWindow (const ImplBorderWindow &) = delete;
                            ImplBorderWindow& operator= (const ImplBorderWindow &) = delete;

public:
                            ImplBorderWindow( vcl::Window* pParent,
                                              SystemParentData* pParentData,
                                              WinBits nStyle = 0,
                                              sal_uInt16 nTypeStyle = 0 );
                            ImplBorderWindow( vcl::Window* pParent, WinBits nStyle = 0,
                                              sal_uInt16 nTypeStyle = 0 );
    virtual                 ~ImplBorderWindow();
    virtual void            dispose() override;

    virtual void            MouseMove( const MouseEvent& rMEvt ) override;
    virtual void            MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void            Tracking( const TrackingEvent& rTEvt ) override;
    virtual void            Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void            Activate() override;
    virtual void            Deactivate() override;
    virtual void            Resize() override;
    virtual void            RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void            StateChanged( StateChangedType nType ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;

    void                    InitView();
    void                    UpdateView( bool bNewView, const Size& rNewOutSize );
    void                    InvalidateBorder();

    using Window::Draw;
    void                    Draw( const Rectangle& rRect, OutputDevice* pDev, const Point& rPos );

    void                    SetDisplayActive( bool bActive );
    void                    SetTitleType( sal_uInt16 nTitleType, const Size& rSize );
    void                    SetBorderStyle( WindowBorderStyle nStyle );
    WindowBorderStyle       GetBorderStyle() const { return mnBorderStyle; }
    void                    SetPin( bool bPin );
    void                    SetRollUp( bool bRollUp, const Size& rSize );
    void                    SetCloseButton();
    void                    SetDockButton( bool bDockButton );
    void                    SetHideButton( bool bHideButton );
    void                    SetMenuButton( bool bMenuButton );

    void                    UpdateMenuHeight();
    void                    SetMenuBarWindow( vcl::Window* pWindow );
    void                    SetMenuBarMode( bool bHide );

    void                    SetMinOutputSize( long nWidth, long nHeight )
                                { mnMinWidth = nWidth; mnMinHeight = nHeight; }
    void                    SetMaxOutputSize( long nWidth, long nHeight )
                                { mnMaxWidth = nWidth; mnMaxHeight = nHeight; }

    void                    GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    long                    CalcTitleWidth() const;

    Rectangle               GetMenuRect() const;

    virtual Size            GetOptimalSize() const override;
};

struct ImplBorderFrameData
{
    VclPtr<ImplBorderWindow> mpBorderWindow;
    VclPtr<OutputDevice>     mpOutDev;
    Rectangle                maTitleRect;
    Rectangle                maPinRect;
    Rectangle                maCloseRect;
    Rectangle                maRollRect;
    Rectangle                maDockRect;
    Rectangle                maMenuRect;
    Rectangle                maHideRect;
    Rectangle                maHelpRect;
    Point                    maMouseOff;
    long                     mnWidth;
    long                     mnHeight;
    long                     mnTrackX;
    long                     mnTrackY;
    long                     mnTrackWidth;
    long                     mnTrackHeight;
    sal_Int32                mnLeftBorder;
    sal_Int32                mnTopBorder;
    sal_Int32                mnRightBorder;
    sal_Int32                mnBottomBorder;
    long                     mnNoTitleTop;
    long                     mnBorderSize;
    long                     mnTitleHeight;
    sal_uInt16               mnHitTest;
    DrawButtonFlags          mnPinState;
    DrawButtonFlags          mnCloseState;
    DrawButtonFlags          mnRollState;
    DrawButtonFlags          mnDockState;
    DrawButtonFlags          mnMenuState;
    DrawButtonFlags          mnHideState;
    DrawButtonFlags          mnHelpState;
    sal_uInt16               mnTitleType;
    bool                     mbFloatWindow;
    bool                     mbDragFull;
    bool                     mbTitleClipped;
};

class ImplBorderWindowView
{
public:
    virtual                 ~ImplBorderWindowView();

    virtual bool            MouseMove( const MouseEvent& rMEvt );
    virtual bool            MouseButtonDown( const MouseEvent& rMEvt );
    virtual bool            Tracking( const TrackingEvent& rTEvt );
    virtual OUString        RequestHelp( const Point& rPos, Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight ) = 0;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const = 0;
    virtual long            CalcTitleWidth() const = 0;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags, const Point* pOffset = nullptr) = 0;
    virtual Rectangle       GetMenuRect() const;

    static void             ImplInitTitle( ImplBorderFrameData* pData );
    static sal_uInt16       ImplHitTest( ImplBorderFrameData* pData, const Point& rPos );
    static bool             ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt );
    static OUString         ImplRequestHelp( ImplBorderFrameData* pData, const Point& rPos, Rectangle& rHelpRect );
    static long             ImplCalcTitleWidth( const ImplBorderFrameData* pData );
};

class ImplNoBorderWindowView : public ImplBorderWindowView
{
public:
                            ImplNoBorderWindowView( ImplBorderWindow* pBorderWindow );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight ) override;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const override;
    virtual long            CalcTitleWidth() const override;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags, const Point* pOffset) override;
};

class ImplSmallBorderWindowView : public ImplBorderWindowView
{
    VclPtr<ImplBorderWindow> mpBorderWindow;
    VclPtr<OutputDevice>     mpOutDev;
    long                    mnWidth;
    long                    mnHeight;
    sal_Int32               mnLeftBorder;
    sal_Int32               mnTopBorder;
    sal_Int32               mnRightBorder;
    sal_Int32               mnBottomBorder;
    bool                    mbNWFBorder;

public:
                            ImplSmallBorderWindowView( ImplBorderWindow* pBorderWindow );

    virtual void            Init( OutputDevice* pOutDev, long nWidth, long nHeight ) override;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const override;
    virtual long            CalcTitleWidth() const override;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags, const Point* pOffset) override;
};

class ImplStdBorderWindowView : public ImplBorderWindowView
{
    ImplBorderFrameData     maFrameData;
    VclPtr<VirtualDevice>   mpATitleVirDev;
    VclPtr<VirtualDevice>   mpDTitleVirDev;

public:
                            ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow );
                            virtual ~ImplStdBorderWindowView();

    virtual bool        MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool        Tracking( const TrackingEvent& rTEvt ) override;
    virtual OUString        RequestHelp( const Point& rPos, Rectangle& rHelpRect ) override;
    virtual Rectangle       GetMenuRect() const override;

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight ) override;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const override;
    virtual long            CalcTitleWidth() const override;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags, const Point* pOffset) override;
};

#endif // INCLUDED_VCL_INC_BRDWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
