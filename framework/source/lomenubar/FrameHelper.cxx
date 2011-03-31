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

#include "FrameHelper.hxx"
#include "AwtKeyToDbusmenuString.hxx"
#include "MenuItemInfo.hxx"
#include "MenuItemStatusListener.hxx"

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MenuEvent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/XMenuExtended.hpp>
#include <com/sun/star/awt/XMenuListener.hpp>
#include <com/sun/star/awt/XPopupMenuExtended.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <rtl/process.h>

#include <gio/gio.h>
#include <libdbusmenu-glib/client.h>
#include <libdbusmenu-gtk/menuitem.h>

using rtl::OUString;
using rtl::OString;
using rtl::OUStringToOString;

using namespace ::com::sun::star;

using com::sun::star::awt::KeyEvent;
using com::sun::star::awt::MenuEvent;
using com::sun::star::awt::SystemDependentXWindow;
using com::sun::star::awt::XMenuListener;
using com::sun::star::awt::XMenuExtended;
using com::sun::star::awt::XMenuListener;
using com::sun::star::awt::MenuEvent;
using com::sun::star::awt::XPopupMenu;
using com::sun::star::awt::XPopupMenuExtended;
using com::sun::star::awt::XSystemDependentWindowPeer;
using com::sun::star::awt::XWindow2;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::PropertyValue;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::frame::XController;
using com::sun::star::frame::XComponentLoader;
using com::sun::star::frame::XDispatch;
using com::sun::star::frame::XDispatchProvider;
using com::sun::star::frame::XDispatchHelper;
using com::sun::star::frame::XModel;
using com::sun::star::frame::XModuleManager;
using com::sun::star::frame::XLayoutManager;
using com::sun::star::frame::XPopupMenuController;
using com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;
using com::sun::star::ui::XUIElement;
using com::sun::star::ui::XUIConfigurationManager;
using com::sun::star::ui::XUIConfigurationManagerSupplier;
using com::sun::star::ui::XAcceleratorConfiguration;
using com::sun::star::ui::XModuleUIConfigurationManagerSupplier;
using com::sun::star::util::URL;
using com::sun::star::util::XURLTransformer;

// ------------------------ Item callbacks ---------------------------
// Item activated. It distpatches the command associated to a given menu item.
void
item_activated (DbusmenuMenuitem *item, guint timestamp, gpointer user_data)
{
    FrameHelper *helper =  (FrameHelper*)user_data;
    OUString command = OUString::createFromAscii(dbusmenu_menuitem_property_get (item, "CommandURL"));
    helper->dispatchCommand (command);
}

// Rebuilds the submenu
gboolean
item_about_to_show (DbusmenuMenuitem *item, gpointer user_data)
{
    //Get the XMenu interface for the MenuBar UIElement
    FrameHelper *helper = (FrameHelper*)user_data;
    Reference < XFrame > xFrame  = helper->getFrame ();
    Reference< XPropertySet > frameProps (xFrame, UNO_QUERY);
    Reference < XLayoutManager > xLayoutManager(frameProps->getPropertyValue(OUString::createFromAscii("LayoutManager")),
                                                UNO_QUERY);
    Reference < XUIElement > menuBar(xLayoutManager->getElement (OUString::createFromAscii("private:resource/menubar/menubar")),
                                     UNO_QUERY);
    Reference < XPropertySet > menuPropSet (menuBar, UNO_QUERY);

    if (!menuPropSet.is ())
    {
        return FALSE;
    }

    Reference < XMenu > xMenu(menuPropSet->getPropertyValue(OUString::createFromAscii("XMenuBar")),
                              UNO_QUERY);
    if (!xMenu.is())
    {
        return FALSE;
    }

    //Find xMenu for the first level item
    Reference < XMenu > xSubMenu;
    Reference < XMenuExtended > xMenuEx (xMenu, UNO_QUERY);
    guint16 root_count = xMenu->getItemCount();
    for (guint16 i = 0; i<root_count ;i++)
    {

        guint16 id = xMenu->getItemId (i);
        if (id == 0)
            continue;

        OUString command = xMenuEx->getCommand (id);

        //We must find the element with the same command URL
        if (! OUString::createFromAscii (dbusmenu_menuitem_property_get (item, "CommandURL")).equals (command))
            continue;

        Reference <XPopupMenu> subPopup (xMenu->getPopupMenu (id), UNO_QUERY);
        xSubMenu = Reference <XMenu> (subPopup, UNO_QUERY);
        break;
    }

    //We only do this for toplevel items
    if (xSubMenu.is ())
    {
        helper->rebuildMenu (xSubMenu, item);
        return FALSE;
    }

    //If it is not a toplevel item we stop trying to rebuild
    return TRUE;
}

