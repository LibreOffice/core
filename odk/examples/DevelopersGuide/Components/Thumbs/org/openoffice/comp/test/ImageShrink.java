/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

package org.openoffice.comp.test;


/*
 * ImageShrink.java
 *
 * Created on 4. Mai 2002, 20:25
 */

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
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
    // http://wiki.openoffice.org/wiki/Passive_Component_Registration

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
