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

#ifndef _VCL_GTKFRAME_HXX
#define _VCL_GTKFRAME_HXX

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
typedef XLIB_Window GdkNativeWindow;
#define GDK_WINDOW_XWINDOW(o) GDK_WINDOW_XID(o)
#define gdk_set_sm_client_id(i) gdk_x11_set_sm_client_id(i)
#define gdk_window_foreign_new_for_display(a,b) gdk_x11_window_foreign_new_for_display(a,b)
#endif

#if !(GLIB_MAJOR_VERSION > 2 || GLIB_MINOR_VERSION >= 26)
    typedef void GDBusConnection;
#endif

class GtkSalFrame : public SalFrame
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
        //--------------------------------------------------------
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
        void            setInputContext( SalInputContext* pContext );
        void            endExtTextInput( sal_uInt16 nFlags );
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
    int                             m_nDuringRender;
    GdkWindow*                      m_pForeignParent;
    GdkNativeWindow                 m_aForeignParentWindow;
    GdkWindow*                      m_pForeignTopLevel;
    GdkNativeWindow                 m_aForeignTopLevelWindow;
    Pixmap                          m_hBackgroundPixmap;
    sal_uLong                       m_nStyle;
    SalExtStyle                     m_nExtStyle;
    GtkFixed*                       m_pFixedContainer;
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

    // signals
    static gboolean     signalButton( GtkWidget*, GdkEventButton*, gpointer );
    static void         signalStyleSet( GtkWidget*, GtkStyle* pPrevious, gpointer );
    static gboolean     signalDraw( GtkWidget*, cairo_t *cr, gpointer );
    static gboolean     signalExpose( GtkWidget*, GdkEventExpose*, gpointer );
    static gboolean     signalFocus( GtkWidget*, GdkEventFocus*, gpointer );
    static gboolean     signalMap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalUnmap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalConfigure( GtkWidget*, GdkEventConfigure*, gpointer );
    static gboolean     signalMotion( GtkWidget*, GdkEventMotion*, gpointer );
    static gboolean     signalKey( GtkWidget*, GdkEventKey*, gpointer );
    static gboolean     signalDelete( GtkWidget*, GdkEvent*, gpointer );
    static gboolean     signalState( GtkWidget*, GdkEvent*, gpointer );
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


    GdkNativeWindow findTopLevelSystemWindow( GdkNativeWindow aWindow );

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

    void resizeWindow( long nWidth, long nHeight );
    void moveWindow( long nX, long nY );

    Size calcDefaultSize();

    void setMinMaxSize();
    void createNewWindow( XLIB_Window aParent, bool bXEmbed, SalX11Screen nXScreen );
    void askForXEmbedFocus( sal_Int32 nTimecode );

    void AllocateFrame();

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

    // dispatches an event, returns true if dispatched
    // and false else; if true was returned the event should
    // be swallowed
    bool Dispatch( const XEvent* pEvent );
    void grabPointer( sal_Bool bGrab, sal_Bool bOwnerEvents = sal_False );

    GtkSalDisplay*  getDisplay();
    GdkDisplay*     getGdkDisplay();
    GtkWidget*  getWindow() const { return m_pWindow; }
    GtkFixed*   getFixedContainer() const { return m_pFixedContainer; }
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
    void pushIgnoreDamage();
    void popIgnoreDamage();
    bool isDuringRender();
    void renderArea( cairo_t *cr, cairo_rectangle_t *src );
#endif
    virtual ~GtkSalFrame();

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual sal_Bool                PostEvent( void* pData );

    virtual void                SetTitle( const OUString& rTitle );
    virtual void                SetIcon( sal_uInt16 nIcon );
    virtual void                SetMenu( SalMenu *pSalMenu );
    virtual SalMenu*            GetMenu( void );
    virtual void                DrawMenuBar();
    void                        EnsureAppMenuWatch();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = sal_False );
    virtual void                Enable( sal_Bool bEnable );
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual sal_Bool            GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay );
    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void                StartPresentation( sal_Bool bStart );
    // Show Window over all other Windows
    virtual void                SetAlwaysOnTop( sal_Bool bOnTop );

    // Window to top and grab focus
    virtual void                ToTop( sal_uInt16 nFlags );

    // this function can call with the same
    // pointer style
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( sal_Bool bMouse );
    virtual void                SetPointerPos( long nX, long nY );

    // flush output buffer
    using SalFrame::Flush;
    virtual void                Flush();
    // flush output buffer, wait till outstanding operations are done
    virtual void                Sync();

    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( sal_uInt16 nFlags );

    virtual OUString              GetKeyName( sal_uInt16 nKeyCode );
    virtual sal_Bool            MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType        GetInputLanguage();

    virtual void                UpdateSettings( AllSettings& rSettings );

    virtual void                Beep();

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*    GetSystemData() const;


    // get current modifier and button mask
    virtual SalPointerState     GetPointerState();

    virtual SalIndicatorState   GetIndicatorState();

    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode );

    // set new parent window
    virtual void                SetParent( SalFrame* pNewParent );
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window internally
    // return false to indicate failure
    virtual bool                SetPluginParent( SystemParentData* pNewParent );

    virtual void                SetScreenNumber( unsigned int );
    virtual void                SetApplicationID( const OUString &rWMClass );

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                    ResetClipRegion();
    // start setting the clipregion consisting of nRects rectangles
    virtual void                    BeginSetClipRegion( sal_uLong nRects );
    // add a rectangle to the clip region
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // done setting up the clipregion
    virtual void                    EndSetClipRegion();

    static GtkSalFrame             *getFromWindow( GtkWindow *pWindow );

    virtual void                    damaged (const basegfx::B2IBox& rDamageRect);
};

#define OOO_TYPE_FIXED ooo_fixed_get_type()

extern "C" {

GType ooo_fixed_get_type( void );

} // extern "C"

#endif //_VCL_GTKFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
