import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;

import com.sun.star.beans.UnknownPropertyException;
/*
 * PathSettingsTest.java
 *
 * Created on 26. March 2003, 08:22
 */

/*
 *
 * @author  Carsten Driesner
 * Provides example code how to access and use the
 * path pathsettings servce.
 */
public class PathSettingsTest extends java.lang.Object {

    /*
     * List of pre-defined path variables supported by
     * the path settings service.
     */
    private static String[] predefinedPathProperties = {
        "Addin",
        "AutoCorrect",
        "AutoText",
        "Backup",
        "Basic",
        "Bitmap",
        "Config",
        "Dictionary",
        "Favorite",
        "Filter",
        "Gallery",
        "Graphic",
        "Help",
        "Linguistic",
        "Module",
        "Palette",
        "Plugin",
        "Storage",
        "Temp",
        "Template",
        "UIConfig",
        "UserConfig",
        "UserDictionary",
        "Work"
    };

    /*
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        XComponentContext xRemoteContext = null;
        XMultiComponentFactory xRemoteServiceManager = null;
        XPropertySet xPathSettingsService = null;

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
                "com.sun.star.comp.framework.PathSettings", xRemoteContext );
            xPathSettingsService = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, pathSubst);

            /* Work with path settings */
            workWithPathSettings( xPathSettingsService );
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    /*
     * Retrieve and set path properties from path settings service
     * @param xPathSettingsService the path settings service
     */
    public static void workWithPathSettings( XPropertySet xPathSettingsService )
    {
        if ( xPathSettingsService != null ) {
            for ( int i=0; i<predefinedPathProperties.length; i++ ) {
                try {
                        /* Retrieve values for path properties from path settings service*/
                        Object aValue = xPathSettingsService.getPropertyValue(
                                            predefinedPathProperties[i] );

                        // getPropertyValue returns an Object, you have to cast it to type that you need
                        String aPath = (String)aValue;
                        System.out.println( "Property="+ predefinedPathProperties[i] +
                                            " Path=" + aPath );
                }
                catch ( com.sun.star.beans.UnknownPropertyException e) {
                    System.out.println( "UnknownPropertyException has been thrown accessing "+predefinedPathProperties[i]);
                }
                catch ( com.sun.star.lang.WrappedTargetException e ) {
                    System.out.println( "WrappedTargetException has been thrown accessing "+predefinedPathProperties[i]);
                }
            }

            // Try to modfiy the work path property. After running this example
            // you should see the new value of "My Documents" in the path options
            // tab page, accessible via "Tools - Options - [Star|Open]Office - Paths".
            // If you want to revert the changes, you can also do it with the path tab page.
            try {
                xPathSettingsService.setPropertyValue( "Work", "$(temp)" );
                String aValue = (String)xPathSettingsService.getPropertyValue( "Work" );
                System.out.println( "The work path should now be " + aValue );
            }
            catch ( com.sun.star.beans.UnknownPropertyException e) {
                System.out.println( "UnknownPropertyException has been thrown accessing PathSettings service");
            }
            catch ( com.sun.star.lang.WrappedTargetException e ) {
                System.out.println( "WrappedTargetException has been thrown accessing PathSettings service");
            }
            catch ( com.sun.star.beans.PropertyVetoException e ) {
                System.out.println( "PropertyVetoException has been thrown accessing PathSettings service");
            }
            catch ( com.sun.star.lang.IllegalArgumentException e ) {
                System.out.println( "IllegalArgumentException has been thrown accessing PathSettings service");
            }
        }
    }
}
