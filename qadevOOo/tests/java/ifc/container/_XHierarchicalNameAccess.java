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

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XHierarchicalNameAccess;

/**
* Testing <code>com.sun.star.container.XHierarchicalNameAccess</code>
* interface methods :
* <ul>
*  <li><code> getByHierarchicalName()</code></li>
*  <li><code> hasByHierarchicalName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ElementName'</code> (of type <code>String</code>):
*   name of the element which exists in the container. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.container.XHierarchicalNameAccess
*/
public class _XHierarchicalNameAccess extends MultiMethodTest{
    public XHierarchicalNameAccess oObj = null;

    /**
    * Calls the method with existing and nonexisting
    * element names. <p>
    * Has <b>OK</b> status if for existing name <code>true</code>
    * is returned and for nonexisting - <code>false</code>.
    */
    public void _hasByHierarchicalName(){
        String name = (String) tEnv.getObjRelation("ElementName") ;
        boolean res = oObj.hasByHierarchicalName(name) ;
        name +="ItMakesThisNameNonExistantForSure";
        boolean res2 = oObj.hasByHierarchicalName(name);
        res &= !res2;
        tRes.tested("hasByHierarchicalName()", res) ;
    } // end _hasByHierarchicalName()

    /**
    * Tries to retrieve an element with existing name. <p>
    * Has <b>OK</b> status if non null object is returned.
    */
    public void _getByHierarchicalName(){
        String name = (String) tEnv.getObjRelation("ElementName") ;
        Object res ;

        try {
            res = oObj.getByHierarchicalName(name) ;

        } catch (NoSuchElementException e) {
            log.println("Element with name '" + name +
                "' doesn't exist in this container") ;
            res = null ;
        }

        tRes.tested("getByHierarchicalName()", res != null) ;

    } // end _getByHierarchicalName()

}  // finish class _XHierarchicalNameAccess



