/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: npshell.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2008-01-04 13:09:31 $
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

#ifdef UNIX

#ifndef MOZ_X11
#  define MOZ_X11
#endif

#include <gtk/gtk.h>
#include <X11/Xlib.h>

typedef struct _PluginInstance
{
    uint16 mode;
#ifdef MOZ_X11
    Window window;
    Display *display;
#endif
    uint32 x, y;
    uint32 width, height;
    NPMIMEType type;
    char *message;

    NPP instance;
    char *pluginsPageUrl;
    char *pluginsFileUrl;
    NPBool pluginsHidden;
#ifdef MOZ_X11
    Visual* visual;
    Colormap colormap;
#endif
    unsigned int depth;
    GtkWidget* dialogBox;

    NPBool exists;  /* Does the widget already exist? */
    int action;     /* What action should we take? (GET or REFRESH) */

} PluginInstance;

typedef struct _MimeTypeElement
{
    PluginInstance *pinst;
    struct _MimeTypeElement *next;
} MimeTypeElement;

#endif  //end of UNIX


#ifdef WNT

#include <windows.h>

typedef struct _PluginInstance
{
    NPWindow*            fWindow;
    uint16                fMode;

    HWND                fhWnd;
    WNDPROC                fDefaultWindowProc;
} PluginInstance;

#endif //end of WNT


/* Extern functions */
extern "C" NPMIMEType dupMimeType(NPMIMEType type);
