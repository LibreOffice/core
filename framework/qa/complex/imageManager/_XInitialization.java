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



package complex.imageManager;




import com.sun.star.lang.XInitialization;
import lib.TestParameters;

/**
* Testing <code>com.sun.star.lang.XInitialization</code>
* interface methods. <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInitialization.args'</code> (of type <code>Object[]</code>):
*   (<b>Optional</b>) : argument for <code>initialize</code>
*   method. If ommitet zero length array is used. </li>
* <ul> <p>
* Test is multithread compilant. <p>
* Till the present time there was no need to recreate environment
* after this test completion.
*/
public class _XInitialization {


    TestParameters tEnv = null;
    public static XInitialization oObj = null;

    public _XInitialization(TestParameters tEnv, XInitialization oObj) {

        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    /**
    * Test calls the method with 0 length array and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    */
    public boolean _initialize() {
        boolean result = true ;

        try {
            Object[] args = (Object[]) tEnv.get("XInitialization.args");
            if (args==null) {
                oObj.initialize(new Object[0]);
            } else {
                oObj.initialize(args);
            }

        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Exception occurred while method calling.") ;
            result = false ;
        }

        return  result ;
    } // finished _initialize()

    /**
    * Disposes object environment.
    */
    public void after() {
//        disposeEnvironment() ;
    }

} // finished class _XInitialization