void
destroy_menuitem (gpointer data)
{
    g_object_unref (G_OBJECT (data));
}

void
destroy_menu_item_info (gpointer data)
{
    delete (MenuItemInfo*)data;
}

// ------------------------ FrameHelper Class -------------------------------
FrameHelper::FrameHelper(const Reference< XMultiServiceFactory >&  rServiceManager,
                         const Reference< XFrame >&        xFrame,
                         DbusmenuServer*                   server)
{
    xMSF = rServiceManager;
    this->xFrame = xFrame;
    this->server = server;

    //Get xUICommands database (to retrieve labels, see FrameJob::getLabelFromCommandURL ())
    Reference < XNameAccess > xNameAccess (xMSF->createInstance(OUString::createFromAscii("com.sun.star.frame.UICommandDescription")),
                                           UNO_QUERY);
    xMM = Reference < XModuleManager> (xMSF->createInstance(OUString::createFromAscii("com.sun.star.frame.ModuleManager")),
                                       UNO_QUERY);
    xNameAccess->getByName(xMM->identify(xFrame)) >>= xUICommands;

    xdp = Reference < XDispatchProvider > (xFrame, UNO_QUERY);
    xTrans = Reference < XURLTransformer > (xMSF->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY);

    xSL = (XStatusListener*)new MenuItemStatusListener (this);

    // This initializes the shortcut database
    getAcceleratorConfigurations (xFrame->getController()->getModel (), xMM);

    // This information is needed for the dynamic submenus
    xPCF = Reference < XMultiComponentFactory > (xMSF->createInstance(OUString::createFromAscii("com.sun.star.frame.PopupMenuControllerFactory")),
                                                 UNO_QUERY);


    // This is a hash table that maps Command URLs to MenuItemInfo classes
    //   to cache command information
    commandsInfo = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          destroy_menu_item_info);

    // These are the arguments needed for the XPopupMenuController
    args = Sequence < Any > (2);
    PropertyValue item;

    item.Name = OUString::createFromAscii("ModuleName");
    item.Value <<= xMM->identify (xFrame);
    args[0] <<= item;

    item.Name = OUString::createFromAscii("Frame");
    item.Value <<= xFrame;
    args[1] <<= item;

    root = NULL;
    watcher_set = FALSE;

    //This variable prevents the helper from being disconnected from the frame
    //for special cases of component dettaching like print preview
    blockDetach = FALSE;
}

void SAL_CALL
FrameHelper::disposing (const EventObject& aEvent) throw (RuntimeException)
{}

FrameHelper::~FrameHelper()
{
    if (server)
        g_object_unref (server);

    if (watcher_set)
        g_bus_unwatch_name (watcher);

    g_hash_table_destroy (commandsInfo);
}

void
FrameHelper::setRootItem (DbusmenuMenuitem *root)
{
    this->root = root;
}

void
FrameHelper::setRegistrarWatcher (guint watcher)
{
    watcher_set = TRUE;
    this->watcher = watcher;
}

void
FrameHelper::setServer (DbusmenuServer *server)
{
    this->server = server;
}

//Getters
Reference < XFrame >
FrameHelper::getFrame ()
{
    return xFrame;
}

XStatusListener*
FrameHelper::getStatusListener ()
{
    return xSL;
}

GHashTable*
FrameHelper::getCommandsInfo ()
{
    return commandsInfo;
}

unsigned long
FrameHelper::getXID ()
{
    Reference< XSystemDependentWindowPeer > xWin( xFrame->getContainerWindow(), UNO_QUERY );

    if (!xWin.is())
        return 0;

    sal_Int8 processID[16];
    rtl_getGlobalProcessId( (sal_uInt8*)processID );
    Sequence <signed char> pidSeq (processID, 16);

    SystemDependentXWindow xWindow;
    xWin->getWindowHandle (pidSeq, SYSTEM_XWINDOW) >>= xWindow;

    return xWindow.WindowHandle;
}

