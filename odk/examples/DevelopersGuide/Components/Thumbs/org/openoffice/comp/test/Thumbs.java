/*
 * Thumbs.java
 *
 * contains static component methods for multiple service implementations
 * in thumbs.jar
 * and a main method to test service implementations
 *
 * Created on 4. Mai 2002, 20:23
 */

package org.openoffice.comp.test;

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import org.openoffice.test.XImageShrink;
//import com.sun.star.test.XSomethingA;


/**
 *
 * @author  dschulten
 */
public class Thumbs {

    static private XComponentContext xRemoteContext = null;
    static private XMultiComponentFactory xRemoteServiceManager = null;

    /** Creates a new instance of Thumbs */
    public Thumbs() {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            xRemoteServiceManager = getRemoteServiceManager(
                "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager");

            Object ob = xRemoteServiceManager.createInstanceWithContext(
                "org.openoffice.test.ImageShrink", xRemoteContext);

           XImageShrink xImageShrink = (XImageShrink) UnoRuntime.queryInterface(
                XImageShrink.class, ob);
           xImageShrink.setSourceDirectory("file:///C:/");
           System.out.println(xImageShrink.getSourceDirectory());

 /*
            System.out.println((xImageShrink.getSourceDirectory()));
 */
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }

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
                                                    ImageShrink.__serviceName,
                                                    regKey);
    }

    protected static XMultiComponentFactory getRemoteServiceManager(String unoUrl) throws java.lang.Exception {
        if (xRemoteContext == null) {
            // First step: create local component context, get local servicemanager and
            // ask it to create a UnoUrlResolver object with an XUnoUrlResolver interface
            XComponentContext xLocalContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();

            Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext );
            // query XUnoUrlResolver interface from urlResolver object
            XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                XUnoUrlResolver.class, urlResolver );

            // Second step: use xUrlResolver interface to import the remote StarOffice.ServiceManager,
            // retrieve its property DefaultContext and get the remote servicemanager
            Object initialObject = xUnoUrlResolver.resolve( unoUrl );
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, initialObject);
            Object context = xPropertySet.getPropertyValue("DefaultContext");
            xRemoteContext = (XComponentContext)UnoRuntime.queryInterface(
                XComponentContext.class, context);
        }
        return xRemoteContext.getServiceManager();
    }
}

