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



import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XSet;
import com.sun.star.lang.XServiceInfo;

public class TestMinimalComponent {
    public static void main(String args[]) {
        com.sun.star.uno.XComponentContext xContext = null;

        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if( xContext != null )
                System.out.println("Connected to a running office ...");

            XServiceInfo xSIMinimalComponent =
                org.openoffice.MinimalComponent.create(xContext);

            System.out.println("\nXServiceInfo is used to get the implementation" +
                               " name: " +
                               xSIMinimalComponent.getImplementationName() +
                               "\nOk\n");
            xContext = null;

            System.exit(0);
        }
        catch( Exception e ) {
            System.err.println( e );
            e.printStackTrace();
            System.exit(1);
        }
    }
}
