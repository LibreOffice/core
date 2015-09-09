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

#include <prex.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(3,0,0)
#  include <gtk/gtkx.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <postx.h>

#include <salframe.hxx>
#include <vcl/sysdata.hxx>
#include <unx/x11windowprovider.hxx>
#include <unx/saltype.h>

#include "tools/link.hxx"

#include <basebmp/bitmapdevice.hxx>
#include <basebmp/scanlineformats.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>

#include <list>
#include <vector>

class GtkSalGraphics;
class GtkSalDisplay;

#if GTK_CHECK_VERSION(3,0,0)
typedef ::Window GdkNativeWindow;
#define GDK_WINDOW_XWINDOW(o) GDK_WINDOW_XID(o)
#define gdk_set_sm_client_id(i) gdk_x11_set_sm_client_id(i)
#define gdk_window_foreign_new_for_display(a,b) gdk_x11_window_foreign_new_for_display(a,b)
#endif

#if !(GLIB_MAJOR_VERSION > 2 || GLIB_MINOR_VERSION >= 26)
    typedef void GDBusConnection;
#endif

class GtkSalFrame : public SalFrame, public X11WindowProvider
{
    static const int nMaxGraphics = 2;

    struct GraphicsHolder
    {
        GtkSalGraphics*     pGraphics;
        bool                bInUse;
        GraphicsHolder()
                : pGraphics( NULL ),
                  bInUse( false )
        {}
        ~GraphicsHolder();
    };

    struct IMHandler
    {

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
            :   window (NULL),
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

