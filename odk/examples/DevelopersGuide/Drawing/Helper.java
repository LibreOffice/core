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
