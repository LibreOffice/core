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

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKFRAME_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKFRAME_HXX

#include <cairo.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(4,0,0)
#include <gtk/gtkx.h>
#endif
#include <gdk/gdkkeysyms.h>

#include <salframe.hxx>
#include <vcl/idle.hxx>
#include <vcl/sysdata.hxx>
#include <unx/saltype.h>
#include <unx/screensaverinhibitor.hxx>

#include <tools/link.hxx>

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

#include <list>
#include <vector>

#include <config_dbus.h>
#include <config_gio.h>

#include <headless/svpgdi.hxx>

#include "gtkdata.hxx"

class GtkSalGraphics;
class GtkSalDisplay;

typedef sal_uIntPtr GdkNativeWindow;
class GtkInstDropTarget;
class GtkInstDragSource;
class GtkDnDTransferable;

class GtkSalMenu;

struct VclToGtkHelper;

class GtkSalFrame final : public SalFrame
{
    struct IMHandler
    {

#if !GTK_CHECK_VERSION(4, 0, 0)
        // Not all GTK Input Methods swallow key release
        // events.  Since they swallow the key press events and we
        // are left with the key release events, we need to
        // manually swallow those.  To do this, we keep a list of
        // the previous 10 key press events in each GtkSalFrame
        // and when we get a key release that matches one of the
        // key press events in our list, we swallow it.
        struct PreviousKeyPress
        {
            GdkWindow *window;
            gint8   send_event;
            guint32 time;
            guint   state;
            guint   keyval;
            guint16 hardware_keycode;
            guint8  group;

            PreviousKeyPress (GdkEventKey *event)
            :   window (nullptr),
                send_event (0),
                time (0),
                state (0),
                keyval (0),
                hardware_keycode (0),
                group (0)
            {
                if (event)
                {
                    window              = event->window;
                    send_event          = event->send_event;
                    time                = event->time;
                    state               = event->state;
                    keyval              = event->keyval;
                    hardware_keycode    = event->hardware_keycode;
                    group               = event->group;
                }
            }

            PreviousKeyPress( const PreviousKeyPress& rPrev )
            :   window( rPrev.window ),
                send_event( rPrev.send_event ),
                time( rPrev.time ),
                state( rPrev.state ),
                keyval( rPrev.keyval ),
                hardware_keycode( rPrev.hardware_keycode ),
                group( rPrev.group )
            {}

            bool operator== (GdkEventKey const *event) const
            {
                return (event != nullptr)
                    && (event->window == window)
                    && (event->send_event == send_event)
                    // ignore non-Gdk state bits, e.g., these used by IBus
                    && ((event->state & GDK_MODIFIER_MASK) == (state & GDK_MODIFIER_MASK))
                    && (event->keyval == keyval)
                    && (event->hardware_keycode == hardware_keycode)
                    && (event->group == group)
                    && (event->time - time < 300)
                    ;
            }
        };
#endif

        GtkSalFrame*                    m_pFrame;
#if !GTK_CHECK_VERSION(4, 0, 0)
        std::list< PreviousKeyPress >   m_aPrevKeyPresses;
#endif
        int                             m_nPrevKeyPresses; // avoid using size()
        GtkIMContext*                   m_pIMContext;
        bool                            m_bFocused;
        bool                            m_bPreeditJustChanged;
        SalExtTextInputEvent            m_aInputEvent;
        std::vector< ExtTextInputAttr > m_aInputFlags;

        IMHandler( GtkSalFrame* );
        ~IMHandler();

        void            createIMContext();
        void            deleteIMContext();
        void            updateIMSpotLocation();
        void            endExtTextInput( EndExtTextInputFlags nFlags );
#if !GTK_CHECK_VERSION(4, 0, 0)
        bool            handleKeyEvent( GdkEventKey* pEvent );
#endif
        void            focusChanged( bool bFocusIn );

        void            doCallEndExtTextInput();
        void            sendEmptyCommit();

        static void         signalIMCommit( GtkIMContext*, gchar*, gpointer );
        static gboolean     signalIMDeleteSurrounding( GtkIMContext*, gint, gint, gpointer );
        static void         signalIMPreeditChanged( GtkIMContext*, gpointer );
        static void         signalIMPreeditEnd( GtkIMContext*, gpointer );
        static void         signalIMPreeditStart( GtkIMContext*, gpointer );
        static gboolean     signalIMRetrieveSurrounding( GtkIMContext*, gpointer );
    };
    friend struct IMHandler;

