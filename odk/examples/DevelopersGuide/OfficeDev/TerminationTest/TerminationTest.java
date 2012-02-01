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



import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.frame.XDesktop;

/**
 *
 * @author  dschulten
 */
public class TerminationTest extends java.lang.Object {

    private static boolean atWork = false;
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        XComponentContext xRemoteContext = null;
        XMultiComponentFactory xRemoteServiceManager = null;
        XDesktop xDesktop = null;

        try {
            // get the remote office context. If necessary a new office
            // process is started
            xRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            xRemoteServiceManager = xRemoteContext.getServiceManager();

            Object desktop = xRemoteServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xRemoteContext);
            xDesktop = (XDesktop)UnoRuntime.queryInterface(XDesktop.class, desktop);

            TerminateListener terminateListener = new TerminateListener();
            xDesktop.addTerminateListener(terminateListener);

            atWork = true;
            // try to terminate while we are at work
            boolean terminated = xDesktop.terminate();
            System.out.println("The Office " +
                (terminated == true ?
                 "has been terminated" :
                 "is still running, we are at work"));

            // no longer at work
            atWork = false;
            // once more: try to terminate
            terminated = xDesktop.terminate();
            System.out.println("The Office " +
                (terminated == true ?
                    "has been terminated" :
                    "is still running. Someone else prevents termination, " +
                    "e.g. the quickstarter"));
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }


    }
    public static boolean isAtWork() {
        return atWork;
    }

}
