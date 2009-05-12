package contextMenuInterceptor;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.text.XTextDocument;
import com.sun.star.ui.XContextMenuInterceptor;
import com.sun.star.ui.XContextMenuInterception;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Exception;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XViewSettingsSupplier;
import complexlib.ComplexTestCase;
import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.PrintWriter;
import share.LogWriter;
import util.AccessibilityTools;
import util.DesktopTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;

/**
 *
 */
public class CheckContextMenuInterceptor extends ComplexTestCase {
    XMultiServiceFactory xMSF = null;
    XFrame xFrame = null;
    Point point = null;
    XWindow xWindow = null;

    public void before() {
        xMSF = (XMultiServiceFactory)param.getMSF();
    }

    public void after() {
        log.println("release the popup menu");
        try {
            Robot rob = new Robot();
            int x = point.X;
            int y = point.Y;
            rob.mouseMove(x, y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }

        com.sun.star.util.XCloseable xClose = (com.sun.star.util.XCloseable)UnoRuntime.queryInterface(
                com.sun.star.util.XCloseable.class, xFrame);

        try
        {
                xClose.close(false);
        }
        catch(com.sun.star.util.CloseVetoException exVeto)
        {
            failed("Test frame couldn't be closed successfully.");
        }

        xFrame = null;
    }

    public String[] getTestMethodNames() {
        return new String[]{"checkContextMenuInterceptor"};
    }

    public void checkContextMenuInterceptor() {
        log.println(" **** Context Menu Interceptor *** ");

        try {
            // intialize the test document
            com.sun.star.lang.XComponent xDrawDoc = DrawTools.createDrawDoc(xMSF);

            SOfficeFactory SOF = SOfficeFactory.getFactory( xMSF);
            XShape oShape = SOF.createShape(xDrawDoc,5000,5000,1500,1000,"GraphicObject");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

            com.sun.star.frame.XModel xModel =
                (com.sun.star.frame.XModel)UnoRuntime.queryInterface(
                com.sun.star.frame.XModel.class, xDrawDoc);

            // get the frame for later usage
            xFrame = xModel.getCurrentController().getFrame();

            // ensure that the document content is optimal visible
            DesktopTools.zoomToEntirePage(xDrawDoc);

            XBitmap xBitmap = null;

            // adding graphic as ObjRelation for GraphicObjectShape
            XPropertySet oShapeProps = (XPropertySet)
                                UnoRuntime.queryInterface(XPropertySet.class,oShape);
            log.println( "Inserting a shape into the document");

            try
            {
                oShapeProps.setPropertyValue(
                    "GraphicURL",util.utils.getFullTestURL("space-metal.jpg"));
                xBitmap = (XBitmap) AnyConverter.toObject(
                    new Type(XBitmap.class),oShapeProps.getPropertyValue
                        ("GraphicObjectFillBitmap"));
            } catch (com.sun.star.lang.WrappedTargetException e) {
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            } catch (com.sun.star.beans.PropertyVetoException e) {
            } catch (com.sun.star.beans.UnknownPropertyException e) {
            }

            // reuse the frame
            com.sun.star.frame.XController xController = xFrame.getController();
            com.sun.star.ui.XContextMenuInterception xContextMenuInterception = null;
            com.sun.star.ui.XContextMenuInterceptor xContextMenuInterceptor = null;

            if ( xController != null )
            {
                log.println( "Creating context menu interceptor");

                // add our context menu interceptor
                xContextMenuInterception =
                    (com.sun.star.ui.XContextMenuInterception)UnoRuntime.queryInterface(
                    com.sun.star.ui.XContextMenuInterception.class, xController );

                if( xContextMenuInterception != null )
                {
                    ContextMenuInterceptor aContextMenuInterceptor = new ContextMenuInterceptor( xBitmap );
                    xContextMenuInterceptor =
                        (com.sun.star.ui.XContextMenuInterceptor)UnoRuntime.queryInterface(
                            com.sun.star.ui.XContextMenuInterceptor.class, aContextMenuInterceptor );

                    log.println( "Register context menu interceptor");
                    xContextMenuInterception.registerContextMenuInterceptor( xContextMenuInterceptor );
                }
            }

        //  utils.shortWait(10000);

            openContextMenu((XModel) UnoRuntime.queryInterface(XModel.class, xDrawDoc));

            checkHelpEntry();

            // remove our context menu interceptor
            if ( xContextMenuInterception != null &&
                 xContextMenuInterceptor  != null    ) {
                log.println( "Release context menu interceptor");
                xContextMenuInterception.releaseContextMenuInterceptor(
                    xContextMenuInterceptor );
            }
        }
        catch ( com.sun.star.uno.RuntimeException ex ) {
            ex.printStackTrace((PrintWriter)log);
            failed("Runtime exception caught!" + ex.getMessage());
        }
        catch ( java.lang.Exception ex ) {
            ex.printStackTrace((PrintWriter)log);
            failed("Java lang exception caught!" + ex.getMessage());
        }
    }

    private void checkHelpEntry(){
        XInterface toolkit = null;

        log.println("get accesibility...");
        try{
            toolkit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e){
            log.println("could not get Toolkit " + e.toString());
        }
        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                              XExtendedToolkit.class, toolkit);

        XAccessible xRoot = null;

        AccessibilityTools at = new AccessibilityTools();

        try {
            xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                          tk.getTopWindow(0));

            xRoot = at.getAccessibleObject(xWindow);

//            at.printAccessibleTree((PrintWriter)log, xRoot, param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
            at.printAccessibleTree((PrintWriter)log, xRoot, true);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            log.println("Couldn't get Window");
        }

