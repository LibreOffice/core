/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSheetFilterableEx.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:56:08 $
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

import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSheetFilterable;
import com.sun.star.sheet.XSheetFilterableEx;
import com.sun.star.uno.UnoRuntime;


/**
 * Inteface test for XSheetFilterableEx
 *
 * methods:
 *
 *  createFilterDescriptorByObject
 *
 */
public class _XSheetFilterableEx extends MultiMethodTest {
    public XSheetFilterableEx oObj = null;

    /**
     * queries a XSheetFilterable from the parent Object and then creates
     * a XSheetFilterDescriptor with it.
     * Returns OK if the Descriptor isn't null and checkFilterDescriptor returns true
     */

    public void _createFilterDescriptorByObject() {
        boolean res = true;
        Object parent = tEnv.getTestObject();
        XSheetFilterable aFilterable = (XSheetFilterable) UnoRuntime.queryInterface(
                                               XSheetFilterable.class, parent);
        XSheetFilterDescriptor desc = oObj.createFilterDescriptorByObject(
                                              aFilterable);

        if (desc != null) {
            res &= checkFilterDescriptor(desc);
        } else {
            log.println("gained XSheetFilterDescriptor is null");
            res &= false;
        }
        tRes.tested("createFilterDescriptorByObject()",res);
    }

    /**
     * returns true if the Method getFilterFields() returns a non empty array
     * and all Fields can be gained without exception
     */

    public boolean checkFilterDescriptor(XSheetFilterDescriptor desc) {
        TableFilterField[] fields = desc.getFilterFields();
        boolean res = true;

        if (fields.length == 0) {
            log.println("The gained Descriptor is empty");
            res &= false;
        } else {
               log.println("Found "+fields.length+" TableFields");
        }

        for (int k = 0; k < fields.length; k++) {
            log.println("StringValue(" + k + "): " + fields[k].StringValue);
            log.println("IsNumeric(" + k + "): " + fields[k].IsNumeric);
            log.println("NumericValue(" + k + "): " + fields[k].NumericValue);
            log.println("Field(" + k + "): " + fields[k].Field);
            log.println("Connection(" + k + "): " + fields[k].Connection);
            log.println("Operator(" + k + "): " + fields[k].Operator);
        }

        return res;
    }
}