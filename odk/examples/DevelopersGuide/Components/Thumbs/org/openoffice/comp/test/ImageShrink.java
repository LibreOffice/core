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
                   org.openoffice.test.XImageShrink,
                   com.sun.star.document.XFilter  {

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
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)          {
        //System.out.println(ImageShrink.class.getName());
        return FactoryHelper.writeRegistryServiceInfo( ImageShrink.class.getName(),
                                                    __serviceName,
                                                    regKey);
    }

    // XFilter implementation
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

    // XImageShrink implementation
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
        String[] retValue= new String[0];
        retValue[0]= __serviceName;
        return retValue;
    }

}
