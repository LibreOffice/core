/*************************************************************************
 *
 *  $RCSfile: _XTextRangeCompare.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:20:51 $
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
import lib.StatusException;

import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextRangeCompare;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Testing <code>com.sun.star.text.XTextRangeCompare</code>
 * interface methods :
 * <ul>
 *  <li><code> compareRegionStarts()</code></li>
 *  <li><code> compareRegionEnds()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'TEXTDOC'</code> <b>optional</b>
 *  (must implement <code>XTextDocument</code>):
 *   can be used to obtain <code>Text</code> of the document from
 *   which cursors can be created.
 *   If the relation does not exist, the relation <code>TEXT</code>
 *   must be specified. </li>
 *  <li> <code>'TEXT'</code> <b>optional</b>
 *  (of type <code>XText</code>):
 *   used to create text cursor.
 *   If the relation does not exist, the relation <code>TEXTDOC</code>
 *   must be specified. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextRangeCompare
 */
public class _XTextRangeCompare extends MultiMethodTest {

    public XTextRangeCompare oObj = null;


    String nameStr = null;

    XTextRange oRange = null;
    XTextCursor cursor1 = null;
    XTextCursor cursor2 = null;
    String startStr = null;
    String endStr = null;
    XText oText = null;

    /**
     * Retrieves <code>XText</code> interface from relation 'TEXTDOC'
     * or from 'TEXT'.
     * @throws StatusException If neither 'TEXTDOC' nore 'TEXT'
     * relation exists.
     */
    public void before() {
        nameStr = this.getClass().getName();

        XInterface oIfc = (XInterface)tEnv.getObjRelation("TEXTDOC");
        if (oIfc!=null) {
            XTextDocument oTDoc = (XTextDocument)UnoRuntime.queryInterface(
                                      XTextDocument.class, oIfc);
            oText = oTDoc.getText();
        }
        XText aText = (XText) tEnv.getObjRelation("TEXT");
        if (aText != null) {
            oText = aText;
        }

        if (oText == null) {
            throw new StatusException(Status.failed
                ("Neither 'TEXTDOC' nore 'TEXT' relation not found")) ;
        }
    }

    /**
     * One cursor is created and to its position a paragraph
     * inserted, then second cursor created and to its position
     * a paragraph inserted. After that two cusors' ends are
     * compared. <p>
     *
     * Has <b>OK</b> status if the compare returns 1, i.e.
     * the second cursor end is before the first.
     */
    public void _compareRegionEnds() {
        boolean bResult = false;
        short n = 0;
        log.println( "testing compareRegionEnds()" );

        try{
          cursor1 = oText.createTextCursor();
          oText.insertString(cursor1, nameStr, false);
          oText.insertControlCharacter(cursor1,
                               ControlCharacter.PARAGRAPH_BREAK, false);
          cursor2 = oText.createTextCursor();
          oText.insertString(cursor2, nameStr + nameStr, false);
          oText.insertControlCharacter(cursor2,
                               ControlCharacter.PARAGRAPH_BREAK, false);
        }catch(com.sun.star.lang.IllegalArgumentException e){
            log.println( "Exception occurs while inserting strings: " );
            e.printStackTrace(log);
        }

        try{
            n = oObj.compareRegionEnds(cursor2, cursor1);
            log.println( "Result (short) : " + n );
        }catch(com.sun.star.lang.IllegalArgumentException e){
            log.println( "Exception: " + e);
        }

        if (n == 1){bResult = true;}
        tRes.tested( "compareRegionEnds()", bResult );
    }

    /**
     * One cursor is created and to its position a paragraph
     * inserted, then second cursor created and to its position
     * a paragraph inserted. After that two cusors' starts are
     * compared. <p>
     *
     * Has <b>OK</b> status if the compare returns 1, i.e.
     * the second cursor start is before the first.
     */
    public void _compareRegionStarts() {
        boolean bResult = false;
        short n = 0;

        try{
            cursor1 = oText.createTextCursor();
            oText.insertString(cursor1, nameStr, false);
            oText.insertControlCharacter(cursor1,
                               ControlCharacter.PARAGRAPH_BREAK, false);
            cursor2 = oText.createTextCursor();
            oText.insertString(cursor2, nameStr + nameStr, false);
            oText.insertControlCharacter(cursor2,
                               ControlCharacter.PARAGRAPH_BREAK, false);
        }catch(com.sun.star.lang.IllegalArgumentException e){
            log.println( "Exception occurs while inserting strings: " + e);
        }
        try{
            n = oObj.compareRegionStarts(cursor2, cursor1);
            log.println( "Result (short) : " + n );
        }catch(com.sun.star.lang.IllegalArgumentException e){
            log.println( "Exception: " + e);
        }
        if (n == 1){bResult = true;}
        tRes.tested( "compareRegionStarts()", bResult );
    }

}

