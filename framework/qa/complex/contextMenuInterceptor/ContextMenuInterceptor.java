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

package complex.contextMenuInterceptor;

import com.sun.star.ui.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;

public class ContextMenuInterceptor implements XContextMenuInterceptor
{

    private final com.sun.star.awt.XBitmap myBitmap;

    public ContextMenuInterceptor(com.sun.star.awt.XBitmap aBitmap)
    {
        myBitmap = aBitmap;
    }

    public ContextMenuInterceptorAction notifyContextMenuExecute(
            com.sun.star.ui.ContextMenuExecuteEvent aEvent) throws RuntimeException
    {
        try
        {
            // Retrieve context menu container and query for service factory to
            // create sub menus, menu entries and separators
            com.sun.star.container.XIndexContainer xContextMenu = aEvent.ActionTriggerContainer;
            com.sun.star.lang.XMultiServiceFactory xMenuElementFactory =
                    UnoRuntime.queryInterface(com.sun.star.lang.XMultiServiceFactory.class, xContextMenu);

            if (xMenuElementFactory != null)
            {

                // create root menu entry for sub menu and sub menu
                com.sun.star.beans.XPropertySet xRootMenuEntry =
                        UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class, xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger"));

                // create a line separator for our new help sub menu
                com.sun.star.beans.XPropertySet xSeparator =
                        UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class, xMenuElementFactory.createInstance("com.sun.star.ui.ActionTriggerSeparator"));
                Short aSeparatorType = Short.valueOf(ActionTriggerSeparatorType.LINE);
                xSeparator.setPropertyValue("SeparatorType", aSeparatorType);

                // query sub menu for index container to get access
                com.sun.star.container.XIndexContainer xSubMenuContainer =
                        UnoRuntime.queryInterface(com.sun.star.container.XIndexContainer.class, xMenuElementFactory.createInstance("com.sun.star.ui.ActionTriggerContainer"));

                // intialize root menu entry "Help"
                xRootMenuEntry.setPropertyValue("Text", "Help");
                xRootMenuEntry.setPropertyValue("CommandURL", "slot:5410");
                xRootMenuEntry.setPropertyValue("HelpURL", "5410");
                xRootMenuEntry.setPropertyValue("SubContainer", xSubMenuContainer);
                xRootMenuEntry.setPropertyValue("Image", myBitmap);

                // create menu entries for the new sub menu
                // intialize help/content menu entry
                // entry "Content"
                XPropertySet xMenuEntry = UnoRuntime.queryInterface(XPropertySet.class, xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger"));
                xMenuEntry.setPropertyValue("Text", "Content");
                xMenuEntry.setPropertyValue("CommandURL", "slot:5401");
                xMenuEntry.setPropertyValue("HelpURL", "5401");

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex(0, xMenuEntry);

                // intialize help/help agent
                // entry "Help Agent"
                xMenuEntry = UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class, xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger"));
                xMenuEntry.setPropertyValue("Text", "Help Agent");
                xMenuEntry.setPropertyValue("CommandURL", "slot:5962");
                xMenuEntry.setPropertyValue("HelpURL", "5962");

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex(1, xMenuEntry);
                // intialize help/tips
                // entry "Tips"
                xMenuEntry = UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class, xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger"));
                xMenuEntry.setPropertyValue("Text", "Tips");
                xMenuEntry.setPropertyValue("CommandURL", "slot:5404");
                xMenuEntry.setPropertyValue("HelpURL", "5404");

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex(2, xMenuEntry);

                // add separator into the given context menu
                xContextMenu.insertByIndex(0, xSeparator);

                // add new sub menu into the given context menu
                xContextMenu.insertByIndex(0, xRootMenuEntry);

                // The controller should execute the modified context menu and stop notifying other
                // interceptors.
                return com.sun.star.ui.ContextMenuInterceptorAction.EXECUTE_MODIFIED;
            }
        }
        catch (com.sun.star.beans.UnknownPropertyException ex)
        {
            // do something useful
            // we used a unknown property
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException ex)
        {
            // do something useful
            // we used an invalid index for accessing a container
        }
        catch (com.sun.star.uno.Exception ex)
        {
            // something strange has happened!
        }
        catch (java.lang.Throwable ex)
        {
            // catch java exceptions do something useful
        }

        return com.sun.star.ui.ContextMenuInterceptorAction.IGNORED;
    }
}
