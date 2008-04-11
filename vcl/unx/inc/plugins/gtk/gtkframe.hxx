/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gtkframe.hxx,v $
 * $Revision: 1.34 $
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

#ifndef _VCL_GTKFRAME_HXX
#define _VCL_GTKFRAME_HXX

#include <prex.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <postx.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessible.hdl>
#include <vcl/salframe.hxx>
#include <vcl/sysdata.hxx>

#include <list>
#include <vector>

class GtkSalGraphics;
class GtkSalDisplay;

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
                    && (event->state == state)
                    && (event->keyval == keyval)
                    && (event->hardware_keycode == hardware_keycode)
                    && (event->group == group)
                    && (event->time - time < 3)
                    ;
            }
        };


        GtkSalFrame*                    m_pFrame;
        std::list< PreviousKeyPress >   m_aPrevKeyPresses;
        int                             m_nPrevKeyPresses; // avoid using size()
        GtkIMContext*                   m_pIMContext;
        bool                            m_bFocused;
        SalExtTextInputEvent            m_aInputEvent;
        std::vector< USHORT >           m_aInputFlags;

        IMHandler( GtkSalFrame* );
        ~IMHandler();

        void            createIMContext();
        void            deleteIMContext();
        void            updateIMSpotLocation();
        void            setInputContext( SalInputContext* pContext );
        void            endExtTextInput( USHORT nFlags );
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

    int                             m_nScreen;
    GtkWidget*                      m_pWindow;
    GdkWindow*                      m_pForeignParent;
    GdkNativeWindow                 m_aForeignParentWindow;
    GdkWindow*                      m_pForeignTopLevel;
    GdkNativeWindow                 m_aForeignTopLevelWindow;
    Pixmap                          m_hBackgroundPixmap;
    ULONG                           m_nStyle;
    SalExtStyle                     m_nExtStyle;
    GtkFixed*                       m_pFixedContainer;
    GtkSalFrame*                    m_pParent;
    std::list< GtkSalFrame* >       m_aChildren;
    GdkWindowState                  m_nState;
    SystemEnvData                   m_aSystemData;
    GraphicsHolder                  m_aGraphics[ nMaxGraphics ];
    USHORT                          m_nKeyModifiers;
    GdkCursor                      *m_pCurrentCursor;
    GdkVisibilityState              m_nVisibility;
    PointerStyle                    m_ePointerStyle;
    int                             m_nSavedScreenSaverTimeout;
    guint                           m_nGSSCookie;
    int                             m_nWorkArea;
    bool                            m_bFullscreen;
    bool                            m_bSingleAltPress;
    bool                            m_bDefaultPos;
    bool                            m_bDefaultSize;
    bool                            m_bSendModChangeOnRelease;
    bool                            m_bWindowIsGtkPlug;
    String                          m_aTitle;

    IMHandler*                      m_pIMHandler;

    Size                            m_aMaxSize;
    Size                            m_aMinSize;
    Rectangle                       m_aRestorePosSize;

    GdkRegion*                      m_pRegion;

    void Init( SalFrame* pParent, ULONG nStyle );
    void Init( SystemParentData* pSysData );
    void InitCommon();

    // signals
    static gboolean     signalButton( GtkWidget*, GdkEventButton*, gpointer );
    static void         signalStyleSet( GtkWidget*, GtkStyle* pPrevious, gpointer );
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
    static void         signalDestroy( GtkObject*, gpointer );

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
            (m_nStyle & SAL_FRAME_STYLE_FLOAT) &&       // only a float can be floatgrab
            !(m_nStyle & SAL_FRAME_STYLE_TOOLTIP) &&    // tool tips are not
            !(m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION); // toolbars are also not
    }

    bool isChild( bool bPlug = true, bool bSysChild = true )
    {
        ULONG nMask = 0;
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
    void createNewWindow( XLIB_Window aParent, bool bXEmbed, int nScreen );
    void askForXEmbedFocus( sal_Int32 nTimecode );
public:
    GtkSalFrame( SalFrame* pParent, ULONG nStyle );
    GtkSalFrame( SystemParentData* pSysData );

    // dispatches an event, returns true if dispatched
    // and false else; if true was returned the event should
    // be swallowed
    bool Dispatch( const XEvent* pEvent );
    void grabPointer( BOOL bGrab, BOOL bOwnerEvents = FALSE );

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
    int getScreenNumber() const { return m_nScreen; }
    void updateScreenNumber();

    void moveToScreen( int nScreen );

    virtual ~GtkSalFrame();

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual BOOL                PostEvent( void* pData );

    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( USHORT nIcon );
    virtual void                SetMenu( SalMenu *pSalMenu );
    virtual void                DrawMenuBar();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    virtual void                Enable( BOOL bEnable );
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual BOOL                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( BOOL bFullScreen, sal_Int32 nDisplay );
    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void                StartPresentation( BOOL bStart );
    // Show Window over all other Windows
    virtual void                SetAlwaysOnTop( BOOL bOnTop );

    // Window to top and grab focus
    virtual void                ToTop( USHORT nFlags );

    // this function can call with the same
    // pointer style
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( BOOL bMouse );
    virtual void                SetPointerPos( long nX, long nY );

    // flush output buffer
    using SalFrame::Flush;
    virtual void                Flush();
    // flush output buffer, wait till outstanding operations are done
    virtual void                Sync();

    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( USHORT nFlags );

    virtual String              GetKeyName( USHORT nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );
    virtual BOOL                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType        GetInputLanguage();

    virtual SalBitmap*          SnapShot();

    virtual void                UpdateSettings( AllSettings& rSettings );

    virtual void                Beep( SoundType eSoundType );

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*    GetSystemData() const;


    // get current modifier and button mask
    virtual SalPointerState     GetPointerState();

    // set new parent window
    virtual void                SetParent( SalFrame* pNewParent );
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window internally
    // return false to indicate failure
    virtual bool                SetPluginParent( SystemParentData* pNewParent );

    virtual void                SetBackgroundBitmap( SalBitmap* );

    virtual void                SetScreenNumber( unsigned int );

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                    ResetClipRegion();
    // start setting the clipregion consisting of nRects rectangles
    virtual void                    BeginSetClipRegion( ULONG nRects );
    // add a rectangle to the clip region
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // done setting up the clipregion
    virtual void                    EndSetClipRegion();

    static GtkSalFrame         *getFromWindow( GtkWindow *pWindow );

    static AtkRole              GetAtkRole( GtkWindow* window );
};


#define OOO_TYPE_FIXED ooo_fixed_get_type()

extern "C" {

GType ooo_fixed_get_type( void );

} // extern "C"

#endif //_VCL_GTKFRAME_HXX
