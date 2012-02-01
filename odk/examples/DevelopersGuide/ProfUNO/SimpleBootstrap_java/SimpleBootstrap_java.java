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



import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;

public class SimpleBootstrap_java {

    public static void main(String[] args) {

        try {
            // get the remote office component context
            XComponentContext xContext =
                com.sun.star.comp.helper.Bootstrap.bootstrap();

            // get the remote office service manager
            XMultiComponentFactory xServiceManager =
                xContext.getServiceManager();

            // get an instance of the remote office desktop UNO service
            Object desktop = xServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xContext );

            // query the XComponentLoader interface from the desktop
            XComponentLoader xComponentLoader =
                (XComponentLoader)UnoRuntime.queryInterface(
                    XComponentLoader.class, desktop );

            // load a spreadsheet document
            String loadURL = "private:factory/scalc";
            PropertyValue[] loadProps = new PropertyValue[0];
            xComponentLoader.loadComponentFromURL(
                loadURL, "_blank", 0, loadProps);
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        System.exit( 0 );
    }
}
