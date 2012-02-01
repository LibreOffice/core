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


import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.test.XSomethingB;
import com.sun.star.uno.Type;

// TestComponentB implements all necessary interfaces self, this is only
// for demonstration. More convenient is to use the impelmentation WeakBase or
// ComponentBase, see implementation of TestComponentA.
public class TestComponentB implements XTypeProvider, XServiceInfo, XSomethingB {
    static final String __serviceName= "com.sun.star.test.SomethingB";

       static byte[] _implementationId;
    private XComponentContext context;
    private Object[] args;

    public TestComponentB(XComponentContext context, Object[] args) {
        this.context= context;
        this.args= args;
    }

    // XSomethingB
    public String methodTwo(String val) {
        if (args.length > 0 && args[0] instanceof String )
            return (String) args[0];
        return val;
    }

    //XTypeProvider
    public com.sun.star.uno.Type[] getTypes(  ) {
        Type[] retValue= new Type[3];
        retValue[0]= new Type( XServiceInfo.class);
        retValue[1]= new Type( XTypeProvider.class);
        retValue[2]= new Type( XSomethingB.class);
        return retValue;
    }
    //XTypeProvider
    synchronized public byte[] getImplementationId(  ) {
        if (_implementationId == null) {
            _implementationId= new byte[16];
            int hash = hashCode();
            _implementationId[0] = (byte)(hash & 0xff);
            _implementationId[1] = (byte)((hash >>> 8) & 0xff);
            _implementationId[2] = (byte)((hash >>> 16) & 0xff);
            _implementationId[3] = (byte)((hash >>>24) & 0xff);
        }
        return _implementationId;
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
