import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;

import com.sun.star.util.XStringSubstitution;
import com.sun.star.frame.TerminationVetoException;
import com.sun.star.frame.XTerminateListener;
/*
 * PathSubstitutionTest.java
 *
 * Created on 26. March 2003, 08:22
 */

/*
 *
 * @author  Carsten Driesner
 */
public class PathSubstitutionTest extends java.lang.Object {

    /*
     * List of pre-defined path variables supported by
     * the path substitution service.
     */
    private static String[] predefinedPathVariables = {
        "$(home)","$(inst)","$(prog)","$(temp)","$(user)",
        "$(work)","$(path)","$(lang)","$(langid)","$(vlang)"
    };

    /*
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        XComponentContext xRemoteContext = null;
        XMultiComponentFactory xRemoteServiceManager = null;
        XStringSubstitution xPathSubstService = null;

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

            Object pathSubst = xRemoteServiceManager.createInstanceWithContext(
                "com.sun.star.comp.framework.PathSubstitution", xRemoteContext );
            xPathSubstService = (XStringSubstitution)UnoRuntime.queryInterface(
                XStringSubstitution.class, pathSubst);

            /* Work with path variables */
            workWithPathVariables( xPathSubstService );
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    public static void workWithPathVariables( XStringSubstitution xPathSubstService )
    {
        if ( xPathSubstService != null ) {
            for ( int i=0; i<predefinedPathVariables.length; i++ ) {
                try {
                        /* Retrieve values for pre-defined path variables */
                        String aValue = xPathSubstService.getSubstituteVariableValue(
                                            predefinedPathVariables[i] );
                        System.out.println( "Variable: "+ predefinedPathVariables[i] +
                                            " value=" + aValue );
                }
                catch ( com.sun.star.container.NoSuchElementException e) {
                    System.out.println( "NoSuchElementException has been thrown accessing "+predefinedPathVariables[i]);
                }
            }

            // Check the resubstitution function
            try {
                String aPath = xPathSubstService.getSubstituteVariableValue(
                                            predefinedPathVariables[0] ); // Use $(home) as starting point
                aPath += "/test"; // extend the path
                System.out.println( "Path="+aPath );
                String aResubstPath = xPathSubstService.reSubstituteVariables( aPath );
                System.out.println( "Resubstituted path="+aResubstPath );
            }
            catch ( com.sun.star.container.NoSuchElementException e ) {
                    System.out.println( "NoSuchElementException has been thrown accessing "+predefinedPathVariables[0]);
            }
        }
    }
}
