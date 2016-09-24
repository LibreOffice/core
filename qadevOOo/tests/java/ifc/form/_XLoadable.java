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

package ifc.form;


import lib.MultiMethodTest;

import com.sun.star.form.XLoadable;

/**
* Testing <code>com.sun.star.form.XLoadable</code>
* interface methods :
* <ul>
*  <li><code> load()</code></li>
*  <li><code> unload()</code></li>
*  <li><code> reload()</code></li>
*  <li><code> isLoaded()</code></li>
*  <li><code> addLoadListener()</code></li>
*  <li><code> removeLoadListener()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.form.XLoadable
*/
public class _XLoadable extends MultiMethodTest {

    public XLoadable oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestLoadListener implements com.sun.star.form.XLoadListener {
        public boolean loadedCalled = false ;
        public boolean reloadedCalled = false ;
        public boolean unloadedCalled = false ;
        private final java.io.PrintWriter log;

        public TestLoadListener(java.io.PrintWriter log) {
            this.log = log ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {
            log.println(" disposing was called.") ;
        }

        public void loaded(com.sun.star.lang.EventObject e) {
            loadedCalled = true ;
            log.println(" loaded was called.") ;
        }

        public void reloaded(com.sun.star.lang.EventObject e) {
            reloadedCalled = true ;
            log.println(" reloaded was called.") ;
        }

        public void reloading(com.sun.star.lang.EventObject e) {
            log.println(" reloading was called.") ;
        }

        public void unloaded(com.sun.star.lang.EventObject e) {
            unloadedCalled = true ;
            log.println(" unloaded was called.") ;
        }

        public void unloading(com.sun.star.lang.EventObject e) {
            log.println(" unloading was called.") ;
        }
    }

    TestLoadListener loadListener = null ;

    /**
    * Creates new listener.
    */
    @Override
    public void before() {
        loadListener = new TestLoadListener(log) ;
    }

    /**
    * Loads the form. <p>
    * Has <b> OK </b> status if <code>isLoaded()</code> returns
    * <code>true</code> and listener method <code>loaded()</code>
    * is called.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> isLoaded() </code> : to be sure form is not loaded </li>
    *  <li> <code> addLoadListener() </code> : to check if this listener method
    *  is called. </li>
    * </ul>
    */
    public void _load() {
        requiredMethod("isLoaded()") ;
        requiredMethod("addLoadListener()") ;

        boolean result = true ;
        oObj.load() ;

        waitForEventIdle();
        result = oObj.isLoaded() && loadListener.loadedCalled ;

        tRes.tested("load()", result) ;
    }

    /**
    * Unloads the form. <p>
    * Has <b> OK </b> status if <code>isLoaded()</code> returns
    * <code>false</code> and listener method <code>unloaded()</code>
    * is called.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> reload() </code> : to be sure the form is loaded </li>
    *  <li> <code> addLoadListener() </code> : to check if this listener method
    *  is called. </li>
    * </ul>
    */
    public void _unload() {
        requiredMethod("reload()") ;
        requiredMethod("addLoadListener()") ;

        boolean result = true ;
        oObj.unload() ;

        waitForEventIdle();
        result = !oObj.isLoaded() && loadListener.unloadedCalled ;

        tRes.tested("unload()", result) ;
    }

    /**
    * Reloads the form. <p>
    * Has <b> OK </b> status if <code>isLoaded()</code> returns
    * <code>true</code> and listener method <code>reloaded()</code>
    * is called.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> load() </code> : to be sure form is loaded </li>
    *  <li> <code> addLoadListener() </code> : to check if this listener method
    *  is called. </li>
    * </ul>
    */
    public void _reload() {
        requiredMethod("load()") ;
        requiredMethod("addLoadListener()") ;

        boolean result = true ;
        oObj.reload() ;

        waitForEventIdle();
        result = oObj.isLoaded() && loadListener.reloadedCalled;

        tRes.tested("reload()", result) ;
    }

    /**
    * Checks if the component is already loaded. If yes it unloads
    * it <p>
    * Has <b> OK </b> status if finally <code>isLoaded()</code> method
    * returns <code>false</code>.
    */
    public void _isLoaded() {

        boolean isLoaded = oObj.isLoaded() ;
        if (isLoaded) oObj.unload();
        isLoaded = oObj.isLoaded() ;

        tRes.tested("isLoaded()", !isLoaded) ;
    }

    /**
    * Adds a listener. If its methods are called or not is checked
    * in other object methods. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _addLoadListener() {

        boolean result = true ;
        oObj.addLoadListener(loadListener) ;

        tRes.tested("addLoadListener()", result) ;
    }

    /**
    * Removes the listener added before. <p>
    * Has <b> OK </b> status if after <code>load()</code> call no
    * listener methods were called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> unload() </code> : to make this test run finally.</li>
    * </ul>
    */
    public void _removeLoadListener() {
        requiredMethod("unload()") ;

        boolean result = true ;
        oObj.removeLoadListener(loadListener) ;
        loadListener.loadedCalled = false ;
        oObj.load();

        result = ! loadListener.loadedCalled ;

        tRes.tested("removeLoadListener()", result) ;
    }

    @Override
    protected void after() {
        disposeEnvironment();
    }
}

