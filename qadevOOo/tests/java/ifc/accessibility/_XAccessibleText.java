/*************************************************************************
 *
 *  $RCSfile: _XAccessibleText.java,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change:$Date: 2003-09-08 10:06:15 $
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
package ifc.accessibility;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.accessibility.AccessibleTextType;
import com.sun.star.accessibility.TextSegment;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;


/**
 * Testing <code>com.sun.star.accessibility.XAccessibleText</code>
 * interface methods :
 * <ul>
 *  <li><code> getCaretPosition()</code></li>
 *  <li><code> setCaretPosition()</code></li>
 *  <li><code> getCharacter()</code></li>
 *  <li><code> getCharacterAttributes()</code></li>
 *  <li><code> getCharacterBounds()</code></li>
 *  <li><code> getCharacterCount()</code></li>
 *  <li><code> getIndexAtPoint()</code></li>
 *  <li><code> getSelectedText()</code></li>
 *  <li><code> getSelectionStart()</code></li>
 *  <li><code> getSelectionEnd()</code></li>
 *  <li><code> setSelection()</code></li>
 *  <li><code> getText()</code></li>
 *  <li><code> getTextRange()</code></li>
 *  <li><code> getTextAtIndex()</code></li>
 *  <li><code> getTextBeforeIndex()</code></li>
 *  <li><code> getTextBehindIndex()</code></li>
 *  <li><code> copyText()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XAccessibleText.Text'</code> (of type <code>String</code>)
 *   <b> optional </b> :
 *   the string presentation of component's text. If the relation
 *   is not specified, then text from method <code>getText()</code>
 *   is used.
 *  </li>
 *  </ul> <p>
 * @see com.sun.star.accessibility.XAccessibleText
 */
public class _XAccessibleText extends MultiMethodTest {

    public XAccessibleText oObj = null;
    protected com.sun.star.awt.Rectangle bounds = null;
    String text = null;
    String editOnly = null;
    Object LimitedBounds = null;
    Rectangle chBounds = null;
    int chCount = 0;


