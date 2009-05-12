/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XAccessibleText.java,v $
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

package complex.toolkit.interface_tests;

import com.sun.star.accessibility.XAccessibleText;
import lib.MultiMethodTest;
//import lib.StatusException;
//import lib.Status;
import com.sun.star.beans.PropertyValue;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Point;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.accessibility.AccessibleTextType;
import com.sun.star.accessibility.TextSegment;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

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
public class _XAccessibleText {

    private LogWriter log;

    private static final String className =
        "com.sun.star.accessibility.XAccessibleText" ;

    public XAccessibleText oObj = null;
    private XMultiServiceFactory xMSF;

    Rectangle chBounds = null;
    int chCount = 0;

    String text = null;
    String editOnly = null;


    public _XAccessibleText(XInterface object, LogWriter log, XMultiServiceFactory xMSF, String editOnly) {
        oObj = (XAccessibleText)UnoRuntime.queryInterface(
                                    XAccessibleText.class, object);
        this.xMSF = xMSF;
        this.log = log;
        this.editOnly = editOnly;
    }


    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value is equal to <code>chCount - 1</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setCaretPosition()</code> </li>
     * </ul>
     */
    public boolean _getCaretPosition() {

        if (editOnly != null) {
            log.println(editOnly);
            return true;
        }

        boolean res = true;
        if ( chCount > 0 ) {
            try {
                oObj.setCaretPosition(chCount - 1);
            } catch (com.sun.star.lang.IndexOutOfBoundsException ie) {

            }
            int carPos = oObj.getCaretPosition();
            log.println("getCaretPosition: " + carPos);
            res = carPos == (chCount - 1);
        }
        return res;
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
    public boolean _setCaretPosition() {
        boolean res = true;

        try {
            log.println("setCaretPosition(-1):");
            oObj.setCaretPosition(-1);
            res &= false;
            log.println("exception was expected");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.println("setCaretPosition(chCount+1):");
            oObj.setCaretPosition(chCount+1);
            res &= false;
            log.println("exception was expected");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }
        if ( chCount > 0 ) {
            try {
                log.println("setCaretPosition(chCount - 1)");
                oObj.setCaretPosition(chCount - 1);
                res &= true;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("unexpected exception");
                e.printStackTrace();
                res &= false;
            }
        }

        return res;
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
    public boolean _getCharacter() {
        boolean res = true;

        try {
            log.println("getCharacter(-1)");
            oObj.getCharacter(-1);
            log.println("Exception was expected");
            res = false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res = true;
        }

        try {
            log.println("getCharacter(chCount)");
            oObj.getCharacter(chCount);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("Checking of every character in the text...");
            boolean isEqCh = true;
            for(int i = 0; i < chCount; i++) {
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
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
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
    public boolean _getCharacterAttributes() {
        boolean res = true;

        try {
            log.println("getCharacterAttributes(-1)");
            oObj.getCharacterAttributes(-1, new String[0]);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getCharacterAttributes(chCount)");
            oObj.getCharacterAttributes(chCount, new String[0]);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            if ( chCount > 0 ) {
                log.println("getCharacterAttributes(chCount-1)");
                PropertyValue[] props = oObj.getCharacterAttributes(chCount - 1, new String[0]);
                res &= props != null;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
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
    public boolean _getCharacterBounds() {
        boolean res = true;

        try {
            log.println("getCharacterBounds(-1)");
            oObj.getCharacterBounds(-1);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getCharacterBounds(chCount)");
            oObj.getCharacterBounds(chCount);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            if (chCount > 0) {
                log.println("getCharacterBounds(chCount-1)");
                chBounds = oObj.getCharacterBounds(chCount-1);
                res &= chBounds != null;
                log.println("rect: " + chBounds.X + ", " + chBounds.Y + ", " +
                    chBounds.Width + ", " + chBounds.Height);
            }

        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }


    /**
     * Calls the method and stores a returned value to the variable
     * <code>chCount</code>.
     * Has OK status if a returned value is equal to the text length.
     */
    public boolean _getCharacterCount() {
        chCount = oObj.getCharacterCount();
        log.println("Character count:" + chCount);
        boolean res = chCount == text.length();
        return res;
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
    public boolean _getIndexAtPoint() {

        boolean res = true;
        log.println("getIndexAtPoint(-1, -1):");
        Point pt = new Point(-1, -1);
        int index = oObj.getIndexAtPoint(pt);
        log.println(Integer.toString(index));
        res &= index == -1;

        if (chBounds != null) {
            pt = new Point(chBounds.X , chBounds.Y );
            log.println("getIndexAtPoint(" + pt.X + ", " + pt.Y + "):");
            index = oObj.getIndexAtPoint(pt);
            log.println(Integer.toString(index));
            res &= index == (chCount - 1);
        }

        return res;
    }

    /**
     * Checks a returned values after different calls of the method
     * <code>setSelection()</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setSelection()</code> </li>
     * </ul>
     */
    public boolean _getSelectedText() {
        if (editOnly != null) {
            log.println(editOnly);
            return true;
        }

        boolean res = true;

        try {
            log.println("setSelection(0, 0)");
            oObj.setSelection(0, 0);
            log.println("getSelectedText():");
            String txt = oObj.getSelectedText();
            log.println("'" + txt + "'");
            res &= txt.length() == 0;

            log.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            log.println("getSelectedText():");
            txt = oObj.getSelectedText();
            log.println("'" + txt + "'");
            res &= txt.equals(text);

            if (chCount > 2) {
                log.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                log.println("getSelectedText():");
                txt = oObj.getSelectedText();
                log.println("'" + txt + "'");
                res &= txt.equals(text.substring(1, chCount - 1));
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }

    /**
     * Checks a returned values after different calls of the method
     * <code>setSelection()</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setSelection()</code> </li>
     * </ul>
     */
    public boolean _getSelectionStart() {
        if (editOnly != null) {
            log.println(editOnly);
            return true;
        }

        boolean res = true;

        try {
            log.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            int start = oObj.getSelectionStart();
            log.println("getSelectionStart():" + start);
            res &= start == 0;

            if (chCount > 2) {
                log.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                start = oObj.getSelectionStart();
                log.println("getSelectionStart():" + start);
                res &= start == 1;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }

    /**
     * Checks a returned values after different calls of the method
     * <code>setSelection()</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setSelection()</code> </li>
     * </ul>
     */
    public boolean _getSelectionEnd() {
        if (editOnly != null) {
            log.println(editOnly);
            return true;
        }

        boolean res = true;

        try {
            log.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            int end = oObj.getSelectionEnd();
            log.println("getSelectionEnd():" + end);
            res &= end == chCount;

            if (chCount > 2) {
                log.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                end = oObj.getSelectionEnd();
                log.println("getSelectionEnd():" + end);
                res &= end == chCount - 1;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
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
    public boolean _setSelection() {
        boolean res = true;
        boolean locRes = true;

        if (editOnly != null) {
            log.println(editOnly);
            return true;
        }

        try {
            log.println("setSelection(-1, chCount-1):");
            locRes = oObj.setSelection(-1, chCount - 1);
            log.println(locRes + " exception was expected");
            res &= !locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("setSelection(0, chCount+1):");
            locRes = oObj.setSelection(0, chCount + 1);
            log.println(locRes + " excepion was expected");
            res &= !locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            if (chCount > 2) {
                log.println("setSelection(1, chCount-1):");
                locRes = oObj.setSelection(1, chCount - 1);
                log.println(Boolean.toString(locRes));
                res &= locRes;

                log.println("setSelection(chCount-1, 1):");
                locRes = oObj.setSelection(chCount - 1, 1);
                log.println(Boolean.toString(locRes));
                res &= locRes;
            }

            if (chCount > 1) {
                log.println("setSelection(0, chCount-1):");
                locRes = oObj.setSelection(0, chCount-1);
                log.println(Boolean.toString(locRes));
                res &= locRes;

                log.println("setSelection(chCount-1, 0):");
                locRes = oObj.setSelection(chCount-1, 0);
                log.println(Boolean.toString(locRes));
                res &= locRes;
            }

            log.println("setSelection(0, 0):");
            locRes = oObj.setSelection(0, 0);
            log.println(Boolean.toString(locRes));
            res &= locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned string is not null
     * received from relation.
     */
    public boolean _getText() {
        text = oObj.getText();
        log.println("getText: '" + text + "'");
        return (text != null);
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
    public boolean _getTextRange() {
        boolean res = true;
        boolean locRes = true;

        try {
            if (chCount > 3) {
                log.println("getTextRange(1, chCount - 2): ");
                String txtRange = oObj.getTextRange(1, chCount - 2);
                log.println(txtRange);
                locRes = txtRange.equals(text.substring(1, chCount - 2));
                res &= locRes;
                if (!locRes) {
                    log.println("Was expected: " +
                        text.substring(1, chCount - 2));
                }
            }

            if (chCount > 0) {
                log.println("getTextRange(0, chCount-1): ");
                String txtRange = oObj.getTextRange(0, chCount-1);
                log.println(txtRange);
                locRes = txtRange.equals(text.substring(0, chCount - 1));
                res &= locRes;
                if (!locRes) {
                    log.println("Was expected: " +
                        text.substring(0, chCount - 1));
                }

                log.println("getTextRange(chCount, 0): ");
                txtRange = oObj.getTextRange(chCount, 0);
                log.println(txtRange);
                res &= txtRange.equals(text);

                log.println("getTextRange(0, 0): ");
                txtRange = oObj.getTextRange(0, 0);
                log.println(txtRange);
                locRes = txtRange.equals("");
                res &= locRes;
                if (!locRes) {
                    log.println("Empty string was expected");
                }
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        try {
            log.println("getTextRange(-1, chCount - 1): ");
            String txtRange = oObj.getTextRange(-1, chCount - 1);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getTextRange(0, chCount + 1): ");
            String txtRange = oObj.getTextRange(0, chCount + 1);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getTextRange(chCount+1, -1): ");
            String txtRange = oObj.getTextRange(chCount+1, -1);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        return res;
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
    public boolean _getTextAtIndex() {
        boolean res = true;

        try {
            log.println("getTextAtIndex(-1, AccessibleTextType.PARAGRAPH):");
            TextSegment txt =
                oObj.getTextAtIndex(-1, AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("getTextAtIndex(chCount+1," +
                " AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextAtIndex(chCount + 1,
                 AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception");
            res &= true;
        }


        try {
            if ( chCount > 0 ) {
                log.println("getTextAtIndex(chCount," +
                    " AccessibleTextType.PARAGRAPH):");
                TextSegment txt = oObj.getTextAtIndex(chCount,
                    AccessibleTextType.PARAGRAPH);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;

                log.println("getTextAtIndex(1," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextAtIndex(1,
                    AccessibleTextType.PARAGRAPH);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text);
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected exception");
            res &= false;
        }


        return res;
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
    public boolean _getTextBeforeIndex() {
        boolean res = true;

        try {
            log.println("getTextBeforeIndex(-1, AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBeforeIndex(-1,
                AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception");
            res &= true;
        }


        try {
            log.println("getTextBeforeIndex(chCount+1, " +
                "AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBeforeIndex(chCount + 1,
                AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception");
            res &= true;
        }

        TextSegment txt = null;
        try {
            if (chCount > 0) {
                log.println("getTextBeforeIndex(chCount," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextBeforeIndex(chCount,
                    AccessibleTextType.PARAGRAPH);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == chCount ;

                log.println("getTextBeforeIndex(1," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextBeforeIndex(1,
                    AccessibleTextType.PARAGRAPH);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;
            }

            if (chCount > 2) {
                log.println("getTextBeforeIndex(chCount-1," +
                    " AccessibleTextType.CHARACTER):");
                txt = oObj.getTextBeforeIndex(chCount - 1,
                    AccessibleTextType.CHARACTER);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(chCount - 2, chCount - 1));
                log.println("getTextBeforeIndex(2," +
                    " AccessibleTextType.CHARACTER):");
                txt = oObj.getTextBeforeIndex(2,
                     AccessibleTextType.CHARACTER);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(1, 2));
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected exception");
            res &= false;
        }


        return res;
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
    public boolean _getTextBehindIndex() {
        boolean res = true;

        try {
            log.println("getTextBehindIndex(-1, AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBehindIndex(-1,
                AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception");
            res &= true;
        }


        try {
            log.println("getTextBehindIndex(chCount+1, " +
                "AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBehindIndex(chCount + 1,
                AccessibleTextType.PARAGRAPH);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected exception");
            res &= true;
        }


        try {
            if ( chCount > 0 ) {
                log.println("getTextBehindIndex(chCount," +
                    " AccessibleTextType.PARAGRAPH):");
                TextSegment txt = oObj.getTextBehindIndex(chCount,
                    AccessibleTextType.PARAGRAPH);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;

                log.println("getTextBehindIndex(chCount-1," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextBehindIndex(chCount - 1,
                    AccessibleTextType.PARAGRAPH);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;
            }
            if ( chCount > 1 ) {
                log.println("getTextBehindIndex(1," +
                    " AccessibleTextType.CHARACTER):");
                TextSegment txt = oObj.getTextBehindIndex(1,
                    AccessibleTextType.CHARACTER);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(2, 3));
            }
            if (chCount > 2) {
                log.println("getTextBehindIndex(chCount-2," +
                    " AccessibleTextType.CHARACTER):");
                TextSegment txt = oObj.getTextBehindIndex(chCount - 2,
                     AccessibleTextType.CHARACTER);
                log.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(chCount - 1, chCount));
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected exception");
            res &= false;
        }


        return res;
    }

    /**
     * Calls the method with invalid parameters an with valid parameter,
     * checks returned values.
     * Has OK status if exception was thrown for invalid parameters,
     * if exception wasn't thrown for valid parameter and if returned value for
     * valid parameter is equal to <code>true</code>.
     */
    public boolean _copyText() {
        boolean res = true;
        boolean locRes = true;

        if (editOnly != null) {
            log.println(editOnly);
            return true;
        }

        try {
            log.println("copyText(-1,chCount):");
            oObj.copyText(-1, chCount);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("copyText(0,chCount+1):");
            oObj.copyText(0, chCount + 1);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("copyText(0,chCount):");
            locRes = oObj.copyText(0, chCount);
            log.println(""+locRes);
            res &= locRes;

            String cbText = null;
            try {
                cbText =
                    util.SysUtils.getSysClipboardText(xMSF);
            } catch (com.sun.star.uno.Exception e) {
                log.println("Couldn't access system clipboard :");
                e.printStackTrace();
            }
            log.println("Clipboard: '" + cbText + "'");
            res &= text.equals(cbText);

            if (chCount > 2) {
                log.println("copyText(1,chCount-1):");
                locRes = oObj.copyText(1, chCount - 1);
                log.println(""+locRes);
                res &= locRes;

                try {
                    cbText = util.SysUtils.getSysClipboardText(xMSF);
                } catch (com.sun.star.uno.Exception e) {
                    log.println("Couldn't access system clipboard :");
                    e.printStackTrace();
                }

                log.println("Clipboard: '" + cbText + "'");
                res &= text.substring(1, chCount - 1).equals(cbText);
            }

        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }
}