    friend class GtkSalObjectWidgetClip;

    SalX11Screen                    m_nXScreen;
    GtkWidget*                      m_pWindow;
    GtkHeaderBar*                   m_pHeaderBar;
    GtkGrid*                        m_pTopLevelGrid;
#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkEventBox*                    m_pEventBox;
    GtkFixed*                       m_pFixedContainer;
    GtkFixed*                       m_pDrawingArea;
#else
    GtkOverlay*                     m_pOverlay;
    GtkFixed*                       m_pFixedContainer;
    GtkDrawingArea*                 m_pDrawingArea;
    GtkEventControllerKey*          m_pKeyController;
    gulong                          m_nSettingChangedSignalId;
#endif
    gulong                          m_nPortalSettingChangedSignalId;
    GDBusProxy*                     m_pSettingsPortal;
#if !GTK_CHECK_VERSION(4, 0, 0)
    GdkWindow*                      m_pForeignParent;
    GdkNativeWindow                 m_aForeignParentWindow;
    GdkWindow*                      m_pForeignTopLevel;
    GdkNativeWindow                 m_aForeignTopLevelWindow;
#endif
    SalFrameStyleFlags              m_nStyle;
    GtkSalFrame*                    m_pParent;
    std::list< GtkSalFrame* >       m_aChildren;
    GdkToplevelState                m_nState;
    SystemEnvData                   m_aSystemData;
    std::unique_ptr<GtkSalGraphics> m_pGraphics;
    bool                            m_bGraphics;
    ModKeyFlags                     m_nKeyModifiers;
    PointerStyle                    m_ePointerStyle;
    ScreenSaverInhibitor            m_ScreenSaverInhibitor;
    gulong                          m_nSetFocusSignalId;
    bool                            m_bFullscreen;
    bool                            m_bDefaultPos;
    bool                            m_bDefaultSize;
    bool                            m_bTooltipBlocked;
    OUString                        m_sWMClass;

    std::unique_ptr<IMHandler>      m_pIMHandler;

    Size                            m_aMaxSize;
    Size                            m_aMinSize;
    tools::Rectangle                m_aRestorePosSize;

    OUString                        m_aTooltip;
    tools::Rectangle                m_aHelpArea;
    tools::Rectangle                m_aFloatRect;
    FloatWinPopupFlags              m_nFloatFlags;
    bool                            m_bFloatPositioned;
    tools::Long                            m_nWidthRequest;
    tools::Long                            m_nHeightRequest;
    cairo_region_t*                 m_pRegion;
    GtkInstDropTarget*              m_pDropTarget;
    GtkInstDragSource*              m_pDragSource;
    bool                            m_bGeometryIsProvisional;
    bool                            m_bIconSetWhileUnmapped;

    GtkSalMenu*                     m_pSalMenu;

#if ENABLE_DBUS && ENABLE_GIO
    private:
    friend void on_registrar_available (GDBusConnection*, const gchar*, const gchar*, gpointer);
    friend void on_registrar_unavailable (GDBusConnection*, const gchar*, gpointer);
#endif
    guint                           m_nWatcherId;

    void Init( SalFrame* pParent, SalFrameStyleFlags nStyle );
    void Init( SystemParentData* pSysData );
    void InitCommon();
    void InvalidateGraphics();

    // signals
#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean     signalButton( GtkWidget*, GdkEventButton*, gpointer );
    static void         signalStyleUpdated(GtkWidget*, gpointer);
#else
    static void         signalStyleUpdated(GtkWidget*, const gchar* pSetting, pointer);
#endif
    void DrawingAreaResized(GtkWidget* pWidget, int nWidth, int nHeight);
    void DrawingAreaDraw(cairo_t *cr);
#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean     signalDraw( GtkWidget*, cairo_t *cr, gpointer );
    static void         sizeAllocated(GtkWidget*, GdkRectangle *pAllocation, gpointer frame);
#else
    static void         signalDraw(GtkDrawingArea*, cairo_t *cr, int width, int height, gpointer);
    static void         sizeAllocated(GtkWidget*, int nWidth, int nHeight, gpointer frame);
#endif
    static void         signalRealize(GtkWidget*, gpointer frame);
    static gboolean     signalTooltipQuery(GtkWidget*, gint x, gint y,
                                     gboolean keyboard_mode, GtkTooltip *tooltip,
                                     gpointer frame);
#if GTK_CHECK_VERSION(4, 0, 0)
    static GdkDragAction signalDragMotion(GtkDropTargetAsync *dest, GdkDrop *drop, double x, double y, gpointer frame);
    static void         signalDragLeave(GtkDropTargetAsync *dest, GdkDrop *drop, gpointer frame);
    static gboolean     signalDragDrop(GtkDropTargetAsync* context, GdkDrop* drop, double x, double y, gpointer frame);

