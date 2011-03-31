/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * A LibreOffice extension to send the menubar structure through DBusMenu
 *
 * Copyright 2011 Canonical, Ltd.
 * Authors:
 *     Alberto Ruiz <alberto.ruiz@codethink.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the the GNU Lesser General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR PURPOSE.  See the applicable
 * version of the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with this program. If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "DesktopJob.h"
#include "FrameJob.h"

#include <gio/gio.h>
#include <libdbusmenu-glib/server.h>

#include <rtl/process.h>
#include <osl/diagnose.h>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

using rtl::OUString;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::NamedValue;
using com::sun::star::document::XEventListener;
using com::sun::star::document::XEventBroadcaster;
using com::sun::star::frame::XFrame;
using com::sun::star::frame::XFramesSupplier;
using com::sun::star::frame::XFrameActionListener;
using com::sun::star::frame::XModel;
using com::sun::star::frame::XLayoutManager;
using com::sun::star::frame::FrameActionEvent;
using com::sun::star::frame::XFrameActionListener;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::EventObject;
using com::sun::star::uno::Any;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::XInterface;

//-------------------------- D-Bus Callbacks ----------------------------------
static void
on_bus (GDBusConnection * connection,
        const gchar * name,
        gpointer user_data)
{
    //TODO: Should we actually do something here?
    return;
}

static void
name_lost (GDBusConnection * connection, const gchar * name, gpointer user_data)
{
    g_error ("Unable to get name '%s' on DBus", name);
    return;
}

// --------------------------- DesktopJob ----------------------------------
Any SAL_CALL DesktopJob::execute( const Sequence< NamedValue >& aArguments )
    throw ( IllegalArgumentException, Exception, RuntimeException )
{
    g_type_init ();

    g_bus_own_name(G_BUS_TYPE_SESSION,
               LIBREOFFICE_BUSNAME,
               G_BUS_NAME_OWNER_FLAGS_NONE,
               on_bus,
               NULL,
               name_lost,
               NULL,
               NULL);

    return Any ();
}

OUString
DesktopJob_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( DESKTOPJOB_IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL
DesktopJob_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( DESKTOPJOB_SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL
DesktopJob_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( DESKTOPJOB_SERVICE_NAME ) );
    return aRet;
}

Reference< XInterface > SAL_CALL
DesktopJob_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new DesktopJob(rSMgr);
}

// XServiceInfo
OUString SAL_CALL
DesktopJob::getImplementationName()
    throw (RuntimeException)
{
    return DesktopJob_getImplementationName();
}

sal_Bool SAL_CALL
DesktopJob::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return DesktopJob_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL
DesktopJob::getSupportedServiceNames()
    throw (RuntimeException)
{
    return DesktopJob_getSupportedServiceNames();
}
