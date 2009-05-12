/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: brdwin.hxx,v $
 * $Revision: 1.4 $
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

#include <vcl/sv.h>
#include <vcl/window.hxx>

class ImplBorderWindowView;

// --------------------------
// - ImplBorderWindow-Types -
// --------------------------

#define BORDERWINDOW_STYLE_OVERLAP          ((USHORT)0x0001)
#define BORDERWINDOW_STYLE_BORDER           ((USHORT)0x0002)
#define BORDERWINDOW_STYLE_FLOAT            ((USHORT)0x0004)
#define BORDERWINDOW_STYLE_FRAME            ((USHORT)0x0008)
#define BORDERWINDOW_STYLE_APP              ((USHORT)0x0010)

#define BORDERWINDOW_HITTEST_TITLE          ((USHORT)0x0001)
#define BORDERWINDOW_HITTEST_LEFT           ((USHORT)0x0002)
#define BORDERWINDOW_HITTEST_MENU           ((USHORT)0x0004)
#define BORDERWINDOW_HITTEST_TOP            ((USHORT)0x0008)
#define BORDERWINDOW_HITTEST_RIGHT          ((USHORT)0x0010)
#define BORDERWINDOW_HITTEST_BOTTOM         ((USHORT)0x0020)
#define BORDERWINDOW_HITTEST_TOPLEFT        ((USHORT)0x0040)
#define BORDERWINDOW_HITTEST_TOPRIGHT       ((USHORT)0x0080)
#define BORDERWINDOW_HITTEST_BOTTOMLEFT     ((USHORT)0x0100)
#define BORDERWINDOW_HITTEST_BOTTOMRIGHT    ((USHORT)0x0200)
#define BORDERWINDOW_HITTEST_CLOSE          ((USHORT)0x0400)
#define BORDERWINDOW_HITTEST_ROLL           ((USHORT)0x0800)
#define BORDERWINDOW_HITTEST_DOCK           ((USHORT)0x1000)
#define BORDERWINDOW_HITTEST_HIDE           ((USHORT)0x2000)
#define BORDERWINDOW_HITTEST_HELP           ((USHORT)0x4000)
#define BORDERWINDOW_HITTEST_PIN            ((USHORT)0x8000)

#define BORDERWINDOW_DRAW_TITLE             ((USHORT)0x0001)
#define BORDERWINDOW_DRAW_BORDER            ((USHORT)0x0002)
#define BORDERWINDOW_DRAW_FRAME             ((USHORT)0x0004)
#define BORDERWINDOW_DRAW_CLOSE             ((USHORT)0x0008)
#define BORDERWINDOW_DRAW_ROLL              ((USHORT)0x0010)
#define BORDERWINDOW_DRAW_DOCK              ((USHORT)0x0020)
#define BORDERWINDOW_DRAW_HIDE              ((USHORT)0x0040)
#define BORDERWINDOW_DRAW_HELP              ((USHORT)0x0080)
#define BORDERWINDOW_DRAW_PIN               ((USHORT)0x0100)
#define BORDERWINDOW_DRAW_MENU              ((USHORT)0x0200)
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

#define BORDERWINDOW_TITLE_NORMAL           ((USHORT)0x0001)
#define BORDERWINDOW_TITLE_SMALL            ((USHORT)0x0002)
#define BORDERWINDOW_TITLE_TEAROFF          ((USHORT)0x0004)
#define BORDERWINDOW_TITLE_NONE             ((USHORT)0x0008)

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
    USHORT                  mnTitleType;
    USHORT                  mnBorderStyle;
    BOOL                    mbFloatWindow;
    BOOL                    mbSmallOutBorder;
    BOOL                    mbFrameBorder;
    BOOL                    mbPined;
    BOOL                    mbRollUp;
    BOOL                    mbMenuHide;
    BOOL                    mbDockBtn;
    BOOL                    mbHideBtn;
    BOOL                    mbHelpBtn;
    BOOL                    mbMenuBtn;
    BOOL                    mbDisplayActive;

    using Window::ImplInit;
    void                    ImplInit( Window* pParent,
                                      WinBits nStyle, USHORT nTypeStyle,
                                      SystemParentData* pParentData );
    void                    ImplInit( Window* pParent,
                                      WinBits nStyle, USHORT nTypeStyle,
                                      const ::com::sun::star::uno::Any& );

                            // Copy assignment is forbidden and not implemented.
                            ImplBorderWindow (const ImplBorderWindow &);
                            ImplBorderWindow& operator= (const ImplBorderWindow &);

