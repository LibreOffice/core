/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

