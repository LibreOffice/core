/*************************************************************************
 *
 *  $RCSfile: _XText.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:18:33 $
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
import lib.Status;
import util.XInstCreator;

import com.sun.star.container.XIndexAccess;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.XInterface;

/**
 * Testing <code>com.sun.star.text.XText</code>
 * interface methods :
 * <ul>
 *  <li><code> insertTextContent()</code></li>
 *  <li><code> removeTextContent()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XTEXTINFO'</code> (of type <code>lib.XInstCreator</code>):
 *   creator which can create instances of <code>XTextContent</code>
 *   implementations. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XText
 */
public class _XText extends MultiMethodTest {

    public static XText oObj = null;        // oObj filled by MultiMethodTest
    XTextCursor oCursor = null;             // textcursor
    XInstCreator info = null;               // instance creator
    XInterface oInt = null;                 // instance to insert and remove

    /**
     * First an instance of text content is created using relation
     * and inserted into text. Then the number of contents is checked
     * using the relation. Second a <code>null</code> content is tried
     * to insert. <p>
     *
     * Has <b> OK </b> status if in the first case after inserting number
     * of content objects is greater than zero and in the second
     * case <code>IllegalArgumentException</code> is thrown. <p>
     */
    public void _insertTextContent() {
        boolean result = false;
        info = (XInstCreator)tEnv.getObjRelation( "XTEXTINFO" );
        oInt = info.createInstance();

        // write to log what we try next
        log.println( "test for createTextCursor()" );
        oCursor = oObj.createTextCursor();

        // write to log what we try next
        log.println( "test for insertTextContent()" );
        try {
            oObj.insertTextContent(oCursor, (XTextContent)oInt, false);
        }
        catch( com.sun.star.lang.IllegalArgumentException iaE ){
            Status.failed(iaE.toString());
            return;
        }

        // get indexaccess to the tablecollection
        XIndexAccess xIA = info.getCollection();

        // this comparision works just because it has to be at least one
        // table at this point regardless which thread inserted it
        // there is although the possibility that the first threads call
        // failed, the second not and comparision happens after second threads
        // otherwise if something fails it should have thrown an exception
        //tRes.tested("insertTextContent()",    xIA.getCount() > 0 );

        if (xIA != null ) {
            result = (xIA.getCount()>0);
        } else {
            result = true;
        }

        if (!result) log.println("The TextContent wasn't inserted");


        // try to insert an invalid TextContent
        log.println( "test for insertTextContent" );
        try {
            oObj.insertTextContent(oCursor, null, false);
            log.println("The expected Exception doesn't occured");
            result &= false;
        }
        catch( com.sun.star.lang.IllegalArgumentException iaE ){
            // Some exception.FAILED
            log.println("Expected Exception occured");
            String msg = iaE.getMessage();
            if (msg.equals("")) {
                log.println("But there is not detailed message");
            } else {
                log.println("Detailed message: "+msg);
            }

            result &= true;
        }

        tRes.tested("insertTextContent()",    result );
    }


    /**
     * Removes the text contet added before. <p>
     * Has <b> OK </b> status if the method successfully returns
     * and no exceptions were thrown. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertTextContent() </code> : inserts the content
     *  to be removed in this test. </li>
     * </ul>
     */
    public void _removeTextContent() {

        // leads to a method which should be called previously
        requiredMethod( "insertTextContent()" );

        // write to log what we try next
        log.println( "test for  removeTextContent" );
        try {
            oObj.removeTextContent( (XTextContent)oInt );
            //oObj.removeTextContent( (XTextContent)oInt );
        }
        catch( com.sun.star.container.NoSuchElementException nseE ){
            // Some exception.FAILED
            Status.failed( nseE.toString() );
            return;
        }

        // no exception occured so it works
        tRes.tested( "removeTextContent()", true );

    }
}  // finish class _XText


