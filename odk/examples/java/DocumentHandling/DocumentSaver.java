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


/** The purpose of this class is to open a specified text document and save this
 * file to a specified URL. The type of the saved file is
 * "swriter: StarOffice XML (Writer)".
 */
public class DocumentSaver {
    /** The main method of the application.
     * @param args The program needs two arguments:
     * - full file name to open,
     * - full file name to save.
     */
    public static void main(String args[]) {
        if ( args.length < 2 ) {
            System.out.println("usage: java -jar DocumentSaver.jar" +
                               "\"<URL|path to load>\" \"<URL|path to save>\"");
            System.out.println("\ne.g.:");
            System.out.println("java -jar DocumentSaver " +
                               "\"file:///f:/TestPrint.doc\"" +
                               "\"file:///f:/TestPrint.odt\"");
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

            java.io.File sourceFile = new java.io.File(args[0]);
            StringBuffer sLoadUrl = new StringBuffer("file:///");
            sLoadUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

            sourceFile = new java.io.File(args[1]);
            StringBuffer sSaveUrl = new StringBuffer("file:///");
            sSaveUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

            com.sun.star.beans.PropertyValue[] propertyValue =
                new com.sun.star.beans.PropertyValue[1];
            propertyValue[0] = new com.sun.star.beans.PropertyValue();
            propertyValue[0].Name = "Hidden";
            propertyValue[0].Value = new Boolean(true);

            Object oDocToStore = xCompLoader.loadComponentFromURL(
                sLoadUrl.toString(), "_blank", 0, propertyValue );
            com.sun.star.frame.XStorable xStorable =
                (com.sun.star.frame.XStorable)UnoRuntime.queryInterface(
                    com.sun.star.frame.XStorable.class, oDocToStore );

            propertyValue = new com.sun.star.beans.PropertyValue[ 2 ];
            propertyValue[0] = new com.sun.star.beans.PropertyValue();
            propertyValue[0].Name = "Overwrite";
            propertyValue[0].Value = new Boolean(true);
            propertyValue[1] = new com.sun.star.beans.PropertyValue();
            propertyValue[1].Name = "FilterName";
            propertyValue[1].Value = "StarOffice XML (Writer)";
            xStorable.storeAsURL( sSaveUrl.toString(), propertyValue );

            System.out.println("\nDocument \"" + sLoadUrl + "\" saved under \"" +
                               sSaveUrl + "\"\n");

            com.sun.star.util.XCloseable xCloseable = (com.sun.star.util.XCloseable)
                UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class,
                                          oDocToStore );

            if (xCloseable != null ) {
                xCloseable.close(false);
            } else
            {
                com.sun.star.lang.XComponent xComp = (com.sun.star.lang.XComponent)
                    UnoRuntime.queryInterface(
                        com.sun.star.lang.XComponent.class, oDocToStore );
                xComp.dispose();
            }
            System.out.println("document closed!");
            System.exit(0);
        }
        catch( Exception e ) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }
}
