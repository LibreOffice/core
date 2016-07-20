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

#ifndef INCLUDED_VCL_DOCKWIN_HXX
#define INCLUDED_VCL_DOCKWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/builder.hxx>
#include <vcl/floatwin.hxx>
#include <vector>

// data to be sent with docking events
struct DockingData
{
    Point       maMousePos;     // in
    Rectangle   maTrackRect;    // in/out
    bool        mbFloating;     // out

    DockingData( const Point& rPt, const Rectangle& rRect, bool b) :
        maMousePos( rPt ), maTrackRect( rRect ), mbFloating( b )
        {};
};

struct EndDockingData
{
    Rectangle   maWindowRect;    // in
    bool        mbFloating;      // in
    bool        mbCancelled;     // in

    EndDockingData( const Rectangle& rRect, bool b, bool bCancelled ) :
        maWindowRect( rRect ), mbFloating( b ), mbCancelled( bCancelled )
        {};
};

struct EndPopupModeData
{
    Point       maFloatingPos;    // in
    bool        mbTearoff;        // in

    EndPopupModeData( const Point& rPos, bool bTearoff ) :
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
    friend class ::vcl::Window;
    friend class DockingManager;
    friend class DockingWindow;

private:

    // the original 'Docking'window
    VclPtr<vcl::Window>    mpDockingWindow;

    // the original DockingWindow members
    VclPtr<FloatingWindow> mpFloatWin;
    VclPtr<vcl::Window>    mpOldBorderWin;
    VclPtr<vcl::Window>    mpParent;
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
    bool            mbDockCanceled:1,
                    mbDocking:1,
                    mbLastFloatMode:1,
                    mbStartFloat:1,
                    mbPinned:1,
                    mbRollUp:1,
                    mbDockBtn:1,
                    mbHideBtn:1,
                    mbStartDockingEnabled:1,
                    mbLocked:1;

                    DECL_LINK_TYPED( PopupModeEnd, FloatingWindow*, void );
    void            ImplEnableStartDocking()  { mbStartDockingEnabled = true; }
    bool            ImplStartDockingEnabled() { return mbStartDockingEnabled; }

public:
    ImplDockingWindowWrapper( const vcl::Window *pWindow );
    virtual ~ImplDockingWindowWrapper();

    vcl::Window*         GetWindow()     { return mpDockingWindow; }
    bool            ImplStartDocking( const Point& rPos );

    // those methods actually call the corresponding handlers
    void            StartDocking( const Point& rPos, Rectangle& rRect );
    bool            Docking( const Point& rPos, Rectangle& rRect );
    void            EndDocking( const Rectangle& rRect, bool bFloatMode );
    bool            PrepareToggleFloatingMode();
    void            ToggleFloatingMode();

    void            SetDragArea( const Rectangle& rRect );
    const Rectangle& GetDragArea() const { return maDragArea;}

    void            Lock();
    void            Unlock();
    bool            IsLocked() const { return mbLocked;}

    void            StartPopupMode( ToolBox* pParentToolBox, FloatWinPopupFlags nPopupModeFlags );
    bool            IsInPopupMode() const;

    void            TitleButtonClick( TitleButton nButton );
    void            Resizing( Size& rSize );
    void            Tracking( const TrackingEvent& rTEvt );

    void            ShowTitleButton( TitleButton nButton, bool bVisible );

    void            SetMinOutputSizePixel( const Size& rSize );

    void            SetMaxOutputSizePixel( const Size& rSize );

    bool            IsDocking() const { return mbDocking; }
    bool            IsDockingCanceled() const { return mbDockCanceled; }

    void            SetFloatingMode( bool bFloatMode );
    bool            IsFloatingMode() const;
    FloatingWindow* GetFloatingWindow() const { return mpFloatWin; }

    void            SetFloatStyle( WinBits nWinStyle );
    WinBits         GetFloatStyle() const { return mnFloatBits;}

    void            setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     PosSizeFlags nFlags );
    Point           GetPosPixel() const;
    Size            GetSizePixel() const;
};

class VCL_DLLPUBLIC DockingManager
{
protected:
    ::std::vector<ImplDockingWindowWrapper *> mDockingWindows;

public:
    DockingManager();
    ~DockingManager();

    void AddWindow( const vcl::Window *pWin );
    void RemoveWindow( const vcl::Window *pWin );

    ImplDockingWindowWrapper*  GetDockingWindowWrapper( const vcl::Window *pWin );
    bool IsDockable( const vcl::Window *pWin );

    bool IsFloating( const vcl::Window *pWin );
    void SetFloatingMode( const vcl::Window *pWin, bool bFloating );

    void Lock( const vcl::Window *pWin );
    void Unlock( const vcl::Window *pWin );
    bool IsLocked( const vcl::Window *pWin );

    void    StartPopupMode( ToolBox *pParentToolBox, const vcl::Window *pWin );
    void    StartPopupMode( ToolBox *pParentToolBox, const vcl::Window *pWin, FloatWinPopupFlags nPopupModeFlags );

