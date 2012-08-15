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

#ifndef _SV_BRDWIN_HXX
#define _SV_BRDWIN_HXX

#include <tools/solar.h>
#include <vcl/window.hxx>

class ImplBorderWindowView;

// --------------------------
// - ImplBorderWindow-Types -
// --------------------------

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

// --------------------
// - ImplBorderWindow -
// --------------------

class ImplBorderWindow : public Window
{
    friend class Window;
    friend class ImplBorderWindowView;
    friend class ImplSmallBorderWindowView;
    friend class ImplStdBorderWindowView;

private:
    ImplBorderWindowView*   mpBorderView;
    Window*                 mpMenuBarWindow;
    long                    mnMinWidth;
    long                    mnMinHeight;
    long                    mnMaxWidth;
    long                    mnMaxHeight;
    long                    mnRollHeight;
    long                    mnOrgMenuHeight;
    sal_uInt16                  mnTitleType;
    sal_uInt16                  mnBorderStyle;
    sal_Bool                    mbFloatWindow;
    sal_Bool                    mbSmallOutBorder;
    sal_Bool                    mbFrameBorder;
    sal_Bool                    mbPined;
    sal_Bool                    mbRollUp;
    sal_Bool                    mbMenuHide;
    sal_Bool                    mbDockBtn;
    sal_Bool                    mbHideBtn;
    sal_Bool                    mbMenuBtn;
    sal_Bool                    mbDisplayActive;

    using Window::ImplInit;
    void                    ImplInit( Window* pParent,
                                      WinBits nStyle, sal_uInt16 nTypeStyle,
                                      SystemParentData* pParentData );
    void                    ImplInit( Window* pParent,
                                      WinBits nStyle, sal_uInt16 nTypeStyle,
                                      const ::com::sun::star::uno::Any& );

                            // Copy assignment is forbidden and not implemented.
                            ImplBorderWindow (const ImplBorderWindow &);
                            ImplBorderWindow& operator= (const ImplBorderWindow &);

public:
                            ImplBorderWindow( Window* pParent,
                                              SystemParentData* pParentData,
                                              WinBits nStyle = 0,
                                              sal_uInt16 nTypeStyle = 0 );
                            ImplBorderWindow( Window* pParent, WinBits nStyle = 0,
                                              sal_uInt16 nTypeStyle = 0 );
                            ~ImplBorderWindow();

    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            Tracking( const TrackingEvent& rTEvt );
    virtual void            Paint( const Rectangle& rRect );
    virtual void            Activate();
    virtual void            Deactivate();
    virtual void            Resize();
    virtual void            RequestHelp( const HelpEvent& rHEvt );
    virtual void            StateChanged( StateChangedType nType );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    void                    InitView();
    void                    UpdateView( sal_Bool bNewView, const Size& rNewOutSize );
    void                    InvalidateBorder();

    using Window::Draw;
    void                    Draw( const Rectangle& rRect, OutputDevice* pDev, const Point& rPos );

    void                    SetDisplayActive( sal_Bool bActive );
    sal_Bool                    IsDisplayActive() const { return mbDisplayActive; }
    void                    SetTitleType( sal_uInt16 nTitleType, const Size& rSize );
    void                    SetBorderStyle( sal_uInt16 nStyle );
    sal_uInt16                  GetBorderStyle() const { return mnBorderStyle; }
    void                    SetPin( sal_Bool bPin );
    void                    SetRollUp( sal_Bool bRollUp, const Size& rSize );
    void                    SetCloser();
    void                    SetDockButton( sal_Bool bDockButton );
    void                    SetHideButton( sal_Bool bHideButton );
    void                    SetMenuButton( sal_Bool bMenuButton );

    void                    UpdateMenuHeight();
    void                    SetMenuBarWindow( Window* pWindow );
    void                    SetMenuBarMode( sal_Bool bHide );

    void                    SetMinOutputSize( long nWidth, long nHeight )
                                { mnMinWidth = nWidth; mnMinHeight = nHeight; }
    void                    SetMaxOutputSize( long nWidth, long nHeight )
                                { mnMaxWidth = nWidth; mnMaxHeight = nHeight; }

    void                    GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    long                    CalcTitleWidth() const;

    Rectangle               GetMenuRect() const;

    virtual Size            GetOptimalSize(WindowSizeType eType) const;
};

// =======================================================================

// -----------------------
// - ImplBorderFrameData -
// -----------------------

