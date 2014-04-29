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


/** This class opens a new or an existing office document.
 */
public class DocumentLoader {
    public static void main(String args[]) {
        if ( args.length < 1 ) {
            System.out.println(
                "usage: java -jar DocumentLoader.jar \"<URL|path>\"" );
            System.out.println( "\ne.g.:" );
            System.out.println(
                "java -jar DocumentLoader.jar \"private:factory/swriter\"" );
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

            String sUrl = args[0];
            if ( sUrl.indexOf("private:") != 0) {
                java.io.File sourceFile = new java.io.File(args[0]);
                StringBuffer sbTmp = new StringBuffer("file:///");
                sbTmp.append(sourceFile.getCanonicalPath().replace('\\', '/'));
                sUrl = sbTmp.toString();
            }

            // Load a Writer document, which will be automatically displayed
            com.sun.star.lang.XComponent xComp = xCompLoader.loadComponentFromURL(
                sUrl, "_blank", 0, new com.sun.star.beans.PropertyValue[0]);

            if ( xComp != null )
                System.exit(0);
            else
                System.exit(1);
        }
        catch( Exception e ) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}
