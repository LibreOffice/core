/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _RotationDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:36:05 $
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

package ifc.drawing;

import lib.MultiPropertyTest;
import lib.Status;

/**
* Testing <code>com.sun.star.drawing.RotationDescriptor</code>
* service properties :
* <ul>
*  <li><code> RotateAngle</code></li>
*  <li><code> ShearAngle</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.RotationDescriptor
*/
public class _RotationDescriptor extends MultiPropertyTest {

    /**
     * Custom tester which increases value by 200
     */
    protected PropertyTester drawMeasureTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            int val = ((Integer)oldValue).intValue() ;
            return new Integer(val + 200) ;
        }
    } ;

    /**
     * !!! DEPRECATED !!!
     */
    public void _RotationPointX() {
        log.println("Testing with custom Property tester") ;
        testProperty("RotationPointX", drawMeasureTester) ;
    }

    /**
     * !!! DEPRECATED !!!
     */
    public void _RotationPointY() {
        log.println("Testing with custom Property tester") ;
        testProperty("RotationPointY", drawMeasureTester) ;
    }

    /**
     * Tests the property with custom tester
     */
    public void _ShearAngle() {

        Object noShear = tEnv.getObjRelation("NoShear");

        if (noShear != null) {
            log.println("This shape type doesn't support shear, see #85556#");
            tRes.tested("ShearAngle",Status.skipped(true));
            return;
        }

        try {
            oObj.setPropertyValue("RotateAngle",new Short((short) 0));
        } catch (Exception e) {
        }
        log.println("Testing with custom Property tester") ;
        testProperty("ShearAngle", drawMeasureTester) ;
    }

    /**
     * Tests the property with custom tester
     */
    public void _RotateAngle() {
        log.println("Testing with custom Property tester") ;
        testProperty("RotateAngle", drawMeasureTester) ;
    }

}  // finish class _RotationDescriptor


