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

package complex.toolkit;

// import complexlib.ComplexTestCase;
import util.SOfficeFactory;
// import complex.toolkit.CallbackClass;
import com.sun.star.awt.XRequestCallback;
import com.sun.star.lang.XMultiServiceFactory;
// import com.sun.star.lang.XComponent;
// import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
// import com.sun.star.awt.XExtendedToolkit;
// import java.io.PrintWriter;

// import org.junit.After;
import org.junit.AfterClass;
// import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 *
 */
public class CheckAsyncCallback /* extends ComplexTestCase*/  {


    XInterface testObject = null;
    XMultiServiceFactory xMSF = null;

//    public String[] getTestMethodNames() {
//        return new String[]{"checkService"};
//    }

/*    public String getTestObjectName() {
        return "com.sun.star.awt.AccessibleStatusBar";
    }
*/
    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }

    /**
     * Check services
     */
    @Test public void checkService() {
            checkAsyncCallbackService();

    }

    /**
     * Test the interfaces
     */
    public void checkAsyncCallbackService() {
        runAllInterfaceTests();
    }

    public void getTestObject() {
        try {
            xMSF = getMSF();
            SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);

            XRequestCallback xAsyncCallback = null;

            XInterface xIfc = (XInterface)xMSF.createInstance(
                                "com.sun.star.awt.AsyncCallback" );
            xAsyncCallback = UnoRuntime.queryInterface(XRequestCallback.class, xIfc);

            testObject=xAsyncCallback;
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        catch(Throwable t) {
            System.out.println("Got throwable:");
            t.printStackTrace();
        }
    }

    public void runAllInterfaceTests() {
        getTestObject();
        System.out.println("*** Now testing XRequestCallback ***");
        _XRequestCallback _xRequestCallback =
                                new _XRequestCallback(testObject, xMSF );
        assertTrue("failed: XRequestCallback::addCallback", _xRequestCallback._addCallback());
    }

    public void checkCallback() {
        getTestObject();
        System.out.println("*** Now testing asynchronous callback service ***");
        XRequestCallback xAsyncCallback = null;
        xAsyncCallback = UnoRuntime.queryInterface(XRequestCallback.class, testObject);
        CallbackClass aCallbackClass = new CallbackClass( xMSF );
        xAsyncCallback.addCallback( aCallbackClass, null );
    }



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
