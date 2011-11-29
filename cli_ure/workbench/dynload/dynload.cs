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

using System;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.bridge;
using unoidl.com.sun.star.frame;

class DynLoad
{

    static void Main(string[] args)
    {
        connect(args);
    }
    
/** Connect to a running office that is accepting connections.
        @return  The ServiceManager to instantiate office components. */
    static private XMultiServiceFactory connect( string[] args )
    {
        if (args.Length == 0)
            Console.WriteLine("You need to provide a file URL to the office" +
                              " program folder\n");
        System.Collections.Hashtable ht = new System.Collections.Hashtable();
        ht.Add( "SYSBINDIR", args[ 0 ] );
        XComponentContext xContext =
            uno.util.Bootstrap.defaultBootstrap_InitialComponentContext(
                 args[ 0 ] + "/uno.ini", ht.GetEnumerator() );
        
        if (xContext != null)
            Console.WriteLine("Successfully created XComponentContext\n");
        else
            Console.WriteLine("Could not create XComponentContext\n");
       
       return null;
    }
}
