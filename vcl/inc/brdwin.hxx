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

#include <vcl/notebookbar/notebookbar.hxx>
#include <vcl/window.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/notebookbar/NotebookBarAddonsMerger.hxx>

#include <com/sun/star/frame/XFrame.hpp>

class ImplBorderWindowView;
enum class DrawButtonFlags;

enum class BorderWindowStyle {
    NONE             = 0x0000,
    Overlap          = 0x0001,
    Float            = 0x0004,
    Frame            = 0x0008,
    App              = 0x0010
};
namespace o3tl {
    template<> struct typed_flags<BorderWindowStyle> : is_typed_flags<BorderWindowStyle, 0x001d> {};
};

enum class BorderWindowHitTest {
    NONE           = 0x0000,
    Title          = 0x0001,
    Left           = 0x0002,
    Menu           = 0x0004,
    Top            = 0x0008,
    Right          = 0x0010,
    Bottom         = 0x0020,
    TopLeft        = 0x0040,
    TopRight       = 0x0080,
    BottomLeft     = 0x0100,
    BottomRight    = 0x0200,
    Close          = 0x0400,
    Dock           = 0x0800,
    Hide           = 0x1000,
    Help           = 0x2000,
};
namespace o3tl {
    template<> struct typed_flags<BorderWindowHitTest> : is_typed_flags<BorderWindowHitTest, 0x3fff> {};
};

enum class BorderWindowTitleType {
    Normal           = 0x0001,
    Small            = 0x0002,
    Tearoff          = 0x0004,
    Popup            = 0x0008,
    NONE             = 0x0010
};
namespace o3tl {
    template<> struct typed_flags<BorderWindowTitleType> : is_typed_flags<BorderWindowTitleType, 0x001f> {};
};

class ImplBorderWindow : public vcl::Window
{
    friend class vcl::Window;
    friend class ImplBorderWindowView;
    friend class ImplSmallBorderWindowView;
    friend class ImplStdBorderWindowView;

private:
    std::unique_ptr<ImplBorderWindowView> mpBorderView;
    VclPtr<vcl::Window>     mpMenuBarWindow;
    VclPtr<NotebookBar>     mpNotebookBar;
    tools::Long                    mnMinWidth;
    tools::Long                    mnMinHeight;
    tools::Long                    mnMaxWidth;
    tools::Long                    mnMaxHeight;
    tools::Long                    mnOrgMenuHeight;
    BorderWindowTitleType   mnTitleType;
    WindowBorderStyle       mnBorderStyle;
    bool                    mbFloatWindow;
    bool                    mbSmallOutBorder;
    bool                    mbFrameBorder;
    bool                    mbMenuHide;
    bool                    mbDockBtn;
    bool                    mbHideBtn;
    bool                    mbMenuBtn;
    bool                    mbDisplayActive;

    using Window::ImplInit;
    void                    ImplInit( vcl::Window* pParent,
                                      WinBits nStyle, BorderWindowStyle nTypeStyle,
                                      SystemParentData* pParentData );

                            ImplBorderWindow (const ImplBorderWindow &) = delete;
                            ImplBorderWindow& operator= (const ImplBorderWindow &) = delete;

public:
                            ImplBorderWindow( vcl::Window* pParent,
                                              SystemParentData* pParentData,
                                              WinBits nStyle,
                                              BorderWindowStyle nTypeStyle );
                            ImplBorderWindow( vcl::Window* pParent, WinBits nStyle,
                                              BorderWindowStyle nTypeStyle );
    virtual                 ~ImplBorderWindow() override;
    virtual void            dispose() override;

    virtual void            MouseMove( const MouseEvent& rMEvt ) override;
    virtual void            MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void            Tracking( const TrackingEvent& rTEvt ) override;
    virtual void            Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void            Activate() override;
    virtual void            Deactivate() override;
    virtual void            Resize() override;
    virtual void            RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void            StateChanged( StateChangedType nType ) override;
    virtual void            DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void            queue_resize(StateChangedType eReason = StateChangedType::Layout) override;

    void                    InitView();
    void                    UpdateView( bool bNewView, const Size& rNewOutSize );
    void                    InvalidateBorder();

    using Window::Draw;
    void                    Draw( OutputDevice* pDev, const Point& rPos );

    void                    SetDisplayActive( bool bActive );
    void                    SetTitleType( BorderWindowTitleType nTitleType, const Size& rSize );
    void                    SetBorderStyle( WindowBorderStyle nStyle );
    WindowBorderStyle       GetBorderStyle() const { return mnBorderStyle; }
    void                    SetCloseButton();
    void                    SetDockButton( bool bDockButton );
    void                    SetHideButton( bool bHideButton );
    void                    SetMenuButton( bool bMenuButton );

    void                    UpdateMenuHeight();
    void                    SetMenuBarWindow( vcl::Window* pWindow );
    void                    SetMenuBarMode( bool bHide );

