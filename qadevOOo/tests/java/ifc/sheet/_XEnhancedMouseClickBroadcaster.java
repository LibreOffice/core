/*
 * _XEnhancedMouseClickBroadcaster.java
 *
 * Created on 26. Februar 2004, 16:27
 */

package ifc.sheet;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XEnhancedMouseClickHandler;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XEnhancedMouseClickBroadcaster;
import com.sun.star.uno.UnoRuntime;
import java.awt.Robot;
import java.awt.event.InputEvent;
import lib.MultiMethodTest;
import lib.StatusException;
import util.AccessibilityTools;
import util.utils;

/**
 *
 * @author  sw93809
 */
public class _XEnhancedMouseClickBroadcaster extends MultiMethodTest {

    public XEnhancedMouseClickBroadcaster oObj;
    protected boolean mousePressed=false;
    protected boolean mouseReleased=false;

    public void _addEnhancedMouseClickHandler() {
        oObj.addEnhancedMouseClickHandler(listener);
        clickOnSheet();
        //make sure that the listener is removed even if the test fails
        if ((!mousePressed) || (!mouseReleased)) {
            oObj.removeEnhancedMouseClickHandler(listener);
        }
        tRes.tested("addEnhancedMouseClickHandler()",mousePressed && mouseReleased);
    }

    public void _removeEnhancedMouseClickHandler() {
        requiredMethod("addEnhancedMouseClickHandler()");
        mousePressed=false;
        mouseReleased=false;
        oObj.removeEnhancedMouseClickHandler(listener);
        clickOnSheet();
        tRes.tested("removeEnhancedMouseClickHandler()",(!mousePressed) && (!mouseReleased));
    }


    protected XEnhancedMouseClickHandler listener = new MyListener();

    protected class MyListener implements XEnhancedMouseClickHandler {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

        public boolean mousePressed(com.sun.star.awt.EnhancedMouseEvent enhancedMouseEvent) {
            log.println("mousePressed");
            mousePressed=true;
            return true;
        }

        public boolean mouseReleased(com.sun.star.awt.EnhancedMouseEvent enhancedMouseEvent) {
            log.println("mouseReleased");
            mouseReleased=true;
            return true;
        }

    }

    protected boolean clickOnSheet() {
        log.println("Trying to get AccessibleSpreadsheet");
        AccessibilityTools at = new AccessibilityTools();
        XComponent xSheetDoc = (XComponent) tEnv.getObjRelation("DOCUMENT");
        XModel xModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSheetDoc);
       XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)tParam.getMSF(), xModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.TABLE  );

        XAccessibleComponent AccessibleSpreadsheet = (XAccessibleComponent) UnoRuntime.queryInterface(XAccessibleComponent.class,AccessibilityTools.SearchedContext);

        log.println("Got " + utils.getImplName(AccessibleSpreadsheet));

        Object toolkit = null;

        try {
            toolkit = ((XMultiServiceFactory)tParam.getMSF()).createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

        XTopWindow tw = null;

        int k = tk.getTopWindowCount();
        for (int i=0;i<k;i++) {
        try {
            XTopWindow tw_temp = tk.getTopWindow(11);
            XAccessible xacc = (XAccessible) UnoRuntime.queryInterface(XAccessible.class,  tw_temp);
            if (xacc != null) {
                if (xacc.getAccessibleContext().getAccessibleName().indexOf("d2")>0) {
                    tw=tw_temp;
                };
            } else {
                log.println("\t unknown window");
            }

        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {

        }
        }
        if (tw == null) {
            System.out.println("No TopWindow :-(");
            return false;
        }
        tw.toFront();

        Point point = AccessibleSpreadsheet.getLocationOnScreen();
        Rectangle rect = AccessibleSpreadsheet.getBounds();

        log.println("Clicking in the center of the AccessibleSpreadsheet");

        try {
            Robot rob = new Robot();
            int x = point.X + (rect.Width / 2);
            int y = point.Y + (rect.Height / 2);
            rob.mouseMove(x, y);
            rob.mousePress(InputEvent.BUTTON1_MASK);
            shortWait();
            rob.mouseRelease(InputEvent.BUTTON1_MASK);
            shortWait();
        } catch (java.awt.AWTException e) {
            log.println("couldn't press mouse button");
        }

        return true;
    }

    private void shortWait() {
        try {
            Thread.currentThread().sleep(500);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }
}
