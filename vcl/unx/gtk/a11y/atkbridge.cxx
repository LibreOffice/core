/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkbridge.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-25 11:25:03 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <plugins/gtk/atkbridge.hxx>
#include <plugins/gtk/gtkframe.hxx>

#include "atkfactory.hxx"
#include "atkutil.hxx"
#include "atkwindow.hxx"

#include <stdio.h>

#if ! ( defined AIX || defined HPUX ) // these have no dl* functions
#include <dlfcn.h>
#endif

bool InitAtkBridge(void)
{
    const char* pVersion = atk_get_toolkit_version();
    if( ! pVersion )
    {
        g_warning( "unable to get gail version number" );
        return false;
    }

    unsigned int major, minor, micro;

    /* check gail minimum version requirements */
    if( sscanf( pVersion, "%u.%u.%u", &major, &minor, &micro) < 3 )
    {
        g_warning( "unable to parse gail version number" );
        return false;
    }

    if( ( (major << 16) | (minor << 8) | micro ) < ( (1 << 16) | 8 << 8 | 6 ) )
    {
        g_warning( "libgail >= 1.8.6 required for accessibility support" );
        return false;
    }

    /* Initialize the AtkUtilityWrapper class */
    g_type_class_unref( g_type_class_ref( OOO_TYPE_ATK_UTIL ) );

    /* Initialize the GailWindow wrapper class */
    g_type_class_unref( g_type_class_ref( OOO_TYPE_WINDOW_WRAPPER ) );

    /* Register AtkObject wrapper factory */
    AtkRegistry * registry = atk_get_default_registry();
    if( registry )
        atk_registry_set_factory_type( registry, OOO_TYPE_FIXED, OOO_TYPE_WRAPPER_FACTORY );

    return true;
}

void DeInitAtkBridge()
{
    restore_gail_window_vtable();

    /* shutdown atk-bridge Gtk+ module here, otherwise it is done in an atexit handler
     * where the VCL Gtk+ plugin might already be unloaded
     */

#if ! ( defined AIX || defined HPUX ) // these have no dl* functions
#if defined __SUNPRO_CC // disable warning: Cannot cast from void* to void(*)()
#pragma disable_warn
#endif
    void (* shutdown) ( void ) =  (void (*) (void)) dlsym( RTLD_DEFAULT, "gnome_accessibility_module_shutdown");
#if defined __SUNPRO_CC
#pragma enable_warn
#endif

    if( shutdown )
        shutdown();
#endif // ! ( defined AIX || defined HPUX )
}

