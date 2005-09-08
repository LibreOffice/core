/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _ChartTitle.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:11:22 $
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

package ifc.chart;

import lib.MultiPropertyTest;
import util.ValueChanger;

/**
* Testing <code>com.sun.star.chart.ChartTitle</code>
* service properties :
* <ul>
*  <li><code> TextRotation</code></li>
*  <li><code> String</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartTitle
*/
public class _ChartTitle extends MultiPropertyTest {

    /**
    * Tests property 'TextRotation'.
    */
    public void _TextRotation() {
        testProperty("TextRotation", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                Object ValueToSet = ValueChanger.changePValue(oldValue);
                int ro = ((Integer) ValueToSet).intValue() * 100;
                return new Integer( ro );
            }
        } );
    }
}