public:
                            ImplBorderWindow( Window* pParent,
                                              SystemParentData* pParentData,
                                              WinBits nStyle = 0,
                                              USHORT nTypeStyle = 0 );
                            ImplBorderWindow( Window* pParent, WinBits nStyle = 0,
                                              USHORT nTypeStyle = 0 );
                            ImplBorderWindow( Window* pParent,
                                              WinBits nStyle, USHORT nTypeStyle,
                                              const ::com::sun::star::uno::Any& );
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
    void                    UpdateView( BOOL bNewView, const Size& rNewOutSize );
    void                    InvalidateBorder();

    using Window::Draw;
    void                    Draw( const Rectangle& rRect, OutputDevice* pDev, const Point& rPos );

    void                    SetDisplayActive( BOOL bActive );
    BOOL                    IsDisplayActive() const { return mbDisplayActive; }
    void                    SetTitleType( USHORT nTitleType, const Size& rSize );
    void                    SetBorderStyle( USHORT nStyle );
    USHORT                  GetBorderStyle() const { return mnBorderStyle; }
    void                    SetPin( BOOL bPin );
    void                    SetRollUp( BOOL bRollUp, const Size& rSize );
    void                    SetCloser();
    void                    SetDockButton( BOOL bDockButton );
    void                    SetHideButton( BOOL bHideButton );
    void                    SetHelpButton( BOOL bHelpButton );
    void                    SetMenuButton( BOOL bMenuButton );

    void                    UpdateMenuHeight();
    void                    SetMenuBarWindow( Window* pWindow );
    void                    SetMenuBarMode( BOOL bHide );

    void                    SetMinOutputSize( long nWidth, long nHeight )
                                { mnMinWidth = nWidth; mnMinHeight = nHeight; }
    void                    SetMaxOutputSize( long nWidth, long nHeight )
                                { mnMaxWidth = nWidth; mnMaxHeight = nHeight; }

    void                    GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    long                    CalcTitleWidth() const;

    Rectangle               GetMenuRect() const;
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
    USHORT                  mnHitTest;
    USHORT                  mnPinState;
    USHORT                  mnCloseState;
    USHORT                  mnRollState;
    USHORT                  mnDockState;
    USHORT                  mnMenuState;
    USHORT                  mnHideState;
    USHORT                  mnHelpState;
    USHORT                  mnTitleType;
    BOOL                    mbFloatWindow;
    BOOL                    mbDragFull;
    BOOL                    mbTitleClipped;
};

// =======================================================================

// ------------------------
// - ImplBorderWindowView -
// ------------------------

class ImplBorderWindowView
{
public:
    virtual                 ~ImplBorderWindowView();

    virtual BOOL            MouseMove( const MouseEvent& rMEvt );
    virtual BOOL            MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL            Tracking( const TrackingEvent& rTEvt );
    virtual String          RequestHelp( const Point& rPos, Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight ) = 0;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const = 0;
    virtual long            CalcTitleWidth() const = 0;
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev = NULL, const Point* pOffset = NULL ) = 0;
    virtual Rectangle       GetMenuRect() const;

    void                    ImplInitTitle( ImplBorderFrameData* pData );
    USHORT                  ImplHitTest( ImplBorderFrameData* pData, const Point& rPos );
    BOOL                    ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt );
    BOOL                    ImplMouseButtonDown( ImplBorderFrameData* pData, const MouseEvent& rMEvt );
    BOOL                    ImplTracking( ImplBorderFrameData* pData, const TrackingEvent& rTEvt );
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
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );
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
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );
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

    virtual BOOL            MouseMove( const MouseEvent& rMEvt );
    virtual BOOL            MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL            Tracking( const TrackingEvent& rTEvt );
    virtual String          RequestHelp( const Point& rPos, Rectangle& rHelpRect );
    virtual Rectangle       GetMenuRect() const;

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight );
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );
};


#endif  // _SV_BRDWIN_HXX