    static void         signalDragFailed(GdkDrag* drag, GdkDragCancelReason reason, gpointer frame);
    static void         signalDragDelete(GdkDrag* drag, gpointer frame);
    static void         signalDragEnd(GdkDrag* drag, gpointer frame);
#else
    static gboolean     signalDragMotion(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
                                         guint time, gpointer frame);
    static gboolean     signalDragDrop(GtkWidget* widget, GdkDragContext *context, gint x, gint y,
                                       guint time, gpointer frame);
    static void         signalDragDropReceived(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
                                               GtkSelectionData *data, guint ttype, guint time, gpointer frame);
    static void         signalDragLeave(GtkWidget *widget, GdkDragContext *context, guint time, gpointer frame);

    static gboolean     signalDragFailed(GtkWidget *widget, GdkDragContext *context, GtkDragResult result, gpointer frame);
    static void         signalDragDelete(GtkWidget *widget, GdkDragContext *context, gpointer frame);
    static void         signalDragEnd(GtkWidget *widget, GdkDragContext *context, gpointer frame);
    static void         signalDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData *data, guint info,
                                          guint time, gpointer frame);

#endif
    static void         gestureSwipe(GtkGestureSwipe* gesture, gdouble velocity_x, gdouble velocity_y, gpointer frame);
    static void         gestureLongPress(GtkGestureLongPress* gesture, gdouble x, gdouble y, gpointer frame);
    bool                DrawingAreaButton(SalEvent nEventType, int nEventX, int nEventY, int nButton, guint32 nTime, guint nState);
#if GTK_CHECK_VERSION(4, 0, 0)
    static void         gesturePressed(GtkGestureClick* gesture, int n_press, gdouble x, gdouble y, gpointer frame);
    static void         gestureReleased(GtkGestureClick* gesture, int n_press, gdouble x, gdouble y, gpointer frame);
    void                gestureButton(GtkGestureClick* gesture, SalEvent nEventType, gdouble x, gdouble y);
#endif
    void                DrawingAreaFocusInOut(SalEvent nEventType);
#if GTK_CHECK_VERSION(4, 0, 0)
    static void         signalFocusEnter(GtkEventControllerFocus* pController, gpointer frame);
    static void         signalFocusLeave(GtkEventControllerFocus* pController, gpointer frame);
#else
    static gboolean     signalFocus( GtkWidget*, GdkEventFocus*, gpointer );
#endif
#if !GTK_CHECK_VERSION(4, 0, 0)
    static void         signalSetFocus(GtkWindow* pWindow, GtkWidget* pWidget, gpointer frame);
#else
    static void         signalSetFocus(GtkWindow* pWindow, GParamSpec* pSpec, gpointer frame);
#endif
    void WindowMap();
    void WindowUnmap();
    bool WindowCloseRequest();
    void DrawingAreaMotion(int nEventX, int nEventY, guint32 nTime, guint nState);
    void DrawingAreaCrossing(SalEvent nEventType, int nEventX, int nEventY, guint32 nTime, guint nState);
    void DrawingAreaScroll(double delta_x, double delta_y, int nEventX, int nEventY, guint32 nTime, guint nState);
#if GTK_CHECK_VERSION(4, 0, 0)
    bool DrawingAreaKey(GtkEventControllerKey* pController, SalEvent nEventType, guint keyval, guint keycode, guint nState);

    static void         signalMap(GtkWidget*, gpointer);
    static void         signalUnmap(GtkWidget*, gpointer);

    static gboolean     signalDelete(GtkWidget*, gpointer);

    static void         signalMotion(GtkEventControllerMotion *controller, double x, double y, gpointer);

    static gboolean     signalScroll(GtkEventControllerScroll* pController, double delta_x, double delta_y, gpointer);

    static void         signalEnter(GtkEventControllerMotion *controller, double x, double y, gpointer);
    static void         signalLeave(GtkEventControllerMotion *controller, gpointer);

    static gboolean     signalKeyPressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer);
    static gboolean     signalKeyReleased(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer);

    static void         signalWindowState(GdkToplevel*, GParamSpec*, gpointer);
