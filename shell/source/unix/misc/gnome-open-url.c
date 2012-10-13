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

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

typedef int   gboolean;
typedef char  gchar;
typedef struct _GError GError;

struct _GError
{
  int domain;
  int code;
  char *message;
};

typedef enum {
  GNOME_VFS_OK
} GnomeVFSResult;

/*
 * Wrapper function which extracs gnome_url_show from libgnome
 */

gboolean gnome_url_show (const char *url, GError **error)
{
    void* handle = dlopen("libgnomevfs-2.so.0", RTLD_LAZY);
    gboolean ret = 0;

    (void)error; /* avoid warning due to unused parameter */

    if( NULL != handle )
    {
        gboolean (* init) (void) =
            (gboolean (*) (void)) dlsym(handle, "gnome_vfs_init");

        if( NULL != init && init() )
        {
            GnomeVFSResult (* func) (const char *url) =
                (GnomeVFSResult (*) (const char *)) dlsym(handle, "gnome_vfs_url_show");

            if( NULL != func )
                ret = (GNOME_VFS_OK == func(url));
        }

        dlclose(handle);
    }

    return ret;
}

/*
 * The intended use of this tool is to pass the argument to
 * the gnome_show_url function of libgnome2.
 */

int main(int argc, char *argv[] )
{
    GError *error = NULL;
    char *fallback;
    char *idx;
    int retcode = -1;

    if( argc != 2 )
    {
        fprintf( stderr, "Usage: gnome-open-url <uri>\n" );
        return -1;
    }

    if( gnome_url_show(argv[1], &error) )
    {
        return 0;
    }

    /*
     * launch open-url command by replacing gnome-open-url from
     * the command line. This is the fallback when running on
     * remote machines with no GNOME installed.
     */

    fallback = strdup(argv[0]);
    idx = strstr(fallback, "gnome-open-url");
    if ( NULL != idx )
    {
        char *args[3];
        strncpy(idx, "open-url", 9);
        args[0] = fallback;
        args[1] = argv[1];
        args[2] = NULL;
        retcode = execv(fallback, args);
    }
    free(fallback);

    return retcode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
