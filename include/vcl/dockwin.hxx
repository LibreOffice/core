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

#pragma once

#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>
#include <o3tl/deleter.hxx>
#include <memory>
#include <vector>

class ToolBox;
class FloatingWindow;
enum class FloatWinPopupFlags;

// data to be sent with docking events
struct DockingData
{
    Point       maMousePos;     // in
    tools::Rectangle   maTrackRect;    // in/out
    bool        mbFloating;     // out

    DockingData( const Point& rPt, const tools::Rectangle& rRect, bool b) :
        maMousePos( rPt ), maTrackRect( rRect ), mbFloating( b )
        {};
};

struct EndDockingData
{
    tools::Rectangle   maWindowRect;    // in
    bool        mbFloating;      // in
    bool        mbCancelled;     // in

    EndDockingData( const tools::Rectangle& rRect, bool b, bool bCancelled ) :
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

class ImplDockingWindowWrapper final
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
    Link<FloatingWindow*,void> maPopupModeEndHdl;
    Point           maFloatPos;
    Point           maDockPos;
    Point           maMouseOff;
    Size            maMinOutSize;
    Size            maMaxOutSize;
    tools::Rectangle       maDragArea;
    tools::Long            mnTrackX;
    tools::Long            mnTrackY;
    tools::Long            mnTrackWidth;
    tools::Long            mnTrackHeight;
    sal_Int32       mnDockLeft;
    sal_Int32       mnDockTop;
    sal_Int32       mnDockRight;
    sal_Int32       mnDockBottom;
    WinBits         mnFloatBits;
    bool            mbDockCanceled:1,
                    mbDocking:1,
                    mbLastFloatMode:1,
                    mbDockBtn:1,
                    mbHideBtn:1,
                    mbStartDockingEnabled:1,
                    mbLocked:1;

                    DECL_LINK( PopupModeEnd, FloatingWindow*, void );
    void            ImplEnableStartDocking()  { mbStartDockingEnabled = true; }
    bool            ImplStartDockingEnabled() const { return mbStartDockingEnabled; }
    void            ImplPreparePopupMode();

public:
    ImplDockingWindowWrapper( const vcl::Window *pWindow );
    ~ImplDockingWindowWrapper();

    vcl::Window*    GetWindow()     { return mpDockingWindow; }
    void            ImplStartDocking( const Point& rPos );

    // those methods actually call the corresponding handlers
    void            StartDocking( const Point& rPos, tools::Rectangle const & rRect );
    bool            Docking( const Point& rPos, tools::Rectangle& rRect );
    void            EndDocking( const tools::Rectangle& rRect, bool bFloatMode );
    bool            PrepareToggleFloatingMode();
    void            ToggleFloatingMode();

    void            SetDragArea( const tools::Rectangle& rRect );
    const tools::Rectangle& GetDragArea() const { return maDragArea;}

    void            Lock();
    void            Unlock();
    bool            IsLocked() const { return mbLocked;}

    void            StartPopupMode( const tools::Rectangle& rRect, FloatWinPopupFlags nPopupModeFlags );
    void            StartPopupMode( ToolBox* pParentToolBox, FloatWinPopupFlags nPopupModeFlags );
    bool            IsInPopupMode() const;

    void            SetPopupModeEndHdl( const Link<FloatingWindow*,void>& rLink ) { maPopupModeEndHdl = rLink; }

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

    void            setPosSizePixel( tools::Long nX, tools::Long nY,
                                     tools::Long nWidth, tools::Long nHeight,
                                     PosSizeFlags nFlags );
    Point           GetPosPixel() const;
    Size            GetSizePixel() const;
};

class VCL_DLLPUBLIC DockingManager
{
    std::vector<std::unique_ptr<ImplDockingWindowWrapper, o3tl::default_delete<ImplDockingWindowWrapper>>> mvDockingWindows;

public:
    DockingManager();
    ~DockingManager();

    DockingManager& operator=( DockingManager const & ) = delete; // MSVC2015 workaround
    DockingManager( DockingManager const & ) = delete; // MSVC2015 workaround

    void AddWindow( const vcl::Window *pWin );
    void RemoveWindow( const vcl::Window *pWin );

    ImplDockingWindowWrapper*  GetDockingWindowWrapper( const vcl::Window *pWin );
    bool IsDockable( const vcl::Window *pWin );

    bool IsFloating( const vcl::Window *pWin );
    void SetFloatingMode( const vcl::Window *pWin, bool bFloating );

