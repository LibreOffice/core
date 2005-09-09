/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XIndent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:41:24 $
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

package ifc.util;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XIndent;

/**
* Testing <code>com.sun.star.util.XCancellable</code>
* interface methods :
* <ul>
*  <li><code> decrementIndent()</code></li>
*  <li><code> incrementIndent()</code></li>
* </ul> <p>
* @see com.sun.star.util.XIndent
*/
public class _XIndent extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XIndent oObj = null ;

    protected XPropertySet PropSet = null;

    /**
     * Ensures that the ObjRelation PropSet is given.
     */
    public void before() {
        PropSet = (XPropertySet) tEnv.getObjRelation("PropSet");
        if (PropSet == null) {
            throw new StatusException(Status.failed("No PropertySet given"));
        }
    }

    /**
     * Calls the method. <p>
     * Has <b>OK</b> status if the property 'ParaIndent' is incremented afterwards<p>
     */
    public void _incrementIndent() {
        int oldValue = getIndent();
        oObj.incrementIndent();
        int newValue = getIndent();
        tRes.tested("incrementIndent()", oldValue < newValue) ;
    }

    /**
     * Calls the method. <p>
     * Has <b>OK</b> status if the property 'ParaIndent' is decremented afterwards<p>
     * requires 'incrementIndent()' to be executed first.
     */
    public void _decrementIndent() {
        requiredMethod("incrementIndent()");
        int oldValue = getIndent();
        oObj.decrementIndent();
        int newValue = getIndent();
        tRes.tested("decrementIndent()", oldValue > newValue) ;
    }

    public short getIndent() {
        short ret = 0;
        try {
            ret = ((Short) PropSet.getPropertyValue("ParaIndent")).shortValue();
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
        } catch (com.sun.star.lang.WrappedTargetException wte) {
        }
        return ret;
    }

}  // finish class _XCancellable

