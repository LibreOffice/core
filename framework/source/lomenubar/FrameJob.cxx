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

#include "FrameJob.hxx"
#include "DesktopJob.hxx"
#include "FrameHelper.hxx"

#define OBJ_PATH_PREFIX "/com/canonical/menu/"

#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <com/sun/star/awt/XMenu.hpp>
#include <com/sun/star/awt/XMenuExtended.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/XPopupMenuExtended.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <iostream>
#include <fstream>

#include <gio/gio.h>
//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/client.h>
#pragma GCC diagnostic error "-Wignored-qualifiers"
//#pragma GCC diagnostic pop

#include <rtl/process.h>
#include <osl/diagnose.h>

using rtl::OUString;
using rtl::OString;
using rtl::OUStringToOString;

using com::sun::star::awt::KeyEvent;
using com::sun::star::awt::SystemDependentXWindow;
using com::sun::star::awt::XMenu;
using com::sun::star::awt::XMenuExtended;
using com::sun::star::awt::XPopupMenu;
using com::sun::star::awt::XPopupMenuExtended;
using com::sun::star::awt::XMenuBar;
using com::sun::star::awt::XSystemDependentWindowPeer;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::WeakReference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW;
using com::sun::star::lang::EventObject;
using com::sun::star::beans::NamedValue;
using com::sun::star::beans::PropertyValue;
using com::sun::star::beans::XPropertySet;
using com::sun::star::document::XEventBroadcaster;
using com::sun::star::frame::XFrame;
using com::sun::star::frame::XFrameActionListener;
using com::sun::star::frame::FrameActionEvent;
using com::sun::star::frame::XController;
using com::sun::star::frame::XLayoutManager;
using com::sun::star::frame::XModel;
using com::sun::star::frame::XModuleManager;
using com::sun::star::frame::XDispatch;
using com::sun::star::frame::XDispatchProvider;
using com::sun::star::frame::XDispatchHelper;
using com::sun::star::frame::XStatusListener;
using com::sun::star::frame::FeatureStateEvent;
using com::sun::star::ui::XUIElement;
using com::sun::star::ui::XUIElementSettings;
using com::sun::star::ui::XUIConfigurationManagerSupplier;
using com::sun::star::ui::XUIConfigurationManager;
using com::sun::star::ui::XModuleUIConfigurationManagerSupplier;
using com::sun::star::ui::XAcceleratorConfiguration;
using com::sun::star::util::URL;
using com::sun::star::util::XURLTransformer;
using com::sun::star::container::XIndexContainer;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::NoSuchElementException;


// This is a helper utility to transform an xid to a /com/canonical/menu/<XID>
// DBUS object path
OString
xid_to_object_path (unsigned long xid)
{

    GString *xid_str = g_string_new ("");
    g_string_printf (xid_str, "%d", (guint32)xid);
    OString object_path = OUStringToOString (OUString::createFromAscii (OBJ_PATH_PREFIX).concat (OUString::createFromAscii(xid_str->str)),
                                             RTL_TEXTENCODING_ASCII_US);
    g_string_free (xid_str, TRUE);
    return object_path;
}

//-------------------------- GObject callbacks -------------------------------//
//This is called when a registrar becomes available. It registers the hides the menubar.
static void
on_registrar_available (GDBusConnection * /*connection*/,
                        const gchar     * /*name*/,
                        const gchar     * /*name_owner*/,
                        gpointer         user_data)
{
    GError     *error = NULL;
    GDBusProxy *proxy;

    FrameHelper *helper = (FrameHelper*)user_data;
    unsigned long xid = helper->getXID();

    proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                           G_DBUS_PROXY_FLAGS_NONE,
                                           NULL,
                                           "com.canonical.AppMenu.Registrar",
                                           "/com/canonical/AppMenu/Registrar",
                                           "com.canonical.AppMenu.Registrar",
                                           NULL,
                                           &error);
    if (error)
    {
        g_warning ("Couldn't get /com/canonical/AppMenu/Registrar proxy");
        return;
    }


    //TODO: Check if window is registered already
    g_dbus_proxy_call_sync (proxy,
                            "RegisterWindow",
                            g_variant_new ("(uo)",
                                           (guint32)xid,
                                           xid_to_object_path (xid).getStr()),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);

    if (error)
    {
        g_warning ("Couldn't call /com/canonical/AppMenu/Registrar.RegisterWindow");
        return;
    }

    //Hide menubar
    Reference < XFrame > xFrame  = helper->getFrame ();
    Reference< XPropertySet > frameProps (xFrame, UNO_QUERY);
    Reference < XLayoutManager > xLayoutManager(frameProps->getPropertyValue(OUString::createFromAscii("LayoutManager")),
                                                UNO_QUERY);
    xLayoutManager->hideElement (OUString::createFromAscii("private:resource/menubar/menubar"));

    return;
}

