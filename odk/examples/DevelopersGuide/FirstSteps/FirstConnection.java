import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;


public class FirstConnection extends java.lang.Object {

    private XComponentContext xRemoteContext = null;
    private XMultiComponentFactory xRemoteServiceManager = null;

    public static void main(String[] args) {
        FirstConnection firstConnection1 = new FirstConnection();
        try {
            firstConnection1.useConnection();
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    protected void useConnection() throws java.lang.Exception {
        try {
            xRemoteServiceManager = this.getRemoteServiceManager(
                    "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager");
            String available = (null != xRemoteServiceManager ? "available" : "not available");
            System.out.println( "remote ServiceManager is " + available );
        }
        catch( com.sun.star.lang.DisposedException e ) { //works from Patch 1
            xRemoteContext = null;
            throw e;
        }
    }

    protected XMultiComponentFactory getRemoteServiceManager(String unoUrl) throws java.lang.Exception {
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
            //XUnoUrlResolver xUnoUrlResolver = (XUnoUrlResolver)urlResolver;

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