/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _VCL_GTKDATA_HXX
#define _VCL_GTKDATA_HXX

#include <tools/prex.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <tools/postx.h>

#include <unx/saldisp.hxx>
#include <unx/saldata.hxx>
#include <unx/gtk/gtksys.hxx>
#include <vcl/ptrstyle.hxx>
#include <osl/conditn.h>

#include <list>

class GtkXLib;
class GtkSalDisplay;

inline GdkWindow * widget_get_window(GtkWidget *widget)
{
#if GTK_CHECK_VERSION(3,0,0)
    return gtk_widget_get_window(widget);
#else
    return widget->window;
#endif
}

inline void widget_set_can_focus(GtkWidget *widget, gboolean can_focus)
{
#if GTK_CHECK_VERSION(3,0,0)
    return gtk_widget_set_can_focus(widget, can_focus);
#else
    if (can_focus)
        GTK_WIDGET_SET_FLAGS( widget, GTK_CAN_FOCUS );
    else
        GTK_WIDGET_UNSET_FLAGS( widget, GTK_CAN_FOCUS );
#endif
}

inline void widget_set_can_default(GtkWidget *widget, gboolean can_default)
{
#if GTK_CHECK_VERSION(3,0,0)
    return gtk_widget_set_can_default(widget, can_default);
#else
    if (can_default)
        GTK_WIDGET_SET_FLAGS( widget, GTK_CAN_DEFAULT );
    else
        GTK_WIDGET_UNSET_FLAGS( widget, GTK_CAN_DEFAULT );
#endif
}

class GtkXLib : public SalXLib
{
    GtkSalDisplay       *m_pGtkSalDisplay;
    std::list<GSource *> m_aSources;
    GSource             *m_pTimeout;
    GSource				*m_pUserEvent;
    oslMutex             m_aDispatchMutex;
    oslCondition         m_aDispatchCondition;
    XIOErrorHandler      m_aOrigGTKXIOErrorHandler;

public:
    static gboolean      timeoutFn(gpointer data);
    static gboolean      userEventFn(gpointer data);

    GtkXLib();
    virtual ~GtkXLib();

    virtual void    Init();
    virtual void    Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual void    Insert( int fd, void* data,
                            YieldFunc	pending,
                            YieldFunc	queued,
                            YieldFunc	handle );
    virtual void    Remove( int fd );

    virtual void    StartTimer( sal_uLong nMS );
    virtual void    StopTimer();
    virtual void    Wakeup();
    virtual void    PostUserEvent();
};

#if GTK_CHECK_VERSION(3,0,0)
class GtkData : public SalData
#else
class GtkData : public X11SalData
#endif
{
public:
    GtkData() {}
    virtual ~GtkData();

    virtual void Init();

    virtual void initNWF();
    virtual void deInitNWF();

    GtkSalDisplay *pDisplay;
    GtkSalDisplay *GetDisplay() { return pDisplay; }
#if GTK_CHECK_VERSION(3,0,0)
    GtkXLib *pXLib_;
    SalXLib *GetLib() { return pXLib_; }
#endif
};

inline GtkData* GetGtkSalData()
{ return (GtkData*)ImplGetSVData()->mpSalData; }

class GtkSalFrame;

#if GTK_CHECK_VERSION(3,0,0)
class GtkSalDisplay
#else
class GtkSalDisplay : public SalDisplay
#endif
{
    GtkSalSystem*                   m_pSys;
    GdkDisplay*                     m_pGdkDisplay;
    GdkCursor                      *m_aCursors[ POINTER_COUNT ];
    bool                            m_bStartupCompleted;

    GdkCursor* getFromXBM( const unsigned char *pBitmap, const unsigned char *pMask,
                           int nWidth, int nHeight, int nXHot, int nYHot );
public:
             GtkSalDisplay( GdkDisplay* pDisplay );
    virtual ~GtkSalDisplay();

    GdkDisplay* GetGdkDisplay() const { return m_pGdkDisplay; }

    virtual void registerFrame( SalFrame* pFrame );
    virtual void deregisterFrame( SalFrame* pFrame );
    GdkCursor *getCursor( PointerStyle ePointerStyle );
    virtual int CaptureMouse( SalFrame* pFrame );

    bool IsXinerama() { return !m_pSys->IsMultiDisplay(); }
    int  GetDefaultScreenNumber() { return m_pSys->GetDefaultDisplayNumber(); }
    int  GetScreenCount() { return m_pSys->GetDisplayScreenCount(); }
    Size GetScreenSize( int screen );
    virtual void initScreen( int nScreen ) const;

    GdkFilterReturn filterGdkEvent( GdkXEvent* sys_event,
                                    GdkEvent* event );
    void startupNotificationCompleted() { m_bStartupCompleted = true; }

    void screenSizeChanged( GdkScreen* );
    void monitorsChanged( GdkScreen* );

    void errorTrapPush();
    void errorTrapPop();

    inline bool HasMoreEvents()     { return m_aUserEvents.size() > 1; }
    inline void EventGuardAcquire() { osl_acquireMutex( hEventGuard_ ); }
    inline void EventGuardRelease() { osl_releaseMutex( hEventGuard_ ); }

#if !GTK_CHECK_VERSION(3,0,0)
    virtual long Dispatch( XEvent *pEvent );
#else
    void  SendInternalEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent = SALEVENT_USEREVENT );
    void  CancelInternalEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent );
    bool  DispatchInternalEvent();

    SalFrame *m_pCapture;
    sal_Bool MouseCaptured( const SalFrame *pFrameData ) const
    { return m_pCapture == pFrameData; }
    SalFrame*	GetCaptureFrame() const
    { return m_pCapture; }

    struct SalUserEvent
    {
        SalFrame*		m_pFrame;
        void*			m_pData;
        sal_uInt16			m_nEvent;

        SalUserEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent = SALEVENT_USEREVENT )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };

    oslMutex        hEventGuard_;
    std::list< SalUserEvent > m_aUserEvents;
    guint32 GetLastUserEventTime( bool b ) { return GDK_CURRENT_TIME; } // horrible hack
#endif
};


#endif // _VCL_GTKDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
