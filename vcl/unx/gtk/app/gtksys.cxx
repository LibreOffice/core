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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <unx/svunx.h>
#include <svdata.hxx>

#include <vcl/window.hxx>
#include <unx/gtk/gtkinst.hxx>
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

#if GTK_CHECK_VERSION(3,0,0)
unsigned int GtkSalSystem::GetDisplayScreenCount()
{
    return 1;
}

bool GtkSalSystem::IsMultiDisplay()
{
    return false;
}

unsigned int GtkSalSystem::GetDefaultDisplayNumber()
{
    return 0;
}

Rectangle GtkSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    g_warning ("FIXME: GetDisplayScreenPosSizePixel unimplemented");
    return Rectangle (0, 0, 1024, 768);
}

Rectangle GtkSalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString GtkSalSystem::GetScreenName( unsigned int nScreen )
{
    return rtl::OUString::createFromAscii( "Jim" );
}

// FIXME: shocking cut/paste from X11SalSystem ... [!] - push me lower ...
#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>

int GtkSalSystem::ShowNativeMessageBox( const String& rTitle,
                                        const String& rMessage,
                                        int nButtonCombination,
                                        int nDefaultButton)
{
    int nDefButton = 0;
    std::list< String > aButtons;
    int nButtonIds[5], nBut = 0;

    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_OK ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_YES ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES;
        aButtons.push_back( Button::GetStandardText( BUTTON_NO ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO )
            nDefButton = 1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
    {
        if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
        {
            aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
            nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        }
        aButtons.push_back( Button::GetStandardText( BUTTON_CANCEL ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL )
            nDefButton = aButtons.size()-1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_ABORT ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT;
        aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        aButtons.push_back( Button::GetStandardText( BUTTON_IGNORE ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE;
        switch( nDefaultButton )
        {
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY: nDefButton = 1;break;
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE: nDefButton = 2;break;
        }
    }
    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons, nDefButton );

    return nResult != -1 ? nButtonIds[ nResult ] : 0;
}
#endif

// convert ~ to indicate mnemonic to '_'
static rtl::OString MapToGtkAccelerator(const String &rStr)
{
    String aRet( rStr );
    aRet.SearchAndReplaceAscii("~", String::CreateFromAscii("_"));
    return rtl::OUStringToOString(aRet, RTL_TEXTENCODING_UTF8);
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
    std::fprintf( stderr, "GtkSalSystem::ShowNativeDialog\n");
#endif

    rtl::OString aTitle(rtl::OUStringToOString(rTitle,
        RTL_TEXTENCODING_UTF8));
    rtl::OString aMessage(rtl::OUStringToOString(rMessage,
        RTL_TEXTENCODING_UTF8));

    /* Create the dialogue */
    GtkWidget* mainwin = gtk_message_dialog_new
            ( NULL, (GtkDialogFlags)0, GTK_MESSAGE_WARNING,
              GTK_BUTTONS_NONE, aMessage.getStr(), NULL );
    gtk_window_set_title( GTK_WINDOW( mainwin ), aTitle.getStr() );

    gint nButtons = 0, nResponse;

    int nButton = 0;
    for( std::list< String >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
    {
        if( nButton == nDefButton )
        {
            gtk_dialog_add_button(GTK_DIALOG( mainwin ),
                MapToGtkAccelerator(*it).getStr(), nButtons);
            gtk_dialog_set_default_response(GTK_DIALOG(mainwin), nButtons);
        }
        else
        {
            rtl::OString aLabel(rtl::OUStringToOString(*it,
                RTL_TEXTENCODING_UTF8));
            gtk_dialog_add_button(GTK_DIALOG(mainwin), aLabel.getStr(),
                nButtons);
        }
        nButtons++;
    }

    nResponse = gtk_dialog_run( GTK_DIALOG(mainwin) );
    if( nResponse == GTK_RESPONSE_NONE || nResponse == GTK_RESPONSE_DELETE_EVENT )
        nResponse = -1;

    gtk_widget_destroy( GTK_WIDGET(mainwin) );

    return nResponse;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
