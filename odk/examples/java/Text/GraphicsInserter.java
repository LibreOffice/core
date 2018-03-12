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
               environment for components which can instantiate within
               frames. */
            com.sun.star.frame.XDesktop xDesktop = UnoRuntime.queryInterface(com.sun.star.frame.XDesktop.class,
                xMCF.createInstanceWithContext("com.sun.star.frame.Desktop",
                                               xContext ) );

            com.sun.star.frame.XComponentLoader xCompLoader =
                UnoRuntime.queryInterface(
                com.sun.star.frame.XComponentLoader.class, xDesktop);

            // Load a Writer document, which will be automatically displayed
            com.sun.star.lang.XComponent xComp = xCompLoader.loadComponentFromURL(
                "private:factory/swriter", "_blank", 0,
                new com.sun.star.beans.PropertyValue[0]);

            // Querying for the interface XTextDocument on the xcomponent
            com.sun.star.text.XTextDocument xTextDoc =
                UnoRuntime.queryInterface(
                com.sun.star.text.XTextDocument.class, xComp);

            // Querying for the interface XMultiServiceFactory on the xtextdocument
            com.sun.star.lang.XMultiServiceFactory xMSFDoc =
                UnoRuntime.queryInterface(
                com.sun.star.lang.XMultiServiceFactory.class, xTextDoc);

            // Providing a log file for output
            PrintWriter printwriterLog = new PrintWriter(
                new BufferedWriter( new FileWriter("log.txt") ) );

            Object oGraphic = null;
            try {
                // Creating the service GraphicObject
                oGraphic =xMSFDoc
                    .createInstance("com.sun.star.text.TextGraphicObject");
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
                UnoRuntime.queryInterface(
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
                UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, oGraphic);
            try {
                // Creating a string for the graphic url
                java.io.File sourceFile = new java.io.File(args[0]);
                StringBuffer sUrl = new StringBuffer("file:///");
                sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));
                System.out.println( "insert graphic \"" + sUrl + "\"");

                com.sun.star.graphic.XGraphicProvider xGraphicProvider =
                    UnoRuntime.queryInterface(com.sun.star.graphic.XGraphicProvider.class,
                        xMCF.createInstanceWithContext("com.sun.star.graphic.GraphicProvider",
                        xContext));


                com.sun.star.beans.PropertyValue[] aMediaProps = new com.sun.star.beans.PropertyValue[] { new com.sun.star.beans.PropertyValue() };
                aMediaProps[0].Name = "URL";
                aMediaProps[0].Value = sUrl;

                com.sun.star.graphic.XGraphic xGraphic =
                    UnoRuntime.queryInterface(com.sun.star.graphic.XGraphic.class,
                                xGraphicProvider.queryGraphic(aMediaProps));

                // Setting the anchor type
                xPropSet.setPropertyValue("AnchorType",
                           com.sun.star.text.TextContentAnchorType.AT_PARAGRAPH );

                // Setting the graphic url
                xPropSet.setPropertyValue( "Graphic", xGraphic );

                // Setting the horizontal position
                xPropSet.setPropertyValue( "HoriOrientPosition",
                                           Integer.valueOf( 5500 ) );

                // Setting the vertical position
                xPropSet.setPropertyValue( "VertOrientPosition",
                                           Integer.valueOf( 4200 ) );

                // Setting the width
                xPropSet.setPropertyValue( "Width", Integer.valueOf( 4400 ) );

                // Setting the height
                xPropSet.setPropertyValue( "Height", Integer.valueOf( 4000 ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
