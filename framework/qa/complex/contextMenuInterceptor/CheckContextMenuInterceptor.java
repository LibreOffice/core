package complex.contextMenuInterceptor;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.XContextMenuInterception;
import com.sun.star.ui.XContextMenuInterceptor;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.File;
import util.AccessibilityTools;
import util.DesktopTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;
import org.openoffice.test.OfficeFileUrl;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

/**
 *
 */
public class CheckContextMenuInterceptor
{

    XMultiServiceFactory xMSF = null;
    XFrame xFrame = null;
    Point point = null;
    XWindow xWindow = null;
    com.sun.star.lang.XComponent xDrawDoc;

    @Before
    public void before()
    {
        xMSF = getMSF();
    }

    @After
    public void after()
    {
        System.out.println("release the popup menu");
        try
        {
            Robot rob = new Robot();
            int x = point.X;
            int y = point.Y;
            rob.mouseMove(x, y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
        }
        catch (java.awt.AWTException e)
        {
            System.out.println("couldn't press mouse button");
        }

        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xFrame);

        try
        {
            xClose.close(true);
        }
        catch (com.sun.star.util.CloseVetoException exVeto)
        {
            fail("Test frame couldn't be closed successfully.");
        }

        xFrame = null;

//        xClose = UnoRuntime.queryInterface(XCloseable.class, xDrawDoc);
//        try
//        {
//            xClose.close(true);
//        }
//        catch (com.sun.star.util.CloseVetoException exVeto)
//        {
//            fail("Test DrawDoc couldn't be closed successfully.");
//        }

    }

//    public String[] getTestMethodNames() {
//        return new String[]{"checkContextMenuInterceptor"};
//    }
    @Test
    public void checkContextMenuInterceptor()
    {
        System.out.println(" **** Context Menu Interceptor *** ");

        try
        {
            // intialize the test document
            xDrawDoc = DrawTools.createDrawDoc(xMSF);

            SOfficeFactory SOF = SOfficeFactory.getFactory(xMSF);
            XShape oShape = SOF.createShape(xDrawDoc, 5000, 5000, 1500, 1000, "GraphicObject");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc, 0)).add(oShape);

            com.sun.star.frame.XModel xModel =
                    UnoRuntime.queryInterface(com.sun.star.frame.XModel.class, xDrawDoc);

            // get the frame for later usage
            xFrame = xModel.getCurrentController().getFrame();

            // ensure that the document content is optimal visible
            DesktopTools.zoomToEntirePage(xDrawDoc);

            XBitmap xBitmap = null;

            // adding graphic as ObjRelation for GraphicObjectShape
            XPropertySet oShapeProps = UnoRuntime.queryInterface(XPropertySet.class, oShape);
            System.out.println("Inserting a shape into the document");

            try
            {
                String sFile = OfficeFileUrl.getAbsolute(new File("space-metal.jpg"));
                // String sFile = util.utils.getFullTestURL("space-metal.jpg");
                oShapeProps.setPropertyValue("GraphicURL", sFile);
                Object oProp = oShapeProps.getPropertyValue("GraphicObjectFillBitmap");
                xBitmap = (XBitmap) AnyConverter.toObject(new Type(XBitmap.class), oProp);
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
            }
            catch (com.sun.star.beans.PropertyVetoException e)
            {
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
            }

            // reuse the frame
            com.sun.star.frame.XController xController = xFrame.getController();
            XContextMenuInterception xContextMenuInterception = null;
            XContextMenuInterceptor xContextMenuInterceptor = null;

            if (xController != null)
            {
                System.out.println("Creating context menu interceptor");

                // add our context menu interceptor
                xContextMenuInterception =
                        UnoRuntime.queryInterface(XContextMenuInterception.class, xController);

                if (xContextMenuInterception != null)
                {
                    ContextMenuInterceptor aContextMenuInterceptor = new ContextMenuInterceptor(xBitmap);
                    xContextMenuInterceptor =
                            UnoRuntime.queryInterface(XContextMenuInterceptor.class, aContextMenuInterceptor);

                    System.out.println("Register context menu interceptor");
                    xContextMenuInterception.registerContextMenuInterceptor(xContextMenuInterceptor);
                }
            }

