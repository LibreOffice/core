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

import java.io.PrintWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;


public class GraphicsInserter {
    public static void main(String args[]) {
        if ( args.length < 1 )
        {
            System.out.println(
                "usage: java -jar GraphicsInserter.jar \"<Graphic URL|path>\"" );
            System.out.println( "\ne.g.:" );
            System.out.println(
                "java -jar GraphicsInserter.jar \"file:///f:/TestGraphics.gif\"" );
            System.exit( 1 );
        }

        com.sun.star.uno.XComponentContext xContext = null;

        try {

            // bootstrap UNO and get the remote component context. The context can
            // be used to get the service manager
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote office service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            com.sun.star.frame.XDesktop xDesktop = (com.sun.star.frame.XDesktop)
                UnoRuntime.queryInterface(com.sun.star.frame.XDesktop.class,
                    xMCF.createInstanceWithContext("com.sun.star.frame.Desktop",
                                                   xContext ) );

            com.sun.star.frame.XComponentLoader xCompLoader =
                (com.sun.star.frame.XComponentLoader)UnoRuntime.queryInterface(
                    com.sun.star.frame.XComponentLoader.class, xDesktop);

            // Load a Writer document, which will be automatically displayed
            com.sun.star.lang.XComponent xComp = xCompLoader.loadComponentFromURL(
                "private:factory/swriter", "_blank", 0,
                new com.sun.star.beans.PropertyValue[0]);

            // Querying for the interface XTextDocument on the xcomponent
            com.sun.star.text.XTextDocument xTextDoc =
                (com.sun.star.text.XTextDocument)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextDocument.class, xComp);

            // Querying for the interface XMultiServiceFactory on the xtextdocument
            com.sun.star.lang.XMultiServiceFactory xMSFDoc =
                (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                    com.sun.star.lang.XMultiServiceFactory.class, xTextDoc);

            // Providing a log file for output
            PrintWriter printwriterLog = new PrintWriter(
                new BufferedWriter( new FileWriter("log.txt") ) );

            Object oGraphic = null;
            try {
                // Creating the service GraphicObject
                oGraphic =
                    xMSFDoc.createInstance("com.sun.star.text.TextGraphicObject");
            }
            catch ( Exception exception ) {
                System.out.println( "Could not create instance" );
                exception.printStackTrace( printwriterLog );
            }

            // Getting the text
            com.sun.star.text.XText xText = xTextDoc.getText();

            // Getting the cursor on the document
            com.sun.star.text.XTextCursor xTextCursor = xText.createTextCursor();

            // Querying for the interface XTextContent on the GraphicObject
            com.sun.star.text.XTextContent xTextContent =
                (com.sun.star.text.XTextContent)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextContent.class, oGraphic );

            // Printing information to the log file
            printwriterLog.println( "inserting graphic" );
            try {
                // Inserting the content
                xText.insertTextContent(xTextCursor, xTextContent, true);
            } catch ( Exception exception ) {
                System.out.println( "Could not insert Content" );
                exception.printStackTrace(System.err);
            }

            // Printing information to the log file
            printwriterLog.println( "adding graphic" );

            // Querying for the interface XPropertySet on GraphicObject
            com.sun.star.beans.XPropertySet xPropSet =
                (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, oGraphic);
            try {
                // Creating a string for the graphic url
                java.io.File sourceFile = new java.io.File(args[0]);
                StringBuffer sUrl = new StringBuffer("file:///");
                sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));
                System.out.println( "insert graphic \"" + sUrl + "\"");

                // Setting the anchor type
                xPropSet.setPropertyValue("AnchorType",
                           com.sun.star.text.TextContentAnchorType.AT_PARAGRAPH );

                // Setting the graphic url
                xPropSet.setPropertyValue( "GraphicURL", sUrl.toString() );

                // Setting the horizontal position
                xPropSet.setPropertyValue( "HoriOrientPosition",
                                           new Integer( 5500 ) );

                // Setting the vertical position
                xPropSet.setPropertyValue( "VertOrientPosition",
                                           new Integer( 4200 ) );

                // Setting the width
                xPropSet.setPropertyValue( "Width", new Integer( 4400 ) );

                // Setting the height
                xPropSet.setPropertyValue( "Height", new Integer( 4000 ) );
            } catch ( Exception exception ) {
                System.out.println( "Couldn't set property 'GraphicURL'" );
                exception.printStackTrace( printwriterLog );
            }

            xContext = null;

            System.exit(0);
        }
        catch( Exception e ) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}