    void Lock( const vcl::Window *pWin );
    void Unlock( const vcl::Window *pWin );
    bool IsLocked( const vcl::Window *pWin );

    void    StartPopupMode( const vcl::Window *pWin, const tools::Rectangle& rRect, FloatWinPopupFlags nPopupModeFlags );
    void    StartPopupMode( ToolBox *pParentToolBox, const vcl::Window *pWin );
    void    StartPopupMode( ToolBox *pParentToolBox, const vcl::Window *pWin, FloatWinPopupFlags nPopupModeFlags );

    void    SetPopupModeEndHdl( const vcl::Window *pWindow, const Link<FloatingWindow*,void>& rLink );

    bool    IsInPopupMode( const vcl::Window *pWin );
    void    EndPopupMode( const vcl::Window *pWin );

    // required because those methods are not virtual in Window (!!!) and must
    // be available from the toolkit
    void        SetPosSizePixel( vcl::Window const *pWin, tools::Long nX, tools::Long nY,
                                tools::Long nWidth, tools::Long nHeight,
                                PosSizeFlags nFlags );
    tools::Rectangle   GetPosSizePixel( const vcl::Window *pWin );
};


class VCL_DLLPUBLIC DockingWindow
    : public vcl::Window
    , public VclBuilderContainer
{
    class SAL_DLLPRIVATE ImplData;
private:
    VclPtr<FloatingWindow> mpFloatWin;
    VclPtr<vcl::Window>    mpOldBorderWin;
    std::unique_ptr<ImplData> mpImplData;
    Point           maFloatPos;
    Point           maDockPos;
    Point           maMouseOff;
    Size            maMinOutSize;
    tools::Long            mnTrackX;
    tools::Long            mnTrackY;
    tools::Long            mnTrackWidth;
    tools::Long            mnTrackHeight;
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
                    mbDockBtn:1,
                    mbHideBtn:1,
                    mbIsCalculatingInitialLayoutSize:1;

protected:
    bool mbIsDeferredInit;
    VclPtr<vcl::Window>  mpDialogParent;
private:

    SAL_DLLPRIVATE void    ImplInitDockingWindowData();
    SAL_DLLPRIVATE void    setPosSizeOnContainee();
    DECL_DLLPRIVATE_LINK( ImplHandleLayoutTimerHdl, Timer*, void );

                           DockingWindow (const DockingWindow &) = delete;
                           DockingWindow & operator= (const DockingWindow &) = delete;

protected:
    SAL_DLLPRIVATE void    SetIdleDebugName( const char *pDebugName );

    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();

    SAL_DLLPRIVATE void DoInitialLayout();

    void loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                const css::uno::Reference<css::frame::XFrame> &rFrame);

public:
    bool            isLayoutEnabled() const;
    void            setOptimalLayoutSize();

    //FIXME: is it okay to make this public?
    void    ImplStartDocking( const Point& rPos );
    SAL_DLLPRIVATE bool    isDeferredInit() const { return mbIsDeferredInit; }
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
    virtual bool    Docking( const Point& rPos, tools::Rectangle& rRect );
    virtual void    EndDocking( const tools::Rectangle& rRect, bool bFloatMode );
    virtual bool    PrepareToggleFloatingMode();
    virtual void    ToggleFloatingMode();

    virtual void    Resizing( Size& rSize );
    virtual bool    Close();
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

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

    virtual void    setPosSizePixel( tools::Long nX, tools::Long nY,
                                     tools::Long nWidth, tools::Long nHeight,
                                     PosSizeFlags nFlags = PosSizeFlags::All ) override;

    Point           GetPosPixel() const override;
    Size            GetSizePixel() const override;
    void            SetOutputSizePixel( const Size& rNewSize ) override;
    Size            GetOutputSizePixel() const;

    virtual void SetText( const OUString& rStr ) override;
    virtual OUString GetText() const override;
    virtual Size GetOptimalSize() const override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
};

class VCL_DLLPUBLIC DropdownDockingWindow : public DockingWindow
{
protected:
    VclPtr<vcl::Window> m_xBox;
public:
    DropdownDockingWindow(vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>(),
        bool bTearable = false);
    virtual ~DropdownDockingWindow() override;
    virtual void dispose() override;
};

class VCL_DLLPUBLIC ResizableDockingWindow : public DockingWindow
{
protected:
    VclPtr<vcl::Window> m_xBox;
public:
    ResizableDockingWindow(vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
    ResizableDockingWindow(vcl::Window* pParent, WinBits nStyle);
    virtual ~ResizableDockingWindow() override;
    virtual void dispose() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
