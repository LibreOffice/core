/*************************************************************************
 *
 *  $RCSfile: gtkobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:53:24 $
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

#include <plugins/gtk/gtkobject.hxx>
#include <plugins/gtk/gtkframe.hxx>
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>

GtkSalObject::GtkSalObject( GtkSalFrame* pParent )
        : m_pSocket( NULL ),
          m_pRegion( NULL )
{
    if( pParent )
    {
        // our plug window
        m_pSocket = gtk_drawing_area_new();
        // insert into container
        gtk_fixed_put( pParent->getFixedContainer(),
                       m_pSocket,
                       0, 0 );
        // realize so we can get a window id
        gtk_widget_realize( m_pSocket );


        // make it transparent; some plugins may not insert
        // their own window here but use the socket window itself
        gtk_widget_set_app_paintable( m_pSocket, TRUE );

        //system data
        SalDisplay* pDisp = GetSalData()->GetDisplay();
        m_aSystemData.pDisplay      = pDisp->GetDisplay();
        m_aSystemData.aWindow       = GDK_WINDOW_XWINDOW(m_pSocket->window);
        m_aSystemData.pSalFrame     = NULL;
        m_aSystemData.pWidget       = m_pSocket;
        m_aSystemData.pVisual       = pDisp->GetVisual()->GetVisual();
        m_aSystemData.nDepth        = pDisp->GetVisual()->GetDepth();
        m_aSystemData.aColormap     = pDisp->GetColormap().GetXColormap();
        m_aSystemData.pAppContext   = NULL;
        m_aSystemData.aShellWindow  = GDK_WINDOW_XWINDOW(GTK_WIDGET(pParent->getWindow())->window);
        m_aSystemData.pShellWidget  = GTK_WIDGET(pParent->getWindow());

        g_signal_connect( G_OBJECT(m_pSocket), "button-press-event", G_CALLBACK(signalButton), this );
        g_signal_connect( G_OBJECT(m_pSocket), "button-release-event", G_CALLBACK(signalButton), this );
        g_signal_connect( G_OBJECT(m_pSocket), "focus-in-event", G_CALLBACK(signalFocus), this );
        g_signal_connect( G_OBJECT(m_pSocket), "focus-out-event", G_CALLBACK(signalFocus), this );
    }
}

GtkSalObject::~GtkSalObject()
{
    if( m_pRegion )
        gdk_region_destroy( m_pRegion );
    if( m_pSocket )
    {
        // remove socket from parent frame's fixed container
        gtk_container_remove( GTK_CONTAINER(gtk_widget_get_parent(m_pSocket)),
                              m_pSocket );
        // get rid of the socket
        gtk_widget_destroy( m_pSocket );
    }
}

void GtkSalObject::ResetClipRegion()
{
    gdk_window_shape_combine_region( m_pSocket->window, NULL, 0, 0 );
}

USHORT GtkSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

void GtkSalObject::BeginSetClipRegion( ULONG nRects )
{
    if( m_pRegion )
        gdk_region_destroy( m_pRegion );
    m_pRegion = gdk_region_new();
}

void GtkSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    GdkRectangle aRect;
    aRect.x         = nX;
    aRect.y         = nY;
    aRect.width     = nWidth;
    aRect.height    = nHeight;

    gdk_region_union_with_rect( m_pRegion, &aRect );
}

void GtkSalObject::EndSetClipRegion()
{
    gdk_window_shape_combine_region( m_pSocket->window, m_pRegion, 0, 0 );
}

void GtkSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    if( m_pSocket )
    {
        GtkFixed* pContainer = GTK_FIXED(gtk_widget_get_parent(m_pSocket));
        gtk_fixed_move( pContainer, m_pSocket, nX, nY );
        gtk_widget_set_size_request( m_pSocket, nWidth, nHeight );
        gtk_container_resize_children( GTK_CONTAINER(pContainer) );
    }
}

void GtkSalObject::Show( BOOL bVisible )
{
    if( m_pSocket )
    {
        if( bVisible )
            gtk_widget_show( m_pSocket );
        else
            gtk_widget_hide( m_pSocket );
    }
}

void GtkSalObject::Enable( BOOL nEnable )
{
}

void GtkSalObject::GrabFocus()
{
}

void GtkSalObject::SetBackground()
{
}

void GtkSalObject::SetBackground( SalColor nSalColor )
{
}

const SystemEnvData* GtkSalObject::GetSystemData() const
{
    return &m_aSystemData;
}


gboolean GtkSalObject::signalButton( GtkWidget* pWidget, GdkEventButton* pEvent, gpointer object )
{
    GtkSalObject* pThis = (GtkSalObject*)object;

    if( pEvent->type == GDK_BUTTON_PRESS )
    {
        GTK_YIELD_GRAB();
        pThis->CallCallback( SALOBJ_EVENT_TOTOP, NULL );
    }

    return FALSE;
}

gboolean GtkSalObject::signalFocus( GtkWidget* pWidget, GdkEventFocus* pEvent, gpointer object )
{
    GtkSalObject* pThis = (GtkSalObject*)object;

    GTK_YIELD_GRAB();

    pThis->CallCallback( pEvent->in ? SALOBJ_EVENT_GETFOCUS : SALOBJ_EVENT_LOSEFOCUS, NULL );

    return FALSE;
}
