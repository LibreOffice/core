/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gtksys.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:29:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <svunx.h>
#include <svdata.hxx>
#include <window.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <cstdio>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

SalSystem *GtkInstance::CreateSalSystem()
{
        return new GtkSalSystem();
}

GtkSalSystem::~GtkSalSystem()
{
}

int GtkSalSystem::ShowNativeDialog( const String& rTitle,
                                    const String& rMessage,
                                    const std::list< String >& rButtons,
                                    int nDefButton )
{

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
            pSVData->mpIntroWindow->Hide();

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkSalSystem::ShowNativeDialog\n");
#endif

    ByteString aTitle( rTitle, RTL_TEXTENCODING_UTF8 );
    ByteString aMessage( rMessage, RTL_TEXTENCODING_UTF8 );

    /* Create the dialogue */
    GtkWidget* mainwin = gtk_message_dialog_new
            ( NULL, (GtkDialogFlags)0, GTK_MESSAGE_WARNING,
              GTK_BUTTONS_NONE, aMessage.GetBuffer(), NULL );
    gtk_window_set_title( GTK_WINDOW( mainwin ), aTitle.GetBuffer() );

    gint nButtons = 0, nResponse;

    int nButton = 0;
    for( std::list< String >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
    {
        ByteString aLabel( *it, RTL_TEXTENCODING_UTF8 );

        if( nButton == nDefButton )
        {
            gtk_dialog_add_button( GTK_DIALOG( mainwin ), aLabel.GetBuffer(), nButtons );
            gtk_dialog_set_default_response( GTK_DIALOG( mainwin ), nButtons );
        }
        else
            gtk_dialog_add_button( GTK_DIALOG( mainwin ), aLabel.GetBuffer(), nButtons );
        nButtons++;
    }

    nResponse = gtk_dialog_run( GTK_DIALOG(mainwin) );
    if( nResponse == GTK_RESPONSE_NONE || nResponse == GTK_RESPONSE_DELETE_EVENT )
        nResponse = -1;

    gtk_widget_destroy( GTK_WIDGET(mainwin) );

    return nResponse;
}
