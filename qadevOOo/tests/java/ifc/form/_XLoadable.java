/*************************************************************************
 *
 *  $RCSfile: _XLoadable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:22:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XLoadable
*/
public class _XLoadable extends MultiMethodTest {

    public XLoadable oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestLoadListener implements com.sun.star.form.XLoadListener {
        public boolean disposingCalled = false ;
        public boolean loadedCalled = false ;
        public boolean reloadedCalled = false ;
        public boolean reloadingCalled = false ;
        public boolean unloadedCalled = false ;
        public boolean unloadingCalled = false ;
        private java.io.PrintWriter log = null ;

        public TestLoadListener(java.io.PrintWriter log) {
            this.log = log ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true ;
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
            reloadingCalled = true ;
            log.println(" reloading was called.") ;
        }

        public void unloaded(com.sun.star.lang.EventObject e) {
            unloadedCalled = true ;
            log.println(" unloaded was called.") ;
        }

        public void unloading(com.sun.star.lang.EventObject e) {
            unloadingCalled = true ;
            log.println(" unloading was called.") ;
        }
    }

    TestLoadListener loadListener = null ;

    /**
    * Creates new listener.
    */
    public void before() {
        loadListener = new TestLoadListener(log) ;
    }

    /**
     * Waits for 0.1 second. Used to get time for load completion.
     */
    private void shortWait() {
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {}
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

        shortWait() ;
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

        shortWait() ;
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

        shortWait() ;
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
    * Has <b> OK </b> status if no runtime exceptions occured.
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

    protected void after() {
        disposeEnvironment();
    }
}