//This is called when the registrar becomes unavailable. It shows the menubar.
static void
on_registrar_unavailable (GDBusConnection * /*connection*/,
                          const gchar     * /*name*/,
                          gpointer         user_data)
{
    //TODO: Unregister window?

    // Show menubar
    FrameHelper *helper = (FrameHelper*)user_data;
    Reference < XFrame > xFrame  = helper->getFrame ();
    Reference< XPropertySet > frameProps (xFrame, UNO_QUERY);
    Reference < XLayoutManager > xLayoutManager(frameProps->getPropertyValue(OUString::createFromAscii("LayoutManager")),
                                                UNO_QUERY);
    xLayoutManager->showElement (OUString::createFromAscii("private:resource/menubar/menubar"));
    return;
}
// ------------------------------- FrameJob --------------------------------------------
Any SAL_CALL FrameJob::execute( const Sequence< NamedValue >& aArguments )
    throw ( IllegalArgumentException, Exception, RuntimeException )
{
    Sequence< NamedValue > lEnv;
    Reference< XModel >    xModel;
    sal_Int32              len    = aArguments.getLength();

    for (int i = 0; i<len; i++)
    {
        if (aArguments[i].Name.equalsAscii("Environment"))
        {
            aArguments[i].Value >>= lEnv;
            break;
        }
    }

    len = lEnv.getLength ();
    for (int i = 0; i<len; i++)
    {
        if (lEnv[i].Name.equalsAscii("Model"))
        {
            lEnv[i].Value >>= xModel;
        }
    }

    //If we didn't get the model we have to quit
    if (!xModel.is())
        return Any();


    Reference< XController > xController( xModel->getCurrentController(), UNO_QUERY);
    if (!xController.is())
        return Any();

    m_xFrame = Reference< XFrame > ( xController->getFrame(), UNO_QUERY);
    if (!m_xFrame.is ())
        return Any();

    exportMenus (m_xFrame);
    return Any();
}

// This function crates a DbusmenuServer and starts the watcher for the AppMenu Registrar bus name
void
FrameJob::exportMenus (Reference < XFrame > xFrame)
{
    //Set the xFrame for this object

    this->m_xFrame = xFrame;

    //Create dbusmenu server object path string
    DbusmenuServer *server = dbusmenu_server_new (xid_to_object_path(getXID (xFrame)).getStr());


    Reference< XPropertySet > frameProps (xFrame, UNO_QUERY);
    Reference < XLayoutManager > xLayoutManager(frameProps->getPropertyValue(OUString::createFromAscii("LayoutManager")),
                                                UNO_QUERY);
    if (!xLayoutManager.is())
    {
        g_object_unref (server);
        return;
    }

    Reference < XUIElement > menuBar(xLayoutManager->getElement (OUString::createFromAscii("private:resource/menubar/menubar")),
                                     UNO_QUERY);
    Reference < XPropertySet > menuPropSet (menuBar, UNO_QUERY);

    if (!menuPropSet.is())
    {
        g_object_unref (server);
        return;
    }

    Reference < XMenu > xMenu (menuPropSet->getPropertyValue(OUString::createFromAscii("XMenuBar")),
                               UNO_QUERY);

    if (!xMenu.is ())
    {
        g_object_unref (server);
        return;
    }

    //Create a new frame helper to close the server when needed
    FrameHelper *helper = new FrameHelper (m_xMSF, xFrame, server);
    xFrame->addFrameActionListener (Reference < XFrameActionListener > (helper));

    //Populate dbusmenu items and start the server
    DbusmenuMenuitem *root = getRootMenuitem (xMenu, (gpointer)helper);
    dbusmenu_server_set_root (server, root);

    //Listen to the availability of the registrar
    guint watcher = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                      "com.canonical.AppMenu.Registrar",
                                      G_BUS_NAME_WATCHER_FLAGS_NONE,
                                      on_registrar_available,
                                      on_registrar_unavailable,
                                      helper,
                                      NULL);
    helper->setRegistrarWatcher (watcher);
}


//Gets the XID for a given XFrame
unsigned long
FrameJob::getXID (css::uno::Reference < css::frame::XFrame > xFrame)
{
    Reference< XSystemDependentWindowPeer > xWin( xFrame->getContainerWindow(), UNO_QUERY);

    if (!xWin.is())
        return 0;

    sal_Int8 processID[16];
    rtl_getGlobalProcessId( (sal_uInt8*)processID );
    Sequence <signed char> pidSeq (processID, 16);

    SystemDependentXWindow xWindow;
    xWin->getWindowHandle (pidSeq, SYSTEM_XWINDOW) >>= xWindow;

    return xWindow.WindowHandle;
}

// Builds a Dbusmenuitem structure from an XMenu object
DbusmenuMenuitem*
FrameJob::getRootMenuitem (Reference < XMenu > xMenu, gpointer helper)
{

    DbusmenuMenuitem *root = dbusmenu_menuitem_new_with_id (0);
    ((FrameHelper*)helper)->setRootItem(root);
    ((FrameHelper*)helper)->rebuildMenu (xMenu, root);

    return root;
}

// XJob
OUString FrameJob_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( FRAMEJOB_IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL FrameJob_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( FRAMEJOB_SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL FrameJob_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( FRAMEJOB_SERVICE_NAME ) );
    return aRet;
}

Reference< XInterface > SAL_CALL FrameJob_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new FrameJob(rSMgr);
}

// XServiceInfo
OUString SAL_CALL FrameJob::getImplementationName()
    throw (RuntimeException)
{
    return FrameJob_getImplementationName();
}

sal_Bool SAL_CALL FrameJob::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return FrameJob_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL FrameJob::getSupportedServiceNames()
    throw (RuntimeException)
{
    return FrameJob_getSupportedServiceNames();
}

