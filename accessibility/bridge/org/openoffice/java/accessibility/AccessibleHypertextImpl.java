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
            XAccessibleHypertext unoAccessibleHypertext = UnoRuntime.queryInterface(XAccessibleHypertext.class, unoObject);
            if (unoAccessibleHypertext != null) {
                return new AccessibleHypertextImpl(unoAccessibleHypertext);
            }

            XAccessibleText unoAccessibleText = UnoRuntime.queryInterface(XAccessibleText.class, unoObject);
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
