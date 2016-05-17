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


public class DocumentPrinter {
    public static void main(String args[]) {
        if ( args.length < 3 ) {
            System.out.println("usage: java -jar DocumentLoader.jar " +
                               "\"<Favoured printer>\" \"<URL|path>\" \"<Pages>\"");
            System.out.println( "\ne.g.:" );
            System.out.println("java -jar DocumentLoader.jar \"amadeus\" " +
                               "\"file:///f:/TestPrint.odt\" \"1-3;7;9\"");
            System.exit(1);
      }

        com.sun.star.uno.XComponentContext xContext = null;

        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote office service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            Object oDesktop = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xContext);

            com.sun.star.frame.XComponentLoader xCompLoader =
                UnoRuntime.queryInterface(
                 com.sun.star.frame.XComponentLoader.class, oDesktop);

            java.io.File sourceFile = new java.io.File(args[1]);
            StringBuffer sUrl = new StringBuffer("file:///");
            sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

            // Load a Writer document, which will be automatically displayed
            com.sun.star.lang.XComponent xComp = xCompLoader.loadComponentFromURL(
                sUrl.toString(), "_blank", 0,
                new com.sun.star.beans.PropertyValue[0] );

            // Querying for the interface XPrintable on the loaded document
            com.sun.star.view.XPrintable xPrintable =
                UnoRuntime.queryInterface(
                com.sun.star.view.XPrintable.class, xComp);

            // Setting the property "Name" for the favoured printer (name of
            // IP address)
            com.sun.star.beans.PropertyValue propertyValue[] =
                new com.sun.star.beans.PropertyValue[1];
            propertyValue[0] = new com.sun.star.beans.PropertyValue();
            propertyValue[0].Name = "Name";
            propertyValue[0].Value = args[ 0 ];

            // Setting the name of the printer
            xPrintable.setPrinter( propertyValue );

            // Setting the property "Pages" so that only the desired pages
            // will be printed.
            propertyValue[0] = new com.sun.star.beans.PropertyValue();
            propertyValue[0].Name = "Pages";
            propertyValue[0].Value = args[ 2 ];

            // Printing the loaded document
            xPrintable.print( propertyValue );

            System.exit(0);
        }
        catch( Exception e ) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