void SAL_CALL
FrameHelper::frameAction(const FrameActionEvent& action) throw (RuntimeException)
{
    //If theh component is detached from the frame, remove this action listener,
    //it is then disposed and destroyed by the frame. We deregister the window
    //from the AppMenu Registrar

    //This is a special case, .uno:printPreview detaches the component but we are
    //not actually switching to another document.
    if (blockDetach)
    {
        blockDetach = TRUE;
        return;
    }

    if (action.Action == frame::FrameAction_COMPONENT_DETACHING)
    {
        GError *error = NULL;


        xFrame->removeFrameActionListener (this);
        Reference< XPropertySet > frameProps (xFrame, UNO_QUERY);
        Reference < XLayoutManager > xLayoutManager(frameProps->getPropertyValue(OUString::createFromAscii("LayoutManager")),
                                                    UNO_QUERY);
        xLayoutManager->showElement (OUString::createFromAscii("private:resource/menubar/menubar"));

        unsigned long xid = getXID();

        GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
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
            g_error_free (error);
            return;
        }

        //TODO: Check if window is registered already
        g_dbus_proxy_call_sync (proxy,
                                "UnregisterWindow",
                                g_variant_new ("(u)", (guint32)xid),
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);

        if (error)
        {
            g_warning ("Couldn't call /com/canonical/AppMenu/Registrar.UnregisterWindow");
            g_error_free (error);
        }

        if (server)
        {
            g_object_unref (server);
            server = NULL;
            root = NULL;
        }

        if (watcher_set)
        {
            g_bus_unwatch_name (watcher);
            watcher_set = FALSE;
        }

        return;
    }
}

