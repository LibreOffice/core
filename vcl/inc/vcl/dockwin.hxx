/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dockwin.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_DOCKWIN_HXX
#define _SV_DOCKWIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/floatwin.hxx>
#include <vector>

// data to be sent with docking events
struct DockingData
{
    Point       maMousePos;     // in
    Rectangle   maTrackRect;    // in/out
    BOOL        mbFloating;     // out
    BOOL        mbLivemode;     // in
    BOOL        mbInteractive;  // in

    DockingData() {};
    DockingData( const Point& rPt, const Rectangle& rRect, BOOL b) :
        maMousePos( rPt ), maTrackRect( rRect ), mbFloating( b ), mbLivemode( FALSE ), mbInteractive( TRUE )
        {};
};

struct EndDockingData
{
    Rectangle   maWindowRect;    // in
    BOOL        mbFloating;      // in
    BOOL        mbCancelled;     // in

    EndDockingData() {};
    EndDockingData( const Rectangle& rRect, BOOL b, BOOL bCancelled ) :
        maWindowRect( rRect ), mbFloating( b ), mbCancelled( bCancelled )
        {};
};

struct EndPopupModeData
{
    Point       maFloatingPos;    // in
    BOOL        mbTearoff;        // in

    EndPopupModeData() {};
    EndPopupModeData( const Point& rPos, BOOL bTearoff ) :
        maFloatingPos( rPos ), mbTearoff( bTearoff )
        {};
};

/** ImplDockingWindowWrapper
 *
 *  ImplDockingWindowWrapper obsoletes the DockingWindow class.
 *  It is better because it can make a "normal window" dockable.
 *  All DockingWindows should be converted the new class.
 */

class ImplDockingWindowWrapper
{
    friend class Window;
    friend class DockingManager;
    friend class DockingWindow;

private:

    // the original 'Docking'window
    Window *mpDockingWindow;

    // the original DockingWindow members
    FloatingWindow* mpFloatWin;
    Window*         mpOldBorderWin;
    Window*         mpParent;
    Point           maFloatPos;
    Point           maDockPos;
    Point           maMouseOff;
    Point           maMouseStart;
    Size            maRollUpOutSize;
    Size            maMinOutSize;
    Size            maMaxOutSize;
    Rectangle       maDragArea;
    long            mnTrackX;
    long            mnTrackY;
    long            mnTrackWidth;
    long            mnTrackHeight;
    sal_Int32       mnDockLeft;
    sal_Int32       mnDockTop;
    sal_Int32       mnDockRight;
    sal_Int32       mnDockBottom;
    WinBits         mnFloatBits;
    BOOL            mbDockCanceled:1,
                    mbFloatPrevented:1,
                    mbDockable:1,
                    mbDocking:1,
                    mbDragFull:1,
                    mbLastFloatMode:1,
                    mbStartFloat:1,
                    mbTrackDock:1,
                    mbPined:1,
                    mbRollUp:1,
                    mbDockBtn:1,
                    mbHideBtn:1,
                    mbStartDockingEnabled:1,
                    mbLocked:1;

    void            ImplInitData();

                    DECL_LINK( PopupModeEnd, void* );
    void            ImplEnableStartDocking( BOOL bEnable = TRUE )  { mbStartDockingEnabled = bEnable; }
    BOOL            ImplStartDockingEnabled()               { return mbStartDockingEnabled; }

public:
    ImplDockingWindowWrapper( const Window *pWindow );
    virtual ~ImplDockingWindowWrapper();

    Window*         GetWindow()     { return mpDockingWindow; }
    BOOL            ImplStartDocking( const Point& rPos );

    // those methods actually call the corresponding handlers
    void            StartDocking( const Point& rPos, Rectangle& rRect );
    BOOL            Docking( const Point& rPos, Rectangle& rRect );
    void            EndDocking( const Rectangle& rRect, BOOL bFloatMode );
    BOOL            PrepareToggleFloatingMode();
    void            ToggleFloatingMode();

    void            SetDragArea( const Rectangle& rRect );
    Rectangle       GetDragArea() const;

    void            Lock();
    void            Unlock();
    BOOL            IsLocked() const;

    void            StartPopupMode( ToolBox* pParentToolBox );
    BOOL            IsInPopupMode() const;

