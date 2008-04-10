/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibilityTools.java,v $
 * $Revision: 1.12 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package util;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;


public class AccessibilityTools {
    public static XAccessibleContext SearchedContext = null;
    public static XAccessible SearchedAccessible = null;
    private static boolean debug = false;

    public AccessibilityTools() {
        //done = false;
        SearchedContext = null;
    }

    public static XAccessible getAccessibleObject(XInterface xObject) {
        XAccessible xAccessible = null;

        try {
            xAccessible = (XAccessible) UnoRuntime.queryInterface(
                XAccessible.class, xObject);
        } catch (Exception e) {
            System.out.println(
                "caught exception while getting accessible object" + e);
            e.printStackTrace();
        }

        return xAccessible;
    }

    public static XWindow getCurrentContainerWindow(XMultiServiceFactory msf,
        XModel xModel) {
        return getWindow(msf, xModel, true);
    }

    public static XWindow getCurrentWindow(XMultiServiceFactory msf,
        XModel xModel) {
        return getWindow(msf, xModel, false);
    }

    private static XWindow getWindow(XMultiServiceFactory msf, XModel xModel,
        boolean containerWindow) {
        XWindow xWindow = null;

        try {
            if (xModel == null) {
                System.out.println("invalid model (==null)");
            }

            XController xController = xModel.getCurrentController();

            if (xController == null) {
                System.out.println("can't get controller from model");
            }

            XFrame xFrame = xController.getFrame();

            if (xFrame == null) {
                System.out.println("can't get frame from controller");
            }

            if (containerWindow)
                xWindow = xFrame.getContainerWindow();
            else
                xWindow = xFrame.getComponentWindow();

            if (xWindow == null) {
                System.out.println("can't get window from frame");
            }
        } catch (Exception e) {
            System.out.println("caught exception while getting current window" + e);
        }

        return xWindow;
    }

    public static XAccessibleContext getAccessibleObjectForRole(XAccessible xacc,
        short role) {
        SearchedContext = null;
        SearchedAccessible = null;
        getAccessibleObjectForRole_(xacc, role);

        return SearchedContext;
    }

    public static XAccessibleContext getAccessibleObjectForRole(XAccessible xacc,
        short role,
        boolean ignoreShowing) {
        SearchedContext = null;
        SearchedAccessible = null;

        if (ignoreShowing) {
            getAccessibleObjectForRoleIgnoreShowing_(xacc, role);
        } else {
            getAccessibleObjectForRole_(xacc, role);
        }

        return SearchedContext;
    }

