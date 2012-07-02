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

package ifc.accessibility;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.beans.PropertyValue;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleEditableText</code>
 * interface methods :
 * <ul>
 *  <li><code> cutText()</code></li>
 *  <li><code> pasteText()</code></li>
 *  <li><code> deleteText()</code></li>
 *  <li><code> insertText()</code></li>
 *  <li><code> replaceText()</code></li>
 *  <li><code> setAttributes()</code></li>
 *  <li><code> setText()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XAccessibleEditableText.hasAttr'</code>
 *  (of type <code>Boolean</code>):
 *   Indicates whether or not the text has changeable attributes.
 *   E.g. text within writer document have attributes which can
 *   be changed, while the text within edit field has fixed
 *   attributes. <p>
 *   If the relation is <code>false</code> then the component
 *   has fixed text attributes. </li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEditableText
 */
public class _XAccessibleEditableText extends MultiMethodTest {

    public XAccessibleEditableText oObj = null;


    String pasteText = null;

    String initialText = "";

    /**
     * Indicates whether or not the text has changeable attributes.
     * E.g. text within writer document have attributes which can
     * be changed, while the text within edit field has fixed
     * attributes.
     */
     private boolean changeableAttr = true;

     /**
      * Retrieves object relation. Stores initial component text
      * for restoding it in <code>after</code>.
      */
     protected void before() {
        Boolean b = (Boolean)
            tEnv.getObjRelation("XAccessibleEditableText.hasAttr");
        if (b != null) {
            changeableAttr = b.booleanValue();
        }

        initialText = oObj.getText();
     }

