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

import com.sun.star.configuration.theDefaultProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.XURLTransformer;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.text.XTextDocument;

/*
 * Provides example code how to enable/disable
 * commands.
 */
public class DisableCommandsTest {

    /*
     * A list of command names
     */
    final private static String[] aCommandURLTestSet =
    {
        "Open",
        "About",
        "SelectAll",
        "Quit",
    };

    private static XComponentContext xRemoteContext = null;
    private static XMultiComponentFactory xRemoteServiceManager = null;
    private static XURLTransformer xTransformer = null;
    private static XMultiServiceFactory xConfigProvider = null;

    /*
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        try {
            // get the remote office context. If necessary a new office
            // process is started
            xRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            xRemoteServiceManager = xRemoteContext.getServiceManager();

            Object transformer = xRemoteServiceManager.createInstanceWithContext(
                          "com.sun.star.util.URLTransformer", xRemoteContext );
            xTransformer = UnoRuntime.queryInterface(com.sun.star.util.XURLTransformer.class,
                                      transformer );

            xConfigProvider = theDefaultProvider.get(xRemoteContext);

            // create a new test document
            Object oDesktop = xRemoteServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xRemoteContext);

            XComponentLoader xCompLoader =UnoRuntime.queryInterface(XComponentLoader.class, oDesktop);

            com.sun.star.lang.XComponent xComponent =
                xCompLoader.loadComponentFromURL("private:factory/swriter",
                    "_blank", 0, new com.sun.star.beans.PropertyValue[0]);
            {
            XTextDocument xDoc =UnoRuntime.queryInterface(XTextDocument.class, xComponent);
            xDoc.getText().setString("You can now check the disabled commands. The "
                                     +"following commands are disabled:\n\n"
                                     +"   Open...\n   Exit\n   Select All\n   "
                                     +"About StarOffice|OpenOffice\n\nPress "
                                     + "\"return\" in the shell where you have "
                                     + "started the example to enable the "
                                     + "commands!\n\nCheck the commands again and "
                                     + "press once more \"return\" to finish the "
                                     + "example and close the document.");

            // ensure that the document content is optimal visible
            com.sun.star.frame.XModel xModel =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XModel.class, xDoc);
            // get the frame for later usage
            com.sun.star.frame.XFrame xFrame =
                xModel.getCurrentController().getFrame();

            com.sun.star.view.XViewSettingsSupplier xViewSettings =
                UnoRuntime.queryInterface(
                com.sun.star.view.XViewSettingsSupplier.class,
                xModel.getCurrentController());
            xViewSettings.getViewSettings().setPropertyValue(
                "ZoomType", Short.valueOf((short)0));
            }
            // test document will be closed later

            // First we need a defined starting point. So we have to remove
            // all commands from the disabled set!
            enableCommands();

            // Check if the commands are usable
            testCommands( false );

            // Disable the commands
            disableCommands();

            // Now the commands shouldn't be usable anymore
            testCommands( true );

            // you can now check the test document and see which commands are
            // disabled
            System.out.println("\nYou can now check the disabled commands.\n"
                               +"Please press 'return' to enable the commands!");
            waitForUserInput();

            // Remove disable commands to make Office usable again
            enableCommands();

            // you can check the test document again and see that the commands
            // are enabled now
            System.out.println("Check again the now enabled commands.\n"
                               +"Please press 'return' to finish the example and "
                               +"close the document!");
            waitForUserInput();

            // close test document
            com.sun.star.util.XCloseable xCloseable = UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class,
                                      xComponent );

            if (xCloseable != null ) {
                xCloseable.close(false);
            } else
            {
                xComponent.dispose();
            }
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    /**
     * Wait for user input -> until the user press 'return'
     */
    private static void waitForUserInput() throws java.io.IOException {

        java.io.BufferedReader reader
            = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        reader.read();
    }

    /**
     * Test the commands that we enabled/disabled
     */
    private static void testCommands( boolean bDisabledCmds )
        throws com.sun.star.uno.Exception
    {
        // We need the desktop to get access to the current frame
        Object desktop = xRemoteServiceManager.createInstanceWithContext(
                            "com.sun.star.frame.Desktop", xRemoteContext );
        com.sun.star.frame.XDesktop xDesktop = UnoRuntime.queryInterface(com.sun.star.frame.XDesktop.class, desktop );
        com.sun.star.frame.XFrame xFrame = xDesktop.getCurrentFrame();
        com.sun.star.frame.XDispatchProvider xDispatchProvider = null;
        if ( xFrame != null )
        {
            // We have a frame. Now we need access to the dispatch provider.
            xDispatchProvider =
                UnoRuntime.queryInterface(
                    com.sun.star.frame.XDispatchProvider.class, xFrame );
            if ( xDispatchProvider != null )
            {
                // As we have the dispatch provider we can now check if we get
                // a dispatch object or not.
                for ( int n = 0; n < aCommandURLTestSet.length; n++ )
                {
                    // Prepare the URL
                    com.sun.star.util.URL[] aURL  = new com.sun.star.util.URL[1];
                    aURL[0] = new com.sun.star.util.URL();
                    com.sun.star.frame.XDispatch xDispatch = null;

                    aURL[0].Complete = ".uno:" + aCommandURLTestSet[n];
                    xTransformer.parseSmart( aURL, ".uno:" );

                    // Try to get a dispatch object for our URL
                    xDispatch = xDispatchProvider.queryDispatch( aURL[0], "", 0 );

                    if ( xDispatch != null )
                    {
                        if ( bDisabledCmds )
                            System.out.println(
                                "Something is wrong, I got dispatch object for "
                                + aURL[0].Complete );
                        else
                            System.out.println( "Ok, dispatch object for "
                                                + aURL[0].Complete  );
                    }
                    else
                    {
                        if ( !bDisabledCmds )
                            System.out.println("Something is wrong, I cannot get dispatch object for " + aURL[0].Complete );
                        else
                            System.out.println( "Ok, no dispatch object for "
                                                + aURL[0].Complete );
                    }
                    resetURL( aURL[0] );
                }
            }
            else
                System.out.println( "Couldn't get XDispatchProvider from Frame!" );
        }
        else
            System.out.println( "Couldn't get current Frame from Desktop!" );
    }

