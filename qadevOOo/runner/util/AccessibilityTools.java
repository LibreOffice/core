/*************************************************************************
 *
 *  $RCSfile: AccessibilityTools.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change:$Date: 2004-07-23 10:43:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package util;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;


public class AccessibilityTools {
    public static XAccessibleContext SearchedContext = null;
    public static XAccessible SearchedAccessible = null;

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

    public static void printAccessibleTree(PrintWriter log, XAccessible xacc) {
        printAccessibleTree(log, xacc, "");
    }

    protected static void printAccessibleTree(PrintWriter log,
                                              XAccessible xacc, String indent) {
        XAccessibleContext ac = xacc.getAccessibleContext();

        log.println(indent + ac.getAccessibleRole() + "," +
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
            log.println(indent + indent + bounds);
        }

        boolean isShowing = ac.getAccessibleStateSet()
                              .contains(com.sun.star.accessibility.AccessibleStateType.SHOWING);
        log.println(indent + indent + "StateType contains SHOWING: " +
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
            log.println(indent + "  " + " ...... [skipped] ......");

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
}