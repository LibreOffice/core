/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextMenuInterceptor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-05-17 13:30:44 $
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
                    (XMultiServiceFactory)UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, xContextMenu );

            if ( xMenuElementFactory != null ) {

                // create root menu entry for sub menu and sub menu
                XPropertySet xRootMenuEntry =
                        (XPropertySet)UnoRuntime.queryInterface(
                        XPropertySet.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger" ));

                // create a line separator for our new help sub menu
                XPropertySet xSeparator =
                        (XPropertySet)UnoRuntime.queryInterface(
                        XPropertySet.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTriggerSeparator" ) );
                Short aSeparatorType = new Short( ActionTriggerSeparatorType.LINE );
                xSeparator.setPropertyValue( "SeparatorType", (Object)aSeparatorType );

                // query sub menu for index container to get access
                XIndexContainer xSubMenuContainer =
                        (XIndexContainer)UnoRuntime.queryInterface(
                        XIndexContainer.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTriggerContainer" ));

                // intialize root menu entry "Help"
                xRootMenuEntry.setPropertyValue( "Text", new String( "Help" ));
                xRootMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5410" ));
                xRootMenuEntry.setPropertyValue( "HelpURL", new String( "5410" ));
                xRootMenuEntry.setPropertyValue( "SubContainer", (Object)xSubMenuContainer );

                // create menu entries for the new sub menu
                // intialize help/content menu entry
                // entry "Content"
                XPropertySet xMenuEntry = (XPropertySet)UnoRuntime.queryInterface(
                        XPropertySet.class, xMenuElementFactory.createInstance(
                        "com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Content" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5401" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5401" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 0, (Object)xMenuEntry );

                // intialize help/help agent
                // entry "Help Agent"
                xMenuEntry = (XPropertySet)UnoRuntime.queryInterface(
                        XPropertySet.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Help Agent" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5962" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5962" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 1, (Object)xMenuEntry );
                // intialize help/tips
                // entry "Tips"
                xMenuEntry = (XPropertySet)UnoRuntime.queryInterface(
                        XPropertySet.class,
                        xMenuElementFactory.createInstance("com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Tips" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5404" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5404" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 2, (Object)xMenuEntry );

                // add separator into the given context menu
                xContextMenu.insertByIndex( 1, (Object)xSeparator );

                // add new sub menu into the given context menu
                xContextMenu.insertByIndex( 1, (Object)xRootMenuEntry );

                // The controller should execute the modified context menu and stop notifying other
                // interceptors.
                return ContextMenuInterceptorAction.EXECUTE_MODIFIED ;
            }
        } catch ( UnknownPropertyException ex ) {
            // do something useful
            // we used a unknown property
        } catch ( IndexOutOfBoundsException ex ) {
            // do something useful
            // we used an invalid index for accessing a container
        } catch ( Exception ex ) {
            // something strange has happend!
        } catch ( Throwable ex ) {
            // catch java exceptions and do something useful
        }

        return ContextMenuInterceptorAction.IGNORED;
    }
}