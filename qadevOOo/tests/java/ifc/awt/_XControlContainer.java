/*************************************************************************
 *
 *  $RCSfile: _XControlContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:10:30 $
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

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;

/**
* Testing <code>com.sun.star.awt.XControlContainer</code>
* interface methods:
* <ul>
*   <li><code> setStatusText() </code></li>
*   <li><code> addControl() </code></li>
*   <li><code> removeControl() </code></li>
*   <li><code> getControl() </code></li>
*   <li><code> getControls() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CONTROL1'</code> (of type <code>XControl</code>):
*  used as a parameter to addControl(), getControl() and removeControl()</li>
*  <li> <code>'CONTROL2'</code> (of type <code>XControl</code>):
*  used as a parameter to addControl(), getControl() and removeControl()</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XControlContainer
*/
public class _XControlContainer extends MultiMethodTest {
    public XControlContainer oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _setStatusText() {
        oObj.setStatusText("testing XControlContainer::setStatusText(String)");
        tRes.tested("setStatusText()",true);
    }

    /**
    * Test calls the method twice - two controls gotten from object relations
    * 'CONTROL1' and 'CONTROL2' added to container.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _addControl() {
        oObj.addControl("CONTROL1", (XControl)tEnv.getObjRelation("CONTROL1"));
        oObj.addControl("CONTROL2", (XControl)tEnv.getObjRelation("CONTROL2"));
        tRes.tested("addControl()",true);
    }

    /**
    * Test calls the method with object relation 'CONTROL1' as a
    * parameter. Then control gotten from container is checked, and if returned
    * value is null then another control 'CONTROL2' is removed from container,
    * otherwise returned value of method test is 'false'.<p>
    * Has <b> OK </b> status if control is removed successfully.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addControl() </code> : adds control to a container </li>
    *  <li> <code> getControl() </code> : gets control from container </li>
    *  <li> <code> getControls() </code> : gets controls from container</li>
    * </ul>
    */
    public void _removeControl() {
        boolean result = true;

        requiredMethod("addControl()");
        requiredMethod("getControl()");
        requiredMethod("getControls()");
        oObj.removeControl( (XControl) tEnv.getObjRelation("CONTROL1") );
        XControl ctrl = oObj.getControl("CONTROL1");
        if (ctrl != null) {
            result = false;
            log.println("'removeControl()' fails; Control still exists");
        } else {
            oObj.removeControl( (XControl) tEnv.getObjRelation("CONTROL2") );
        }
        tRes.tested("removeControl()", result);
    }

    /**
    * Test calls the method with 'CONTROL1' as a parameter, then we just
    * compare returned object and object relation 'CONTROL1'.<p>
    * Has <b> OK </b> status if value returned by the method is equal to
    * a corresponding object relation.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addControl() </code> : adds control to a container </li>
    * </ul>
    */
    public void _getControl() {
        requiredMethod("addControl()");
        XControl xCtrlComp = oObj.getControl("CONTROL1");
        XControl xCl = (XControl) tEnv.getObjRelation("CONTROL1");
        tRes.tested("getControl()", xCtrlComp.equals(xCl));
    }

    /**
    * Test calls the method, then returned value is checked.<p>
    * Has <b> OK </b> status if returned array consists of at least two
    * elements.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addControl() </code> : adds control to a container </li>
    * </ul>
    */
    public void _getControls() {
        requiredMethod("addControl()");
        XControl[] xCtrls = oObj.getControls();
        tRes.tested("getControls()",xCtrls.length >= 2);
    }
}

