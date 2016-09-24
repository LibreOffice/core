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

package ifc.util;

import com.sun.star.table.XCell;
import lib.MultiMethodTest;

import com.sun.star.util.XReplaceDescriptor;
import com.sun.star.util.XReplaceable;
import com.sun.star.util.XSearchDescriptor;

/**
 * Testing <code>com.sun.star.util.XReplaceable</code>
 * interface methods :
 * <ul>
 *  <li><code> createReplaceDescriptor()</code></li>
 *  <li><code> replaceAll()</code></li>
 * </ul> <p>
 *
 * The requipment for the tested object is that it
 * <b>must containt</b> string 'xTextDoc'. Only
 * in that case this interface is tested correctly. <p>
 *
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.util.XReplaceable
 */
public class _XReplaceable extends MultiMethodTest {

    public XReplaceable oObj = null;
    public XReplaceDescriptor Rdesc = null;
    private String mSearchString = "xTextDoc";
    private static final String mReplaceString = "** xTextDoc";
    private boolean mDispose = false;

    /**
     * Creates an entry to search for, if the current object does not provide
     * one. In this case, the environment is disposed after the test, since
     * the inserted object may influence following tests.
     *
     */
    @Override
    protected void before() {
        Object o = tEnv.getObjRelation("SEARCHSTRING");
        if (o != null) {
            mSearchString = (String)o;
        }
        // use object relation for XSearchable
        o = tEnv.getObjRelation("XSearchable.MAKEENTRYINCELL");
        if (o != null) {
            XCell[] cells = new XCell[0];
            if (o instanceof XCell) {
                cells = new XCell[]{(XCell)o};
            }
            else if (o instanceof XCell[]) {
                cells = (XCell[])o;
            }
            else {
                log.println("Needed object relation 'XSearchable.MAKEENTRYINCELL' is there, but is of type '"
                            + o.getClass().getName() + "'. Should be 'XCell' or 'XCell[]' instead.");
            }
            for (int i=0; i<cells.length; i++) {
                cells[i].setFormula(mSearchString);
            }
            mDispose = true;
        }
    }

    /**
     * Creates the descriptor for replacing string 'xTextDoc'
     * with string '** xTextDoc'. <p>
     * Has <b> OK </b> status if the returned descriptor is not
     * <code>null</code>. <p>
     */
    public void _createReplaceDescriptor() {

        log.println("testing createReplaceDescriptor() ... ");

        Rdesc = oObj.createReplaceDescriptor();
        Rdesc.setSearchString(mSearchString);
        Rdesc.setReplaceString(mReplaceString);
        tRes.tested("createReplaceDescriptor()", true);

    }

    /**
     * Replaces the text using descriptor created before. Then
     * search is performed in the target text. <p>
     *
     * Has <b> OK </b> status if the string '**' is found in
     * the text. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> createReplaceDescriptor() </code> : replace
     *    descriptor is created. </li>
     * </ul>
     */
    public void _replaceAll() {
        requiredMethod("createReplaceDescriptor()");
        oObj.replaceAll(Rdesc);
        XSearchDescriptor SDesc = oObj.createSearchDescriptor();
        SDesc.setSearchString("**");
        boolean res = (oObj.findFirst(SDesc) != null);
        // redo replacement
        Rdesc.setSearchString(mReplaceString);
        Rdesc.setReplaceString(mSearchString);
        oObj.replaceAll(Rdesc);
        res &= (oObj.findFirst(SDesc) == null);

        tRes.tested("replaceAll()",res);
    }

    /**
     * In case the interface itself made the entry to search for, the environment
     * must be disposed
     */
    @Override
    protected void after() {
        if(mDispose) {
            disposeEnvironment();
        }
    }
}

