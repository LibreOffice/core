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
* Test is <b> NOT </b> multithread compilant. <p>
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
            XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
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

