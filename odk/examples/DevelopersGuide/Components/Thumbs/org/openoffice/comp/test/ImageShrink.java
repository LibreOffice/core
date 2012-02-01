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


/*
 * ImageShrink.java
 *
 * Created on 4. Mai 2002, 20:25
 */


/**
 *
 * @author  dschulten
 */

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lib.uno.helper.WeakBase;

public class ImageShrink extends WeakBase
        implements com.sun.star.lang.XServiceInfo,
                   org.openoffice.test.XImageShrinkFilter {

    com.sun.star.uno.XComponentContext xComponentContext = null;


    // maintain a static implementation id for all instances of ImageShrink
    // initialized by the first call to getImplementationId()
    static byte[] _implementationId;


    // hold the service name in a private static member variable of the class
    protected static final String __serviceName = "org.openoffice.test.ImageShrink";


    String destDir = "";
    String sourceDir = "";
    boolean cancel = false;
    com.sun.star.awt.Size dimension = new com.sun.star.awt.Size();

    /** Creates a new instance of ImageShrink */
    public ImageShrink() {
    }


        // static __getServiceFactory() Implementation
    public static XSingleServiceFactory __getServiceFactory(String implName,
            XMultiServiceFactory multiFactory,
            com.sun.star.registry.XRegistryKey regKey)    {

        com.sun.star.lang.XSingleServiceFactory xSingleServiceFactory = null;
        if (implName.equals( ImageShrink.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory( ImageShrink.class,
                  ImageShrink.__serviceName,
                  multiFactory,
                  regKey);

        return xSingleServiceFactory;
    }

    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)          {
//         //System.out.println(ImageShrink.class.getName());
//         return FactoryHelper.writeRegistryServiceInfo( ImageShrink.class.getName(),
//                                                     __serviceName,
//                                                     regKey);
//     }

    // XFilter implementation  (a sub-interface of XImageShrinkFilter)
    public void cancel() {
        cancel = true;
    }

    public boolean filter(com.sun.star.beans.PropertyValue[] propertyValue) {
        // while cancel = false,
        // scale images found in sourceDir according to dimension and
        // write them to destDir, using the image file format given in


        // []propertyValue
        return true;
    }

    // XImageShrink implementation (a sub-interface of XImageShrinkFilter)
    public String getDestinationDirectory() {
        return destDir;
    }

    public com.sun.star.awt.Size getDimension() {
        return dimension;
    }

    public String getSourceDirectory() {
        return sourceDir;
    }

    public void setDestinationDirectory(String str) {
        destDir = str;
    }

    public void setDimension(com.sun.star.awt.Size size) {
        dimension = size;
    }

    public void setSourceDirectory(String str) {
        sourceDir = str;
    }

    //XServiceInfo implementation
    public String getImplementationName(  ) {
        return getClass().getName();
    }

    public boolean supportsService(String serviceName) {
        if ( serviceName.equals( __serviceName))
            return true;
        return false;
    }

    public String[] getSupportedServiceNames(  ) {
        return new String[] { __serviceName };
    }

}
