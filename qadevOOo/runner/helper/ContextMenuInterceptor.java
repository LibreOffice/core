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


package helper;

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.ActionTriggerSeparatorType;
import com.sun.star.ui.ContextMenuInterceptorAction;
import com.sun.star.ui.XContextMenuInterceptor;
import com.sun.star.uno.UnoRuntime;

public class ContextMenuInterceptor implements XContextMenuInterceptor {

    public ContextMenuInterceptorAction notifyContextMenuExecute(
            com.sun.star.ui.ContextMenuExecuteEvent aEvent ) throws RuntimeException {
        try {
            // Retrieve context menu container and query for service factory to
            // create sub menus, menu entries and separators
            XIndexContainer xContextMenu = aEvent.ActionTriggerContainer;
            XMultiServiceFactory xMenuElementFactory =
                    UnoRuntime.queryInterface(
            XMultiServiceFactory.class, xContextMenu );

            if ( xMenuElementFactory != null ) {

                // create root menu entry for sub menu and sub menu
                XPropertySet xRootMenuEntry =
                        UnoRuntime.queryInterface(
                XPropertySet.class,
                xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger" ));

                // create a line separator for our new help sub menu
                XPropertySet xSeparator =
                        UnoRuntime.queryInterface(
                XPropertySet.class,
                xMenuElementFactory.createInstance("com.sun.star.ui.ActionTriggerSeparator" ) );
                Short aSeparatorType = Short.valueOf( ActionTriggerSeparatorType.LINE );
                xSeparator.setPropertyValue( "SeparatorType", aSeparatorType );

                // query sub menu for index container to get access
                XIndexContainer xSubMenuContainer =
                        UnoRuntime.queryInterface(
                XIndexContainer.class,
                xMenuElementFactory.createInstance("com.sun.star.ui.ActionTriggerContainer" ));

                // initialize root menu entry "Help"
                xRootMenuEntry.setPropertyValue( "Text", "Help" );
                xRootMenuEntry.setPropertyValue( "CommandURL", "slot:5410" );
                xRootMenuEntry.setPropertyValue( "HelpURL", "5410" );
                xRootMenuEntry.setPropertyValue( "SubContainer", xSubMenuContainer );

                // create menu entries for the new sub menu
                // initialize help/content menu entry
                // entry "Content"
                XPropertySet xMenuEntry = UnoRuntime.queryInterface(
                        XPropertySet.class, xMenuElementFactory.createInstance(
                        "com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", "Content" );
                xMenuEntry.setPropertyValue( "CommandURL", "slot:5401" );
                xMenuEntry.setPropertyValue( "HelpURL", "5401" );

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 0, xMenuEntry );

                // initialize help/help on help
                // entry "Help on Help"
                xMenuEntry = UnoRuntime.queryInterface(
                        XPropertySet.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue("Text", "Help on Help");
                xMenuEntry.setPropertyValue("CommandURL", "slot:5400");
                xMenuEntry.setPropertyValue("HelpURL", "5400");

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 1, xMenuEntry );
                // initialize help/tips
                // entry "Tips"
                xMenuEntry = UnoRuntime.queryInterface(
                        XPropertySet.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", "Tips" );
                xMenuEntry.setPropertyValue( "CommandURL", "slot:5404" );
                xMenuEntry.setPropertyValue( "HelpURL", "5404" );

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 2, xMenuEntry );

                // add separator into the given context menu
                xContextMenu.insertByIndex( 1, xSeparator );

                // add new sub menu into the given context menu
                xContextMenu.insertByIndex( 1, xRootMenuEntry );

                // The controller should execute the modified context menu and stop notifying other
                // interceptors.
                return ContextMenuInterceptorAction.EXECUTE_MODIFIED ;
            }
        } catch ( UnknownPropertyException ex ) {
            // do something useful
            // we used an unknown property
        } catch ( IndexOutOfBoundsException ex ) {
            // do something useful
            // we used an invalid index for accessing a container
        } catch ( Exception ex ) {
            // something strange has happened!
        } catch ( Throwable ex ) {
            // catch java exceptions and do something useful
        }

        return ContextMenuInterceptorAction.IGNORED;
    }
}
