/*************************************************************************
 *
 *  $RCSfile: npshell.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-20 10:06:36 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define MIME_TYPES_HANDLED "application/vnd.stardivision.calc:sdc:StarCalc 3.0 - 5.0;\
application/vnd.stardivision.chart:sds:StarChart 3.0 - 5.0;\
application/vnd.stardivision.draw:sda:StarDraw 3.0 - 5.0;\
application/vnd.stardivision.impress:sdd:StarImpress 3.0 - 5.0;\
application/vnd.stardivision.impress-packed:sdp:StarImpress-packed 3.0 - 5.0;\
application/vnd.stardivision.math:smf:StarMath 3.0 - 5.0;\
application/vnd.stardivision.writer:vor:StarWriter Vorlage 3.0 - 5.0;\
application/vnd.stardivision.writer-global:sgl:StarWriter Global 3.0 - 5.0;\
application/vnd.staroffice.writer:sdw:StarWriter;\
application/vnd.sun.xml.calc:sxc:StarCalc;\
application/vnd.sun.xml.calc.template:stc:StarCalc Tempalte;\
application/vnd.sun.xml.chart:sxs:StarChart;\
application/vnd.sun.xml.chart.template:sts:StarChart Template;\
application/vnd.sun.xml.draw:sxd:StarDraw;\
application/vnd.sun.xml.draw.template:std:StarDraw Template;\
application/vnd.sun.xml.impress:sxi:StarImpress;\
application/vnd.sun.xml.impress.template:sti:StarImpress Template;\
application/vnd.sun.xml.math:sxm:StarMath;\
application/vnd.sun.xml.writer:sxw:StarWriter;\
application/vnd.sun.xml.writer.global:sxg:StarWriter Global;\
application/vnd.sun.xml.writer.template:stw:StarWriter Template;\
application/msword:doc:Microsoft Word Document;\
application/vnd.ms-excel:xls:Microsoft Excel Document;\
application/vnd.ms-powerpoint:ppt:Microsoft Powerpoint Document"

#define PLUGIN_NAME         "StarOffice Plugin"
#define PLUGIN_DESCRIPTION  "The StarOffice plugin handles all staroffice documents"

#ifdef UNIX

#define MOZ_X11

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
