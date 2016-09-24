/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
            com.sun.star.frame.XDesktop xDesktop = UnoRuntime.queryInterface(com.sun.star.frame.XDesktop.class,
                xMCF.createInstanceWithContext("com.sun.star.frame.Desktop",
                                               xContext ) );

            com.sun.star.frame.XComponentLoader xCompLoader =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XComponentLoader.class, xDesktop);

            com.sun.star.lang.XComponent xComponent =
                xCompLoader.loadComponentFromURL("private:factory/swriter",
                    "_blank", 0, new com.sun.star.beans.PropertyValue[0]);
            {
            com.sun.star.text.XTextDocument xDoc =UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class,
                                      xComponent);
            xDoc.getText().setString("Please select something in this text and press then \"return\" in the shell where you have started the example.\n");

            // ensure that the document content is optimal visible
            com.sun.star.frame.XModel xModel =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XModel.class, xDoc);

            com.sun.star.view.XViewSettingsSupplier xViewSettings =
                UnoRuntime.queryInterface(
                com.sun.star.view.XViewSettingsSupplier.class, xModel.getCurrentController());
            xViewSettings.getViewSettings().setPropertyValue(
                "ZoomType", Short.valueOf((short)0));
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
                UnoRuntime.queryInterface(
                com.sun.star.view.XSelectionSupplier.class, xController );

            Object oSelection = xSelSupplier.getSelection();

            com.sun.star.lang.XServiceInfo xServInfo =
                UnoRuntime.queryInterface(
                com.sun.star.lang.XServiceInfo.class, oSelection );

            if ( xServInfo.supportsService("com.sun.star.text.TextRanges") )
            {
                com.sun.star.container.XIndexAccess xIndexAccess =
                    UnoRuntime.queryInterface(
                    com.sun.star.container.XIndexAccess.class, oSelection);

                int count = xIndexAccess.getCount();
                com.sun.star.text.XTextRange xTextRange = null;
                for ( int i = 0; i < count; i++ ) {
                    xTextRange = UnoRuntime.queryInterface(
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
            com.sun.star.util.XCloseable xCloseable = UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
