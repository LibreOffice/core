/*************************************************************************
 *
 *  $RCSfile: _XEnumeration.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:22:25 $
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

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.WrappedTargetException;

/**
* Testing <code>com.sun.star.container.XEnumeration</code>
* interface methods :
* <ul>
*  <li><code> hasMoreElements()</code></li>
*  <li><code> nextElement()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ENUM'</code> (of type <code>XEnumerationAccess</code>):
*   This test creates its own oObj because the method nextElement()
*   will be modified this Object directly so other threads may be faild.
*  </li>
* <ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.container.XEnumeration
*/
public class _XEnumeration extends MultiMethodTest {

    public XEnumeration oObj = null;
    public XEnumerationAccess Enum = null;

    /**
    * Retrieves relation and sets oObj to a separate enumeration
    * created. Retrieves all elements from enumeration.<p>
    * Has <b> OK </b> status if all elements successfully retrieved
    * and exceptions occured.
    */
    public void _hasMoreElements() {
        boolean result = true;

        log.println("get all elements");
        while ( oObj.hasMoreElements() ) {
            try {
                Object oAny = oObj.nextElement();
            } catch (WrappedTargetException e) {
                log.println("hasMoreElements() : " + e);
                result = false;
                break;
            } catch (NoSuchElementException e) {
                log.println("hasMoreElements() : " + e);
                result = false;
                break;
            }
        }

        tRes.tested("hasMoreElements()", result);
        return;
    } // end hasMoreElements

    /**
    * Calls the method (on starting this method there is no more elements
    * in the enumeration. <p>
    * Has <b> OK </b> status if only <code>NoSuchElementException</code>
    * exception rises. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> hasMoreElements() </code> : it retrieves all elements </li>
    * </ul>
    */
    public void _nextElement(){
        requiredMethod("hasMoreElements()");
        boolean result = true;
        log.println("additional call must throw NoSuchElementException");

        try {
            Object oAny = oObj.nextElement();
            log.println("nextElement: no exception!");
            result = false;
        } catch (WrappedTargetException e) {
            log.println("nextElement: wrong exception!");
            result = false;
        } catch (NoSuchElementException e) {
            log.println("nextElement: correct exception");
        }

        tRes.tested("nextElement()", result);

        return;

    } // end NextElement

} //end XEnumeration

