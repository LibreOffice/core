/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XAccessibleTextLog.java,v $
 * $Revision: 1.4 $
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
package org.openoffice.java.accessibility.logging;

import org.openoffice.java.accessibility.*;


/**
 *
 */
public class XAccessibleTextLog
    implements com.sun.star.accessibility.XAccessibleText {
    private com.sun.star.accessibility.XAccessibleText unoObject;
    private String name = "[Unknown] NoName";

    /** Creates a new instance of XAccessibleTextLog */
    public XAccessibleTextLog(
        com.sun.star.accessibility.XAccessibleText xAccessibleText) {
        unoObject = xAccessibleText;
        setName(xAccessibleText);
    }

    private void setName(
        com.sun.star.accessibility.XAccessibleText xAccessibleText) {
        try {
            com.sun.star.accessibility.XAccessibleContext unoAccessibleContext = (com.sun.star.accessibility.XAccessibleContext) com.sun.star.uno.UnoRuntime.queryInterface(com.sun.star.accessibility.XAccessibleContext.class,
                    xAccessibleText);

            if (unoAccessibleContext != null) {
                name = "[" +
                    AccessibleRoleAdapter.getAccessibleRole(unoAccessibleContext.getAccessibleRole()) +
                    "] " + unoAccessibleContext.getAccessibleName() + ": ";
            }
        } catch (com.sun.star.uno.RuntimeException e) {
        }
    }

    private String getPartString(short s) {
        String part = "INVALID";

        switch (s) {
            case com.sun.star.accessibility.AccessibleTextType.CHARACTER:
                part = "CHARACTER";

                break;

            case com.sun.star.accessibility.AccessibleTextType.WORD:
                part = "WORD";

                break;

            case com.sun.star.accessibility.AccessibleTextType.SENTENCE:
                part = "SENTENCE";

                break;

            case com.sun.star.accessibility.AccessibleTextType.LINE:
                part = "LINE";

                break;

            case com.sun.star.accessibility.AccessibleTextType.ATTRIBUTE_RUN:
                part = "ATTRIBUTE_RUN";

                break;

            default:
                break;
        }

        return part;
    }

    private String dumpTextSegment(com.sun.star.accessibility.TextSegment ts) {
        if (ts != null) {
            return "(" + ts.SegmentStart + "," + ts.SegmentEnd + "," +
            ts.SegmentText + ")";
        }

        return "NULL";
    }

    public boolean copyText(int param, int param1)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.copyText(param, param1);
    }

    public int getCaretPosition() {
        int pos = unoObject.getCaretPosition();
        System.err.println(name + "getCaretPosition() returns " + pos);

        return pos;
    }

    public char getCharacter(int param)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getCharacter(param);
    }

    public com.sun.star.beans.PropertyValue[] getCharacterAttributes(
        int param, String[] str)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getCharacterAttributes(param, str);
    }

    public com.sun.star.awt.Rectangle getCharacterBounds(int param)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        try {
            com.sun.star.awt.Rectangle r = unoObject.getCharacterBounds(param);
            System.err.println(name + "getCharacterBounds(" + param +
                ") returns (" + r.X + "," + r.Y + "," + r.Width + "," +
                r.Height + ")");

            return r;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            System.err.println("IndexOutOufBoundsException caught for " + name +
                "getCharacterBounds(" + param + ")");
            throw e;
        }
    }

    public int getCharacterCount() {
        return unoObject.getCharacterCount();
    }

    public int getIndexAtPoint(com.sun.star.awt.Point point) {
        try {
            int index = unoObject.getIndexAtPoint(point);
            System.err.println(name + "getIndexAtPoint(" + point.X + ", " +
                point.Y + ") returns " + index);

            return index;
        } catch (com.sun.star.uno.RuntimeException e) {
            System.err.println(name +
                "RuntimeException caught for getIndexAtPoint(" + point.X +
                ", " + point.Y + ")");
            System.err.println(e.getMessage());
            throw e;
        }
    }

    public String getSelectedText() {
        return unoObject.getSelectedText();
    }

    public int getSelectionEnd() {
        return unoObject.getSelectionEnd();
    }

    public int getSelectionStart() {
        return unoObject.getSelectionStart();
    }

    public String getText() {
        return unoObject.getText();
    }

    public com.sun.star.accessibility.TextSegment getTextAtIndex(int param,
        short param1)
        throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.IllegalArgumentException {
        try {
            com.sun.star.accessibility.TextSegment ts = unoObject.getTextAtIndex(param,
                    param1);
            System.err.println(name + "getTextAtIndex(" +
                getPartString(param1) + "," + param + ") returns " +
                dumpTextSegment(ts));

            return ts;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            System.err.println("IndexOutOufBoundsException caught for " + name +
                " getTextAtIndex(" + getPartString(param1) + "," + param1 +
                ")");
            throw e;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println("IllegalArgumentException caught for " + name +
                " getTextAtIndex(" + getPartString(param1) + "," + param + ")");
            throw e;
        }
    }

    public com.sun.star.accessibility.TextSegment getTextBeforeIndex(
        int param, short param1)
        throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.IllegalArgumentException {
        try {
            com.sun.star.accessibility.TextSegment ts = unoObject.getTextBeforeIndex(param,
                    param1);
            System.err.println(name + " getTextBeforeIndex(" +
                getPartString(param1) + "," + param + ") returns " +
                dumpTextSegment(ts));

            return ts;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            System.err.println("IndexOutOufBoundsException caught for " + name +
                " getTextBeforeIndex(" + getPartString(param1) + "," + param1 +
                ")");
            throw e;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println("IllegalArgumentException caught for " + name +
                " getTextBeforeIndex(" + getPartString(param1) + "," + param +
                ")");
            throw e;
        }
    }

    public com.sun.star.accessibility.TextSegment getTextBehindIndex(
        int param, short param1)
        throws com.sun.star.lang.IndexOutOfBoundsException,
            com.sun.star.lang.IllegalArgumentException {
        try {
            com.sun.star.accessibility.TextSegment ts = unoObject.getTextBehindIndex(param,
                    param1);
            System.err.println(name + " getTextBehindIndex(" +
                getPartString(param1) + "," + param + ") returns " +
                dumpTextSegment(ts));

            return ts;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            System.err.println("IndexOutOufBoundsException caught for " + name +
                " getTextBehindIndex(" + getPartString(param1) + "," + param1 +
                ")");
            throw e;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.err.println("IllegalArgumentException caught for " + name +
                " getTextBehindIndex(" + getPartString(param1) + "," + param +
                ")");
            throw e;
        }
    }

    public String getTextRange(int param, int param1)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getTextRange(param, param1);
    }

    public boolean setCaretPosition(int param)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.setCaretPosition(param);
    }

    public boolean setSelection(int param, int param1)
        throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.setSelection(param, param1);
    }
}
