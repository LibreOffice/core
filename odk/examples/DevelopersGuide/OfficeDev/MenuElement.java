import com.sun.star.ui.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.RuntimeException;

// A helper class to determine the menu element type
public class MenuElement
{
    static public boolean IsMenuEntry( com.sun.star.beans.XPropertySet xMenuElement ) {
        com.sun.star.lang.XServiceInfo xServiceInfo =
            (com.sun.star.lang.XServiceInfo)UnoRuntime.queryInterface(
                com.sun.star.lang.XServiceInfo.class, xMenuElement );

        return xServiceInfo.supportsService( "com.sun.star.ui.ActionTrigger" );
    }

    static public boolean IsMenuSeparator( com.sun.star.beans.XPropertySet xMenuElement ) {
        com.sun.star.lang.XServiceInfo xServiceInfo =
            (com.sun.star.lang.XServiceInfo)UnoRuntime.queryInterface(
                com.sun.star.lang.XServiceInfo.class, xMenuElement );

        return xServiceInfo.supportsService( "com.sun.star.ui.ActionTriggerSeparator" );
    }
}