/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
 * HACK: avoid error messages caused by not setting a GNOME program name
 */

gchar* gnome_gconf_get_gnome_libs_settings_relative (const gchar *subkey)
{
    void* handle = dlopen("libglib-2.0.so.0", RTLD_LAZY);

    (void)subkey; /* avoid warning due to unused parameter */

    if( NULL != handle )
    {
        gchar* (* g_strdup)(const gchar*) = (gchar* (*)(const gchar*)) dlsym(handle, "g_strdup");

        if( NULL != g_strdup)
            return g_strdup("/apps/gnome-settings/gnome-open-url");
    }

    return NULL;
}

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
    char *index;

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
    index = strstr(fallback, "gnome-open-url");
    if ( NULL != index )
    {
        char *args[3];
        strncpy(index, "open-url", 9);
        args[0] = fallback;
        args[1] = argv[1];
        args[2] = NULL;
        return execv(fallback, args);
    }

    return -1;
}