//This function rebuilds (or builds from scratch) a DbusmenuMenuitem structure
//from a given pair of XMenu/Dbusmenuitem.
void
FrameHelper::rebuildMenu (Reference < XMenu >  xMenu,
                          DbusmenuMenuitem    *parent)
{
    g_return_if_fail (parent != NULL);
    GList *items = dbusmenu_menuitem_get_children (parent);
    guint nitems = g_list_length (items);   //number of available Dbusmenuitems
    guint16 count = xMenu->getItemCount (); //number of real menu items

    // One item does not represent always the same command.
    // We do this for performance reasons, as it's really hard to match a command with
    // a single dbusmenuitem given the lack of information provided by the status listener
    if (count > nitems)
    {
        // Add enough Dbusmenuitems to replicate all
        for (guint16 i = 0; i < (count - nitems); i++)
        {
            DbusmenuMenuitem *item = dbusmenu_menuitem_new ();
            dbusmenu_menuitem_child_append (parent, item);
            g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(item_activated), this);
        }
        items = dbusmenu_menuitem_get_children (parent);
    }
    if (count < nitems)
    {
        // If there is an excess of Dbusmenuitems we make them invisible
        for (guint16 i = nitems - 1; i >= count; i--)
        {
            DbusmenuMenuitem *item = DBUSMENU_MENUITEM (g_list_nth_data(items, i));
            dbusmenu_menuitem_property_set_bool (item, DBUSMENU_MENUITEM_PROP_VISIBLE, FALSE);
        }
    }

    for (guint16 i = 0; i<count; i++)
    {
        Reference < XMenuExtended > xMenuEx (xMenu, UNO_QUERY);
        guint16 id = xMenu->getItemId (i);
        OUString oUCommand = xMenuEx->getCommand (id);
        OString command = OUStringToOString (oUCommand, RTL_TEXTENCODING_ASCII_US);

        DbusmenuMenuitem *item = DBUSMENU_MENUITEM(g_list_nth_data(items, i));

        if (!item)
            continue;

        if (!DBUSMENU_IS_MENUITEM (item))
            continue;

        // We drop the WindowList, doesn't work properly and it's useless anyhow
        if (oUCommand.equals (OUString::createFromAscii (".uno:WindowList")))
            continue;

        //We set the default properties (in case it was not visible or a separator)
        dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_DEFAULT);
        dbusmenu_menuitem_property_set_bool (item, DBUSMENU_MENUITEM_PROP_VISIBLE, TRUE);

        if (id == 0)
        {
            dbusmenu_menuitem_property_set (item, "CommandURL", "slot:0");
            dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
            //Getting rid of any possible children
            g_list_free_full (dbusmenu_menuitem_take_children (item), destroy_menuitem);
            continue;
        }

        //Setting the command
        dbusmenu_menuitem_property_set (item, "CommandURL", command.getStr());

        //Getting a shortcut
        KeyEvent kev = findShortcutForCommand (oUCommand);

        if (kev.KeyCode != 0)  //KeyCode must have a value
        {
            GVariantBuilder builder;
            const gchar* keystring = AwtKeyToDbusmenuString(kev.KeyCode);

            g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

            //We map KeyEvent.Modifiers with Dbusmenu modifiers strings
            if (awt::KeyModifier::SHIFT & kev.Modifiers)
                g_variant_builder_add(&builder, "s", DBUSMENU_MENUITEM_SHORTCUT_SHIFT);
            if (awt::KeyModifier::MOD2 & kev.Modifiers)
                g_variant_builder_add(&builder, "s", DBUSMENU_MENUITEM_SHORTCUT_ALT);
            if (awt::KeyModifier::MOD1 & kev.Modifiers || awt::KeyModifier::MOD3 & kev.Modifiers)
                g_variant_builder_add(&builder, "s", DBUSMENU_MENUITEM_SHORTCUT_CONTROL);

            g_variant_builder_add(&builder, "s", keystring);

            GVariant * inside = g_variant_builder_end(&builder);
            g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
            g_variant_builder_add_value(&builder, inside);

            GVariant * outsidevariant = g_variant_builder_end(&builder);
            dbusmenu_menuitem_property_set_variant(item, DBUSMENU_MENUITEM_PROP_SHORTCUT, outsidevariant);
        }

        // Lookup for a MenuItemInfo object for this menuitem, create one if it doesn't exist
        // this object caches the values that change on status updates.
        MenuItemInfo* commInfo = (MenuItemInfo*)g_hash_table_lookup (commandsInfo, (gconstpointer)command.getStr());
        if (!commInfo)
        {
            commInfo = new MenuItemInfo ();
            g_hash_table_insert (commandsInfo, g_strdup (command.getStr()), commInfo);

            OUString oULabel = getLabelFromCommandURL(oUCommand);
            if (oULabel.getLength() == 0)
            {
                oULabel = xMenu->getItemText (id);
            }

            //Replace tilde with underscore for Dbusmenu Alt accelerators
            oULabel = oULabel.replace ((sal_Unicode)0x007e, (sal_Unicode)0x005f);
            // GLib behaves better than OUStringToOString wrt encoding transformation
            gchar* label = g_utf16_to_utf8 (oULabel.getStr(),
                                            oULabel.getLength(),
                                            NULL, NULL, NULL);
            commInfo->setLabel (label);
            g_free (label);
        }

        //Update the check state directly from the data, this is more reliable
        Reference < XPopupMenu > popUp (xMenu, UNO_QUERY);
        if (popUp.is() && popUp->isItemChecked (id))
        {
            commInfo->setCheckState (DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED);
        }

        dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_LABEL, commInfo->getLabel ());
        dbusmenu_menuitem_property_set_bool (item, DBUSMENU_MENUITEM_PROP_ENABLED, commInfo->getEnabled ());

        //TODO: Find a selection of which commands are radio toggle type
        if (commInfo->getCheckState () != DBUSMENU_MENUITEM_TOGGLE_STATE_UNKNOWN)
        {
            dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,  commInfo->getCheckType ());
            dbusmenu_menuitem_property_set_int (item, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE, commInfo->getCheckState ());
        }

        // Adding status listener
        URL commandURL;
        commandURL.Complete = oUCommand;
        xTrans->parseStrict (commandURL);

        Reference < XDispatch > xDispatch  = xdp->queryDispatch (commandURL, OUString(), 0);
        if (xDispatch.is())
            xDispatch->addStatusListener (xSL, commandURL);

        Reference < XPopupMenu > subPopMenu (xMenu->getPopupMenu (id), UNO_QUERY);

        //Some menus do not provide the information available through the normal XMenu interface,
        //we need to access that info through a special XPopupMenuController
        if (isSpecialSubmenu (oUCommand))
        {
            Reference < XPropertySet > xMSFProps (xMSF, UNO_QUERY);
            Reference <XComponentContext> xContext (xMSFProps->getPropertyValue (OUString::createFromAscii ("DefaultContext")),
                                                    UNO_QUERY);

            Reference < XPopupMenuController > xRFC (xPCF->createInstanceWithArgumentsAndContext(oUCommand,
                                                                                                 args,
                                                                                                 xContext),
                                                     UNO_QUERY);

            Reference < XPopupMenu > xPO (xMSF->createInstance(OUString::createFromAscii ("stardiv.Toolkit.VCLXPopupMenu")),
                                          UNO_QUERY);

            if (xRFC.is () && xPO.is ())
            {
                xRFC->setPopupMenu (xPO);
                xRFC->updatePopupMenu ();
                Reference < XMenu > subMenu (xPO, UNO_QUERY);
                rebuildMenu (subMenu, item);
            }
            else if (subPopMenu.is ())
            {
                Reference <XMenu> subMenu (subPopMenu, UNO_QUERY);
                rebuildMenu (subMenu, item);
            }
        }

        // Introspect submenus
        else if (subPopMenu.is ())
        {
            Reference <XMenu> subMenu (subPopMenu, UNO_QUERY);
            g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ABOUT_TO_SHOW, G_CALLBACK(item_about_to_show), this);
            rebuildMenu (subMenu, item);
        }
        else
        {
            //Getting rid of any possible children
            g_list_free_full (dbusmenu_menuitem_take_children (item), destroy_menuitem);
        }
    }

    return;
}

