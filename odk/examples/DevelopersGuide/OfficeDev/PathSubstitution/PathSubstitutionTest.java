/*************************************************************************
 *
 *  $RCSfile: PathSubstitutionTest.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:42:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;

import com.sun.star.util.XStringSubstitution;
import com.sun.star.frame.TerminationVetoException;
import com.sun.star.frame.XTerminateListener;
/*
 * PathSubstitutionTest.java
 *
 * Created on 26. March 2003, 08:22
 */

/*
 *
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
            // connect
            XComponentContext xLocalContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);
            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();
            Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext );
            XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                XUnoUrlResolver.class, urlResolver );
            Object initialObject = xUnoUrlResolver.resolve(
                "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" );
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, initialObject);
            Object context = xPropertySet.getPropertyValue("DefaultContext");
            xRemoteContext = (XComponentContext)UnoRuntime.queryInterface(
                XComponentContext.class, context);
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
                    System.out.println( "NoSuchElementException has been thrown accessing "+predefinedPathVariables[i]);
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
                    System.out.println( "NoSuchElementException has been thrown accessing "+predefinedPathVariables[0]);
            }
        }
    }
}
