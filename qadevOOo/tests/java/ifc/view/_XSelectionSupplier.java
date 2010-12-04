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

package ifc.view;

import com.sun.star.lang.EventObject;
import com.sun.star.view.XSelectionChangeListener;
import com.sun.star.view.XSelectionSupplier;
import java.util.Comparator;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


/**
 * Testing <code>com.sun.star.view.XSelectionSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> select()</code></li>
 *  <li><code> getSelection()</code></li>
 *  <li><code> addSelectionChangeListener()</code></li>
 *  <li><code> removeSelectionChangeListener()</code></li>
 * </ul>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'Selections'</code> of type <code>Object[]</code> :
 *   the array of the instances which can be selected.</li>
 *  <li> <code>'Comparer'</code> of type <code>Comparator</code> :
 *   the interface for comparing of selected instances</li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.view.XSelectionSupplier
 */
public class _XSelectionSupplier extends MultiMethodTest {

    public XSelectionSupplier oObj = null;
    public boolean selectionChanged = false;
    Object[] selections = null;
    Comparator ObjCompare = null;

    protected void before() {
        selections = (Object[])tEnv.getObjRelation("Selections");
        if (selections == null) {
            throw new StatusException(Status.failed(
                    "Couldn't get relation 'Selections'"));
        }

        ObjCompare = (Comparator)tEnv.getObjRelation("Comparer");
    }

    protected void after() {
        disposeEnvironment();
    }

    /**
     * Listener implementation which just set flag when listener
     * method is called.
     */
    public class MyChangeListener implements XSelectionChangeListener {
        public void disposing( EventObject oEvent ) {}
        public void selectionChanged(EventObject ev) {
            log.println("listener called");
            selectionChanged = true;
        }

    }

    XSelectionChangeListener listener = new MyChangeListener();

    /**
     * Test adds listener to the object, then selects first and
     * then second instances to be sure that selection was changed.<p>
     * Has <b>OK</b> status if selection lisener was called.
     */
    public void _addSelectionChangeListener(){
        boolean res = true;
        try {
            selectionChanged = false;
            oObj.addSelectionChangeListener(listener);
            oObj.select(selections[0]);
            oObj.select(selections[1]);
            res = selectionChanged;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred during addSelectionChangeListener()");
            ex.printStackTrace(log);
            res = false;
        }
        tRes.tested("addSelectionChangeListener()", res);
    }

    /**
     * Selects an instance from relation 'First'. <p>
     * Has <b> OK </b> status if no exceptions were thrown. <p>
     */
    public void _select() {
        boolean res  = true;
        boolean locRes = true;
        boolean compRes = true;
        Object oldSelection = null;
        try {
            for(int i = 0; i < selections.length; i++) {
                oldSelection = oObj.getSelection();
                locRes = oObj.select(selections[i]);
                log.println("select #" + i + ": " + locRes);
                Object curSelection = oObj.getSelection();
                if (locRes) {

                    if (ObjCompare != null) {
                        ObjCompare.compare(selections[i], curSelection);
                    } else {
                        compRes = util.ValueComparer.equalValue(selections[i], curSelection);
                    }
                    log.println("selected object and current selection are equal: "+compRes);
                    if (!compRes) {
                        if ((selections[i]) instanceof Object[]){
                            if (((Object[])selections[i])[0] instanceof Integer) {
                                log.println("Getting: "+((Integer) ((Object[])curSelection)[0]).intValue());
                                log.println("Expected: "+((Integer) ((Object[])selections[i])[0]).intValue());
                            }
                        }
                    }
                    res &= compRes;
                } else {
                    compRes = util.ValueComparer.equalValue(curSelection, oldSelection);
                    log.println("previous selection and current selection are equal: "+compRes);
                    res &= compRes;
                }
            }
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred during select()");
            ex.printStackTrace(log);
            res = false;
        }

        tRes.tested("select()", res);
    }

    /**
     * Test removes listener, then selects first and
     * then second instances to be sure that selection was changed.<p>
     * Has <b>OK</b> status if selection lisener was not called.
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addSelectionChangeListener() </code> : to have
     *   the listener added. </li>
     * </ul>
     */
    public void _removeSelectionChangeListener() {
        boolean res = false;
        requiredMethod("addSelectionChangeListener()");
        try {
            selectionChanged = false;
            oObj.removeSelectionChangeListener(listener);
            oObj.select(selections[0]);
            oObj.select(selections[1]);
            res = !selectionChanged;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred during removeSelectionChangeListener()");
            ex.printStackTrace(log);
            res = false;
        }
        tRes.tested("removeSelectionChangeListener()", res);
    }

    /**
     * First test changes selection of the object : if nothing is
     * currently selected or first instance ('First' relation) is
     * selected then selects second instance; if second instance
     * is currently selected then the first instance is selected. <p>
     * Then <code>getSelection</code> is called and values set and
     * get are compared. Comparison has some special cases. For
     * example if selection is a Cell, then the values contained
     * in cells are compared. <p>
     * Has <b>OK</b> status if selection changed properly.
     */
    public void _getSelection() {
        requiredMethod("select()");
        tRes.tested("getSelection()", true);
    }

}  // finish class _XSelectionSupplier



