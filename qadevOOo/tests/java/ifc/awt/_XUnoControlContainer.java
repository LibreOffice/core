/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XUnoControlContainer.java,v $
 * $Revision: 1.4 $
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
 * Test is <b> NOT </b> multithread compilant. <p>
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
    protected void before() {

        tabControl1 = (XTabController) tEnv.getObjRelation("TABCONTROL1");
        tabControl2 = (XTabController) tEnv.getObjRelation("TABCONTROL2");
        if ((tabControl1 == null) || (tabControl2 == null)){
            log.println("ERROR: Needed object realtions 'TABCONTROL1' and "
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