    /**
     * Ensure that there are no disabled commands in the user layer. The
     * implementation removes all commands from the disabled set!
     */
    private static void enableCommands() {
        // Set the root path for our configuration access
        com.sun.star.beans.PropertyValue[] lParams =
            new com.sun.star.beans.PropertyValue[1];

        lParams[0] = new com.sun.star.beans.PropertyValue();
        lParams[0].Name  = "nodepath";
        lParams[0].Value = "/org.openoffice.Office.Commands/Execute/Disabled";

        try {
            // Create configuration update access to have write access to the
            // configuration
            Object xAccess = xConfigProvider.createInstanceWithArguments(
                             "com.sun.star.configuration.ConfigurationUpdateAccess",
                             lParams );

            com.sun.star.container.XNameAccess xNameAccess =
                UnoRuntime.queryInterface(
                com.sun.star.container.XNameAccess.class, xAccess );

            if ( xNameAccess != null ) {
                // We need the XNameContainer interface to remove the nodes by name
                com.sun.star.container.XNameContainer xNameContainer =
                    UnoRuntime.queryInterface(
                    com.sun.star.container.XNameContainer.class, xAccess );

                // Retrieves the names of all Disabled nodes
                String[] aCommandsSeq = xNameAccess.getElementNames();
                for ( int n = 0; n < aCommandsSeq.length; n++ ) {
                    try {
                        // remove the node
                        xNameContainer.removeByName( aCommandsSeq[n] );
                    }
                    catch ( com.sun.star.lang.WrappedTargetException e ) {
                    }
                    catch ( com.sun.star.container.NoSuchElementException e ) {
                    }
                }
            }

            // Commit our changes
            com.sun.star.util.XChangesBatch xFlush =
                UnoRuntime.queryInterface(
                com.sun.star.util.XChangesBatch.class, xAccess);

            xFlush.commitChanges();
        }
        catch ( com.sun.star.uno.Exception e ) {
            System.out.println( "Exception detected!" );
            System.out.println( e );
        }
    }

    /**
     * Disable all commands defined in the aCommandURLTestSet array
     */
    private static void disableCommands() {
        // Set the root path for our configuration access
        com.sun.star.beans.PropertyValue[] lParams =
            new com.sun.star.beans.PropertyValue[1];

        lParams[0] = new com.sun.star.beans.PropertyValue();
        lParams[0].Name  = "nodepath";
        lParams[0].Value = "/org.openoffice.Office.Commands/Execute/Disabled";

        try {
            // Create configuration update access to have write access to the
            // configuration
            Object xAccess = xConfigProvider.createInstanceWithArguments(
                             "com.sun.star.configuration.ConfigurationUpdateAccess",
                             lParams );

            com.sun.star.lang.XSingleServiceFactory xSetElementFactory =
                UnoRuntime.queryInterface(
                com.sun.star.lang.XSingleServiceFactory.class, xAccess );

            com.sun.star.container.XNameContainer xNameContainer =
                UnoRuntime.queryInterface(
                com.sun.star.container.XNameContainer.class, xAccess );

            if ( xSetElementFactory != null && xNameContainer != null ) {
                Object[] aArgs = new Object[0];

                for ( int i = 0; i < aCommandURLTestSet.length; i++ ) {
                    // Create the nodes with the XSingleServiceFactory of the
                    // configuration
                    Object xNewElement =
                        xSetElementFactory.createInstanceWithArguments( aArgs );

                    if ( xNewElement != null ) {
                        // We have a new node. To set the properties of the node
                        // we need the XPropertySet interface.
                        com.sun.star.beans.XPropertySet xPropertySet =
                            UnoRuntime.queryInterface(
                            com.sun.star.beans.XPropertySet.class,
                            xNewElement );

                        if ( xPropertySet != null ) {
                            // Create a unique node name.
                            String aCmdNodeName = "Command-";
                            aCmdNodeName += i;

                            // Insert the node into the Disabled set
                            xPropertySet.setPropertyValue( "Command",
                                                           aCommandURLTestSet[i] );
                            xNameContainer.insertByName( aCmdNodeName,
                                                         xNewElement );
                        }
                    }
                }

                // Commit our changes
                com.sun.star.util.XChangesBatch xFlush =
                    UnoRuntime.queryInterface(
                    com.sun.star.util.XChangesBatch.class, xAccess);
                xFlush.commitChanges();
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            System.err.println( "Exception detected!" + e);
            e.printStackTrace();
        }
    }

    /**
     * reset URL so it can be reused
     *
     * @param aURL
     *          the URL that should be reset
     */
    private static void resetURL( com.sun.star.util.URL aURL )
    {
        aURL.Protocol   = "";
        aURL.User       = "";
        aURL.Password   = "";
        aURL.Server     = "";
        aURL.Port       = 0;
        aURL.Path       = "";
        aURL.Name       = "";
        aURL.Arguments  = "";
        aURL.Mark       = "";
        aURL.Main       = "";
        aURL.Complete   = "";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
