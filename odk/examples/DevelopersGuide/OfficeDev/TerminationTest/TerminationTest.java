import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;

import com.sun.star.frame.XDesktop;
import com.sun.star.frame.TerminationVetoException;
import com.sun.star.frame.XTerminateListener;
/*
 * TerminationTest.java
 *
 * Created on 11. Oktober 2002, 15:25
 */

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
            // connect
            XComponentContext xLocalContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);
            XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();
            Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", xLocalContext );
            XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                XUnoUrlResolver.class, urlResolver );
            Object initialObject = xUnoUrlResolver.resolve(
                "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" );
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, initialObject);
            Object context = xPropertySet.getPropertyValue("DefaultContext");
            xRemoteContext = (XComponentContext)UnoRuntime.queryInterface(
                XComponentContext.class, context);
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
