/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XGridColumnFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:46:43 $
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

import com.sun.star.beans.XPropertySet;
import com.sun.star.form.XGridColumnFactory;


/**
* Testing <code>com.sun.star.form.XGridColumnFactory</code>
* interface methods :
* <ul>
*  <li><code> createColumn()</code></li>
*  <li><code> getColumnTypes()</code></li>
* </ul>
* Test is multithread compilant. <p>
* @see com.sun.star.form.XGridColumnFactory
*/
public class _XGridColumnFactory extends MultiMethodTest {

    public XGridColumnFactory oObj = null;


    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns
    * non null value.
    */
    public void _getColumnTypes() {

        String[] types = oObj.getColumnTypes();
        tRes.tested("getColumnTypes()",types!=null);
    }

    /**
    * Creates column of type 'TextField'. <p>
    * Has <b>OK</b> status if non null value returned.
    */
    public void _createColumn() {

        try {
            XPropertySet column = oObj.createColumn("TextField");
            tRes.tested("createColumn()",column!=null);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while creating Column");
            tRes.tested("createColumn()",false);
        }
    }
}  // finish class _XGridColumnFactory



