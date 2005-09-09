/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _TableColumn.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:05:07 $
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

package ifc.table;

import lib.MultiPropertyTest;
import util.ValueComparer;

import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.table.TableColumn</code>
* service properties :
* <ul>
*  <li><code> Width</code></li>
*  <li><code> OptimalWidth</code></li>
*  <li><code> IsVisible</code></li>
*  <li><code> IsStartOfNewPage</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.table.TableColumn
*/
public class _TableColumn extends MultiPropertyTest {

    public void _OptimalWidth() {
        boolean res = false;
        try {
            XInterface tObject = tEnv.getTestObject();
            XCellRange aRange = (XCellRange)
                    UnoRuntime.queryInterface(XCellRange.class, tObject);
            aRange.getCellByPosition(0,0).setFormula("Thats a pretty long text");
            Object width_before = oObj.getPropertyValue("Width");
            log.println("Before setting to optimal width: "+width_before);
            oObj.setPropertyValue("OptimalWidth", new Boolean(true));
            Object width_after = oObj.getPropertyValue("Width");
            log.println("After setting to optimal width: "+width_after);
            res = ! ValueComparer.equalValue(width_before,width_after);
        } catch (Exception e) {
           res = false;
           e.printStackTrace(log);
        }

        tRes.tested("OptimalWidth",res );
    }

} //finish class _TableColumn