#else
    static gboolean     signalMap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalUnmap( GtkWidget*, GdkEvent*, gpointer );

    static gboolean     signalDelete( GtkWidget*, GdkEvent*, gpointer );

    static gboolean     signalMotion( GtkWidget*, GdkEventMotion*, gpointer );

    static gboolean     signalScroll( GtkWidget*, GdkEvent*, gpointer );

    static gboolean     signalCrossing( GtkWidget*, GdkEventCrossing*, gpointer );

    static gboolean     signalKey( GtkWidget*, GdkEventKey*, gpointer );

    static gboolean     signalWindowState( GtkWidget*, GdkEvent*, gpointer );
#endif
#if !GTK_CHECK_VERSION(4, 0, 0)
    static gboolean     signalConfigure( GtkWidget*, GdkEventConfigure*, gpointer );
#endif
    static void         signalDestroy( GtkWidget*, gpointer );

    void            Center();
    void            SetDefaultSize();

    bool            doKeyCallback( guint state,
                                   guint keyval,
                                   guint16 hardware_keycode,
                                   guint8 group,
                                   sal_Unicode aOrigCode,
                                   bool bDown,
                                   bool bSendRelease
                                   );

#if !GTK_CHECK_VERSION(4, 0, 0)
    static GdkNativeWindow findTopLevelSystemWindow( GdkNativeWindow aWindow );
#endif

    static int m_nFloats;

    bool isFloatGrabWindow() const
    {
        return
            (m_nStyle & SalFrameStyleFlags::FLOAT) &&                // only a float can be floatgrab
            !(m_nStyle & SalFrameStyleFlags::TOOLTIP) &&             // tool tips are not
            !(m_nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION);   // toolbars are also not
    }

    bool isChild( bool bPlug = true, bool bSysChild = true )
    {
        SalFrameStyleFlags nMask = SalFrameStyleFlags::NONE;
        if( bPlug )
            nMask |= SalFrameStyleFlags::PLUG;
        if( bSysChild )
            nMask |= SalFrameStyleFlags::SYSTEMCHILD;
        return bool(m_nStyle & nMask);
    }

    //call gtk_window_resize
    void window_resize(tools::Long nWidth, tools::Long nHeight);
    //call gtk_widget_set_size_request
    void widget_set_size_request(tools::Long nWidth, tools::Long nHeight);

    void resizeWindow( tools::Long nWidth, tools::Long nHeight );
    void moveWindow( tools::Long nX, tools::Long nY );

    Size calcDefaultSize();

    void setMinMaxSize();

    void AllocateFrame();
    void TriggerPaintEvent();

    void updateWMClass();

    enum class SetType { RetainSize, Fullscreen, UnFullscreen };

    void SetScreen( unsigned int nNewScreen, SetType eType, tools::Rectangle const *pSize = nullptr );

    void SetIcon(const char* pIcon);

    bool HandleMenubarMnemonic(guint eState, guint nKeyval);

    void ListenPortalSettings();

public:
    cairo_surface_t*                m_pSurface;
    basegfx::B2IVector              m_aFrameSize;
    DamageHandler                   m_aDamageHandler;
    std::vector<GdkEvent*>          m_aPendingScrollEvents;
#if !GTK_CHECK_VERSION(4, 0, 0)
    Idle                            m_aSmoothScrollIdle;
