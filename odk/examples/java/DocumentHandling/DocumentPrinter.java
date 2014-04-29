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
                (com.sun.star.frame.XComponentLoader)
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
                (com.sun.star.view.XPrintable)UnoRuntime.queryInterface(
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
