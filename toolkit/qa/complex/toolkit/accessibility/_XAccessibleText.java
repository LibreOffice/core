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

package complex.toolkit.accessibility;

import com.sun.star.accessibility.XAccessibleText;
// import lib.MultiMethodTest;
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
// import share.LogWriter;

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

    // private LogWriter log;

    public XAccessibleText oObj = null;
    private XMultiServiceFactory xMSF;

    Rectangle chBounds = null;
    int chCount = 0;

    String text = null;
    String editOnly = null;


    public _XAccessibleText(XInterface object, XMultiServiceFactory xMSF, String editOnly) {
        oObj = UnoRuntime.queryInterface(XAccessibleText.class, object);
        this.xMSF = xMSF;
        // this.log = log;
        this.editOnly = editOnly;
    }


    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value is equal to <code>chCount - 1</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>setCaretPosition()</code> </li>
     * </ul>
     * @return
     */
    public boolean _getCaretPosition() {

        if (editOnly != null) {
            System.out.println(editOnly);
            return true;
        }

        boolean res = true;
        if ( chCount > 0 ) {
            try {
                oObj.setCaretPosition(chCount - 1);
            } catch (com.sun.star.lang.IndexOutOfBoundsException ie) {

            }
            int carPos = oObj.getCaretPosition();
            System.out.println("getCaretPosition: " + carPos);
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
     * @return
     */
    public boolean _setCaretPosition() {
        boolean res = true;

        try {
            System.out.println("setCaretPosition(-1):");
            oObj.setCaretPosition(-1);
            res &= false;
            System.out.println("exception was expected");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("expected exception");
            res &= true;
        }

        try {
            System.out.println("setCaretPosition(chCount+1):");
            oObj.setCaretPosition(chCount+1);
            res &= false;
            System.out.println("exception was expected");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("expected exception");
            res &= true;
        }
        if ( chCount > 0 ) {
            try {
                System.out.println("setCaretPosition(chCount - 1)");
                oObj.setCaretPosition(chCount - 1);
                res &= true;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("unexpected exception");
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
     * @return
     */
    public boolean _getCharacter() {
        boolean res = true;

        try {
            System.out.println("getCharacter(-1)");
            oObj.getCharacter(-1);
            System.out.println("Exception was expected");
            res = false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res = true;
        }

        try {
            System.out.println("getCharacter(chCount)");
            oObj.getCharacter(chCount);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("Checking of every character in the text...");
            boolean isEqCh = true;
            for(int i = 0; i < chCount; i++) {
                char ch = oObj.getCharacter(i);
                isEqCh = ch == text.charAt(i);
                res &= isEqCh;
                if (!isEqCh) {
                    System.out.println("At the position " + i +
                        "was expected character: " + text.charAt(i));
                    System.out.println("but was returned: " + ch);
                    break;
                }
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _getCharacterAttributes() {
        boolean res = true;

        try {
            System.out.println("getCharacterAttributes(-1)");
            oObj.getCharacterAttributes(-1, new String[0]);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("getCharacterAttributes(chCount)");
            oObj.getCharacterAttributes(chCount, new String[0]);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            if ( chCount > 0 ) {
                System.out.println("getCharacterAttributes(chCount-1)");
                PropertyValue[] props = oObj.getCharacterAttributes(chCount - 1, new String[0]);
                res &= props != null;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _getCharacterBounds() {
        boolean res = true;

        try {
            System.out.println("getCharacterBounds(-1)");
            oObj.getCharacterBounds(-1);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("getCharacterBounds(chCount)");
            oObj.getCharacterBounds(chCount);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            if (chCount > 0) {
                System.out.println("getCharacterBounds(chCount-1)");
                chBounds = oObj.getCharacterBounds(chCount-1);
                res &= chBounds != null;
                System.out.println("rect: " + chBounds.X + ", " + chBounds.Y + ", " +
                    chBounds.Width + ", " + chBounds.Height);
            }

        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }


    /**
     * Calls the method and stores a returned value to the variable
     * <code>chCount</code>.
     * Has OK status if a returned value is equal to the text length.
     * @return
     */
    public boolean _getCharacterCount() {
        chCount = oObj.getCharacterCount();
        System.out.println("Character count:" + chCount);
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
     * @return
     */
    public boolean _getIndexAtPoint() {

        boolean res = true;
        System.out.println("getIndexAtPoint(-1, -1):");
        Point pt = new Point(-1, -1);
        int index = oObj.getIndexAtPoint(pt);
        System.out.println(Integer.toString(index));
        res &= index == -1;

        if (chBounds != null) {
            pt = new Point(chBounds.X , chBounds.Y );
            System.out.println("getIndexAtPoint(" + pt.X + ", " + pt.Y + "):");
            index = oObj.getIndexAtPoint(pt);
            System.out.println(Integer.toString(index));
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
     * @return
     */
    public boolean _getSelectedText() {
        if (editOnly != null) {
            System.out.println(editOnly);
            return true;
        }

        boolean res = true;

        try {
            System.out.println("setSelection(0, 0)");
            oObj.setSelection(0, 0);
            System.out.println("getSelectedText():");
            String txt = oObj.getSelectedText();
            System.out.println("'" + txt + "'");
            res &= txt.length() == 0;

            System.out.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            System.out.println("getSelectedText():");
            txt = oObj.getSelectedText();
            System.out.println("'" + txt + "'");
            res &= txt.equals(text);

            if (chCount > 2) {
                System.out.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                System.out.println("getSelectedText():");
                txt = oObj.getSelectedText();
                System.out.println("'" + txt + "'");
                res &= txt.equals(text.substring(1, chCount - 1));
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _getSelectionStart() {
        if (editOnly != null) {
            System.out.println(editOnly);
            return true;
        }

        boolean res = true;

        try {
            System.out.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            int start = oObj.getSelectionStart();
            System.out.println("getSelectionStart():" + start);
            res &= start == 0;

            if (chCount > 2) {
                System.out.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                start = oObj.getSelectionStart();
                System.out.println("getSelectionStart():" + start);
                res &= start == 1;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _getSelectionEnd() {
        if (editOnly != null) {
            System.out.println(editOnly);
            return true;
        }

        boolean res = true;

        try {
            System.out.println("setSelection(0, chCount)");
            oObj.setSelection(0, chCount);
            int end = oObj.getSelectionEnd();
            System.out.println("getSelectionEnd():" + end);
            res &= end == chCount;

            if (chCount > 2) {
                System.out.println("setSelection(1, chCount-1)");
                oObj.setSelection(1, chCount - 1);
                end = oObj.getSelectionEnd();
                System.out.println("getSelectionEnd():" + end);
                res &= end == chCount - 1;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _setSelection() {
        boolean res = true;
        boolean locRes = true;

        if (editOnly != null) {
            System.out.println(editOnly);
            return true;
        }

        try {
            System.out.println("setSelection(-1, chCount-1):");
            locRes = oObj.setSelection(-1, chCount - 1);
            System.out.println(locRes + " exception was expected");
            res &= !locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("setSelection(0, chCount+1):");
            locRes = oObj.setSelection(0, chCount + 1);
            System.out.println(locRes + " excepion was expected");
            res &= !locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            if (chCount > 2) {
                System.out.println("setSelection(1, chCount-1):");
                locRes = oObj.setSelection(1, chCount - 1);
                System.out.println(Boolean.toString(locRes));
                res &= locRes;

                System.out.println("setSelection(chCount-1, 1):");
                locRes = oObj.setSelection(chCount - 1, 1);
                System.out.println(Boolean.toString(locRes));
                res &= locRes;
            }

            if (chCount > 1) {
                System.out.println("setSelection(0, chCount-1):");
                locRes = oObj.setSelection(0, chCount-1);
                System.out.println(Boolean.toString(locRes));
                res &= locRes;

                System.out.println("setSelection(chCount-1, 0):");
                locRes = oObj.setSelection(chCount-1, 0);
                System.out.println(Boolean.toString(locRes));
                res &= locRes;
            }

            System.out.println("setSelection(0, 0):");
            locRes = oObj.setSelection(0, 0);
            System.out.println(Boolean.toString(locRes));
            res &= locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned string is not null
     * received from relation.
     * @return
     */
    public boolean _getText() {
        text = oObj.getText();
        System.out.println("getText: '" + text + "'");
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
     * @return
     */
    public boolean _getTextRange() {
        boolean res = true;
        boolean locRes = true;

        try {
            if (chCount > 3) {
                System.out.println("getTextRange(1, chCount - 2): ");
                String txtRange = oObj.getTextRange(1, chCount - 2);
                System.out.println(txtRange);
                locRes = txtRange.equals(text.substring(1, chCount - 2));
                res &= locRes;
                if (!locRes) {
                    System.out.println("Was expected: " +
                        text.substring(1, chCount - 2));
                }
            }

            if (chCount > 0) {
                System.out.println("getTextRange(0, chCount-1): ");
                String txtRange = oObj.getTextRange(0, chCount-1);
                System.out.println(txtRange);
                locRes = txtRange.equals(text.substring(0, chCount - 1));
                res &= locRes;
                if (!locRes) {
                    System.out.println("Was expected: " +
                        text.substring(0, chCount - 1));
                }

                System.out.println("getTextRange(chCount, 0): ");
                txtRange = oObj.getTextRange(chCount, 0);
                System.out.println(txtRange);
                res &= txtRange.equals(text);

                System.out.println("getTextRange(0, 0): ");
                txtRange = oObj.getTextRange(0, 0);
                System.out.println(txtRange);
                locRes = txtRange.equals("");
                res &= locRes;
                if (!locRes) {
                    System.out.println("Empty string was expected");
                }
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        try {
            System.out.println("getTextRange(-1, chCount - 1): ");
            String txtRange = oObj.getTextRange(-1, chCount - 1);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("getTextRange(0, chCount + 1): ");
            String txtRange = oObj.getTextRange(0, chCount + 1);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("getTextRange(chCount+1, -1): ");
            String txtRange = oObj.getTextRange(chCount+1, -1);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
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
     * @return
     */
    public boolean _getTextAtIndex() {
        boolean res = true;

        try {
            System.out.println("getTextAtIndex(-1, AccessibleTextType.PARAGRAPH):");
            TextSegment txt =
                oObj.getTextAtIndex(-1, AccessibleTextType.PARAGRAPH);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("getTextAtIndex(chCount+1," +
                " AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextAtIndex(chCount + 1,
                 AccessibleTextType.PARAGRAPH);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Expected exception");
            res &= true;
        }


        try {
            if ( chCount > 0 ) {
                System.out.println("getTextAtIndex(chCount," +
                    " AccessibleTextType.PARAGRAPH):");
                TextSegment txt = oObj.getTextAtIndex(chCount,
                    AccessibleTextType.PARAGRAPH);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;

                System.out.println("getTextAtIndex(1," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextAtIndex(1,
                    AccessibleTextType.PARAGRAPH);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text);
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _getTextBeforeIndex() {
        boolean res = true;

        try {
            System.out.println("getTextBeforeIndex(-1, AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBeforeIndex(-1,
                AccessibleTextType.PARAGRAPH);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Expected exception");
            res &= true;
        }


        try {
            System.out.println("getTextBeforeIndex(chCount+1, " +
                "AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBeforeIndex(chCount + 1,
                AccessibleTextType.PARAGRAPH);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        TextSegment txt = null;
        try {
            if (chCount > 0) {
                System.out.println("getTextBeforeIndex(chCount," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextBeforeIndex(chCount,
                    AccessibleTextType.PARAGRAPH);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == chCount ;

                System.out.println("getTextBeforeIndex(1," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextBeforeIndex(1,
                    AccessibleTextType.PARAGRAPH);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;
            }

            if (chCount > 2) {
                System.out.println("getTextBeforeIndex(chCount-1," +
                    " AccessibleTextType.CHARACTER):");
                txt = oObj.getTextBeforeIndex(chCount - 1,
                    AccessibleTextType.CHARACTER);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(chCount - 2, chCount - 1));
                System.out.println("getTextBeforeIndex(2," +
                    " AccessibleTextType.CHARACTER):");
                txt = oObj.getTextBeforeIndex(2,
                     AccessibleTextType.CHARACTER);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(1, 2));
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _getTextBehindIndex() {
        boolean res = true;

        try {
            System.out.println("getTextBehindIndex(-1, AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBehindIndex(-1,
                AccessibleTextType.PARAGRAPH);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Expected exception");
            res &= true;
        }


        try {
            System.out.println("getTextBehindIndex(chCount+1, " +
                "AccessibleTextType.PARAGRAPH):");
            TextSegment txt = oObj.getTextBehindIndex(chCount + 1,
                AccessibleTextType.PARAGRAPH);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Expected exception");
            res &= true;
        }


        try {
            if ( chCount > 0 ) {
                System.out.println("getTextBehindIndex(chCount," +
                    " AccessibleTextType.PARAGRAPH):");
                TextSegment txt = oObj.getTextBehindIndex(chCount,
                    AccessibleTextType.PARAGRAPH);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;

                System.out.println("getTextBehindIndex(chCount-1," +
                    " AccessibleTextType.PARAGRAPH):");
                txt = oObj.getTextBehindIndex(chCount - 1,
                    AccessibleTextType.PARAGRAPH);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.length() == 0;
            }
            if ( chCount > 1 ) {
                System.out.println("getTextBehindIndex(1," +
                    " AccessibleTextType.CHARACTER):");
                TextSegment txt = oObj.getTextBehindIndex(1,
                    AccessibleTextType.CHARACTER);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(2, 3));
            }
            if (chCount > 2) {
                System.out.println("getTextBehindIndex(chCount-2," +
                    " AccessibleTextType.CHARACTER):");
                TextSegment txt = oObj.getTextBehindIndex(chCount - 2,
                     AccessibleTextType.CHARACTER);
                System.out.println("'" + txt.SegmentText + "'");
                res &= txt.SegmentText.equals(text.substring(chCount - 1, chCount));
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }  catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Unexpected exception");
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
     * @return
     */
    public boolean _copyText() {
        boolean res = true;
        boolean locRes = true;

        if (editOnly != null) {
            System.out.println(editOnly);
            return true;
        }

        try {
            System.out.println("copyText(-1,chCount):");
            oObj.copyText(-1, chCount);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("copyText(0,chCount+1):");
            oObj.copyText(0, chCount + 1);
            System.out.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Expected exception");
            res &= true;
        }

        try {
            System.out.println("copyText(0,chCount):");
            locRes = oObj.copyText(0, chCount);
            System.out.println(""+locRes);
            res &= locRes;

            String cbText = null;
            try {
                cbText =
                    util.SysUtils.getSysClipboardText(xMSF);
            } catch (com.sun.star.uno.Exception e) {
                System.out.println("Couldn't access system clipboard :");
                e.printStackTrace();
            }
            System.out.println("Clipboard: '" + cbText + "'");
            res &= text.equals(cbText);

            if (chCount > 2) {
                System.out.println("copyText(1,chCount-1):");
                locRes = oObj.copyText(1, chCount - 1);
                System.out.println(""+locRes);
                res &= locRes;

                try {
                    cbText = util.SysUtils.getSysClipboardText(xMSF);
                } catch (com.sun.star.uno.Exception e) {
                    System.out.println("Couldn't access system clipboard :");
                    e.printStackTrace();
                }

                System.out.println("Clipboard: '" + cbText + "'");
                res &= text.substring(1, chCount - 1).equals(cbText);
            }

        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Unexpected exception");
            e.printStackTrace();
            res &= false;
        }

        return res;
    }
}
