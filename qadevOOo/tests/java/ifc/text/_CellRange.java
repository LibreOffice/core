/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _CellRange.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:11:52 $
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

/**
* Testing <code>com.sun.star.text.CellRange</code>
* service properties :
* <ul>
*  <li><code> BackColor</code></li>
*  <li><code> BackGraphicURL</code></li>
*  <li><code> BackGraphicFilter</code></li>
*  <li><code> BackGraphicLocation</code></li>
*  <li><code> BackTransparent</code></li>
*  <li><code> ChartColumnAsLabel</code></li>
*  <li><code> ChartRowAsLabel</code></li>
*  <li><code> NumberFormat</code></li>
*  <li><code> TopMargin</code></li>
*  <li><code> BottomMargin</code></li>
*
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code> crazy-blue.jpg </code> : jpeg image used to test
*  BackGraphicURL()</li>
*  <li> <code> space-metal.jpg </code> : jpeg image used to test
*  BackGraphicURL()</li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.CellProperties
*/
public class _CellRange extends MultiPropertyTest {

    /**
    * Redefined method returns value, that differs from property value.
    */
    public void _BackColor() {
        final Short val1 = new Short( (short) 4 );
        final Short val2 = new Short( (short) 6 );
        log.println("Testing with custom Property tester") ;
        testProperty("BackColor", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                if ( oldValue.equals(val1) )
                    return val2;
                else
                    return val1;
            }
        });
    }

    /**
     * This property could be changed only when graphic
     * URL is set.
     */
    public void _BackGraphicFilter() {
        executeMethod("BackGraphicURL");
        testProperty("BackGraphicFilter");
    }

    /**
    * Redefined method returns value, that differs from property value.
    */
    public void _BackGraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("BackGraphicURL", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                if (oldValue.equals(util.utils.getFullTestURL
                        ("space-metal.jpg")))
                    return util.utils.getFullTestURL("crazy-blue.jpg");
                else
                    return util.utils.getFullTestURL("space-metal.jpg");
            }
        });
    }
}

