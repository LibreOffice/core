/*************************************************************************
 *
 *  $RCSfile: _XSheetAnnotation.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:02:58 $
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

package ifc.sheet;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.ValueComparer;

import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.table.CellAddress;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XSheetAnnotation</code>
* interface methods :
* <ul>
*  <li><code> getPosition()</code></li>
*  <li><code> getAuthor()</code></li>
*  <li><code> getDate()</code></li>
*  <li><code> getIsVisible()</code></li>
*  <li><code> setIsVisible()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CELLPOS'</code> (of type
*   <code>com.sun.star.table.CellAddress</code>):
*   The position of cell with annotation. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.sheet.XSheetAnnotation
*/
public class _XSheetAnnotation extends MultiMethodTest {

    public XSheetAnnotation oObj = null;

        /**
        * Gets the author of annotation. <p>
        * Has <b>OK</b> status if not null value returned.
        */
        public void _getAuthor() {
          String author = oObj.getAuthor();
          tRes.tested("getAuthor()", author != null);
        }

        /**
        * Gets the modification date of annotation. <p>
        * Has <b>OK</b> status if not null value returned.
        */
        public void _getDate() {
          String date = oObj.getDate();
          tRes.tested("getDate()", date != null);
        }

        /**
        * Sets the string of annotation, then makes it visible and
        * checks the value returned by <code>getIsVisible</code> method. <p>
        * Has <b>OK</b> status if the method returns <code>true</code>.
        */
        public void _getIsVisible() {
          XSimpleText oText  = (XSimpleText)
            UnoRuntime.queryInterface(XSimpleText.class, oObj);
          oText.setString("XSheetAnnotation");
          oObj.setIsVisible(true);
          boolean bVis = oObj.getIsVisible();
          tRes.tested("getIsVisible()", bVis);
        }

        /**
        * Gets the position of annotated cell and compares it to
        * the position passed as relation. <p>
        * Has <b>OK</b> status if these positions are equal and not
        * null.
        */
        public void _getPosition() {
          boolean bResult = false;
          CellAddress sCAddr = (CellAddress) tEnv.getObjRelation("CELLPOS") ;
          if (sCAddr == null) throw new StatusException(Status.failed
            ("Relation 'CELLPOS' not found"));

          CellAddress oCAddr = oObj.getPosition();

          bResult = (oCAddr != null) && (sCAddr != null) &&
            ValueComparer.equalValue(oCAddr, sCAddr) ;

          tRes.tested("getPosition()", bResult);
        }

        /**
        * Sets the string of annotation, makes it hidden and then
        * visible. Visibility is checked in both cases. <p>
        * Has <b>OK</b> status if the <code>getIsVisible</code> method
        * returns <code>flase</code> in the first case and <code>true</code>
        * in the second.
        */
        public void _setIsVisible() {
          boolean bResult = true;
          XSimpleText oText  = (XSimpleText)
              UnoRuntime.queryInterface(XSimpleText.class, oObj);
          oText.setString("XSheetAnnotation");
          oObj.setIsVisible(false);
          boolean bVis = oObj.getIsVisible();
          if (!bVis) {
              oObj.setIsVisible(true);
              bVis = oObj.getIsVisible();
              if (bVis) {
                  bResult = true;
              }
          }

          tRes.tested("setIsVisible()", bResult);
        }

} // EOC _XSheetAnnotation

