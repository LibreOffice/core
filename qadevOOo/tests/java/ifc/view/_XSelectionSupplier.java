/*************************************************************************
 *
 *  $RCSfile: _XSelectionSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:33:23 $
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

package ifc.view;

import java.util.Comparator;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.EventObject;
import com.sun.star.view.XSelectionChangeListener;
import com.sun.star.view.XSelectionSupplier;

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
    Comparator comparer = null;

    protected void before() {
        selections = (Object[])tEnv.getObjRelation("Selections");
        if (selections == null) {
            throw new StatusException(Status.failed(
            "Couldn't get relation 'Selections'"));
        }

        comparer = (Comparator)tEnv.getObjRelation("Comparer");
        if (comparer == null) {
            throw new StatusException(Status.failed(
            "Couldn't get relation 'Comparer'"));
        }
    }

    /**
    * Listener implementation which just set flag when listener
    * method is called.
    */
    protected class MyChangeListener implements XSelectionChangeListener {
        public void disposing ( EventObject oEvent ) {}
        public void selectionChanged(EventObject ev) {
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
            oObj.addSelectionChangeListener(listener);
            oObj.select(selections[0]);
            oObj.select(selections[1]);
            res = selectionChanged;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured during addSelectionChangeListener()");
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
                    compRes = util.ValueComparer.equalValue(selections[i], curSelection);
                    log.println("selected object and current selection are equal: "+compRes);
                    res &= compRes;
                } else {
                    compRes = util.ValueComparer.equalValue(curSelection, oldSelection);
                    log.println("previous selection and current selection are equal: "+compRes);
                    res &= compRes;
                }
            }
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured during select()");
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
            log.println("Exception occured during removeSelectionChangeListener()");
            ex.printStackTrace(log);
            res = false;
        }
        tRes.tested("removeSelectionChangeListener()", res);
    }

    /**
    * Firts test change selection of the object : if nothing is
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



