/*************************************************************************
 *
 *  $RCSfile: TestInspector.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-31 17:12:17 $
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
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            // Creating an instance of the instance inspector with arguments
            org.openoffice.XInstanceInspector xInstInspector =
                org.openoffice.InstanceInspector.create(xContext);

            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            com.sun.star.frame.XComponentLoader xCmpLoader =
                (com.sun.star.frame.XComponentLoader)UnoRuntime.queryInterface(
                    com.sun.star.frame.XComponentLoader.class,
                    xMCF.createInstanceWithContext(
                        "com.sun.star.frame.Desktop", xContext));

            // Load a new spreadsheet document, which will be automaticly
            // displayed and is used for inspection
            com.sun.star.lang.XComponent xComp =
                xCmpLoader.loadComponentFromURL("private:factory/scalc", "_blank",
                                  0, new com.sun.star.beans.PropertyValue[0] );
            // Inspect the spreadsheet
            xInstInspector.inspect( xComp );

            System.out.println("You can now insepct the new spreadsheet " +
                               "document ...\n");

        }
        catch( Exception e ) {
            System.err.println( e + e.getMessage());
            e.printStackTrace();
        }
        System.exit( 0 );
    }
}
