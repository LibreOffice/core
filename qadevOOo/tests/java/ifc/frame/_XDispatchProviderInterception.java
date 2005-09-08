/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDispatchProviderInterception.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:01:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

