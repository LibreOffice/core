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



import sun.awt.*;
import com.sun.star.awt.*;

public class SystemWindowAdapter
{
    static public java.awt.Frame createFrame( int windowHandle )
    {
        java.awt.Frame aFrame;

        // we're initialized with the operating system window handle
        // as the parameter. We then generate a dummy Java frame with
        // that window as the parent, to fake a root window for the
        // Java implementation.

         // now, we're getting slightly system dependent here.
         String os = (String) System.getProperty( "os.name" );

         // create the embedded frame
         if( os.startsWith( "Windows" ) )
             aFrame = new sun.awt.windows.WEmbeddedFrame( windowHandle );
         else
             throw new com.sun.star.uno.RuntimeException();

        return aFrame;
    }
}
