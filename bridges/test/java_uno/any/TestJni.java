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



package test.java_uno.anytest;

public class TestJni
{
    static { System.loadLibrary( "test_javauno_any" ); }
    private static native XTransport create_jni_transport(ClassLoader loader);

    public static void main( String args [] )
    {
        if (TestAny.test(
                create_jni_transport(TestJni.class.getClassLoader()), false ))
        {
            System.out.println( "jni any test succeeded." );
        }
        else
        {
            System.err.println( "jni any test failed!" );
            System.exit( 1 );
        }
    }
}
