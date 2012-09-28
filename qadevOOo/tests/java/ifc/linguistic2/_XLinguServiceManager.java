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
    }

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


