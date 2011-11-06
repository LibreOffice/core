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



package complex.toolkit;

import com.sun.star.awt.XUnitConversion;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.awt.XWindow;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.awt.XWindowPeer;

import util.DesktopTools;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * This complex test is only for testing the com.sun.star.awt.XUnitConversion methods
 * These are converter methods to get the size of a well known awt component
 * in a com.sun.star.util.MeasureUnit you want.
 * You don't need to know the factors to calculate by hand.
 *
 * @author ll93751
 */
public class UnitConversion
{
    /**
     * returns the delta value between a and b
     * @param a
     * @param b
     * @return
     */
    private int delta(int a, int b)
    {
        final int n = Math.abs(a - b);
        return n;
    }

    private XUnitConversion m_xConversion = null;

    /**
     * Not really a check,
     * only a simple test call to convertSizeToLogic(...) with different parameters
     * @param _aSize
     * @param _aMeasureUnit
     * @param _sEinheit
     */
    private void checkSize(com.sun.star.awt.Size _aSize, short _aMeasureUnit, String _sEinheit)
    {
        try
        {
            com.sun.star.awt.Size aSizeIn = m_xConversion.convertSizeToLogic(_aSize, _aMeasureUnit);
            System.out.println("Window size:");
            System.out.println("Width:" + aSizeIn.Width + " " + _sEinheit);
            System.out.println("Height:" + aSizeIn.Height + " " + _sEinheit);
            System.out.println("");
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            System.out.println("Caught IllegalArgumentException in convertSizeToLogic with '" + _sEinheit + "' " + e.getMessage());
        }
    }

    /**
     * The real test function
     * 1. try to get the XMultiServiceFactory of an already running office. Therefore make sure an (open|star)office is running with
     *    parameters like -accept="socket,host=localhost,port=8100;urp;"
     * 2. try to create an empty window
     * 3. try to convert the WindowPeer to an XWindow
     * 4. try to resize and move the window to an other position, so we get a well knowing position and size.
     * 5. run some more tests
     *
     * If no test fails, the test is well done and returns with 'PASSED, OK'
     *
     */
    @Test
    public void testXUnitConversion()
    {
        final XMultiServiceFactory xMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());

        assertNotNull("failed: There is no office.", xMSF);

        // create a window
        XWindowPeer xWindowPeer = DesktopTools.createFloatingWindow(xMSF);
        assertNotNull("failed: there is no window peer", xWindowPeer);


