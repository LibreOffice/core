/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CheckAsyncCallback.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 11:42:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package complex.toolkit;

import complexlib.ComplexTestCase;
import util.SOfficeFactory;
import complex.toolkit.interface_tests._XRequestCallback;
import complex.toolkit.CallbackClass;
import com.sun.star.awt.XRequestCallback;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.XExtendedToolkit;
import java.io.PrintWriter;

/**
 *
 */
public class CheckAsyncCallback extends ComplexTestCase {


    XInterface testObject = null;
    XMultiServiceFactory xMSF = null;

    public String[] getTestMethodNames() {
        return new String[]{"checkService"};
    }

/*    public String getTestObjectName() {
        return "com.sun.star.awt.AccessibleStatusBar";
    }
*/
    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }

    /**
     * Check services
     */
    public void checkService() {
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
            xMSF = (XMultiServiceFactory)param.getMSF();
            SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);

            XRequestCallback xAsyncCallback = null;

            XInterface xIfc = (XInterface)xMSF.createInstance(
                                "com.sun.star.awt.AsyncCallback" );
            xAsyncCallback = (XRequestCallback)
                UnoRuntime.queryInterface(XRequestCallback.class,xIfc);

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
        log.println("*** Now testing XRequestCallback ***");
        _XRequestCallback _xRequestCallback =
                                new _XRequestCallback(testObject, log, xMSF );
        assure("failed: XRequestCallback::addCallback", _xRequestCallback._addCallback());
    }

    public void checkCallback() {
        getTestObject();
        log.println("*** Now testing asynchronous callback service ***");
        XRequestCallback xAsyncCallback = null;
        xAsyncCallback = (XRequestCallback)
            UnoRuntime.queryInterface( XRequestCallback.class, testObject );
        CallbackClass aCallbackClass = new CallbackClass( log, xMSF );
        xAsyncCallback.addCallback( aCallbackClass, null );
    }
}
