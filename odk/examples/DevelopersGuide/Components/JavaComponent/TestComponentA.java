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


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.test.XSomethingA;
import com.sun.star.uno.Type;

// TestComponentA use the implementation helper WeakBase
public class TestComponentA extends WeakBase implements XServiceInfo, XSomethingA {
    static final String __serviceName= "com.sun.star.test.SomethingA";

       static byte[] _implementationId;

    public TestComponentA() {
    }

    // XSomethingA
    public String methodOne(String val) {
        return val;
    }

    //XServiceInfo
    public String getImplementationName(  ) {
        return getClass().getName();
    }
    // XServiceInfo
    public boolean supportsService( /*IN*/String serviceName ) {
        if ( serviceName.equals( __serviceName))
            return true;
        return false;
    }
    //XServiceInfo
    public String[] getSupportedServiceNames(  ) {
        String[] retValue= new String[0];
        retValue[0]= __serviceName;
        return retValue;
    }

}
