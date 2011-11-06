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


package com.sun.star.lib.uno.protocols.urp;



class TestObject implements TestXInterface {
    public void method1( /*IN*/java.lang.Object itf ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void method2( /*OUT*/java.lang.Object[] itf ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void method3( /*INOUT*/java.lang.Object[] itf ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public Object method4(  ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return null;
    }

    public Object returnAny(  ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return null;
    }


    public void method() throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void methodWithInParameter( /*IN*/String text ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void methodWithOutParameter( /*OUT*/String[] text ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public void methodWithInOutParameter( /*INOUT*/String[] text ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
    }

    public String methodWithResult(  ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return "TestObject_resultString";
    }

    public String MethodWithIn_Out_InOut_Paramters_and_result( /*IN*/String text, /*OUT*/String[] outtext, /*INOUT*/String[] inouttext ) throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException {
        return "TestObject_resultString";
    }
}

