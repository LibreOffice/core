/*
 * TypeDetection6FileFormat.java
 *
 * Created on 26. April 2004, 10:37
 */

package complex.filter.misc;

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import util.utils;

/**
 *
 * @author  cn93815
 */
public class TypeDetection6FileFormat extends ComplexTestCase {

    static XMultiServiceFactory xMSF;

    /**
     * A function to tell the framework, which test functions are available.
     * @return All test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"checkFilterFactory",
                            "checkTypeDetection"};
    }

    /** Create the environment for following tests.
     * Use either a component loader from desktop or
     * from frame
     * @throws Exception Exception
     */
    public void before() throws Exception {

        xMSF = (XMultiServiceFactory)param.getMSF();
        assure("Could not get XMultiServiceFactory", xMSF != null);

    }

    /**
     * call the function <CODE>checkFileFormatSupport</CODE> to test <CODE>com.sun.star.document.FilterFactory</CODE>
     * @see com.sun.star.document.FilterFactory
     */
    public void checkFilterFactory(){
        checkFileFormatSupport("com.sun.star.document.FilterFactory");
    }

    /**
     * call the function <CODE>checkFileFormatSupport</CODE> to test <CODE>com.sun.star.document.TypeDetection</CODE>
     * @see com.sun.star.document.TypeDetection
     */
    public void checkTypeDetection(){
        checkFileFormatSupport("com.sun.star.document.TypeDetection");

    }

    /**
     * test the given service <CODE>serviceName</CODE>.
     * The serve was created and the filter 'TypeDetection6FileFormat' was searched
     * @param serviceName the name of the service to test
     */
    private void checkFileFormatSupport(String serviceName){
        log.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        log.println("testing service '" + serviceName + "'");

        XInterface oObj = null;
        try{
            oObj = getTestObject(serviceName);
            log.println("ImplName: "+utils.getImplName(oObj));
        } catch (java.lang.Exception e){
            failed("could not get test object");
        }
        XNameAccess xNA = (XNameAccess) UnoRuntime.queryInterface
            (XNameAccess.class, oObj);
        String msg = "Could not find filter 'TypeDetection6FileFormat'!";
        msg += "\nMaybe 'TypeDetection6FileFormat.xcu' is not registered.";
        assure(msg, xNA.hasByName("TypeDetection6FileFormat"),CONTINUE);


    }

     /**
     * Creates an instance for the given <CODE>serviceName</CODE>
     * @param serviceName the name of the service which should be created
     * @throws Exception was thrown if creataion failes
     * @return <CODE>XInterface</CODE> of service
     */
    public XInterface getTestObject(String serviceName) throws Exception{

        Object oInterface = xMSF.createInstance(serviceName);

        if (oInterface == null) {
            failed("Service wasn't created") ;
            throw new Exception("could not create service '"+serviceName+"'");
        }
        return (XInterface) oInterface;
    }


}
