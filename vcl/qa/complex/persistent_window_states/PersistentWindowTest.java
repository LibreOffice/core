/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package complex.persistent_window_states;

import com.sun.star.uno.Any;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XFrames;
import com.sun.star.container.XIndexAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.awt.Rectangle;
import com.sun.star.util.XCloseable;
import helper.ConfigurationRead;



// import org.junit.After;
import org.junit.AfterClass;
// import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * Parameters:
 * <ul>
 *   <li>NoOffice=yes - StarOffice is not started initially.</li>
 * </ul>
 */
public class PersistentWindowTest
{

    // private XMultiServiceFactory xMSF;
//    private OfficeProvider oProvider;
    private int iOfficeCloseTime = 0;

    /**
     * A frunction to tell the framework, which test functions are available.
     * Right now, it's only 'checkPersistentWindowState'.
     * @return All test methods.
     */
//    public String[] getTestMethodNames()
//    {
//        return new String[]
//                {
//                    "checkPersistentWindowState"
//                };
//    }

   /**
     * The test parameters
     */
  //  private static TestParameters param = null;

    /**
     * Test if all available document types change the
     * persistent Window Attributes
     *
     * The test follows basically these steps:
     * - Create a configuration reader and a componentloader
     * - Look for all document types in the configuration
     * - Do for every doc type
     *   - start office
     *   - read configuration attibute settings
     *   - create a new document
     *   - resize the document and close it
     *   - close office
     *   - start office
     *   - read configuration attribute settings
     *   - create another new document
     *   - compare old settings with new ones: should be different
     *   - compare the document size with the resized document: should be equal
     *   - close office
     * - Test finished
     */
    @Test public void checkPersistentWindowState()
    {
        // final XMultiServiceFactory xMsf = getMSF();

        // some Tests need the qadevOOo TestParameters, it is like a Hashmap for Properties.
//        param = new TestParameters();
//        param.put("ServiceFactory", xMsf); // some qadevOOo functions need the ServiceFactory

        try
        {

            // At first we are already connected
            // if (!connect())
            // {
            //     return;
            // }

            // fetch the multi service factory for setup
            // XMultiServiceFactory xCP = getMSF();

            // create the configuration reader
            // ConfigurationRead cfgRead = new ConfigurationRead(xCP);

            // just test the wrong ones, not all.
            String[] els = new String[]
            {
                "Office/Factories/com.sun.star.drawing.DrawingDocument",
                "Office/Factories/com.sun.star.formula.FormulaProperties",
                //"Office/Factories/com.sun.star.presentation.PresentationDocument",
                "Office/Factories/com.sun.star.sheet.SpreadsheetDocument",
                "Office/Factories/com.sun.star.text.GlobalDocument",
                "Office/Factories/com.sun.star.text.TextDocument",
                "Office/Factories/com.sun.star.text.WebDocument",
            };
            // uncomment the following line for all doc types
            // String [] els = cfgRead.getSubNodeNames("Office/Factories");

            System.out.println("Found " + els.length + " document types to test.\n");
            disconnect();

            // for all types
            for (int i = 0; i < els.length; i++)
            {
                System.out.println("\tStart test for document type " + i + ": " + els[i]);
                // exclude chart documents: cannot be created this way.
                if (els[i].indexOf("ChartDocument") != -1)
                {
                    System.out.println("Skipping chart document: cannot be create like this.");
                    continue;
                }

                // start an office
                connect();

                // get configuration
                String[] settings = getConfigurationAndLoader(getMSF(), els[i]);
                if (settings == null)
                {
                    System.out.println("Skipping document type " + els[i]);
                    disconnect();
                    continue;
                }
                String cfg = settings[1];

                // load a document
                DocumentHandle handle = loadDocument(getMSF(), settings[0]);

                // first size
                Rectangle rect1 = handle.getDocumentPosSize();

                // resize
                handle.resizeDocument();
                // after resize
                Rectangle rect2 = handle.getDocumentPosSize();

                // disposeManager and start a new office
                disconnect();

                connect();

                // get configuration
                settings = getConfigurationAndLoader(getMSF(), els[i]);

                String newCfg = settings[1];

                // load a document
                handle = loadDocument(getMSF(), settings[0]);

                Rectangle newRect = handle.getDocumentPosSize();

                // print the settings and window sizes
                System.out.println("----------------------------");
                System.out.println("Initial Config String      : " + cfg);
                System.out.println("Config String after restart: " + newCfg);

                System.out.println("----------------------------");
                System.out.println("Initial window       (X,Y,Width,Height): "
                        + rect1.X + ";" + rect1.Y + ";" + rect1.Width + ";" + rect1.Height);
                System.out.println("Window after resize  (X,Y,Width,Height): "
                        + rect2.X + ";" + rect2.Y + ";" + rect2.Width + ";" + rect2.Height);
                System.out.println("Window after restart (X,Y,Width,Height): "
                        + newRect.X + ";" + newRect.Y + ";" + newRect.Width + ";"
                        + newRect.Height);

                // compare to see if resize worked
                System.out.println("----------------------------");
                if (els[i].indexOf("SpreadsheetDocument") == -1 &&
                    els[i].indexOf("DrawingDocument") == -1)
                {
                    // leave out Spreadsheet- and DrawingDocumnt
                    assertTrue("Resize values for " + els[i] + " are equal.", !compareRectangles(rect1, rect2));
                }
                // compare settings and sizes
                assertTrue("Config settings for " + els[i] + " were not changed.", !cfg.equals(newCfg));
                assertTrue("Resized and restarted window for " + els[i] + " are not equal.", compareRectangles(rect2, newRect));
                System.out.println("----------------------------");

                // disposeManager
                disconnect();

                System.out.println("\tFinish test for document type " + i + ": " + els[i]);

            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * Get the configuration settings and the document loader
     * @param  xMSF A MultiServiceFactory from an office
     * @param cfgString A configuration string
     * @return Settings and Loader
     */
    private static String[] getConfigurationAndLoader(XMultiServiceFactory xMSF,
            String cfgString)
    {
        String[] conf = new String[2];

        try
        {
            Object o = xMSF.createInstance(
                    "com.sun.star.configuration.ConfigurationProvider");

            // fetch the multi service factory for setup
            XMultiServiceFactory xCP = UnoRuntime.queryInterface(XMultiServiceFactory.class, o);

            // create the configuration reader
            ConfigurationRead cfgRead = new ConfigurationRead(xCP);

            // get the document loader
            String loader = getStringFromObject(
                    cfgRead.getByHierarchicalName(cfgString + "/ooSetupFactoryEmptyDocumentURL"));

            if (loader == null)
            {
                return null;
            }
            System.out.println("\tLoader: " + loader);

            // read attributes
            String hierchName = cfgString + "/ooSetupFactoryWindowAttributes";
            String setupSettings = getStringFromObject(cfgRead.getByHierarchicalName(hierchName));
            // remove slots: just plain document types have to start
            if (loader.indexOf("?slot") != -1)
            {
                loader = loader.substring(0, loader.indexOf("?slot"));
                System.out.println("Loader: " + loader);
            }

            conf[0] = loader;
            conf[1] = setupSettings;
        }
        catch (com.sun.star.uno.Exception e)
        {
        }
        return conf;
    }

    /**
     * Load a document
     * @param xMSF A MultiServiceFactory from an office
     * @param docLoader A documet loader
     * @return A handle to the document
     */
    private DocumentHandle loadDocument(XMultiServiceFactory xMSF,
            String docLoader)
    {
        DocumentHandle docHandle = null;
        try
        {
            // create component loaader
            XComponentLoader xCompLoader = UnoRuntime.queryInterface(XComponentLoader.class, xMSF.createInstance("com.sun.star.frame.Desktop"));
            XFramesSupplier xFrameSupp = UnoRuntime.queryInterface(XFramesSupplier.class, xCompLoader);
            // close all existing frames
            XFrames xFrames = xFrameSupp.getFrames();
            XIndexAccess xAcc = UnoRuntime.queryInterface(XIndexAccess.class, xFrames);
            for (int i = 0; i < xAcc.getCount(); i++)
            {
                XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xAcc.getByIndex(i));
                try
                {
                    if (xClose != null)
                    {
                        xClose.close(false);
                    }
                    else
                    {
                        fail("Could not query frame for XCloseable!");
                    }
                }
                catch (com.sun.star.uno.Exception e)
                {
                    e.printStackTrace();
                    fail("Could not query frame for XCloseable!");
                }
            }
            docHandle = new DocumentHandle(xCompLoader);
            docHandle.loadDocument(docLoader, false);
        }
        catch (com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }
        catch (java.lang.Exception e)
        {
            e.printStackTrace();
        }
        return docHandle;
    }

    private boolean connect()
    {
        try
        {
            connection.setUp();
        }
        catch (java.lang.InterruptedException e)
        {
            fail("can't connect.");
        }
        catch (Exception e)
        {
            fail("can't connect.");
        }
        return true;
    }

    private boolean disconnect()
    {
        try
        {
            connection.tearDown();
        }
        catch (java.lang.InterruptedException e)
        {
            fail("can't disconnect.");
        }
        catch (Exception e)
        {
            fail("can't disconnect.");
        }
        return true;
    }

    private static String getStringFromObject(Object oName)
    {
        if (oName instanceof String)
        {
            return (String) oName;
        }
        String value = null;
        if (oName instanceof Any)
        {
            try
            {
                value = AnyConverter.toString(oName);
                if (value == null)
                {
                    System.out.println("Got a void css.uno.Any as loading string.");
                }
            }
            catch (Exception e)
            {
                System.out.println("This document type cannot be opened directly.");
            }
        }
        return value;
    }

    /**
     * Compare two rectangles. Return true, if both are equal, false
     * otherwise.
     * @param rect1 First Rectangle.
     * @param rect2 Second Rectangle.
     * @return True, if the rectangles are equal.
     */
    private boolean compareRectangles(Rectangle rect1, Rectangle rect2)
    {
        boolean result = true;
        result &= (rect1.X == rect2.X);
        result &= (rect1.Y == rect2.Y);
        result &= (rect1.Width == rect2.Width);
        result &= (rect1.Height == rect2.Height);
        return result;
    }



        private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
