/*************************************************************************
 *
 *  $RCSfile: _XSet.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:24:08 $
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

