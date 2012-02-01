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



// __________ Imports __________

import com.sun.star.uno.UnoRuntime;

public class Helper
{
    // __________ static helper methods __________

    /** Connect to an office, if no office is running a new instance is started.
     * A new connection is established and the service manger from the running
     * offic eis returned.
     */
    static public com.sun.star.uno.XComponentContext connect()
        throws Exception
    {
        // get the remote office component context
        com.sun.star.uno.XComponentContext xOfficeContext =
            com.sun.star.comp.helper.Bootstrap.bootstrap();

        // if connection fails an exception is thrown
        System.out.println("Connected to a running office ...");

        return xOfficeContext;
    }

    /** creates and instantiates new document
    */
    static public com.sun.star.lang.XComponent createDocument(
        com.sun.star.uno.XComponentContext xOfficeContext,
        String sURL, String sTargetFrame, int nSearchFlags,
        com.sun.star.beans.PropertyValue[] aArgs )
            throws Exception
    {
        com.sun.star.lang.XComponent xComponent = null;
        com.sun.star.frame.XComponentLoader aLoader =
            (com.sun.star.frame.XComponentLoader)UnoRuntime.queryInterface(
                com.sun.star.frame.XComponentLoader.class,
                xOfficeContext.getServiceManager().createInstanceWithContext(
                    "com.sun.star.frame.Desktop", xOfficeContext));

        xComponent = (com.sun.star.lang.XComponent)UnoRuntime.queryInterface(
            com.sun.star.lang.XComponent.class, aLoader.loadComponentFromURL(
                sURL, sTargetFrame, nSearchFlags, aArgs ) );

        if ( xComponent == null )
            throw new Exception( "could not create document: " + sURL );
        return xComponent;
    }
}
