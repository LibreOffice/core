/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



import com.sun.star.uno.UnoRuntime;

/** This class gives you information on the selected objects (text range, text
 * frame, or graphics) at an OpenOffice.org Server. The Office must be started in
 * advance and you must have selected something (text, graphics, ...)
 */
public class WriterSelector {
    /**
     * @param args No arguments.
     */
    public static void main(String args[]) {
        com.sun.star.uno.XComponentContext xContext = null;

        try {

            // bootstrap UNO and get the remote component context. The context can
            // be used to get the service manager
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote office service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            // get a new instance of the desktop
            com.sun.star.frame.XDesktop xDesktop = (com.sun.star.frame.XDesktop)
                UnoRuntime.queryInterface(com.sun.star.frame.XDesktop.class,
                    xMCF.createInstanceWithContext("com.sun.star.frame.Desktop",
                                                   xContext ) );

            com.sun.star.frame.XComponentLoader xCompLoader =
                (com.sun.star.frame.XComponentLoader)UnoRuntime.queryInterface(
                    com.sun.star.frame.XComponentLoader.class, xDesktop);

            com.sun.star.lang.XComponent xComponent =
                xCompLoader.loadComponentFromURL("private:factory/swriter",
                    "_blank", 0, new com.sun.star.beans.PropertyValue[0]);
            {
            com.sun.star.text.XTextDocument xDoc =(com.sun.star.text.XTextDocument)
                UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class,
                                          xComponent);
            xDoc.getText().setString("Please select something in this text and press then \"return\" in the shell where you have started the example.\n");

            // ensure that the document content is optimal visible
            com.sun.star.frame.XModel xModel =
                (com.sun.star.frame.XModel)UnoRuntime.queryInterface(
                    com.sun.star.frame.XModel.class, xDoc);

            com.sun.star.view.XViewSettingsSupplier xViewSettings =
                (com.sun.star.view.XViewSettingsSupplier)UnoRuntime.queryInterface(
                    com.sun.star.view.XViewSettingsSupplier.class, xModel.getCurrentController());
            xViewSettings.getViewSettings().setPropertyValue(
                "ZoomType", new Short((short)0));
            }
            // test document will be closed later

            System.out.println("\nPlease select something in the test document and press then \"return\" to continues the example ...");
            char c = 'X';
            do{
                c = (char) System.in.read();
            }while ((c != 13) && (c != 10));

            // Getting the current frame from the OpenOffice.org Server.
            com.sun.star.frame.XFrame xframe = xDesktop.getCurrentFrame();

            // Getting the controller.
            com.sun.star.frame.XController xController = xframe.getController();

            com.sun.star.view.XSelectionSupplier xSelSupplier =
                (com.sun.star.view.XSelectionSupplier)UnoRuntime.queryInterface(
                    com.sun.star.view.XSelectionSupplier.class, xController );

            Object oSelection = xSelSupplier.getSelection();

            com.sun.star.lang.XServiceInfo xServInfo =
                (com.sun.star.lang.XServiceInfo)UnoRuntime.queryInterface(
                    com.sun.star.lang.XServiceInfo.class, oSelection );

            if ( xServInfo.supportsService("com.sun.star.text.TextRanges") )
            {
                com.sun.star.container.XIndexAccess xIndexAccess =
                    (com.sun.star.container.XIndexAccess)UnoRuntime.queryInterface(
                        com.sun.star.container.XIndexAccess.class, oSelection);

                int count = xIndexAccess.getCount();
                com.sun.star.text.XTextRange xTextRange = null;
                for ( int i = 0; i < count; i++ ) {
                    xTextRange = (com.sun.star.text.XTextRange)
                        UnoRuntime.queryInterface(
                            com.sun.star.text.XTextRange.class,
                            xIndexAccess.getByIndex(i));

                    System.out.println( "You have selected a text range: \""
                                        + xTextRange.getString() + "\"." );
                }
            }

            if ( xServInfo.supportsService("com.sun.star.text.TextGraphicObject") )
            {
                System.out.println( "You have selected a graphics." );
            }

            if ( xServInfo.supportsService("com.sun.star.text.TextTableCursor") )
            {
                System.out.println( "You have selected a text table." );
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

            System.exit(0);
        }
        catch( Exception e ) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}
