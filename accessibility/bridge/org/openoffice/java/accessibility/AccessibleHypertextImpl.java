/*************************************************************************
 *
 *  $RCSfile: AccessibleHypertextImpl.java,v $
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

package org.openoffice.java.accessibility;

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
        super(xAccessibleHypertext);
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
