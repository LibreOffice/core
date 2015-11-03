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
package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XText;
import com.sun.star.text.XTextRange;


/**
* Testing <code>com.sun.star.text.XTextRange</code>
* interface methods :
* <ul>
*  <li><code> getText()</code></li>
*  <li><code> getStart()</code></li>
*  <li><code> getEnd()</code></li>
*  <li><code> getString()</code></li>
*  <li><code> setString()</code></li>
* </ul> <p>
* First the content is set to 'Middle' string value, then
* start range is retrieved and its content is set to 'Start'
* and end range is set to 'End'. Finally the whole TextRange
* is checked and it must be 'StartMiddleEnd'. <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.text.XTextRange
*/
public class _XTextRange extends MultiMethodTest {

    public XTextRange oObj = null;            // oObj is filled by setField()
                                            // in MultiMethodTest
    XTextRange oStartRange = null;          // startrange of textrang
    XTextRange oEndRange = null;            // endrange of textrang

    /**
    * Retrieves the start range and sets its context to
    * 'Start' string. <p>
    * Has <b>OK</b> status if the whole range string starts
    * with 'Start' substring. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setString </code>  </li>
    * </ul>
    */
    public void _getStart() {

        XText the_text = (XText) tEnv.getObjRelation("XTEXT");

        if (the_text != null) {
            the_text.setString("");
        }

        String exp="";

        oObj.setString("MiddleEnd");

        oStartRange = oObj.getStart();
        oStartRange.setString("Start");

        if (the_text !=null)  {
            exp = the_text.getString();
        } else exp = oObj.getText().getString();

        log.println("Start: "+exp);

        tRes.tested( "getStart()", oStartRange != null &&
            exp.startsWith("Start"));


        oStartRange.setString("");

    }

    /**
    * Retrieves the end range and sets its context to
    * 'End' string. <p>
    * Has <b>OK</b> status if the whole range string ends
    * with 'End' substring. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setString </code>  </li>
    * </ul>
    */
    public void _getEnd() {
        XText the_text = (XText) tEnv.getObjRelation("XTEXT");

        if (the_text != null) {
            the_text.setString("");
        }

        String exp="";
        oObj.setString("StartMiddle");

        oEndRange = oObj.getEnd();
        oEndRange.setString("End");

        if (the_text !=null)  {
            exp = the_text.getString();
        } else exp = oObj.getText().getString();

        log.println("End: "+exp);

        tRes.tested( "getEnd()", exp.endsWith("End"));

        oEndRange.setString("");
    }

    /**
    * Gets the text of the range and retrieves its String content. <p>
    * Has <b>OK</b> status if the string returned equals to
    * 'StartMiddleEnd' value. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setString </code> to get finally the string expected.</li>
    *  <li> <code> getStart </code> to get finally the string expected.</li>
    *  <li> <code> getEnd </code> to get finally the string expected.</li>
    * </ul>
    */
    public void _getText() {
        requiredMethod("setString()");
        requiredMethod("getStart()");
        requiredMethod("getEnd()");

        XText txt = oObj.getText() ;

        tRes.tested( "getText()", txt != null &&
            txt.getString().equals("StartMiddle"));
    }

    /**
    * Gets the String of the range. <p>
    * Has <b>OK</b> status if the string returned equals to
    * 'StartMiddleEnd' value. <p>
    */
    public void _getString() {

        oObj.setString("StartMiddleEnd");
        String gStr = oObj.getString() ;

        tRes.tested( "getString()", gStr != null &&
            gStr.equals("StartMiddleEnd"));

    }

    /**
    * Sets the string content of the range to 'Middle' value. <p>
    * Has <b>OK</b> status if <code>getString</code> method returns
    * 'Middle' value.
    */
    public void _setString() {
        oObj.setString("Middle") ;

        tRes.tested("setString()", "Middle".equals(oObj.getString())) ;
    }
}


