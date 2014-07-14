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



package complex.tdoc;

import lib.MultiMethodTest;

import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import share.LogWriter;

/**
* Testing <code>com.sun.star.lang.XTypeProvider</code>
* interface methods :
* <ul>
*  <li><code> getTypes()</code></li>
*  <li><code> getImplementationId()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.lang.XTypeProvider
*/
public class _XTypeProvider {

    public static XTypeProvider oObj = null;
    public static Type[] types = null;
    public LogWriter log = null;

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public boolean _getImplementationId() {
        boolean result = true;
        log.println("testing getImplementationId() ... ");

        log.println("The ImplementationId is "+oObj.getImplementationId());
        result = true;

        return result;

    } // end getImplementationId()


    /**
    * ?alls the method and checks the return value.<p>
    * Has <b>OK</b> status if one of the return value equals to the
    * type <code>com.sun.star.lang.XTypeProvider</code>.
    */
    public boolean _getTypes() {
        boolean result = false;
        log.println("getting Types...");
        types = oObj.getTypes();
        for (int i=0;i<types.length;i++) {
            int k = i+1;
            log.println(k+". Type is "+types[i].toString());
            if (types[i].toString().equals
                    ("Type[com.sun.star.lang.XTypeProvider]")) {
                result = true;
            }
        }
        if (!result) {
            log.println("Component must provide Type "
                +"<com.sun.star.lang.XTypeProvider>");
        }

        return result;

    } // end getTypes()

}

