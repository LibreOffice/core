/*************************************************************************
 *
 *  $RCSfile: _DataPilotField.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2005-03-29 13:04:39 $
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

package ifc.sheet;

import com.sun.star.sheet.DataPilotFieldAutoShowInfo;
import com.sun.star.sheet.DataPilotFieldLayoutInfo;
import com.sun.star.sheet.DataPilotFieldReference;
import com.sun.star.sheet.DataPilotFieldSortInfo;
import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.sheet.DataPilotField</code>
* service properties :
* <ul>
*  <li><code> Orientation</code></li>
*  <li><code> Function</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.DataPilotField
*/
public class _DataPilotField extends MultiPropertyTest {

    /**
     *This class is destined to custom test of property <code>SortInfo</code>.
     */
    protected PropertyTester SortInfoTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldSortInfo sortInfo = new DataPilotFieldSortInfo();
        sortInfo.Mode = com.sun.star.sheet.DataPilotFieldSortMode.MANUAL;
            sortInfo.IsAscending = false;
            sortInfo.Field = "COL1";
            log.println("Having Value and returning a new one");
            return sortInfo.equals(oldValue) ? null : sortInfo;
        }
    };

    /**
     * Test property <code>SortInfo</code> using custom <code>PropertyTest</code>.
     */
    public void _SortInfo() {
        testProperty("SortInfo", SortInfoTester);
    }

    public void _HasSortInfo() {
        requiredMethod("SortInfo");
        testProperty("HasSortInfo");
    }

    /**
     *This class is destined to custom test of property <code>LayoutInfo</code>.
     */
    protected PropertyTester LayoutInfoTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldLayoutInfo layoutInfo = new DataPilotFieldLayoutInfo();
            layoutInfo.LayoutMode = com.sun.star.sheet.DataPilotFieldLayoutMode.OUTLINE_SUBTOTALS_BOTTOM;
            layoutInfo.AddEmptyLines = true;
            log.println("Having Value and returning a new one");
            return layoutInfo.equals(oldValue) ? null : layoutInfo;
        }
    };

    /**
     * Test property <code>LayoutInfo</code> using custom <code>PropertyTest</code>.
     */
    public void _LayoutInfo() {
        testProperty("LayoutInfo", LayoutInfoTester);
    }

    public void _HasLayoutInfo() {
        requiredMethod("LayoutInfo");
        testProperty("HasLayoutInfo");
    }

    /**
     *This class is destined to custom test of property <code>AutoShowInfo</code>.
     */
    protected PropertyTester AutoShowInfoTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldAutoShowInfo AutoShowInfo = new DataPilotFieldAutoShowInfo();
            AutoShowInfo.DataField = "Col1";
            AutoShowInfo.IsEnabled = true;
            log.println("Having Value and returning a new one");
            return AutoShowInfo.equals(oldValue) ? null : AutoShowInfo;
        }
    };

    /**
     * Test property <code>AutoShowInfo</code> using custom <code>PropertyTest</code>.
     */
    public void _AutoShowInfo() {
        testProperty("AutoShowInfo", AutoShowInfoTester);
    }

    public void _HasAutoShowInfo() {
        requiredMethod("AutoShowInfo");
        testProperty("HasAutoShowInfo");
    }

    /**
     *This class is destined to custom test of property <code>Reference</code>.
     */
    protected PropertyTester ReferenceTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldReference Reference = new DataPilotFieldReference();
            Reference.ReferenceField="Col1";
            Reference.ReferenceItemType = com.sun.star.sheet.DataPilotFieldReferenceItemType.NAMED;
            log.println("Having Value and returning a new one");
            return Reference.equals(oldValue) ? null : Reference;
        }
    };

    /**
     * Test property <code>Reference</code> using custom <code>PropertyTest</code>.
     */
    public void _Reference() {
        testProperty("Reference", ReferenceTester);
    }

    public void _HasReference() {
        requiredMethod("Reference");
        testProperty("HasReference");
    }

    /*
     * this property can only set a true value to false, fo ungrouping purposes
     */
    public void _IsGroupField() {
        boolean result = true;
        try {
            Boolean propValue = (Boolean) oObj.getPropertyValue ("IsGroupField");
            if (propValue.booleanValue ()) {
                oObj.setPropertyValue ("IsGroupField", Boolean.FALSE);
                Boolean propV = (Boolean) oObj.getPropertyValue ("IsGroupField");
                result = !propV.booleanValue ();
            }
        } catch (Exception e) {
            log.println ("Exception while getting Property 'IsGroupField'"+e);
            result = false;
        }
        tRes.tested ("IsGroupField",result);
    }
}


