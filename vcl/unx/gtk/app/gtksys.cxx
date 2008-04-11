/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gtksys.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <svunx.h>
#include <vcl/svdata.hxx>
#include <vcl/window.hxx>
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