    /**
     * Retrieves a string representation of the component's text.
     * The length of retrieved string must be greater than zero.
     */
    protected void before() {
        Object xat = tEnv.getObjRelation("XAccessibleText");

        XAccessibleComponent component = null;

        if (xat != null) {
            oObj = (XAccessibleText) UnoRuntime.queryInterface(
                           XAccessibleText.class, xat);
            component = (XAccessibleComponent) UnoRuntime.queryInterface(
                                XAccessibleComponent.class, xat);
        }

        text = (String) tEnv.getObjRelation("XAccessibleText.Text");

        if (text == null) {
            text = oObj.getText();
        }

        if (text.length() == 0) {
            throw new StatusException(Status.failed(
                                              "The length of text must be greater than zero"));
        }

        editOnly = (String) tEnv.getObjRelation("EditOnly");
        LimitedBounds = tEnv.getObjRelation("LimitedBounds");

        if (component == null) {
            component = (XAccessibleComponent) UnoRuntime.queryInterface(
                                XAccessibleComponent.class,
                                tEnv.getTestObject());
        }

        bounds = component.getBounds();

        log.println("Text is '" + text + "'");
        System.out.println("############################");
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value is equal to <code>chCount - 1</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setCaretPosition()</code> </li>
     * </ul>
     */
    public void _getCaretPosition() {
        requiredMethod("getCharacterCount()");

        if (editOnly != null) {
            log.println(editOnly);
            throw new StatusException(Status.skipped(true));
        }

        boolean res = true;
        boolean sc = true;

        try {
            oObj.setCaretPosition(chCount - 1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ie) {
        }

        int carPos = oObj.getCaretPosition();
        log.println("getCaretPosition: " + carPos);

        if (sc) {
            res = carPos == (chCount - 1);
        } else {
            log.println(
                    "Object is read only and Caret position couldn't be set");
            res = carPos == -1;
        }

        tRes.tested("getCaretPosition()", res);
    }

    /**
     * Calls the method with the wrong index and with the correct index
     * <code>chCount - 1</code>.
     * Has OK status if exception was thrown for wrong index and
     * if exception wasn't thrown for the correct index.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _setCaretPosition() {
        requiredMethod("getCharacterCount()");

        boolean res = true;

        try {
            log.print("setCaretPosition(-1):");
            oObj.setCaretPosition(-1);
            res &= false;
            log.println("exception was expected");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("setCaretPosition(chCount+1):");
            oObj.setCaretPosition(chCount + 1);
            res &= false;
            log.println("exception was expected");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.println("setCaretPosition(chCount - 1)");
            oObj.setCaretPosition(chCount - 1);
            res &= true;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("setCaretPosition()", res);
    }

    /**
     * Calls the method with the wrong index and with the correct indexes.
     * Checks every character in the text.
     * Has OK status if exception was thrown for wrong index,
     * if exception wasn't thrown for the correct index and
     * if every character is equal to corresponding character in the text.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getCharacter() {
        requiredMethod("getCharacterCount()");

        boolean res = true;

        try {
            log.println("getCharacter(-1)");
            oObj.getCharacter(-1);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res = true;
        }

        try {
            log.println("getCharacter(chCount)");
            oObj.getCharacter(chCount);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("Checking of every character in the text...");

            boolean isEqCh = true;

            for (int i = 0; i < chCount; i++) {
                char ch = oObj.getCharacter(i);
                isEqCh = ch == text.charAt(i);
                res &= isEqCh;

                if (!isEqCh) {
                    log.println("At the position " + i +
                                "was expected character: " + text.charAt(i));
                    log.println("but was returned: " + ch);

                    break;
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getCharacter()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct index,
     * checks a returned value.
     * Has OK status if exception was thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value isn't <code>null</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getCharacterAttributes() {
        requiredMethod("getCharacterCount()");

        boolean res = true;
        String[] attr = new String[] { "" };

        try {
            log.println("getCharacterAttributes(-1)");
            oObj.getCharacterAttributes(-1, attr);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getCharacterAttributes(chCount)");
            oObj.getCharacterAttributes(chCount, attr);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getCharacterAttributes(chCount-1)");

            PropertyValue[] props = oObj.getCharacterAttributes(chCount - 1,
                                                                attr);
            res &= (props != null);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getCharacterAttributes()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct index.
     * checks and stores a returned value.
     * Has OK status if exception was thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value isn't <code>null</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getCharacterBounds() {
        requiredMethod("getCharacterCount()");

        boolean res = true;

        int lastIndex = chCount;

        if (LimitedBounds != null) {
            if (LimitedBounds instanceof Integer) {
                lastIndex = ((Integer) LimitedBounds).intValue();
            } else {
                lastIndex = chCount - 1;
            }

            log.println(LimitedBounds);
        }

        try {
            log.println("getCharacterBounds(-1)");
            oObj.getCharacterBounds(-1);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getCharacterBounds(" + (lastIndex + 1) + ")");
            oObj.getCharacterBounds(lastIndex + 1);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            for (int i = 0; i < (lastIndex + 1); i++) {
                log.println("getCharacterBounds(" + i + ")");
                chBounds = oObj.getCharacterBounds(i);

                boolean localres = true;
                localres = chBounds.X >= 0;
                localres &= (chBounds.Y >= 0);
                localres &= ((chBounds.X + chBounds.Width) <= bounds.Width);
                localres &= ((chBounds.X + chBounds.Width) > 0);
                localres &= ((chBounds.Y + chBounds.Height) <= bounds.Height);
                localres &= ((chBounds.Y + chBounds.Height) > 0);

                if (!localres) {
                    log.println("Character bounds outside component");
                    log.println("Character rect: " + chBounds.X + ", " +
                                chBounds.Y + ", " + chBounds.Width + ", " +
                                chBounds.Height);
                    log.println("Component rect: " + bounds.X + ", " +
                                bounds.Y + ", " + bounds.Width + ", " +
                                bounds.Height);
                    res &= localres;
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getCharacterBounds()", res);
    }

    /**
     * Calls the method and stores a returned value to the variable
     * <code>chCount</code>.
     * Has OK status if a returned value is equal to the text length.
     */
    public void _getCharacterCount() {
        chCount = oObj.getCharacterCount();
        log.println("Character count:" + chCount);

        boolean res = chCount == text.length();
        tRes.tested("getCharacterCount()", res);
    }

    /**
     * Calls the method for an invalid point and for the point of rectangle
     * returned by the method <code>getCharacterBounds()</code>.
     * Has OK status if returned value is equal to <code>-1</code> for an
     * invalid point and if returned value is equal to <code>chCount-1</code>
     * for a valid point.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterBounds()</code> </li>
     * </ul>
     */
    public void _getIndexAtPoint() {
        //requiredMethod("getCharacterBounds()");
        boolean res = true;
        log.print("getIndexAtPoint(-1, -1):");

        Point pt = new Point(-1, -1);
        int index = oObj.getIndexAtPoint(pt);
        log.println(index);
        res &= (index == -1);

        int lastIndex = chCount;

        if (LimitedBounds != null) {
            if (LimitedBounds instanceof Integer) {
                lastIndex = ((Integer) LimitedBounds).intValue();
            } else {
                lastIndex = chCount - 1;
            }

            log.println(LimitedBounds);
        }

        for (int i = 0; i < lastIndex; i++) {
            Rectangle aRect = null;
            String text = "empty";

            try {
                aRect = oObj.getCharacterBounds(i);
                text = oObj.getTextAtIndex(i, (short) 1).SegmentText;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            }

            int x = aRect.X + (aRect.Width / 2);
            int y = aRect.Y + (aRect.Height / 2);
            Point aPoint = new Point(x, y);
            int nIndex = oObj.getIndexAtPoint(aPoint);

            int[] previous = (int[]) tEnv.getObjRelation("PreviousUsed");

            if (previous != null) {
                for (int k = 0; k < previous.length; k++) {
                    if (i == previous[k]) {
                        nIndex++;
                    }
                }
            }

            if (nIndex != i) {
                log.println("## Method didn't work for Point (" + x + "," + y +
                            ")");
                log.println("Expected Index " + i);
                log.println("Gained Index: " + nIndex);
                log.println("CharacterAtIndex: " + text);
                res &= false;
            }
        }

        tRes.tested("getIndexAtPoint()", res);
    }

    /**
     * Checks a returned values after different calls of the method
     * <code>setSelection()</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setSelection()</code> </li>
     * </ul>
     */
    public void _getSelectedText() {
        if (editOnly != null) {
            log.println(editOnly);
            throw new StatusException(Status.skipped(true));
        }

        requiredMethod("setSelection()");

        boolean res = true;

        try {
            log.println("setSelection(0, 0)");
            oObj.setSelection(0, 0);
            log.print("getSelectedText():");

            String txt = oObj.getSelectedText();
            log.println("'" + txt + "'");
            res &= (txt.length() == 0);

            log.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            log.print("getSelectedText():");
            txt = oObj.getSelectedText();
            log.println("'" + txt + "'");
            res &= txt.equals(text);

            if (chCount > 2) {
                log.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                log.print("getSelectedText():");
                txt = oObj.getSelectedText();
                log.println("'" + txt + "'");
                res &= txt.equals(text.substring(1, chCount - 1));
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getSelectedText()", res);
    }

    /**
     * Checks a returned values after different calls of the method
     * <code>setSelection()</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setSelection()</code> </li>
     * </ul>
     */
    public void _getSelectionStart() {
        if (editOnly != null) {
            log.println(editOnly);
            throw new StatusException(Status.skipped(true));
        }

        requiredMethod("setSelection()");

        boolean res = true;

        try {
            log.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);

            int start = oObj.getSelectionStart();
            log.println("getSelectionStart():" + start);
            res &= (start == 0);

            if (chCount > 2) {
                log.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                start = oObj.getSelectionStart();
                log.println("getSelectionStart():" + start);
                res &= (start == 1);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getSelectionStart()", res);
    }

    /**
     * Checks a returned values after different calls of the method
     * <code>setSelection()</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setSelection()</code> </li>
     * </ul>
     */
    public void _getSelectionEnd() {
        if (editOnly != null) {
            log.println(editOnly);
            throw new StatusException(Status.skipped(true));
        }

        requiredMethod("setSelection()");

        boolean res = true;

        try {
            log.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);

            int end = oObj.getSelectionEnd();
            log.println("getSelectionEnd():" + end);
            res &= (end == chCount);

            if (chCount > 2) {
                log.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                end = oObj.getSelectionEnd();
                log.println("getSelectionEnd():" + end);
                res &= (end == (chCount - 1));
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getSelectionEnd()", res);
    }

    /**
     * Calls the method with invalid parameters an with valid parameters.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameters.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _setSelection() {
        requiredMethod("getCharacterCount()");

        boolean res = true;
        boolean locRes = true;

        if (editOnly != null) {
            log.println(editOnly);
            throw new StatusException(Status.skipped(true));
        }

        try {
            log.print("setSelection(-1, chCount-1):");
            locRes = oObj.setSelection(-1, chCount - 1);
            log.println(locRes + " excepion was expected");
            res &= !locRes;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.print("setSelection(0, chCount+1):");
            locRes = oObj.setSelection(0, chCount + 1);
            log.println(locRes + " excepion was expected");
            res &= !locRes;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            if (chCount > 2) {
                log.print("setSelection(1, chCount-1):");
                locRes = oObj.setSelection(1, chCount - 1);
                log.println(locRes);
                res &= locRes;

                log.print("setSelection(chCount-1, 1):");
                locRes = oObj.setSelection(chCount - 1, 1);
                log.println(locRes);
                res &= locRes;
            }

            log.print("setSelection(0, chCount-1):");
            locRes = oObj.setSelection(0, chCount - 1);
            log.println(locRes);
            res &= locRes;

            log.print("setSelection(chCount-1, 0):");
            locRes = oObj.setSelection(chCount - 1, 0);
            log.println(locRes);
            res &= locRes;

            log.print("setSelection(0, 0):");
            locRes = oObj.setSelection(0, 0);
            log.println(locRes);
            res &= locRes;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("setSelection()", res);
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned string is equal to string
     * received from relation.
     */
    public void _getText() {
        String txt = oObj.getText();
        log.println("getText: " + txt);

        boolean res = txt.equals(text);
        tRes.tested("getText()", res);
    }

    /**
     * Calls the method with invalid parameters an with valid parameters,
     * checks returned values.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameters and if returned values
     * are equal to corresponding substrings of the text received by relation.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getTextRange() {
        requiredMethod("getCharacterCount()");

        boolean res = true;
        boolean locRes = true;

        String txtRange = "";

        try {
            if (chCount > 3) {
                log.print("getTextRange(1, chCount - 2): ");

                txtRange = oObj.getTextRange(1, chCount - 2);
                log.println(txtRange);
                locRes = txtRange.equals(text.substring(1, chCount - 2));
                res &= locRes;

                if (!locRes) {
                    log.println("Was expected: " +
                                text.substring(1, chCount - 2));
                }
            }

            log.print("getTextRange(0, chCount-1): ");

            txtRange = oObj.getTextRange(0, chCount - 1);
            log.println(txtRange);
            locRes = txtRange.equals(text.substring(0, chCount - 1));
            res &= locRes;

            if (!locRes) {
                log.println("Was expected: " +
                            text.substring(0, chCount - 1));
            }

            log.print("getTextRange(chCount, 0): ");
            txtRange = oObj.getTextRange(chCount, 0);
            log.println(txtRange);
            res &= txtRange.equals(text);

            log.print("getTextRange(0, 0): ");
            txtRange = oObj.getTextRange(0, 0);
            log.println(txtRange);
            locRes = txtRange.equals("");
            res &= locRes;

            if (!locRes) {
                log.println("Empty string was expected");
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        try {
            log.print("getTextRange(-1, chCount - 1): ");

            txtRange = oObj.getTextRange(-1, chCount - 1);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.print("getTextRange(0, chCount + 1): ");

            txtRange = oObj.getTextRange(0, chCount + 1);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.print("getTextRange(chCount+1, -1): ");

            txtRange = oObj.getTextRange(chCount + 1, -1);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        tRes.tested("getTextRange()", res);
    }

    /**
     * Calls the method with invalid parameters an with valid parameters,
     * checks returned values.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameters and if returned values
     * are equal to corresponding substrings of the text received by relation.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getTextAtIndex() {
        requiredMethod("getCharacterCount()");
        TextSegment txt = null;
        boolean res = true;

        try {
            log.print("getTextAtIndex(-1, AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextAtIndex(-1,
                                                  AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("UnExpected exception");
            res &= false;
        }

        try {
            log.print("getTextAtIndex(chCount+1," +
                      " AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextAtIndex(chCount + 1,
                                                  AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("UnExpected exception");
            res &= false;
        }

        try {
            log.print("getTextAtIndex(chCount," +
                      " AccessibleTextType.WORD):");

            txt = oObj.getTextAtIndex(chCount, AccessibleTextType.WORD);
            log.println("'" + txt.SegmentText + "'");
            res &= compareLength(0,txt.SegmentText);

            log.print("getTextAtIndex(1," +
                      " AccessibleTextType.PARAGRAPH):");
            txt = oObj.getTextAtIndex(1, AccessibleTextType.PARAGRAPH);
            log.println("'" + txt.SegmentText + "'");
            res &= compareStrings(text,txt.SegmentText);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getTextAtIndex()", res);
    }

    /**
     * Calls the method with invalid parameters an with valid parameters,
     * checks returned values.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameters and if returned values
     * are equal to corresponding substrings of the text received by relation.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getTextBeforeIndex() {
        requiredMethod("getCharacterCount()");
        TextSegment txt = null;
        boolean res = true;

        try {
            log.print("getTextBeforeIndex(-1, AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextBeforeIndex(-1,
                                                      AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("UnExpected exception");
            res &= false;
        }

        try {
            log.print("getTextBeforeIndex(chCount+1, " +
                      "AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextBeforeIndex(chCount + 1,
                                                      AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("UnExpected exception");
            res &= true;
        }

        try {
            log.print("getTextBeforeIndex(chCount," +
                      " AccessibleTextType.WORD):");

            txt = oObj.getTextBeforeIndex(chCount,
                                                 AccessibleTextType.WORD);
            log.println("'" + txt.SegmentText + "'");
            res &= compareLength(chCount, txt.SegmentText);

            log.print("getTextBeforeIndex(1," +
                      " AccessibleTextType.PARAGRAPH):");
            txt = oObj.getTextBeforeIndex(1, AccessibleTextType.PARAGRAPH);
            log.println("'" + txt.SegmentText + "'");
            res &= compareLength(0, txt.SegmentText);

            log.print("getTextBeforeIndex(chCount-1," +
                      " AccessibleTextType.CHARACTER):");
            txt = oObj.getTextBeforeIndex(chCount - 1,
                                          AccessibleTextType.CHARACTER);
            log.println("'" + txt.SegmentText + "'");
            res &= compareStrings(text.substring(chCount - 2, chCount - 1),
                                  txt.SegmentText);

            if (chCount > 2) {
                log.print("getTextBeforeIndex(2," +
                          " AccessibleTextType.CHARACTER):");
                txt = oObj.getTextBeforeIndex(2, AccessibleTextType.CHARACTER);
                log.println("'" + txt.SegmentText + "'");
                res &= compareStrings(text.substring(1, 2), txt.SegmentText);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getTextBeforeIndex()", res);
    }

    /**
     * Calls the method with invalid parameters an with valid parameters,
     * checks returned values.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameters and if returned values
     * are equal to corresponding substrings of the text received by relation.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getCharacterCount()</code> </li>
     * </ul>
     */
    public void _getTextBehindIndex() {
        requiredMethod("getCharacterCount()");
        TextSegment txt = null;
        boolean res = true;

        try {
            log.print("getTextBehindIndex(-1, AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextBehindIndex(-1,
                                                      AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("UnExpected exception");
            res &= true;
        }

        try {
            log.print("getTextBehindIndex(chCount+1, " +
                      "AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextBehindIndex(chCount + 1,
                                                      AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("UnExpected exception");
            res &= true;
        }

        try {
            log.print("getTextBehindIndex(chCount," +
                      " AccessibleTextType.PARAGRAPH):");

            txt = oObj.getTextBehindIndex(chCount,
                                                 AccessibleTextType.PARAGRAPH);
            log.println("'" + txt.SegmentText + "'");
            res &= (txt.SegmentText.length() == 0);

            log.print("getTextBehindIndex(chCount-1," +
                      " AccessibleTextType.PARAGRAPH):");
            txt = oObj.getTextBehindIndex(chCount - 1,
                                          AccessibleTextType.PARAGRAPH);
            log.println("'" + txt.SegmentText + "'");
            res &= (txt.SegmentText.length() == 0);

            log.print("getTextBehindIndex(1," +
                      " AccessibleTextType.CHARACTER):");
            txt = oObj.getTextBehindIndex(1, AccessibleTextType.CHARACTER);
            log.println("'" + txt.SegmentText + "'");
            res &= txt.SegmentText.equals(text.substring(2, 3));

            if (chCount > 2) {
                log.print("getTextBehindIndex(chCount-2," +
                          " AccessibleTextType.CHARACTER):");
                txt = oObj.getTextBehindIndex(chCount - 2,
                                              AccessibleTextType.CHARACTER);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(chCount - 1, chCount));
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getTextBehindIndex()", res);
    }

    /**
     * Calls the method with invalid parameters an with valid parameter,
     * checks returned values.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameter and if returned value for
     * valid parameter is equal to <code>true</code>.
     */
    public void _copyText() {
        boolean res = true;
        boolean locRes = true;

        if (editOnly != null) {
            log.println(editOnly);
            throw new StatusException(Status.skipped(true));
        }

        try {
            log.print("copyText(-1,chCount):");
            oObj.copyText(-1, chCount);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.print("copyText(0,chCount+1):");
            oObj.copyText(0, chCount + 1);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.print("copyText(0,chCount):");
            locRes = oObj.copyText(0, chCount);
            log.println(locRes);
            res &= locRes;

            String cbText = null;

            try {
                cbText = util.SysUtils.getSysClipboardText((XMultiServiceFactory)tParam.getMSF());
            } catch (com.sun.star.uno.Exception e) {
                log.println("Couldn't access system clipboard :");
                e.printStackTrace(log);
            }

            log.println("Clipboard: '" + cbText + "'");
            res &= text.equals(cbText);

            if (chCount > 2) {
                log.print("copyText(1,chCount-1):");
                locRes = oObj.copyText(1, chCount - 1);
                log.println(locRes);
                res &= locRes;

                try {
                    cbText = util.SysUtils.getSysClipboardText((XMultiServiceFactory)tParam.getMSF());
                } catch (com.sun.star.uno.Exception e) {
                    log.println("Couldn't access system clipboard :");
                    e.printStackTrace(log);
                }

                log.println("Clipboard: '" + cbText + "'");
                res &= text.substring(1, chCount - 1).equals(cbText);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("copyText()", res);
    }

    public boolean compareStrings(String expected, String getting) {
        boolean res = expected.equals(getting);

        if (!res) {
            log.println("## The result isn't the expected:");
            log.println("\tGetting: " + getting);
            log.println("\tExpected: " + expected);
        }

        return res;
    }

    public boolean compareLength(int expected, String getting) {
        boolean res = (expected == getting.length());

        if (!res) {
            log.println("## The result isn't the expected:");
            log.println("\tGetting: " + getting.length());
            log.println("\tExpected: " + expected);
        }

        return res;
    }
}