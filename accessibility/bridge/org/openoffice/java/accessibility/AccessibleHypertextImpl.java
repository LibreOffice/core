/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleHypertextImpl.java,v $
 * $Revision: 1.5 $
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

package org.openoffice.java.accessibility;

import org.openoffice.java.accessibility.logging.*;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

/** The AccessibleHypertextImpl mapps all calls to the java AccessibleHypertext
 *  interface to the corresponding methods of the UNO XAccessibleHypertext
 *  interface.
 */
public class AccessibleHypertextImpl extends AccessibleTextImpl
    implements javax.accessibility.AccessibleHypertext {

    protected class Hyperlink extends javax.accessibility.AccessibleHyperlink {
        protected XAccessibleHyperlink unoObject;

        public Hyperlink(XAccessibleHyperlink xHyperlink) {
            unoObject = xHyperlink;
        }

        public int getStartIndex() {
            try {
                System.err.println("StartIndex: " + unoObject.getStartIndex());
                return unoObject.getStartIndex();
            } catch (com.sun.star.uno.RuntimeException e) {
                return -1;
            }
        }

        public Object getAccessibleActionObject(int param) {
            System.err.println("getActionObject");
            try {
                Object any = unoObject.getAccessibleActionObject(param);
                if (AnyConverter.isString(any)) {
                    String url = AnyConverter.toString(any);
                    if (null != url) {
                        return new java.net.URL(url);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            } catch (java.net.MalformedURLException exception) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }

            return null;
        }

        public int getEndIndex() {
            try {
                System.err.println("StartIndex: " + unoObject.getEndIndex());
                return unoObject.getEndIndex();
            } catch (com.sun.star.uno.RuntimeException e) {
                return -1;
            }
        }

        public Object getAccessibleActionAnchor(int param) {
            System.err.println("getActionAnchor");
            try {
                Object any = unoObject.getAccessibleActionObject(param);
                if (AnyConverter.isString(any)) {
                    System.err.println("Anchor: " + AnyConverter.toString(any));
                    return AnyConverter.toString(any);
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return null;
        }

        public boolean isValid() {
            return unoObject.isValid();
        }

        public boolean doAccessibleAction(int param) {
            try {
                return unoObject.doAccessibleAction(param);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return false;
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }

        public String getAccessibleActionDescription(int param) {
            try {
                return unoObject.getAccessibleActionDescription(param);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return null;
            }
        }

        public int getAccessibleActionCount() {
            try {
                return unoObject.getAccessibleActionCount();
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }
    }

    /** Creates new AccessibleHypertextImpl */
    public AccessibleHypertextImpl(XAccessibleHypertext xAccessibleHypertext) {
        if (Build.PRODUCT) {
            unoObject = xAccessibleHypertext;
        } else {
            String property = System.getProperty("AccessBridgeLogging");
            if ((property != null) && (property.indexOf("text") != -1)) {
                unoObject = new XAccessibleHypertextLog(xAccessibleHypertext);
            } else {
                unoObject = xAccessibleHypertext;
            }
        }
    }

    public static javax.accessibility.AccessibleText get(com.sun.star.uno.XInterface unoObject) {
        try {
            XAccessibleHypertext unoAccessibleHypertext = (XAccessibleHypertext)
                UnoRuntime.queryInterface(XAccessibleHypertext.class, unoObject);
            if (unoAccessibleHypertext != null) {
                return new AccessibleHypertextImpl(unoAccessibleHypertext);
            }

            XAccessibleText unoAccessibleText = (XAccessibleText)
                UnoRuntime.queryInterface(XAccessibleText.class, unoObject);
            if (unoAccessibleText != null) {
                return new AccessibleTextImpl(unoAccessibleText);
            }
        } catch (com.sun.star.uno.RuntimeException e) {
        }
        return null;
    }

    public javax.accessibility.AccessibleHyperlink getLink(int param) {
        try {
            return new Hyperlink(((XAccessibleHypertext) unoObject).getHyperLink(param));
        }

        catch(com.sun.star.lang.IndexOutOfBoundsException exception) {
            throw new IndexOutOfBoundsException(exception.getMessage());
        }
    }

    public int getLinkCount() {
        try {
            return ((XAccessibleHypertext) unoObject).getHyperLinkCount();
        } catch (com.sun.star.uno.RuntimeException e) {
            return 0;
        }
    }

    public int getLinkIndex(int param) {
        try {
            return ((XAccessibleHypertext) unoObject).getHyperLinkIndex(param);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            return -1;
        } catch (com.sun.star.uno.RuntimeException e) {
            return -1;
        }
    }
}
