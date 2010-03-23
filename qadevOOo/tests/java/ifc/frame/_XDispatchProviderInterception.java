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

package ifc.frame;

import java.io.PrintWriter;

import lib.MultiMethodTest;

import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatchProviderInterception;
import com.sun.star.frame.XDispatchProviderInterceptor;
import com.sun.star.util.URL;

/**
* Testing <code>com.sun.star.frame.XDispatchProviderInterception</code>
* interface methods :
* <ul>
*  <li><code> registerDispatchProviderInterceptor()</code></li>
*  <li><code> releaseDispatchProviderInterceptor()</code></li>
* </ul> <p>
* @see com.sun.star.frame.XDispatchProviderInterception
*/
public class _XDispatchProviderInterception extends MultiMethodTest {

    public XDispatchProviderInterception oObj = null;

    /**
     * Implementation of <code>XDispatchProviderInterceptor</code> interface
     * which writes info about method calls to log, stores master and
     * slave interceptors, and redirect all dispatch queries to the master
     * provider.
     */
    public class TestInterceptor implements XDispatchProviderInterceptor {
        private PrintWriter log = null ;
        public XDispatchProvider master = null, slave = null ;

        public TestInterceptor(PrintWriter log) {
            this.log = log ;
        }

        public XDispatchProvider getSlaveDispatchProvider() {
            log.println("getSlaveDispatchProvider() called.") ;
            return slave;
        }
        public XDispatchProvider getMasterDispatchProvider() {
            log.println("getMasterDispatchProvider() called.") ;
            return master;
        }
        public void setSlaveDispatchProvider(XDispatchProvider prov) {
            log.println("setSlaveDispatchProvider(" + prov + ") called.") ;
            slave = prov ;
        }
        public void setMasterDispatchProvider(XDispatchProvider prov) {
            log.println("setMasterDispatchProvider(" + prov + ") called.") ;
            master = prov ;
        }

        public XDispatch queryDispatch(URL url, String frame, int flags) {
            log.println("my queryDispatch('" + url.Complete + "', '" +
                frame + "', " + flags + ") called.") ;
            return slave.queryDispatch(url, frame, flags) ;
        }
        public XDispatch[] queryDispatches(DispatchDescriptor[] desc) {
            log.println("my queryDispatches() called.") ;
            return slave.queryDispatches(desc) ;
        }
    }


    TestInterceptor interceptor = null ;
    /**
    * Cereates new interceptor implementation.
    */
    public void before() {
        interceptor = new TestInterceptor(log) ;
    }

    /**
    * Registers new interceptor implementation. <p>
    * Has <b> OK </b> status if during registering interceptor's
    * <code>setMasterDispatchProvider</code> method with non null
    * parameter was called. <p>
    */
    public void _registerDispatchProviderInterceptor() {

        boolean result = true ;
        oObj.registerDispatchProviderInterceptor(interceptor) ;

        result = interceptor.master != null ;

        tRes.tested("registerDispatchProviderInterceptor()", result) ;
    }

    /**
    * Releases interceptor. <p>
    * Has <b> OK </b> status if during method call interceptor's
    * <code>setMasterDispatchProvider</code> method with <code>null</code>
    * parameter was called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerDispatchProviderInterceptor </code> </li>
    * </ul>
    */
    public void _releaseDispatchProviderInterceptor() {
        requiredMethod("registerDispatchProviderInterceptor()") ;

        boolean result = true ;
        oObj.releaseDispatchProviderInterceptor(interceptor) ;

        result = interceptor.master == null ;

        tRes.tested("releaseDispatchProviderInterceptor()", result) ;
    }
}

