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



import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.uno.UnoRuntime;

import java.io.DataInputStream;

public class TestInspector {

    public static void main(String args[]) {
        com.sun.star.uno.XComponentContext xContext = null;
        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if( xContext != null )
                System.out.println("Connected to a running office ...");
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
            System.exit(1);
        }

        try {
            com.sun.star.lang.XMultiComponentFactory xMCF = xContext.getServiceManager();
            // Creating an instance of the instance inspector with arguments

            XSingleComponentFactory xFactory = Inspector.__getComponentFactory(Inspector._Inspector.class.getName());
            Object obj= null;
            if (xFactory != null) {
                obj = xFactory.createInstanceWithContext(xContext);
            }
            org.openoffice.XInstanceInspector xInstInspector = null;
            if (obj != null) {
                xInstInspector = (org.openoffice.XInstanceInspector)UnoRuntime.queryInterface(org.openoffice.XInstanceInspector.class, obj);
            }

            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            com.sun.star.frame.XComponentLoader xCmpLoader =  (com.sun.star.frame.XComponentLoader)UnoRuntime.queryInterface( com.sun.star.frame.XComponentLoader.class,
                    xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", xContext));

            // Load a new spreadsheet document, which will be automatically
            // displayed and is used for inspection
            com.sun.star.lang.XComponent xComp = xCmpLoader.loadComponentFromURL("private:factory/scalc", "_blank", 0, new com.sun.star.beans.PropertyValue[0] );
            xInstInspector.inspect(xCmpLoader, "");
            System.out.println("You can now inspect the new spreadsheet " + "document ...\n");
        }
        catch( Exception e ) {
            System.err.println( e + e.getMessage());
            e.printStackTrace();
        }
//        System.exit( 0 );
    }
}