#endif
    int                             m_nGrabLevel;
    bool                            m_bSalObjectSetPosSize;
    GtkSalFrame(SalFrame* pParent, SalFrameStyleFlags nStyle, vcl::Window&);
    GtkSalFrame(SystemParentData* pSysData, vcl::Window&);

    guint                           m_nMenuExportId;
    guint                           m_nActionGroupExportId;
    guint                           m_nHudAwarenessId;
    std::vector<gulong>             m_aMouseSignalIds;

    void grabPointer(bool bGrab, bool bKeyboardAlso, bool bOwnerEvents);

    static GtkSalDisplay*  getDisplay();
    static GdkDisplay*     getGdkDisplay();
    GtkWidget*  getWindow() const { return m_pWindow; }
    GtkFixed*   getFixedContainer() const { return GTK_FIXED(m_pFixedContainer); }
    GtkWidget*  getMouseEventWidget() const;
    GtkGrid*    getTopLevelGridWidget() const { return m_pTopLevelGrid; }
    const SalX11Screen& getXScreenNumber() const { return m_nXScreen; }
    int GetDisplayScreen() const { return maGeometry.screen(); }
    void updateScreenNumber();

    cairo_t* getCairoContext() const;
    void damaged(sal_Int32 nExtentsLeft, sal_Int32 nExtentsTop,
                 sal_Int32 nExtentsRight, sal_Int32 nExtentsBottom) const;

    void registerDropTarget(GtkInstDropTarget* pDropTarget)
    {
        assert(!m_pDropTarget);
        m_pDropTarget = pDropTarget;
    }

    void deregisterDropTarget(GtkInstDropTarget const * pDropTarget)
    {
        assert(m_pDropTarget == pDropTarget); (void)pDropTarget;
        m_pDropTarget = nullptr;
    }

    void registerDragSource(GtkInstDragSource* pDragSource)
    {
        assert(!m_pDragSource);
        m_pDragSource = pDragSource;
    }

    void deregisterDragSource(GtkInstDragSource const * pDragSource)
    {
        assert(m_pDragSource == pDragSource); (void)pDragSource;
        m_pDragSource = nullptr;
    }

    void startDrag(const css::datatransfer::dnd::DragGestureEvent& rEvent,
                   const css::uno::Reference<css::datatransfer::XTransferable>& rTrans,
                   VclToGtkHelper& rConversionHelper,
                   GdkDragAction sourceActions);

    void closePopup();

    void addGrabLevel();
    void removeGrabLevel();

#if !GTK_CHECK_VERSION(4, 0, 0)
    void nopaint_container_resize_children(GtkContainer*);

    void LaunchAsyncScroll(GdkEvent const * pEvent);
    DECL_LINK(AsyncScroll, Timer *, void);
