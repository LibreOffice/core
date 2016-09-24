/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    public static class TestInterceptor implements XDispatchProviderInterceptor {
        private final PrintWriter log;
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
    * Creates new interceptor implementation.
    */
    @Override
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

