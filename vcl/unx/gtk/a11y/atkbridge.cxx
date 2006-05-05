/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkbridge.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-05 10:53:26 $
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

#include <plugins/gtk/atkbridge.hxx>
#include <plugins/gtk/gtkframe.hxx>

#include "atkfactory.hxx"
#include "atkutil.hxx"
#include "atkwindow.hxx"

#include <stdio.h>

#if ! ( defined AIX || defined HPUX ) // these have no dl* functions
#include <dlfcn.h>
#endif

void InitAtkBridge(void)
{
    unsigned int major, minor, micro;

    /* check gail minimum version requirements */
    if( sscanf( atk_get_toolkit_version(), "%u.%u.%u", &major, &minor, &micro) < 3 )
    {
        g_warning( "unable to parse gail version number" );
        return;
    }

    if( ( (major << 16) | (minor << 8) | micro ) < ( (1 << 16) | 8 << 8 | 6 ) )
    {
        g_warning( "libgail >= 1.8.6 required for accessibility support" );
        return;
    }

    /* get at-spi version by checking the libspi.so version number  */
#if ! ( defined AIX || defined HPUX ) // these have no dl* functions

    /* libspi should be mapped by loading libatk-bridge.so already */
    void * sym = dlsym( RTLD_DEFAULT, "spi_accessible_new" );
    g_return_if_fail( sym != NULL );

    Dl_info dl_info;
    int ret = dladdr( sym, &dl_info );
    g_return_if_fail( ret != 0 );

    char path[PATH_MAX];
    if( NULL == realpath(dl_info.dli_fname, path) )
    {
        perror( "unable to resolve libspi.so.0" );
        return;
    }

    const char * cp = strrchr(path, '/');
    if( cp != NULL )
        ++cp;
    else
        cp = dl_info.dli_fname;

    if( sscanf( cp, "libspi.so.%u.%u.%u", &major, &minor, &micro) < 3 )
    {
        g_warning( "unable to parse at-spi version number: %s", cp );
        return;
    }

    if( ( (major << 16) | (minor << 8) | micro ) < ( 10 << 8 | 6 ) )
    {
        g_warning( "at-spi >= 1.7 required for accessibility support" );
        return;
    }

#endif // ! ( defined AIX || defined HPUX )

    /* Initialize the AtkUtilityWrapper class */
    g_type_class_unref( g_type_class_ref( OOO_TYPE_ATK_UTIL ) );

    /* Initialize the GailWindow wrapper class */
    g_type_class_unref( g_type_class_ref( OOO_TYPE_WINDOW_WRAPPER ) );

    /* Register AtkObject wrapper factory */
    AtkRegistry * registry = atk_get_default_registry();
    if( registry)
        atk_registry_set_factory_type( registry, OOO_TYPE_FIXED, OOO_TYPE_WRAPPER_FACTORY );
}

