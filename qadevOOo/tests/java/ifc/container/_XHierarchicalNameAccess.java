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
* Test is <b> NOT </b> multithread compliant. <p>
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



