/*************************************************************************
 *
 *  $RCSfile: _XController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:38:00 $
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

package ifc.frame;

import lib.MultiMethodTest;

import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.util.XModifiable;


/**
* Testing <code>com.sun.star.frame.XController</code>
* interface methods:
* <ul>
*  <li><code> getFrame() </code></li>
*  <li><code> attachFrame() </code></li>
*  <li><code> getModel() </code></li>
*  <li><code> attachModel() </code></li>
*  <li><code> getViewData() </code></li>
*  <li><code> restoreViewData() </code></li>
*  <li><code> suspend() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Frame'</code> (of type <code>XFrame</code>):
*   any other frame, used for tests</li>
*  <li> <code>'FirstModel'</code> (of type <code>XModel</code>):
*   model of a controller tested</li>
*  <li> <code>'SecondModel'</code> (of type <code>XModel</code>):
*   other model, used for tests </li>
*  <li> <code>'HasViewData'</code> (of type <code>Boolean</code>):
*   (optional relation) if it exsists, so controller has no view data</li>
*  <li> <code>'SecondController'</code> (of type <code>XController</code>):
*   other controller, used for tests </li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XController
*/
public class _XController extends MultiMethodTest {
    public XController oObj = null;
    public XModel firstModel = null;
    public XModel secondModel = null;
    public XFrame frame = null;
    public Object ViewData = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns object, that's equal to
    * previously obtained object relation 'Frame'.
    * The following method tests are to be completed successfully before:
    * <ul>
    *  <li> <code> attachFrame() </code> : attachs frame obtained object
    * relation 'Frame' </li>
    * </ul>
    */
    public void _getFrame() {
        requiredMethod("attachFrame()");
        XFrame getting = oObj.getFrame();
        boolean eq = getting.equals(frame);
        if ( !eq ) {
            log.println("Getting: " + getting.toString());
            log.println("Expected: " + frame.toString());
        }
        tRes.tested("getFrame()", eq);
    }

    /**
    * After obtaining a corresponding object relation test calls the method.
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _attachFrame() {
        frame = (XFrame) tEnv.getObjRelation("Frame");
        oObj.attachFrame(frame);
        tRes.tested("attachFrame()", true);
    }

    /**
    * At first object relation 'FirstModel' is gotten. Then test calls the
    * method. <p>
    * Has <b> OK </b> status if string repersentation of an object, returned by
    * the method is equal to string representation of corresponding object
    * relation.
    */
    public void _getModel() {
        firstModel = (XModel) tEnv.getObjRelation("FirstModel");
        XModel getting = oObj.getModel();
        String out1 = "";
        String out2 = "";
        if ( (firstModel == null) ) out1="none";
        else out1 = firstModel.toString();
        if ( (getting == null) ) out2="none"; else out2 = getting.toString();
        boolean eq = out1.equals(out2);
        if ( !eq ) {
            log.println("Getting: " + out2);
            log.println("Expected: " + out1);
        }
        tRes.tested("getModel()", eq);
    }

    /**
    * At first, we obtain an object relation 'SecondModel'. Then test calls
    * the method and check result. <p>
    * Has <b> OK </b> status if method returns true and attached model is
    * equal to a model 'SecondModel' obtained before.
    * <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getModel() </code> : returns model (XModel) of the
    *  XController object</li>
    * </ul>
    */
    public void _attachModel() {
        boolean result = false;

        requiredMethod("getModel()");
        secondModel = (XModel) tEnv.getObjRelation("SecondModel");
        XModel gotBefore = oObj.getModel();
        boolean attached = oObj.attachModel(secondModel);
        XModel gotAfter = oObj.getModel();
        if ( attached ) {
            if ( ! gotBefore.equals(gotAfter) ) {
                if ( gotAfter.equals(secondModel) ) {
                    result = true;
                } else {
                    log.println("Attached and gotten models are not equal");
                    log.println("Getting: " + gotAfter.toString());
                    log.println("Expected: " + secondModel.toString());
                }
            } else {
                log.println("method did not change model");
            }
        } else {
            result=true;
            log.println("attachModel() returns false");
            log.println("as expected, see #82938");
        }
        tRes.tested("attachModel()", result);
        oObj.attachModel(firstModel);
    }

    /**
    * At first gotten object relation 'HasViewData' is checked. Then if
    * 'HasViewData' is null, test calls the method. <p>
    * Has <b> OK </b> status if obtained object relation is not null, or if
    * the method does not return null.
    */
    public void _getViewData() {
        if (tEnv.getObjRelation("HasViewData") != null) {
            log.println("This Object has no View Data");
            tRes.tested("getViewData()", true);
            return;
        }
        ViewData = oObj.getViewData();
        tRes.tested( "getViewData()", ViewData != null );
    }

    /**
    * If obtained object relation 'HasViewData' is null, test calls the method.
    * <p>Has <b> OK </b> status if obtained object relation is not null, or
    * if no exceptions were thrown while method call.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getViewData() </code> : gets view data of an object. </li>
    * </ul>
    */
    public void _restoreViewData() {
        requiredMethod("getViewData()");
        if (tEnv.getObjRelation("HasViewData") != null) {
            log.println("This Object has no View Data");
            tRes.tested("restoreViewData()", true);
            return;
        }
        oObj.restoreViewData(ViewData);
        tRes.tested( "restoreViewData()", true );
    }

    /**
    * Has <b> OK </b> status if the method returns true.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> restoreViewData() </code> : restores view status of an
    *  object </li>
    * </ul>
    */
    public void _suspend() {
        requiredMethod("restoreViewData()");
        XModifiable modify = (XModifiable) tEnv.getObjRelation("Modifiable");
        if (modify != null) {
            try {
                modify.setModified(false);
            } catch (com.sun.star.beans.PropertyVetoException pve) {
                log.println("PropertyVetoException, couldn't change Modify flag");
            }
        }
        tRes.tested( "suspend()", oObj.suspend(true) );
    }

}

