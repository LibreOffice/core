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

import lib.MultiMethodTest;

import com.sun.star.container.XIndexAccess;
import com.sun.star.table.XCell;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.util.XSearchable;

/**
 * Testing <code>com.sun.star.util.XSearchable</code>
 * interface methods :
 * <ul>
 *  <li><code> createSearchDescriptor()</code></li>
 *  <li><code> findAll()</code></li>
 *  <li><code> findFirst()</code></li>
 *  <li><code> findNext()</code></li>
 * </ul> <p>
 *
 * The requipment for the tested object is that it
 * <b>must containt</b> string 'xTextDoc'. Only
 * in that case this interface is tested correctly. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XSearchable
 */
public class _XSearchable extends MultiMethodTest {

    public XSearchable oObj = null;                // oObj filled by MultiMethodTest
    public XSearchDescriptor Sdesc = null;
    public Object start = null;
    private String mSearchString = "xTextDoc";
    private boolean mDispose = false;
    private boolean mExcludeFindNext = false;

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
        mExcludeFindNext = (tEnv.getObjRelation("EXCLUDEFINDNEXT")==null)?false:true;
    }

    /**
     * Creates the search descriptor which searches for
     * 'xTextDoc' string. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _createSearchDescriptor() {

        log.println("testing createSearchDescriptor() ... ");

        Sdesc = oObj.createSearchDescriptor();
        Sdesc.setSearchString(mSearchString);
        tRes.tested("createSearchDescriptor()", Sdesc != null);

    }

    /**
     * Performs search using descriptor created before. <p>
     * Has <b> OK </b> status if the method not <code>null</code>
     * collections. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> createSearchDescriptor() </code> : creates the descriptor
     *  required for search. </li>
     * </ul>
     */
    public void _findAll() {

        requiredMethod("createSearchDescriptor()");
        log.println("testing findAll()");

        XIndexAccess IA = oObj.findAll(Sdesc);
        tRes.tested("findAll()", IA != null);
    }

    /**
     * Performs search using descriptor created before. Storing the
     * first occurrence result. <p>
     * Has <b> OK </b> status if the method not <code>null</code>
     * value. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> createSearchDescriptor() </code> : creates the descriptor
     *  required for search. </li>
     * </ul>
     */
    public void _findFirst() {

        requiredMethod("createSearchDescriptor()");
        log.println("testing findFirst()");
        start = oObj.findFirst(Sdesc);
        tRes.tested("findFirst()", start != null);
    }

    /**
     * Performs search using descriptor and first search result
     * created before. <p>
     * Has <b> OK </b> status if the method not <code>null</code>
     * value. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> findFirst() </code> : to have first search result. </li>
     * </ul>
     */
    public void _findNext() {
        if (mExcludeFindNext) {
            log.println("Testing findNext() excluded, because only one" +
                        " search result is available.");
            tRes.tested("findNext()", true);
        }
        else{
            requiredMethod("findFirst()");

            log.println("testing findNext()");
            Object xI = oObj.findNext(start,Sdesc);
            tRes.tested("findNext()", xI != null);
        }
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


