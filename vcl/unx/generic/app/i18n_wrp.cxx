/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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

    if (g_dlmodule)
    {
        /* assuming one XvaOpenIM call */
        dlclose(g_dlmodule);
            g_dlmodule = (void*)0;
        g_open_im = (OpenFunction)NULL;
        s = True;
      }
    return (s);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
