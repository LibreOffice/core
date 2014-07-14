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

import com.sun.star.container.XIndexReplace;

public class _Text extends MultiPropertyTest {

    protected boolean compare(Object ob1, Object ob2) {

        return super.compare(ob1, ob2);

    }

    public void _NumberingRules() {
        //This property is readOnly
        boolean bResult = false;

        try {
            Object theRules2 = oObj.getPropertyValue( "NumberingRules" );
            if (!(theRules2 instanceof com.sun.star.container.XIndexReplace)) {
                log.println("Property 'NumberingRules' is an instance of "+
                    theRules2.getClass().getName());
                log.println("Expected was an instance of "+
                    "com.sun.star.container.XIndexReplace");
            }
            log.println("Trying to cast property 'NumberingRules' "+
                "to XIndexReplace");
            XIndexReplace theRules = (XIndexReplace)
                oObj.getPropertyValue( "NumberingRules" );
            bResult = true;
        } catch( Exception e ) {
            log.println("Error occurred while trying to get property"+
                " 'NumberingRules'");
            e.printStackTrace( log );
            bResult = false;
        }

        tRes.tested( "NumberingRules", bResult);
    }

}  // finish class _Text



