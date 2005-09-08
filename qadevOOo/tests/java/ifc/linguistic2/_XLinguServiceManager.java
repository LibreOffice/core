/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLinguServiceManager.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:18:46 $
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

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.LinguServiceEvent;
import com.sun.star.linguistic2.XHyphenator;
import com.sun.star.linguistic2.XLinguServiceEventListener;
import com.sun.star.linguistic2.XLinguServiceManager;
import com.sun.star.linguistic2.XSpellChecker;
import com.sun.star.linguistic2.XThesaurus;

/**
*Testing <code>com.sun.star.linguistic2.XLinguServiceManager</code>
* interface methods:
* <ul>
*   <li><code>getSpellChecker()</code></li>
*   <li><code>getHyphenator()</code></li>
*   <li><code>getThesaurus()</code></li>
*   <li><code>addLinguServiceManagerListener()</code></li>
*   <li><code>removeLinguServiceManagerListener()</code></li>
*   <li><code>getAvailableServices()</code></li>
*   <li><code>setConfiguredServices()</code></li>
*   <li><code>getConfiguredServices()</code></li>
* </ul> <p>
*@see com.sun.star.linguistic2.XLinguServiceManager
*/
public class _XLinguServiceManager extends MultiMethodTest {

    public XLinguServiceManager oObj = null;
    boolean listenerCalled = false;

    /**
    * Class implements interface <code>XLinguServiceEventListener</code>
    * for test method <code>addLinguServiceManagerListener</code>.
    * @see com.sun.star.linguistic2.XLinguServiceEventListener
    */
    public class MyLinguServiceEventListener implements
            XLinguServiceEventListener {
        public void disposing ( EventObject oEvent ) {
            log.println("Listener has been disposed");
        }
        public void processLinguServiceEvent(LinguServiceEvent aServiceEvent) {
            listenerCalled = true;
            log.println("Listener called");
        }
    };

    XLinguServiceEventListener listener = new MyLinguServiceEventListener();

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSpellChecker() {
        XSpellChecker SC = oObj.getSpellChecker();
        tRes.tested("getSpellChecker()", SC != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getHyphenator() {
        XHyphenator HN = oObj.getHyphenator();
        tRes.tested("getHyphenator()", HN != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getThesaurus() {
        XThesaurus TS = oObj.getThesaurus();
        tRes.tested("getThesaurus()", TS != null);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to true. <p>
    */
    public void _addLinguServiceManagerListener() {
        boolean res = oObj.addLinguServiceManagerListener(listener);
        tRes.tested("addLinguServiceManagerListener()", res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to true. <p>
    */
    public void _removeLinguServiceManagerListener() {
        boolean res = oObj.removeLinguServiceManagerListener(listener);
        tRes.tested("removeLinguServiceManagerListener()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if length of returned array is
    * greater than zero.<p>
    */
    public void _getAvailableServices() {
        String[] services = oObj.getAvailableServices(
            "com.sun.star.linguistic2.Hyphenator",
            new Locale("en", "US", "") );
        tRes.tested("getAvailableServices()", services.length > 0);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if length of returned array is
    * greater than zero.<p>
    */
    public void _getConfiguredServices() {
        String[] services = oObj.getConfiguredServices(
            "com.sun.star.linguistic2.Hyphenator",
            new Locale("en", "US", "") );
        tRes.tested("getConfiguredServices()", services.length > 0);
    }

    /**
    * Test sets empty list of service, checks value returned
    * by method <code>getConfiguredServices()</code> and all services
    * restored finally. <p>
    * Has <b> OK </b> status if length of obtained service list equal to zero.
    * <p>The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getConfiguredServices() </code></li>
    * </ul>
    */
    public void _setConfiguredServices() {
        requiredMethod("getConfiguredServices()");

        String[] services = oObj.getConfiguredServices(
            "com.sun.star.linguistic2.Hyphenator",new Locale("en","US",""));

        String[] empty = new String[0];
        oObj.setConfiguredServices(
            "com.sun.star.linguistic2.Hyphenator",
            new Locale("en", "US", ""),
            empty );

        String[] get = oObj.getConfiguredServices(
            "com.sun.star.linguistic2.Hyphenator", new Locale("en","US",""));

        boolean res = (get.length == 0);

        oObj.setConfiguredServices(
            "com.sun.star.linguistic2.Hyphenator",
            new Locale("en", "US", ""),
            services );

        tRes.tested("setConfiguredServices()", res);
    }

}  // finish class _XLinguServiceManager