    bool    IsInPopupMode( const vcl::Window *pWin );
    void    EndPopupMode( const vcl::Window *pWin );

    // required because those methods are not virtual in Window (!!!) and must
    // be availbale from the toolkit
    void        SetPosSizePixel( vcl::Window *pWin, long nX, long nY,
                                long nWidth, long nHeight,
                                PosSizeFlags nFlags );
    Rectangle   GetPosSizePixel( const vcl::Window *pWin );
};


class VCL_DLLPUBLIC DockingWindow
    : public vcl::Window
    , public VclBuilderContainer
{
    class   ImplData;
private:
    VclPtr<FloatingWindow> mpFloatWin;
    VclPtr<vcl::Window>    mpOldBorderWin;
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
    sal_Int32       mnDockLeft;
    sal_Int32       mnDockTop;
    sal_Int32       mnDockRight;
    sal_Int32       mnDockBottom;
    WinBits         mnFloatBits;
    Idle            maLayoutIdle;
    bool            mbDockCanceled:1,
                    mbDockable:1,
                    mbDocking:1,
                    mbDragFull:1,
                    mbLastFloatMode:1,
                    mbStartFloat:1,
                    mbPinned:1,
                    mbRollUp:1,
                    mbDockBtn:1,
                    mbHideBtn:1,
                    mbIsCalculatingInitialLayoutSize:1;

protected:
    bool mbIsDefferedInit;
    VclPtr<vcl::Window>  mpDialogParent;
private:

    SAL_DLLPRIVATE void    ImplInitDockingWindowData();
    SAL_DLLPRIVATE void setPosSizeOnContainee(Size aSize, Window &rBox);
    DECL_DLLPRIVATE_LINK_TYPED( ImplHandleLayoutTimerHdl, Idle*, void );

                           DockingWindow (const DockingWindow &) = delete;
                           DockingWindow & operator= (const DockingWindow &) = delete;

protected:
    SAL_DLLPRIVATE void    SetIdleDebugName( const sal_Char *pDebugName );

    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

    SAL_DLLPRIVATE void DoInitialLayout();

    void loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                const css::uno::Reference<css::frame::XFrame> &rFrame);

public:
    bool            isLayoutEnabled() const;
    void            setOptimalLayoutSize();
    bool            isCalculatingInitialLayoutSize() const { return mbIsCalculatingInitialLayoutSize; }

    SAL_DLLPRIVATE bool    ImplStartDocking( const Point& rPos );
    SAL_DLLPRIVATE bool    isDeferredInit() const { return mbIsDefferedInit; }
    SAL_DLLPRIVATE bool    hasPendingLayout() const { return maLayoutIdle.IsActive(); }
    virtual        void    doDeferredInit(WinBits nBits);
protected:
                    DockingWindow( WindowType nType );

public:
    DockingWindow(vcl::Window* pParent, WinBits nStyle);
    DockingWindow(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
        const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
    virtual ~DockingWindow() override;
    virtual void dispose() override;

    virtual void    StartDocking();
    virtual bool    Docking( const Point& rPos, Rectangle& rRect );
    virtual void    EndDocking( const Rectangle& rRect, bool bFloatMode );
    virtual bool    PrepareToggleFloatingMode();
    virtual void    ToggleFloatingMode();

    virtual void    Resizing( Size& rSize );
    virtual bool    Close();
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            RollDown();
    bool            IsRollUp() const;

    void            SetMinOutputSizePixel( const Size& rSize );
    const Size&     GetMinOutputSizePixel() const;

    void            SetMaxOutputSizePixel( const Size& rSize );

    bool            IsDocking() const { return mbDocking; }
    bool            IsDockable() const { return mbDockable; }
    bool            IsDockingCanceled() const { return mbDockCanceled; }

    void            SetFloatingMode( bool bFloatMode );
    bool            IsFloatingMode() const;
    FloatingWindow* GetFloatingWindow() const { return mpFloatWin; }

    void            SetFloatingPos( const Point& rNewPos );
    Point           GetFloatingPos() const;

    void            SetFloatStyle( WinBits nWinStyle );
    WinBits         GetFloatStyle() const;

    virtual void    setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     PosSizeFlags nFlags = PosSizeFlags::All ) override;
    using Window::SetPosSizePixel;
    Point           GetPosPixel() const override;
    Size            GetSizePixel() const override;
    void            SetOutputSizePixel( const Size& rNewSize ) override;
    Size            GetOutputSizePixel() const;

    virtual void SetText( const OUString& rStr ) override;
    virtual OUString GetText() const override;
    virtual Size GetOptimalSize() const override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
};


inline void DockingWindow::RollDown()
{
    if ( mpFloatWin )
        mpFloatWin->RollDown();
    mbRollUp = false;
}

inline bool DockingWindow::IsRollUp() const
{
    if ( mpFloatWin )
        return mpFloatWin->IsRollUp();
    return mbRollUp;
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

inline void DockingWindow::SetIdleDebugName( const sal_Char *pDebugName )
{
    maLayoutIdle.SetDebugName( pDebugName );
}


#endif // INCLUDED_VCL_DOCKWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