        // resize and move the window to a well known position and size
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xWindowPeer);
        assertNotNull("failed: there is no window, cast wrong?", xWindow);

        xWindow.setVisible(Boolean.TRUE);

        int x = 100;
        int y = 100;
        int width = 640;
        int height = 480;
        xWindow.setPosSize(x, y, width, height, com.sun.star.awt.PosSize.POSSIZE);

        com.sun.star.awt.Rectangle aRect = xWindow.getPosSize();
        com.sun.star.awt.Point aPoint = new com.sun.star.awt.Point(aRect.X, aRect.Y);
        com.sun.star.awt.Size aSize = new com.sun.star.awt.Size(aRect.Width, aRect.Height);

        System.out.println("Window position and size in pixel:");
        System.out.println("X:" + aPoint.X);
        System.out.println("Y:" + aPoint.Y);
        System.out.println("Width:" + aSize.Width);
        System.out.println("Height:" + aSize.Height);
        System.out.println("");

        assertTrue("Window pos size wrong", aSize.Width == width && aSize.Height == height && aPoint.X == x && aPoint.Y == y);

        m_xConversion = UnoRuntime.queryInterface(XUnitConversion.class, xWindowPeer);

        // try to get the position of the window in 1/100mm with the XUnitConversion method
        try
        {
            com.sun.star.awt.Point aPointInMM_100TH = m_xConversion.convertPointToLogic(aPoint, com.sun.star.util.MeasureUnit.MM_100TH);
            System.out.println("Window position:");
            System.out.println("X:" + aPointInMM_100TH.X + " 1/100mm");
            System.out.println("Y:" + aPointInMM_100TH.Y + " 1/100mm");
            System.out.println("");
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            fail("failed: IllegalArgumentException caught in convertPointToLogic " + e.getMessage());
        }

        // try to get the size of the window in 1/100mm with the XUnitConversion method
        com.sun.star.awt.Size aSizeInMM_100TH = null;
        com.sun.star.awt.Size aSizeInMM_10TH = null;
        try
        {
            aSizeInMM_100TH = m_xConversion.convertSizeToLogic(aSize, com.sun.star.util.MeasureUnit.MM_100TH);
            System.out.println("Window size:");
            System.out.println("Width:" + aSizeInMM_100TH.Width + " 1/100mm");
            System.out.println("Height:" + aSizeInMM_100TH.Height + " 1/100mm");
            System.out.println("");

            // try to get the size of the window in 1/10mm with the XUnitConversion method

            aSizeInMM_10TH = m_xConversion.convertSizeToLogic(aSize, com.sun.star.util.MeasureUnit.MM_10TH);
            System.out.println("Window size:");
            System.out.println("Width:" + aSizeInMM_10TH.Width + " 1/10mm");
            System.out.println("Height:" + aSizeInMM_10TH.Height + " 1/10mm");
            System.out.println("");

            // check the size with a delta which must be smaller a given difference
            assertTrue("Size.Width  not correct", delta(aSizeInMM_100TH.Width, aSizeInMM_10TH.Width * 10) < 10);
            assertTrue("Size.Height not correct", delta(aSizeInMM_100TH.Height, aSizeInMM_10TH.Height * 10) < 10);

            // new
            checkSize(aSize, com.sun.star.util.MeasureUnit.PIXEL, "pixel");
            checkSize(aSize, com.sun.star.util.MeasureUnit.APPFONT, "appfont");
            checkSize(aSize, com.sun.star.util.MeasureUnit.SYSFONT, "sysfont");

            // simply check some more parameters
            checkSize(aSize, com.sun.star.util.MeasureUnit.MM, "mm");
            checkSize(aSize, com.sun.star.util.MeasureUnit.CM, "cm");
            checkSize(aSize, com.sun.star.util.MeasureUnit.INCH_1000TH, "1/1000inch");
            checkSize(aSize, com.sun.star.util.MeasureUnit.INCH_100TH, "1/100inch");
            checkSize(aSize, com.sun.star.util.MeasureUnit.INCH_10TH, "1/10inch");
            checkSize(aSize, com.sun.star.util.MeasureUnit.INCH, "inch");
            // checkSize(aSize, com.sun.star.util.MeasureUnit.M, "m");
            checkSize(aSize, com.sun.star.util.MeasureUnit.POINT, "point");
            checkSize(aSize, com.sun.star.util.MeasureUnit.TWIP, "twip");
            // checkSize(aSize, com.sun.star.util.MeasureUnit.KM, "km");
            // checkSize(aSize, com.sun.star.util.MeasureUnit.PICA, "pica");
            // checkSize(aSize, com.sun.star.util.MeasureUnit.FOOT, "foot");
            // checkSize(aSize, com.sun.star.util.MeasureUnit.MILE, "mile");
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            fail("failed: IllegalArgumentException caught in convertSizeToLogic " + e.getMessage());
        }

        // convert the 1/100mm window size back to pixel
        try
        {
            com.sun.star.awt.Size aNewSize = m_xConversion.convertSizeToPixel(aSizeInMM_100TH, com.sun.star.util.MeasureUnit.MM_100TH);
            System.out.println("Window size:");
            System.out.println("Width:" + aNewSize.Width + " pixel");
            System.out.println("Height:" + aNewSize.Height + " pixel");

            // assure the pixels are the same as we already know
            assertTrue("failed: Size from pixel to 1/100mm to pixel", aSize.Width == aNewSize.Width && aSize.Height == aNewSize.Height);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            fail("failed: IllegalArgumentException caught in convertSizeToPixel " + e.getMessage());
        }

        // close the window.
        // IMHO a little bit stupid, but the XWindow doesn't support a XCloseable interface
        xWindow.dispose();
    }

    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println( "--------------------------------------------------------------------------------" );
        System.out.println( "starting class: " + UnitConversion.class.getName() );
        System.out.println( "connecting ..." );
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println();
        System.out.println( "tearing down connection" );
        connection.tearDown();
        System.out.println( "finished class: " + UnitConversion.class.getName() );
        System.out.println( "--------------------------------------------------------------------------------" );
    }

    private static final OfficeConnection connection = new OfficeConnection();
}