//Gets the menu item Label given a CommandURL
//This is a work around for bug: https://bugs.freedesktop.org/show_bug.cgi?id=34127
OUString
FrameHelper::getLabelFromCommandURL (OUString commandURL)
{
    OUString label;

    Sequence < PropertyValue > commandProps;

    if (commandURL.getLength () < 1)
        return label;

    if (!xUICommands.is())
        return label;

    try
    {
        xUICommands->getByName (commandURL) >>= commandProps;
    }
    catch (com::sun::star::container::NoSuchElementException e)
    {
        return label;
    }

    for (sal_Int32 i = 0; i < commandProps.getLength(); i++)
    {
        if ( commandProps[i].Name.equalsAsciiL (RTL_CONSTASCII_STRINGPARAM ("Label")))
        {
            commandProps[i].Value >>= label;
            label = label.replace ((sal_Unicode)0x007e, (sal_Unicode)0x005f);
            //break;
        }
    }

    return label;
}

//This method is a facility to bootstrap the Dbusmenuitem strcuture from the menubar
void
FrameHelper::rebuildMenuFromRoot ()
{
    Reference < XFrame >         xFrame = getFrame ();
    Reference < XPropertySet >   frameProps     (xFrame, UNO_QUERY);
    Reference < XLayoutManager > xLayoutManager (frameProps->getPropertyValue(OUString::createFromAscii("LayoutManager")),
                                                 UNO_QUERY);
    Reference < XUIElement >   menuBar     (xLayoutManager->getElement (OUString::createFromAscii("private:resource/menubar/menubar")),
                                            UNO_QUERY);
    Reference < XPropertySet > menuPropSet (menuBar, UNO_QUERY);

    if (!menuPropSet.is ())
        return;

    Reference < XMenu > xMenu (menuPropSet->getPropertyValue(OUString::createFromAscii("XMenuBar")),
                               UNO_QUERY);
    if (!xMenu.is ())
        return;

    rebuildMenu (xMenu, root);
}

//Some menus are special, this is the list of them
gboolean
FrameHelper::isSpecialSubmenu (OUString command)
{
    const gchar * specialSubmenus[11] = {".uno:CharFontName",
                                         ".uno:FontHeight",
                                         ".uno:ObjectMenue",
                                         ".uno:InsertPageHeader",
                                         ".uno:InsertPageFooter",
                                         ".uno:ChangeControlType",
                                         ".uno:AvailableToolbars",
                                         ".uno:ScriptOrganizer",
                                         ".uno:RecentFileList",
                                         ".uno:AddDirect",
                                         ".uno:AutoPilotMenu"};

    for (gint i = 0; i < 11; i++)
    {
        if (command.equals (OUString::createFromAscii (specialSubmenus[i])))
            return TRUE;
    }
    return FALSE;
}

