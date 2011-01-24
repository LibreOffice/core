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

struct XIMArg
{
    char *name;
    char *value;
};

#if defined(SOLARIS) && !defined(__GNUC__)
#include <varargs.h>
#else
#include <stdarg.h>
#endif
#include <sal/alloca.h>

#include <string.h>
#include <dlfcn.h>

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include "unx/XIM.h"

#define XIIIMP_LIB       "xiiimp.so.2"

#ifdef SOLARIS
#define XIIIMP_PATH     "/usr/openwin/lib/locale/common/" XIIIMP_LIB
#else /* Linux */
#define XIIIMP_PATH     "/usr/lib/im/" XIIIMP_LIB
#endif

extern "C" {
typedef XIM (*OpenFunction)(Display*, XrmDatabase, char*, char*, XIMArg*);
}

/* global variables */
static void *g_dlmodule = 0;
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

#if defined(SOLARIS) && !defined(__GNUC__)
      va_start(variable);
#else
    va_start(variable, res_class);
#endif
      total_count = XvaCountArgs(variable);
      va_end(variable);

      if (total_count > 0)
    {
        /* call a new open IM method */

        XIMArg* args = (XIMArg*)alloca( (total_count + 1) * sizeof(XIMArg) );

        /*
          * now package it up so we can set it along
          */
#if defined(SOLARIS) && !defined(__GNUC__)
        va_start(variable);
#else
        va_start(variable, res_class);
#endif
        XvaGetArgs( variable, args );
        va_end(variable);

        if (!g_dlmodule)
        {
            g_dlmodule = dlopen(XIIIMP_LIB, RTLD_LAZY);
            if(!g_dlmodule)
            {
                g_dlmodule = dlopen(XIIIMP_PATH, RTLD_LAZY);
                if (!g_dlmodule)
                    goto legacy_XIM;
            }
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

// in #if to prevent warning "warning: label 'legacy_XIM' defined but not used"
     legacy_XIM:

    if (!xim)
        xim = XOpenIM(display, rdb, res_name, res_class);

    return xim;
}

/*
 * Close the connection to the input manager, and free the XIM structure
 */

Status XvaCloseIM(XIM)
{
      Status s = False;

    if (!g_dlmodule)
    {
        /* assuming one XvaOpenIM call */
        dlclose(g_dlmodule);
            g_dlmodule = (void*)0;
        g_open_im = (OpenFunction)NULL;
        s = True;
      }
    return (s);
}



