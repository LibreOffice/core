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

package ifc.awt;

import com.sun.star.awt.XTabController;
import com.sun.star.awt.XUnoControlContainer;
import lib.MultiMethodTest;

/**
 * Testing <code>com.sun.star.awt.XUnoControlContainer</code>
 * interface methods :
 * <ul>
 *  <li><code> addTabController()</code></li>
 *  <li><code> removeTabController()</code></li>
 *  <li><code> getTabControllers()</code></li>
 *  <li><code> setTabControllers()</code></li>
 * </ul> <p>
 *
* This test needs the following object relations :
* <ul>
*  <li> <code>'TABCONTROL1'</code> (of type <code>XTabController</code>)</li>
*  <li> <code>'TABCONTROL2'</code> (of type <code>XTabController</code>)</li>
 *</ul>
 *
 * Test is <b> NOT </b> multithread compliant. <p>
 */

public class _XUnoControlContainer extends MultiMethodTest {
    public XUnoControlContainer oObj = null;
    private XTabController[] TabControllers = new XTabController[2];
    private XTabController tabControl1 = null;
    private XTabController tabControl2 = null;

    /**
     * This method gets the object relations.
     *
     */
    @Override
    protected void before() {

        tabControl1 = (XTabController) tEnv.getObjRelation("TABCONTROL1");
        tabControl2 = (XTabController) tEnv.getObjRelation("TABCONTROL2");
        if ((tabControl1 == null) || (tabControl2 == null)){
            log.println("ERROR: Needed object relations 'TABCONTROL1' and "
            + "'TABCONTROL2' are not found.");
        }
        TabControllers[0] = tabControl1;
        TabControllers[1] = tabControl2;

    }



    /**
    * This tests removes the object relations <code>TABCONTROL1</code> and
    * <code>TABCONTROL1</code>.
    * Has <b> OK </b> status if the sequenze of <code>XTabController[]</code>
    * get before calling method is smaller then sequenze of
    * <code>XTabController[]</code> get after calling method.<p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getTabControllers() </code> </li>
    *  <li> <code> removeTabController() </code> </li>
    * </ul>
    */
    public void _setTabControllers() {
        requiredMethod( "getTabControllers()");
        requiredMethod( "removeTabController()");

        log.println("removing TABCONTROL1 and TABCONTROL2");
        oObj.removeTabController(tabControl1);
        oObj.removeTabController(tabControl2);

        log.println("get current controllers");
        XTabController[] myTabControllers = oObj.getTabControllers();

        log.println("set new controllers");
        oObj.setTabControllers( TabControllers );

        log.println("get new current controllers");
        XTabController[] myNewTabControllers = oObj.getTabControllers();

        tRes.tested("setTabControllers()",
                    (myTabControllers.length < myNewTabControllers.length ));

    }

    /**
    * Test calls the method, then checks returned value.<p>
    * Has <b> OK </b> status if method returns a value that greater then zerro.<p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addTabController() </code></li>
    * </ul>
    */
    public void _getTabControllers() {
        requiredMethod( "addTabController()");
        XTabController[] myTabControllers = oObj.getTabControllers();
        tRes.tested("getTabControllers()", ( myTabControllers.length > 0));
    }

    /**
    * Test calls the method with object relation 'TABCONTROL1' as a parameter.<p>
    * Has <b> OK </b> status if the sequenze of <code>XTabController[]</code>
    * get before calling method is smaller then sequenze of
    * <code>XTabController[]</code> get after calling method.<p>
    */
    public void _addTabController() {
        log.println("get current controllers");
        XTabController[] myTabControllers = oObj.getTabControllers();

        log.println("add TABCONTROL1");
        oObj.addTabController( tabControl1 );

        log.println("get new current controllers");
        XTabController[] myNewTabControllers = oObj.getTabControllers();

        tRes.tested("addTabController()",
                    (myTabControllers.length < myNewTabControllers.length ));
    }

    /**
    * Test calls the method with object relation 'TABCONTROL2' as a parameter.<p>
    * Has <b> OK </b> status if the sequenze of <code>XTabController[]</code>
    * get before calling method is smaller then sequenze of
    * <code>XTabController[]</code> get after calling method.<p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getTabControllers() </code></li>
    *  <li> <code> addTabController() </code></li>
    * </ul>
    */
    public void _removeTabController() {
        requiredMethod( "getTabControllers()");
        requiredMethod( "addTabController()");

        log.println("add TABCONTROL2");
        oObj.addTabController( tabControl2 );

        log.println("get current controllers");
        XTabController[] myTabControllers = oObj.getTabControllers();

        log.println("remove TABCONTROL2");
        oObj.removeTabController(tabControl2);

        log.println("get new current controllers");
        XTabController[] myNewTabControllers = oObj.getTabControllers();

        tRes.tested("removeTabController()",
                    (myTabControllers.length > myNewTabControllers.length ));
    }

}