void
FrameHelper::dispatchCommand (OUString command)
{
    OUString target = OUString::createFromAscii ("");
    Reference < XDispatchHelper > xdh (xMSF->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.DispatchHelper"))),
                                       UNO_QUERY);

    //g_debug ("%s", OUStringToOString (command, RTL_TEXTENCODING_ASCII_US).getStr());

    // This is a special case, we don't want the helper to be disconnected from the frame
    // when PrintPreview dettaches. See the frameAction method.
    if (command.equals (OUString::createFromAscii (".uno:PrintPreview")))
    {
        blockDetach = TRUE;
    }

    // This is a special case for the recentfilelist
    if (command.matchAsciiL ("vnd.sun.star.popup:RecentFileList", 33, 0))
    {
        target = OUString::createFromAscii ("_default");

        Reference < XPropertySet > xMSFProps (xMSF, UNO_QUERY);
        Reference <XComponentContext> xContext (xMSFProps->getPropertyValue (OUString::createFromAscii ("DefaultContext")),
                                                UNO_QUERY);
        Reference < XPopupMenuController > xRFC (xPCF->createInstanceWithArgumentsAndContext(OUString::createFromAscii (".uno:RecentFileList"),
                                                                                             args,
                                                                                             xContext),
                                                 UNO_QUERY);
        Reference < XMenuListener > xML (xRFC, UNO_QUERY);

        Reference < XPopupMenu > xPO (xMSF->createInstance(OUString::createFromAscii ("stardiv.Toolkit.VCLXPopupMenu")),
                                      UNO_QUERY);

        if (xRFC.is () && xPO.is ())
        {
            xRFC->setPopupMenu (xPO);
            xRFC->updatePopupMenu ();
            Reference < XMenu > subMenu (xPO, UNO_QUERY);
            Reference < XMenuExtended > subMenuEx (xPO, UNO_QUERY);

            //We need to find the item idd
            for (int i = 0; i < subMenu->getItemCount (); i++)
            {
                int id = subMenu->getItemId (i);

                if (subMenuEx->getCommand (id).equals (command))
                {
                    MenuEvent mev;
                    mev.MenuId = id;

                    xML->select (mev);
                }
            }
        }

        return;
    }

    if (command.matchAsciiL ("private:factory/", 16, 0))
        target = OUString::createFromAscii ("_blank");

    xdh->executeDispatch (Reference < XDispatchProvider > (xFrame, UNO_QUERY),
                          command,
                          target,
                          0,
                          Sequence < PropertyValue > ());
}

//Set all the accelerator configuration sources
void
FrameHelper::getAcceleratorConfigurations (Reference < XModel >        xModel,
                                           Reference < XModuleManager> xModuleManager)
{
    //Get document shortcut database
    Reference< XUIConfigurationManagerSupplier > docUISupplier(xModel, UNO_QUERY);
    Reference< XUIConfigurationManager >         docUIManager = docUISupplier->getUIConfigurationManager();
    Reference< XAcceleratorConfiguration >       docAccelConf(docUIManager->getShortCutManager(), UNO_QUERY);
    this->docAccelConf = docAccelConf;

    //Get module shurtcut database
    Reference< XModuleUIConfigurationManagerSupplier > modUISupplier(xMSF->createInstance(OUString::createFromAscii("com.sun.star.ui.ModuleUIConfigurationManagerSupplier")),
                                                                     UNO_QUERY);
    Reference< XUIConfigurationManager >   modUIManager = modUISupplier->getUIConfigurationManager(xModuleManager->identify(xFrame));
    Reference< XAcceleratorConfiguration > modAccelConf(modUIManager->getShortCutManager(), UNO_QUERY);
    this->modAccelConf = modAccelConf;

    //Get global shortcut database
    Reference< XAcceleratorConfiguration > globAccelConf(xMSF->createInstance(OUString::createFromAscii("com.sun.star.ui.GlobalAcceleratorConfiguration")),
                                                         UNO_QUERY);
    this->globAccelConf = globAccelConf;
}


//This function finds a KeyEvent with the shortcut information for each command
KeyEvent
FrameHelper::findShortcutForCommand (OUString command)

{
    KeyEvent kev;

    Sequence < OUString > commands (1);
    commands[0] = command;

    try
    {
        Sequence < Any > evs = docAccelConf->getPreferredKeyEventsForCommandList (commands);

        for (int j = 0; j < evs.getLength (); j++)
        {
            KeyEvent ev;
            if (evs[j] >>= ev)
                return ev;
        }
    }
    catch (...)
    {}
    try
    {
        Sequence < Any > evs = modAccelConf->getPreferredKeyEventsForCommandList (commands);

        for (int j = 0; j < evs.getLength (); j++)
        {
            KeyEvent ev;
            if (evs[j] >>= ev)
                return ev;
        }
    }
    catch (...)
    {}
    try
    {
        Sequence < Any > evs = globAccelConf->getPreferredKeyEventsForCommandList (commands);

        for (int j = 0; j < evs.getLength (); j++)
        {
            KeyEvent ev;
            if (evs[j] >>= ev)
                return ev;
        }
    }
    catch (...)
    {}

    //NOTE: For some reason this item does not return its shortcut. Setting manually:
    if (command.equals (OUString::createFromAscii (".uno:HelpIndex")))
    {
        kev.KeyCode = awt::Key::F1;
    }

    return kev;
}
