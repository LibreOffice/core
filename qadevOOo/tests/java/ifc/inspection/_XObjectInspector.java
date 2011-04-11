/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package ifc.inspection;

import com.sun.star.inspection.XObjectInspector;
import com.sun.star.inspection.XObjectInspectorModel;
import com.sun.star.inspection.XObjectInspectorUI;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


/**
 * Testing <code>com.sun.star.inspection.XObjectInspector</code>
 * interface methods :
 * <ul>
 *  <li><code> inspect()</code></li>
 *  <li><code> InspectorModel()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XObjectInspector.toInspect'</code>
 *  (of type <code>Object []</code>):
 *   acceptable collection of one or more objects which can be inspected by <code>inspect()</code> </li>
 * <ul> <p>
 *
 */

public class _XObjectInspector extends MultiMethodTest {

    /**
     * the test object
     */
    public XObjectInspector oObj = null;
    /**
     * This variable was filled with the object relation
     * <CODE>XObjectInspector.toInspect</CODE> and was used to
     * test the method <CODE>inspect()</CODE>
     */
    public Object[] oInspect = null;
    /**
     * This variable was filled with the object relation
     * <CODE>XObjectInspector.InspectorModelToSet</CODE> and was used to
     * test the method <CODE>setInspectorModel()</CODE>
     */
    public XObjectInspectorModel xSetModel = null;

    /**
     * get object relations
     * <ul>
     *   <li>XObjectInspector.toInspect</li>
     * </ul>
     */
    public void before() {

        oInspect = (Object[]) tEnv.getObjRelation("XObjectInspector.toInspect");

        if (oInspect == null) throw new StatusException
                (Status.failed("Relation 'XObjectInspector.toInspect' not found.")) ;

        xSetModel = (XObjectInspectorModel) tEnv.getObjRelation("XObjectInspector.InspectorModelToSet");

        if (xSetModel == null) throw new StatusException
                (Status.failed("Relation 'XObjectInspector.InspectorModelToSet' not found.")) ;
    }

    /**
     * Inspects a new collection of one or more objects given by object realtion
     * <CODE>XObjectInspector.toInspect</CODE><br>
     * Has <b>OK</b> status if no runtime exceptions occurred.
     */
    public void _inspect() {

        boolean result = true;

        try {
            oObj.inspect(oInspect);

        } catch (com.sun.star.util.VetoException e){
            log.println("ERROR:" + e.toString());
            result = false;
        }

        tRes.tested("inspect()", result) ;
    }

    /**
     * First call the method <CODE>getInspectorModel()</CODE> and save the value<br>
     * Second call the method <CODE>setInspectorModel()</CODE> with the module variable
     * <CODE>xSetModel</CODE> as parameter.<br> Then <CODE>getInspectorModel()</CODE>
     * was called and the returned valued was compared to the saved variable
     * <CODE>xSetModel</CODE><br>
     * Has <CODE>OK</CODE> status if the returned value is equal to
     * <CODE>xSetModel</CODE>.and the saved value is not null.
     */
    public void _InspectorModel() {

        log.println("testing 'getInspectorModel()'...");
        XObjectInspectorModel xGetModel = oObj.getInspectorModel() ;

        boolean result = xGetModel != null;

        log.println(result? "got a not null object -> OK" : "got a NULL object -> FAILED");

        log.println("testing 'setInspectorModel()'...");
        oObj.setInspectorModel(xSetModel);

        XObjectInspectorModel xNewModel = oObj.getInspectorModel();

        if (result) oObj.setInspectorModel(xGetModel);

        result &= xSetModel.equals(xNewModel);

        tRes.tested("InspectorModel()", result) ;
    }

    /**
     * Calls the method <CODE>getInspectorUI()</CODE>
     * Has <b>OK</b> returned value is not null
     */
    public void _InspectorUI() {

        XObjectInspectorUI oUI = oObj.getInspectorUI();

        tRes.tested("InspectorUI()", oUI != null) ;

    }
}


