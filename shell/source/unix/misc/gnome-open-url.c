/*************************************************************************
 *
 *  $RCSfile: gnome-open-url.c,v $
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

/*
 * HACK: avoid error messages caused by not setting a GNOME program name
 */

gchar* gnome_gconf_get_gnome_libs_settings_relative (const gchar *subkey)
{
    void* handle = dlopen("libglib-2.0.so.0", RTLD_LAZY);

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
    void* handle = dlopen("libgnome-2.so.0", RTLD_LAZY);

    if( NULL != handle)
    {
        gboolean (* func) (const char *url, GError **error) =
            (gboolean (*) (const char *, GError **)) dlsym(handle, "gnome_url_show");

        if( NULL != func )
            return func(url, error);
    }

    return 0;
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
        char *args[3] = { NULL, argv[1], NULL };
        strncpy(index, "open-url", 9);
        args[0] = fallback;
        return execv(fallback, args);
    }

    return -1;
}