    void            TitleButtonClick( USHORT nButton );
    void            Pin();
    void            Roll();
    void            PopupModeEnd();
    void            Resizing( Size& rSize );
    BOOL            Close();
    void            Tracking( const TrackingEvent& rTEvt );
    long            Notify( NotifyEvent& rNEvt );

    void            ShowTitleButton( USHORT nButton, BOOL bVisible = TRUE );
    BOOL            IsTitleButtonVisible( USHORT nButton ) const;

    void            SetPin( BOOL bPin );
    BOOL            IsPined() const;

    void            RollUp();
    void            RollDown();
    BOOL            IsRollUp() const;

    void            SetRollUpOutputSizePixel( const Size& rSize );
    Size            GetRollUpOutputSizePixel() const;

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const;

    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;

    BOOL            IsDocking() const { return mbDocking; }
    BOOL            IsDockable() const { return mbDockable; }
    BOOL            IsDockingCanceled() const { return mbDockCanceled; }
    BOOL            IsFloatingPrevented() const { return mbFloatPrevented; }

    void            SetFloatingMode( BOOL bFloatMode = FALSE );
    BOOL            IsFloatingMode() const;
    FloatingWindow* GetFloatingWindow() const { return mpFloatWin; }

    void            SetFloatingPos( const Point& rNewPos );
    Point           GetFloatingPos() const;

    void            SetFloatStyle( WinBits nWinStyle );
    WinBits         GetFloatStyle() const;
    void            SetTabStop();

    virtual void    SetPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     USHORT nFlags = WINDOW_POSSIZE_ALL );
    void            SetPosSizePixel( const Point& rNewPos,
                                     const Size& rNewSize )
                        { mpDockingWindow->SetPosSizePixel( rNewPos, rNewSize ); }
    Point           GetPosPixel() const;
    Size            GetSizePixel() const;
    void            SetOutputSizePixel( const Size& rNewSize );
    Size            GetOutputSizePixel() const;
};

class VCL_DLLPUBLIC DockingManager
{
protected:
    ::std::vector<ImplDockingWindowWrapper *> mDockingWindows;

public:
    DockingManager();
    ~DockingManager();

    void AddWindow( const Window *pWin );
    void RemoveWindow( const Window *pWin );

    ImplDockingWindowWrapper*  GetDockingWindowWrapper( const Window *pWin );
    BOOL IsDockable( const Window *pWin );

    BOOL IsFloating( const Window *pWin );
    void SetFloatingMode( const Window *pWin, BOOL bFloating );

    void Lock( const Window *pWin );
    void Unlock( const Window *pWin );
    BOOL IsLocked( const Window *pWin );

    void    StartPopupMode( ToolBox *pParentToolBox, const Window *pWin );
    BOOL    IsInPopupMode( const Window *pWin );

    // required because those methods are not virtual in Window (!!!) and must
    // be availbale from the toolkit
    void        SetPosSizePixel( Window *pWin, long nX, long nY,
                                long nWidth, long nHeight,
                                USHORT nFlags = WINDOW_POSSIZE_ALL );
    Rectangle   GetPosSizePixel( const Window *pWin );
};

// -----------------
// - DockingWindow -
// -----------------

