import com.sun.star.ui.*;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Exception;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XController;


public class ContextMenuInterceptor implements XContextMenuInterceptor {

    /**
     *Description of the Method
     *
     *@param  args  Description of Parameter
     *@since
     */
    public static void main(String args[])
    {
        try {
            OfficeConnect aConnect = OfficeConnect.createConnection("localhost", "8100");

            com.sun.star.frame.XDesktop xDesktop =
                (com.sun.star.frame.XDesktop)aConnect.createRemoteInstance(
                    com.sun.star.frame.XDesktop.class, "com.sun.star.frame.Desktop");
            com.sun.star.frame.XFrame xFrame = xDesktop.getCurrentFrame();
            if ( xFrame != null ) {
                com.sun.star.frame.XController xController = xFrame.getController();
                if ( xController != null ) {
                    com.sun.star.ui.XContextMenuInterception xContextMenuInterception =
                        (com.sun.star.ui.XContextMenuInterception)UnoRuntime.queryInterface(
                            com.sun.star.ui.XContextMenuInterception.class, xController );
                    if( xContextMenuInterception != null ) {
                        ContextMenuInterceptor aContextMenuInterceptor = new ContextMenuInterceptor();
                        com.sun.star.ui.XContextMenuInterceptor xContextMenuInterceptor =
                            (com.sun.star.ui.XContextMenuInterceptor)UnoRuntime.queryInterface(
                                com.sun.star.ui.XContextMenuInterceptor.class, aContextMenuInterceptor );
                        xContextMenuInterception.registerContextMenuInterceptor( xContextMenuInterceptor );

                        System.out.println( "\n ... all context menus of the current frame contains " +
                                            "now a 'Help' entry with the\n     submenus 'Content', " +
                                            "'Help Agent' and 'Tips'.\n\n Press 'Return' to remove the " +
                                            "context menu interceptor and finish the example!");

                        if (System.in.read() > 0) {
                            xContextMenuInterception.releaseContextMenuInterceptor(
                                xContextMenuInterceptor );
                            System.out.println( " ... context menu interceptor removed!" );
                        }
                    }
                }
            }
        }
        catch ( com.sun.star.uno.RuntimeException ex ) {
            // something strange has happend!
            System.out.println( " Sample caught exception! " + ex );
            System.exit(1);
        }
        catch ( java.lang.Throwable ex ) {
            // catch java exceptions – do something useful
            System.out.println( " Sample caught exception! " + ex );
            System.exit(1);
        }

        System.out.println(" ... exit!\n");
        System.exit( 0 );
    }

    /**
     *Description of the Method
     *
     *@param  args  Description of Parameter
     *@since
     */
    public ContextMenuInterceptorAction notifyContextMenuExecute(
             com.sun.star.ui.ContextMenuExecuteEvent aEvent ) throws RuntimeException {

        try {

            // Retrieve context menu container and query for service factory to
            // create sub menus, menu entries and separators
            com.sun.star.container.XIndexContainer xContextMenu = aEvent.ActionTriggerContainer;
            com.sun.star.lang.XMultiServiceFactory xMenuElementFactory =
                (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                com.sun.star.lang.XMultiServiceFactory.class, xContextMenu );
            if ( xMenuElementFactory != null ) {
                // create root menu entry and sub menu
                com.sun.star.beans.XPropertySet xRootMenuEntry =
                    (XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class,
                        xMenuElementFactory.createInstance( "com.sun.star.ui.ActionTrigger" ));

                // create a line separator for our new help sub menu
                com.sun.star.beans.XPropertySet xSeparator =
                    (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class,
                        xMenuElementFactory.createInstance( "com.sun.star.ui.ActionTriggerSeparator" ));

                Short aSeparatorType = new Short( ActionTriggerSeparatorType.LINE );
                xSeparator.setPropertyValue( "SeparatorType", (Object)aSeparatorType );

                // query sub menu for index container to get access
                com.sun.star.container.XIndexContainer xSubMenuContainer =
                    (com.sun.star.container.XIndexContainer)UnoRuntime.queryInterface(
                        com.sun.star.container.XIndexContainer.class,
                            xMenuElementFactory.createInstance(
                                "com.sun.star.ui.ActionTriggerContainer" ));

                // intialize root menu entry
                xRootMenuEntry.setPropertyValue( "Text", new String( "Help" ));
                xRootMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5410" ));
                xRootMenuEntry.setPropertyValue( "HelpURL", new String( "5410" ));
                xRootMenuEntry.setPropertyValue( "SubContainer", (Object)xSubMenuContainer );

                // create menu entries for the new sub menu

                // intialize help/content menu entry
                XPropertySet xMenuEntry = (XPropertySet)UnoRuntime.queryInterface(
                                              XPropertySet.class, xMenuElementFactory.createInstance(
                                                  "com.sun.star.ui.ActionTrigger" ));

                xMenuEntry.setPropertyValue( "Text", new String( "Content" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5401" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5401" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 0, (Object)xMenuEntry );

                // intialize help/help agent
                xMenuEntry = (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                                 com.sun.star.beans.XPropertySet.class,
                                     xMenuElementFactory.createInstance(
                                         "com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Help Agent" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5962" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5962" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 1, (Object)xMenuEntry );

                // intialize help/tips
                xMenuEntry = (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                                 com.sun.star.beans.XPropertySet.class,
                                     xMenuElementFactory.createInstance(
                                         "com.sun.star.ui.ActionTrigger" ));
                xMenuEntry.setPropertyValue( "Text", new String( "Tips" ));
                xMenuEntry.setPropertyValue( "CommandURL", new String( "slot:5404" ));
                xMenuEntry.setPropertyValue( "HelpURL", new String( "5404" ));

                // insert menu entry to sub menu
                xSubMenuContainer.insertByIndex( 2, (Object)xMenuEntry );

                // add separator into the given context menu
                xContextMenu.insertByIndex( 0, (Object)xSeparator );

                // add new sub menu into the given context menu
                xContextMenu.insertByIndex( 0, (Object)xRootMenuEntry );

                // The controller should execute the modified context menu and stop notifying other
                // interceptors.
                return com.sun.star.ui.ContextMenuInterceptorAction.EXECUTE_MODIFIED;
            }
        }
        catch ( com.sun.star.beans.UnknownPropertyException ex ) {
            // do something useful
            // we used a unknown property
        }
        catch ( com.sun.star.lang.IndexOutOfBoundsException ex ) {
            // do something useful
            // we used an invalid index for accessing a container
        }
        catch ( com.sun.star.uno.Exception ex ) {
            // something strange has happend!
        }
        catch ( java.lang.Throwable ex ) {
            // catch java exceptions – do something useful
        }

        return com.sun.star.ui.ContextMenuInterceptorAction.IGNORED;
    }
}
