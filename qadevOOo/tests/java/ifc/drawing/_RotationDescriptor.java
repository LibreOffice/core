/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


