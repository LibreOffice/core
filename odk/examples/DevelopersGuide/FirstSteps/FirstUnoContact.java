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



public class FirstUnoContact {

    public static void main(String[] args) {
        try {
            // get the remote office component context
            com.sun.star.uno.XComponentContext xContext =
                com.sun.star.comp.helper.Bootstrap.bootstrap();

            System.out.println("Connected to a running office ...");

            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            String available = (xMCF != null ? "available" : "not available");
            System.out.println( "remote ServiceManager is " + available );
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }
}