#endif

    virtual ~GtkSalFrame() override;

    virtual sal_Int32 GetSgpMetric(vcl::SGPmetric eMetric) const override;

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*        AcquireGraphics() override;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) override;

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual bool                PostEvent(std::unique_ptr<ImplSVEvent> pData) override;

    virtual void                SetTitle( const OUString& rTitle ) override;
    virtual void                SetIcon( sal_uInt16 nIcon ) override;
    virtual void                SetMenu( SalMenu *pSalMenu ) override;
    SalMenu*                    GetMenu();
    void                        EnsureAppMenuWatch();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) override;
    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( bool bVisible, bool bNoActivate = false ) override;
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void                SetMinClientSize( tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                SetMaxClientSize( tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                SetPosSize( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt16 nFlags ) override;
    void GetDPI(sal_Int32& rDPIX, sal_Int32& rDPIY) override;
    virtual void GetClientSize(sal_Int32& rWidth, sal_Int32& rHeight) override;
    virtual void                GetWorkArea( tools::Rectangle& rRect ) override;
    virtual SalFrame*           GetParent() const override;
    virtual void SetWindowState(const vcl::WindowData*) override;
    virtual bool GetWindowState(vcl::WindowData*) override;
    virtual void                ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) override;
    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void                StartPresentation( bool bStart ) override;
    // Show Window over all other Windows
    virtual void                SetAlwaysOnTop( bool bOnTop ) override;

    // Window to top and grab focus
    virtual void                ToTop( SalFrameToTop nFlags ) override;

    // this function can call with the same
    // pointer style
    virtual void                SetPointer( PointerStyle ePointerStyle ) override;
    virtual void                CaptureMouse( bool bMouse ) override;
    virtual void                GrabFocus() override;
    virtual void                SetPointerPos( tools::Long nX, tools::Long nY ) override;

    // flush output buffer
    using SalFrame::Flush;
    virtual void                Flush() override;
    // flush output buffer, wait till outstanding operations are done

    virtual void                SetInputContext( SalInputContext* pContext ) override;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) override;

    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) override;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override;

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType        GetInputLanguage() override;

    virtual void                UpdateSettings( AllSettings& rSettings ) override;

    virtual void                Beep() override;

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*    GetSystemData() const override;

    virtual void                ResolveWindowHandle(SystemEnvData& rData) const override;

    // get current modifier and button mask
    virtual SalPointerState     GetPointerState() override;

    virtual KeyIndicatorState   GetIndicatorState() override;

    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) override;

    // set new parent window
    virtual void                SetParent( SalFrame* pNewParent ) override;
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window internally
    // return false to indicate failure
    virtual void                SetPluginParent( SystemParentData* pNewParent ) override;

    virtual void                SetScreenNumber( unsigned int ) override;
    virtual void                SetApplicationID( const OUString &rWMClass ) override;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                ResetClipRegion() override;
    // start setting the clipregion consisting of nRects rectangles
    virtual void                BeginSetClipRegion( sal_uInt32 nRects ) override;
    // add a rectangle to the clip region
    virtual void                UnionClipRegion( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    // done setting up the clipregion
    virtual void                EndSetClipRegion() override;

    virtual void                PositionByToolkit(const tools::Rectangle& rRect, FloatWinPopupFlags nFlags) override;
    virtual void                SetModal(bool bModal) override;
    virtual bool                GetModal() const override;
    void                        HideTooltip();
    void                        BlockTooltip();
    void                        UnblockTooltip();
    virtual bool                ShowTooltip(const OUString& rHelpText, const tools::Rectangle& rHelpArea) override;
    virtual void*               ShowPopover(const OUString& rHelpText, vcl::Window* pParent, const tools::Rectangle& rHelpArea, QuickHelpFlags nFlags) override;
    virtual bool                UpdatePopover(void* nId, const OUString& rHelpText, vcl::Window* pParent, const tools::Rectangle& rHelpArea) override;
    virtual bool                HidePopover(void* nId) override;
    virtual weld::Window*       GetFrameWeld() const override;

    static GtkSalFrame         *getFromWindow( GtkWidget *pWindow );

    static sal_uIntPtr          GetNativeWindowHandle(GtkWidget *pWidget);

    //Call the usual SalFrame Callback, but catch uno exceptions and delegate
    //to GtkSalData to rethrow them after the gsignal is processed when its safe
    //to do so again in our own code after the g_main_context_iteration call
    //which triggers the gsignals.
    bool                        CallCallbackExc(SalEvent nEvent, const void* pEvent) const;

    // call gtk_widget_queue_draw on the drawing widget
    void                        queue_draw();

    static void                 KeyCodeToGdkKey(const vcl::KeyCode& rKeyCode,
        guint* pGdkKeyCode, GdkModifierType *pGdkModifiers);

    static guint32              GetLastInputEventTime();
    static void                 UpdateLastInputEventTime(guint32 nUserInputTime);
    static sal_uInt16           GetMouseModCode(guint nState);
    static sal_uInt16           GetKeyCode(guint nKeyVal);
#if !GTK_CHECK_VERSION(4, 0, 0)
    static guint                GetKeyValFor(GdkKeymap* pKeyMap, guint16 hardware_keycode, guint8 group);
#endif
    static sal_uInt16           GetKeyModCode(guint nState);
    static GdkEvent*            makeFakeKeyPress(GtkWidget* pWidget);
#if !GTK_CHECK_VERSION(4, 0, 0)
    static SalWheelMouseEvent   GetWheelEvent(const GdkEventScroll& rEvent);
    static gboolean             NativeWidgetHelpPressed(GtkAccelGroup*, GObject*, guint,
        GdkModifierType, gpointer pFrame);
#endif
    static OUString             GetPreeditDetails(GtkIMContext* pIMContext, std::vector<ExtTextInputAttr>& rInputFlags, sal_Int32& rCursorPos, sal_uInt8& rCursorFlags);

    const cairo_font_options_t* get_font_options();

    void SetColorScheme(GVariant* variant);

    void DisallowCycleFocusOut();
    bool IsCycleFocusOutDisallowed() const;
    void AllowCycleFocusOut();
};

#define OOO_TYPE_FIXED ooo_fixed_get_type()

#if !GTK_CHECK_VERSION(4, 0, 0)
extern "C" {

GType ooo_fixed_get_type();
AtkObject* ooo_fixed_get_accessible(GtkWidget *obj);

} // extern "C"
#endif

#if !GTK_CHECK_VERSION(3, 22, 0)
enum GdkAnchorHints
{
  GDK_ANCHOR_FLIP_X   = 1 << 0,
  GDK_ANCHOR_FLIP_Y   = 1 << 1,
  GDK_ANCHOR_SLIDE_X  = 1 << 2,
  GDK_ANCHOR_SLIDE_Y  = 1 << 3,
  GDK_ANCHOR_RESIZE_X = 1 << 4,
  GDK_ANCHOR_RESIZE_Y = 1 << 5,
  GDK_ANCHOR_FLIP     = GDK_ANCHOR_FLIP_X | GDK_ANCHOR_FLIP_Y,
  GDK_ANCHOR_SLIDE    = GDK_ANCHOR_SLIDE_X | GDK_ANCHOR_SLIDE_Y,
  GDK_ANCHOR_RESIZE   = GDK_ANCHOR_RESIZE_X | GDK_ANCHOR_RESIZE_Y
};
#endif

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
