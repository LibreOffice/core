/*************************************************************************
 *
 *  $RCSfile: _XRelativeTextContentInsert.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:18:00 $
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
import util.XInstCreator;

import com.sun.star.text.XRelativeTextContentInsert;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Testing <code>com.sun.star.text.XRelativeTextContentInsert</code>
 * interface methods :
 * <ul>
 *  <li><code> insertTextContentBefore()</code></li>
 *  <li><code> insertTextContentAfter()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'PARA'</code> (of type <code>XInstCreator</code>):
 *   the creator which can create instances of
 *   <code>com.sun.star.text.Paragraph</code> service. </li>
 *  <li> <code>'XTEXTINFO'</code> (of type <code>XInstCreator</code>):
 *   the creator which can create instances of soem text content
 *   service (objects which implement <code>XTextContent</code>).
 *  </li>
 * <ul> <p>
 *
 * Tested component <b>must implement</b> <code>XText</code>
 * interface for proper testing. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XRelativeTextContentInsert
 */
public class _XRelativeTextContentInsert extends MultiMethodTest {

    public XRelativeTextContentInsert oObj = null;
    public XTextContent content = null;

    /**
     * First an instance of <code>Paragraph</code> service created
     * using relation and inserted into text. Then an instance
     * of text content is created and inserted after the paragraph. <p>
     *
     * Has <b>OK</b> status if no exceptions occured.
     */
    public void _insertTextContentAfter() {

        try {
            XInstCreator para = (XInstCreator)tEnv.getObjRelation( "PARA" );
            XInterface oInt = para.createInstance();
            XTextContent new_content = (XTextContent) oInt;
            XText theText = (XText)
                UnoRuntime.queryInterface(XText.class,oObj);
            XTextCursor oCursor = theText.createTextCursor();
            XInstCreator info = (XInstCreator)
                tEnv.getObjRelation( "XTEXTINFO" );
            oInt = info.createInstance();
            content = (XTextContent) oInt;
            theText.insertTextContent(oCursor, content, false);
            oObj.insertTextContentAfter(new_content,content);
            tRes.tested("insertTextContentAfter()",true);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while checking "+
                "insertTextContentAfter()");
            ex.printStackTrace(log);
            tRes.tested("insertTextContentAfter()",false);
        }


    } // end _insertTextContentAfter()

    /**
     * An instance of text content is created using relation
     * and inserted before the paragraph which was added into
     * text in <code>insertTextContentAfter</code> method test. <p>
     *
     * Has <b>OK</b> status if no exceptions occured. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertTextContentAfter() </code> : here the
     *  <code>Paragraph</code> instance is inserted. </li>
     * </ul>
     */
    public void _insertTextContentBefore() {
        requiredMethod("insertTextContentAfter()");
        try {
            XInstCreator para = (XInstCreator)tEnv.getObjRelation( "PARA" );
            XInterface oInt = para.createInstance();
            XTextContent new_content = (XTextContent) oInt;
            oObj.insertTextContentBefore(new_content,content);
            tRes.tested("insertTextContentBefore()",true);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while checking "+
                "insertTextContentBefore()");
            ex.printStackTrace(log);
            tRes.tested("insertTextContentBefore()",false);
        }


    } // end _insertTextContentBefore()

}

