/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XGridFieldDataSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:46:57 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XGridFieldDataSupplier;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.form.XGridFieldDataSupplier</code>
* interface methods :
* <ul>
*  <li><code>queryFieldDataType()</code></li>
*  <li><code>queryFieldData()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XGridFieldDataSupplier
*/
public class _XGridFieldDataSupplier extends MultiMethodTest {
     public XGridFieldDataSupplier oObj = null;

    /**
    * Calls the method and checks returned value. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is not null.
    */
     public void _queryFieldDataType() {
        boolean[] dataType = oObj.queryFieldDataType(new Type(String.class));
        tRes.tested("queryFieldDataType()", dataType != null);
     }

    /**
    * Calls the method and checks returned value. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is not null.
    */
     public void _queryFieldData() {
        Object[] data = oObj.queryFieldData(0, new Type(String.class));
        tRes.tested("queryFieldData()", data != null);
     }
}// finish class _XGridFieldDataSupplier