            //  utils.shortWait(10000);

            openContextMenu(UnoRuntime.queryInterface(XModel.class, xDrawDoc));

            checkHelpEntry();

            // remove our context menu interceptor
            if (xContextMenuInterception != null
                    && xContextMenuInterceptor != null)
            {
                System.out.println("Release context menu interceptor");
                xContextMenuInterception.releaseContextMenuInterceptor(
                        xContextMenuInterceptor);
            }
        }
        catch (com.sun.star.uno.RuntimeException ex)
        {
            // ex.printStackTrace();
            fail("Runtime exception caught!" + ex.getMessage());
        }
        catch (java.lang.Exception ex)
        {
            // ex.printStackTrace();
            fail("Java lang exception caught!" + ex.getMessage());
        }
    }

    private void checkHelpEntry()
    {
        XInterface toolkit = null;

        System.out.println("get accesibility...");
        try
        {
            toolkit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit");
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println("could not get Toolkit " + e.toString());
        }
        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, toolkit);

        XAccessible xRoot = null;

        AccessibilityTools at = new AccessibilityTools();

        try
        {
            xWindow = UnoRuntime.queryInterface(XWindow.class, tk.getTopWindow(0));

            xRoot = at.getAccessibleObject(xWindow);

//            at.printAccessibleTree((PrintWriter)log, xRoot, param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
            // at.printAccessibleTree(System.out, xRoot, true);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            System.out.println("Couldn't get Window");
        }

        XAccessibleContext oPopMenu = at.getAccessibleObjectForRole(xRoot, AccessibleRole.POPUP_MENU);

        System.out.println("ImplementationName: " + util.utils.getImplName(oPopMenu));

        XAccessible xHelp = null;
        try
        {
            System.out.println("Try to get first entry of context menu...");
            xHelp = oPopMenu.getAccessibleChild(0);

        }
        catch (IndexOutOfBoundsException e)
        {
            fail("Not possible to get first entry of context menu");
        }

        if (xHelp == null)
        {
            fail("first entry of context menu is NULL");
        }

        XAccessibleContext xHelpCont = xHelp.getAccessibleContext();

        if (xHelpCont == null)
        {
            fail("No able to retrieve accessible context from first entry of context menu");
        }

        String aAccessibleName = xHelpCont.getAccessibleName();
        if (!aAccessibleName.equals("Help"))
        {
            System.out.println("Accessible name found = " + aAccessibleName);
            fail("First entry of context menu is not from context menu interceptor");
        }

        try
        {
            System.out.println("try to get first children of Help context...");
            XAccessible xHelpChild = xHelpCont.getAccessibleChild(0);

        }
        catch (IndexOutOfBoundsException e)
        {
            fail("not possible to get first children of Help context");
        }

    }

    private void openContextMenu(XModel aModel)
    {

        System.out.println("try to open contex menu...");
        AccessibilityTools at = new AccessibilityTools();

        xWindow = at.getCurrentWindow(xMSF, aModel);

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XInterface oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        XAccessibleComponent window = UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);

        point = window.getLocationOnScreen();
        Rectangle rect = window.getBounds();

        System.out.println("klick mouse button...");
        try
        {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2);
            int y = point.Y + (rect.Height / 2);
            rob.mouseMove(x, y);
            System.out.println("Press Button");
            rob.mousePress(InputEvent.BUTTON3_MASK);
            System.out.println("Release Button");
            rob.mouseRelease(InputEvent.BUTTON3_MASK);
            System.out.println("done");
        }
        catch (java.awt.AWTException e)
        {
            System.out.println("couldn't press mouse button");
        }

        utils.shortWait(3000);

    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
