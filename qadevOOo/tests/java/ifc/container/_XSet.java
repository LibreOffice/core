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

import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiServiceFactory;

/**
* Testing <code>com.sun.star.container.XSet</code>
* interface methods :
* <ul>
*  <li><code> has()</code></li>
*  <li><code> insert()</code></li>
*  <li><code> remove()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.container.XSet
*/
public class _XSet extends MultiMethodTest {

    public static XSet oObj = null;

    private Object element = null ;
    private boolean hasResult = true ;

    /**
    * Inserts the element stored and removed before. <p>
    * Has <b> OK </b> status if <code>has()</code> method returns
    * <code>true</code>.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> remove() </code> : element from set is stored and
    *    removed </li>
    * </ul>
    */
    public void _insert() {
        boolean res = true ;
        try {
            XMultiServiceFactory xMSF = tParam.getMSF();
            element = xMSF.createInstance(
                            "com.sun.star.reflection.TypeDescriptionProvider");


            oObj.insert(element) ;

            if (!oObj.has(element)) {
                res = false ;
                log.println("After adding element method has()" +
                    " returned false") ;
                hasResult = false ;
            }
        } catch (com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log) ;
            res = false ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            res = false ;
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("insert()", res) ;
    }

    /**
    * Through <code>XEnumeration</code> interface an element from the set
    * is retrieved. Then this element removed.<p>
    * Has <b> OK </b> status if after removing <code>has()</code> method
    * returns false.
    */
    public void _remove() {
        requiredMethod("insert()");
        boolean res = true ;

        try {

            // get an element to be removed/inserted

            oObj.remove(element) ;

            if (oObj.has(element)) {
                res = false ;
                log.println("After removing element method has()" +
                    " returned true") ;
                hasResult = false ;
            }



        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log) ;
            res = false ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            res = false ;
        }

        tRes.tested("remove()", res) ;
    }

    /**
    * Does nothing. Testing performed in previous methods.<p>
    * Has <b> OK </b> status if after <code>remove()</code> call
    * methods returned <code>false</code>, and after <code>remove()</code>
    * call methods returned <code>true</code>.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insert() </code> : here the method is checked </li>
    *  <li> <code> remove() </code> : here the method is checked </li>
    * </ul>
    */
    public void _has() {

        requiredMethod("insert()") ;
        requiredMethod("remove()") ;

        tRes.tested("has()", hasResult) ;
    }

} // finished class _XSet

