/*************************************************************************
 *
 *  $RCSfile: _XTextRange.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:20:37 $
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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.text.XTextRange
*/
public class _XTextRange extends MultiMethodTest {

    public XTextRange oObj = null;            // oObj is filled by setField()
                                            // in MultiMethodTest
    XTextRange oStartRange = null;          // startrange of textrang
    XTextRange oEndRange = null;            // endrange of textrang
    String startStr = null;                 // string in startrange
    String endStr = null;                   // string in endrange

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

        tRes.tested( "getEnd()", oEndRange != null &&
            exp.endsWith("End"));
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
            txt.getString().equals("StartMiddleEnd"));
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


