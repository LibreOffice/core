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



import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
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
