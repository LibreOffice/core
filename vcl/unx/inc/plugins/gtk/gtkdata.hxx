/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gtkdata.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:02:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCL_GTKDATA_HXX
#define _VCL_GTKDATA_HXX

#include <prex.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <postx.h>

#include <saldisp.hxx>
#include <saldata.hxx>
#include <ptrstyle.hxx>

#include <list>

class GtkData : public X11SalData
{
public:
    GtkData() {}
    virtual ~GtkData();

    virtual void Init();

    virtual void initNWF();
    virtual void deInitNWF();
};

class GtkSalFrame;

class GtkSalDisplay : public SalDisplay
{
    GdkDisplay*                     m_pGdkDisplay;
    GdkCursor                      *m_aCursors[ POINTER_COUNT ];
    bool                            m_bStartupCompleted;
    GdkCursor* getFromXPM( const char *pBitmap, const char *pMask,
                           int nWidth, int nHeight, int nXHot, int nYHot );
public:
             GtkSalDisplay( GdkDisplay* pDisplay );
    virtual ~GtkSalDisplay();

    GdkDisplay* GetGdkDisplay() const { return m_pGdkDisplay; }

    virtual void deregisterFrame( SalFrame* pFrame );
    GdkCursor *getCursor( PointerStyle ePointerStyle );
    virtual int CaptureMouse( SalFrame* pFrame );
    virtual long Dispatch( XEvent *pEvent );

    static GdkFilterReturn filterGdkEvent( GdkXEvent* sys_event,
                                           GdkEvent* event,
                                           gpointer data );
    inline bool HasMoreEvents()     { return m_aUserEvents.size() > 1; }
    inline void EventGuardAcquire() { osl_acquireMutex( hEventGuard_ ); }
    inline void EventGuardRelease() { osl_releaseMutex( hEventGuard_ ); }
    void startupNotificationCompleted() { m_bStartupCompleted = true; }
};


#endif // _VCL_GTKDATA_HXX
