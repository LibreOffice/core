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

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.UnsupportedFlavorException;
import com.sun.star.datatransfer.XTransferable;
import com.sun.star.datatransfer.clipboard.XClipboard;
import com.sun.star.datatransfer.clipboard.XClipboardNotifier;
import com.sun.star.text.XTextDocument;
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
            // get the remote office context. If necessary a new office
            // process is started
            XComponentContext xOfficeContext =
                com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            // get the service manager from the office context
            XMultiComponentFactory xServiceManager =
                xOfficeContext.getServiceManager();

            // create a new test document
            Object oDesktop = xServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xOfficeContext);

            XComponentLoader xCompLoader =(XComponentLoader)
                UnoRuntime.queryInterface(XComponentLoader.class, oDesktop);

            com.sun.star.lang.XComponent xComponent =
                xCompLoader.loadComponentFromURL("private:factory/swriter",
                    "_blank", 0, new com.sun.star.beans.PropertyValue[0]);
            {
            XTextDocument xDoc =(XTextDocument)
                UnoRuntime.queryInterface(XTextDocument.class, xComponent);
            xDoc.getText().setString("In the first step, paste the current content of the clipboard in the document!\nThe text \"Hello world!\" shall be insert at the current cursor position below.\n\nIn the second step, please select some words and put it into the clipboard! ...\n\nCurrent clipboard content = ");

            // ensure that the document content is optimal visible
            com.sun.star.frame.XModel xModel =
                (com.sun.star.frame.XModel)UnoRuntime.queryInterface(
                    com.sun.star.frame.XModel.class, xDoc);
            // get the frame for later usage
            com.sun.star.frame.XFrame xFrame =
                xModel.getCurrentController().getFrame();

            com.sun.star.view.XViewSettingsSupplier xViewSettings =
                (com.sun.star.view.XViewSettingsSupplier)UnoRuntime.queryInterface(
                    com.sun.star.view.XViewSettingsSupplier.class,
                    xModel.getCurrentController());
            xViewSettings.getViewSettings().setPropertyValue(
                "ZoomType", new Short((short)0));
            }
            // test document will be closed later

            Object oClipboard = xServiceManager.createInstanceWithContext(
                "com.sun.star.datatransfer.clipboard.SystemClipboard",
                xOfficeContext);

            XClipboard xClipboard = (XClipboard)
                UnoRuntime.queryInterface(XClipboard.class, oClipboard);

            //---------------------------------------------------
            // registering as clipboard listener
            //---------------------------------------------------

            XClipboardNotifier xClipNotifier = (XClipboardNotifier)
                UnoRuntime.queryInterface(XClipboardNotifier.class, oClipboard);

            ClipboardListener aClipListener= new ClipboardListener();

            xClipNotifier.addClipboardListener(aClipListener);

            // Read ClipBoard
            readClipBoard(xClipboard);

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

            // Read ClipBoard again
            readClipBoard(xClipboard);

            //---------------------------------------------------
            // unregistering as clipboard listener
            //---------------------------------------------------
            xClipNotifier.removeClipboardListener(aClipListener);

            // close test document
            com.sun.star.util.XCloseable xCloseable = (com.sun.star.util.XCloseable)
                UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class,
                                          xComponent );

            if (xCloseable != null ) {
                xCloseable.close(false);
            } else
            {
                xComponent.dispose();
            }

            System.exit(0);
        }
        catch( Exception ex )
        {
            ex.printStackTrace();
        }
    }

    public static void readClipBoard(XClipboard xClipboard)
        throws java.lang.Exception
    {
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
                System.out.print("Unicode text on the clipboard ...\nYour selected text \"");
                Object aData = xTransferable.getTransferData(aUniFlv);
                System.out.println(AnyConverter.toString(aData)
                                   + "\" is now in the clipboard.\n");
            }
        }
        catch( UnsupportedFlavorException ex )
        {
            System.err.println( "Requested format is not available on the clipboard!" );
            ex.printStackTrace();
        }
    }
}
