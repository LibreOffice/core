/*************************************************************************
 *
 *  $RCSfile: Clipboard.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:32:39 $
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
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.datatransfer.*;
import com.sun.star.datatransfer.clipboard.*;
import com.sun.star.uno.AnyConverter;

//-------------------------------------------------
// Demonstrates the usage of the clipboard service
//-------------------------------------------------

public class Clipboard
{
    public static void main(String[] args)
    {
        try
        {
            String sConnection;
            if ( args.length >= 2 )
                sConnection = "uno:" + args[0] + ";urp;StarOffice.ServiceManager" ;
            else
                sConnection = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

            /* Bootstraps a component context with the jurt base components
               registered. Component context to be granted to a component for running.
               Arbitrary values can be retrieved from the context.
            */

            XComponentContext xComponentContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

            /* Gets the service manager instance to be used (or null). This method has
               been added for convenience, because the service manager is a often used
               object.
            */

            XMultiComponentFactory xMultiComponentFactory =
                xComponentContext.getServiceManager();

            /* Creates an instance of the component UnoUrlResolver which
               supports the services specified by the factory.
            */

            Object objectUrlResolver =
                xMultiComponentFactory.createInstanceWithContext(
                    "com.sun.star.bridge.UnoUrlResolver",
                    xComponentContext );

            // Create a new url resolver

            XUnoUrlResolver xurlresolver = (XUnoUrlResolver)
                UnoRuntime.queryInterface(XUnoUrlResolver.class, objectUrlResolver);

            // Resolves an object that is specified as follow:
            // uno:<connection description>;<protocol description>;<initial object name>

            Object objectInitial = xurlresolver.resolve(sConnection);

            // Create a service manager from the initial object
            xMultiComponentFactory = (XMultiComponentFactory)
            UnoRuntime.queryInterface(XMultiComponentFactory.class, objectInitial);

            Object oClipboard =
            xMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.datatransfer.clipboard.SystemClipboard",
                xComponentContext);

            XClipboard xClipboard = (XClipboard)
                UnoRuntime.queryInterface(XClipboard.class, oClipboard);

            //---------------------------------------------------
            // registering as clipboard listener
            //---------------------------------------------------

            XClipboardNotifier xClipNotifier = (XClipboardNotifier)
                UnoRuntime.queryInterface(XClipboardNotifier.class, oClipboard);

            ClipboardListener aClipListener= new ClipboardListener();

            xClipNotifier.addClipboardListener(aClipListener);

            //---------------------------------------------------
            // get a list of formats currently on the clipboard
            //---------------------------------------------------

            XTransferable xTransferable = xClipboard.getContents();

            DataFlavor[] aDflvArr = xTransferable.getTransferDataFlavors();

            // print all available formats

            System.out.println("Reading the clipboard...");
            System.out.println("Available clipboard formats:");

            DataFlavor aUniFlv = null;

            for (int i=0;i<aDflvArr.length;i++)
            {
                System.out.println( "MimeType: " +
                                    aDflvArr[i].MimeType +
                                    " HumanPresentableName: " +
                                    aDflvArr[i].HumanPresentableName );

                // if there is the format unicode text on the clipboard save the
                // corresponding DataFlavor so that we can later output the string

                if ( aDflvArr[i].MimeType.equals("text/plain;charset=utf-16") )
                {
                    aUniFlv = aDflvArr[i];
                }
            }

            System.out.println("");

            try
            {
                if (aUniFlv != null)
                {
                    System.out.println("Unicode text on the clipboard...");
                    Object aData = xTransferable.getTransferData(aUniFlv);
                    System.out.println(AnyConverter.toString(aData));
                }
            }
            catch( UnsupportedFlavorException ex )
            {
                System.err.println( "Requested format is not available on the clipboard!" );
            }

            //---------------------------------------------------
            // becoming a clipboard owner
            //---------------------------------------------------

            System.out.println("Becoming a clipboard owner...");
            System.out.println("");

            ClipboardOwner aClipOwner = new ClipboardOwner();
            xClipboard.setContents(new TextTransferable("Hello World!"), aClipOwner);
            int iFirst = 0;

            while (aClipOwner.isClipboardOwner())
            {
                if (iFirst != 2) {
                    if (iFirst == 1) {
                        System.out.println("Change clipboard ownership by putting something into the clipboard!\n");
                        System.out.print("Still clipboard owner...");
                    } else {
                        System.out.println("Still clipboard owner...");
                    }
                    ++iFirst;
                } else {
                    System.out.print(".");
                }
                Thread.sleep(1000);
            }

            //---------------------------------------------------
            // unregistering as clipboard listener
            //---------------------------------------------------

            xClipNotifier.removeClipboardListener(aClipListener);

            System.exit(0);
        }
        catch( Exception exception )
        {
            System.err.println(exception);
        }
    }
}
