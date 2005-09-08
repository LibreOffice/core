/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPropertyWithState.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:06:50 $
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

package ifc.beans;

import com.sun.star.beans.PropertyState;
import com.sun.star.uno.XInterface;
import lib.MultiMethodTest;

import com.sun.star.beans.XPropertyWithState;
import lib.Status;
import lib.StatusException;

/**
* Testing <code>com.sun.star.beans.XPropertyWithState</code>
* interface methods :
* <ul>
*  <li><code> getDefaultAsProperty()</code></li>
*  <li><code> getStateAsProperty()</code></li>
*  <li><code> setToDefaultAsProperty()</code></li>
* </ul> <p>
* @see com.sun.star.beans.XPropertyWithState
*/

public class _XPropertyWithState extends MultiMethodTest {

    /**
     * the test object
     */
    public XPropertyWithState oObj;


    /**
     * Test calls the method.
     * Test has ok status if no
     * <CODE>com.sun.star.lang.WrappedTargetException</CODE>
     * was thrown
     */
    public void _getDefaultAsProperty() {
        try{

            XInterface defaultState = (XInterface) oObj.getDefaultAsProperty();

        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException(Status.failed("'com.sun.star.lang.WrappedTargetException' was thrown"));
        }

        tRes.tested("getDefaultAsProperty()", true);
    }

    /**
     * Test is ok if <CODE>getStateAsProperty()</CODE> returns
     * as <CODE>PropertyState</CODE> which is not <CODE>null</CODE>
     */
    public void _getStateAsProperty() {

        boolean res = true;

        PropertyState propState = oObj.getStateAsProperty();

        if (propState == null) {
            log.println("the returned PropertyState is null -> FALSE");
            res = false;
        }

        tRes.tested("getStateAsProperty()", res);
    }

    /**
     * Test calls the method.
     * Test has ok status if no
     * <CODE>com.sun.star.lang.WrappedTargetException</CODE>
     * was thrown
     */
    public void _setToDefaultAsProperty() {
        try{

            oObj.setToDefaultAsProperty();

        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException(Status.failed("'com.sun.star.lang.WrappedTargetException' was thrown"));
        }

        tRes.tested("setToDefaultAsProperty()", true);
    }

}
