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



class MyUnoObject implements com.sun.star.uno.XInterface
{
    public MyUnoObject() {}

    protected void finalize() throws Throwable {
        super.finalize();
        System.out.println( "finalizer called" );
    }

    public static void main( String args[] ) throws java.lang.InterruptedException
    {
        com.sun.star.uno.XInterface a = new MyUnoObject();
        a = null;

        // ask the garbage collector politely
        System.gc();
        synchronized(  Thread.currentThread() )
        {
            // wait a second
            Thread.currentThread().wait( 1000 );
        }
        System.out.println( "leaving" );

        // It is java VM dependend, whether the finalizer was called or not
    }
}
