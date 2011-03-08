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
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XReplaceable
 */
public class _XReplaceable extends MultiMethodTest {

    public XReplaceable oObj = null;
    public XReplaceDescriptor Rdesc = null;
    private String mSearchString = "xTextDoc";
    private String mReplaceString = "** xTextDoc";
    private boolean mDispose = false;

    /**
     * Creates an entry to search for, if the current object does not provide
     * one. In this case, the environment is disposed after the test, since
     * the inserted object may influence following tests.
     *
     */
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
        tRes.tested("createReplaceDescriptor()", Rdesc != null);

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
    protected void after() {
        if(mDispose) {
            disposeEnvironment();
        }
    }
}

