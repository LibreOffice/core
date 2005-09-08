/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSheetFilterDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:55:38 $
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

package ifc.sheet;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XSheetFilterDescriptor;

/**
* Testing <code>com.sun.star.sheet.XSheetFilterDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getFilterFields()</code></li>
*  <li><code> setFilterFields()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetFilterDescriptor
*/
public class _XSheetFilterDescriptor extends MultiMethodTest {

    public XSheetFilterDescriptor oObj = null;
    public TableFilterField[] TFF = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getFilterFields(){
        TFF = oObj.getFilterFields();
        tRes.tested("getFilterFields()", TFF != null );
    }

    /**
    * Test sets new value of the definitions of the filter fields,
    * gets the current definitions of the filter fields and compares
    * returned value with value that was stored by method
    * <code>getFilterFields()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFilterFields() </code> : to have the current definitions
    *  of the filter fields </li>
    * </ul>
    */
    public void _setFilterFields() {
        requiredMethod("getFilterFields()");
        TableFilterField[] newTFF = new TableFilterField[1];
        newTFF[0] = new TableFilterField();
        oObj.setFilterFields(newTFF);
        newTFF = oObj.getFilterFields();
        tRes.tested("setFilterFields()", ! ValueComparer.equalValue(TFF, newTFF));
    }

}  // finish class _XSheetFilterDescriptor


