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
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.view.XSelectionSupplier
 */
public class _XSelectionSupplier extends MultiMethodTest {

    public XSelectionSupplier oObj = null;
    public boolean selectionChanged = false;
    Object[] selections = null;
    Comparator<Object> ObjCompare = null;

    @Override
    protected void before() {
        selections = (Object[])tEnv.getObjRelation("Selections");
        if (selections == null) {
            throw new StatusException(Status.failed(
                    "Couldn't get relation 'Selections'"));
        }

        ObjCompare = (Comparator<Object>)tEnv.getObjRelation("Comparer");
    }

    @Override
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
     * Has <b>OK</b> status if selection listener was called.
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
                    if (!compRes && (selections[i]) instanceof Object[]){
                        if (((Object[])selections[i])[0] instanceof Integer) {
                            log.println("Getting: "+((Integer) ((Object[])curSelection)[0]).intValue());
                            log.println("Expected: "+((Integer) ((Object[])selections[i])[0]).intValue());
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
     * Has <b>OK</b> status if selection listener was not called.
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



