/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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


