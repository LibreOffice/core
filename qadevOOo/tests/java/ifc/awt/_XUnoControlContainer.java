/*************************************************************************
 *
 *  $RCSfile: _XUnoControlContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:20:11 $
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

package ifc.awt;

import com.sun.star.awt.XTabController;
import com.sun.star.awt.XUnoControlContainer;
import lib.MultiMethodTest;
import lib.Status;
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