    /**
     * Calls the method with the wrong indexes and with the correct indexes.
     * Stores cutted text in the variable <code>pasteText</code>.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct indexes.
     */
    public void _cutText() {
        boolean res = true;
        boolean locRes = true;
        String curText = null;

        String oldText = oObj.getText();
        log.println("Text: '" + oldText + "'");
        int length = oObj.getCharacterCount();
        log.println("Character count: " + length);

        try {
            log.print("cutText(-1," + (length-1) + "): ");
            locRes = oObj.cutText(-1, length - 1);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(oldText);
        }

        try {
            log.print("cutText(0," + (length+1) + "): ");
            locRes = oObj.cutText(0, length + 1);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(oldText);
        }

        try {
            pasteText = oldText;
            log.print("cutText(0," + length + "): ");
            locRes = oObj.cutText(0, length);
            log.println(locRes);
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.length() == 0 && locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("cutText()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct indexes.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct indexes and if cutted text was
     * pasted.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>cutText()</code> </li>
     * </ul>
     */
    public void _pasteText() {
        requiredMethod("cutText()");
        boolean res = true;
        boolean locRes = true;
        String curText = null;

        String text = oObj.getText();
        log.println("Text: '" + text + "'");
        int length = oObj.getCharacterCount();
        log.println("Character count: " + length);

        try {
            log.print("pasteText(-1): ");
            locRes = oObj.pasteText(-1);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("pasteText(" + (length+1) + "): ");
            locRes = oObj.pasteText(length + 1);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("pasteText(" + (length) + "): ");
            locRes = oObj.pasteText(length);
            log.println(locRes);
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text + pasteText) && locRes;
            log.println("Expected text: '" + text + pasteText + "'");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("pasteText()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct indexes,
     * checks text after method call.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct indexes and if deleted string
     * was really deleted from the text.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>insertText()</code> </li>
     * </ul>
     */
    public void _deleteText() {
        executeMethod("insertText()");
        boolean res = true;
        boolean locRes = true;
        String curText = null;

        String text = oObj.getText();
        log.println("Text: '" + text + "'");
        int length = oObj.getCharacterCount();
        log.println("Character count: " + length);

        try {
            log.print("deleteText(-1," + length + "): ");
            locRes = oObj.deleteText(-1, length);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("deleteText(0," + (length+1) + "): ");
            locRes = oObj.deleteText(0, length + 1);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            if (length >= 1) {
                log.print("deleteText(" + (length-1) + "," + (length) + "): ");
                locRes = oObj.deleteText(length - 1, length);
                log.println(locRes);
                String expStr = text.substring(0, length - 1);
                curText = oObj.getText();
                log.println("Current text: '" + curText + "'");
                res &= curText.equals(expStr);
                log.println("Expected text: '" + expStr + "'");
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("deleteText()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct indexes,
     * checks text after method call.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct indexes and if inserted string
     * was really inserted into the text.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>pasteText()</code> </li>
     * </ul>
     */
    public void _insertText() {
        executeMethod("pasteText()");
        boolean res = true;
        boolean locRes = true;
        String curText = null;

        String text = oObj.getText();
        log.println("Text: '" + text + "'");
        int length = oObj.getCharacterCount();
        log.println("Character count: " + length);

        final String insStr = "Inserted string";

        try {
            log.print("insertText(insStr, -1): ");
            locRes = oObj.insertText(insStr, -1);
            log.println(locRes);
            log.println("exception was expected=> FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("insertText(insStr," + (length+1) + "): ");
            locRes = oObj.insertText(insStr, length+1);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("insertText(insStr," + length + "): ");
            locRes = oObj.insertText(insStr, length);
            log.println(locRes);
            curText = oObj.getText();
            res &= curText.equals(text + insStr);
            log.println("Current text: '" + curText + "'");
            log.println("Expected text: '" + text + insStr + "'");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception => FAILED");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("insertText()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct indexes,
     * checks text after method call.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct indexes and if part of text
     * was really replaced by the specified replacement string.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>deleteText()</code> </li>
     * </ul>
     */
    public void _replaceText() {
        executeMethod("deleteText()");
        boolean res = true;
        boolean locRes = true;
        String curText = null;

        final String sReplacement = "String for replace";
        String oldText = oObj.getText();
        int startIndx = oldText.length();
        oObj.setText(oldText + " part of string for replace");

        String text = oObj.getText();
        log.println("Text: '" + text + "'");
        int length = oObj.getCharacterCount();
        log.println("Character count: " + length);

        try {
            log.print("replaceText(-1," + length + "): ");
            locRes = oObj.replaceText(-1, length, sReplacement);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("replaceText(0," + (length+1) + "): ");
            locRes = oObj.replaceText(0, length + 1, sReplacement);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            res &= curText.equals(text);
        }

        try {
            log.print("replaceText(" + startIndx + "," + length + "): ");
            locRes = oObj.replaceText(startIndx, length, sReplacement);
            log.println(locRes);
            curText = oObj.getText();
            log.println("Current text: '" + curText + "'");
            log.println("Expected text: '" + oldText + sReplacement + "'");
            res &= curText.equals(oldText + sReplacement);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("replaceText()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct indexes,
     * checks attributes after method call.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct indexes and if attributes
     * of text was changed.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>replaceText()</code> </li>
     * </ul>
     */
    public void _setAttributes() {
        executeMethod("replaceText()");
        boolean res = true;
        boolean locRes = true;

        String text = oObj.getText();
        log.println("Text: '" + text + "'");
        int length = oObj.getCharacterCount();
        log.println("Length: " + length);

        PropertyValue[] attrs = null;

        try {
            attrs = oObj.getCharacterAttributes(0, new String[]{""});
            log.print("setAttributes(-1," + (length - 1) + "):");
            locRes = oObj.setAttributes(-1, length - 1, attrs);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            res &= true;
        }

        try {
            log.print("setAttributes(0," + (length+1) + "):");
            locRes = oObj.setAttributes(0, length + 1, attrs);
            log.println(locRes);
            log.println("exception was expected => FAILED");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception => OK");
            res &= true;
        }

        //change old attributes set
        for(int i = 0; i < attrs.length; i++) {
            if (attrs[i].Name.equals("CharColor")) {
                attrs[i].Value = new Integer(-2);
            }
        }

        try {
            log.print("setAttributes(0," + length + "):");
            locRes = oObj.setAttributes(0, length, attrs);
            log.println(locRes);
            res &= (changeableAttr && locRes)
                || (!changeableAttr && !locRes);
            if (changeableAttr) {
                log.print("checking that new attributes was set...");
                PropertyValue[] newAttrs = oObj.getCharacterAttributes(0, new String[]{""});
                locRes = ValueComparer.equalValue(attrs, newAttrs);
                log.println(locRes);
                res &= locRes;
            } else {
                log.println("Text attributes can't be changed.");
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("unexpected exception => FAILED");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("setAttributes()", res);
    }

    /**
     * Calls the method with different parameters and checks text.
     */
    public void _setText() {
        executeMethod("setAttributes()");
        boolean res = true;
        boolean locRes = true;

        String oldText = oObj.getText();
        log.println("Current text: '" + oldText + "'");

        String newText = "New text";
        log.print("setText('" + newText + "'): ");
        locRes = oObj.setText(newText);
        log.println(locRes);
        String newCurText = oObj.getText();
        log.println("getText(): '" + newCurText + "'");
        res &= locRes && newCurText.equals(newText);

        newText = "";
        log.print("setText('" + newText + "'): ");
        locRes = oObj.setText(newText);
        log.println(locRes);
        newCurText = oObj.getText();
        log.println("getText(): '" + newCurText + "'");
        res &= locRes && newCurText.equals(newText);

        log.print("setText('" + oldText + "'): ");
        locRes = oObj.setText(oldText);
        log.println(locRes);
        newCurText = oObj.getText();
        log.println("getText(): '" + newCurText + "'");
        res &= locRes && newCurText.equals(oldText);

        tRes.tested("setText()", res);
    }

    /**
     * Restores initial component text.
     */
    protected void after() {
        oObj.setText(initialText);
    }
}