            bool operator== (GdkEventKey *event) const
            {
                return (event != NULL)
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

        GtkSalFrame*                    m_pFrame;
        std::list< PreviousKeyPress >   m_aPrevKeyPresses;
        int                             m_nPrevKeyPresses; // avoid using size()
        GtkIMContext*                   m_pIMContext;
        bool                            m_bFocused;
        bool                            m_bPreeditJustChanged;
        SalExtTextInputEvent            m_aInputEvent;
        std::vector< sal_uInt16 >           m_aInputFlags;

        IMHandler( GtkSalFrame* );
        ~IMHandler();

        void            createIMContext();
        void            deleteIMContext();
        void            updateIMSpotLocation();
        void            endExtTextInput( EndExtTextInputFlags nFlags );
        bool            handleKeyEvent( GdkEventKey* pEvent );
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

    SalX11Screen                    m_nXScreen;
    GtkWidget*                      m_pWindow;
    GtkEventBox*                    m_pEventBox;
    GtkFixed*                       m_pFixedContainer;
    GdkWindow*                      m_pForeignParent;
    GdkNativeWindow                 m_aForeignParentWindow;
    GdkWindow*                      m_pForeignTopLevel;
    GdkNativeWindow                 m_aForeignTopLevelWindow;
    Pixmap                          m_hBackgroundPixmap;
    sal_uLong                       m_nStyle;
    SalExtStyle                     m_nExtStyle;
    GtkSalFrame*                    m_pParent;
    std::list< GtkSalFrame* >       m_aChildren;
    GdkWindowState                  m_nState;
    SystemEnvData                   m_aSystemData;
    GraphicsHolder                  m_aGraphics[ nMaxGraphics ];
    sal_uInt16                      m_nKeyModifiers;
    GdkCursor                      *m_pCurrentCursor;
    GdkVisibilityState              m_nVisibility;
    PointerStyle                    m_ePointerStyle;
    int                             m_nSavedScreenSaverTimeout;
    guint                           m_nGSMCookie;
    int                             m_nWorkArea;
    bool                            m_bFullscreen;
    bool                            m_bSpanMonitorsWhenFullscreen;
    bool                            m_bDefaultPos;
    bool                            m_bDefaultSize;
    bool                            m_bSendModChangeOnRelease;
    bool                            m_bWindowIsGtkPlug;
    bool                            m_bSetFocusOnMap;
    OUString                   m_aTitle;
    OUString                   m_sWMClass;

    IMHandler*                      m_pIMHandler;

    Size                            m_aMaxSize;
    Size                            m_aMinSize;
    Rectangle                       m_aRestorePosSize;

#if GTK_CHECK_VERSION(3,0,0)
    cairo_region_t*                 m_pRegion;
#else
    GdkRegion*                      m_pRegion;
#endif

    SalMenu*                        m_pSalMenu;

#if defined(ENABLE_DBUS) && defined(ENABLE_GIO)
    public:
    void EnsureDbusMenuSynced();
    private:
    SalMenu*                        m_pLastSyncedDbusMenu;
    friend void ensure_dbus_setup(GdkWindow* gdkWindow, GtkSalFrame* pSalFrame);
    friend void on_registrar_available (GDBusConnection*, const gchar*, const gchar*, gpointer);
    friend void on_registrar_unavailable (GDBusConnection*, const gchar*, gpointer);
#endif
    guint                           m_nWatcherId;

    void Init( SalFrame* pParent, sal_uLong nStyle );
    void Init( SystemParentData* pSysData );
    void InitCommon();
    void InvalidateGraphics();

    // signals
    static gboolean     signalButton( GtkWidget*, GdkEventButton*, gpointer );
    static void         signalStyleSet( GtkWidget*, GtkStyle* pPrevious, gpointer );
#if GTK_CHECK_VERSION(3,0,0)
    static gboolean     signalDraw( GtkWidget*, cairo_t *cr, gpointer );
    static void         signalFlagsChanged( GtkWidget*, GtkStateFlags, gpointer );
#if GTK_CHECK_VERSION(3,14,0)
    static void         gestureSwipe(GtkGestureSwipe* gesture, gdouble velocity_x, gdouble velocity_y, gpointer frame);
    static void         gestureLongPress(GtkGestureLongPress* gesture, gpointer frame);
#endif
#else
    static gboolean     signalExpose( GtkWidget*, GdkEventExpose*, gpointer );
#endif
    static gboolean     signalFocus( GtkWidget*, GdkEventFocus*, gpointer );
    static gboolean     signalMap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalUnmap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalConfigure( GtkWidget*, GdkEventConfigure*, gpointer );
    static gboolean     signalMotion( GtkWidget*, GdkEventMotion*, gpointer );
    static gboolean     signalKey( GtkWidget*, GdkEventKey*, gpointer );
    static gboolean     signalDelete( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalWindowState( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalScroll( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalCrossing( GtkWidget*, GdkEventCrossing*, gpointer );
    static gboolean     signalVisibility( GtkWidget*, GdkEventVisibility*, gpointer );
    static void         signalDestroy( GtkWidget*, gpointer );

    void            Center();
    void            SetDefaultSize();
    void            setAutoLock( bool bLock );
    void            setScreenSaverTimeout( int nTimeout );

    void            doKeyCallback( guint state,
                                   guint keyval,
                                   guint16 hardware_keycode,
                                   guint8 group,
                                   guint32 time,
                                   sal_Unicode aOrigCode,
                                   bool bDown,
                                   bool bSendRelease
                                   );

    static GdkNativeWindow findTopLevelSystemWindow( GdkNativeWindow aWindow );

    static int m_nFloats;

    bool isFloatGrabWindow() const
    {
        return
            (m_nStyle & SAL_FRAME_STYLE_FLOAT) &&                // only a float can be floatgrab
            !(m_nStyle & SAL_FRAME_STYLE_TOOLTIP) &&             // tool tips are not
            !(m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) && // toolbars are also not
            !(m_nStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE);       // focusable floats are not
    }

    bool isChild( bool bPlug = true, bool bSysChild = true )
    {
        sal_uLong nMask = 0;
        if( bPlug )
            nMask |= SAL_FRAME_STYLE_PLUG;
        if( bSysChild )
            nMask |= SAL_FRAME_STYLE_SYSTEMCHILD;
        return (m_nStyle & nMask) != 0;
    }

    //call gtk_window_resize if the current size differs and
    //block Paints until Configure is received and the size
    //is valid again
    void window_resize(long nWidth, long nHeight);
    //call gtk_widget_set_size_request if the current size request differs and
    //block Paints until Configure is received and the size
    //is valid again
    void widget_set_size_request(long nWidth, long nHeight);

    void resizeWindow( long nWidth, long nHeight );
    void moveWindow( long nX, long nY );

    Size calcDefaultSize();

    void setMinMaxSize();
    void createNewWindow( ::Window aParent, bool bXEmbed, SalX11Screen nXScreen );
    void askForXEmbedFocus( sal_Int32 nTimecode );

    void AllocateFrame();
    void TriggerPaintEvent();

    void updateWMClass();
    void SetScreen( unsigned int nNewScreen, int eType, Rectangle *pSize = NULL );

    DECL_LINK( ImplDelayedFullScreenHdl, void* );
public:
#if GTK_CHECK_VERSION(3,0,0)
    basebmp::BitmapDeviceSharedPtr  m_aFrame;
#endif
    GtkSalFrame( SalFrame* pParent, sal_uLong nStyle );
    GtkSalFrame( SystemParentData* pSysData );

    guint                           m_nMenuExportId;
    guint                           m_nAppMenuExportId;
    guint                           m_nActionGroupExportId;
    guint                           m_nAppActionGroupExportId;
    guint                           m_nHudAwarenessId;
    std::vector<gulong>             m_aMouseSignalIds;

    // dispatches an event, returns true if dispatched
    // and false else; if true was returned the event should
    // be swallowed
    bool Dispatch( const XEvent* pEvent );
    void grabPointer(bool bGrab, bool bOwnerEvents = false);
    void grabKeyboard(bool bGrab);

    static GtkSalDisplay*  getDisplay();
    static GdkDisplay*     getGdkDisplay();
    GtkWidget*  getWindow() const { return m_pWindow; }
    GtkFixed*   getFixedContainer() const { return m_pFixedContainer; }
    GtkWidget*  getMouseEventWidget() const;
    GdkWindow*  getForeignParent() const { return m_pForeignParent; }
    GdkNativeWindow getForeignParentWindow() const { return m_aForeignParentWindow; }
    GdkWindow*  getForeignTopLevel() const { return m_pForeignTopLevel; }
    GdkNativeWindow getForeignTopLevelWindow() const { return m_aForeignTopLevelWindow; }
    GdkVisibilityState getVisibilityState() const
    { return m_nVisibility; }
    Pixmap getBackgroundPixmap() const { return m_hBackgroundPixmap; }
    SalX11Screen getXScreenNumber() const { return m_nXScreen; }
    int          GetDisplayScreen() const { return maGeometry.nDisplayScreenNumber; }
    void updateScreenNumber();

#if GTK_CHECK_VERSION(3,0,0)
    // only for gtk3 ...
    cairo_t* getCairoContext() const;
    void damaged (const basegfx::B2IBox& rDamageRect);
#endif
    virtual ~GtkSalFrame();

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*        AcquireGraphics() SAL_OVERRIDE;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual bool                PostEvent( void* pData ) SAL_OVERRIDE;

    virtual void                SetTitle( const OUString& rTitle ) SAL_OVERRIDE;
    virtual void                SetIcon( sal_uInt16 nIcon ) SAL_OVERRIDE;
    virtual void                SetMenu( SalMenu *pSalMenu ) SAL_OVERRIDE;
    SalMenu*                    GetMenu();
    virtual void                DrawMenuBar() SAL_OVERRIDE;
    void                        EnsureAppMenuWatch();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) SAL_OVERRIDE;
    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( bool bVisible, bool bNoActivate = false ) SAL_OVERRIDE;
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void                SetMinClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                SetMaxClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void                GetClientSize( long& rWidth, long& rHeight ) SAL_OVERRIDE;
    virtual void                GetWorkArea( Rectangle& rRect ) SAL_OVERRIDE;
    virtual SalFrame*           GetParent() const SAL_OVERRIDE;
    virtual void                SetWindowState( const SalFrameState* pState ) SAL_OVERRIDE;
    virtual bool                GetWindowState( SalFrameState* pState ) SAL_OVERRIDE;
    virtual void                ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) SAL_OVERRIDE;
    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void                StartPresentation( bool bStart ) SAL_OVERRIDE;
    // Show Window over all other Windows
    virtual void                SetAlwaysOnTop( bool bOnTop ) SAL_OVERRIDE;

    // Window to top and grab focus
    virtual void                ToTop( sal_uInt16 nFlags ) SAL_OVERRIDE;

    // this function can call with the same
    // pointer style
    virtual void                SetPointer( PointerStyle ePointerStyle ) SAL_OVERRIDE;
    virtual void                CaptureMouse( bool bMouse ) SAL_OVERRIDE;
    virtual void                SetPointerPos( long nX, long nY ) SAL_OVERRIDE;

    // flush output buffer
    using SalFrame::Flush;
    virtual void                Flush() SAL_OVERRIDE;

    virtual void                SetInputContext( SalInputContext* pContext ) SAL_OVERRIDE;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) SAL_OVERRIDE;

    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) SAL_OVERRIDE;

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType        GetInputLanguage() SAL_OVERRIDE;

