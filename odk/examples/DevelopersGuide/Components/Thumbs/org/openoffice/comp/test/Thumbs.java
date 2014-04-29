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


package org.openoffice.comp.test;

import com.sun.star.uno.UnoRuntime;


/**
 * Test class for testing the ImageShrink service skeleton.
 * Note: the image shrink functionality is not implemented
 */
public class Thumbs {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            // get the remote office component context
            com.sun.star.uno.XComponentContext xRemoteContext =
                com.sun.star.comp.helper.Bootstrap.bootstrap();

            System.out.println("Connected to a running office ...");

            // use the generated default create method to instantiate a
            // new ImageShrink object
            org.openoffice.test.XImageShrinkFilter xImageShrinkFilter =
                org.openoffice.test.ImageShrink.create(xRemoteContext);

            System.out.println("ImageShrink component successfully instantiated");

            java.io.File f = new java.io.File(".");
            System.out.println("set SourceDrectory ...");
            xImageShrinkFilter.setSourceDirectory(f.getCanonicalPath());

            System.out.println("source Directory = "
                               + xImageShrinkFilter.getSourceDirectory());
        }
        catch (java.lang.Exception e){
            System.err.println("Error: " + e);
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }
}

