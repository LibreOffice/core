/*************************************************************************
 *
 *  $RCSfile: gtkframe.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:51:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

class X11SalGraphics;
class GtkSalDisplay;

class GtkSalFrame : public SalFrame
{
    static const int nMaxGraphics = 2;

    struct GraphicsHolder
    {
        X11SalGraphics*     pGraphics;
        bool                bInUse;
        GraphicsHolder()
                : pGraphics( NULL ),
                  bInUse( false )
        {}
        ~GraphicsHolder();
    };

    GtkWindow*              m_pWindow;
    GdkWindow*              m_pForeignParent;
    GdkWindow*              m_pForeignTopLevel;
    ULONG                   m_nStyle;
    GtkFixed*               m_pFixedContainer;
    GtkSalFrame*            m_pParent;
    GdkWindowState          m_nState;
    GtkIMContext*           m_pIMContext;
    SystemEnvData           m_aSystemData;
    GraphicsHolder          m_aGraphics[ nMaxGraphics ];
    USHORT                  m_nKeyModifiers;
    GdkCursor              *m_pCurrentCursor;
    GdkVisibilityState      m_nVisibility;
    int                     m_nSavedScreenSaverTimeout;
    bool                    m_bResizeable;
    bool                    m_bSingleAltPress;
    bool                    m_bDefaultPos;
    bool                    m_bDefaultSize;
    bool                    m_bSendModChangeOnRelease;
    bool                    m_bWasPreedit;

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
    static void         signalIMCommit( GtkIMContext*, gchar*, gpointer );
    static gboolean     signalIMDeleteSurrounding( GtkIMContext*, gint, gint, gpointer );
    static void         signalIMPreeditChanged( GtkIMContext*, gpointer );
    static void         signalIMPreeditEnd( GtkIMContext*, gpointer );
    static void         signalIMPreeditStart( GtkIMContext*, gpointer );
    static gboolean     signalIMRetrieveSurrounding( GtkIMContext*, gpointer );


    GtkSalDisplay*  getDisplay();
    GdkDisplay*     getGdkDisplay();
    void            Center();
    void            SetDefaultSize();
    void            setAutoLock( bool bLock );
    void            setScreenSaverTimeout( int nTimeout );

    GdkNativeWindow findTopLevelSystemWindow( GdkNativeWindow aWindow );

    static int m_nFloats;

    bool isFloatGrabWindow() const
    {
        return
            (m_nStyle & SAL_FRAME_STYLE_FLOAT) &&       // only a float can be floatgrab
            !(m_nStyle & SAL_FRAME_STYLE_TOOLTIP);      // tool tips are not
    }

    Size calcDefaultSize();

public:
    GtkSalFrame( SalFrame* pParent, ULONG nStyle );
    GtkSalFrame( SystemParentData* pSysData );

    // dispatches an event, returns true if dispatched
    // and false else; if true was returned the event should
    // be swallowed
    bool Dispatch( const XEvent* pEvent );
    void grabPointer( BOOL bGrab, BOOL bOwnerEvents = FALSE );

    GtkWindow*  getWindow() const { return m_pWindow; }
    GtkFixed*   getFixedContainer() const { return m_pFixedContainer; }
    GdkWindow*  getForeignParent() const { return m_pForeignParent; }
    GdkWindow*  getForeignTopLevel() const { return m_pForeignTopLevel; }
    GdkVisibilityState getVisibilityState() const
    { return m_nVisibility; }

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
    virtual void              SetMenu( SalMenu *pSalMenu );
    virtual void              DrawMenuBar();

    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    virtual void                Enable( BOOL bEnable );
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void              SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual BOOL                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( BOOL bFullScreen );
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
    virtual void                Flush();
    // flush output buffer, wait till outstanding operations are done
    virtual void                Sync();

    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( USHORT nFlags );

    virtual String              GetKeyName( USHORT nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );

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

};


#endif //_VCL_GTKFRAME_HXX
