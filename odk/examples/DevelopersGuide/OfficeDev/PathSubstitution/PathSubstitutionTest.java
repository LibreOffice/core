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
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;

import com.sun.star.util.XStringSubstitution;

/*
 * @author  Carsten Driesner
 */
public class PathSubstitutionTest extends java.lang.Object {

    /*
     * List of pre-defined path variables supported by
     * the path substitution service.
     */
    private static String[] predefinedPathVariables = {
        "$(home)","$(inst)","$(prog)","$(temp)","$(user)",
        "$(work)","$(path)","$(lang)","$(langid)","$(vlang)"
    };

    /*
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        XComponentContext xRemoteContext = null;
        XMultiComponentFactory xRemoteServiceManager = null;
        XStringSubstitution xPathSubstService = null;

        try {
            // get the remote office context. If necessary a new office
            // process is started
            xRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            xRemoteServiceManager = xRemoteContext.getServiceManager();

            Object pathSubst = xRemoteServiceManager.createInstanceWithContext(
                "com.sun.star.comp.framework.PathSubstitution", xRemoteContext );
            xPathSubstService = (XStringSubstitution)UnoRuntime.queryInterface(
                XStringSubstitution.class, pathSubst);

            /* Work with path variables */
            workWithPathVariables( xPathSubstService );
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    public static void workWithPathVariables( XStringSubstitution xPathSubstService )
    {
        if ( xPathSubstService != null ) {
            for ( int i=0; i<predefinedPathVariables.length; i++ ) {
                try {
                        /* Retrieve values for pre-defined path variables */
                        String aValue = xPathSubstService.getSubstituteVariableValue(
                                            predefinedPathVariables[i] );
                        System.out.println( "Variable: "+ predefinedPathVariables[i] +
                                            " value=" + aValue );
                }
                catch ( com.sun.star.container.NoSuchElementException e) {
                    System.err.println( "NoSuchElementException has been thrown accessing "+predefinedPathVariables[i]);
                }
            }

            // Check the resubstitution function
            try {
                String aPath = xPathSubstService.getSubstituteVariableValue(
                                            predefinedPathVariables[0] ); // Use $(home) as starting point
                aPath += "/test"; // extend the path
                System.out.println( "Path="+aPath );
                String aResubstPath = xPathSubstService.reSubstituteVariables( aPath );
                System.out.println( "Resubstituted path="+aResubstPath );
            }
            catch ( com.sun.star.container.NoSuchElementException e ) {
                System.err.println( "NoSuchElementException has been thrown accessing "+predefinedPathVariables[0]);
            }
        }
    }
}
