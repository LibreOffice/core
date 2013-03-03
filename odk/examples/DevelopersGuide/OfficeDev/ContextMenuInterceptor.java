/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.beans.XPropertySet;
import com.sun.star.ui.ActionTriggerSeparatorType;
import com.sun.star.ui.ContextMenuInterceptorAction;
import com.sun.star.ui.XContextMenuInterceptor;
import com.sun.star.uno.UnoRuntime;

public class ContextMenuInterceptor implements XContextMenuInterceptor {

    /**
     *Description of the Method
     *
     *@param  args  Description of Parameter
     *@since
     */
    public static void main(String args[])
    {
        try {
            OfficeConnect aConnect = OfficeConnect.createConnection();

            com.sun.star.frame.XDesktop xDesktop =
                (com.sun.star.frame.XDesktop)aConnect.createRemoteInstance(
                    com.sun.star.frame.XDesktop.class,"com.sun.star.frame.Desktop");

            // create a new test document
            com.sun.star.frame.XComponentLoader xCompLoader =
                (com.sun.star.frame.XComponentLoader)UnoRuntime.queryInterface(
                    com.sun.star.frame.XComponentLoader.class, xDesktop);

            com.sun.star.lang.XComponent xComponent =
                xCompLoader.loadComponentFromURL("private:factory/swriter",
                    "_blank", 0, new com.sun.star.beans.PropertyValue[0]);

            // intialize the test document
            com.sun.star.frame.XFrame xFrame = null;
            {
            com.sun.star.text.XTextDocument xDoc =(com.sun.star.text.XTextDocument)
                UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class,
                                          xComponent);

            String infoMsg = new String("All context menus of the created document frame contains now a 'Help' entry with the submenus 'Content', 'Help Agent' and 'Tips'.\n\nPress 'Return' in the shell to remove the context menu interceptor and finish the example!");
            xDoc.getText().setString(infoMsg);

            // ensure that the document content is optimal visible
            com.sun.star.frame.XModel xModel =
                (com.sun.star.frame.XModel)UnoRuntime.queryInterface(
                    com.sun.star.frame.XModel.class, xDoc);
            // get the frame for later usage
            xFrame = xModel.getCurrentController().getFrame();

            com.sun.star.view.XViewSettingsSupplier xViewSettings =
                (com.sun.star.view.XViewSettingsSupplier)UnoRuntime.queryInterface(
                    com.sun.star.view.XViewSettingsSupplier.class, xModel.getCurrentController());
            xViewSettings.getViewSettings().setPropertyValue(
                "ZoomType", new Short((short)0));
            }
            // test document will be closed later

            // reuse the frame
            com.sun.star.frame.XController xController = xFrame.getController();
            if ( xController != null ) {
                com.sun.star.ui.XContextMenuInterception xContextMenuInterception =
                    (com.sun.star.ui.XContextMenuInterception)UnoRuntime.queryInterface(
                        com.sun.star.ui.XContextMenuInterception.class, xController );
                if( xContextMenuInterception != null ) {
                    ContextMenuInterceptor aContextMenuInterceptor = new ContextMenuInterceptor();
                    com.sun.star.ui.XContextMenuInterceptor xContextMenuInterceptor =
                        (com.sun.star.ui.XContextMenuInterceptor)UnoRuntime.queryInterface(
                            com.sun.star.ui.XContextMenuInterceptor.class, aContextMenuInterceptor );
                    xContextMenuInterception.registerContextMenuInterceptor( xContextMenuInterceptor );

                    System.out.println( "\n ... all context menus of the created document frame contains now a 'Help' entry with the\n     submenus 'Content', 'Help Agent' and 'Tips'.\n\nPress 'Return' to remove the context menu interceptor and finish the example!");

                    java.io.BufferedReader reader
                        = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));
                    reader.read();

                    xContextMenuInterception.releaseContextMenuInterceptor(
                        xContextMenuInterceptor );
                    System.out.println( " ... context menu interceptor removed!" );
                }
            }

            // close test document
            com.sun.star.util.XCloseable xCloseable = (com.sun.star.util.XCloseable)
                UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class,
                                          xComponent );

            if (xCloseable != null ) {
                xCloseable.close(false);
            } else
            {
                xComponent.dispose();
            }
        }
        catch ( com.sun.star.uno.RuntimeException ex ) {
            // something strange has happened!
            System.out.println( " Sample caught exception! " + ex );
            System.exit(1);
        }
        catch ( java.lang.Exception ex ) {
            // catch java exceptions and do something useful
            System.out.println( " Sample caught exception! " + ex );
            System.exit(1);
        }

        System.out.println(" ... exit!\n");
        System.exit( 0 );
    }

    /**
     *Description of the Method
     *
     *@param  args  Description of Parameter
     *@since
     */
    public ContextMenuInterceptorAction notifyContextMenuExecute(
             com.sun.star.ui.ContextMenuExecuteEvent aEvent ) throws RuntimeException {

        try {

            // Retrieve context menu container and query for service factory to
            // create sub menus, menu entries and separators
            com.sun.star.container.XIndexContainer xContextMenu = aEvent.ActionTriggerContainer;
            com.sun.star.lang.XMultiServiceFactory xMenuElementFactory =
                (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                com.sun.star.lang.XMultiServiceFactory.class, xContextMenu );
            if ( xMenuElementFactory != null ) {
                // create root menu entry and sub menu
                com.sun.star.beans.XPropertySet xRootMenuEntry =
                    (XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class,
                        xMenuElementFactory.createInstance( "com.sun.star.ui.ActionTrigger" ));

                // create a line separator for our new help sub menu
                com.sun.star.beans.XPropertySet xSeparator =
                    (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class,
                        xMenuElementFactory.createInstance( "com.sun.star.ui.ActionTriggerSeparator" ));

                Short aSeparatorType = new Short( ActionTriggerSeparatorType.LINE );
                xSeparator.setPropertyValue( "SeparatorType", (Object)aSeparatorType );

                // query sub menu for index container to get access
                com.sun.star.container.XIndexContainer xSubMenuContainer =
                    (com.sun.star.container.XIndexContainer)UnoRuntime.queryInterface(
                        com.sun.star.container.XIndexContainer.class,
                            xMenuElementFactory.createInstance(
                                "com.sun.star.ui.ActionTriggerContainer" ));

                // intialize root menu entry
                xRootMenuEntry.setPropertyValue( "Text", new String( "Help" ));
                xRootMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5410" ));
                xRootMenuEntry.setPropertyValue( "HelpURL", new String( "5410" ));
                xRootMenuEntry.setPropertyValue( "SubContainer", (Object)xSubMenuContainer );

                // create menu entries for the new sub menu

                // intialize help/content menu entry
                XPropertySet xMenuEntry = (XPropertySet)UnoRuntime.queryInterface(
                                              XPropertySet.class, xMenuElementFactory.createInstance(
                                                  "com.sun.star.ui.ActionTrigger" ));

                xMenuEntry.setPropertyValue( "Text", new String( "Content" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5401" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5401" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 0, (Object)xMenuEntry );

                // intialize help/help agent
                xMenuEntry = (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                                 com.sun.star.beans.XPropertySet.class,
                                     xMenuElementFactory.createInstance(
                                         "com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Help Agent" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5962" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5962" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 1, (Object)xMenuEntry );

                // intialize help/tips
                xMenuEntry = (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                                 com.sun.star.beans.XPropertySet.class,
                                     xMenuElementFactory.createInstance(
                                         "com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Tips" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5404" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5404" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 2, (Object)xMenuEntry );

                // add separator into the given context menu
                xContextMenu.insertByIndex( 0, (Object)xSeparator );

                // add new sub menu into the given context menu
                xContextMenu.insertByIndex( 0, (Object)xRootMenuEntry );

                // The controller should execute the modified context menu and stop notifying other
                // interceptors.
                return com.sun.star.ui.ContextMenuInterceptorAction.EXECUTE_MODIFIED;
            }
        }
        catch ( com.sun.star.beans.UnknownPropertyException ex ) {
            // do something useful
            // we used a unknown property
        }
        catch ( com.sun.star.lang.IndexOutOfBoundsException ex ) {
            // do something useful
            // we used an invalid index for accessing a container
        }
        catch ( com.sun.star.uno.Exception ex ) {
            // something strange has happened!
        }
        catch ( java.lang.Exception ex ) {
            // catch java exceptions and something useful
        }

        return com.sun.star.ui.ContextMenuInterceptorAction.IGNORED;
    }
}
