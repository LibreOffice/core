/*************************************************************************
 *
 *  $RCSfile: _XTabController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:12:54 $
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

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XTabController;
import com.sun.star.awt.XTabControllerModel;

/**
* This interface is DEPRECATED !!!
* All test results are SKIPPED.OK now.
* Testing <code>com.sun.star.awt.XTabController</code>
* interface methods :
* <ul>
*  <li><code> activateFirst()</code></li>
*  <li><code> activateLast()</code></li>
*  <li><code> activateTabOrder()</code></li>
*  <li><code> autoTabOrder()</code></li>
*  <li><code> getContainer()</code></li>
*  <li><code> getCotrols()</code></li>
*  <li><code> getModel()</code></li>
*  <li><code> setContainer()</code></li>
*  <li><code> setModel()</code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'MODEL'</code> : <code>XTabControllerModel</code> a model for
*   the object</li>
*  <li> <code>'CONTAINER'</code> : <code>XControlContainer</code> a container
*   for the object</li>
* <ul> <p>
* @see com.sun.star.awt.XTabController
*/
public class _XTabController extends MultiMethodTest {
    public XTabController oObj = null;

    /**
    * Test calls the method with object relation 'MODEL' as a parameter.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setModel() {
        oObj.setModel( (XTabControllerModel) tEnv.getObjRelation("MODEL"));
        tRes.tested("setModel()", Status.skipped(true) );
    }

    /**
    * Test calls the method, then checks returned value.<p>
    * Has <b> OK </b> status if method returns a value that equals to
    * corresponding object relation.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setModel() </code> : sets model for the object </li>
    * </ul>
    */
    public void _getModel() {
        requiredMethod("setModel()");
        XTabControllerModel setModel = oObj.getModel();
        tRes.tested("getModel()", Status.skipped(true));
    }

    /**
    * Test calls the method with object relation 'CONTAINER' as a parameter.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setContainer() {
        oObj.setContainer( (XControlContainer)
            tEnv.getObjRelation("CONTAINER"));
        tRes.tested("setContainer()", Status.skipped(true));
    }

    /**
    * Test calls the method, then checks returned value.<p>
    * Has <b> OK </b> status if method returns a value that equals to
    * corresponding object relation.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setContainer() </code> : sets container for the object</li>
    * </ul>
    */
    public void _getContainer() {
        requiredMethod( "setContainer()");
        XControlContainer setContainer = oObj.getContainer();
        XControlContainer relContainer = (XControlContainer )
                                            tEnv.getObjRelation("CONTAINER");
        tRes.tested("getContainer()", Status.skipped(true) );
    }

    /**
    * Test calls the method, then checks returned sequence.<p>
    * Has <b> OK </b> status if returned sequence is not null.<p>
    */
    public void _getControls() {
        XControl[] aControl = oObj.getControls();
        tRes.tested("getControls()", Status.skipped(true) );
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _autoTabOrder() {
        oObj.autoTabOrder();
        tRes.tested("autoTabOrder()", Status.skipped(true));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _activateTabOrder() {
        oObj.activateTabOrder();
        tRes.tested("activateTabOrder()", Status.skipped(true));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _activateFirst() {
        oObj.activateFirst();
        tRes.tested("activateFirst()", Status.skipped(true));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _activateLast() {
        oObj.activateLast();
        tRes.tested("activateLast()", Status.skipped(true));
    }
}

