/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
    public static XAccessible SearchedAccessible = null;
    private static boolean debug = false;

    public AccessibilityTools() {
        //done = false;
    }

    public static XAccessible getAccessibleObject(XInterface xObject) {
        return UnoRuntime.queryInterface(XAccessible.class, xObject);
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
        SearchedAccessible = null;
        return getAccessibleObjectForRole_(xacc, role);
    }

    public static XAccessibleContext getAccessibleObjectForRole(XAccessible xacc,
        short role,
        boolean ignoreShowing) {
        SearchedAccessible = null;

        if (ignoreShowing) {
            return getAccessibleObjectForRoleIgnoreShowing_(xacc, role);
        } else {
            return getAccessibleObjectForRole_(xacc, role);
        }
    }

    public static XAccessibleContext getAccessibleObjectForRoleIgnoreShowing_(XAccessible xacc,
        short role) {
        XAccessibleContext ac = xacc.getAccessibleContext();

        if (ac.getAccessibleRole() == role) {
            SearchedAccessible = xacc;
            return ac;
        } else {
            int k = ac.getAccessibleChildCount();

            if (ac.getAccessibleChildCount() > 100) {
                k = 50;
            }

            for (int i = 0; i < k; i++) {
                try {
                    XAccessibleContext ac2 = getAccessibleObjectForRoleIgnoreShowing_(
                        ac.getAccessibleChild(i), role);

                    if (ac2 != null) {
                        return ac2;
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
            return null;
        }
    }

    public static XAccessibleContext getAccessibleObjectForRole_(XAccessible xacc,
        short role) {
        XAccessibleContext ac = xacc.getAccessibleContext();
        boolean isShowing = ac.getAccessibleStateSet()
        .contains(com.sun.star.accessibility.AccessibleStateType.SHOWING);

        if ((ac.getAccessibleRole() == role) && isShowing) {
            SearchedAccessible = xacc;
            return ac;
        } else {
            int k = ac.getAccessibleChildCount();

            if (ac.getAccessibleChildCount() > 100) {
                k = 50;
            }

            for (int i = 0; i < k; i++) {
                try {
                    XAccessibleContext ac2 = getAccessibleObjectForRole_(ac.getAccessibleChild(i), role);

                    if (ac2 != null) {
                        return ac2;
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
            return null;
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

        // hotfix for i91828:
        // if role to search is 0 then ignore the role.
        if ( (role == 0 || ac.getAccessibleRole() == role) &&
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
                int iNumb = Integer.valueOf(sNumb).intValue();
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

        XAccessibleComponent aComp = UnoRuntime.queryInterface(
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
        XAccessibleContext xAC = UnoRuntime.queryInterface(
            XAccessibleContext.class, AC);

        if (xAC != null) {
            return "" + xAC.getAccessibleRole() + "," +
                xAC.getAccessibleName() + "(" +
                xAC.getAccessibleDescription() + "):";
        }

        XAccessible xA = UnoRuntime.queryInterface(
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