class VCL_DLLPUBLIC DockingWindow : public Window
{
    class   ImplData;
private:
    FloatingWindow* mpFloatWin;
    Window*         mpOldBorderWin;
    ImplData*       mpImplData;
    Point           maFloatPos;
    Point           maDockPos;
    Point           maMouseOff;
    Point           maMouseStart;
    Size            maRollUpOutSize;
    Size            maMinOutSize;
    long            mnTrackX;
    long            mnTrackY;
    long            mnTrackWidth;
    long            mnTrackHeight;
    sal_Int32           mnDockLeft;
    sal_Int32           mnDockTop;
    sal_Int32           mnDockRight;
    sal_Int32           mnDockBottom;
    WinBits         mnFloatBits;
    BOOL            mbDockCanceled:1,
                    mbDockPrevented:1,
                    mbFloatPrevented:1,
                    mbDockable:1,
                    mbDocking:1,
                    mbDragFull:1,
                    mbLastFloatMode:1,
                    mbStartFloat:1,
                    mbTrackDock:1,
                    mbPined:1,
                    mbRollUp:1,
                    mbDockBtn:1,
                    mbHideBtn:1;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void    ImplInitDockingWindowData();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         DockingWindow (const DockingWindow &);
    SAL_DLLPRIVATE         DockingWindow & operator= (const DockingWindow &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
    SAL_DLLPRIVATE BOOL    ImplStartDocking( const Point& rPos );
//#endif

protected:
                    DockingWindow( WindowType nType );

public:
                    DockingWindow( Window* pParent, WinBits nStyle = WB_STDDOCKWIN );
                    DockingWindow( Window* pParent, const ResId& rResId );
                    ~DockingWindow();

    virtual void    StartDocking();
    virtual BOOL    Docking( const Point& rPos, Rectangle& rRect );
    virtual void    EndDocking( const Rectangle& rRect, BOOL bFloatMode );
    virtual BOOL    PrepareToggleFloatingMode();
    virtual void    ToggleFloatingMode();

    virtual void    TitleButtonClick( USHORT nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    PopupModeEnd();
    virtual void    Resizing( Size& rSize );
    virtual BOOL    Close();
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            ShowTitleButton( USHORT nButton, BOOL bVisible = TRUE );
    BOOL            IsTitleButtonVisible( USHORT nButton ) const;

    void            SetPin( BOOL bPin );
    BOOL            IsPined() const;

    void            RollUp();
    void            RollDown();
    BOOL            IsRollUp() const;

    void            SetRollUpOutputSizePixel( const Size& rSize );
    Size            GetRollUpOutputSizePixel() const;

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const;

    void            SetMaxOutputSizePixel( const Size& rSize );
    const Size&     GetMaxOutputSizePixel() const;

    BOOL            IsDocking() const { return mbDocking; }
    BOOL            IsDockable() const { return mbDockable; }
    BOOL            IsDockingCanceled() const { return mbDockCanceled; }
    BOOL            IsDockingPrevented() const { return mbDockPrevented; }
    BOOL            IsFloatingPrevented() const { return mbFloatPrevented; }

    void            SetFloatingMode( BOOL bFloatMode = FALSE );
    BOOL            IsFloatingMode() const;
    FloatingWindow* GetFloatingWindow() const { return mpFloatWin; }

    void            SetFloatingPos( const Point& rNewPos );
    Point           GetFloatingPos() const;

    void            SetFloatStyle( WinBits nWinStyle );
    WinBits         GetFloatStyle() const;
    void            SetTabStop();

    virtual void    SetPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     USHORT nFlags = WINDOW_POSSIZE_ALL );
    void            SetPosSizePixel( const Point& rNewPos,
                                     const Size& rNewSize )
                        { Window::SetPosSizePixel( rNewPos, rNewSize ); }
    Point           GetPosPixel() const;
    Size            GetSizePixel() const;
    void            SetOutputSizePixel( const Size& rNewSize );
    Size            GetOutputSizePixel() const;
};

inline void DockingWindow::SetPin( BOOL bPin )
{
    if ( mpFloatWin )
        mpFloatWin->SetPin( bPin );
    mbPined = bPin;
}

inline BOOL DockingWindow::IsPined() const
{
    if ( mpFloatWin )
        return mpFloatWin->IsPined();
    return mbPined;
}

inline void DockingWindow::RollUp()
{
    if ( mpFloatWin )
        mpFloatWin->RollUp();
    mbRollUp = TRUE;
}

inline void DockingWindow::RollDown()
{
    if ( mpFloatWin )
        mpFloatWin->RollDown();
    mbRollUp = FALSE;
}

inline BOOL DockingWindow::IsRollUp() const
{
    if ( mpFloatWin )
        return mpFloatWin->IsRollUp();
    return mbRollUp;
}

inline void DockingWindow::SetRollUpOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetRollUpOutputSizePixel( rSize );
    maRollUpOutSize = rSize;
}

inline Size DockingWindow::GetRollUpOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetRollUpOutputSizePixel();
    return maRollUpOutSize;
}

inline void DockingWindow::SetMinOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMinOutputSizePixel( rSize );
    maMinOutSize = rSize;
}

inline const Size& DockingWindow::GetMinOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetMinOutputSizePixel();
    return maMinOutSize;
}

inline void DockingWindow::SetFloatingPos( const Point& rNewPos )
{
    if ( mpFloatWin )
        mpFloatWin->SetPosPixel( rNewPos );
    else
        maFloatPos = rNewPos;
}


#endif  // _SV_DOCKWIN_HXX
