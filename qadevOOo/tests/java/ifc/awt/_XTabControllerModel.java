/*************************************************************************
 *
 *  $RCSfile: _XTabControllerModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:13:02 $
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
import lib.Status;

import com.sun.star.awt.XTabControllerModel;

/**
* Testing <code>com.sun.star.awt.XTabControllerModel</code>
* interface methods :
* <ul>
*  <li><code> getGroupControl()</code></li>
*  <li><code> setGroupControl()</code></li>
*  <li><code> setControlModels()</code></li>
*  <li><code> getControlModels()</code></li>
*  <li><code> setGroup()</code></li>
*  <li><code> getGroupCount()</code></li>
*  <li><code> getGroup()</code></li>
*  <li><code> getGroupByName()</code></li>
* </ul> <p>
*
* Since the interface is <b>DEPRECATED</b> all methods have
* status SKIPPED.OK <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'Model1'</code> : <code>XControlModel</code> relation
*    , any control model belonging to the object tested.</li>
*  <li> <code>'Model2'</code> : <code>XControlModel</code> relation
*    , any control model belonging to the object tested.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XTabControllerModel
*/
public class _XTabControllerModel extends MultiMethodTest {

    public XTabControllerModel oObj = null;


    /**
    * Sets group control to <code>true</code> then calls
    * <code>getGroupControl()</code> method and checks the value. <p>
    * Has <b> OK </b> status if the method returns <code>true</code>
    * <p>
    */
    public void _getGroupControl() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroupControl()", Status.skipped(true));
    }

    /**
    * Sets group control to <code>false</code> then calls
    * <code>getGroupControl()</code> method and checks the value. <p>
    * Has <b> OK </b> status if the method returns <code>false</code>
    * <p>
    */
    public void _setGroupControl() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("setGroupControl()", Status.skipped(true));
    }

    /**
    * Test calls the method and sets control models to a single
    * model from 'Model1' relation. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _setControlModels() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("setControlModels()", Status.skipped(true));
    }

    /**
    * Calls method and checks if models were properly set in
    * <code>setControlModels</code> method test. <p>
    * Has <b>OK</b> status if the model sequence set is equal
    * to the sequence get. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setControlModels </code> : to set sequence of models.</li>
    * </ul>
    */
    public void _getControlModels() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getControlModels()", Status.skipped(true));
    }

    /**
    * Sets the group named 'XTabControlModel' to sequence with a single
    * element from 'Model2' relation. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public void _setGroup() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("setGroup()", Status.skipped(true));
    }

    /**
    * Calls method and checks if the group was properly set in
    * <code>setGroup</code> method test. <p>
    * Has <b>OK</b> status if the sequence set is equal
    * to the sequence get. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGroup </code> : to set the sequence.</li>
    * </ul>
    */
    public void _getGroup() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroup()", Status.skipped(true));
    }

    /**
    * Retrieves group named 'XTabControllerModel' added in <code>
    * setGroup</code> method test, and checks it. <p>
    * Has <b> OK </b> status if sequence get is equal to sequence set.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGroup </code> : to set the sequence.</li>
    * </ul>
    */
    public void _getGroupByName() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroupByName()", Status.skipped(true));
    }

    /**
    * Gets number of groups.
    * Has <b> OK </b> status if the number is greater than 0.
    * <ul>
    *  <li> <code> setGroup </code> : to has at least one group.</li>
    * </ul>
    */
    public void _getGroupCount() {
        log.println("Always SKIPPED.OK since deprecated.");
        tRes.tested("getGroupCount()", Status.skipped(true));
    }

}

