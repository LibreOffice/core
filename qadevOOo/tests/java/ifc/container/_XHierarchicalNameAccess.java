/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XHierarchicalNameAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:21:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