        XAccessibleContext oPopMenu = at.getAccessibleObjectForRole(xRoot, AccessibleRole.POPUP_MENU);

        log.println("ImplementationName: " + util.utils.getImplName(oPopMenu));

        XAccessible xHelp = null;
        try{
            log.println("Try to get first entry of context menu...");
            xHelp = oPopMenu.getAccessibleChild(0);

        } catch (IndexOutOfBoundsException e){
            failed("Not possible to get first entry of context menu");
        }

        if (xHelp == null) failed("first entry of context menu is NULL");

        XAccessibleContext xHelpCont = xHelp.getAccessibleContext();

        if ( xHelpCont == null )
            failed("No able to retrieve accessible context from first entry of context menu");

        String aAccessibleName = xHelpCont.getAccessibleName();
        if ( !aAccessibleName.equals( "Help" )) {
            log.println("Accessible name found = "+aAccessibleName );
            failed("First entry of context menu is not from context menu interceptor");
        }

        try
        {
            log.println("try to get first children of Help context...");
            XAccessible xHelpChild = xHelpCont.getAccessibleChild(0);

        } catch (IndexOutOfBoundsException e){
            failed("not possible to get first children of Help context");
        }

    }

    private void openContextMenu(XModel aModel){

        log.println("try to open contex menu...");
        AccessibilityTools at = new AccessibilityTools();

        xWindow = at.getCurrentWindow(xMSF, aModel);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XInterface oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        XAccessibleComponent window = (XAccessibleComponent) UnoRuntime.queryInterface(
                                              XAccessibleComponent.class, oObj);

        point = window.getLocationOnScreen();
        Rectangle rect = window.getBounds();

        log.println("klick mouse button...");
        try {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2);
            int y = point.Y + (rect.Height / 2);
            rob.mouseMove(x, y);
            System.out.println("Press Button");
            rob.mousePress(InputEvent.BUTTON3_MASK);
            System.out.println("Release Button");
            rob.mouseRelease(InputEvent.BUTTON3_MASK);
            System.out.println("done");
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }

        utils.shortWait(3000);

    }
}
