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

#ifndef INCLUDED_EXTENSIONS_SOURCE_NSPLUGIN_SOURCE_NPSHELL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_NSPLUGIN_SOURCE_NPSHELL_HXX

#ifdef UNIX

#ifndef MOZ_X11
#  define MOZ_X11
#endif

#include <gtk/gtk.h>
#include <X11/Xlib.h>

typedef struct _PluginInstance
{
    uint16_t mode;
#ifdef MOZ_X11
    Window window;
    Display *display;
#endif
    uint32_t x, y;
    uint32_t width, height;
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
    uint16_t                fMode;

    HWND                fhWnd;
    WNDPROC                fDefaultWindowProc;
} PluginInstance;

#endif //end of WNT


/* Extern functions */
extern "C" NPMIMEType dupMimeType(NPMIMEType type);

#endif // INCLUDED_EXTENSIONS_SOURCE_NSPLUGIN_SOURCE_NPSHELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
