/*************************************************************************
 *
 *  $RCSfile: _XSearchable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:25:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Pu blic License Version 2.1
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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.container.XIndexAccess;
import com.sun.star.table.XCell;
import com.sun.star.uno.XInterface;
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
     * first occurence result. <p>
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