    virtual void                UpdateSettings( AllSettings& rSettings ) SAL_OVERRIDE;

    virtual void                Beep() SAL_OVERRIDE;

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*    GetSystemData() const SAL_OVERRIDE;

    // get current modifier and button mask
    virtual SalPointerState     GetPointerState() SAL_OVERRIDE;

    virtual KeyIndicatorState   GetIndicatorState() SAL_OVERRIDE;

    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) SAL_OVERRIDE;

    // set new parent window
    virtual void                SetParent( SalFrame* pNewParent ) SAL_OVERRIDE;
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window internally
    // return false to indicate failure
    virtual bool                SetPluginParent( SystemParentData* pNewParent ) SAL_OVERRIDE;

    virtual void                SetScreenNumber( unsigned int ) SAL_OVERRIDE;
    virtual void                SetApplicationID( const OUString &rWMClass ) SAL_OVERRIDE;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                ResetClipRegion() SAL_OVERRIDE;
    // start setting the clipregion consisting of nRects rectangles
    virtual void                BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
    // add a rectangle to the clip region
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    // done setting up the clipregion
    virtual void                EndSetClipRegion() SAL_OVERRIDE;

    static GtkSalFrame         *getFromWindow( GtkWindow *pWindow );

    virtual Window              GetX11Window() SAL_OVERRIDE;

    static void                 KeyCodeToGdkKey(const vcl::KeyCode& rKeyCode,
        guint* pGdkKeyCode, GdkModifierType *pGdkModifiers);
};

#define OOO_TYPE_FIXED ooo_fixed_get_type()

extern "C" {

GType ooo_fixed_get_type();
AtkObject* ooo_fixed_get_accessible(GtkWidget *obj);

} // extern "C"

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