    void SetNotebookBar(const OUString& rUIXMLDescription,
                        const css::uno::Reference<css::frame::XFrame>& rFrame,
                        const NotebookBarAddonsItem &aNotebookBarAddonsItem);
    void CloseNotebookBar();
    const VclPtr<NotebookBar>& GetNotebookBar() const { return mpNotebookBar; }

    void                    SetMinOutputSize( tools::Long nWidth, tools::Long nHeight )
                                { mnMinWidth = nWidth; mnMinHeight = nHeight; }
    void                    SetMaxOutputSize( tools::Long nWidth, tools::Long nHeight )
                                { mnMaxWidth = nWidth; mnMaxHeight = nHeight; }

    void                    GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    tools::Long                    CalcTitleWidth() const;

    tools::Rectangle               GetMenuRect() const;

    virtual Size            GetOptimalSize() const override;
};

struct ImplBorderFrameData
{
    VclPtr<ImplBorderWindow> mpBorderWindow;
    VclPtr<OutputDevice>     mpOutDev;
    tools::Rectangle                maTitleRect;
    tools::Rectangle                maCloseRect;
    tools::Rectangle                maDockRect;
    tools::Rectangle                maMenuRect;
    tools::Rectangle                maHideRect;
    tools::Rectangle                maHelpRect;
    Point                    maMouseOff;
    tools::Long                     mnWidth;
    tools::Long                     mnHeight;
    tools::Long                     mnTrackX;
    tools::Long                     mnTrackY;
    tools::Long                     mnTrackWidth;
    tools::Long                     mnTrackHeight;
    sal_Int32                mnLeftBorder;
    sal_Int32                mnTopBorder;
    sal_Int32                mnRightBorder;
    sal_Int32                mnBottomBorder;
    tools::Long                     mnNoTitleTop;
    tools::Long                     mnBorderSize;
    tools::Long                     mnTitleHeight;
    BorderWindowHitTest      mnHitTest;
    DrawButtonFlags          mnCloseState;
    DrawButtonFlags          mnDockState;
    DrawButtonFlags          mnMenuState;
    DrawButtonFlags          mnHideState;
    DrawButtonFlags          mnHelpState;
    BorderWindowTitleType    mnTitleType;
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
    virtual OUString        RequestHelp( const Point& rPos, tools::Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, tools::Long nWidth, tools::Long nHeight ) = 0;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const = 0;
    virtual tools::Long            CalcTitleWidth() const = 0;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, const Point* pOffset = nullptr) = 0;
    virtual tools::Rectangle       GetMenuRect() const;

    static void             ImplInitTitle( ImplBorderFrameData* pData );
    static BorderWindowHitTest ImplHitTest( ImplBorderFrameData const * pData, const Point& rPos );
    static void             ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt );
    static OUString         ImplRequestHelp( ImplBorderFrameData const * pData, const Point& rPos, tools::Rectangle& rHelpRect );
    static tools::Long             ImplCalcTitleWidth( const ImplBorderFrameData* pData );
};

class ImplNoBorderWindowView final : public ImplBorderWindowView
{
public:
                            ImplNoBorderWindowView();

    virtual void            Init( OutputDevice* pDev, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const override;
    virtual tools::Long            CalcTitleWidth() const override;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, const Point* pOffset = nullptr) override;
};

class ImplSmallBorderWindowView : public ImplBorderWindowView
{
    VclPtr<ImplBorderWindow> mpBorderWindow;
    VclPtr<OutputDevice>     mpOutDev;
    tools::Long                    mnWidth;
    tools::Long                    mnHeight;
    sal_Int32               mnLeftBorder;
    sal_Int32               mnTopBorder;
    sal_Int32               mnRightBorder;
    sal_Int32               mnBottomBorder;
    bool                    mbNWFBorder;

public:
                            ImplSmallBorderWindowView( ImplBorderWindow* pBorderWindow );

    virtual void            Init( OutputDevice* pOutDev, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const override;
    virtual tools::Long            CalcTitleWidth() const override;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, const Point* pOffset = nullptr) override;
};

class ImplStdBorderWindowView : public ImplBorderWindowView
{
    ImplBorderFrameData     maFrameData;

public:
                            ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow );
                            virtual ~ImplStdBorderWindowView() override;

    virtual bool        MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool        Tracking( const TrackingEvent& rTEvt ) override;
    virtual OUString        RequestHelp( const Point& rPos, tools::Rectangle& rHelpRect ) override;
    virtual tools::Rectangle       GetMenuRect() const override;

    virtual void            Init( OutputDevice* pDev, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void            GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                       sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const override;
    virtual tools::Long            CalcTitleWidth() const override;
    virtual void            DrawWindow(vcl::RenderContext& rRenderContext, const Point* pOffset = nullptr) override;
};

#endif // INCLUDED_VCL_INC_BRDWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