    public static void getAccessibleObjectForRoleIgnoreShowing_(XAccessible xacc,
        short role) {
        XAccessibleContext ac = xacc.getAccessibleContext();

        if (ac.getAccessibleRole() == role) {
            SearchedContext = ac;
            SearchedAccessible = xacc;
        } else {
            int k = ac.getAccessibleChildCount();

            if (ac.getAccessibleChildCount() > 100) {
                k = 50;
            }

            for (int i = 0; i < k; i++) {
                try {
                    getAccessibleObjectForRoleIgnoreShowing_(
                        ac.getAccessibleChild(i), role);

                    if (SearchedContext != null) {
                        return;
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }
    }

    public static void getAccessibleObjectForRole_(XAccessible xacc,
        short role) {
        XAccessibleContext ac = xacc.getAccessibleContext();
        boolean isShowing = ac.getAccessibleStateSet()
        .contains(com.sun.star.accessibility.AccessibleStateType.SHOWING);

        if ((ac.getAccessibleRole() == role) && isShowing) {
            SearchedContext = ac;
            SearchedAccessible = xacc;
        } else {
            int k = ac.getAccessibleChildCount();

            if (ac.getAccessibleChildCount() > 100) {
                k = 50;
            }

            for (int i = 0; i < k; i++) {
                try {
                    getAccessibleObjectForRole_(ac.getAccessibleChild(i), role);

                    if (SearchedContext != null) {
                        return;
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }
    }

    public static XAccessibleContext getAccessibleObjectForRole(XAccessible xacc,
        short role,
        String name) {
        return getAccessibleObjectForRole(xacc, role, name, "");
    }

    public static XAccessibleContext getAccessibleObjectForRole(XAccessible xacc,
        short role,
        String name,
        boolean ignoreShowing) {
        if (ignoreShowing) {
            return getAccessibleObjectForRoleIgnoreShowing(xacc, role, name,
                "");
        } else {
            return getAccessibleObjectForRole(xacc, role, name, "");
        }
    }

    public static XAccessibleContext getAccessibleObjectForRoleIgnoreShowing(XAccessible xacc,
        short role,
        String name,
        String implName) {
        XAccessibleContext ac = xacc.getAccessibleContext();
        if ((ac.getAccessibleRole() == role) &&
            (ac.getAccessibleName().indexOf(name) > -1) &&
            (utils.getImplName(ac).indexOf(implName) > -1)) {
            SearchedAccessible = xacc;

            //System.out.println("FOUND the desired component -- "+ ac.getAccessibleName() +isShowing);
            return ac;
        } else {
            int k = ac.getAccessibleChildCount();

            if (ac.getAccessibleChildCount() > 100) {
                k = 50;
            }

            for (int i = 0; i < k; i++) {
                try {
                    XAccessibleContext ac1 = getAccessibleObjectForRoleIgnoreShowing(
                        ac.getAccessibleChild(i),
                        role, name, implName);

                    if (ac1 != null) {
                        return ac1;
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }

        return null;
    }

    public static XAccessibleContext getAccessibleObjectForRole(XAccessible xacc,
        short role,
        String name,
        String implName) {
        XAccessibleContext ac = xacc.getAccessibleContext();
        boolean isShowing = ac.getAccessibleStateSet()
        .contains(com.sun.star.accessibility.AccessibleStateType.SHOWING);
        if ((ac.getAccessibleRole() == role) &&
            (ac.getAccessibleName().indexOf(name) > -1) &&
            (utils.getImplName(ac).indexOf(implName) > -1) &&
            isShowing) {
            SearchedAccessible = xacc;

            //System.out.println("FOUND the desired component -- "+ ac.getAccessibleName() +isShowing);
            return ac;
        } else {
            int k = ac.getAccessibleChildCount();

            if (ac.getAccessibleChildCount() > 100) {
                k = 50;
            }

            for (int i = 0; i < k; i++) {
                try {
                    XAccessibleContext ac1 = getAccessibleObjectForRole(
                        ac.getAccessibleChild(i),
                        role, name, implName);

                    if (ac1 != null) {
                        return ac1;
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }

        return null;
    }

    /**
     * This methods retunrs the <CODE>XAccessibleContext</CODE> of a named Sheet-Cell like "G5".<p>
     * @param xSheetAcc The <CODE>XAccessibleContext</CODE> of a Sheet
     * @param cellName The name of a cell like "A5"
     * @return the <CODE>XAccessiblecontext</CODE> of the named cell
     */
    public static XAccessibleContext getSheetCell(XAccessibleContext xSheetAcc, String cellName){

        int cellIndex = 0;
        int column =0;
        int charMem = 0;
        for (int n=0; n<cellName.length(); n++){
            String cha = cellName.substring(n,n+1);
            System.out.println("char: " + cha + " ");

            byte[] bytes = cha.getBytes();

            if ((bytes[0] >= 'A') && (bytes[0] <= 'Z')){
                charMem = bytes[0]-64;
                column++;
                if ( column == 2 ){
                    cellIndex += charMem * 26;
                }
                cellIndex= cellIndex+ (bytes[0]-65);
            } else {
                String sNumb = cellName.substring(n, cellName.length());
                int iNumb = new Integer(0).valueOf(sNumb).intValue();
                cellIndex += (iNumb-1) * 256;
                System.out.println("numb:" + (iNumb-1) * 256);
            }

        }

        //System.out.println("cellName:  " + cellName + " cellIndex: " + cellIndex);

        try {
            XAccessibleContext ac = xSheetAcc.getAccessibleChild(cellIndex).getAccessibleContext();
            System.out.println(ac.getAccessibleRole() + "," +
                ac.getAccessibleName() + "(" +
                ac.getAccessibleDescription() + "):" +
                utils.getImplName(ac));

            return ac;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            System.out.println("ERROR: could not get child at index " + cellIndex +"': " + ex.toString());
            return null;
        }
    }

    public static void printAccessibleTree(PrintWriter log, XAccessible xacc, boolean debugIsActive) {
        debug = debugIsActive;
        if (debug) printAccessibleTree(log, xacc, "");
    }

    public static void printAccessibleTree(PrintWriter log, XAccessible xacc) {
        printAccessibleTree(log, xacc, "");
    }

    protected static void printAccessibleTree(PrintWriter log,
        XAccessible xacc, String indent) {

        XAccessibleContext ac = xacc.getAccessibleContext();

        logging(log,indent + ac.getAccessibleRole() + "," +
            ac.getAccessibleName() + "(" +
            ac.getAccessibleDescription() + "):" +
            utils.getImplName(ac));

        XAccessibleComponent aComp = (XAccessibleComponent) UnoRuntime.queryInterface(
            XAccessibleComponent.class, xacc);

        if (aComp != null) {
            String bounds = "(" + aComp.getBounds().X + "," +
                aComp.getBounds().Y + ")" + " (" +
                aComp.getBounds().Width + "," +
                aComp.getBounds().Height + ")";
            bounds = "The boundary Rectangle is " + bounds;
            logging(log,indent + indent + bounds);
        }

        boolean isShowing = ac.getAccessibleStateSet()
        .contains(com.sun.star.accessibility.AccessibleStateType.SHOWING);
        logging(log,indent + indent + "StateType contains SHOWING: " +
            isShowing);

        int k = ac.getAccessibleChildCount();

        if (ac.getAccessibleChildCount() > 100) {
            k = 50;
        }

        for (int i = 0; i < k; i++) {
            try {
                printAccessibleTree(log, ac.getAccessibleChild(i),
                    indent + "  ");
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("Couldn't get Child");
            }
        }

        if (ac.getAccessibleChildCount() > 100) {
            k = ac.getAccessibleChildCount();

            int st = ac.getAccessibleChildCount() - 50;
            logging(log,indent + "  " + " ...... [skipped] ......");

            for (int i = st; i < k; i++) {
                try {
                    printAccessibleTree(log, ac.getAccessibleChild(i),
                        indent + "  ");
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }
    }

    public static String accessibleToString(Object AC) {
        XAccessibleContext xAC = (XAccessibleContext) UnoRuntime.queryInterface(
            XAccessibleContext.class, AC);

        if (xAC != null) {
            return "" + xAC.getAccessibleRole() + "," +
                xAC.getAccessibleName() + "(" +
                xAC.getAccessibleDescription() + "):";
        }

        XAccessible xA = (XAccessible) UnoRuntime.queryInterface(
            XAccessible.class, AC);

        if (xA == null) {
            return "(Not supported)";
        }

        xAC = xA.getAccessibleContext();

        return "" + xAC.getAccessibleRole() + "," + xAC.getAccessibleName() +
            "(" + xAC.getAccessibleDescription() + ")";
    }

    public static boolean equals(XAccessible c1, XAccessible c2) {
        if ((c1 == null) || (c2 == null)) {
            return c1 == c2;
        }

        return AccessibilityTools.equals(c1.getAccessibleContext(),
            c2.getAccessibleContext());
    }

    public static boolean equals(XAccessibleContext c1, XAccessibleContext c2) {
        if ((c1 == null) || (c2 == null)) {
            return c1 == c2;
        }

        if (c1.getAccessibleRole() != c2.getAccessibleRole()) {
            return false;
        }

        if (!c1.getAccessibleName().equals(c2.getAccessibleName())) {
            return false;
        }

        if (!c1.getAccessibleDescription()
        .equals(c2.getAccessibleDescription())) {
            return false;
        }

        if (c1.getAccessibleChildCount() != c2.getAccessibleChildCount()) {
            return false;
        }

        return AccessibilityTools.equals(c1.getAccessibleParent(),
            c2.getAccessibleParent());
    }

    private static void logging(PrintWriter log, String content){
        if (debug) log.println(content);
    }
}