struct ImplBorderFrameData
{
    ImplBorderWindow*       mpBorderWindow;
    OutputDevice*           mpOutDev;
    Rectangle               maTitleRect;
    Rectangle               maPinRect;
    Rectangle               maCloseRect;
    Rectangle               maRollRect;
    Rectangle               maDockRect;
    Rectangle               maMenuRect;
    Rectangle               maHideRect;
    Rectangle               maHelpRect;
    Point                   maMouseOff;
    long                    mnWidth;
    long                    mnHeight;
    long                    mnTrackX;
    long                    mnTrackY;
    long                    mnTrackWidth;
    long                    mnTrackHeight;
    sal_Int32               mnLeftBorder;
    sal_Int32               mnTopBorder;
    sal_Int32               mnRightBorder;
    sal_Int32               mnBottomBorder;
    long                    mnNoTitleTop;
    long                    mnBorderSize;
    long                    mnTitleHeight;
    long                    mnTitleOff;
    sal_uInt16                  mnHitTest;
    sal_uInt16                  mnPinState;
    sal_uInt16                  mnCloseState;
    sal_uInt16                  mnRollState;
    sal_uInt16                  mnDockState;
    sal_uInt16                  mnMenuState;
    sal_uInt16                  mnHideState;
    sal_uInt16                  mnHelpState;
    sal_uInt16                  mnTitleType;
    sal_Bool                    mbFloatWindow;
    sal_Bool                    mbDragFull;
    sal_Bool                    mbTitleClipped;
};

// =======================================================================

// ------------------------
// - ImplBorderWindowView -
// ------------------------

class ImplBorderWindowView
{
public:
    virtual                 ~ImplBorderWindowView();

    virtual sal_Bool            MouseMove( const MouseEvent& rMEvt );
    virtual sal_Bool            MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool            Tracking( const TrackingEvent& rTEvt );
    virtual String          RequestHelp( const Point& rPos, Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight ) = 0;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const = 0;
    virtual long            CalcTitleWidth() const = 0;
    virtual void            DrawWindow( sal_uInt16 nDrawFlags, OutputDevice* pOutDev = NULL, const Point* pOffset = NULL ) = 0;
    virtual Rectangle       GetMenuRect() const;

    void                    ImplInitTitle( ImplBorderFrameData* pData );
    sal_uInt16                  ImplHitTest( ImplBorderFrameData* pData, const Point& rPos );
    sal_Bool                    ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt );
    sal_Bool                    ImplMouseButtonDown( ImplBorderFrameData* pData, const MouseEvent& rMEvt );
    sal_Bool                    ImplTracking( ImplBorderFrameData* pData, const TrackingEvent& rTEvt );
    String                  ImplRequestHelp( ImplBorderFrameData* pData, const Point& rPos, Rectangle& rHelpRect );
    long                    ImplCalcTitleWidth( const ImplBorderFrameData* pData ) const;
};

// =======================================================================

// --------------------------
// - ImplNoBorderWindowView -
// --------------------------

class ImplNoBorderWindowView : public ImplBorderWindowView
{
public:
                            ImplNoBorderWindowView( ImplBorderWindow* pBorderWindow );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight );
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( sal_uInt16 nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );
};


// =======================================================================

// -----------------------------
// - ImplSmallBorderWindowView -
// -----------------------------

class ImplSmallBorderWindowView : public ImplBorderWindowView
{
    ImplBorderWindow*       mpBorderWindow;
    OutputDevice*           mpOutDev;
    long                    mnWidth;
    long                    mnHeight;
    sal_Int32               mnLeftBorder;
    sal_Int32               mnTopBorder;
    sal_Int32               mnRightBorder;
    sal_Int32               mnBottomBorder;
    bool                    mbNWFBorder;

public:
                            ImplSmallBorderWindowView( ImplBorderWindow* pBorderWindow );

    virtual void            Init( OutputDevice* pOutDev, long nWidth, long nHeight );
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( sal_uInt16 nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );
};


// =======================================================================

// ---------------------------
// - ImplStdBorderWindowView -
// ---------------------------


class ImplStdBorderWindowView : public ImplBorderWindowView
{
    ImplBorderFrameData     maFrameData;
    VirtualDevice*          mpATitleVirDev;
    VirtualDevice*          mpDTitleVirDev;

public:
                            ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow );
                            ~ImplStdBorderWindowView();

    virtual sal_Bool            MouseMove( const MouseEvent& rMEvt );
    virtual sal_Bool            MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Bool            Tracking( const TrackingEvent& rTEvt );
    virtual String          RequestHelp( const Point& rPos, Rectangle& rHelpRect );
    virtual Rectangle       GetMenuRect() const;

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight );
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( sal_uInt16 nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );
};


#endif  // _SV_BRDWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
