/*************************************************************************
 *
 *  $RCSfile: i18n_wrp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: svesik $ $Date: 2001-02-16 01:06:39 $
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

struct XIMArg
{
    char *name;
    char *value;
};

#if !defined(LINUX) && !defined(FREEBSD) && !defined(NETBSD)
#include <varargs.h>
#else
#include <stdarg.h>
#endif
#include <string.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include "XIM.h"


#ifdef SOLARIS
#define XIIIMP_PATH     "/usr/openwin/lib/locale/common/xiiimp.so.2"
#else /* Linux */
#define XIIIMP_PATH     "/usr/lib/im/xiiimp.so.2"
#endif

/* global variables */
static void *g_dlmodule = 0;

extern "C" {
typedef XIM (*OpenFunction)(Display*, XrmDatabase, char*, char*, XIMArg*);
}

static OpenFunction g_open_im = (OpenFunction)NULL;

/* utility function to transform vararg list into an array of XIMArg */

int
XvaCountArgs( XIMArg *pInArgs )
{
    int nArgs = 0;
    char *pName, *pValue;

    while ( (pName = pInArgs->name) != NULL )
    {
        pValue = pInArgs->value;

        if ( strcmp(pName, XNVaNestedList) == 0 )
        {
            nArgs += XvaCountArgs( (XIMArg*)pValue );
        }
        else
        {
            nArgs += 1;
        }
        pInArgs++;
    }

    return nArgs;
}

int
XvaCountArgs( va_list pInArgs )
{
    int nArgs = 0;
    char *pName, *pValue;

    while ( (pName = va_arg(pInArgs, char*)) != NULL)
    {
        pValue = va_arg(pInArgs, char*);

        if ( strcmp(pName, XNVaNestedList) == 0 )
        {
            nArgs += XvaCountArgs( (XIMArg*)pValue );
        }
        else
        {
            nArgs += 1;
        }
    }

    return nArgs;
}

XIMArg*
XvaGetArgs( XIMArg *pInArgs, XIMArg *pOutArgs )
{
    char *pName, *pValue;

    while ( (pName = pInArgs->name) != NULL )
    {
        pValue = pInArgs->value;

        if ( strcmp(pName, XNVaNestedList) == 0 )
        {
            pOutArgs = XvaGetArgs( (XIMArg*)pValue, pOutArgs );
        }
        else
        {
            pOutArgs->name  = pName;
            pOutArgs->value = pValue;
            pOutArgs++;
        }
        pInArgs++;
    }

    return pOutArgs;
}

void
XvaGetArgs( va_list pInArgs, XIMArg *pOutArgs )
{
    char *pName, *pValue;

    while ((pName = va_arg(pInArgs, char*)) != NULL)
    {
        pValue = va_arg(pInArgs, char*);

        if ( strcmp(pName, XNVaNestedList) == 0 )
        {
            pOutArgs = XvaGetArgs( (XIMArg*)pValue, pOutArgs );
        }
        else
        {
            pOutArgs->name  = pName;
            pOutArgs->value = pValue;
            pOutArgs++;
        }
    }

    pOutArgs->name  = NULL;
    pOutArgs->value = NULL;
}


/* Puplic functions */

#ifdef __cplusplus
extern "C"
#endif
XIM
XvaOpenIM(Display *display, XrmDatabase rdb,
        char *res_name, char *res_class, ...)
{
      XIM xim = (XIM)0;
      va_list variable;
      int total_count = 0;

      /*
        * so count the stuff dangling here
     */

#if defined(LINUX) || defined(FREEBSD) || defined(NETBSD)
    va_start(variable, res_class);
#else
      va_start(variable);
#endif
      total_count = XvaCountArgs(variable);
      va_end(variable);

      if (total_count > 0)
    {
        /* call a new open IM method */

        XIMArg* args = (XIMArg*)Xmalloc( (total_count + 1) * sizeof(XIMArg) );

        /*
          * now package it up so we can set it along
          */
#if defined(LINUX) || defined(FREEBSD) || defined(NETBSD)

        va_start(variable, res_class);
#else
        va_start(variable);
#endif
        XvaGetArgs( variable, args );
        va_end(variable);

        if (!g_dlmodule)
        {
              g_dlmodule = dlopen(XIIIMP_PATH, RTLD_LAZY);
              if (!g_dlmodule)
                goto legacy_XIM;

              g_open_im = (OpenFunction)(long)dlsym(g_dlmodule, "__XOpenIM");
              if (!g_open_im)
                goto legacy_XIM;

              xim = (*g_open_im)(display, (XrmDatabase)rdb,
                  (char*)res_name, (char *)res_class, (XIMArg*)args);
        }
        else
        {
              goto legacy_XIM;
        }
      }

     legacy_XIM:

    if (!xim)
        xim = XOpenIM(display, rdb, res_name, res_class);

    return xim;
}

/*
 * Close the connection to the input manager, and free the XIM structure
 */

Status
XvaCloseIM(XIM im)
{
      Status s;
    #if 0
      XCloseIM(im);
      s = (im->methods->close)(im); /* we can use the same close module  */
    #endif

    if (!g_dlmodule)
    {
        /* assuming one XvaOpenIM call */
        dlclose(g_dlmodule);
            g_dlmodule = (void*)0;
        g_open_im = (OpenFunction)NULL;
      }
    #if 0
      if (im)
        Xfree((char *)im);
    #endif

    return (s);
}



