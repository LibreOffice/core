/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _TextColumns.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:15:23 $
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

package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.text.XTextColumns;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.text.TextColumns</code>
* service properties :
* <ul>
*  <li><code> IsAutomatic</code></li>
*  <li><code> AutomaticDistance</code></li>
*  <li><code> SeparatorLineWidth</code></li>
*  <li><code> SeparatorLineColor</code></li>
*  <li><code> SeparatorLineRelativeHeight</code></li>
*  <li><code> SeparatorLineVerticalAlignment</code></li>
*  <li><code> SeparatorLineIsOn</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.TextColumns
*/
public class _TextColumns extends MultiPropertyTest {


    /**
    * Redefined method returns specific value, that differs from property
    * value. ( (oldValue + referenceValue) / 2 ).
    */
    public void _AutomaticDistance() {
        log.println("Testing with custom Property tester") ;
        testProperty("AutomaticDistance", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                XTextColumns xTC = (XTextColumns)
                    UnoRuntime.queryInterface
                        (XTextColumns.class,tEnv.getTestObject());
                int ref = xTC.getReferenceValue();
                int setting = ( ( (Integer) oldValue).intValue() + ref) / 2;
                return new Integer(setting);
            }
        });
    }


} //finish class _TextColumns

